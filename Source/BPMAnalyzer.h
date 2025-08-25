/*
  ==============================================================================
    BPMAnalyzer.h
    BPM Detection und Tempo Matching für DJ Mixer
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include <vector>
#include <complex>

class BPMAnalyzer
{
public:
    BPMAnalyzer()
    {
        reset();
    }

    ~BPMAnalyzer() = default;

    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        setupAnalysis();
    }

    void reset()
    {
        detectedBPM = 0.0;
        confidence = 0.0;
        analysisComplete = false;
        audioBuffer.clear();
        beatTimes.clear();
    }

    // Audio-Samples für Analyse hinzufügen
    void processAudioBlock(const float* audioData, int numSamples)
    {
        if (analysisComplete) return;

        // Audio in internen Buffer sammeln
        for (int i = 0; i < numSamples; ++i)
        {
            audioBuffer.push_back(audioData[i]);
        }

        // Genug Samples für Analyse? (ca. 10 Sekunden)
        if (audioBuffer.size() >= static_cast<size_t>(sampleRate * 10.0))
        {
            analyzeBPM();
            analysisComplete = true;
        }
    }

    // BPM aus Audio-Datei analysieren
    bool analyzeFile(const juce::File& audioFile)
    {
        reset();

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(audioFile));
        if (!reader) return false;

        setSampleRate(reader->sampleRate);

        // Audio laden (erste 30 Sekunden für bessere Analyse)
        int samplesToRead = static_cast<int>(juce::jmin(reader->lengthInSamples,
            static_cast<juce::int64 > (sampleRate * 30.0)));

        juce::AudioBuffer<float> fileBuffer(1, samplesToRead);
        reader->read(&fileBuffer, 0, samplesToRead, 0, true, false); // Mono

        // BPM analysieren
        audioBuffer.clear();
        audioBuffer.reserve(samplesToRead);

        const float* channelData = fileBuffer.getReadPointer(0);
        for (int i = 0; i < samplesToRead; ++i)
        {
            audioBuffer.push_back(channelData[i]);
        }

        analyzeBPM();
        analysisComplete = true;

        return detectedBPM > 0.0;
    }

    double getBPM() const { return detectedBPM; }
    double getConfidence() const { return confidence; }
    bool isAnalysisComplete() const { return analysisComplete; }

    // Tempo-Matching: Berechne Pitch-Ratio für BPM-Anpassung
    double calculatePitchRatio(double targetBPM) const
    {
        if (detectedBPM <= 0.0) return 1.0;
        return targetBPM / detectedBPM;
    }

    // Automatisches BPM-Matching zwischen zwei Tracks
    static double calculateSyncRatio(double sourceBPM, double targetBPM)
    {
        if (sourceBPM <= 0.0 || targetBPM <= 0.0) return 1.0;

        double ratio = targetBPM / sourceBPM;

        // Intelligente BPM-Anpassung: Halbe/Doppelte BPMs erkennen
        if (ratio > 1.5 && ratio < 2.2) return ratio / 2.0;  // Ziel ist ~2x schneller
        if (ratio < 0.7 && ratio > 0.4) return ratio * 2.0;  // Ziel ist ~2x langsamer

        return ratio;
    }

private:
    double sampleRate = 44100.0;
    double detectedBPM = 0.0;
    double confidence = 0.0;
    bool analysisComplete = false;

    std::vector<float> audioBuffer;
    std::vector<double> beatTimes;

    void setupAnalysis()
    {
        // Setup für Audio-Analyse
    }

    void analyzeBPM()
    {
        if (audioBuffer.empty()) return;

        // Simplified BPM detection algorithm
        std::vector<float> onset_strength = calculateOnsetStrength();
        std::vector<double> tempo_candidates = findTempoCandidates(onset_strength);

        if (!tempo_candidates.empty())
        {
            detectedBPM = tempo_candidates[0];
            confidence = 0.8; // Vereinfacht
        }
    }

    std::vector<float> calculateOnsetStrength()
    {
        // Vereinfachte Onset Detection
        std::vector<float> onsets;

        const int hopSize = 512;
        const int windowSize = 1024;

        for (size_t i = 0; i + windowSize < audioBuffer.size(); i += hopSize)
        {
            float energy = 0.0f;
            for (int j = 0; j < windowSize; ++j)
            {
                energy += audioBuffer[i + j] * audioBuffer[i + j];
            }
            onsets.push_back(energy);
        }

        // High-pass filtern für Beat-Detection
        for (size_t i = 1; i < onsets.size(); ++i)
        {
            float diff = onsets[i] - onsets[i - 1];
            onsets[i] = juce::jmax(0.0f, diff);
        }

        return onsets;
    }

    std::vector<double> findTempoCandidates(const std::vector<float>& onsetStrength)
    {
        std::vector<double> candidates;

        // Autocorrelation für Tempo-Detection
        const double minBPM = 60.0;
        const double maxBPM = 200.0;
        const double hopTime = 512.0 / sampleRate;

        int minLag = static_cast<int>(60.0 / (maxBPM * hopTime));
        int maxLag = static_cast<int>(60.0 / (minBPM * hopTime));

        double bestCorrelation = 0.0;
        double bestBPM = 0.0;

        for (int lag = minLag; lag <= maxLag; ++lag)
        {
            double correlation = 0.0;
            int count = 0;

            for (size_t i = lag; i < onsetStrength.size(); ++i)
            {
                correlation += onsetStrength[i] * onsetStrength[i - lag];
                count++;
            }

            if (count > 0)
            {
                correlation /= count;

                if (correlation > bestCorrelation)
                {
                    bestCorrelation = correlation;
                    bestBPM = 60.0 / (lag * hopTime);
                }
            }
        }

        if (bestBPM > 0.0)
        {
            candidates.push_back(bestBPM);
        }

        return candidates;
    }
};
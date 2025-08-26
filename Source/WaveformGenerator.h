/*
  ==============================================================================

    WaveformGenerator.h
    Created: 26 Aug 2025 2:50:03pm
    Author:  mpue

  ==============================================================================
*/

#pragma once
/*
  ==============================================================================
    WaveformGenerator.h
    Created: 26 Aug 2025
    Author:  mpue

    Utility class für Waveform-Daten Generierung aus Audio-Dateien
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class WaveformGenerator
{
public:
    struct WaveformData
    {
        std::vector<float> samples;
        double duration;
        int sampleRate;
        bool isValid;

        WaveformData() : duration(0.0), sampleRate(0), isValid(false) {}
    };

    // Hauptfunktion - generiert Waveform-Daten aus Audio-Datei
    static WaveformData generateWaveformData(const juce::File& audioFile, int targetSamples = 2000)
    {
        WaveformData result;

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));

        if (reader == nullptr)
        {
            DBG("Could not create reader for file: " + audioFile.getFullPathName());
            return result;
        }

        result.duration = reader->lengthInSamples / reader->sampleRate;
        result.sampleRate = (int)reader->sampleRate;
        result.isValid = true;

        // Calculate how many samples to skip for desired resolution
        int totalSamples = (int)reader->lengthInSamples;
        int skipSamples = juce::jmax(1, totalSamples / targetSamples);
        int numChannels = (int)reader->numChannels;

        result.samples.reserve(targetSamples);

        // Read audio in chunks to avoid memory issues with large files
        const int bufferSize = 8192;
        juce::AudioBuffer<float> buffer(numChannels, bufferSize);

        int samplesProcessed = 0;
        juce::int64 currentPos = 0;

        while (currentPos < reader->lengthInSamples)
        {
            int samplesToRead = juce::jmin(bufferSize, (int)(reader->lengthInSamples - currentPos));

            if (!reader->read(&buffer, 0, samplesToRead, currentPos, true, true))
            {
                DBG("Error reading audio data at position: " + juce::String(currentPos));
                break;
            }

            // Process the buffer and extract peak values
            for (int i = 0; i < samplesToRead; i += skipSamples)
            {
                if (result.samples.size() >= targetSamples)
                    break;

                float peak = 0.0f;

                // Find peak value across all channels in this segment
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    const float* channelData = buffer.getReadPointer(channel);

                    // Look ahead a few samples for better peak detection
                    for (int j = 0; j < juce::jmin(skipSamples, samplesToRead - i); ++j)
                    {
                        float sample = std::abs(channelData[i + j]);
                        peak = juce::jmax(peak, sample);
                    }
                }

                result.samples.push_back(peak);
            }

            currentPos += samplesToRead;
            samplesProcessed += samplesToRead;
        }

        // Apply some smoothing to make waveform look nicer
        if (result.samples.size() > 2)
        {
            smoothWaveform(result.samples);
        }

        DBG("Generated waveform with " + juce::String(result.samples.size()) + " samples");
        DBG("Duration: " + juce::String(result.duration, 2) + " seconds");

        return result;
    }

    // Alternative: Generiert RMS-basierte Waveform (smoother)
    static WaveformData generateRMSWaveform(const juce::File& audioFile, int targetSamples = 2000)
    {
        WaveformData result;

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));

        if (reader == nullptr) return result;

        result.duration = reader->lengthInSamples / reader->sampleRate;
        result.sampleRate = (int)reader->sampleRate;
        result.isValid = true;

        int totalSamples = (int)reader->lengthInSamples;
        int samplesPerBlock = juce::jmax(1, totalSamples / targetSamples);
        int numChannels = (int)reader->numChannels;

        result.samples.reserve(targetSamples);

        const int bufferSize = 8192;
        juce::AudioBuffer<float> buffer(numChannels, bufferSize);

        juce::int64 currentPos = 0;

        while (currentPos < reader->lengthInSamples && result.samples.size() < targetSamples)
        {
            int samplesToRead = juce::jmin(bufferSize, (int)(reader->lengthInSamples - currentPos));

            if (!reader->read(&buffer, 0, samplesToRead, currentPos, true, true))
                break;

            // Calculate RMS for blocks
            for (int blockStart = 0; blockStart < samplesToRead; blockStart += samplesPerBlock)
            {
                if (result.samples.size() >= targetSamples) break;

                int blockEnd = juce::jmin(blockStart + samplesPerBlock, samplesToRead);
                float rmsSum = 0.0f;
                int rmsCount = 0;

                for (int channel = 0; channel < numChannels; ++channel)
                {
                    const float* channelData = buffer.getReadPointer(channel);

                    for (int i = blockStart; i < blockEnd; ++i)
                    {
                        float sample = channelData[i];
                        rmsSum += sample * sample;
                        rmsCount++;
                    }
                }

                float rms = 0.0f;
                if (rmsCount > 0)
                {
                    rms = std::sqrt(rmsSum / rmsCount);
                }

                result.samples.push_back(rms);
            }

            currentPos += samplesToRead;
        }

        return result;
    }

    // Hilfsfunktion für Audio-Dauer (falls du die separat brauchst)
    static double getAudioDuration(const juce::File& audioFile)
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));

        if (reader != nullptr)
        {
            return reader->lengthInSamples / reader->sampleRate;
        }

        return 0.0;
    }

    // Async Version für große Dateien (nicht-blockierend)
    static void generateWaveformDataAsync(const juce::File& audioFile,
        int targetSamples,
        std::function<void(WaveformData)> callback)
    {
        // Startet Background-Thread für Waveform-Generierung
        juce::Thread::launch([audioFile, targetSamples, callback]() {
            WaveformData data = generateWaveformData(audioFile, targetSamples);

            // Callback auf Message Thread ausführen
            juce::MessageManager::callAsync([callback, data]() {
                callback(data);
                });
            });
    }

private:
    // Smoothing-Filter für bessere Darstellung
    static void smoothWaveform(std::vector<float>& samples)
    {
        if (samples.size() < 3) return;

        // Simple 3-point moving average
        std::vector<float> smoothed;
        smoothed.reserve(samples.size());

        smoothed.push_back(samples[0]); // Erstes Sample

        for (size_t i = 1; i < samples.size() - 1; ++i)
        {
            float avg = (samples[i - 1] + samples[i] + samples[i + 1]) / 3.0f;
            smoothed.push_back(avg);
        }

        smoothed.push_back(samples.back()); // Letztes Sample

        samples = std::move(smoothed);
    }
};
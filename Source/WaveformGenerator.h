/*
  ==============================================================================

    WaveformGenerator.h
    Created: 26 Aug 2025 2:50:03pm
    Author:  mpue
    Updated: Verbesserte Waveform-Darstellung

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class WaveformGenerator
{
public:
    struct WaveformData
    {
        std::vector<float> minSamples;  // Negative Peaks
        std::vector<float> maxSamples;  // Positive Peaks
        double duration;
        int sampleRate;
        bool isValid;

        WaveformData() : duration(0.0), sampleRate(0), isValid(false) {}

        // Für Rückwärtskompatibilität - gibt RMS/Average-Werte zurück
        std::vector<float> getSamples() const
        {
            std::vector<float> result;
            result.reserve(maxSamples.size());

            for (size_t i = 0; i < maxSamples.size() && i < minSamples.size(); ++i)
            {
                // RMS-ähnlicher Wert aus min/max
                float rms = std::sqrt((maxSamples[i] * maxSamples[i] + minSamples[i] * minSamples[i]) * 0.5f);
                result.push_back(rms);
            }
            return result;
        }
    };

    // Hauptfunktion - generiert klassische Min/Max Waveform-Daten
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

        // Calculate how many samples per waveform point
        int totalSamples = (int)reader->lengthInSamples;
        int samplesPerPoint = juce::jmax(1, totalSamples / targetSamples);
        int numChannels = (int)reader->numChannels;

        result.minSamples.reserve(targetSamples);
        result.maxSamples.reserve(targetSamples);

        // Read audio in chunks
        const int bufferSize = 8192;
        juce::AudioBuffer<float> buffer(numChannels, bufferSize);

        juce::int64 currentPos = 0;
        int currentPointSamples = 0;
        float currentMin = 0.0f;
        float currentMax = 0.0f;

        while (currentPos < reader->lengthInSamples)
        {
            int samplesToRead = juce::jmin(bufferSize, (int)(reader->lengthInSamples - currentPos));

            if (!reader->read(&buffer, 0, samplesToRead, currentPos, true, true))
            {
                DBG("Error reading audio data at position: " + juce::String(currentPos));
                break;
            }

            // Process each sample in the buffer
            for (int i = 0; i < samplesToRead; ++i)
            {
                // Find min/max across all channels for this sample
                float sampleMin = 0.0f;
                float sampleMax = 0.0f;

                for (int channel = 0; channel < numChannels; ++channel)
                {
                    float sample = buffer.getSample(channel, i);
                    sampleMin = juce::jmin(sampleMin, sample);
                    sampleMax = juce::jmax(sampleMax, sample);
                }

                // Update current point's min/max
                if (currentPointSamples == 0)
                {
                    currentMin = sampleMin;
                    currentMax = sampleMax;
                }
                else
                {
                    currentMin = juce::jmin(currentMin, sampleMin);
                    currentMax = juce::jmax(currentMax, sampleMax);
                }

                currentPointSamples++;

                // When we've collected enough samples for one point, store it
                if (currentPointSamples >= samplesPerPoint ||
                    (currentPos + i + 1) >= reader->lengthInSamples)
                {
                    result.minSamples.push_back(currentMin);
                    result.maxSamples.push_back(currentMax);

                    currentPointSamples = 0;
                    currentMin = 0.0f;
                    currentMax = 0.0f;

                    if (result.maxSamples.size() >= targetSamples)
                        break;
                }
            }

            if (result.maxSamples.size() >= targetSamples)
                break;

            currentPos += samplesToRead;
        }

        // Apply smoothing for better visual appearance
        if (result.maxSamples.size() > 2)
        {
            smoothWaveform(result.minSamples, result.maxSamples);
        }

        DBG("Generated waveform with " + juce::String(result.maxSamples.size()) + " points");
        DBG("Duration: " + juce::String(result.duration, 2) + " seconds");

        return result;
    }

    // RMS-basierte Version (smoother, für Envelope-ähnliche Darstellung)
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

        result.minSamples.reserve(targetSamples);
        result.maxSamples.reserve(targetSamples);

        const int bufferSize = 8192;
        juce::AudioBuffer<float> buffer(numChannels, bufferSize);

        juce::int64 currentPos = 0;

        while (currentPos < reader->lengthInSamples && result.maxSamples.size() < targetSamples)
        {
            int samplesToRead = juce::jmin(bufferSize, (int)(reader->lengthInSamples - currentPos));

            if (!reader->read(&buffer, 0, samplesToRead, currentPos, true, true))
                break;

            // Calculate RMS for blocks
            for (int blockStart = 0; blockStart < samplesToRead; blockStart += samplesPerBlock)
            {
                if (result.maxSamples.size() >= targetSamples) break;

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

                // Für RMS: symmetrische Darstellung
                result.minSamples.push_back(-rms);
                result.maxSamples.push_back(rms);
            }

            currentPos += samplesToRead;
        }

        return result;
    }

    // Hilfsfunktion für Audio-Dauer
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

    // Async Version für große Dateien
    static void generateWaveformDataAsync(const juce::File& audioFile,
        int targetSamples,
        std::function<void(WaveformData)> callback)
    {
        juce::Thread::launch([audioFile, targetSamples, callback]() {
            WaveformData data = generateWaveformData(audioFile, targetSamples);

            juce::MessageManager::callAsync([callback, data]() {
                callback(data);
                });
            });
    }

    static void drawWaveform(juce::Graphics& g, const WaveformData& data,
        juce::Rectangle<float> bounds,
        juce::Colour colour = juce::Colours::grey,
        float zoomFactor = 1.0f,
        float offsetFactor = 0.0f)
    {
        if (!data.isValid || data.maxSamples.empty()) return;

        g.setColour(colour);

        float width = bounds.getWidth();
        float height = bounds.getHeight();
        float centerY = bounds.getCentreY();

        // Berechne sichtbaren Bereich basierend auf Zoom und Offset
        size_t totalSamples = data.maxSamples.size();
        size_t visibleSamples = (size_t)(totalSamples / zoomFactor);
        size_t startSample = (size_t)(offsetFactor * (totalSamples - visibleSamples));

        // Verhindere Out-of-Bounds
        startSample = juce::jmin(startSample, totalSamples - 1);
        size_t endSample = juce::jmin(startSample + visibleSamples, totalSamples);

        if (startSample >= endSample) return;

        juce::Path waveformPath;
        bool pathStarted = false;

        // Zeichne die obere Hälfte der Waveform (positive Werte)
        for (size_t i = startSample; i < endSample; ++i)
        {
            float relativePos = (float)(i - startSample) / (float)(endSample - startSample);
            float x = bounds.getX() + (relativePos * width);
            float y = centerY - (data.maxSamples[i] * height * 0.4f);

            if (!pathStarted)
            {
                waveformPath.startNewSubPath(x, y);
                pathStarted = true;
            }
            else
            {
                waveformPath.lineTo(x, y);
            }
        }

        // Zeichne die untere Hälfte der Waveform (negative Werte) - rückwärts
        for (int i = (int)endSample - 1; i >= (int)startSample; --i)
        {
            float relativePos = (float)(i - startSample) / (float)(endSample - startSample);
            float x = bounds.getX() + (relativePos * width);
            float y = centerY - (data.minSamples[i] * height * 0.4f);

            waveformPath.lineTo(x, y);
        }

        waveformPath.closeSubPath();
        g.fillPath(waveformPath);

        // Optional: Zeichne auch eine Mittellinie
        g.setColour(colour.withAlpha(0.3f));
        g.drawHorizontalLine((int)centerY, bounds.getX(), bounds.getRight());
    }
private:
    // Verbesserter Smoothing-Filter für Min/Max-Werte
    static void smoothWaveform(std::vector<float>& minSamples, std::vector<float>& maxSamples)
    {
        if (minSamples.size() < 3 || maxSamples.size() < 3) return;

        // 3-point moving average für beide Arrays
        auto smoothArray = [](std::vector<float>& samples) {
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
            };

        smoothArray(minSamples);
        smoothArray(maxSamples);
    }
};
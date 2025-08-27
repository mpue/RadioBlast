/*
  ==============================================================================
    Sampler.h
    Created: 17 Apr 2018 1:12:15pm
    Author:  Matthias Pueski
    Renovated: August 2025
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include "ADSR.h"

class Sampler {
public:

    explicit Sampler(float sampleRate, int bufferSize);
    ~Sampler() = default;

    // Core playback control
    void play();
    void stop();
    void reset();
    void nextSample();

    // Sample loading
    void loadSample(const juce::File& file);
    void loadSample(std::unique_ptr<juce::InputStream> input);

    // Position control
    void setStartPosition(long start) noexcept { startPosition = start; setDirty(true); }
    long getStartPosition() const noexcept { return startPosition; }

    void setEndPosition(long end) noexcept { endPosition = end; setDirty(true); }
    long getEndPosition() const noexcept { return endPosition; }

    void setSampleLength(long length) noexcept { sampleLength = length; setDirty(true); }
    long getSampleLength() const noexcept { return sampleLength; }

    void setCurrentPosition(long position) noexcept { currentSample = position; }
    long getCurrentPosition() const noexcept { return currentSample; }

    // Playback parameters
    void setLoop(bool shouldLoop) noexcept { loop = shouldLoop; }
    bool isLoop() const noexcept { return loop; }

    void setVolume(float newVolume) noexcept {
        volume = juce::jlimit(0.0f, 1.0f, newVolume);
    }
    float getVolume() const noexcept { return volume; }

    void setPitch(float newPitch) noexcept;
    float getPitch() const noexcept { return pitch; }

    // State queries
    bool hasSample() const noexcept { return loaded && sampleBuffer != nullptr; }
    bool isPlaying() const noexcept { return playing.load(); }
    bool isDone() const noexcept { return !loop && currentSample >= sampleLength - 1; }
    bool isDirty() const noexcept { return dirty.load(); }

    // Sample access
    float getSampleAt(int channel, long position) const;
    float getCurrentSample(int channel) const;
    float getOutput(int channel);

    // Buffer access
    const juce::AudioSampleBuffer* getSampleBuffer() const noexcept { return sampleBuffer.get(); }

    // Editor integration
    void setLoaded(bool isLoaded) noexcept { loaded = isLoaded; }
    void setDirty(bool isDirty) noexcept { dirty = isDirty; }

private:

    // Audio management
    std::unique_ptr<juce::AudioFormatManager> formatManager;
    std::unique_ptr<juce::AudioSampleBuffer> sampleBuffer;

    // Interpolation
    std::unique_ptr<juce::CatmullRomInterpolator> interpolatorLeft;
    std::unique_ptr<juce::CatmullRomInterpolator> interpolatorRight;

    // Envelope
    std::unique_ptr<SynthLab::ADSR> samplerEnvelope;

    // Sample parameters
    float sampleRate;
    int bufferSize;
    std::atomic<float> volume{ 0.5f };
    std::atomic<float> pitch{ 1.0f };

    // Position tracking
    std::atomic<long> currentSample{ 0 };
    long startPosition = 0;
    long endPosition = 0;
    long sampleLength = 0;

    // State flags
    std::atomic<bool> loop{ false };
    std::atomic<bool> loaded{ false };
    std::atomic<bool> playing{ false };
    std::atomic<bool> dirty{ false };

    // Thread safety
    juce::CriticalSection bufferLock;

    // Helper methods
    void initializeInterpolators();
    void validateSampleBounds();
    float interpolateSample(int channel, double position) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sampler)
};
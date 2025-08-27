/*
  ==============================================================================
    Sampler.cpp
    Created: 17 Apr 2018 1:12:15pm
    Author:  Matthias Pueski
    Renovated: August 2025
  ==============================================================================
*/

#include "Sampler.h"
#include "../AudioManager.h"
#include "SmartSample.h"

Sampler::Sampler(float sampleRate, int bufferSize)
    : sampleRate(sampleRate)
    , bufferSize(bufferSize)
    , formatManager(std::make_unique<juce::AudioFormatManager>())
    , sampleBuffer(std::make_unique<juce::AudioSampleBuffer>(2, 16384 * 1024))
    , samplerEnvelope(std::make_unique<SynthLab::ADSR>())
{
    // Initialize format manager with common formats
    formatManager->registerBasicFormats();

    // Initialize interpolators
    initializeInterpolators();

    // Configure envelope
    samplerEnvelope->setAttackRate(0.1f * sampleRate);
    samplerEnvelope->setReleaseRate(0.3f * sampleRate);
    samplerEnvelope->setSustainLevel(1.0f);
    samplerEnvelope->setDecayRate(0.1f * sampleRate);
}

void Sampler::initializeInterpolators() {
    interpolatorLeft = std::make_unique<juce::CatmullRomInterpolator>();
    interpolatorRight = std::make_unique<juce::CatmullRomInterpolator>();
}

void Sampler::validateSampleBounds() {
    if (sampleLength <= 0) return;

    startPosition = juce::jlimit(0L, sampleLength - 1, startPosition);
    endPosition = juce::jlimit(startPosition + 1, sampleLength, endPosition);
    currentSample = juce::jlimit(startPosition, endPosition - 1, currentSample.load());
}

void Sampler::nextSample() {
    if (sampleLength <= 0 || !playing.load()) return;

    const auto current = currentSample.load();

    if (isLoop()) {
        if (current < endPosition - 1) {
            currentSample = current + 1;
        }
        else {
            currentSample = startPosition;
        }
    }
    else {
        if (current < endPosition - 1) {
            currentSample = current + 1;
        }
        else {
            playing = false;
            currentSample = startPosition;
        }
    }
}

void Sampler::play() {
    if (!hasSample()) return;

    samplerEnvelope->gate(127);
    playing = true;

    // Reset to start position if we're at the end
    if (isDone()) {
        reset();
    }
}

void Sampler::stop() {
    samplerEnvelope->gate(0);
    playing = false;
}

void Sampler::reset() {
    currentSample = startPosition;
    samplerEnvelope->reset();
}

float Sampler::getCurrentSample(int channel) const {
    if (!hasSample() || isDone() || channel < 0 || channel >= 2) {
        return 0.0f;
    }

    const auto position = currentSample.load();
    const auto pitchValue = pitch.load();

    // Use interpolation for pitch shifting
    if (std::abs(pitchValue - 1.0f) > 0.001f) {
        return interpolateSample(channel, static_cast<double>(position) / pitchValue);
    }

    // Direct sample access for normal playback
    if (position >= 0 && position < sampleBuffer->getNumSamples()) {
        juce::ScopedLock lock(bufferLock);
        return sampleBuffer->getSample(channel, static_cast<int>(position));
    }

    return 0.0f;
}

float Sampler::interpolateSample(int channel, double position) const {
    if (!hasSample() || channel < 0 || channel >= sampleBuffer->getNumChannels()) {
        return 0.0f;
    }

    juce::ScopedLock lock(bufferLock);

    const int maxSample = sampleBuffer->getNumSamples() - 1;
    if (position < 0.0 || position > maxSample) {
        return 0.0f;
    }

    // Linear interpolation for real-time performance
    const int intPos = static_cast<int>(position);
    const float fraction = static_cast<float>(position - intPos);

    if (intPos >= maxSample) {
        return sampleBuffer->getSample(channel, maxSample);
    }

    const float sample1 = sampleBuffer->getSample(channel, intPos);
    const float sample2 = sampleBuffer->getSample(channel, intPos + 1);

    return sample1 + fraction * (sample2 - sample1);
}

float Sampler::getSampleAt(int channel, long position) const {
    juce::ScopedLock lock(bufferLock);

    if (!sampleBuffer || !hasSample()) {
        return 0.0f;
    }

    const int numChannels = sampleBuffer->getNumChannels();
    const int numSamples = sampleBuffer->getNumSamples();

    if (numChannels == 0 || numSamples == 0) {
        return 0.0f;
    }

    // Clamp channel to valid range
    const int safeChannel = juce::jlimit(0, numChannels - 1, channel);

    // Clamp position to valid range
    if (position < 0) return 0.0f;
    if (position >= static_cast<long>(numSamples)) return 0.0f;

    const int safePosition = static_cast<int>(juce::jmin(position,
        static_cast<long>(numSamples - 1)));

    return sampleBuffer->getSample(safeChannel, safePosition);
}

float Sampler::getOutput(int channel) {
    if (!isPlaying()) {
        return 0.0f;
    }

    const float sample = getCurrentSample(channel);
    const float envelope = samplerEnvelope->process();
    const float vol = volume.load();

    return sample * envelope * vol;
}

void Sampler::loadSample(const juce::File& file) {
    if (!file.exists()) return;

    SmartSample sample;
    sample.loadFromFile(file, sampleRate);

    if (sample.getLengthInSamples() <= 0) return;

    {
        juce::ScopedLock lock(bufferLock);

        // Create new buffer with sample data
        sampleBuffer = std::make_unique<juce::AudioSampleBuffer>(sample.getBuffer());

        // Handle mono to stereo conversion
        if (sampleBuffer->getNumChannels() == 1) {
            auto stereoBuffer = std::make_unique<juce::AudioSampleBuffer>(2, sampleBuffer->getNumSamples());
            stereoBuffer->copyFrom(0, 0, *sampleBuffer, 0, 0, sampleBuffer->getNumSamples());
            stereoBuffer->copyFrom(1, 0, *sampleBuffer, 0, 0, sampleBuffer->getNumSamples());
            sampleBuffer = std::move(stereoBuffer);
        }

        // Update sample parameters
        sampleLength = sample.getLengthInSamples();
        endPosition = sampleLength;
        startPosition = 0;
        currentSample = 0;
    }

    // Reset interpolators for new sample
    initializeInterpolators();

    loaded = true;
    setDirty(true);
}

void Sampler::loadSample(std::unique_ptr<juce::InputStream> input) {
    if (!input) return;

    auto reader = std::unique_ptr<juce::AudioFormatReader>(
        formatManager->createReaderFor(std::move(input))
    );

    if (!reader) return;

    const auto numSamples = static_cast<int>(reader->lengthInSamples);
    const auto numChannels = juce::jmin(2, static_cast<int>(reader->numChannels));

    {
        juce::ScopedLock lock(bufferLock);

        sampleBuffer = std::make_unique<juce::AudioSampleBuffer>(numChannels, numSamples);
        reader->read(sampleBuffer.get(), 0, numSamples, 0, true, numChannels > 1);

        // Handle mono to stereo conversion
        if (numChannels == 1) {
            auto stereoBuffer = std::make_unique<juce::AudioSampleBuffer>(2, numSamples);
            stereoBuffer->copyFrom(0, 0, *sampleBuffer, 0, 0, numSamples);
            stereoBuffer->copyFrom(1, 0, *sampleBuffer, 0, 0, numSamples);
            sampleBuffer = std::move(stereoBuffer);
        }

        sampleLength = numSamples;
        endPosition = sampleLength;
        startPosition = 0;
        currentSample = 0;
    }

    // Reset interpolators for new sample
    initializeInterpolators();

    loaded = true;
    setDirty(true);
}

void Sampler::setPitch(float newPitch) noexcept {
    // Clamp pitch to reasonable range
    const float clampedPitch = juce::jlimit(0.25f, 4.0f, newPitch);
    pitch = clampedPitch;
    setDirty(true);
}
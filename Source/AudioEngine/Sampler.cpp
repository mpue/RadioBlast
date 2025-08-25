/*
  ==============================================================================

    Sampler.cpp
    Created: 17 Apr 2018 1:12:15pm
    Author:  Matthias Pueski

  ==============================================================================
*/

#include "Sampler.h"
#include "../AudioManager.h"
#include "SmartSample.h"

using juce::CatmullRomInterpolator;
using juce::AudioSampleBuffer;
using juce::File;
using juce::AudioFormatReader;
using juce::ScopedPointer;
using juce::AudioFormatReaderSource;
using juce::InputStream;

Sampler::Sampler(float sampleRate, int bufferSize) {
    this->sampleRate = sampleRate;
    this->bufferSize = bufferSize;
    this->manager = AudioManager::getInstance()->getFormatManager();
    this->interpolatorLeft = new CatmullRomInterpolator();
    this->interpolatorRight = new CatmullRomInterpolator();
    this->sampleBuffer = new AudioSampleBuffer(2,16384*1024);
    this->samplerEnvelope = new SynthLab::ADSR();
    this->samplerEnvelope->setAttackRate(0.1 * sampleRate);
    this->samplerEnvelope->setReleaseRate(0.3 * sampleRate);
}

Sampler::~Sampler() {
	stop();
    if (tempBufferLeft != nullptr)
        delete this->tempBufferLeft;
    if (tempBufferRight != nullptr)
        delete this->tempBufferRight;

	samplerEnvelope->reset();
    delete samplerEnvelope;
	samplerEnvelope = NULL;
	delete interpolatorLeft;
    delete interpolatorRight;
    
}

void Sampler::nextSample() {
    if (sampleLength > 0) {
        if (isLoop()) {
            
            if (currentSample < sampleLength - 1&& currentSample < endPosition) {
                currentSample++;
            }
            else {
                currentSample = startPosition;
            }
            // currentSample = (currentSample + 1) % sampleLength;
        }
        else {
            if (currentSample < sampleLength - 1 && currentSample < endPosition) {
                currentSample++;
            }
            else {
                playing = false;
                currentSample = 0;
            }
        }
    }

}

void Sampler::play() {
    samplerEnvelope->gate(127);
    playing = true;
}

void Sampler::stop() {
    samplerEnvelope->gate(0);
    playing = false;
}

float Sampler::getCurrentSample(int channel){
    
    if (sampleLength > 0 && !isDone()) {

        if (pitch != 1) {
            if (channel == 0) {
                return tempBufferLeft[currentSample] * volume;
            }
            else {
                return tempBufferRight[currentSample] * volume;
            }
        }
        else {
            if (currentSample < sampleBuffer->getNumSamples())
                return sampleBuffer->getSample(channel, static_cast<int>(currentSample)) * volume;
            else
                return 0;
        }

    }
    
    return 0;
}

float Sampler::getSampleAt(int channel, long pos){
    return sampleBuffer->getSample(channel, static_cast<int>(pos)) * volume;
}

void Sampler::loadSample(File file) {
    SmartSample sample;
    sample.loadFromFile(file, sampleRate);

    if (sampleBuffer != nullptr)
        delete sampleBuffer;

    sampleBuffer = new AudioSampleBuffer(sample.getBuffer());
    sampleLength = sample.getLengthInSamples();
    endPosition = sampleLength;
    startPosition = 0;
    currentSample = 0;

    // Mono-Fallback: wenn nur 1 Kanal vorhanden ist, dupliziere ihn für Rechts
    if (sampleBuffer->getNumChannels() == 1) {
        AudioSampleBuffer* stereoBuffer = new AudioSampleBuffer(2, sampleBuffer->getNumSamples());
        stereoBuffer->copyFrom(0, 0, *sampleBuffer, 0, 0, sampleBuffer->getNumSamples());
        stereoBuffer->copyFrom(1, 0, *sampleBuffer, 0, 0, sampleBuffer->getNumSamples());
        delete sampleBuffer;
        sampleBuffer = stereoBuffer;
    }

    loaded = true;
}

void Sampler::loadSample(juce::InputStream* input) {
    /*
    AudioFormatReader* reader = manager->createReaderFor(std::make_unique<juce::InputStream>(input));
    ScopedPointer<AudioFormatReaderSource> afr = new AudioFormatReaderSource(reader, true);
    sampleBuffer = new AudioSampleBuffer(2, static_cast<int>(reader->lengthInSamples));
    this->tempBufferLeft = new float[reader->lengthInSamples * 2];
    this->tempBufferRight = new float[reader->lengthInSamples * 2];
    
    reader->read(sampleBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    sampleLength = reader->lengthInSamples;
    endPosition = sampleLength;
    startPosition = 0;
    loaded = true;
    */
}


void Sampler::setStartPosition(long start) {
    this->startPosition = start;
    this->currentSample = startPosition;
}

long Sampler::getStartPosition() {
    return startPosition;
}

void Sampler::setEndPosition(long end) {
    this->endPosition = end;
}

long Sampler::getEndPosition() {
    return this->endPosition;
}

void Sampler::setSampleLength(long length) {
    this->sampleLength = length;
}

long Sampler::getSampleLength(){
    return this->sampleLength;
}

void Sampler::setLoop(bool loop) {
    this->loop = loop;
}

float Sampler::getPitch() {
    return pitch;
}

void Sampler::setPitch(float pitch) {
    
    this->pitch = pitch;
    
    if (pitch < 0.25) {
        pitch = 0.25;
    }
    if (pitch > 2) {
        pitch = 2;
    }
    
    if (getSampleLength() > 0) {
        if (this->tempBufferLeft == nullptr)
            this->tempBufferLeft = new float[getSampleBuffer()->getNumSamples() * 2];
        if (this->tempBufferRight == nullptr)
            this->tempBufferRight = new float[getSampleBuffer()->getNumSamples() * 2];
        interpolatorLeft->process(pitch, getSampleBuffer()->getReadPointer(0),tempBufferLeft , getSampleBuffer()->getNumSamples());
        interpolatorLeft->process(pitch, getSampleBuffer()->getReadPointer(1),tempBufferRight, getSampleBuffer()->getNumSamples());
    }

}

bool Sampler::isLoop() {
    return this->loop;
}

void Sampler::setVolume(float volume) {
    this->volume = volume;
}

AudioSampleBuffer* Sampler::getSampleBuffer() {
    return sampleBuffer;
}
bool Sampler::hasSample() {
    return loaded;
}

#include "StutterEffectComponent.h"

StutterEffectComponent::StutterEffectComponent()
{
    // Setup buttons
    addAndMakeVisible(stutterButton1);
    stutterButton1.setButtonText("1/16 Classic");
    stutterButton1.addListener(this);
    stutterButton1.setColour(juce::TextButton::buttonColourId, juce::Colours::darkslateblue);

    addAndMakeVisible(stutterButton2);
    stutterButton2.setButtonText("1/8 Gate");
    stutterButton2.addListener(this);
    stutterButton2.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);

    addAndMakeVisible(stutterButton3);
    stutterButton3.setButtonText("1/4 Reverse");
    stutterButton3.addListener(this);
    stutterButton3.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);

    addAndMakeVisible(stutterButton4);
    stutterButton4.setButtonText("1/2 Pitched");
    stutterButton4.addListener(this);
    stutterButton4.setColour(juce::TextButton::buttonColourId, juce::Colours::darkorange);

    // Setup title
    addAndMakeVisible(titleLabel);
    titleLabel.setText("STUTTER EFFECTS", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    setSize(400, 150);
}

StutterEffectComponent::~StutterEffectComponent()
{
    stopTimer();
}

void StutterEffectComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 2.0f);

    // Highlight active stutter
    if (stutterState.isActive)
    {
        g.setColour(juce::Colours::yellow.withAlpha(0.3f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
    }
}

void StutterEffectComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    // Title area
    titleLabel.setBounds(area.removeFromTop(30));

    area.removeFromTop(10); // spacing

    // Button area
    auto buttonArea = area;
    int buttonWidth = buttonArea.getWidth() / 4 - 5;

    stutterButton1.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    stutterButton2.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    stutterButton3.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    stutterButton4.setBounds(buttonArea.removeFromLeft(buttonWidth));
}

void StutterEffectComponent::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBufferSize = samplesPerBlock;

    // Prepare stutter buffer (1 second should be enough for most stutters)
    stutterBuffer.setSize(2, static_cast<int>(sampleRate));
    stutterBuffer.clear();
}

void StutterEffectComponent::processAudioBuffer(juce::AudioBuffer<float>& buffer)
{
    if (!stutterState.isActive)
        return;

    switch (stutterState.type)
    {
    case CLASSIC_STUTTER:
        processClassicStutter(buffer);
        break;
    case GATE_STUTTER:
        processGateStutter(buffer);
        break;
    case REVERSE_STUTTER:
        processReverseStutter(buffer);
        break;
    case PITCHED_STUTTER:
        processPitchedStutter(buffer);
        break;
    }
}

void StutterEffectComponent::releaseResources()
{
    stutterBuffer.setSize(0, 0);
}

void StutterEffectComponent::buttonClicked(juce::Button* button)
{
    if (button == &stutterButton1)
    {
        if (stutterState.isActive && stutterState.type == CLASSIC_STUTTER)
            stopStutter();
        else
            startStutter(CLASSIC_STUTTER, 0);
    }
    else if (button == &stutterButton2)
    {
        if (stutterState.isActive && stutterState.type == GATE_STUTTER)
            stopStutter();
        else
            startStutter(GATE_STUTTER, 1);
    }
    else if (button == &stutterButton3)
    {
        if (stutterState.isActive && stutterState.type == REVERSE_STUTTER)
            stopStutter();
        else
            startStutter(REVERSE_STUTTER, 2);
    }
    else if (button == &stutterButton4)
    {
        if (stutterState.isActive && stutterState.type == PITCHED_STUTTER)
            stopStutter();
        else
            startStutter(PITCHED_STUTTER, 3);
    }

    repaint();
}

void StutterEffectComponent::timerCallback()
{
    // Auto-stop stutter after certain time (optional)
}

void StutterEffectComponent::startStutter(StutterType type, int subdivision)
{
    stutterState.isActive = true;
    stutterState.type = type;
    stutterState.currentPosition = 0;
    stutterState.fadeCounter = 0.0f;
    stutterState.bufferCaptured = false;

    // Calculate subdivision length in samples (assuming 120 BPM for now)
    double beatsPerSecond = 120.0 / 60.0;
    double subdivisionTime = (1.0 / beatsPerSecond) / stutterSubdivisions[subdivision];
    stutterState.subdivisionSamples = static_cast<int>(currentSampleRate * subdivisionTime);

    stutterBuffer.clear();
}

void StutterEffectComponent::stopStutter()
{
    stutterState.isActive = false;
    stutterState.currentPosition = 0;
    stutterState.bufferCaptured = false;
}

void StutterEffectComponent::processClassicStutter(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // Capture the first subdivision into our stutter buffer
    if (!stutterState.bufferCaptured && stutterState.currentPosition < stutterState.subdivisionSamples)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            int capturePos = stutterState.currentPosition + sample;
            if (capturePos < stutterState.subdivisionSamples && capturePos < stutterBuffer.getNumSamples())
            {
                for (int ch = 0; ch < std::min(numChannels, stutterBuffer.getNumChannels()); ++ch)
                {
                    stutterBuffer.setSample(ch, capturePos, buffer.getSample(ch, sample));
                }
            }
        }

        stutterState.currentPosition += numSamples;

        // Mark buffer as captured when we have enough samples
        if (stutterState.currentPosition >= stutterState.subdivisionSamples)
        {
            stutterState.bufferCaptured = true;
            stutterState.currentPosition = 0;
        }
        return; // Don't process during capture phase
    }

    // Play back the captured audio in a loop
    for (int sample = 0; sample < numSamples; ++sample)
    {
        int playbackPosition = stutterState.currentPosition % stutterState.subdivisionSamples;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (playbackPosition < stutterBuffer.getNumSamples() && ch < stutterBuffer.getNumChannels())
            {
                buffer.setSample(ch, sample, stutterBuffer.getSample(ch, playbackPosition));
            }
        }

        stutterState.currentPosition++;
    }
}

void StutterEffectComponent::processGateStutter(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        int gatePosition = stutterState.currentPosition % stutterState.subdivisionSamples;
        bool gateOpen = gatePosition < (stutterState.subdivisionSamples / 2);

        if (!gateOpen)
        {
            // Mute the audio
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                buffer.setSample(ch, sample, 0.0f);
            }
        }

        stutterState.currentPosition++;
    }
}

void StutterEffectComponent::processReverseStutter(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // Capture phase
    if (!stutterState.bufferCaptured && stutterState.currentPosition < stutterState.subdivisionSamples)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            int capturePos = stutterState.currentPosition + sample;
            if (capturePos < stutterState.subdivisionSamples && capturePos < stutterBuffer.getNumSamples())
            {
                for (int ch = 0; ch < std::min(numChannels, stutterBuffer.getNumChannels()); ++ch)
                {
                    stutterBuffer.setSample(ch, capturePos, buffer.getSample(ch, sample));
                }
            }
        }

        stutterState.currentPosition += numSamples;

        if (stutterState.currentPosition >= stutterState.subdivisionSamples)
        {
            stutterState.bufferCaptured = true;
            stutterState.currentPosition = 0;
        }
        return;
    }

    // Play back reversed
    for (int sample = 0; sample < numSamples; ++sample)
    {
        int playbackPosition = stutterState.currentPosition % stutterState.subdivisionSamples;
        int reversePosition = stutterState.subdivisionSamples - 1 - playbackPosition;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (reversePosition >= 0 && reversePosition < stutterBuffer.getNumSamples() &&
                ch < stutterBuffer.getNumChannels())
            {
                buffer.setSample(ch, sample, stutterBuffer.getSample(ch, reversePosition));
            }
        }

        stutterState.currentPosition++;
    }
}

void StutterEffectComponent::processPitchedStutter(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // Capture phase
    if (!stutterState.bufferCaptured && stutterState.currentPosition < stutterState.subdivisionSamples)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            int capturePos = stutterState.currentPosition + sample;
            if (capturePos < stutterState.subdivisionSamples && capturePos < stutterBuffer.getNumSamples())
            {
                for (int ch = 0; ch < std::min(numChannels, stutterBuffer.getNumChannels()); ++ch)
                {
                    stutterBuffer.setSample(ch, capturePos, buffer.getSample(ch, sample));
                }
            }
        }

        stutterState.currentPosition += numSamples;

        if (stutterState.currentPosition >= stutterState.subdivisionSamples)
        {
            stutterState.bufferCaptured = true;
            stutterState.currentPosition = 0;
        }
        return;
    }

    // Play back with pitch modulation
    for (int sample = 0; sample < numSamples; ++sample)
    {
        int playbackPosition = stutterState.currentPosition % stutterState.subdivisionSamples;

        // Simple pitch modulation by changing playback speed
        float pitchFactor = 1.0f + 0.3f * std::sin(2.0f * juce::MathConstants<float>::pi *
            playbackPosition / (float)stutterState.subdivisionSamples);

        int modulatedPosition = static_cast<int>(playbackPosition * pitchFactor);
        modulatedPosition = modulatedPosition % stutterState.subdivisionSamples;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (modulatedPosition < stutterBuffer.getNumSamples() && ch < stutterBuffer.getNumChannels())
            {
                buffer.setSample(ch, sample, stutterBuffer.getSample(ch, modulatedPosition));
            }
        }

        stutterState.currentPosition++;
    }
}
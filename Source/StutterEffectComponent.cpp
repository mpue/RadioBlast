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
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Setup parameter controls
    // Length Slider
    addAndMakeVisible(lengthSlider);
    lengthSlider.setRange(0.1, 2.0, 0.1);
    lengthSlider.setValue(0.5);
    lengthSlider.setSliderStyle(juce::Slider::LinearVertical);
    lengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

    addAndMakeVisible(lengthLabel);
    lengthLabel.setText("Length", juce::dontSendNotification);
    lengthLabel.setFont(juce::Font(12.0f));
    lengthLabel.setJustificationType(juce::Justification::centred);
    lengthLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Intensity Slider
    addAndMakeVisible(intensitySlider);
    intensitySlider.setRange(0.0, 1.0, 0.01);
    intensitySlider.setValue(0.8);
    intensitySlider.setSliderStyle(juce::Slider::LinearVertical);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

    addAndMakeVisible(intensityLabel);
    intensityLabel.setText("Intensity", juce::dontSendNotification);
    intensityLabel.setFont(juce::Font(12.0f));
    intensityLabel.setJustificationType(juce::Justification::centred);
    intensityLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Feedback Slider
    addAndMakeVisible(feedbackSlider);
    feedbackSlider.setRange(0.0, 0.9, 0.01);
    feedbackSlider.setValue(0.3);
    feedbackSlider.setSliderStyle(juce::Slider::LinearVertical);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setFont(juce::Font(12.0f));
    feedbackLabel.setJustificationType(juce::Justification::centred);
    feedbackLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Mix Slider
    addAndMakeVisible(mixSlider);
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(1.0);
    mixSlider.setSliderStyle(juce::Slider::LinearVertical);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

    addAndMakeVisible(mixLabel);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setFont(juce::Font(12.0f));
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    setSize(500, 220);
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
    titleLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(5);

    // Button area
    auto buttonArea = area.removeFromTop(40);
    int buttonWidth = buttonArea.getWidth() / 4 - 5;

    stutterButton1.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    stutterButton2.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    stutterButton3.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    stutterButton4.setBounds(buttonArea.removeFromLeft(buttonWidth));

    area.removeFromTop(10);

    // Parameter controls area
    auto paramArea = area;
    int paramWidth = paramArea.getWidth() / 4 - 5;

    // Length controls
    auto lengthArea = paramArea.removeFromLeft(paramWidth);
    lengthLabel.setBounds(lengthArea.removeFromTop(20));
    lengthSlider.setBounds(lengthArea);

    paramArea.removeFromLeft(5);

    // Intensity controls
    auto intensityArea = paramArea.removeFromLeft(paramWidth);
    intensityLabel.setBounds(intensityArea.removeFromTop(20));
    intensitySlider.setBounds(intensityArea);

    paramArea.removeFromLeft(5);

    // Feedback controls
    auto feedbackArea = paramArea.removeFromLeft(paramWidth);
    feedbackLabel.setBounds(feedbackArea.removeFromTop(20));
    feedbackSlider.setBounds(feedbackArea);

    paramArea.removeFromLeft(5);

    // Mix controls
    auto mixArea = paramArea.removeFromLeft(paramWidth);
    mixLabel.setBounds(mixArea.removeFromTop(20));
    mixSlider.setBounds(mixArea);
}

void StutterEffectComponent::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBufferSize = samplesPerBlock;

    // Prepare stutter buffer (2 seconds should be enough for longest stutters)
    stutterBuffer.setSize(2, static_cast<int>(sampleRate * 2.0));
    stutterBuffer.clear();

    // Prepare dry buffer for mixing
    dryBuffer.setSize(2, samplesPerBlock);
    dryBuffer.clear();
}

void StutterEffectComponent::processAudioBuffer(juce::AudioBuffer<float>& buffer)
{
    if (!stutterState.isActive)
        return;

    // Store dry signal for mixing
    dryBuffer.makeCopyOf(buffer);

    // Update parameters from sliders
    updateParameters();

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

    // Apply dry/wet mixing
    applyDryWetMix(buffer, dryBuffer);
}

void StutterEffectComponent::releaseResources()
{
    stutterBuffer.setSize(0, 0);
    dryBuffer.setSize(0, 0);
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

    // Calculate subdivision length in samples with length parameter
    double beatsPerSecond = 120.0 / 60.0;
    double subdivisionTime = (1.0 / beatsPerSecond) / stutterSubdivisions[subdivision];
    subdivisionTime *= stutterState.length; // Apply length multiplier
    stutterState.subdivisionSamples = static_cast<int>(currentSampleRate * subdivisionTime);

    // Ensure we don't exceed buffer size
    stutterState.subdivisionSamples = std::min(stutterState.subdivisionSamples,
        stutterBuffer.getNumSamples() - 1);

    stutterBuffer.clear();
}

void StutterEffectComponent::updateParameters()
{
    stutterState.length = static_cast<float>(lengthSlider.getValue());
    stutterState.intensity = static_cast<float>(intensitySlider.getValue());
    stutterState.feedback = static_cast<float>(feedbackSlider.getValue());
    stutterState.mix = static_cast<float>(mixSlider.getValue());
}

void StutterEffectComponent::applyDryWetMix(juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer)
{
    float wetLevel = stutterState.mix;
    float dryLevel = 1.0f - wetLevel;

    int numChannels = std::min(wetBuffer.getNumChannels(), dryBuffer.getNumChannels());
    int numSamples = std::min(wetBuffer.getNumSamples(), dryBuffer.getNumSamples());

    for (int ch = 0; ch < numChannels; ++ch)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float wetSample = wetBuffer.getSample(ch, sample) * wetLevel;
            float drySample = dryBuffer.getSample(ch, sample) * dryLevel;
            wetBuffer.setSample(ch, sample, wetSample + drySample);
        }
    }
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
                    float inputSample = buffer.getSample(ch, sample);
                    float feedbackSample = stutterBuffer.getSample(ch, capturePos) * stutterState.feedback;
                    stutterBuffer.setSample(ch, capturePos, inputSample + feedbackSample);
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

    // Play back the captured audio in a loop with intensity control
    for (int sample = 0; sample < numSamples; ++sample)
    {
        int playbackPosition = stutterState.currentPosition % stutterState.subdivisionSamples;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (playbackPosition < stutterBuffer.getNumSamples() && ch < stutterBuffer.getNumChannels())
            {
                float stutterSample = stutterBuffer.getSample(ch, playbackPosition);
                buffer.setSample(ch, sample, stutterSample * stutterState.intensity);
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
        float gatePhase = static_cast<float>(gatePosition) / stutterState.subdivisionSamples;

        // Create gate pattern based on intensity
        bool gateOpen = gatePhase < stutterState.intensity;

        if (!gateOpen)
        {
            // Apply smooth fade instead of hard cut for less harsh sound
            float fade = 1.0f - stutterState.intensity;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                buffer.setSample(ch, sample, buffer.getSample(ch, sample) * fade);
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

    // Play back with pitch modulation based on intensity parameter
    for (int sample = 0; sample < numSamples; ++sample)
    {
        int playbackPosition = stutterState.currentPosition % stutterState.subdivisionSamples;

        // Pitch modulation depth controlled by intensity
        float pitchDepth = stutterState.intensity * 0.5f; // Max 50% pitch change
        float pitchFactor = 1.0f + pitchDepth * std::sin(2.0f * juce::MathConstants<float>::pi *
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
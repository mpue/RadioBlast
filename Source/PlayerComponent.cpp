#include "PlayerComponent.h"



PlayerComponent::PlayerComponent()
{
    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);
    deviceManager.addAudioCallback(&audioSourcePlayer);

    playButton.addListener(this);
    stopButton.addListener(this);

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
}

PlayerComponent::~PlayerComponent()
{
    audioSourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
}

void PlayerComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Drag an audio file here", getLocalBounds(), juce::Justification::centredTop);
}

void PlayerComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    playButton.setBounds(area.removeFromTop(30));
    stopButton.setBounds(area.removeFromTop(30).translated(0, 10));
}

void PlayerComponent::buttonClicked(juce::Button* button)
{
    if (button == &playButton) transportSource.start();
    else if (button == &stopButton) transportSource.stop();
}

bool PlayerComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    return true;
}

void PlayerComponent::filesDropped(const juce::StringArray& files, int, int)
{
    loadFile(juce::File(files[0]));
}

void PlayerComponent::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        audioSourcePlayer.setSource(&transportSource);
    }
}

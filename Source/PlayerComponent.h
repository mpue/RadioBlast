#pragma once
#include <JuceHeader.h>

class PlayerComponent : public juce::Component,
                        public juce::FileDragAndDropTarget,
                        public juce::Button::Listener
{
public:
    PlayerComponent();
    ~PlayerComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioDeviceManager deviceManager;

    void loadFile(const juce::File& file);
};

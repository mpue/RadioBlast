/*
  ==============================================================================

    SamplePlayer.h
    Created: 26 Aug 2025 10:17:18pm
    Author:  mpue

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class SamplePlayer : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::Button::Listener,
    public juce::ComboBox::Listener
{
public:
    enum class PlayMode
    {
        OneShot,
        LoopForward,
        LoopBackward
    };

    struct SampleSlot
    {
        juce::AudioBuffer<float> buffer;
        juce::String fileName;
        int currentPosition = 0;
        bool isPlaying = false;
        PlayMode playMode = PlayMode::OneShot;
        float gain = 1.0f;

        void reset()
        {
            currentPosition = 0;
            isPlaying = false;
        }

        void stop()
        {
            isPlaying = false;
            currentPosition = 0;
        }
    };

    //==============================================================================
    SamplePlayer()
    {
        // Initialize 8 sample slots
        sampleSlots.resize(8);

        setupUI();
        formatManager.registerBasicFormats();
    }

    ~SamplePlayer() override = default;

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.fillAll(Colour(0xff333333));

        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("Sample Player", getLocalBounds().removeFromTop(30),
            juce::Justification::centred);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(35); // Title space

        const int slotHeight = 60;
        const int margin = 5;

        for (int i = 0; i < 8; ++i)
        {
            auto slotArea = area.removeFromTop(slotHeight);
            slotArea.reduce(margin, margin);

            // Play button
            playButtons[i]->setBounds(slotArea.removeFromLeft(80));
            slotArea.removeFromLeft(5);

            // Mode combo box
            modeComboBoxes[i]->setBounds(slotArea.removeFromLeft(100));
            slotArea.removeFromLeft(5);

            // File name label
            fileNameLabels[i]->setBounds(slotArea);
        }
    }

    //==============================================================================
    // FileDragAndDropTarget implementation
    bool isInterestedInFileDrag(const juce::StringArray& files) override
    {
        for (const auto& file : files)
        {
            if (file.endsWith(".wav") || file.endsWith(".aiff") ||
                file.endsWith(".mp3") || file.endsWith(".flac"))
                return true;
        }
        return false;
    }

    void fileDragEnter(const juce::StringArray&, int, int) override {}
    void fileDragMove(const juce::StringArray&, int, int) override {}
    void fileDragExit(const juce::StringArray&) override {}

    void filesDropped(const juce::StringArray& files, int x, int y) override
    {
        // Find which slot the file was dropped on
        int slotIndex = findSlotAtPosition(y);

        if (slotIndex >= 0 && slotIndex < 8 && !files.isEmpty())
        {
            loadSampleIntoSlot(files[0], slotIndex);
        }
    }

    //==============================================================================
    // Button::Listener implementation
    void buttonClicked(juce::Button* button) override
    {
        for (int i = 0; i < 8; ++i)
        {
            if (button == playButtons[i].get())
            {
                toggleSamplePlayback(i);
                break;
            }
        }
    }

    //==============================================================================
    // ComboBox::Listener implementation
    void comboBoxChanged(juce::ComboBox* comboBox) override
    {
        for (int i = 0; i < 8; ++i)
        {
            if (comboBox == modeComboBoxes[i].get())
            {
                int selectedId = comboBox->getSelectedId();
                sampleSlots[i].playMode = static_cast<PlayMode>(selectedId - 1);
                break;
            }
        }
    }

    //==============================================================================
    // Hauptmethode für Sample-Processing - wird in Ihrem Audio-Loop aufgerufen
    void generateSampleOutput(std::vector<float>& leftOut, std::vector<float>& rightOut, int numSamples, float gain)
    {
        // Initialize output vectors with zeros
        std::fill(leftOut.begin(), leftOut.end(), 0.0f);
        std::fill(rightOut.begin(), rightOut.end(), 0.0f);

        if (leftOut.size() < numSamples) leftOut.resize(numSamples, 0.0f);
        if (rightOut.size() < numSamples) rightOut.resize(numSamples, 0.0f);

        for (auto& slot : sampleSlots)
        {
            if (!slot.isPlaying || slot.buffer.getNumSamples() == 0)
                continue;

            processSampleSlot(slot, leftOut, rightOut, numSamples, gain);
        }
    }

    //==============================================================================
    void stopAllSamples()
    {
        for (auto& slot : sampleSlots)
        {
            slot.stop();
        }
        updateButtonStates();
    }

    void setSampleGain(int slotIndex, float gain)
    {
        if (slotIndex >= 0 && slotIndex < sampleSlots.size())
        {
            sampleSlots[slotIndex].gain = juce::jlimit(0.0f, 2.0f, gain);
        }
    }

    // Trigger specific sample
    void triggerSample(int slotIndex)
    {
        if (slotIndex >= 0 && slotIndex < sampleSlots.size())
        {
            auto& slot = sampleSlots[slotIndex];
            if (slot.buffer.getNumSamples() > 0)
            {
                slot.isPlaying = true;
                slot.currentPosition = 0;
                updateButtonStates();
            }
        }
    }

    // Check if any sample is playing
    bool isAnySamplePlaying() const
    {
        for (const auto& slot : sampleSlots)
        {
            if (slot.isPlaying) return true;
        }
        return false;
    }

private:
    //==============================================================================
    std::vector<SampleSlot> sampleSlots;
    juce::AudioFormatManager formatManager;

    // UI Components
    std::array<std::unique_ptr<juce::TextButton>, 8> playButtons;
    std::array<std::unique_ptr<juce::ComboBox>, 8> modeComboBoxes;
    std::array<std::unique_ptr<juce::Label>, 8> fileNameLabels;

    //==============================================================================
    void setupUI()
    {
        for (int i = 0; i < 8; ++i)
        {
            // Play buttons
            playButtons[i] = std::make_unique<juce::TextButton>("Play " + juce::String(i + 1));
            playButtons[i]->addListener(this);
            addAndMakeVisible(*playButtons[i]);

            // Mode combo boxes
            modeComboBoxes[i] = std::make_unique<juce::ComboBox>();
            modeComboBoxes[i]->addItem("One Shot", 1);
            modeComboBoxes[i]->addItem("Loop Forward", 2);
            modeComboBoxes[i]->addItem("Loop Backward", 3);
            modeComboBoxes[i]->setSelectedId(1);
            modeComboBoxes[i]->addListener(this);
            addAndMakeVisible(*modeComboBoxes[i]);

            // File name labels
            fileNameLabels[i] = std::make_unique<juce::Label>();
            fileNameLabels[i]->setText("Drop audio file here...", juce::dontSendNotification);
            fileNameLabels[i]->setColour(juce::Label::backgroundColourId, juce::Colours::black);
            fileNameLabels[i]->setColour(juce::Label::textColourId, juce::Colours::white);
            fileNameLabels[i]->setJustificationType(juce::Justification::centredLeft);
            addAndMakeVisible(*fileNameLabels[i]);
        }
    }

    //==============================================================================
    int findSlotAtPosition(int y)
    {
        auto area = getLocalBounds();
        area.removeFromTop(35);

        const int slotHeight = 60 + 10; // including margin
        return juce::jmin(7, y / slotHeight);
    }

    //==============================================================================
    void loadSampleIntoSlot(const juce::String& filePath, int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= sampleSlots.size())
            return;

        juce::File audioFile(filePath);

        if (!audioFile.existsAsFile())
            return;

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(audioFile));

        if (reader == nullptr)
            return;

        auto& slot = sampleSlots[slotIndex];

        // Stop current playback
        slot.stop();

        // Load the audio data
        slot.buffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&slot.buffer, 0, (int)reader->lengthInSamples, 0, true, true);

        slot.fileName = audioFile.getFileNameWithoutExtension();

        // Update UI
        fileNameLabels[slotIndex]->setText(slot.fileName, juce::dontSendNotification);
        updateButtonStates();
    }

    //==============================================================================
    void toggleSamplePlayback(int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= sampleSlots.size())
            return;

        auto& slot = sampleSlots[slotIndex];

        if (slot.buffer.getNumSamples() == 0)
            return; // No sample loaded

        if (slot.isPlaying)
        {
            slot.stop();
        }
        else
        {
            slot.isPlaying = true;
            slot.currentPosition = 0;
        }

        updateButtonStates();
    }

    //==============================================================================
    void processSampleSlot(SampleSlot& slot, std::vector<float>& leftOut, std::vector<float>& rightOut, int numSamples, float masterGain)
    {
        const int sampleLength = slot.buffer.getNumSamples();
        const int numChannels = slot.buffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            if (!slot.isPlaying)
                break;

            // Handle different play modes
            int readPosition = slot.currentPosition;

            if (slot.playMode == PlayMode::LoopBackward)
            {
                readPosition = sampleLength - 1 - slot.currentPosition;
            }

            // Bounds check
            if (readPosition < 0 || readPosition >= sampleLength)
            {
                if (slot.playMode == PlayMode::OneShot)
                {
                    slot.stop();
                    break;
                }
                else // Loop modes
                {
                    slot.currentPosition = 0;
                    readPosition = slot.playMode == PlayMode::LoopBackward ?
                        sampleLength - 1 : 0;
                }
            }

            // Get sample values
            float leftSample = 0.0f;
            float rightSample = 0.0f;

            if (numChannels >= 1)
            {
                leftSample = slot.buffer.getSample(0, readPosition);
            }

            if (numChannels >= 2)
            {
                rightSample = slot.buffer.getSample(1, readPosition);
            }
            else
            {
                rightSample = leftSample; // Mono zu Stereo
            }

            // Apply gains and mix to output
            float finalGain = slot.gain * masterGain;
            leftOut[sample] += leftSample * finalGain;
            rightOut[sample] += rightSample * finalGain;

            // Advance position
            if (slot.playMode == PlayMode::LoopBackward)
            {
                slot.currentPosition++;
                if (slot.currentPosition >= sampleLength)
                    slot.currentPosition = 0;
            }
            else
            {
                slot.currentPosition++;
                if (slot.currentPosition >= sampleLength && slot.playMode != PlayMode::OneShot)
                    slot.currentPosition = 0;
            }
        }
    }

    //==============================================================================
    void updateButtonStates()
    {
        for (int i = 0; i < 8; ++i)
        {
            const auto& slot = sampleSlots[i];

            if (slot.isPlaying)
            {
                playButtons[i]->setButtonText("Stop " + juce::String(i + 1));
                playButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            }
            else
            {
                playButtons[i]->setButtonText("Play " + juce::String(i + 1));
                playButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplePlayer)
};
/*
  ==============================================================================
    MixerComponent.h
    Created: 23 Aug 2025 9:58:12pm
    Author:  mpue
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "BPMAnalyzer.h"
//==============================================================================
/*
*/
class MixerComponent : public juce::Component
{
public:
    MixerComponent()
    {
        setSize(400, 600); // Etwas breiter für besseres Layout

        // Volume-Slider für beide Kanäle
        leftSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
        rightSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
        leftSlider->setRange(0.0, 1.0, 0.01);
        rightSlider->setRange(0.0, 1.0, 0.01);
        leftSlider->setValue(1.0);
        rightSlider->setValue(1.0);

        addAndMakeVisible(leftSlider.get());
        addAndMakeVisible(rightSlider.get());

        // Channel Labels
        leftLabel = std::make_unique<juce::Label>("", "DECK A");
        rightLabel = std::make_unique<juce::Label>("", "DECK B");
        leftLabel->setJustificationType(juce::Justification::centred);
        rightLabel->setJustificationType(juce::Justification::centred);
        leftLabel->setFont(14.0f);
        rightLabel->setFont(14.0f);
        addAndMakeVisible(leftLabel.get());
        addAndMakeVisible(rightLabel.get());

        // MIDI Learn Buttons
        leftLearnButton = std::make_unique<juce::TextButton>("MIDI Learn");
        rightLearnButton = std::make_unique<juce::TextButton>("MIDI Learn");
        addAndMakeVisible(leftLearnButton.get());
        addAndMakeVisible(rightLearnButton.get());

        // MIDI CC Status Labels
        leftCCLabel = std::make_unique<juce::Label>("", "CC: -");
        rightCCLabel = std::make_unique<juce::Label>("", "CC: -");
        leftCCLabel->setJustificationType(juce::Justification::centred);
        rightCCLabel->setJustificationType(juce::Justification::centred);
        leftCCLabel->setFont(10.0f);
        rightCCLabel->setFont(10.0f);
        addAndMakeVisible(leftCCLabel.get());
        addAndMakeVisible(rightCCLabel.get());

        // Output Routing Dropdowns
        leftOutputCombo = std::make_unique<juce::ComboBox>("LeftOutput");
        rightOutputCombo = std::make_unique<juce::ComboBox>("RightOutput");

        // Output Optionen hinzufügen
        leftOutputCombo->addItem("Master L+R", 1);
        leftOutputCombo->addItem("Left Only", 2);
        leftOutputCombo->addItem("Right Only", 3);
        leftOutputCombo->addItem("Cue (Headphones)", 4);
        leftOutputCombo->addItem("Muted", 5);

        rightOutputCombo->addItem("Master L+R", 1);
        rightOutputCombo->addItem("Left Only", 2);
        rightOutputCombo->addItem("Right Only", 3);
        rightOutputCombo->addItem("Cue (Headphones)", 4);
        rightOutputCombo->addItem("Muted", 5);

        // Standard: beide auf Master
        leftOutputCombo->setSelectedId(1);
        rightOutputCombo->setSelectedId(1);

        addAndMakeVisible(leftOutputCombo.get());
        addAndMakeVisible(rightOutputCombo.get());

        // Output Labels
        leftOutputLabel = std::make_unique<juce::Label>("", "Output:");
        rightOutputLabel = std::make_unique<juce::Label>("", "Output:");
        leftOutputLabel->setJustificationType(juce::Justification::left);
        rightOutputLabel->setJustificationType(juce::Justification::left);
        leftOutputLabel->setFont(11.0f);
        rightOutputLabel->setFont(11.0f);
        addAndMakeVisible(leftOutputLabel.get());
        addAndMakeVisible(rightOutputLabel.get());

        // Crossfader Section
        crossfader = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow);
        crossfader->setRange(-1.0, 1.0, 0.01);
        crossfader->setValue(0.0);
        addAndMakeVisible(crossfader.get());

        crossfaderLabel = std::make_unique<juce::Label>("", "CROSSFADER");
        crossfaderLabel->setJustificationType(juce::Justification::centred);
        crossfaderLabel->setFont(12.0f);
        addAndMakeVisible(crossfaderLabel.get());

        crossfaderLearnButton = std::make_unique<juce::TextButton>("MIDI Learn");
        addAndMakeVisible(crossfaderLearnButton.get());

        crossfaderCCLabel = std::make_unique<juce::Label>("", "CC: -");
        crossfaderCCLabel->setJustificationType(juce::Justification::centred);
        crossfaderCCLabel->setFont(10.0f);
        addAndMakeVisible(crossfaderCCLabel.get());

        // Button Callbacks
        leftLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::LeftVolume); };
        rightLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::RightVolume); };
        crossfaderLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::Crossfader); };

        // BPM Display und Controls
        leftBPMLabel = std::make_unique<juce::Label>("", "120.0 BPM");
        rightBPMLabel = std::make_unique<juce::Label>("", "120.0 BPM");
        leftBPMLabel->setJustificationType(juce::Justification::centred);
        rightBPMLabel->setJustificationType(juce::Justification::centred);
        leftBPMLabel->setFont(12.0f);
        rightBPMLabel->setFont(12.0f);
        leftBPMLabel->setColour(juce::Label::backgroundColourId, juce::Colours::black);
        rightBPMLabel->setColour(juce::Label::backgroundColourId, juce::Colours::black);
        leftBPMLabel->setColour(juce::Label::textColourId, juce::Colours::lime);
        rightBPMLabel->setColour(juce::Label::textColourId, juce::Colours::lime);
        addAndMakeVisible(leftBPMLabel.get());
        addAndMakeVisible(rightBPMLabel.get());

        // Pitch/Tempo Slider (±8% wie bei echten DJ Mixern)
        leftPitchSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
        rightPitchSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
        leftPitchSlider->setRange(-0.08, 0.08, 0.001); // ±8%
        rightPitchSlider->setRange(-0.08, 0.08, 0.001);
        leftPitchSlider->setValue(0.0); // Center
        rightPitchSlider->setValue(0.0);
        leftPitchSlider->setSkewFactor(1.0);
        rightPitchSlider->setSkewFactor(1.0);
        addAndMakeVisible(leftPitchSlider.get());
        addAndMakeVisible(rightPitchSlider.get());

        // Pitch Labels
        leftPitchLabel = std::make_unique<juce::Label>("", "PITCH");
        rightPitchLabel = std::make_unique<juce::Label>("", "PITCH");
        leftPitchLabel->setJustificationType(juce::Justification::centred);
        rightPitchLabel->setJustificationType(juce::Justification::centred);
        leftPitchLabel->setFont(10.0f);
        rightPitchLabel->setFont(10.0f);
        addAndMakeVisible(leftPitchLabel.get());
        addAndMakeVisible(rightPitchLabel.get());

        // Sync Buttons
        leftSyncButton = std::make_unique<juce::TextButton>("SYNC");
        rightSyncButton = std::make_unique<juce::TextButton>("SYNC");
        leftSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
        rightSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
        addAndMakeVisible(leftSyncButton.get());
        addAndMakeVisible(rightSyncButton.get());

        // Sync Button Callbacks
        leftSyncButton->onClick = [this]() { syncLeftToRight(); };
        rightSyncButton->onClick = [this]() { syncRightToLeft(); };

        // BPM Analyzer
        leftBPMAnalyzer = std::make_unique<BPMAnalyzer>();
        rightBPMAnalyzer = std::make_unique<BPMAnalyzer>();

        // Reset MIDI mappings
        resetMidiMappings();
    }

    ~MixerComponent() override
    {
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff222222));   // Dunkler Hintergrund
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1);   // Rahmen um die gesamte Komponente

        auto area = getLocalBounds().reduced(10);

        // Deck Bereiche visuell trennen
        int deckWidth = (area.getWidth() - 30) / 2;

        // Left Deck Background
        auto leftDeckRect = juce::Rectangle<int>(10, 10, deckWidth, 420);
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRoundedRectangle(leftDeckRect.toFloat(), 5.0f);
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(leftDeckRect.toFloat(), 5.0f, 1.0f);

        // Right Deck Background  
        auto rightDeckRect = juce::Rectangle<int>(deckWidth + 40, 10, deckWidth, 420);
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRoundedRectangle(rightDeckRect.toFloat(), 5.0f);
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(rightDeckRect.toFloat(), 5.0f, 1.0f);

        // Crossfader Section Background
        auto crossfaderRect = juce::Rectangle<int>(20, 450, getWidth() - 40, 100);
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRoundedRectangle(crossfaderRect.toFloat(), 5.0f);
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(crossfaderRect.toFloat(), 5.0f, 1.0f);

        // Crossfader Markierungen
        g.setColour(juce::Colours::lightgrey);
        g.setFont(10.0f);
        auto crossfaderBounds = crossfader->getBounds();
        g.drawText("A", crossfaderBounds.getX() - 15, crossfaderBounds.getY() + 5, 10, 15, juce::Justification::centred);
        g.drawText("⬌", crossfaderBounds.getCentreX() - 5, crossfaderBounds.getY() + 5, 10, 15, juce::Justification::centred);
        g.drawText("B", crossfaderBounds.getRight() + 5, crossfaderBounds.getY() + 5, 10, 15, juce::Justification::centred);

        // Learning indicator
        if (isLearning)
        {
            g.setColour(juce::Colours::red.withAlpha(0.3f));
            g.fillRect(getLocalBounds());
            g.setColour(juce::Colours::white);
            g.setFont(16.0f);
            g.drawText("MIDI Learning - Move a control on your MIDI device...",
                getLocalBounds(), juce::Justification::centred);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        if (area.isEmpty()) return; // Safety check

        area = area.reduced(10); // 10px Rand rundherum
        if (area.isEmpty()) return; // Safety check nach reduce

        // === DECK SECTIONS ===
        int deckWidth = juce::jmax(50, (area.getWidth() - 30) / 2); // Mindestbreite 50px

        // Left Deck Area
        auto leftArea = area.withWidth(deckWidth);
        auto rightArea = area.withX(area.getX() + deckWidth + 30).withWidth(deckWidth);

        // === LEFT DECK LAYOUT ===
        if (!leftArea.isEmpty())
        {
            auto leftDeckArea = leftArea;

            // Deck Label
            if (leftDeckArea.getHeight() >= 25)
            {
                auto leftLabelArea = leftDeckArea.removeFromTop(25);
                if (leftLabel)
                    leftLabel->setBounds(leftLabelArea);
            }

            if (leftDeckArea.getHeight() >= 5)
                leftDeckArea.removeFromTop(5); // Spacer

            // Volume Slider (Hauptteil)
            if (leftDeckArea.getHeight() >= 250)
            {
                auto leftSliderArea = leftDeckArea.removeFromTop(250);
                if (leftSlider)
                    leftSlider->setBounds(leftSliderArea.reduced(juce::jmin(20, leftSliderArea.getWidth() / 4), 0));
            }

            if (leftDeckArea.getHeight() >= 10)
                leftDeckArea.removeFromTop(10); // Spacer

            // MIDI Learn Section
            if (leftDeckArea.getHeight() >= 25)
            {
                auto leftMidiArea = leftDeckArea.removeFromTop(25);
				if (leftLearnButton)
                    leftLearnButton->setBounds(leftMidiArea);
            }

            if (leftDeckArea.getHeight() >= 5)
                leftDeckArea.removeFromTop(5); // Spacer

            // CC Label
            if (leftDeckArea.getHeight() >= 20)
            {
                auto leftCCArea = leftDeckArea.removeFromTop(20);
				if (leftCCLabel)
                    leftCCLabel->setBounds(leftCCArea);
            }

            if (leftDeckArea.getHeight() >= 10)
                leftDeckArea.removeFromTop(10); // Spacer

            // Output Section
            if (leftDeckArea.getHeight() >= 20)
            {
                auto leftOutputLabelArea = leftDeckArea.removeFromTop(20);
				if (leftOutputLabel)
                    leftOutputLabel->setBounds(leftOutputLabelArea);
            }

            if (leftDeckArea.getHeight() >= 5)
                leftDeckArea.removeFromTop(5); // Spacer

            if (leftDeckArea.getHeight() >= 25)
            {
                auto leftOutputComboArea = leftDeckArea.removeFromTop(25);
				if (leftOutputCombo)
                    leftOutputCombo->setBounds(leftOutputComboArea);
            }

            if (leftDeckArea.getHeight() >= 5)
                leftDeckArea.removeFromTop(25); // Spacer

            if (leftSyncButton) {
                leftSyncButton->setBounds(leftDeckArea);
				leftSyncButton->setSize(leftDeckArea.getWidth(), 30);
            }
        }

        // === RIGHT DECK LAYOUT ===
        if (!rightArea.isEmpty())
        {
            auto rightDeckArea = rightArea;

            // Deck Label
            if (rightDeckArea.getHeight() >= 25)
            {
                auto rightLabelArea = rightDeckArea.removeFromTop(25);
				if (rightLabel)
                    rightLabel->setBounds(rightLabelArea);
            }

            if (rightDeckArea.getHeight() >= 5)
                rightDeckArea.removeFromTop(5); // Spacer

            // Volume Slider (Hauptteil)
            if (rightDeckArea.getHeight() >= 250)
            {
                auto rightSliderArea = rightDeckArea.removeFromTop(250);
				if (rightSlider)
                    rightSlider->setBounds(rightSliderArea.reduced(juce::jmin(20, rightSliderArea.getWidth() / 4), 0));
            }

            if (rightDeckArea.getHeight() >= 10)
                rightDeckArea.removeFromTop(10); // Spacer

            // MIDI Learn Section
            if (rightDeckArea.getHeight() >= 25)
            {
                auto rightMidiArea = rightDeckArea.removeFromTop(25);
				if (rightLearnButton)
                    rightLearnButton->setBounds(rightMidiArea);
            }

            if (rightDeckArea.getHeight() >= 5)
                rightDeckArea.removeFromTop(5); // Spacer

            // CC Label
            if (rightDeckArea.getHeight() >= 20)
            {
                auto rightCCArea = rightDeckArea.removeFromTop(20);
                if (rightCCLabel)
                    rightCCLabel->setBounds(rightCCArea);
            }

            if (rightDeckArea.getHeight() >= 10)
                rightDeckArea.removeFromTop(10); // Spacer

            // Output Section
            if (rightDeckArea.getHeight() >= 20)
            {
                auto rightOutputLabelArea = rightDeckArea.removeFromTop(20);
				if (rightOutputLabel)
                    rightOutputLabel->setBounds(rightOutputLabelArea);
            }

            if (rightDeckArea.getHeight() >= 5)
                rightDeckArea.removeFromTop(5); // Spacer

            if (rightDeckArea.getHeight() >= 25)
            {
                auto rightOutputComboArea = rightDeckArea.removeFromTop(25);
				if (rightOutputCombo)
                    rightOutputCombo->setBounds(rightOutputComboArea);
            }

            if (rightDeckArea.getHeight() >= 5)
                rightDeckArea.removeFromTop(25); // Spacer

            if (rightSyncButton) {
                rightSyncButton->setBounds(rightDeckArea);
				rightSyncButton->setSize(rightDeckArea.getWidth(), 30);
            }

        }



        // === CROSSFADER SECTION (unten, über beide Decks) ===
        auto totalBounds = getLocalBounds();
        if (totalBounds.getHeight() >= 120)
        {
            auto crossfaderMainArea = totalBounds.removeFromBottom(120);
            crossfaderMainArea = crossfaderMainArea.reduced(20, 10); // Ränder

            if (!crossfaderMainArea.isEmpty())
            {
                // Crossfader Label
                if (crossfaderMainArea.getHeight() >= 25)
                {
                    auto crossfaderLabelArea = crossfaderMainArea.removeFromTop(25);
					if (crossfaderLabel)
                        crossfaderLabel->setBounds(crossfaderLabelArea);
                }

                if (crossfaderMainArea.getHeight() >= 5)
                    crossfaderMainArea.removeFromTop(5); // Spacer

                // Crossfader Slider
                if (crossfaderMainArea.getHeight() >= 40)
                {
                    auto crossfaderSliderArea = crossfaderMainArea.removeFromTop(40);
					if (crossfader)
                        crossfader->setBounds(crossfaderSliderArea);
                }

                if (crossfaderMainArea.getHeight() >= 5)
                    crossfaderMainArea.removeFromTop(5); // Spacer

                // Crossfader MIDI Controls (horizontal layout)
                if (crossfaderMainArea.getHeight() >= 25 && crossfaderMainArea.getWidth() >= 110)
                {
                    auto crossfaderControlsArea = crossfaderMainArea.removeFromTop(25);
                    auto crossfaderLearnArea = crossfaderControlsArea.removeFromLeft(100);

					if (crossfaderLearnButton)
                        crossfaderLearnButton->setBounds(crossfaderLearnArea);

                    if (crossfaderControlsArea.getWidth() >= 90)
                    {
                        crossfaderControlsArea.removeFromLeft(10); // Spacer zwischen Button und Label
						if (crossfaderCCLabel)
                            crossfaderCCLabel->setBounds(crossfaderControlsArea.removeFromLeft(80));
                    }
                }
            }
        }
    }

    // MIDI Learn Funktionalität
    enum class MidiTarget
    {
        LeftVolume,
        RightVolume,
        Crossfader
    };

    void startMidiLearn(MidiTarget target)
    {
        currentLearningTarget = target;
        isLearning = true;

        // Button visual feedback
        switch (target)
        {
        case MidiTarget::LeftVolume:
            leftLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            leftLearnButton->setButtonText("Learning...");
            break;
        case MidiTarget::RightVolume:
            rightLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            rightLearnButton->setButtonText("Learning...");
            break;
        case MidiTarget::Crossfader:
            crossfaderLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            crossfaderLearnButton->setButtonText("Learning...");
            break;
        }

        repaint();

        // Auto-stop learning after 10 seconds
        juce::Timer::callAfterDelay(10000, [this]() {
            if (isLearning) stopMidiLearn();
            });
    }

    void stopMidiLearn()
    {
        isLearning = false;

        // Reset button colors and text
        leftLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        rightLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        crossfaderLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);

        leftLearnButton->setButtonText("MIDI Learn");
        rightLearnButton->setButtonText("MIDI Learn");
        crossfaderLearnButton->setButtonText("MIDI Learn");

        repaint();
    }

    // Diese Funktion sollte von MainComponent aus aufgerufen werden
    void handleMidiMessage(const juce::MidiMessage& message)
    {
        if (message.isController())
        {
            int ccNumber = message.getControllerNumber();
            int ccValue = message.getControllerValue();

            // Alle GUI Updates müssen auf dem Message Thread ausgeführt werden
            juce::MessageManager::callAsync([this, ccNumber, ccValue]()
                {
                    if (isLearning)
                    {
                        // MIDI Learn Mode
                        switch (currentLearningTarget)
                        {
                        case MidiTarget::LeftVolume:
                            leftVolumeCC = ccNumber;
                            leftCCLabel->setText("CC: " + juce::String(ccNumber), juce::dontSendNotification);
                            break;
                        case MidiTarget::RightVolume:
                            rightVolumeCC = ccNumber;
                            rightCCLabel->setText("CC: " + juce::String(ccNumber), juce::dontSendNotification);
                            break;
                        case MidiTarget::Crossfader:
                            crossfaderCC = ccNumber;
                            crossfaderCCLabel->setText("CC: " + juce::String(ccNumber), juce::dontSendNotification);
                            break;
                        }
                        stopMidiLearn();
                    }
                    else
                    {
                        // Normal MIDI Control Mode
                        float normalizedValue = ccValue / 127.0f;

                        if (ccNumber == leftVolumeCC && leftVolumeCC != -1)
                        {
                            leftSlider->setValue(normalizedValue, juce::sendNotificationSync);
                        }
                        else if (ccNumber == rightVolumeCC && rightVolumeCC != -1)
                        {
                            rightSlider->setValue(normalizedValue, juce::sendNotificationSync);
                        }
                        else if (ccNumber == crossfaderCC && crossfaderCC != -1)
                        {
                            // Crossfader range is -1 to +1
                            float crossfaderValue = (normalizedValue * 2.0f) - 1.0f;
                            crossfader->setValue(crossfaderValue, juce::sendNotificationSync);
                        }
                    }
                });
        }
    }

    void resetMidiMappings()
    {
        leftVolumeCC = -1;
        rightVolumeCC = -1;
        crossfaderCC = -1;

        leftCCLabel->setText("CC: -", juce::dontSendNotification);
        rightCCLabel->setText("CC: -", juce::dontSendNotification);
        crossfaderCCLabel->setText("CC: -", juce::dontSendNotification);
    }

    // BPM und Sync Funktionen
    void analyzeBPMFromFile(const juce::File& audioFile, bool isLeftDeck)
    {
        if (isLeftDeck)
        {
            leftBPMAnalyzer->analyzeFile(audioFile);
            updateBPMDisplay(true);
        }
        else
        {
            rightBPMAnalyzer->analyzeFile(audioFile);
            updateBPMDisplay(false);
        }
    }

    void updateBPMDisplay(bool isLeftDeck)
    {
        if (isLeftDeck && leftBPMAnalyzer->isAnalysisComplete())
        {
            double bpm = leftBPMAnalyzer->getBPM();
            leftBPMLabel->setText(juce::String(bpm, 1) + " BPM", juce::dontSendNotification);
        }
        else if (!isLeftDeck && rightBPMAnalyzer->isAnalysisComplete())
        {
            double bpm = rightBPMAnalyzer->getBPM();
            rightBPMLabel->setText(juce::String(bpm, 1) + " BPM", juce::dontSendNotification);
        }
    }

    void syncLeftToRight()
    {
        if (leftBPMAnalyzer->isAnalysisComplete() && rightBPMAnalyzer->isAnalysisComplete())
        {
            double leftBPM = leftBPMAnalyzer->getBPM();
            double rightBPM = rightBPMAnalyzer->getBPM();
            double syncRatio = BPMAnalyzer::calculateSyncRatio(leftBPM, rightBPM);

            // Pitch-Anpassung: 1.0 = kein Pitch, 1.05 = +5%, 0.95 = -5%
            double pitchAdjust = (syncRatio - 1.0);
            leftPitchSlider->setValue(juce::jlimit(-0.08, 0.08, pitchAdjust), juce::sendNotificationSync);

            // Visual Feedback
            leftSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            juce::Timer::callAfterDelay(2000, [this]() {
                leftSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
                });
        }
    }

    void syncRightToLeft()
    {
        if (leftBPMAnalyzer->isAnalysisComplete() && rightBPMAnalyzer->isAnalysisComplete())
        {
            double leftBPM = leftBPMAnalyzer->getBPM();
            double rightBPM = rightBPMAnalyzer->getBPM();
            double syncRatio = BPMAnalyzer::calculateSyncRatio(rightBPM, leftBPM);

            double pitchAdjust = (syncRatio - 1.0);
            rightPitchSlider->setValue(juce::jlimit(-0.08, 0.08, pitchAdjust), juce::sendNotificationSync);

            // Visual Feedback
            rightSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            juce::Timer::callAfterDelay(2000, [this]() {
                rightSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
                });
        }
    }

    // Pitch-Werte für Audio-Engine
    double getLeftPitch() const { return 1.0 + leftPitchSlider->getValue(); }
    double getRightPitch() const { return 1.0 + rightPitchSlider->getValue(); }
    
    // Audio-Funktionen mit Output Routing
    float getLeftGain() {
        float baseGain = (float)leftSlider->getValue();
        float crossfaderValue = (float)crossfader->getValue();

        float crossfaderGain = 1.0f;
        if (crossfaderValue > 0.0f) {
            crossfaderGain = 1.0f - crossfaderValue;
        }

        return baseGain * crossfaderGain;
    }

    float getRightGain() {
        float baseGain = (float)rightSlider->getValue();
        float crossfaderValue = (float)crossfader->getValue();

        float crossfaderGain = 1.0f;
        if (crossfaderValue < 0.0f) {
            crossfaderGain = 1.0f + crossfaderValue;
        }

        return baseGain * crossfaderGain;
    }

    // Output Routing - gibt zurück ob Kanal zu bestimmtem Ausgang geht
    bool isLeftChannelRoutedToMaster() const {
        int selectedOutput = leftOutputCombo->getSelectedId();
        return selectedOutput == 1 || selectedOutput == 2; // Master L+R oder Left Only
    }

    bool isRightChannelRoutedToMaster() const {
        int selectedOutput = rightOutputCombo->getSelectedId();
        return selectedOutput == 1 || selectedOutput == 3; // Master L+R oder Right Only
    }

    bool isLeftChannelRoutedToCue() const {
        return leftOutputCombo->getSelectedId() == 4; // Cue (Headphones)
    }

    bool isRightChannelRoutedToCue() const {
        return rightOutputCombo->getSelectedId() == 4; // Cue (Headphones)
    }

    // Für Stereo-Routing: welcher physische Ausgang
    enum class OutputDestination {
        MasterLeft,
        MasterRight,
        CueLeft,
        CueRight,
        Muted
    };

    OutputDestination getLeftChannelDestination() const {
        switch (leftOutputCombo->getSelectedId()) {
        case 1: return OutputDestination::MasterLeft;  // Master L+R -> Left
        case 2: return OutputDestination::MasterLeft;  // Left Only -> Left
        case 3: return OutputDestination::MasterRight; // Right Only -> Right
        case 4: return OutputDestination::CueLeft;     // Cue -> Cue Left
        case 5: return OutputDestination::Muted;       // Muted
        default: return OutputDestination::MasterLeft;
        }
    }

    OutputDestination getRightChannelDestination() const {
        switch (rightOutputCombo->getSelectedId()) {
        case 1: return OutputDestination::MasterRight; // Master L+R -> Right
        case 2: return OutputDestination::MasterLeft;  // Left Only -> Left
        case 3: return OutputDestination::MasterRight; // Right Only -> Right
        case 4: return OutputDestination::CueRight;    // Cue -> Cue Right
        case 5: return OutputDestination::Muted;       // Muted
        default: return OutputDestination::MasterRight;
        }
    }

    float getCrossfaderValue() {
        return (float)crossfader->getValue();
    }

    // Getter für ComboBoxes (für MainComponent)
    juce::ComboBox* getLeftOutputCombo() const { return leftOutputCombo.get(); }
    juce::ComboBox* getRightOutputCombo() const { return rightOutputCombo.get(); }

    // Getter für MIDI Mappings (für Persistierung)
    int getLeftVolumeCC() const { return leftVolumeCC; }
    int getRightVolumeCC() const { return rightVolumeCC; }
    int getCrossfaderCC() const { return crossfaderCC; }

    // Setter für MIDI Mappings (für Laden gespeicherter Werte)
    void setLeftVolumeCC(int cc) {
        leftVolumeCC = cc;
        leftCCLabel->setText(cc >= 0 ? "CC: " + juce::String(cc) : "CC: -", juce::dontSendNotification);
    }
    void setRightVolumeCC(int cc) {
        rightVolumeCC = cc;
        rightCCLabel->setText(cc >= 0 ? "CC: " + juce::String(cc) : "CC: -", juce::dontSendNotification);
    }
    void setCrossfaderCC(int cc) {
        crossfaderCC = cc;
        crossfaderCCLabel->setText(cc >= 0 ? "CC: " + juce::String(cc) : "CC: -", juce::dontSendNotification);
    }

private:
    // GUI Components
    std::unique_ptr<juce::Slider> leftSlider = nullptr;
    std::unique_ptr<juce::Slider> rightSlider = nullptr;
    std::unique_ptr<juce::Slider> crossfader = nullptr;

    std::unique_ptr<juce::ComboBox> leftOutputCombo = nullptr;
    std::unique_ptr<juce::ComboBox> rightOutputCombo = nullptr;

    std::unique_ptr<juce::TextButton> leftLearnButton = nullptr;
    std::unique_ptr<juce::TextButton> rightLearnButton = nullptr;
    std::unique_ptr<juce::TextButton> crossfaderLearnButton = nullptr;

    std::unique_ptr<juce::Label> leftLabel = nullptr;
    std::unique_ptr<juce::Label> rightLabel = nullptr;
    std::unique_ptr<juce::Label> crossfaderLabel = nullptr;
    std::unique_ptr<juce::Label> leftOutputLabel = nullptr;
    std::unique_ptr<juce::Label> rightOutputLabel = nullptr;

    std::unique_ptr<juce::Label> leftCCLabel = nullptr;
    std::unique_ptr<juce::Label> rightCCLabel = nullptr;
    std::unique_ptr<juce::Label> crossfaderCCLabel = nullptr;

    // GUI Components - BPM und Pitch
    std::unique_ptr<juce::Label> leftBPMLabel = nullptr;
    std::unique_ptr<juce::Label> rightBPMLabel = nullptr;
    std::unique_ptr<juce::Slider> leftPitchSlider = nullptr;
    std::unique_ptr<juce::Slider> rightPitchSlider = nullptr;
    std::unique_ptr<juce::Label> leftPitchLabel = nullptr;
    std::unique_ptr<juce::Label> rightPitchLabel = nullptr;
    std::unique_ptr<juce::TextButton> leftSyncButton = nullptr;
    std::unique_ptr<juce::TextButton> rightSyncButton = nullptr;

    // BPM Analysis
    std::unique_ptr<BPMAnalyzer> leftBPMAnalyzer = nullptr;
    std::unique_ptr<BPMAnalyzer> rightBPMAnalyzer = nullptr;

    // MIDI Learning State
    bool isLearning = false;
    MidiTarget currentLearningTarget = MidiTarget::LeftVolume;

    // MIDI CC Mappings
    int leftVolumeCC = -1;
    int rightVolumeCC = -1;
    int crossfaderCC = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};
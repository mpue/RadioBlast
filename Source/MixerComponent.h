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
#include "LevelMeterComponent.h"
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
        leftBPMLabel = std::make_unique<juce::Label>("120.0 BPM");
        rightBPMLabel = std::make_unique<juce::Label>("120.0 BPM");
        leftBPMLabel->setJustificationType(juce::Justification::centred);
        rightBPMLabel->setJustificationType(juce::Justification::centred);
        leftBPMLabel->setFont(12.0f);
        rightBPMLabel->setFont(12.0f);
        // leftBPMLabel->setColour(juce::Label::backgroundColourId, juce::Colours::black);
        // rightBPMLabel->setColour(juce::Label::backgroundColourId, juce::Colours::black);
        leftBPMLabel->setColour(juce::Label::textColourId, juce::Colours::lime);
        rightBPMLabel->setColour(juce::Label::textColourId, juce::Colours::lime);
        addAndMakeVisible(leftBPMLabel.get());
        addAndMakeVisible(rightBPMLabel.get());

        // Pitch/Tempo Slider (±8% wie bei echten DJ Mixern)
        leftPitchSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
        rightPitchSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
        leftPitchSlider->setRange(-1.0, 1.0, 0.01); // ±8%
        rightPitchSlider->setRange(-1.0, 1.0, 0.01);
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

        addLevelMetersToMixerComponent();

        // Reset MIDI mappings
        resetMidiMappings();
    }

    ~MixerComponent() override
    {
    }


    // Add these to your MixerComponent class (in the constructor):
    void addLevelMetersToMixerComponent()
    {
        // Level meters for both channels
        leftLevelMeter = std::make_unique<LevelMeter>();
        rightLevelMeter = std::make_unique<LevelMeter>();
        masterLevelMeterL = std::make_unique<LevelMeter>();
        masterLevelMeterR = std::make_unique<LevelMeter>();

        addAndMakeVisible(leftLevelMeter.get());
        addAndMakeVisible(rightLevelMeter.get());
        addAndMakeVisible(masterLevelMeterL.get());
        addAndMakeVisible(masterLevelMeterR.get());

        // Level meter labels
        leftLevelLabel = std::make_unique<juce::Label>("", "L");
        rightLevelLabel = std::make_unique<juce::Label>("", "R");
        masterLevelLabel = std::make_unique<juce::Label>("", "MASTER");

        leftLevelLabel->setJustificationType(juce::Justification::centred);
        rightLevelLabel->setJustificationType(juce::Justification::centred);
        masterLevelLabel->setJustificationType(juce::Justification::centred);

        leftLevelLabel->setFont(10.0f);
        rightLevelLabel->setFont(10.0f);
        masterLevelLabel->setFont(12.0f);

        addAndMakeVisible(leftLevelLabel.get());
        addAndMakeVisible(rightLevelLabel.get());
        addAndMakeVisible(masterLevelLabel.get());
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

    // Ersetze die komplette resized() Methode in deiner MixerComponent.h mit dieser Version:
    void resized() override
    {
        auto area = getLocalBounds();
        if (area.isEmpty()) return;

        area = area.reduced(10); // Außenrand
        if (area.isEmpty()) return;

        // === LAYOUT BEREICHE DEFINIEREN ===
        int crossfaderHeight = 120;
        int centerWidth = 60; // Breiter für Master-Meter
        int deckWidth = (area.getWidth() - centerWidth - 20) / 2; // 20px Abstände

        // Bereiche aufteilen
        auto leftDeckArea = area.withWidth(deckWidth);
        auto centerArea = juce::Rectangle<int>(leftDeckArea.getRight() + 10, area.getY(),
            centerWidth, area.getHeight() - crossfaderHeight);
        auto rightDeckArea = juce::Rectangle<int>(centerArea.getRight() + 10, area.getY(),
            deckWidth, area.getHeight() - crossfaderHeight);
        auto crossfaderArea = area.removeFromBottom(crossfaderHeight).reduced(20, 10);

        // === LINKES DECK LAYOUT ===
        layoutSingleDeck(leftDeckArea, true);

        // === RECHTES DECK LAYOUT ===
        layoutSingleDeck(rightDeckArea, false);

        // === MASTER PEGELANZEIGEN (MITTE) ===
        layoutMasterMeters(centerArea);

        // === CROSSFADER BEREICH ===
        layoutCrossfaderSection(crossfaderArea);
    }


    // Inline Layout-Methoden - alles in der Header-Datei

    void layoutSingleDeck(juce::Rectangle<int> deckArea, bool isLeft)
    {
        if (deckArea.isEmpty()) return;

        auto workingArea = deckArea.reduced(5); // Innenrand

        // Komponenten für dieses Deck holen
        auto slider = isLeft ? leftSlider.get() : rightSlider.get();
        auto levelMeter = isLeft ? leftLevelMeter.get() : rightLevelMeter.get();
        auto label = isLeft ? leftLabel.get() : rightLabel.get();
        auto learnButton = isLeft ? leftLearnButton.get() : rightLearnButton.get();
        auto ccLabel = isLeft ? leftCCLabel.get() : rightCCLabel.get();
        auto outputLabel = isLeft ? leftOutputLabel.get() : rightOutputLabel.get();
        auto outputCombo = isLeft ? leftOutputCombo.get() : rightOutputCombo.get();
        auto syncButton = isLeft ? leftSyncButton.get() : rightSyncButton.get();
        auto pitchSlider = isLeft ? leftPitchSlider.get() : rightPitchSlider.get();
        auto pitchLabel = isLeft ? leftPitchLabel.get() : rightPitchLabel.get();
        auto levelLabel = isLeft ? leftLevelLabel.get() : rightLevelLabel.get();

        // === DECK LABEL (OBEN) ===
        if (label && workingArea.getHeight() >= 25)
        {
            auto labelArea = workingArea.removeFromTop(25);
            label->setBounds(labelArea);
            workingArea.removeFromTop(5); // Spacer
        }

        // === HAUPTBEREICH: VOLUME SLIDER + PEGELANZEIGE NEBENEINANDER ===
        if (workingArea.getHeight() >= 200)
        {
            auto mainControlArea = workingArea.removeFromTop(200);

            // Volume Slider nimmt den Hauptteil ein
            int sliderWidth = mainControlArea.getWidth() - 30; // Platz für Pegelanzeige
            if (slider)
            {
                auto sliderArea = mainControlArea.withWidth(sliderWidth).reduced(5, 0);
                slider->setBounds(sliderArea);
            }

            // Pegelanzeige rechts neben Slider - DEUTLICH SICHTBAR
            if (levelMeter)
            {
                auto meterArea = juce::Rectangle<int>(mainControlArea.getX() + sliderWidth + 2,
                    mainControlArea.getY(),
                    25, mainControlArea.getHeight() - 20);
                levelMeter->setBounds(meterArea);

                // Level Label unter Pegelanzeige
                if (levelLabel)
                {
                    auto levelLabelArea = juce::Rectangle<int>(meterArea.getX() - 2,
                        meterArea.getBottom() + 2,
                        29, 15);
                    levelLabel->setBounds(levelLabelArea);
                    levelLabel->setText("LEV", juce::dontSendNotification); // Kürzer für bessere Sichtbarkeit
                }
            }

            workingArea.removeFromTop(10); // Spacer
        }

        // === MIDI LEARN BEREICH ===
        if (learnButton && workingArea.getHeight() >= 25)
        {
            auto midiArea = workingArea.removeFromTop(25);
            learnButton->setBounds(midiArea);
            workingArea.removeFromTop(3); // Kleiner Spacer
        }

        if (ccLabel && workingArea.getHeight() >= 18)
        {
            auto ccArea = workingArea.removeFromTop(18);
            ccLabel->setBounds(ccArea);
            workingArea.removeFromTop(8); // Spacer
        }

        // === OUTPUT ROUTING ===
        if (outputLabel && workingArea.getHeight() >= 18)
        {
            auto outputLabelArea = workingArea.removeFromTop(18);
            outputLabel->setBounds(outputLabelArea);
            workingArea.removeFromTop(3); // Spacer
        }

        if (outputCombo && workingArea.getHeight() >= 25)
        {
            auto outputComboArea = workingArea.removeFromTop(25);
            outputCombo->setBounds(outputComboArea);
            workingArea.removeFromTop(10); // Spacer
        }

        // === SYNC BUTTON ===
        if (syncButton && workingArea.getHeight() >= 30)
        {
            auto syncArea = workingArea.removeFromTop(30);
            syncButton->setBounds(syncArea);
            workingArea.removeFromTop(5); // Kleiner Spacer
        }

        // === PITCH SLIDER (DIREKT UNTER SYNC) ===
        if (pitchSlider && workingArea.getHeight() >= 25)
        {
            auto pitchArea = workingArea.removeFromTop(25);
            pitchSlider->setBounds(pitchArea);
            workingArea.removeFromTop(3); // Spacer
        }

        // === PITCH LABEL ===
        if (pitchLabel && workingArea.getHeight() >= 15)
        {
            auto pitchLabelArea = workingArea.removeFromTop(15);
            pitchLabel->setBounds(pitchLabelArea);
        }
    }

    void layoutMasterMeters(juce::Rectangle<int> centerArea)
    {
        if (centerArea.isEmpty()) return;

        auto workingArea = centerArea.reduced(5);

        // Master Label oben
        if (masterLevelLabel && workingArea.getHeight() >= 20)
        {
            auto labelArea = workingArea.removeFromTop(20);
            masterLevelLabel->setBounds(labelArea);
            workingArea.removeFromTop(10); // Spacer
        }

        // Master Pegelanzeigen - nebeneinander, größer und prominenter
        if (masterLevelMeterL && masterLevelMeterR && workingArea.getHeight() >= 150)
        {
            auto meterArea = workingArea.removeFromTop(150);

            int meterWidth = 20;
            int spacing = (meterArea.getWidth() - (2 * meterWidth)) / 2;

            auto leftMeterBounds = juce::Rectangle<int>(meterArea.getX(), meterArea.getY(),
                meterWidth, meterArea.getHeight());
            auto rightMeterBounds = juce::Rectangle<int>(meterArea.getX() + meterWidth + spacing,
                meterArea.getY(),
                meterWidth, meterArea.getHeight());

            masterLevelMeterL->setBounds(leftMeterBounds);
            masterLevelMeterR->setBounds(rightMeterBounds);

            workingArea.removeFromTop(5); // Spacer
        }

        // L/R Labels unter den Metern - create if needed
        if (workingArea.getHeight() >= 15)
        {
            auto labelArea = workingArea.removeFromTop(15);

            // Create L/R label if it doesn't exist
            if (!masterLRLabel)
            {
                masterLRLabel = std::make_unique<juce::Label>("", "L    R");
                masterLRLabel->setJustificationType(juce::Justification::centred);
                masterLRLabel->setFont(10.0f);
                addAndMakeVisible(masterLRLabel.get());
            }
            masterLRLabel->setBounds(labelArea);
        }
    }

    void layoutCrossfaderSection(juce::Rectangle<int> crossfaderArea)
    {
        if (crossfaderArea.isEmpty()) return;

        // Crossfader Label
        if (crossfaderLabel && crossfaderArea.getHeight() >= 20)
        {
            auto labelArea = crossfaderArea.removeFromTop(20);
            crossfaderLabel->setBounds(labelArea);
            crossfaderArea.removeFromTop(5); // Spacer
        }

        // Crossfader Slider
        if (crossfader && crossfaderArea.getHeight() >= 35)
        {
            auto sliderArea = crossfaderArea.removeFromTop(35);
            crossfader->setBounds(sliderArea);
            crossfaderArea.removeFromTop(5); // Spacer
        }

        // Crossfader Controls (horizontal)
        if (crossfaderLearnButton && crossfaderCCLabel && crossfaderArea.getHeight() >= 25)
        {
            auto controlsArea = crossfaderArea.removeFromTop(25);

            // Learn Button links
            auto learnArea = controlsArea.removeFromLeft(120);
            crossfaderLearnButton->setBounds(learnArea);

            // Spacer
            controlsArea.removeFromLeft(10);

            // CC Label rechts
            if (controlsArea.getWidth() >= 60)
            {
                crossfaderCCLabel->setBounds(controlsArea.removeFromLeft(60));
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
            leftSyncButton->setButtonText(juce::String(bpm, 1) + " BPM");
        }
        else if (!isLeftDeck && rightBPMAnalyzer->isAnalysisComplete())
        {
            double bpm = rightBPMAnalyzer->getBPM();
            rightSyncButton->setButtonText(juce::String(bpm, 1) + " BPM");
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
    
    float getLeftChannelGain() {
        return (float)leftSlider->getValue();
    }

    float getRightChannelGain() {
        return (float)rightSlider->getValue();
    }

    // Get gain for master output (with crossfader influence)
    float getLeftMasterGain() {
        float baseGain = (float)leftSlider->getValue();
        float crossfaderValue = (float)crossfader->getValue();

        float crossfaderGain = 1.0f;
        if (crossfaderValue > 0.0f) {
            crossfaderGain = 1.0f - crossfaderValue;
        }

        return baseGain * crossfaderGain;
    }

    float getRightMasterGain() {
        float baseGain = (float)rightSlider->getValue();
        float crossfaderValue = (float)crossfader->getValue();

        float crossfaderGain = 1.0f;
        if (crossfaderValue < 0.0f) {
            crossfaderGain = 1.0f + crossfaderValue;
        }

        return baseGain * crossfaderGain;
    }

    // Get gain for cue output (no crossfader influence)
    float getLeftCueGain() {
        return (float)leftSlider->getValue();
    }

    float getRightCueGain() {
        return (float)rightSlider->getValue();
    }

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

    void updateLeftChannelLevel(float leftLevel, float rightLevel)
    {
        if (leftLevelMeter)
        {
            // Use RMS for more stable meter display
            float rmsLevel = std::sqrt((leftLevel * leftLevel + rightLevel * rightLevel) * 0.5f);
            leftLevelMeter->setLevel(rmsLevel);
        }
    }

    void updateRightChannelLevel(float leftLevel, float rightLevel)
    {
        if (rightLevelMeter)
        {
            float rmsLevel = std::sqrt((leftLevel * leftLevel + rightLevel * rightLevel) * 0.5f);
            rightLevelMeter->setLevel(rmsLevel);
        }
    }

    void updateMasterLevels(float masterLeft, float masterRight)
    {
        if (masterLevelMeterL)
            masterLevelMeterL->setLevel(std::abs(masterLeft));
        if (masterLevelMeterR)
            masterLevelMeterR->setLevel(std::abs(masterRight));
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

    std::unique_ptr<LevelMeter> leftLevelMeter = nullptr;
    std::unique_ptr<LevelMeter> rightLevelMeter = nullptr;
    std::unique_ptr<LevelMeter> masterLevelMeterL = nullptr;
    std::unique_ptr<LevelMeter> masterLevelMeterR = nullptr;

    std::unique_ptr<juce::Label> leftLevelLabel = nullptr;
    std::unique_ptr<juce::Label> rightLevelLabel = nullptr;
    std::unique_ptr<juce::Label> masterLevelLabel = nullptr;

    std::unique_ptr<juce::Label> masterLRLabel = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};
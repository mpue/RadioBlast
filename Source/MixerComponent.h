/*
  ==============================================================================
	MixerComponent.h - Verbesserte Version
	- Master-Meter über volle Höhe
	- Größerer Abstand zwischen Decks
	- MIDI Learn für Pitch-Regler
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "BPMAnalyzer.h"
#include "LevelMeterComponent.h"
#include "BaseComponent.h"

class MixerComponent : public BaseComponent
{
public:
	MixerComponent()
	{
		setSize(578, 570); // Breiter für besseres Layout mit mehr Abstand


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

		// MIDI Learn Buttons für Volume
		leftLearnButton = std::make_unique<juce::TextButton>("MIDI Learn Vol");
		rightLearnButton = std::make_unique<juce::TextButton>("MIDI Learn Vol");
		addAndMakeVisible(leftLearnButton.get());
		addAndMakeVisible(rightLearnButton.get());

		// MIDI CC Status Labels für Volume
		leftCCLabel = std::make_unique<juce::Label>("", "Vol CC: -");
		rightCCLabel = std::make_unique<juce::Label>("", "Vol CC: -");
		leftCCLabel->setJustificationType(juce::Justification::centred);
		rightCCLabel->setJustificationType(juce::Justification::centred);
		leftCCLabel->setFont(9.0f);
		rightCCLabel->setFont(9.0f);
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
		crossfader = std::make_unique<juce::Slider>();
		crossfader->setSliderStyle(juce::Slider::LinearHorizontal);
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

		// Pitch/Tempo Slider (±8% wie bei echten DJ Mixern)
		leftPitchSlider = std::make_unique<juce::Slider>();
		leftPitchSlider->setSliderStyle(juce::Slider::LinearHorizontal);
		rightPitchSlider = std::make_unique<juce::Slider>();
		rightPitchSlider->setSliderStyle(juce::Slider::LinearHorizontal);
		leftPitchSlider->setRange(-1.0, 1.0, 0.01);
		rightPitchSlider->setRange(-1.0, 1.0, 0.01);
		leftPitchSlider->setValue(0.0);
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

		// MIDI Learn Buttons für Pitch
		leftPitchLearnButton = std::make_unique<juce::TextButton>("MIDI Learn Pitch");
		rightPitchLearnButton = std::make_unique<juce::TextButton>("MIDI Learn Pitch");
		addAndMakeVisible(leftPitchLearnButton.get());
		addAndMakeVisible(rightPitchLearnButton.get());

		// MIDI CC Status Labels für Pitch
		leftPitchCCLabel = std::make_unique<juce::Label>("", "Pitch CC: -");
		rightPitchCCLabel = std::make_unique<juce::Label>("", "Pitch CC: -");
		leftPitchCCLabel->setJustificationType(juce::Justification::centred);
		rightPitchCCLabel->setJustificationType(juce::Justification::centred);
		leftPitchCCLabel->setFont(9.0f);
		rightPitchCCLabel->setFont(9.0f);
		addAndMakeVisible(leftPitchCCLabel.get());
		addAndMakeVisible(rightPitchCCLabel.get());

		// Sync Buttons
		leftSyncButton = std::make_unique<juce::TextButton>("SYNC");
		rightSyncButton = std::make_unique<juce::TextButton>("SYNC");
		leftSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
		rightSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
		addAndMakeVisible(leftSyncButton.get());
		addAndMakeVisible(rightSyncButton.get());

		// Button Callbacks - ERWEITERT für Pitch
		leftLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::LeftVolume); };
		rightLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::RightVolume); };
		crossfaderLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::Crossfader); };
		leftPitchLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::LeftPitch); };
		rightPitchLearnButton->onClick = [this]() { startMidiLearn(MidiTarget::RightPitch); };

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
		g.fillAll(juce::Colour(0xff222222));
		g.setColour(juce::Colours::grey);
		g.drawRect(getLocalBounds(), 1);

		auto area = getLocalBounds().reduced(10);

		// Deck Bereiche visuell trennen - VERGRÖSSERTE ABSTÄNDE
		int centerWidth = 80; // Verbreitert für Master-Meter
		int deckWidth = (area.getWidth() - centerWidth - 40) / 2; // 40px Abstände statt 20px

		// Left Deck Background
		auto leftDeckRect = juce::Rectangle<int>(10, 10, deckWidth, 420);
		g.setColour(juce::Colour(0xff2a2a2a));
		g.fillRoundedRectangle(leftDeckRect.toFloat(), 5.0f);
		g.setColour(juce::Colours::darkgrey);
		g.drawRoundedRectangle(leftDeckRect.toFloat(), 5.0f, 1.0f);

		// Right Deck Background - GRÖSSERER ABSTAND
		auto rightDeckRect = juce::Rectangle<int>(deckWidth + 50 + centerWidth, 10, deckWidth, 420);
		g.setColour(juce::Colour(0xff2a2a2a));
		g.fillRoundedRectangle(rightDeckRect.toFloat(), 5.0f);
		g.setColour(juce::Colours::darkgrey);
		g.drawRoundedRectangle(rightDeckRect.toFloat(), 5.0f, 1.0f);

		// Crossfader Section Background
		auto crossfaderRect = juce::Rectangle<int>(20, 450, getWidth() - 40, 120);
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

			juce::String learningText = "MIDI Learning - Move ";
			switch (currentLearningTarget)
			{
			case MidiTarget::LeftVolume: learningText += "LEFT VOLUME"; break;
			case MidiTarget::RightVolume: learningText += "RIGHT VOLUME"; break;
			case MidiTarget::LeftPitch: learningText += "LEFT PITCH"; break;
			case MidiTarget::RightPitch: learningText += "RIGHT PITCH"; break;
			case MidiTarget::Crossfader: learningText += "CROSSFADER"; break;
			}
			learningText += " control...";

			g.drawText(learningText, getLocalBounds(), juce::Justification::centred);
		}
	}


	void onResized() override
	{
		const int minWidth = 578;
		const int minHeight = 580;

		setSize( minWidth, minHeight);

		auto area = getLocalBounds();
		if (area.isEmpty()) return;

		area = area.reduced(10);
		if (area.isEmpty()) return;

		// === LAYOUT BEREICHE DEFINIEREN - VERGRÖSSERTE ABSTÄNDE ===
		int crossfaderHeight = 120;
		int centerWidth = 80; // Breiter für Master-Meter über volle Höhe
		int spacing = 20; // Vergrößerter Abstand zwischen Bereichen
		int deckWidth = (area.getWidth() - centerWidth - (spacing * 2)) / 2;

		// Bereiche aufteilen
		auto leftDeckArea = area.withWidth(deckWidth);
		auto centerArea = juce::Rectangle<int>(leftDeckArea.getRight() + spacing, area.getY(),
			centerWidth, area.getHeight() - crossfaderHeight);
		auto rightDeckArea = juce::Rectangle<int>(centerArea.getRight() + spacing, area.getY(),
			deckWidth, area.getHeight() - crossfaderHeight);
		auto crossfaderArea = area.removeFromBottom(crossfaderHeight).reduced(20, 10);

		// === DECK LAYOUTS ===
		layoutSingleDeck(leftDeckArea, true);
		layoutSingleDeck(rightDeckArea, false);

		// === MASTER PEGELANZEIGEN - ÜBER VOLLE HÖHE ===
		layoutMasterMeters(centerArea);

		// === CROSSFADER BEREICH ===
		layoutCrossfaderSection(crossfaderArea);
	}

	void layoutSingleDeck(juce::Rectangle<int> deckArea, bool isLeft)
	{
		if (deckArea.isEmpty()) return;

		auto workingArea = deckArea.reduced(8); // Etwas mehr Innenrand

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
		auto pitchLearnButton = isLeft ? leftPitchLearnButton.get() : rightPitchLearnButton.get();
		auto pitchCCLabel = isLeft ? leftPitchCCLabel.get() : rightPitchCCLabel.get();
		auto levelLabel = isLeft ? leftLevelLabel.get() : rightLevelLabel.get();

		// === DECK LABEL (OBEN) ===
		if (label && workingArea.getHeight() >= 25)
		{
			auto labelArea = workingArea.removeFromTop(25);
			label->setBounds(labelArea);
			workingArea.removeFromTop(8);
		}

		// === HAUPTBEREICH: VOLUME SLIDER + PEGELANZEIGE ===
		if (workingArea.getHeight() >= 180)
		{
			auto mainControlArea = workingArea.removeFromTop(180);

			int sliderWidth = mainControlArea.getWidth() - 35;
			if (slider)
			{
				auto sliderArea = mainControlArea.withWidth(sliderWidth).reduced(3, 0);
				slider->setBounds(sliderArea);
			}

			// Pegelanzeige rechts - SEHR SICHTBAR
			if (levelMeter)
			{
				auto meterArea = juce::Rectangle<int>(mainControlArea.getX() + sliderWidth + 3,
					mainControlArea.getY(),
					28, mainControlArea.getHeight() - 18);
				levelMeter->setBounds(meterArea);

				if (levelLabel)
				{
					auto levelLabelArea = juce::Rectangle<int>(meterArea.getX() - 2,
						meterArea.getBottom() + 2,
						32, 15);
					levelLabel->setBounds(levelLabelArea);
					levelLabel->setText("LEV", juce::dontSendNotification);
				}
			}

			workingArea.removeFromTop(12);
		}

		// === VOLUME MIDI LEARN ===
		if (learnButton && workingArea.getHeight() >= 22)
		{
			auto midiArea = workingArea.removeFromTop(22);
			learnButton->setBounds(midiArea);
			workingArea.removeFromTop(3);
		}

		if (ccLabel && workingArea.getHeight() >= 15)
		{
			auto ccArea = workingArea.removeFromTop(15);
			ccLabel->setBounds(ccArea);
			workingArea.removeFromTop(8);
		}

		// === PITCH SLIDER + MIDI LEARN ===
		if (pitchSlider && workingArea.getHeight() >= 25)
		{
			auto pitchArea = workingArea.removeFromTop(25);
			pitchSlider->setBounds(pitchArea);
			workingArea.removeFromTop(3);
		}

		if (pitchLabel && workingArea.getHeight() >= 15)
		{
			auto pitchLabelArea = workingArea.removeFromTop(15);
			pitchLabel->setBounds(pitchLabelArea);
			workingArea.removeFromTop(5);
		}

		// PITCH MIDI LEARN
		if (pitchLearnButton && workingArea.getHeight() >= 20)
		{
			auto pitchMidiArea = workingArea.removeFromTop(20);
			pitchLearnButton->setBounds(pitchMidiArea);
			workingArea.removeFromTop(3);
		}

		if (pitchCCLabel && workingArea.getHeight() >= 15)
		{
			auto pitchCCArea = workingArea.removeFromTop(15);
			pitchCCLabel->setBounds(pitchCCArea);
			workingArea.removeFromTop(8);
		}

		// === OUTPUT ROUTING ===
		if (outputLabel && workingArea.getHeight() >= 15)
		{
			auto outputLabelArea = workingArea.removeFromTop(15);
			outputLabel->setBounds(outputLabelArea);
			workingArea.removeFromTop(3);
		}

		if (outputCombo && workingArea.getHeight() >= 22)
		{
			auto outputComboArea = workingArea.removeFromTop(22);
			outputCombo->setBounds(outputComboArea);
			workingArea.removeFromTop(8);
		}

		// === SYNC BUTTON ===
		if (syncButton && workingArea.getHeight() >= 25)
		{
			auto syncArea = workingArea.removeFromTop(25);
			syncButton->setBounds(syncArea);
		}
	}

	void layoutMasterMeters(juce::Rectangle<int> centerArea)
	{
		if (centerArea.isEmpty()) return;

		auto workingArea = centerArea.reduced(5);

		// Master Label oben
		if (masterLevelLabel && workingArea.getHeight() >= 18)
		{
			auto labelArea = workingArea.removeFromTop(18);
			masterLevelLabel->setBounds(labelArea);
			workingArea.removeFromTop(8);
		}

		// Master Pegelanzeigen - ÜBER VOLLE VERFÜGBARE HÖHE
		if (masterLevelMeterL && masterLevelMeterR && workingArea.getHeight() >= 100)
		{
			// Gesamte verfügbare Höhe nutzen (minus Platz für L/R Label)
			auto meterArea = workingArea.removeFromTop(workingArea.getHeight() - 20);

			int meterWidth = 25;
			int totalMetersWidth = 2 * meterWidth;
			int spacing = std::max(5, (meterArea.getWidth() - totalMetersWidth) / 2);

			auto leftMeterBounds = juce::Rectangle<int>(
				meterArea.getX() + (meterArea.getWidth() - totalMetersWidth - 10) / 2,
				meterArea.getY(),
				meterWidth, meterArea.getHeight());

			auto rightMeterBounds = juce::Rectangle<int>(
				leftMeterBounds.getRight() + 5,
				meterArea.getY(),
				meterWidth, meterArea.getHeight());

			masterLevelMeterL->setBounds(leftMeterBounds);
			masterLevelMeterR->setBounds(rightMeterBounds);

			workingArea.removeFromTop(5);
		}

		// L/R Labels unter den Metern
		if (workingArea.getHeight() >= 15)
		{
			auto labelArea = workingArea.removeFromTop(15);

			if (!masterLRLabel)
			{
				masterLRLabel = std::make_unique<juce::Label>("", "L    R");
				masterLRLabel->setJustificationType(juce::Justification::centred);
				masterLRLabel->setFont(11.0f);
				masterLRLabel->setColour(juce::Label::textColourId, juce::Colours::lightblue);
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
			crossfaderArea.removeFromTop(5);
		}

		// Crossfader Slider
		if (crossfader && crossfaderArea.getHeight() >= 35)
		{
			auto sliderArea = crossfaderArea.removeFromTop(35);
			crossfader->setBounds(sliderArea);
			crossfaderArea.removeFromTop(5);
		}

		// Crossfader Controls (horizontal)
		if (crossfaderLearnButton && crossfaderCCLabel && crossfaderArea.getHeight() >= 25)
		{
			auto controlsArea = crossfaderArea.removeFromTop(25);

			auto learnArea = controlsArea.removeFromLeft(120);
			crossfaderLearnButton->setBounds(learnArea);

			controlsArea.removeFromLeft(10);

			if (controlsArea.getWidth() >= 60)
			{
				crossfaderCCLabel->setBounds(controlsArea.removeFromLeft(60));
			}
		}
	}

	// ERWEITERTE MIDI Learn Funktionalität
	enum class MidiTarget
	{
		LeftVolume,
		RightVolume,
		LeftPitch,    // NEU
		RightPitch,   // NEU
		Crossfader
	};

	void startMidiLearn(MidiTarget target)
	{
		currentLearningTarget = target;
		isLearning = true;

		// Alle Buttons zurücksetzen
		resetLearnButtonColors();

		// Aktuellen Button hervorheben
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
		case MidiTarget::LeftPitch:
			leftPitchLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
			leftPitchLearnButton->setButtonText("Learning...");
			break;
		case MidiTarget::RightPitch:
			rightPitchLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
			rightPitchLearnButton->setButtonText("Learning...");
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
		resetLearnButtonColors();
		repaint();
	}

	void resetLearnButtonColors()
	{
		// Volume Learn Buttons
		leftLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		rightLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		leftLearnButton->setButtonText("MIDI Learn Vol");
		rightLearnButton->setButtonText("MIDI Learn Vol");

		// Pitch Learn Buttons
		leftPitchLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		rightPitchLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		leftPitchLearnButton->setButtonText("MIDI Learn Pitch");
		rightPitchLearnButton->setButtonText("MIDI Learn Pitch");

		// Crossfader Learn Button
		crossfaderLearnButton->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		crossfaderLearnButton->setButtonText("MIDI Learn");
	}

	// ERWEITERTE MIDI Message Handler
	void handleMidiMessage(const juce::MidiMessage& message)
	{
		if (message.isController())
		{
			int ccNumber = message.getControllerNumber();
			int ccValue = message.getControllerValue();
			int ccChannel = message.getChannel();

			juce::MessageManager::callAsync([this, ccNumber, ccValue, ccChannel]()
				{
					if (isLearning)
					{
						// MIDI Learn Mode - ERWEITERT für Pitch
						switch (currentLearningTarget)
						{
						case MidiTarget::LeftVolume:
							leftVolumeCC = ccNumber;
							leftVolumeChannel = ccChannel;
							leftCCLabel->setText("Vol CC: " + juce::String(ccNumber), juce::dontSendNotification);
							break;
						case MidiTarget::RightVolume:
							rightVolumeCC = ccNumber;
							rightVolumeChannel = ccChannel;
							rightCCLabel->setText("Vol CC: " + juce::String(ccNumber), juce::dontSendNotification);
							break;
						case MidiTarget::LeftPitch:
							leftPitchCC = ccNumber;
							leftPitchChannel = ccChannel;
							leftPitchCCLabel->setText("Pitch CC: " + juce::String(ccNumber), juce::dontSendNotification);
							break;
						case MidiTarget::RightPitch:
							rightPitchCC = ccNumber;
							rightPitchChannel = ccChannel;
							rightPitchCCLabel->setText("Pitch CC: " + juce::String(ccNumber), juce::dontSendNotification);
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
						// Normal MIDI Control Mode - ERWEITERT für Pitch
						float normalizedValue = ccValue / 127.0f;

						if (ccNumber == leftVolumeCC && leftVolumeCC != -1 && ccChannel == leftVolumeChannel && leftVolumeCC != -1)
						{
							leftSlider->setValue(normalizedValue, juce::sendNotificationSync);
						}
						else if (ccNumber == rightVolumeCC && rightVolumeCC != -1 && ccChannel == rightVolumeChannel && rightVolumeChannel != -1)
						{
							rightSlider->setValue(normalizedValue, juce::sendNotificationSync);
						}
						else if (ccNumber == leftPitchCC && leftPitchCC != -1 && ccChannel == leftPitchChannel && leftPitchChannel != -1)
						{
							// Pitch range: -1 to +1 (center = 0)
							float pitchValue = (normalizedValue * 2.0f) - 1.0f;
							leftPitchSlider->setValue(pitchValue, juce::sendNotificationSync);
						}
						else if (ccNumber == rightPitchCC && rightPitchCC != -1 && ccChannel == rightPitchChannel && rightPitchChannel != -1)
						{
							float pitchValue = (normalizedValue * 2.0f) - 1.0f;
							rightPitchSlider->setValue(pitchValue, juce::sendNotificationSync);
						}
						else if (ccNumber == crossfaderCC && crossfaderCC != -1)
						{
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
		leftPitchCC = -1;   // NEU
		rightPitchCC = -1;  // NEU
		crossfaderCC = -1;

		leftCCLabel->setText("Vol CC: -", juce::dontSendNotification);
		rightCCLabel->setText("Vol CC: -", juce::dontSendNotification);
		leftPitchCCLabel->setText("Pitch CC: -", juce::dontSendNotification);
		rightPitchCCLabel->setText("Pitch CC: -", juce::dontSendNotification);
		crossfaderCCLabel->setText("CC: -", juce::dontSendNotification);
	}

	// BPM und Sync Funktionen (unverändert)
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

			double pitchAdjust = (syncRatio - 1.0);
			leftPitchSlider->setValue(juce::jlimit(-0.08, 0.08, pitchAdjust), juce::sendNotificationSync);

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

			rightSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
			juce::Timer::callAfterDelay(2000, [this]() {
				rightSyncButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
				});
		}
	}

	// Audio-Funktionen (unverändert)
	double getLeftPitch() const { return 1.0 + leftPitchSlider->getValue(); }
	double getRightPitch() const { return 1.0 + rightPitchSlider->getValue(); }

	float getLeftChannelGain() {
		return (float)leftSlider->getValue();
	}

	float getRightChannelGain() {
		return (float)rightSlider->getValue();
	}

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

	float getLeftCueGain() {
		return (float)leftSlider->getValue();
	}

	float getRightCueGain() {
		return (float)rightSlider->getValue();
	}

	// Output Routing (unverändert)
	enum class OutputDestination {
		MasterLeft,
		MasterRight,
		CueLeft,
		CueRight,
		Muted
	};

	bool isLeftChannelRoutedToMaster() const {
		int selectedOutput = leftOutputCombo->getSelectedId();
		return selectedOutput == 1 || selectedOutput == 2;
	}

	bool isRightChannelRoutedToMaster() const {
		int selectedOutput = rightOutputCombo->getSelectedId();
		return selectedOutput == 1 || selectedOutput == 3;
	}

	bool isLeftChannelRoutedToCue() const {
		return leftOutputCombo->getSelectedId() == 4;
	}

	bool isRightChannelRoutedToCue() const {
		return rightOutputCombo->getSelectedId() == 4;
	}

	OutputDestination getLeftChannelDestination() const {
		switch (leftOutputCombo->getSelectedId()) {
		case 1: return OutputDestination::MasterLeft;
		case 2: return OutputDestination::MasterLeft;
		case 3: return OutputDestination::MasterRight;
		case 4: return OutputDestination::CueLeft;
		case 5: return OutputDestination::Muted;
		default: return OutputDestination::MasterLeft;
		}
	}

	OutputDestination getRightChannelDestination() const {
		switch (rightOutputCombo->getSelectedId()) {
		case 1: return OutputDestination::MasterRight;
		case 2: return OutputDestination::MasterLeft;
		case 3: return OutputDestination::MasterRight;
		case 4: return OutputDestination::CueRight;
		case 5: return OutputDestination::Muted;
		default: return OutputDestination::MasterRight;
		}
	}

	float getCrossfaderValue() {
		return (float)crossfader->getValue();
	}

	// Getter für ComboBoxes
	juce::ComboBox* getLeftOutputCombo() const { return leftOutputCombo.get(); }
	juce::ComboBox* getRightOutputCombo() const { return rightOutputCombo.get(); }

	// ERWEITERTE Getter für MIDI Mappings
	int getLeftVolumeCC() const { return leftVolumeCC; }
	int getRightVolumeCC() const { return rightVolumeCC; }
	int getLeftPitchCC() const { return leftPitchCC; }      // NEU
	int getRightPitchCC() const { return rightPitchCC; }    // NEU
	int getCrossfaderCC() const { return crossfaderCC; }

	// ERWEITERTE Setter für MIDI Mappings
	void setLeftVolumeCC(int cc) {
		leftVolumeCC = cc;
		leftCCLabel->setText(cc >= 0 ? "Vol CC: " + juce::String(cc) : "Vol CC: -", juce::dontSendNotification);
	}
	void setRightVolumeCC(int cc) {
		rightVolumeCC = cc;
		rightCCLabel->setText(cc >= 0 ? "Vol CC: " + juce::String(cc) : "Vol CC: -", juce::dontSendNotification);
	}
	void setLeftPitchCC(int cc) {                          // NEU
		leftPitchCC = cc;
		leftPitchCCLabel->setText(cc >= 0 ? "Pitch CC: " + juce::String(cc) : "Pitch CC: -", juce::dontSendNotification);
	}
	void setRightPitchCC(int cc) {                         // NEU
		rightPitchCC = cc;
		rightPitchCCLabel->setText(cc >= 0 ? "Pitch CC: " + juce::String(cc) : "Pitch CC: -", juce::dontSendNotification);
	}
	void setCrossfaderCC(int cc) {
		crossfaderCC = cc;
		crossfaderCCLabel->setText(cc >= 0 ? "CC: " + juce::String(cc) : "CC: -", juce::dontSendNotification);
	}

	void updateChannelLevels(int deck, float leftSample, float rightSample)
	{
		if (deck == 0 && leftLevelMeter) // Deck A
		{
			// True stereo metering - nutze nur den linken Kanal für linken Meter
			float level = std::abs(leftSample);
			leftLevelMeter->setLevel(level);
		}
		else if (deck == 1 && rightLevelMeter) // Deck B  
		{
			float level = std::abs(rightSample);
			rightLevelMeter->setLevel(level);
		}
	}

	// Separate Methode für echte Master-Levels
	void updateTrueMasterLevels(float finalMasterLeft, float finalMasterRight)
	{
		if (masterLevelMeterL)
		{
			masterLevelMeterL->setLevel(calculatePeakLevel(finalMasterLeft));
		}
		if (masterLevelMeterR)
		{
			masterLevelMeterR->setLevel(calculatePeakLevel(finalMasterRight));
		}
	}

private:
	float calculatePeakLevel(float sample)
	{
		float absSample = std::abs(sample);

		// Logarithmic scaling für bessere Sichtbarkeit
		if (absSample < 0.001f) return 0.0f;

		float dbValue = 20.0f * std::log10(absSample);
		// Map from -60dB to 0dB to 0.0 to 1.0
		float normalizedLevel = juce::jlimit(0.0f, 1.0f, (dbValue + 60.0f) / 60.0f);

		return normalizedLevel;
	}
private:
	// GUI Components (erweitert)
	std::unique_ptr<juce::Slider> leftSlider = nullptr;
	std::unique_ptr<juce::Slider> rightSlider = nullptr;
	std::unique_ptr<juce::Slider> crossfader = nullptr;

	std::unique_ptr<juce::ComboBox> leftOutputCombo = nullptr;
	std::unique_ptr<juce::ComboBox> rightOutputCombo = nullptr;

	// Volume MIDI Learn
	std::unique_ptr<juce::TextButton> leftLearnButton = nullptr;
	std::unique_ptr<juce::TextButton> rightLearnButton = nullptr;
	std::unique_ptr<juce::TextButton> crossfaderLearnButton = nullptr;

	// Pitch MIDI Learn - NEU
	std::unique_ptr<juce::TextButton> leftPitchLearnButton = nullptr;
	std::unique_ptr<juce::TextButton> rightPitchLearnButton = nullptr;

	std::unique_ptr<juce::Label> leftLabel = nullptr;
	std::unique_ptr<juce::Label> rightLabel = nullptr;
	std::unique_ptr<juce::Label> crossfaderLabel = nullptr;
	std::unique_ptr<juce::Label> leftOutputLabel = nullptr;
	std::unique_ptr<juce::Label> rightOutputLabel = nullptr;

	// Volume CC Labels
	std::unique_ptr<juce::Label> leftCCLabel = nullptr;
	std::unique_ptr<juce::Label> rightCCLabel = nullptr;
	std::unique_ptr<juce::Label> crossfaderCCLabel = nullptr;

	// Pitch CC Labels - NEU
	std::unique_ptr<juce::Label> leftPitchCCLabel = nullptr;
	std::unique_ptr<juce::Label> rightPitchCCLabel = nullptr;

	// Pitch Controls
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

	int leftVolumeCC = -1;
	int leftVolumeChannel = -1;
	int rightVolumeCC = -1;
	int rightVolumeChannel = -1;
	int leftPitchCC = -1;   
	int leftPitchChannel = -1;
	int rightPitchCC = -1;  
	int rightPitchChannel = -1;
	
	int crossfaderCC = -1;

	float lastLeftPeak = 0.0f;
	float lastRightPeak = 0.0f;
	float lastMasterPeakL = 0.0f;
	float lastMasterPeakR = 0.0f;

	// Level Meters
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
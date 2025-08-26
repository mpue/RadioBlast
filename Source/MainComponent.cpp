#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
	: TimeSliceThread("PropertyWatcher"),
	resizerBar(&stretchableManager, 1, true)
{
	djLookAndFeel = std::make_unique<DJLookAndFeel>();
	setLookAndFeel(djLookAndFeel.get());

	setSize(1280, 800);

	menuBar = std::make_unique<juce::MenuBarComponent>(this);
	addAndMakeVisible(menuBar.get());


	juce::File initialDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory).getFullPathName();

	// Left
	leftFilter = std::make_unique<juce::WildcardFileFilter>("*.wav;*.mp3;*.ogg,*.aiff", "*", "*");
	leftDirectoryContents = std::make_unique<juce::DirectoryContentsList>(leftFilter.get(), *this);
	leftDirectoryContents->setIgnoresHiddenFiles(false);
	leftModel = std::make_unique<FileBrowserModel>(leftDirectoryContents.get(), initialDir);
	leftFileBrowser = std::make_unique<ExtendedFileBrowser>(
		juce::File::getSpecialLocation(juce::File::userHomeDirectory),
		leftFilter.get(),
		leftModel.get(),
		true
	);
	leftDirectoryContents->addChangeListener(leftFileBrowser.get());

	// Right
	rightFilter = std::make_unique<juce::WildcardFileFilter>("*.wav;*.mp3;*.ogg,*.aiff", "*", "*");
	rightDirectoryContents = std::make_unique<juce::DirectoryContentsList>(rightFilter.get(), *this);
	rightDirectoryContents->setIgnoresHiddenFiles(false);
	rightModel = std::make_unique<FileBrowserModel>(rightDirectoryContents.get(), initialDir);
	rightFileBrowser = std::make_unique<ExtendedFileBrowser>(
		juce::File::getSpecialLocation(juce::File::userHomeDirectory),
		rightFilter.get(),
		rightModel.get(),
		false
	);
	rightDirectoryContents->addChangeListener(rightFileBrowser.get());

	mixer = std::make_unique<MixerComponent>();
	leftPlayList = std::make_unique<PlaylistComponent>();
	rightPlayList = std::make_unique<PlaylistComponent>();

	wave = std::make_unique<DualWaveformComponent>();

	// addAndMakeVisible(dock);
	// addAndMakeVisible(tabDock);
	addAndMakeVisible(advancedDock);

	// Add visible components
	advancedDock.addComponentToNewColumn(leftFileBrowser.get(), 0, 0);
	advancedDock.addComponentToNewColumn(mixer.get(), 0, 1, 200);
	advancedDock.addComponentToNewColumn(rightFileBrowser.get(), 0, 2);
	advancedDock.addComponentToNewRow(leftPlayList.get(), 1);
	advancedDock.addComponentToNewColumn(rightPlayList.get(), 1, 1);
	advancedDock.addComponentToNewRow(wave.get(), 2);

	juce::PropertiesFile::Options options;
	options.applicationName = "RadioBlast";
	options.filenameSuffix = ".settings";
	options.osxLibrarySubFolder = "Application Support";
	appProperties.setStorageParameters(options);

	// AudioDeviceManager initialisieren
	audioDeviceManager.initialise(2, 2, nullptr, true);

	// Gespeicherte Audio-Einstellungen laden
	createConfig();

	// Audio permissions
	if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
		&& !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
	{
		juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
			[&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
	}
	else
	{
		setAudioChannels(4, 8);
	}

	setupMidiInputs();
	startThread(); // file watcher


	if (leftFileBrowser)
	{
		leftFileBrowser->setTrackLoadedCallback([this](const juce::File& file, bool left) {
			onTrackLoaded(file, true); // true = left deck
			});
	}

	if (rightFileBrowser)
	{
		rightFileBrowser->setTrackLoadedCallback([this](const juce::File& file, bool left) {
			onTrackLoaded(file, false); // false = right deck
			});
	}


	leftPlayList->onFileSelected = [this](const juce::File& file) {
		// Datei in Audio-Engine laden und abspielen
		onTrackLoaded(file, true);
		leftFileBrowser->getSampler()->loadSample(file);
		leftFileBrowser->getSampler()->play();
		int deck = 0; // Left deck

		WaveformGenerator::generateWaveformDataAsync(file, 2000,
			[this, deck](WaveformGenerator::WaveformData data) {
				if (data.isValid)
				{
					wave->setWaveformData(deck, data.samples, data.duration);
				}
			});

		};

	leftPlayList->onPlaylistFinished = [this]() {

		};

	rightPlayList->onFileSelected = [this](const juce::File& file) {
		// Datei in Audio-Engine laden und abspielen
		onTrackLoaded(file, false);
		rightFileBrowser->getSampler()->loadSample(file);
		rightFileBrowser->getSampler()->play();
		int deck = 1; // right deck

		WaveformGenerator::generateWaveformDataAsync(file, 2000,
			[this, deck](WaveformGenerator::WaveformData data) {
				if (data.isValid)
				{
					wave->setWaveformData(deck, data.samples, data.duration);
				}
			});

		};


	wave->onScrubbed = [this](int deck, double position) {
		// Position des entsprechenden Samplers setzen
		if (deck == 0)  {
			leftFileBrowser->getSampler()->setCurrentPosition((long)(position * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()));
		}
		else if (deck == 1) {
			rightFileBrowser->getSampler()->setCurrentPosition((long)(position * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()));
		}
	};

	wave->onPositionClicked = [this](int deck, double position) {
		if (deck == 0) {
			leftFileBrowser->getSampler()->setCurrentPosition((long)(position * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()));
		}
		else if (deck == 1) {
			rightFileBrowser->getSampler()->setCurrentPosition((long)(position * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()));
		}
	};

	rightPlayList->onPlaylistFinished = [this]() {

		};

	resized();
}

MainComponent::~MainComponent()
{
	setLookAndFeel(nullptr);
	cleanupMidiInputs();
	shutdownAudio();
}

void MainComponent::setupMidiInputs()
{
	// Alle verfügbaren MIDI Input Devices finden
	auto midiInputs = juce::MidiInput::getAvailableDevices();

	for (auto& input : midiInputs)
	{
		if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
		{
			deviceManager.addMidiInputDeviceCallback(input.identifier, this);
		}
	}

	// Callback für Device Manager Changes
	deviceManager.addChangeListener(this);
}

void MainComponent::cleanupMidiInputs()
{
	// Alle MIDI Input Callbacks entfernen
	auto midiInputs = juce::MidiInput::getAvailableDevices();

	for (auto& input : midiInputs)
	{
		deviceManager.removeMidiInputDeviceCallback(input.identifier, this);
	}

	deviceManager.removeChangeListener(this);
}

// ChangeListener für Device Manager Changes
void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &deviceManager)
	{
		// MIDI Device Setup aktualisieren wenn sich Geräte ändern
		cleanupMidiInputs();
		setupMidiInputs();
	}

}

juce::StringArray MainComponent::getMenuBarNames()
{
	return { "File", "Audio", "Help" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
	juce::PopupMenu menu;

	if (topLevelMenuIndex == 0) // File Menu
	{
		menu.addItem(exit, "Exit", true);
	}
	else if (topLevelMenuIndex == 1) // Audio Menu
	{
		menu.addItem(audioSettings, "Audio Settings...", true);
		menu.addItem(midiSettings, "MIDI Settings...", true);
	}
	else if (topLevelMenuIndex == 2) // Help Menu
	{
		menu.addItem(about, "About...", true);
	}

	return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	switch (menuItemID)
	{
	case audioSettings:
		showAudioSettings();
		break;

	case midiSettings:
		showAudioSettings(); // MIDI Settings sind im Audio Settings Dialog integriert
		break;

	case about:
		showAbout();
		break;

	case exit:
		juce::JUCEApplication::getInstance()->systemRequestedQuit();
		break;

	default:
		break;
	}
}

void MainComponent::createConfig() {
	String userHome = File::getSpecialLocation(File::userHomeDirectory).getFullPathName();

	File appDir = File(userHome + "/.RadioBlast");

	if (!appDir.exists()) {
		appDir.createDirectory();
	}

	File configFile = File(userHome + "/.RadioBlast/config.xml");

	if (configFile.exists()) {
		std::unique_ptr<XmlElement> xml = XmlDocument(configFile).getDocumentElement();
		deviceManager.initialise(2, 2, xml.get(), true);
	}
}

void MainComponent::showAudioSettings()
{
	AudioDeviceSelectorComponent* selector = new AudioDeviceSelectorComponent(deviceManager, 2, 16, 2, 16, true, true, true, false);
	DialogWindow::LaunchOptions launchOptions;

	launchOptions.dialogTitle = ("Audio Settings");
	launchOptions.escapeKeyTriggersCloseButton = true;
	launchOptions.resizable = false;
	launchOptions.useNativeTitleBar = false;
	launchOptions.useBottomRightCornerResizer = true;
	launchOptions.componentToCentreAround = getParentComponent();
	launchOptions.content.setOwned(selector);
	launchOptions.content->setSize(600, 580);
	launchOptions.dialogBackgroundColour = Colour(0xff222222);

	DialogWindow* window = launchOptions.launchAsync();

	window->setLookAndFeel(djLookAndFeel.get());

	std::function<void(int)> lambda =
		[this, selector](int result) {
		AudioDeviceManager::AudioDeviceSetup setup;

		deviceManager.getAudioDeviceSetup(setup);
		deviceManager.restartLastAudioDevice();

		std::unique_ptr<XmlElement> config = deviceManager.createStateXml();

		String userHome = File::getSpecialLocation(File::userHomeDirectory).getFullPathName();

		File appDir = File(userHome + "/.RadioBlast");

		if (!appDir.exists()) {
			appDir.createDirectory();
		}

		File configFile = File(userHome + "/.RadioBlast/config.xml");

		if (config != nullptr) {
			config->writeToFile(configFile, "");
			config = nullptr;
		}

		delete selector;
		};

	ModalComponentManager::Callback* callback = ModalCallbackFunction::create(lambda);
	ModalComponentManager::getInstance()->attachCallback(window, callback);
}

void MainComponent::closeSettingsWindow()
{
	settingsWindow->setVisible(false);
}

void MainComponent::showAbout()
{
	juce::AlertWindow::showMessageBoxAsync(
		juce::AlertWindow::InfoIcon,
		"About",
		"DJ Mixer Application\n\n"
		"Version 1.0\n"
		"Built with JUCE Framework\n\n"
		"Features:\n"
		"• Dual deck audio playback\n"
		"• MIDI controllable mixer with crossfader\n"
		"• File browser and playlist management\n"
		"• MIDI Learn functionality",
		"OK"
	);
}


//==============================================================================
void MainComponent::prepareToPlay(int, double) {}
void MainComponent::releaseResources() {}

void MainComponent::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
	if (mixer)
	{
		mixer->handleMidiMessage(message);
	}

}

// MainComponent.cpp - erweiterte copyBufferFromSampler Methode
// Replace the getNextAudioBlock method in MainComponent.cpp with this updated version:

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	bufferToFill.clearActiveBufferRegion();

	Sampler* leftSampler = leftFileBrowser->getSampler();
	Sampler* rightSampler = rightFileBrowser->getSampler();

	// Pitch-Werte von Mixer holen
	double leftPitch = mixer->getLeftPitch();
	double rightPitch = mixer->getRightPitch();

	auto* writableBuffer = const_cast<juce::AudioBuffer<float>*>(bufferToFill.buffer);
	if (!writableBuffer) return;

	float** outputData = const_cast<float**>(writableBuffer->getArrayOfWritePointers());
	const int outNumChans = writableBuffer->getNumChannels();
	if (!outputData || outNumChans < 2) return;

	// Stereo Buffers für beide Sampler (mit Pitch-Shifting)
	std::vector<float> leftSamplerL(bufferToFill.numSamples, 0.0f);
	std::vector<float> leftSamplerR(bufferToFill.numSamples, 0.0f);
	std::vector<float> rightSamplerL(bufferToFill.numSamples, 0.0f);
	std::vector<float> rightSamplerR(bufferToFill.numSamples, 0.0f);

	// Level monitoring variables
	float leftChannelSum = 0.0f;
	float rightChannelSum = 0.0f;
	float masterLeftSum = 0.0f;
	float masterRightSum = 0.0f;

	// Generate sampler outputs
	if (leftSampler && leftSampler->isPlaying()) {
		generateSamplerOutputWithPitch(leftSampler, leftSamplerL, leftSamplerR,
			bufferToFill.numSamples, mixer->getLeftChannelGain(), leftPitch);
	}

	if (rightSampler && rightSampler->isPlaying()) {
		generateSamplerOutputWithPitch(rightSampler, rightSamplerL, rightSamplerR,
			bufferToFill.numSamples, mixer->getRightChannelGain(), rightPitch);
	}

	// Output Routing with level monitoring
	for (int j = 0; j < bufferToFill.numSamples; ++j) {

		// Calculate channel levels before routing (for deck meters)
		float leftChannelLevel = std::sqrt(leftSamplerL[j] * leftSamplerL[j] + leftSamplerR[j] * leftSamplerR[j]);
		float rightChannelLevel = std::sqrt(rightSamplerL[j] * rightSamplerL[j] + rightSamplerR[j] * rightSamplerR[j]);

		leftChannelSum += leftChannelLevel * leftChannelLevel;
		rightChannelSum += rightChannelLevel * rightChannelLevel;

		// === LEFT DECK ROUTING ===
		auto leftDest = mixer->getLeftChannelDestination();

		if (mixer->isLeftChannelRoutedToMaster()) {
			float masterGain = mixer->getLeftMasterGain() / mixer->getLeftChannelGain();

			switch (leftDest) {
			case MixerComponent::OutputDestination::MasterLeft:
				if (outNumChans > 0) {
					float outputL = leftSamplerL[j] * masterGain;
					float outputR = leftSamplerR[j] * masterGain;
					outputData[0][j] += outputL;
					if (outNumChans > 1) outputData[1][j] += outputR;

					// Track master levels
					masterLeftSum += outputL * outputL;
					masterRightSum += outputR * outputR;
				}
				break;
			case MixerComponent::OutputDestination::MasterRight:
				if (outNumChans > 1) {
					float monoOutput = (leftSamplerL[j] + leftSamplerR[j]) * masterGain * 0.5f;
					outputData[1][j] += monoOutput;
					masterRightSum += monoOutput * monoOutput;
				}
				break;
			}
		}

		if (mixer->isLeftChannelRoutedToCue()) {
			if (outNumChans > 2) outputData[2][j] += leftSamplerL[j];
			if (outNumChans > 3) outputData[3][j] += leftSamplerR[j];
		}

		// === RIGHT DECK ROUTING ===
		auto rightDest = mixer->getRightChannelDestination();

		if (mixer->isRightChannelRoutedToMaster()) {
			float masterGain = mixer->getRightMasterGain() / mixer->getRightChannelGain();

			switch (rightDest) {
			case MixerComponent::OutputDestination::MasterLeft:
				if (outNumChans > 0) {
					float monoOutput = (rightSamplerL[j] + rightSamplerR[j]) * masterGain * 0.5f;
					outputData[0][j] += monoOutput;
					masterLeftSum += monoOutput * monoOutput;
				}
				break;
			case MixerComponent::OutputDestination::MasterRight:
				if (outNumChans > 0) {
					float outputL = rightSamplerL[j] * masterGain;
					float outputR = rightSamplerR[j] * masterGain;
					outputData[0][j] += outputL;
					if (outNumChans > 1) outputData[1][j] += outputR;

					// Track master levels
					masterLeftSum += outputL * outputL;
					masterRightSum += outputR * outputR;
				}
				break;
			}
		}

		if (mixer->isRightChannelRoutedToCue()) {
			if (outNumChans > 2) outputData[2][j] += rightSamplerL[j];
			if (outNumChans > 3) outputData[3][j] += rightSamplerR[j];
		}
	}

	// Calculate RMS levels for meters
	if (bufferToFill.numSamples > 0)
	{
		leftChannelRMS = std::sqrt(leftChannelSum / bufferToFill.numSamples);
		rightChannelRMS = std::sqrt(rightChannelSum / bufferToFill.numSamples);
		masterLeftRMS = std::sqrt(masterLeftSum / bufferToFill.numSamples);
		masterRightRMS = std::sqrt(masterRightSum / bufferToFill.numSamples);

		// Update level meters on message thread
		juce::MessageManager::callAsync([this]()
			{
				if (mixer)
				{
					mixer->updateLeftChannelLevel(leftChannelRMS, leftChannelRMS);
					mixer->updateRightChannelLevel(rightChannelRMS, rightChannelRMS);
					mixer->updateMasterLevels(masterLeftRMS, masterRightRMS);
				}
			});
	}
}
// Hilfsfunktion für Pitch-Shifting
void MainComponent::generateSamplerOutputWithPitch(Sampler* sampler,
	std::vector<float>& outputL,
	std::vector<float>& outputR,
	int numSamples, float gain, double pitch)
{
	static double phase = 0.0;
	static float prevSampleL = 0.0f;
	static float prevSampleR = 0.0f;
	static float currentSampleL = 0.0f;
	static float currentSampleR = 0.0f;
	static bool needNewSample = true;

	for (int i = 0; i < numSamples; ++i) {
		if (std::abs(pitch - 1.0) < 0.001) {
			// Kein Pitch-Shifting - normaler Durchlauf
			outputL[i] = sampler->getOutput(0) * gain;
			outputR[i] = sampler->getOutput(1) * gain;
			sampler->nextSample();
		}
		else {
			// Pitch-Shifting mit korrekter Interpolation

			// Neue Samples holen wenn nötig
			while (phase >= 1.0) {
				prevSampleL = currentSampleL;
				prevSampleR = currentSampleR;
				currentSampleL = sampler->getOutput(0);
				currentSampleR = sampler->getOutput(1);
				sampler->nextSample();
				phase -= 1.0;
				needNewSample = false;
			}

			// Beim ersten Mal oder nach Reset
			if (needNewSample) {
				currentSampleL = sampler->getOutput(0);
				currentSampleR = sampler->getOutput(1);
				prevSampleL = currentSampleL;
				prevSampleR = currentSampleR;
				needNewSample = false;
			}

			// Linear interpolation zwischen den Samples
			float fracPart = static_cast<float>(phase);
			outputL[i] = (prevSampleL + fracPart * (currentSampleL - prevSampleL)) * gain;
			outputR[i] = (prevSampleR + fracPart * (currentSampleR - prevSampleR)) * gain;

			// Phase für nächstes Sample erhöhen
			phase += pitch;
		}
	}
}

// Erweiterte Header-Deklaration in MainComponent.h

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0xff333333));
}

void MainComponent::resized()
{
	auto area = getLocalBounds();

	// Platz für Menüleiste reservieren
	if (menuBar)
	{
		auto menuArea = area.removeFromTop(24); // Menü ist typisch 24px hoch
		menuBar->setBounds(menuArea);
	}

	// dock.setBounds(area.reduced(4));
	// tabDock.setBounds(area.reduced(4));
	advancedDock.setBounds(area.reduced(4));
}

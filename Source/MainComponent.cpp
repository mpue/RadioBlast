#include "MainComponent.h"
#include "FXComponent.h"


//==============================================================================
MainComponent::MainComponent()
	: TimeSliceThread("PropertyWatcher"),
	resizerBar(&stretchableManager, 1, true), layoutManager(advancedDock, "defaultLayout")
{
	djLookAndFeel = std::make_unique<CyberpunkDJLookAndFeel>();
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
	mixer->setSizeDisplayEnabled(true);
	leftPlayList = std::make_unique<PlaylistComponent>();
	rightPlayList = std::make_unique<PlaylistComponent>();

	wave = std::make_unique<DualWaveformComponent>();
	samplePlayer = std::make_unique<SamplePlayer>();

	fxComponent = std::make_unique<AdvancedFXComponent>();
	addAndMakeVisible(advancedDock);

	fxComponent->chorusBypass.setToggleState(true, juce::sendNotification);
	fxComponent->reverbBypass.setToggleState(true, juce::sendNotification);
	fxComponent->filterBypass.setToggleState(true, juce::sendNotification);
	fxComponent->eqBypass.setToggleState(true, juce::sendNotification);

	midiMonitor = std::make_unique<MidiMonitorComponent>();
	stutterEffect = std::make_unique<StutterEffectComponent>();

	// AudioDeviceManager initialisieren
	// Add visible components
	int width = getLocalBounds().getWidth();
	advancedDock.addComponentToNewColumn(leftFileBrowser.get(), 0, 0);
	advancedDock.addComponentToNewColumn(mixer.get(), 0, 1, 570);
	advancedDock.addComponentToNewColumn(samplePlayer.get(), 0, 2, 300);
	advancedDock.addComponentToNewColumn(rightFileBrowser.get(), 0, 3);

	advancedDock.addComponentToNewRow(leftPlayList.get(), 1);
	advancedDock.addComponentToNewColumn(fxComponent.get(),  1, 1, width / 4);
	advancedDock.addComponentToNewColumn(rightPlayList.get(),1, 2,  width / 4);
	advancedDock.addComponentToNewColumn(midiMonitor.get(), 1, 3, width / 4);
	advancedDock.addComponentToNewColumn(stutterEffect.get(), 1, 3, width / 4);

	leftFileBrowser->setName("Browser A");
	rightFileBrowser->setName("Browser B");
	mixer->setName("Mixer");
	leftPlayList->setName("Playlist A");
	rightPlayList->setName("Playlist B");
	samplePlayer->setName("Sample Player");
	fxComponent->setName("FX");
	midiMonitor->setName("MIDI Monitor");
	stutterEffect->setName("Stutter FX");

	advancedDock.addComponentToNewRow(wave.get(), 2);

	juce::PropertiesFile::Options options;
	options.applicationName = "RadioBlast";
	options.filenameSuffix = ".settings";
	options.osxLibrarySubFolder = "Application Support";
	appProperties.setStorageParameters(options);

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
					wave->setWaveformData(deck, data);
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
					wave->setWaveformData(deck, data);
				}
			});

		};


	wave->onPositionChanged = [this](int deck, double pos, bool playing) {
		if (deck == 0)  {
			leftFileBrowser->getSampler()->setCurrentPosition((long)(pos * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()));
		}
		else if (deck == 1) {
			rightFileBrowser->getSampler()->setCurrentPosition((long)(pos * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate()));
		}
	};

	wave->onScrubStart = [this](int deck, double pos) {
		if (deck == 0) {
		
		}
		else if (deck == 1) {
		
		}
	};

	wave->onScrubEnd = [this](int deck, double pos) {
		if (deck == 0) {
		

		}
		else if (deck == 1) {
		
		}
	};

	rightPlayList->onPlaylistFinished = [this]() {

	};

	updateFXParameters();

	resized();

	layoutManager.loadLayoutOnStartup();
}

MainComponent::~MainComponent()
{
	layoutManager.saveLayoutOnShutdown();
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
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	currentSampleRate = sampleRate;

	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = samplesPerBlockExpected;
	spec.numChannels = 2; // Stereo

	masterFX.prepare(spec);

	// Prepare stutter effect
	stutterEffect->prepareToPlay(sampleRate,samplesPerBlockExpected);
}
void MainComponent::releaseResources() {}

void MainComponent::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
	if (mixer)
	{
		mixer->handleMidiMessage(message);
	}
	if (midiMonitor) {
		midiMonitor->addMidiEvent(message);
	}

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	bufferToFill.clearActiveBufferRegion();

	// === FX PARAMETER UPDATE ===
	updateFXParameters();

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

	// Sample Player Buffers
	std::vector<float> samplePlayerL(bufferToFill.numSamples, 0.0f);
	std::vector<float> samplePlayerR(bufferToFill.numSamples, 0.0f);

	// === NEU: Master Mix Buffers für FX Processing ===
	std::vector<float> masterMixL(bufferToFill.numSamples, 0.0f);
	std::vector<float> masterMixR(bufferToFill.numSamples, 0.0f);

	// Level monitoring variables
	float leftChannelSum = 0.0f;
	float rightChannelSum = 0.0f;
	float masterLeftSum = 0.0f;
	float masterRightSum = 0.0f;
	float samplePlayerSum = 0.0f;

	// Generate sampler outputs
	if (leftSampler && leftSampler->isPlaying()) {
		generateSamplerOutputWithPitch(leftSampler, leftSamplerL, leftSamplerR,
			bufferToFill.numSamples, mixer->getLeftChannelGain(), leftPitch);
	}

	if (rightSampler && rightSampler->isPlaying()) {
		generateSamplerOutputWithPitch(rightSampler, rightSamplerL, rightSamplerR,
			bufferToFill.numSamples, mixer->getRightChannelGain(), rightPitch);
	}

	// Sample Player Output generieren
	if (samplePlayer && samplePlayer->isAnySamplePlaying()) {
		samplePlayer->generateSampleOutput(samplePlayerL, samplePlayerR,
			bufferToFill.numSamples, 1.0f);
	}

	// === AUDIO ROUTING ZUM MASTER MIX ===
	for (int j = 0; j < bufferToFill.numSamples; ++j) {

		// Calculate channel levels before routing (for deck meters)
		float leftChannelLevel = std::sqrt(leftSamplerL[j] * leftSamplerL[j] + leftSamplerR[j] * leftSamplerR[j]);
		float rightChannelLevel = std::sqrt(rightSamplerL[j] * rightSamplerL[j] + rightSamplerR[j] * rightSamplerR[j]);
		float samplePlayerLevel = std::sqrt(samplePlayerL[j] * samplePlayerL[j] + samplePlayerR[j] * samplePlayerR[j]);

		leftChannelSum += leftChannelLevel * leftChannelLevel;
		rightChannelSum += rightChannelLevel * rightChannelLevel;
		samplePlayerSum += samplePlayerLevel * samplePlayerLevel;

		// === LEFT DECK ROUTING ZUM MASTER MIX ===
		auto leftDest = mixer->getLeftChannelDestination();

		if (mixer->isLeftChannelRoutedToMaster()) {
			float masterGain = mixer->getLeftMasterGain();

			switch (leftDest) {
			case MixerComponent::OutputDestination::MasterLeft:
				masterMixL[j] += leftSamplerL[j] * masterGain;
				masterMixR[j] += leftSamplerR[j] * masterGain;
				break;
			case MixerComponent::OutputDestination::MasterRight:
			{
				float monoOutput = (leftSamplerL[j] + leftSamplerR[j]) * masterGain * 0.5f;
				masterMixR[j] += monoOutput;
			}
			break;
			}
		}

		// === RIGHT DECK ROUTING ZUM MASTER MIX ===
		auto rightDest = mixer->getRightChannelDestination();

		if (mixer->isRightChannelRoutedToMaster()) {
			float masterGain = mixer->getRightMasterGain();

			switch (rightDest) {
			case MixerComponent::OutputDestination::MasterLeft:
			{
				float monoOutput = (rightSamplerL[j] + rightSamplerR[j]) * masterGain * 0.5f;
				masterMixL[j] += monoOutput;
			}
			break;
			case MixerComponent::OutputDestination::MasterRight:
				masterMixL[j] += rightSamplerL[j] * masterGain;
				masterMixR[j] += rightSamplerR[j] * masterGain;
				break;
			}
		}

		// === Sample Player zum Master Mix hinzufügen ===
		masterMixL[j] += samplePlayerL[j];
		masterMixR[j] += samplePlayerR[j];

		// === CUE ROUTING (bypassed FX) ===
		if (mixer->isLeftChannelRoutedToCue()) {
			if (outNumChans > 2) outputData[2][j] += leftSamplerL[j];
			if (outNumChans > 3) outputData[3][j] += leftSamplerR[j];
		}

		if (mixer->isRightChannelRoutedToCue()) {
			if (outNumChans > 2) outputData[2][j] += rightSamplerL[j];
			if (outNumChans > 3) outputData[3][j] += rightSamplerR[j];
		}
	}

	// === 🎛️ MASTER FX PROCESSING - HIER PASSIERT DIE MAGIE! ===
	processFXChain(masterFX, masterMixL, masterMixR, bufferToFill.numSamples);

	juce::AudioBuffer<float> tempBuffer(2, bufferToFill.numSamples);

	// Kopiere Master Mix in temporären Buffer
	for (int i = 0; i < bufferToFill.numSamples; ++i) {
		tempBuffer.setSample(0, i, masterMixL[i]);
		tempBuffer.setSample(1, i, masterMixR[i]);
	}

	// Wende Stutter-Effekt direkt auf den temporären Buffer an
	if (stutterEffect) {
		stutterEffect->processAudioBuffer(tempBuffer);
	}

	// === Final Output zu Hardware (mit Stutter-Effekt) ===
	for (int j = 0; j < bufferToFill.numSamples; ++j) {
		if (outNumChans > 0) {
			outputData[0][j] = tempBuffer.getSample(0, j);
			masterLeftSum += tempBuffer.getSample(0, j) * tempBuffer.getSample(0, j);
		}
		if (outNumChans > 1) {
			outputData[1][j] = tempBuffer.getSample(1, j);
			masterRightSum += tempBuffer.getSample(1, j) * tempBuffer.getSample(1, j);
		}
	}

	// === LEVEL METER UPDATES ===
	if (bufferToFill.numSamples > 0)
	{
		leftChannelRMS = std::sqrt(leftChannelSum / bufferToFill.numSamples);
		rightChannelRMS = std::sqrt(rightChannelSum / bufferToFill.numSamples);
		masterLeftRMS = std::sqrt(masterLeftSum / bufferToFill.numSamples);
		masterRightRMS = std::sqrt(masterRightSum / bufferToFill.numSamples);
		samplePlayerRMS = std::sqrt(samplePlayerSum / bufferToFill.numSamples);

		// === FX COMPONENT LEVEL UPDATES ===
		juce::AudioBuffer<float> levelBuffer(2, bufferToFill.numSamples);
		for (int i = 0; i < bufferToFill.numSamples; ++i)
		{
			levelBuffer.setSample(0, i, tempBuffer.getSample(0, i));
			levelBuffer.setSample(1, i, tempBuffer.getSample(1, i));
		}

		// === NEU: Level Updates an Mixer weiterleiten ===
		juce::MessageManager::callAsync([this, leftChannelRMS = leftChannelRMS, rightChannelRMS = rightChannelRMS,
			masterLeftRMS = masterLeftRMS, masterRightRMS = masterRightRMS,
			samplePlayerRMS = samplePlayerRMS]() mutable
			{
				// Update bestehende Mixer Level Meters
				if (mixer)
				{
					mixer->updateChannelLevels(0, leftChannelRMS, rightChannelRMS);   // Left deck
					mixer->updateChannelLevels(1, leftChannelRMS, rightChannelRMS);   // Right deck (beide gleich für jetzt)
					mixer->updateTrueMasterLevels(masterLeftRMS, masterRightRMS);     // Master output levels
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


void MainComponent::updateFXParameters()
{
	// Parameter direkt von UI-Komponenten lesen (einfacher Weg ohne AudioProcessorValueTreeState)
	updateFilterParameters(masterFX);
	updateEQParameters(masterFX);
	updateChorusParameters(masterFX);
	updateReverbParameters(masterFX);

	// Master Volume direkt vom Knob lesen
	if (fxComponent)
		masterFX.masterVolume = (float)fxComponent->masterVolumeKnob.getValue();
}

void MainComponent::updateFilterParameters(FXChain& fx, const juce::String& prefix)
{
	if (!fxComponent) return;

	auto cutoff = (float)fxComponent->filterSection.cutoffKnob.getValue();
	auto resonance = (float)fxComponent->filterSection.resonanceKnob.getValue();
	auto drive = (float)fxComponent->filterSection.driveKnob.getValue();
	auto type = fxComponent->filterSection.typeCombo.getSelectedId() - 1; // ComboBox IDs starten bei 1
	fx.filterBypass = fxComponent->filterBypass.getToggleState();

	for (auto& filter : fx.filters)
	{
		filter.setCutoffFrequency(cutoff);
		filter.setResonance(resonance);

		switch (type)
		{
		case 0: filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); break;
		case 1: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
		case 2: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
		// case 3: filter.setType(juce::dsp::StateVariableTPTFilterType::notch); break;
		default: filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); break;
		}
	}
}

void MainComponent::updateEQParameters(FXChain& fx, const juce::String& prefix)
{
	if (!fxComponent) return;

	auto lowGain = juce::Decibels::decibelsToGain((float)fxComponent->eqSection.lowKnob.getValue());
	auto midGain = juce::Decibels::decibelsToGain((float)fxComponent->eqSection.midKnob.getValue());
	auto highGain = juce::Decibels::decibelsToGain((float)fxComponent->eqSection.highKnob.getValue());
	auto lowFreq = (float)fxComponent->eqSection.lowFreqKnob.getValue();
	auto highFreq = (float)fxComponent->eqSection.highFreqKnob.getValue();
	fx.eqBypass = fxComponent->eqBypass.getToggleState();

	// Stereo EQ - beide Kanäle gleich einstellen
	for (int i = 0; i < 2; ++i)
	{
		fx.lowShelfFilter[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
			currentSampleRate, lowFreq, 0.707f, lowGain);
		fx.peakingFilter[i].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
			currentSampleRate, 1000.0f, 1.0f, midGain);
		fx.highShelfFilter[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
			currentSampleRate, highFreq, 0.707f, highGain);
	}
}

void MainComponent::updateChorusParameters(FXChain& fx, const juce::String& prefix)
{
	if (!fxComponent) return;

	auto rate = (float)fxComponent->chorusSection.rateKnob.getValue();
	auto depth = (float)fxComponent->chorusSection.depthKnob.getValue();
	auto feedback = (float)fxComponent->chorusSection.feedbackKnob.getValue();
	auto mix = (float)fxComponent->chorusSection.mixKnob.getValue();
	fx.chorusBypass = fxComponent->chorusBypass.getToggleState();

	fx.chorus.setRate(rate);
	fx.chorus.setDepth(depth);
	fx.chorus.setFeedback(feedback);
	fx.chorus.setMix(mix);
	fx.chorus.setCentreDelay(7.0f);
}

void MainComponent::updateReverbParameters(FXChain& fx, const juce::String& prefix)
{
	if (!fxComponent) return;

	juce::dsp::Reverb::Parameters reverbParams;
	reverbParams.roomSize = (float)fxComponent->reverbSection.roomSizeKnob.getValue();
	reverbParams.damping = (float)fxComponent->reverbSection.dampingKnob.getValue();
	reverbParams.wetLevel = (float)fxComponent->reverbSection.wetKnob.getValue();
	reverbParams.dryLevel = (float)fxComponent->reverbSection.dryKnob.getValue();
	reverbParams.width = (float)fxComponent->reverbSection.widthKnob.getValue();
	reverbParams.freezeMode = 0.0f;

	fx.reverbBypass = fxComponent->reverbBypass.getToggleState();
	fx.reverb.setParameters(reverbParams);
}

void MainComponent::processFXChain(FXChain& fx, std::vector<float>& leftChannel, std::vector<float>& rightChannel, int numSamples)
{
	// Create audio buffer from vectors
	juce::AudioBuffer<float> buffer(2, numSamples);

	// Copy input data
	for (int i = 0; i < numSamples; ++i)
	{
		buffer.setSample(0, i, leftChannel[i]);
		buffer.setSample(1, i, rightChannel[i]);
	}

	// Process FX Chain
	juce::dsp::AudioBlock<float> block(buffer);
	juce::dsp::ProcessContextReplacing<float> context(block);

	// Filter (bereits Stereo-fähig)
	if (!fx.filterBypass)
	{
		for (auto& filter : fx.filters)
			filter.process(context);
	}

	// EQ - Stereo Processing
	if (!fx.eqBypass)
	{
		// Separate Channels für EQ
		for (int channel = 0; channel < 2; ++channel)
		{
			for (int sample = 0; sample < numSamples; ++sample)
			{
				float input = buffer.getSample(channel, sample);

				// Chain: Low → Peak → High
				float output = fx.lowShelfFilter[channel].processSample(input);
				output = fx.peakingFilter[channel].processSample(output);
				output = fx.highShelfFilter[channel].processSample(output);

				buffer.setSample(channel, sample, output);
			}
		}
	}

	// Chorus (bereits Stereo-fähig)
	if (!fx.chorusBypass)
		fx.chorus.process(context);

	// Reverb (bereits Stereo-fähig)
	if (!fx.reverbBypass)
		fx.reverb.process(context);

	// Master Volume
	auto masterGain = juce::Decibels::decibelsToGain(fx.masterVolume);
	if (masterGain != 1.0f)
		block.multiplyBy(masterGain);

	// Copy back to vectors
	for (int i = 0; i < numSamples; ++i)
	{
		leftChannel[i] = buffer.getSample(0, i);
		rightChannel[i] = buffer.getSample(1, i);
	}
}
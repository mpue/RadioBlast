#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : TimeSliceThread("PropertyWatcher"),
    resizerBar(&stretchableManager, 1, true)
{
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

    // addAndMakeVisible(dock);
    // addAndMakeVisible(tabDock);
    addAndMakeVisible(advancedDock);

    // Add visible components
    advancedDock.addComponentToNewColumn(leftFileBrowser.get(),0,0);
    advancedDock.addComponentToNewColumn(mixer.get(),0,1, 100);
    advancedDock.addComponentToNewColumn(rightFileBrowser.get(), 0, 2);
    advancedDock.addComponentToNewRow(leftPlayList.get(), 1);
    advancedDock.addComponentToNewColumn(rightPlayList.get(), 1,1);

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

    resized();
}

MainComponent::~MainComponent()
{
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
    else
    {
        // Bestehende ChangeListener Logik für File Browser etc.
        // ... dein bestehender changeListenerCallback Code ...
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

void MainComponent::showAudioSettings()
{
    if (settingsWindow != nullptr)
    {
        settingsWindow->toFront(true);        
    }

    // Audio Device Selector erstellen
    audioSettingsComponent = std::make_unique<juce::AudioDeviceSelectorComponent>(
        deviceManager,
        0, 2,    // Minimum/Maximum Input Channels
        2, 8,    // Minimum/Maximum Output Channels
        true,   // Show MIDI Input Options
        true,    // Show MIDI Output Options  
        true,   // Show Channels as Stereo Pairs
        false    // Hide Advanced Options
    );

    // Custom DialogWindow Klasse für Close-Handling
    class SettingsDialogWindow : public juce::DialogWindow
    {
    public:
        SettingsDialogWindow(const juce::String& name, juce::Colour backgroundColour, bool escapeKeyTriggersCloseButton, bool addToDesktop, MainComponent* parent)
            : DialogWindow(name, backgroundColour, escapeKeyTriggersCloseButton, addToDesktop), parentComponent(parent)
        {
            setSize(800, 600);
        }

        void closeButtonPressed() override
        {
            if (parentComponent)
                parentComponent->closeSettingsWindow();
        }

    private:
        MainComponent* parentComponent;
    };

    if (settingsWindow == nullptr) {
        // Dialog Window erstellen
        settingsWindow = std::make_unique<SettingsDialogWindow>("Audio Settings",
            juce::Colours::darkgrey,
            true,
            true,
            this);

    }
		
    settingsWindow->setContentOwned(audioSettingsComponent.release(), true);
	settingsWindow->setSize(600, 400);
    settingsWindow->setResizable(true, false);
    settingsWindow->setUsingNativeTitleBar(false);
    settingsWindow->centreAroundComponent(this, settingsWindow->getWidth(), settingsWindow->getHeight());
    settingsWindow->setVisible(true);
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
void MainComponent::copyBufferFromSampler(const juce::AudioSourceChannelInfo& bufferToFill, Sampler* sampler, bool& retFlag, float gain, double pitchRatio)
{
    retFlag = true;
    auto* writableBuffer = const_cast<juce::AudioBuffer<float>*>(bufferToFill.buffer);
    if (!writableBuffer) return;

    float** outputData = const_cast<float**>(writableBuffer->getArrayOfWritePointers());
    const int outNumChans = writableBuffer->getNumChannels();
    if (!outputData || outNumChans == 0) return;

    if (sampler && sampler->isPlaying()) {
        // Pitch-Shifting durch Sample-Rate Manipulation
        // Einfache Implementierung - für bessere Qualität würde man einen echten Pitch-Shifter nutzen

        if (std::abs(pitchRatio - 1.0) < 0.001) {
            // Kein Pitch-Shifting nötig
            for (int j = 0; j < bufferToFill.numSamples; ++j) {
                float sL = sampler->getOutput(0) * gain;
                float sR = sampler->getOutput(1) * gain;
                if (outNumChans > 0 && outputData[0]) outputData[0][j] += sL;
                if (outNumChans > 1 && outputData[1]) outputData[1][j] += sR;
                sampler->nextSample();
            }
        }
        else {
            // Pitch-Shifting durch Sample-Rate Änderung
            static double leftPhase = 0.0;
            static double rightPhase = 0.0;

            for (int j = 0; j < bufferToFill.numSamples; ++j) {
                // Linear interpolation für Pitch-Shifting
                double intPart;
                double fracPart = std::modf(leftPhase, &intPart);

                // Aktuelle und nächste Samples holen
                float sL_curr = sampler->getOutput(0) * gain;
                float sR_curr = sampler->getOutput(1) * gain;

                sampler->nextSample();

                float sL_next = sampler->getOutput(0) * gain;
                float sR_next = sampler->getOutput(1) * gain;

                // Linear interpolation
                float sL = sL_curr + fracPart * (sL_next - sL_curr);
                float sR = sR_curr + fracPart * (sR_next - sR_curr);

                if (outNumChans > 0 && outputData[0]) outputData[0][j] += sL;
                if (outNumChans > 1 && outputData[1]) outputData[1][j] += sR;

                // Phase für nächstes Sample
                leftPhase += pitchRatio;
                rightPhase += pitchRatio;

                // Phase-Wrap verhindern
                if (leftPhase >= 2.0) {
                    leftPhase -= 1.0;
                }
            }
        }
    }

    retFlag = false;
}

// Erweiterte getNextAudioBlock mit Pitch-Shifting
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    Sampler* leftSampler = leftFileBrowser->getSampler();
    Sampler* rightSampler = rightFileBrowser->getSampler();

    // Pitch-Werte von Mixer holen
    double leftPitch = mixer->getLeftPitch();   // 1.0 = normal, 1.05 = +5%, 0.95 = -5%
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

    // Left Sampler mit Pitch-Shifting
    if (leftSampler && leftSampler->isPlaying()) {
        generateSamplerOutputWithPitch(leftSampler, leftSamplerL, leftSamplerR,
            bufferToFill.numSamples, mixer->getLeftGain(), leftPitch);
    }

    // Right Sampler mit Pitch-Shifting
    if (rightSampler && rightSampler->isPlaying()) {
        generateSamplerOutputWithPitch(rightSampler, rightSamplerL, rightSamplerR,
            bufferToFill.numSamples, mixer->getRightGain(), rightPitch);
    }

    // Output Routing (wie gehabt)
    for (int j = 0; j < bufferToFill.numSamples; ++j) {
        // Left Deck Routing
        auto leftDest = mixer->getLeftChannelDestination();
        switch (leftDest) {
        case MixerComponent::OutputDestination::CueLeft:
        case MixerComponent::OutputDestination::CueRight:
            if (outNumChans > 2) outputData[2][j] += leftSamplerL[j];
            if (outNumChans > 3) outputData[3][j] += leftSamplerR[j];
            break;
        case MixerComponent::OutputDestination::Muted:
            break;
        default:
            // Master Output Handling
            if (mixer->getLeftOutputCombo()->getSelectedId() == 1) { // Master L+R
                if (outNumChans > 0) outputData[0][j] += leftSamplerL[j];
                if (outNumChans > 1) outputData[1][j] += leftSamplerR[j];
            }
            break;
        }

        // Right Deck Routing
        auto rightDest = mixer->getRightChannelDestination();
        switch (rightDest) {
        case MixerComponent::OutputDestination::CueLeft:
        case MixerComponent::OutputDestination::CueRight:
            if (outNumChans > 2) outputData[2][j] += rightSamplerL[j];
            if (outNumChans > 3) outputData[3][j] += rightSamplerR[j];
            break;
        case MixerComponent::OutputDestination::Muted:
            break;
        default:
            if (mixer->getRightOutputCombo()->getSelectedId() == 1) { // Master L+R
                if (outNumChans > 0) outputData[0][j] += rightSamplerL[j];
                if (outNumChans > 1) outputData[1][j] += rightSamplerR[j];
            }
            break;
        }
    }
}

// Hilfsfunktion für Pitch-Shifting
void MainComponent::generateSamplerOutputWithPitch(Sampler* sampler,
    std::vector<float>& outputL,
    std::vector<float>& outputR,
    int numSamples, float gain, double pitch)
{
    static double phase = 0.0;

    for (int i = 0; i < numSamples; ++i) {
        if (std::abs(pitch - 1.0) < 0.001) {
            // Kein Pitch-Shifting
            outputL[i] = sampler->getOutput(0) * gain;
            outputR[i] = sampler->getOutput(1) * gain;
            sampler->nextSample();
        }
        else {
            // Simple Pitch-Shifting durch Sample-Interpolation
            double intPart;
            double fracPart = std::modf(phase, &intPart);

            float sL1 = sampler->getOutput(0) * gain;
            float sR1 = sampler->getOutput(1) * gain;

            sampler->nextSample();

            float sL2 = sampler->getOutput(0) * gain;
            float sR2 = sampler->getOutput(1) * gain;

            // Linear interpolation
            outputL[i] = sL1 + fracPart * (sL2 - sL1);
            outputR[i] = sR1 + fracPart * (sR2 - sR1);

            phase += pitch;
            if (phase >= 2.0) phase -= 1.0;
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

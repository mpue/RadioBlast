#pragma once

#include <JuceHeader.h>
#include "ExtendedFileBrowser.h"
#include "PlaylistComponent.h"
#include "MixerComponent.h"
#include "JDockableWindows.h"
#include "JAdvancedDock.h"
#include "DJLookAndFeel.h"
#include "WaveformGenerator.h"
#include "DualWavefromComponent.h"
#include "SamplePlayer.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, 
                      public juce::TimeSliceClient, 
                      public juce::TimeSliceThread, 
                      public juce::MidiInputCallback,
                      public juce::MenuBarModel,
	                  public juce::ChangeListener,
                      public juce::DragAndDropContainer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void copyBufferFromSampler(const juce::AudioSourceChannelInfo& bufferToFill, Sampler* sampler, bool& retFlag, float gain,double pitchRatio);
    void releaseResources() override;

    void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    virtual int useTimeSlice() override {
		return 10; // Return 0 to indicate that we don't need to be called again immediately
    }

    void closeSettingsWindow();
    void setupMidiInputs();
    void cleanupMidiInputs();
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void createConfig();

    void generateSamplerOutputWithPitch(Sampler* sampler, std::vector<float>& outputL,
        std::vector<float>& outputR, int numSamples,
        float gain, double pitch);

    // BPM Analysis Integration - wird aufgerufen wenn Track geladen wird
    void onTrackLoaded(const juce::File& audioFile, bool isLeftDeck)
    {
        // BPM analysieren im Hintergrund
        if (mixer) {
            mixer->analyzeBPMFromFile(audioFile, isLeftDeck);
        }
    }

private:
    //==============================================================================
    // Your private member variables go here...

    std::unique_ptr<juce::MenuBarComponent> menuBar;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSettingsComponent = nullptr;
    std::unique_ptr<juce::DialogWindow> settingsWindow = nullptr;

    // Menu Item IDs
    enum MenuItemIDs
    {
        audioSettings = 1000,
        midiSettings = 1001,
        about = 1002,
        exit = 1003
    };

    void showAudioSettings();
    void showAbout();

	std::unique_ptr<ExtendedFileBrowser> leftFileBrowser = nullptr;
    std::unique_ptr<ExtendedFileBrowser> rightFileBrowser = nullptr;
    
    std::unique_ptr<juce::WildcardFileFilter> leftFilter = nullptr;
    std::unique_ptr<juce::DirectoryContentsList> leftDirectoryContents = nullptr;
    std::unique_ptr<FileBrowserModel> leftModel = nullptr;

    std::unique_ptr<juce::WildcardFileFilter> rightFilter = nullptr;
    std::unique_ptr<juce::DirectoryContentsList> rightDirectoryContents = nullptr;
    std::unique_ptr<FileBrowserModel> rightModel = nullptr;

    std::unique_ptr<PlaylistComponent> leftPlayList = nullptr;
    std::unique_ptr<PlaylistComponent> rightPlayList = nullptr;
	std::unique_ptr<MixerComponent> mixer = nullptr;
    
    std::unique_ptr<DualWaveformComponent> wave = nullptr;

    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar;

    DockableWindowManager dockManager;
    WindowDockVertical dock{ dockManager };
    TabDock tabDock{ dockManager };
    JAdvancedDock advancedDock{ dockManager };

    juce::AudioDeviceManager audioDeviceManager;
    juce::ApplicationProperties appProperties;

    std::unique_ptr<DJLookAndFeel> djLookAndFeel;
    std::unique_ptr<SamplePlayer> samplePlayer;

    float leftChannelRMS = 0.0f;
    float rightChannelRMS = 0.0f;
    float masterLeftRMS = 0.0f;
    float masterRightRMS = 0.0f;
    float samplePlayerRMS = 0.0f;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

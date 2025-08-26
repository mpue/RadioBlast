/*
  ==============================================================================
    PlayListComponent.h
    Created: 26 Aug 2025
    Author:  mpue

    Playlist mit Drag&Drop, Doppelklick-Play und Auto-Durchlauf
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <random>
#include <algorithm>

class PlaylistComponent : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::DragAndDropTarget,
    public juce::TableListBoxModel
{
public:
    // Constructor
    PlaylistComponent();

    // Destructor
    ~PlaylistComponent() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // FileDragAndDropTarget overrides
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // DragAndDropTarget overrides
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override;

    // Public interface methods
    void addTrack(const juce::File& file);
    void removeTrack(int index);
    void clearPlaylist();
    void shufflePlaylist();

    void playCurrentTrack();
    void playTrack(int index);
    void stopPlayback();
    void playNextTrack();
    void playPreviousTrack();

    // Playlist file operations
    void savePlaylistDialog();
    void loadPlaylistDialog();
    void newPlaylist();
    bool savePlaylist(const juce::File& file);
    bool loadPlaylist(const juce::File& file);
    void quickSave();

    // Callbacks
    std::function<void(const juce::File&)> onFileSelected;
    std::function<void()> onPlaylistFinished;
    std::function<void(const juce::String&)> onPlaylistNameChanged;

    // Getters
    bool getIsPlaying() const { return isPlaying; }
    int getCurrentTrackIndex() const { return currentTrackIndex; }
    size_t getPlaylistSize() const { return playlist.size(); }
    juce::File getCurrentFile() const;
    juce::String getCurrentPlaylistName() const { return currentPlaylistName; }
    juce::File getCurrentPlaylistFile() const { return currentPlaylistFile; }
    bool getHasUnsavedChanges() const { return hasUnsavedChanges; }

private:
    // Private methods
    void setupPlaylistButtons();
    bool saveDJPlaylist(const juce::File& file);
    bool loadDJPlaylist(const juce::File& file);
    bool saveM3UPlaylist(const juce::File& file);
    bool loadM3UPlaylist(const juce::File& file);
    bool savePLSPlaylist(const juce::File& file);
    bool loadPLSPlaylist(const juce::File& file);

    juce::File getDefaultPlaylistDirectory();
    void updateWindowTitle();

    bool isAudioFile(const juce::File& file) const;
    bool containsFile(const juce::File& file) const;
    double getAudioFileDuration(const juce::File& file) const;
    juce::String formatDuration(double seconds) const;
    void updateStatusDisplay();
    void startAutoPlayTimer(double trackDuration);
    void showContextMenu(int rowNumber);

    // Data structures
    struct PlaylistTrack
    {
        juce::File file;
        double duration = 0.0;
        bool hasError = false;
    };

    // Member variables
    std::vector<PlaylistTrack> playlist;
    std::unique_ptr<juce::TableListBox> table;

    // Control buttons
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::TextButton> clearButton;
    std::unique_ptr<juce::ToggleButton> shuffleButton;
    std::unique_ptr<juce::ToggleButton> repeatButton;

    // Playlist management buttons
    std::unique_ptr<juce::TextButton> savePlaylistButton;
    std::unique_ptr<juce::TextButton> loadPlaylistButton;
    std::unique_ptr<juce::TextButton> newPlaylistButton;
    std::unique_ptr<juce::Label> playlistNameLabel;

    // Status display
    std::unique_ptr<juce::Label> statusLabel;

    // State variables
    int currentTrackIndex = -1;
    bool isPlaying = false;
    bool isDragOver = false;

    // Playlist management state
    juce::String currentPlaylistName = "Untitled Playlist";
    juce::File currentPlaylistFile;
    bool hasUnsavedChanges = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
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
    // Callback für Kommunikation mit MainComponent
    std::function<void(const juce::File&)> onFileSelected;
    std::function<void()> onPlaylistFinished;

    PlaylistComponent()
    {
        setSize(400, 300);
		setOpaque(true);

        // Table Setup
        table = std::make_unique<juce::TableListBox>("PlaylistTable", this);
        table->setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
        table->setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff222222));
        table->setOutlineThickness(1);

        table->setOutlineThickness(1);

        // Spalten definieren
        table->getHeader().addColumn("Track", 1, 250, 50, 400);
        table->getHeader().addColumn("Duration", 2, 70, 50, 100);
        table->getHeader().addColumn("Status", 3, 60, 50, 100);

        table->setHeaderHeight(22);
        table->setRowHeight(20);



        addAndMakeVisible(table.get());

        // Control Buttons
        playButton = std::make_unique<juce::TextButton>("Play");
        stopButton = std::make_unique<juce::TextButton>("Stop");
        clearButton = std::make_unique<juce::TextButton>("Clear");
        shuffleButton = std::make_unique<juce::ToggleButton>("Shuffle");
        repeatButton = std::make_unique<juce::ToggleButton>("Repeat");

        addAndMakeVisible(playButton.get());
        addAndMakeVisible(stopButton.get());
        addAndMakeVisible(clearButton.get());
        addAndMakeVisible(shuffleButton.get());
        addAndMakeVisible(repeatButton.get());

        // Button Callbacks
        playButton->onClick = [this]() { playCurrentTrack(); };
        stopButton->onClick = [this]() { stopPlayback(); };
        clearButton->onClick = [this]() { clearPlaylist(); };
        shuffleButton->onStateChange = [this]() {
            if (shuffleButton->getToggleState()) {
                shufflePlaylist();
            }
            };

        // Status Display
        statusLabel = std::make_unique<juce::Label>("", "Drag audio files here or double-click to play");
        statusLabel->setJustificationType(juce::Justification::centred);
        statusLabel->setFont(12.0f);
        addAndMakeVisible(statusLabel.get());

        // Auto-play Timer wird später initialisiert

        currentTrackIndex = -1;
        isPlaying = false;
    }

    ~PlaylistComponent() override
    {
        // Timer wird automatisch gestoppt wenn Component zerstört wird
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xff333333));
        g.fillRect(getLocalBounds());

        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1);

        // Header
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRect(0, 0, getWidth(), 30);
        g.setColour(juce::Colours::lightgrey);
        g.setFont(14.0f);
        g.drawText("PLAYLIST", 10, 5, getWidth() - 20, 20, juce::Justification::left);

        // Track Count
        juce::String trackInfo = juce::String(playlist.size()) + " tracks";
        if (currentTrackIndex >= 0 && currentTrackIndex < playlist.size())
        {
            trackInfo += " (playing " + juce::String(currentTrackIndex + 1) + ")";
        }
        g.drawText(trackInfo, 10, 5, getWidth() - 20, 20, juce::Justification::right);

        // Drag & Drop Overlay
        if (isDragOver)
        {
            g.setColour(juce::Colours::blue.withAlpha(0.2f));
            g.fillRect(getLocalBounds().reduced(2));
            g.setColour(juce::Colours::blue);
            g.drawRect(getLocalBounds().reduced(2), 2);

            g.setColour(juce::Colours::white);
            g.setFont(16.0f);
            g.drawText("Drop audio files here", getLocalBounds(), juce::Justification::centred);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();

        // Header Space
        area.removeFromTop(35);

        // Control Buttons am unteren Rand
        auto buttonArea = area.removeFromBottom(40);
        buttonArea = buttonArea.reduced(5);

        int buttonWidth = buttonArea.getWidth() / 5;
        if (playButton)
            playButton->setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
        if (stopButton)
            stopButton->setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
        if (clearButton)
            clearButton->setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));        
        if (shuffleButton)
            shuffleButton->setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
        if (repeatButton)
            repeatButton->setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));

        // Status Label
        auto statusArea = area.removeFromBottom(25);
        if (statusLabel)
            statusLabel->setBounds(statusArea.reduced(5));

        // Table nimmt den Rest
        if (table)
            table->setBounds(area.reduced(5));
    }

    // === DRAG & DROP ===
    bool isInterestedInFileDrag(const juce::StringArray& files) override
    {
        for (const auto& file : files)
        {
            juce::File f(file);
            if (isAudioFile(f))
                return true;
        }
        return false;
    }

    void fileDragEnter(const juce::StringArray& files, int x, int y) override
    {
        isDragOver = true;
        repaint();
    }

    void fileDragExit(const juce::StringArray& files) override
    {
        isDragOver = false;
        repaint();
    }

    void filesDropped(const juce::StringArray& files, int x, int y) override
    {
        isDragOver = false;

        for (const auto& file : files)
        {
            juce::File f(file);
            if (isAudioFile(f))
            {
                addTrack(f);
            }
        }

        table->updateContent();
        updateStatusDisplay();
        repaint();
    }

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override
    {

        if (!dragSourceDetails.description.isVoid()) {
            if (dragSourceDetails.description.isArray()) {
                juce::StringArray files = *dragSourceDetails.description.getArray();

                for (const auto& file : files)
                {
                    juce::File f(file);
                    if (isAudioFile(f))
                        return true;
                }
            }
            else {
                juce::String file = dragSourceDetails.description.toString();
                juce::File f(file);
                if (isAudioFile(f))
					return true;
            }

        }

        return false;
    }

    void itemDragEnter(const SourceDetails& dragSourceDetails) override
    {
        isDragOver = true;
        repaint();
    }

    void itemDragExit(const SourceDetails& dragSourceDetails) override
    {
        isDragOver = false;
        repaint();
    }

    void itemDropped(const SourceDetails& dragSourceDetails) override
    {
        isDragOver = false;

        // StringArray verarbeiten
       
        if (!dragSourceDetails.description.isVoid()) {
            if (dragSourceDetails.description.isArray()) {
                juce::StringArray files = *dragSourceDetails.description.getArray();

                for (const auto& file : files)
                {
                    juce::File f(file);
                    if (isAudioFile(f))
                        addTrack(f);
                }
            }
            else {
                juce::String file = dragSourceDetails.description.toString();
                juce::File f(file);
                if (isAudioFile(f))
                    addTrack(f);
            }

        }

        table->updateContent();
        updateStatusDisplay();
        repaint();
    }

    // === TABLE MODEL ===
    int getNumRows() override
    {
        return playlist.size();
    }

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber == currentTrackIndex && isPlaying)
        {
            g.setColour(juce::Colours::darkgreen.withAlpha(0.3f));
            g.fillRect(0, 0, width, height);
        }
        else if (rowIsSelected)
        {
            g.setColour(juce::Colours::lightblue.withAlpha(0.3f));
            g.fillRect(0, 0, width, height);
        }
        else if (rowNumber % 2 == 0)
        {
            g.setColour(juce::Colour(0xff333333));
            g.fillRect(0, 0, width, height);
        }
    }

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber >= playlist.size()) return;

        const auto& track = playlist[rowNumber];

        g.setColour(juce::Colours::white);
        g.setFont(11.0f);

        juce::String text;
        switch (columnId)
        {
        case 1: // Track Name
            text = track.file.getFileNameWithoutExtension();
            break;
        case 2: // Duration
            text = formatDuration(track.duration);
            break;
        case 3: // Status
            if (rowNumber == currentTrackIndex && isPlaying)
                text = "♪ Playing";
            else if (track.hasError)
                text = "Error";
            else
                text = "Ready";
            break;
        }

        g.drawText(text, 4, 0, width - 8, height, juce::Justification::centredLeft);
    }

    void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override
    {
        if (rowNumber >= 0 && rowNumber < playlist.size())
        {
            playTrack(rowNumber);
        }
    }

    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override
    {
        // Rechtsklick für Kontext-Menu
        if (e.mods.isPopupMenu() && rowNumber >= 0 && rowNumber < playlist.size())
        {
            showContextMenu(rowNumber);
        }
    }

    // === PLAYLIST FUNKTIONEN ===
    void addTrack(const juce::File& file)
    {
        if (!isAudioFile(file) || containsFile(file))
            return;

        PlaylistTrack track;
        track.file = file;
        track.duration = getAudioFileDuration(file);
        track.hasError = (track.duration <= 0.0);

        playlist.push_back(track);
    }

    void removeTrack(int index)
    {
        if (index >= 0 && index < playlist.size())
        {
            // Falls aktueller Track gelöscht wird
            if (index == currentTrackIndex)
            {
                stopPlayback();
            }
            else if (index < currentTrackIndex)
            {
                currentTrackIndex--;
            }

            playlist.erase(playlist.begin() + index);
            table->updateContent();
            updateStatusDisplay();
            repaint();
        }
    }

    void clearPlaylist()
    {
        stopPlayback();
        playlist.clear();
        currentTrackIndex = -1;
        table->updateContent();
        updateStatusDisplay();
        repaint();
    }

    void shufflePlaylist()
    {
        if (playlist.size() <= 1) return;

        // Aktuellen Track merken
        juce::File currentFile;
        if (currentTrackIndex >= 0 && currentTrackIndex < playlist.size())
        {
            currentFile = playlist[currentTrackIndex].file;
        }

        // Shuffle
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(playlist.begin(), playlist.end(), g);

        // Aktuellen Track-Index anpassen
        if (currentFile.exists())
        {
            for (int i = 0; i < playlist.size(); ++i)
            {
                if (playlist[i].file == currentFile)
                {
                    currentTrackIndex = i;
                    break;
                }
            }
        }

        table->updateContent();
        repaint();
    }

    // === PLAYBACK KONTROLLE ===
    void playCurrentTrack()
    {
        if (currentTrackIndex >= 0 && currentTrackIndex < playlist.size())
        {
            playTrack(currentTrackIndex);
        }
        else if (!playlist.empty())
        {
            playTrack(0);
        }
    }

    void playTrack(int index)
    {
        if (index < 0 || index >= playlist.size()) return;

        currentTrackIndex = index;
        isPlaying = true;

        const auto& track = playlist[index];

        // Callback an MainComponent
        if (onFileSelected)
        {
            onFileSelected(track.file);
        }

        updateStatusDisplay();
        table->updateContent();
        repaint();

        // Auto-play Timer für nächsten Track starten
        startAutoPlayTimer(track.duration);
    }

    void stopPlayback()
    {
        isPlaying = false;

        updateStatusDisplay();
        table->updateContent();
        repaint();
    }

    void playNextTrack()
    {
        if (playlist.empty()) return;

        int nextIndex = currentTrackIndex + 1;

        // Repeat-Modus oder am Ende
        if (nextIndex >= playlist.size())
        {
            if (repeatButton->getToggleState())
            {
                nextIndex = 0; // Von vorne beginnen
            }
            else
            {
                // Playlist beendet
                stopPlayback();
                currentTrackIndex = -1;

                if (onPlaylistFinished)
                {
                    onPlaylistFinished();
                }
                return;
            }
        }

        playTrack(nextIndex);
    }

    void playPreviousTrack()
    {
        if (playlist.empty()) return;

        int prevIndex = currentTrackIndex - 1;

        if (prevIndex < 0)
        {
            if (repeatButton->getToggleState())
            {
                prevIndex = playlist.size() - 1; // Zum letzten Track
            }
            else
            {
                prevIndex = 0; // Beim ersten bleiben
            }
        }

        playTrack(prevIndex);
    }

    // === HELPER FUNKTIONEN ===
    bool isAudioFile(const juce::File& file) const
    {
        juce::String extension = file.getFileExtension().toLowerCase();
        return extension == ".wav" || extension == ".mp3" ||
            extension == ".flac" || extension == ".ogg" ||
            extension == ".aiff" || extension == ".m4a";
    }

    bool containsFile(const juce::File& file) const
    {
        for (const auto& track : playlist)
        {
            if (track.file == file)
                return true;
        }
        return false;
    }

    double getAudioFileDuration(const juce::File& file) const
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(file));

        if (reader != nullptr)
        {
            return reader->lengthInSamples / reader->sampleRate;
        }

        return 0.0;
    }

    juce::String formatDuration(double seconds) const
    {
        if (seconds <= 0.0) return "--:--";

        int minutes = (int)(seconds / 60.0);
        int secs = (int)(seconds) % 60;

        return juce::String::formatted("%d:%02d", minutes, secs);
    }

    void updateStatusDisplay()
    {
        juce::String status;

        if (playlist.empty())
        {
            status = "Drag audio files here or double-click to play";
        }
        else if (isPlaying && currentTrackIndex >= 0)
        {
            status = "Playing: " + playlist[currentTrackIndex].file.getFileNameWithoutExtension();
        }
        else
        {
            status = juce::String(playlist.size()) + " tracks loaded - Double-click to play";
        }

        statusLabel->setText(status, juce::dontSendNotification);
    }

    void startAutoPlayTimer(double trackDuration)
    {
        // Timer für Track-Ende (mit kleinem Puffer)
        int timerMs = (int)((trackDuration + 1.0) * 1000.0);

        // Starte Timer für automatischen nächsten Track
        juce::Timer::callAfterDelay(timerMs, [this]() {
            if (isPlaying) {
                playNextTrack();
            }
            });
    }

    void showContextMenu(int rowNumber)
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Play Track");
        menu.addSeparator();
        menu.addItem(2, "Remove from Playlist");
        menu.addItem(3, "Show in Finder/Explorer");

        // Korrekte JUCE PopupMenu Syntax
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this, rowNumber](int menuResult) {
                switch (menuResult)
                {
                case 1:
                    playTrack(rowNumber);
                    break;
                case 2:
                    removeTrack(rowNumber);
                    break;
                case 3:
                    if (rowNumber >= 0 && rowNumber < playlist.size())
                    {
                        playlist[rowNumber].file.revealToUser();
                    }
                    break;
                }
            });
    }

    // === GETTER ===
    bool getIsPlaying() const { return isPlaying; }
    int getCurrentTrackIndex() const { return currentTrackIndex; }
    size_t getPlaylistSize() const { return playlist.size(); }

    juce::File getCurrentFile() const
    {
        if (currentTrackIndex >= 0 && currentTrackIndex < playlist.size())
        {
            return playlist[currentTrackIndex].file;
        }
        return {};
    }

private:
    struct PlaylistTrack
    {
        juce::File file;
        double duration = 0.0;
        bool hasError = false;
    };

    std::vector<PlaylistTrack> playlist;
    std::unique_ptr<juce::TableListBox> table;

    // Controls
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::TextButton> clearButton;
    std::unique_ptr<juce::ToggleButton> shuffleButton;
    std::unique_ptr<juce::ToggleButton> repeatButton;

    std::unique_ptr<juce::Label> statusLabel;

    // State
    int currentTrackIndex = -1;
    bool isPlaying = false;
    bool isDragOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
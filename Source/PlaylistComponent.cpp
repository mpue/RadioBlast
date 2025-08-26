/*
  ==============================================================================
    PlayListComponent.cpp
    Created: 26 Aug 2025
    Author:  mpue
  ==============================================================================
*/

#include "PlayListComponent.h"

PlaylistComponent::PlaylistComponent()
{
    setSize(400, 300);
    setOpaque(true);

    // Table Setup
    table = std::make_unique<juce::TableListBox>("PlaylistTable", this);
    table->setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
    table->setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff222222));
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

    // Setup playlist management buttons
    setupPlaylistButtons();

    currentTrackIndex = -1;
    isPlaying = false;
}

PlaylistComponent::~PlaylistComponent()
{
    // Timer wird automatisch gestoppt wenn Component zerstört wird
}

void PlaylistComponent::setupPlaylistButtons()
{
    // Neue Buttons hinzufügen
    savePlaylistButton = std::make_unique<juce::TextButton>("Save PL");
    loadPlaylistButton = std::make_unique<juce::TextButton>("Load PL");
    newPlaylistButton = std::make_unique<juce::TextButton>("New PL");

    addAndMakeVisible(savePlaylistButton.get());
    addAndMakeVisible(loadPlaylistButton.get());
    addAndMakeVisible(newPlaylistButton.get());

    // Button Callbacks
    savePlaylistButton->onClick = [this]() { savePlaylistDialog(); };
    loadPlaylistButton->onClick = [this]() { loadPlaylistDialog(); };
    newPlaylistButton->onClick = [this]() { newPlaylist(); };

    // Playlist Name Label
    playlistNameLabel = std::make_unique<juce::Label>("PlaylistName", "Untitled Playlist");
    playlistNameLabel->setJustificationType(juce::Justification::centred);
    playlistNameLabel->setFont(juce::Font(12.0f, juce::Font::bold));
    playlistNameLabel->setEditable(true);
    playlistNameLabel->onTextChange = [this]() {
        currentPlaylistName = playlistNameLabel->getText();
        hasUnsavedChanges = true;
        };
    addAndMakeVisible(playlistNameLabel.get());
}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds();

    // Header Space mit Playlist-Controls
    auto headerArea = area.removeFromTop(55);

    // Playlist Name
    auto nameArea = headerArea.removeFromTop(25);
    nameArea = nameArea.reduced(10, 2);
    if (playlistNameLabel)
        playlistNameLabel->setBounds(nameArea);

    // Playlist Buttons
    auto playlistButtonArea = headerArea.reduced(5);
    int plButtonWidth = playlistButtonArea.getWidth() / 3;
    if (newPlaylistButton)
        newPlaylistButton->setBounds(playlistButtonArea.removeFromLeft(plButtonWidth).reduced(2));
    if (loadPlaylistButton)
        loadPlaylistButton->setBounds(playlistButtonArea.removeFromLeft(plButtonWidth).reduced(2));
    if (savePlaylistButton)
        savePlaylistButton->setBounds(playlistButtonArea.removeFromLeft(plButtonWidth).reduced(2));

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

void PlaylistComponent::paint(juce::Graphics& g)
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

// === DRAG & DROP IMPLEMENTATIONS ===
bool PlaylistComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        juce::File f(file);
        if (isAudioFile(f))
            return true;
    }
    return false;
}

void PlaylistComponent::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    isDragOver = true;
    repaint();
}

void PlaylistComponent::fileDragExit(const juce::StringArray& files)
{
    isDragOver = false;
    repaint();
}

void PlaylistComponent::filesDropped(const juce::StringArray& files, int x, int y)
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

bool PlaylistComponent::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
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

void PlaylistComponent::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    isDragOver = true;
    repaint();
}

void PlaylistComponent::itemDragExit(const SourceDetails& dragSourceDetails)
{
    isDragOver = false;
    repaint();
}

void PlaylistComponent::itemDropped(const SourceDetails& dragSourceDetails)
{
    isDragOver = false;

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

// === TABLE MODEL IMPLEMENTATIONS ===
int PlaylistComponent::getNumRows()
{
    return playlist.size();
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
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

void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
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

void PlaylistComponent::cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& e)
{
    if (rowNumber >= 0 && rowNumber < playlist.size())
    {
        playTrack(rowNumber);
    }
}

void PlaylistComponent::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& e)
{
    // Rechtsklick für Kontext-Menu
    if (e.mods.isPopupMenu() && rowNumber >= 0 && rowNumber < playlist.size())
    {
        showContextMenu(rowNumber);
    }
}

// === PLAYLIST FUNKTIONEN ===
void PlaylistComponent::addTrack(const juce::File& file)
{
    if (!isAudioFile(file) || containsFile(file))
        return;

    PlaylistTrack track;
    track.file = file;
    track.duration = getAudioFileDuration(file);
    track.hasError = (track.duration <= 0.0);

    playlist.push_back(track);

    hasUnsavedChanges = true;
    updateWindowTitle();
}

void PlaylistComponent::removeTrack(int index)
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

        hasUnsavedChanges = true;
        updateWindowTitle();
    }
}

void PlaylistComponent::clearPlaylist()
{
    stopPlayback();
    playlist.clear();
    currentTrackIndex = -1;
    table->updateContent();
    updateStatusDisplay();
    hasUnsavedChanges = true;
    updateWindowTitle();
    repaint();
}

void PlaylistComponent::shufflePlaylist()
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
    hasUnsavedChanges = true;
    updateWindowTitle();
    repaint();
}

// === PLAYBACK KONTROLLE ===
void PlaylistComponent::playCurrentTrack()
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

void PlaylistComponent::playTrack(int index)
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

void PlaylistComponent::stopPlayback()
{
    isPlaying = false;
    updateStatusDisplay();
    table->updateContent();
    repaint();
}

void PlaylistComponent::playNextTrack()
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

void PlaylistComponent::playPreviousTrack()
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

juce::File PlaylistComponent::getCurrentFile() const
{
    if (currentTrackIndex >= 0 && currentTrackIndex < playlist.size())
    {
        return playlist[currentTrackIndex].file;
    }
    return {};
}

// === PLAYLIST SPEICHERN/LADEN ===
void PlaylistComponent::savePlaylistDialog()
{
    if (playlist.empty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Playlist leer",
            "Es gibt nichts zu speichern. Fügen Sie zuerst Tracks hinzu."
        );
        return;
    }

    juce::FileChooser chooser("Playlist speichern...",
        getDefaultPlaylistDirectory(),
        "*.djpl;*.m3u;*.pls");

    auto flags = juce::FileBrowserComponent::saveMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::warnAboutOverwriting;

    chooser.launchAsync(flags, [this](const juce::FileChooser& fc) {
        if (fc.getResults().size() > 0)
        {
            auto file = fc.getResult();

            // Standard-Extension hinzufügen falls keine vorhanden
            if (!file.hasFileExtension("djpl") &&
                !file.hasFileExtension("m3u") &&
                !file.hasFileExtension("pls"))
            {
                file = file.withFileExtension("djpl");
            }

            if (savePlaylist(file))
            {
                currentPlaylistFile = file;
                currentPlaylistName = file.getFileNameWithoutExtension();
                playlistNameLabel->setText(currentPlaylistName, juce::dontSendNotification);
                hasUnsavedChanges = false;
                updateWindowTitle();
            }
        }
        });
}

void PlaylistComponent::loadPlaylistDialog()
{
    if (hasUnsavedChanges)
    {
        int result = juce::AlertWindow::showYesNoCancelBox(
            juce::AlertWindow::QuestionIcon,
            "Ungespeicherte Änderungen",
            "Die aktuelle Playlist wurde geändert. Möchten Sie die Änderungen speichern?",
            "Speichern", "Nicht speichern", "Abbrechen", this, nullptr
        );

        if (result == 1) // Speichern
        {
            if (currentPlaylistFile.exists())
                savePlaylist(currentPlaylistFile);
            else
                savePlaylistDialog();
        }
        else if (result == 0) // Abbrechen
        {
            return;
        }
        // Bei "Nicht speichern" (result == 2) einfach weitermachen
    }

    juce::FileChooser chooser("Playlist laden...",
        getDefaultPlaylistDirectory(),
        "*.djpl;*.m3u;*.pls");

    auto flags = juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles;

    chooser.launchAsync(flags, [this](const juce::FileChooser& fc) {
        if (fc.getResults().size() > 0)
        {
            loadPlaylist(fc.getResult());
        }
        });
}

void PlaylistComponent::newPlaylist()
{
    if (hasUnsavedChanges)
    {
        int result = juce::AlertWindow::showYesNoCancelBox(
            juce::AlertWindow::QuestionIcon,
            "Ungespeicherte Änderungen",
            "Die aktuelle Playlist wurde geändert. Möchten Sie die Änderungen speichern?",
            "Speichern", "Nicht speichern", "Abbrechen", this, nullptr
        );

        if (result == 1) // Speichern
        {
            if (currentPlaylistFile.exists())
                savePlaylist(currentPlaylistFile);
            else
                savePlaylistDialog();
        }
        else if (result == 0) // Abbrechen
        {
            return;
        }
    }

    clearPlaylist();
    currentPlaylistFile = juce::File();
    currentPlaylistName = "Untitled Playlist";
    playlistNameLabel->setText(currentPlaylistName, juce::dontSendNotification);
    hasUnsavedChanges = false;
    updateWindowTitle();
}

bool PlaylistComponent::savePlaylist(const juce::File& file)
{
    juce::String extension = file.getFileExtension().toLowerCase();

    try
    {
        if (extension == ".djpl")
            return saveDJPlaylist(file);
        else if (extension == ".m3u")
            return saveM3UPlaylist(file);
        else if (extension == ".pls")
            return savePLSPlaylist(file);
        else
            return saveDJPlaylist(file.withFileExtension("djpl"));
    }
    catch (const std::exception& e)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Fehler beim Speichern",
            "Playlist konnte nicht gespeichert werden: " + juce::String(e.what())
        );
        return false;
    }
}

bool PlaylistComponent::loadPlaylist(const juce::File& file)
{
    if (!file.exists())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Datei nicht gefunden",
            "Die Playlist-Datei wurde nicht gefunden: " + file.getFullPathName()
        );
        return false;
    }

    juce::String extension = file.getFileExtension().toLowerCase();

    try
    {
        bool success = false;

        if (extension == ".djpl")
            success = loadDJPlaylist(file);
        else if (extension == ".m3u")
            success = loadM3UPlaylist(file);
        else if (extension == ".pls")
            success = loadPLSPlaylist(file);

        if (success)
        {
            currentPlaylistFile = file;
            currentPlaylistName = file.getFileNameWithoutExtension();
            playlistNameLabel->setText(currentPlaylistName, juce::dontSendNotification);
            hasUnsavedChanges = false;
            updateWindowTitle();

            table->updateContent();
            updateStatusDisplay();
            repaint();
        }

        return success;
    }
    catch (const std::exception& e)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Fehler beim Laden",
            "Playlist konnte nicht geladen werden: " + juce::String(e.what())
        );
        return false;
    }
}

void PlaylistComponent::quickSave()
{
    if (currentPlaylistFile.exists())
    {
        savePlaylist(currentPlaylistFile);
    }
    else
    {
        savePlaylistDialog();
    }
}

// === SPEZIFISCHE DATEI-FORMATE ===
bool PlaylistComponent::saveDJPlaylist(const juce::File& file)
{
    juce::DynamicObject::Ptr playlistObj = new juce::DynamicObject();

    // Metadaten
    playlistObj->setProperty("name", currentPlaylistName);
    playlistObj->setProperty("version", "1.0");
    playlistObj->setProperty("created", juce::Time::getCurrentTime().toString(true, true));
    playlistObj->setProperty("trackCount", (int)playlist.size());

    // Tracks Array
    juce::Array<juce::var> tracksArray;

    for (const auto& track : playlist)
    {
        juce::DynamicObject::Ptr trackObj = new juce::DynamicObject();
        trackObj->setProperty("file", track.file.getFullPathName());
        trackObj->setProperty("name", track.file.getFileNameWithoutExtension());
        trackObj->setProperty("duration", track.duration);
        trackObj->setProperty("hasError", track.hasError);

        tracksArray.add(juce::var(trackObj.get()));
    }

    playlistObj->setProperty("tracks", tracksArray);

    // JSON schreiben
    juce::var playlistVar(playlistObj.get());
    juce::String jsonString = juce::JSON::toString(playlistVar, true);

    return file.replaceWithText(jsonString);
}

bool PlaylistComponent::loadDJPlaylist(const juce::File& file)
{
    juce::String content = file.loadFileAsString();
    juce::var json = juce::JSON::parse(content);

    if (!json.isObject())
        return false;

    juce::DynamicObject::Ptr obj = json.getDynamicObject();
    if (!obj)
        return false;

    // Playlist leeren
    playlist.clear();
    currentTrackIndex = -1;
    isPlaying = false;

    // Name laden
    if (obj->hasProperty("name"))
        currentPlaylistName = obj->getProperty("name").toString();

    // Tracks laden
    if (obj->hasProperty("tracks"))
    {
        juce::Array<juce::var>* tracksArray = obj->getProperty("tracks").getArray();
        if (tracksArray)
        {
            for (const auto& trackVar : *tracksArray)
            {
                if (trackVar.isObject())
                {
                    juce::DynamicObject::Ptr trackObj = trackVar.getDynamicObject();
                    if (trackObj && trackObj->hasProperty("file"))
                    {
                        juce::File trackFile(trackObj->getProperty("file").toString());
                        if (trackFile.exists() && isAudioFile(trackFile))
                        {
                            PlaylistTrack track;
                            track.file = trackFile;

                            // Gespeicherte Duration verwenden oder neu berechnen
                            if (trackObj->hasProperty("duration"))
                                track.duration = trackObj->getProperty("duration");
                            else
                                track.duration = getAudioFileDuration(trackFile);

                            track.hasError = (track.duration <= 0.0);
                            playlist.push_back(track);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool PlaylistComponent::saveM3UPlaylist(const juce::File& file)
{
    juce::StringArray lines;
    lines.add("#EXTM3U");

    for (const auto& track : playlist)
    {
        // Extended info line
        juce::String extinf = "#EXTINF:" + juce::String((int)track.duration) + "," +
            track.file.getFileNameWithoutExtension();
        lines.add(extinf);

        // File path
        lines.add(track.file.getFullPathName());
    }

    return file.replaceWithText(lines.joinIntoString("\n"));
}

bool PlaylistComponent::loadM3UPlaylist(const juce::File& file)
{
    juce::StringArray lines = juce::StringArray::fromLines(file.loadFileAsString());

    playlist.clear();
    currentTrackIndex = -1;
    isPlaying = false;

    for (int i = 0; i < lines.size(); ++i)
    {
        juce::String line = lines[i].trim();

        // Skip comments and empty lines
        if (line.startsWith("#") || line.isEmpty())
            continue;

        // Treat as file path
        juce::File trackFile(line);

        // Try relative path if absolute doesn't exist
        if (!trackFile.exists())
        {
            trackFile = file.getParentDirectory().getChildFile(line);
        }

        if (trackFile.exists() && isAudioFile(trackFile))
        {
            addTrack(trackFile);
        }
    }

    return !playlist.empty();
}

bool PlaylistComponent::savePLSPlaylist(const juce::File& file)
{
    juce::StringArray lines;
    lines.add("[playlist]");
    lines.add("NumberOfEntries=" + juce::String((int)playlist.size()));

    for (int i = 0; i < playlist.size(); ++i)
    {
        const auto& track = playlist[i];
        lines.add("File" + juce::String(i + 1) + "=" + track.file.getFullPathName());
        lines.add("Title" + juce::String(i + 1) + "=" + track.file.getFileNameWithoutExtension());
        lines.add("Length" + juce::String(i + 1) + "=" + juce::String((int)track.duration));
    }

    lines.add("Version=2");

    return file.replaceWithText(lines.joinIntoString("\n"));
}

bool PlaylistComponent::loadPLSPlaylist(const juce::File& file)
{
    juce::StringArray lines = juce::StringArray::fromLines(file.loadFileAsString());

    playlist.clear();
    currentTrackIndex = -1;
    isPlaying = false;

    for (const auto& line : lines)
    {
        juce::String trimmed = line.trim();

        if (trimmed.startsWith("File"))
        {
            int equalPos = trimmed.indexOfChar('=');
            if (equalPos > 0)
            {
                juce::String filePath = trimmed.substring(equalPos + 1);
                juce::File trackFile(filePath);

                // Try relative path if absolute doesn't exist
                if (!trackFile.exists())
                {
                    trackFile = file.getParentDirectory().getChildFile(filePath);
                }

                if (trackFile.exists() && isAudioFile(trackFile))
                {
                    addTrack(trackFile);
                }
            }
        }
    }

    return !playlist.empty();
}

// === HILFSFUNKTIONEN ===
juce::File PlaylistComponent::getDefaultPlaylistDirectory()
{
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto playlistDir = documentsDir.getChildFile("DJ Playlists");

    if (!playlistDir.exists())
        playlistDir.createDirectory();

    return playlistDir;
}

void PlaylistComponent::updateWindowTitle()
{
    // Diese Funktion kann von der Hauptkomponente überschrieben werden
    // um den Fenstertitel zu aktualisieren
    if (onPlaylistNameChanged)
    {
        juce::String title = currentPlaylistName;
        if (hasUnsavedChanges)
            title += " *";

        onPlaylistNameChanged(title);
    }
}

bool PlaylistComponent::isAudioFile(const juce::File& file) const
{
    juce::String extension = file.getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".mp3" ||
        extension == ".flac" || extension == ".ogg" ||
        extension == ".aiff" || extension == ".m4a";
}

bool PlaylistComponent::containsFile(const juce::File& file) const
{
    for (const auto& track : playlist)
    {
        if (track.file == file)
            return true;
    }
    return false;
}

double PlaylistComponent::getAudioFileDuration(const juce::File& file) const
{
    if (!file.exists() || file.getSize() == 0)
        return 0.0;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    auto fileStream = std::make_unique<juce::FileInputStream>(file);
    if (!fileStream->openedOk())
        return 0.0;

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(std::move(fileStream)));

    if (reader != nullptr && reader->lengthInSamples > 0)
    {
        return static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    }

    return 0.0;
}

juce::String PlaylistComponent::formatDuration(double seconds) const
{
    if (seconds <= 0.0) return "--:--";

    int minutes = (int)(seconds / 60.0);
    int secs = (int)(seconds) % 60;

    return juce::String::formatted("%d:%02d", minutes, secs);
}

void PlaylistComponent::updateStatusDisplay()
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

void PlaylistComponent::startAutoPlayTimer(double trackDuration)
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

void PlaylistComponent::showContextMenu(int rowNumber)
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
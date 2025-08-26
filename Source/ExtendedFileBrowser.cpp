//
//  ExtendedFileBrowser.cpp
//  Synthlab - App
//
//  Created by Matthias Pueski on 30.04.18.
//  Copyright © 2018 Pueski. All rights reserved.
//

#include "ExtendedFileBrowser.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioManager.h"

using juce::File;
using juce::WildcardFileFilter;
using juce::TableListBox;
using juce::Viewport;
using juce::Colour;
using juce::String;
using juce::DirectoryContentsList;
using juce::ChangeBroadcaster;
using juce::Graphics;
using juce::ValueTree;
using juce::OutputStream;
using juce::XmlElement;
using juce::XmlDocument;
using juce::ScopedPointer;

ExtendedFileBrowser::ExtendedFileBrowser(const File& initialFileOrDirectory,const WildcardFileFilter* fileFilter, FileBrowserModel* model, bool leftPlayer) : initialDir(initialFileOrDirectory), 
left(left){
    
    // addMouseListener(this, true);
    table = new TableListBox();
    table->getHeader().addColumn("File", 1, 350);
    table->getHeader().addColumn("Size", 2, 50);
    table->setModel(model);
   
    juce::Array<juce::File> drives;

    File::findFileSystemRoots(drives);

    for (int i = 0; i < drives.size(); i++) {
        juce::File f = drives.getReference(i);
        juce::TextButton* button = new juce::TextButton(f.getFileName());
        button->setSize(30, 20) ;
        button->setTopLeftPosition(i * 35, 0);
        driveButtons.push_back(button);
        addAndMakeVisible(button);
        button->addListener(this);
    }

    juce::TextButton* stopButton = new juce::TextButton("STOP");
    stopButton->setSize(35, 20);
    stopButton->setTopLeftPosition(drives.size() * 35, 0);
    driveButtons.push_back(stopButton);
    addAndMakeVisible(stopButton);
    stopButton->addListener(this);


    this->model = model;
    view  = new Viewport();
    view->setTopLeftPosition(0,25);
    view->setViewedComponent(table);
	view->setScrollBarsShown(true, false);
    addAndMakeVisible(view);
    
	sampler = new Sampler(44100, 512);
    table->addMouseListener(this, true);
    
    // loadState();
    
      

    model->update();

    repaint();
}

ExtendedFileBrowser::~ExtendedFileBrowser() {
    // saveState();
    delete table;
    delete view;
    for (int i = 0; i < driveButtons.size(); i++) {
        delete driveButtons.at(i);
    }
        
}

void ExtendedFileBrowser::mouseDrag(const juce::MouseEvent& event) {
    // Nur starten wenn noch nicht am Dragging und Maus weit genug bewegt
    if (!isDragging && event.getDistanceFromDragStart() > 5) {

        // Prüfen ob eine Zeile ausgewählt ist
        int selectedRow = table->getSelectedRow();
        if (selectedRow > 0) { // > 0 weil 0 ist "[..]"

            File selectedFile = model->getDirectoryList()->getFile(selectedRow);

            // Nur Audio-Dateien können gedraggt werden
            if (isAudioFile(selectedFile)) {
                isDragging = true;

                // StringArray mit Dateipfaden für Drag&Drop erstellen
                juce::StringArray files;
                files.add(selectedFile.getFullPathName());

                // WICHTIG: Drag-Data muss als StringArray übergeben werden
                juce::var dragDescription = files;

                // DEBUG: Prüfen was übergeben wird
                DBG("Dragging files: " + files.joinIntoString(", "));

                // Thumbnail für Drag-Vorschau erstellen
                juce::Image dragImage(juce::Image::ARGB, 100, 20, true);
                juce::Graphics g(dragImage);
                g.setColour(juce::Colours::darkblue.withAlpha(0.8f));
                g.fillRoundedRectangle(0, 0, 100, 20, 5.0f);
                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText(selectedFile.getFileNameWithoutExtension(),
                    0, 0, 100, 20, juce::Justification::centred);

                // Drag starten - mit Parent als Container
                // if (auto* container = findParentComponentOfClass<juce::DragAndDropContainer>())
                {
                    startDragging(dragDescription, this, dragImage, true);
                }
                // DEBUG-Ausgabe
                DBG("Starting drag for file: " + selectedFile.getFullPathName());
            }
        }
    }
}
bool ExtendedFileBrowser::isAudioFile(const juce::File& file) const {
    if (file.isDirectory()) return false;

    juce::String extension = file.getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".mp3" ||
        extension == ".flac" || extension == ".ogg" ||
        extension == ".aiff" || extension == ".aif" ||
        extension == ".m4a";
}

juce::StringArray ExtendedFileBrowser::getSelectedAudioFiles() const {
    juce::StringArray audioFiles;

    int selectedRow = table->getSelectedRow();
    if (selectedRow > 0) { // > 0 weil 0 ist "[..]"
        File selectedFile = model->getDirectoryList()->getFile(selectedRow);
        if (isAudioFile(selectedFile)) {
            audioFiles.add(selectedFile.getFullPathName());
        }
    }

    // Hier könntest du später Multi-Select-Support hinzufügen
    // indem du durch alle ausgewählten Zeilen iterierst

    return audioFiles;
}

void ExtendedFileBrowser::paint(juce::Graphics &g) {
    g.fillAll (Colour (0xff333333));
    g.fillRect (getLocalBounds());
    // Component::paint(g);
}

void ExtendedFileBrowser::resized() {
    if (getParentComponent() != nullptr) {
        setSize(getParentWidth(), getParentHeight());
        view->setSize(getWidth(), getHeight());
        table->setSize(getWidth(), getHeight() - 30);
        
    }
}

void ExtendedFileBrowser::changeListenerCallback (ChangeBroadcaster* source) {
    table->updateContent();
}

void ExtendedFileBrowser::mouseDown(const juce::MouseEvent &event) {

    isDragging = false;
    dragStartPosition = event.getPosition();

    if (table->getSelectedRow() > 0) {
        File* f = new File(model->getDirectoryList()->getFile(table->getSelectedRow()));
        if (f->exists()) {
            if (!f->isDirectory()) {
            }
            
        }
        delete f;
    }

}

void ExtendedFileBrowser::mouseDoubleClick(const juce::MouseEvent &event) {
    
    if (table->getSelectedRow() > 0) {
        File* f = new File(model->getDirectoryList()->getFile(table->getSelectedRow()));
        if (f->exists()) {
            if (f->isDirectory()) {
                model->setCurrentDir(*f);
            }
            else {
                selectedFile = f;

                if (!f->getFileNameWithoutExtension().startsWith(".")) {
                    if (f->getFileExtension().toLowerCase().contains("wav") ||
                        f->getFileExtension().toLowerCase().contains("mp3") ||
                        f->getFileExtension().toLowerCase().contains("aif") ||
                        f->getFileExtension().toLowerCase().contains("ogg")) {
                        sampler->stop();
                        sampler->loadSample(*f);
                        if (onTrackLoadedCallback) {
						    onTrackLoadedCallback(*f, left);
                        }
                        sampler->play();
                    }
                }
                else {
                    f->startAsProcess();
                }


                sendChangeMessage();
            }
        }
        delete f;
    }
    else {
        File current = File(model->getCurrentDir());
        File parent = File(current.getParentDirectory());
        model->setCurrentDir(parent);
    }

}

void ExtendedFileBrowser::buttonClicked(juce::Button* button)
{
    if (button->getButtonText() == "STOP") {
		sampler->stop();
    }
    else {
        juce::File file = juce::File(button->getButtonText() + "\\");
        model->setCurrentDir(file);
    }

}

void ExtendedFileBrowser::timerCallback() {
   
}

//===========================================================================
// Model
//===========================================================================

FileBrowserModel::FileBrowserModel(DirectoryContentsList* directoryList, File& initalDir) {
    this->directoryList = directoryList;
    this->currentDirectory = initalDir.getFullPathName();
    directoryList->setDirectory(initalDir, true,true);
	setCurrentDir(initalDir);
}

int FileBrowserModel::getNumRows() {
    return directoryList->getNumFiles() + 1;
}
void FileBrowserModel::paintCell (Graphics& g,
                int rowNumber,
                int columnId,
                int width, int height,
                bool rowIsSelected) {
    
    g.setColour(juce::Colours::black);
    
    String text = "";
    
    if (columnId == 1) {
 
        if (rowNumber > 0) {
            text = directoryList->getFile(rowNumber).getFileName();
        }
        else {
            text = "[..]";
        }
        g.setColour(juce::Colours::white);
        g.drawText(text, 0,0, width,height, juce::Justification::centredLeft);
       
    }
    else if (columnId == 2) {
        if (rowNumber > 0) {
            text = String(directoryList->getFile(rowNumber).getSize() / 1024) + "kB";
        }
        else {
            text = "";
        }
        
        g.setColour(juce::Colours::white);
        g.drawText(text, 0,0, width,height, juce::Justification::right);
    }
    
}

void FileBrowserModel::paintRowBackground (Graphics& g,
                         int rowNumber,
                         int width, int height,
                         bool rowIsSelected) {

    
    if (rowIsSelected) {
        g.setColour(juce::Colours::orange);
    }
    else {
        g.setColour(Colour (0xff222222));
    }
    
    g.fillRect(0,0,width,height);
}

void FileBrowserModel::setCurrentDir(juce::File& dir){
    this->currentDirectory = dir.getFullPathName();
    directoryList->setDirectory(dir, true, true);
}

void FileBrowserModel::update() {
    directoryList->refresh();
}

DirectoryContentsList* FileBrowserModel::getDirectoryList() {
    return directoryList;
}

void ExtendedFileBrowser::saveState(){
    String userHome = File::getSpecialLocation(File::userHomeDirectory).getFullPathName();
    File appDir = File(userHome+"/.RadioBlast");
    
    if (!appDir.exists()) {
        appDir.createDirectory();
    }
    
    File configFile = File(userHome+"/.RadioBlast/state.xml");
    
    if (!configFile.exists()) {
        configFile.create();
    }
    else {
        configFile.deleteFile();
        configFile = File(userHome+"/.RadioBlast/state.xml");
    }
    
    ValueTree* v = new ValueTree("SavedState");
    
    ValueTree child = ValueTree("File");
    child.setProperty("lastDirectory", model->getCurrentDir(), nullptr);
    v->addChild(child, -1, nullptr);
    
    std::unique_ptr<OutputStream> os = configFile.createOutputStream();    
    std::unique_ptr<XmlElement> xml = v->createXml();    
    xml->writeToStream(*os, "");
    
    os = nullptr;
    xml = nullptr;

    delete v;
}

void ExtendedFileBrowser::loadState() {
    String userHome = File::getSpecialLocation(File::userHomeDirectory).getFullPathName();
    
    File appDir = File(userHome+"/.RadioBlast");
    
    if (!appDir.exists()) {
        appDir.createDirectory();
    }
    
    File configFile = File(userHome+"/.RadioBlast/state.xml");
    
    if (configFile.exists()) {
        std::unique_ptr<XmlElement> xml = XmlDocument(configFile).getDocumentElement();

        if (xml == nullptr)
			return;

        ValueTree v = ValueTree::fromXml(*xml.get());
        
        if (v.getNumChildren() > 0) {
            String path = v.getChild(0).getProperty("lastDirectory");
            File file = File(path);
            model->setCurrentDir(file);
        }
        xml = nullptr;
    }
}

void ExtendedFileBrowser::focusLost(FocusChangeType cause)
{
    Component::focusLost(cause);
    sampler->stop();
}


File* ExtendedFileBrowser::getSelectedFile() {
    return selectedFile;
}

//
//  ExtendedFileBrowser.hpp
//  Synthlab - App
//
//  Created by Matthias Pueski on 30.04.18.
//  Copyright © 2018 Pueski. All rights reserved.
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioEngine/Sampler.h"

class FileBrowserModel : public juce::TableListBoxModel {
public:
    
    FileBrowserModel(juce::DirectoryContentsList* directoryList, juce::File& initalDir);
    
    virtual void paintRowBackground (juce::Graphics& g,
                                     int rowNumber,
                                     int width, int height,
                                     bool rowIsSelected) override;
    virtual void paintCell (juce::Graphics& g,
                            int rowNumber,
                            int columnId,
                            int width, int height,
                            bool rowIsSelected) override;
    virtual int getNumRows() override;
    void setCurrentDir(juce::File& dir);
    void update();
    juce::DirectoryContentsList* getDirectoryList();
    
    juce::String getCurrentDir() {
        return currentDirectory;
    }
    
    juce::var getDragSourceDescription (const juce::SparseSet<int>& currentlySelectedRows) override{
        
        if (!currentlySelectedRows.getTotalRange().isEmpty()) {
            juce::File file = directoryList->getFile(currentlySelectedRows.getTotalRange().getStart());
            return file.getFullPathName();
        }
        
        return juce::var();
    }
    
private:
    juce::DirectoryContentsList* directoryList;
    juce::String currentDirectory = "";

};

class ExtendedFileBrowser : public juce::Component,  
                            public juce::ChangeListener, 
                            public juce::Timer, 
                            public juce::ChangeBroadcaster,
                            public juce::Button::Listener,
                            public juce::DragAndDropContainer
{
    
    
public:
    ExtendedFileBrowser(const juce::File& initialFileOrDirectory,const juce::WildcardFileFilter* fileFilter, FileBrowserModel* model, bool left);
    ~ExtendedFileBrowser();
    
    std::function<void(const juce::File&, bool)> onTrackLoadedCallback;

    void setTrackLoadedCallback(std::function<void(const juce::File&, bool)> callback)
    {
        onTrackLoadedCallback = callback;
    }

    bool isValidDirectory(const juce::File& dir);
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void paint (juce::Graphics& g) override;
    void resized() override;
    virtual void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    juce::File* getSelectedFile();
    void saveState();
    void loadState();
    
    void focusLost(FocusChangeType cause) override;
    
    juce::TableListBox* getTable() {
        return table;
    };
    
    FileBrowserModel* getModel() {
        return model;
    }
    
    Sampler* getSampler() {
        return sampler;
	}

    void buttonClicked(juce::Button* button) override;

    virtual void timerCallback() override;

    bool isAudioFile(const juce::File& file) const;
    juce::StringArray getSelectedAudioFiles() const;

private:
    FileBrowserModel* model = nullptr;
    const juce::File& initialDir;
    const juce::WildcardFileFilter* filter;
    juce::File* selectedFile;
    juce::TableListBox* table = nullptr;
    juce::Viewport* view = nullptr;
    Sampler* sampler;
    bool playing = false;
    std::vector<juce::Button*> driveButtons;
    bool left = false;
    bool isDragging = false;
    juce::Point<int> dragStartPosition;

};


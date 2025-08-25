/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#if JUCE_WINDOWS
#include <windows.h>
#endif
using juce::JUCEApplication;
using juce::String;
using juce::Logger;
using juce::DocumentWindow;
using juce::ScopedPointer;
using juce::Desktop;
using juce::Rectangle;

//==============================================================================
class RadioBlastApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    RadioBlastApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow(String name) : DocumentWindow(name,
            Desktop::getInstance().getDefaultLookAndFeel()
            .findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons)
        {
            
            setUsingNativeTitleBar(false);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);

            juce::Rectangle<int> r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
            centreWithSize(r.getWidth(), r.getHeight());
            juce::MessageManager::callAsync([this]()
                {
                    setMinimised(false);
                    setVisible(true);
                    toFront(true);
#if JUCE_WINDOWS
#undef byte
                    if (auto* peer = getPeer())
                    {
                        if (auto hwnd = reinterpret_cast<HWND>(peer->getNativeHandle()))
                            SetForegroundWindow(hwnd);
                    }
#endif
                });
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (RadioBlastApplication)

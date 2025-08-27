/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application with splash screen.

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

namespace RadioBlast
{

    //==============================================================================
    class SplashScreen : public juce::Component, public juce::Timer
    {
    public:
        SplashScreen()
        {
            setSize(800, 563);

            // Zentriere den Splash Screen
            auto bounds = juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea;
            setCentrePosition(bounds.getCentreX(), bounds.getCentreY());

            // Mache das Fenster sichtbar
            setVisible(true);
            addToDesktop(juce::ComponentPeer::windowHasDropShadow);

            //  Optional: Lade dein Logo/Image hier
            logo = juce::ImageCache::getFromMemory(BinaryData::splash_png, BinaryData::splash_pngSize);
        }

        void paint(juce::Graphics& g) override
        {
            // Hintergrund
            g.fillAll(juce::Colours::white);


             // Rahmen
            g.setColour(juce::Colours::darkgrey);
            g.drawRect(getLocalBounds(), 2);

             if (logo.isValid())
             {
                 auto logoArea = getLocalBounds();
                 g.drawImage(logo, logoArea.toFloat(), juce::RectanglePlacement::centred);
             }
            // App Name
            g.setColour(juce::Colours::grey);
            g.setFont(24.0f);
            g.drawText("RadioBlast", getLocalBounds().removeFromTop(getHeight() / 2),
                juce::Justification::centred, true);

            // Loading Text
            g.setFont(14.0f);
            auto loadingArea = getLocalBounds().removeFromBottom(getHeight() / 3);
            g.drawText("Wird geladen...", loadingArea, juce::Justification::centred, true);

            // Progress Bar
            auto progressArea = loadingArea.removeFromBottom(30).reduced(40, 5);
            g.setColour(juce::Colours::lightgrey);
            g.fillRect(progressArea);

            g.setColour(juce::Colours::grey);
            auto progressWidth = (int)(progressArea.getWidth() * progress);
            g.fillRect(progressArea.removeFromLeft(progressWidth));
            
        }

        void startLoading()
        {
            progress = 0.0f;
            startTimer(50); // Update alle 50ms
        }

        void timerCallback() override
        {
            progress += 0.02f; // Simuliere Ladevorgang

            if (progress >= 1.0f)
            {
                progress = 1.0f;
                stopTimer();

                // Kurz warten, dann schlieﬂen
                juce::Timer::callAfterDelay(500, [this]()
                    {
                        if (onLoadingComplete)
                            onLoadingComplete();
                    });
            }

            repaint();
        }

        std::function<void()> onLoadingComplete;

    private:
        float progress = 0.0f;
        juce::Image logo;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplashScreen)
    };

}

//==============================================================================
class RadioBlastApplication : public juce::JUCEApplication
{
public:
    RadioBlastApplication() {}

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        // Erstelle und zeige Splash Screen
        splashScreen = std::make_unique<RadioBlast::SplashScreen>();
        splashScreen->onLoadingComplete = [this]()
            {
                // Wenn Laden abgeschlossen ist, erstelle Hauptfenster
                createMainWindow();
                // Schlieﬂe Splash Screen
                splashScreen.reset();
            };

        // Starte Ladevorgang
        splashScreen->startLoading();
    }

    void shutdown() override
    {
        splashScreen.reset();
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
    }

private:
    void createMainWindow()
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    class MainWindow : public juce::DocumentWindow
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
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<RadioBlast::SplashScreen> splashScreen;
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(RadioBlastApplication)
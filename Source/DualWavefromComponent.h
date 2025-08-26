/*
  ==============================================================================
    DualWaveformComponent.h
    Created: 26 Aug 2025
    Author:  mpue

    Dual-Deck Waveform Display mit Zoom, Scrubbing und visuelles Beatmatching
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class DualWaveformComponent : public juce::Component,
    public juce::Timer,
    public juce::ChangeListener
{
public:
    // Callbacks für die beiden Decks
    std::function<void(int deck, double position)> onScrubbed;
    std::function<void(int deck, double position)> onPositionClicked;

    enum class SyncMode
    {
        None,
        BPMSync,
        PhaseSync
    };

    DualWaveformComponent()
    {
        setSize(800, 300);

        // Initialize waveform data
        deck1Waveform.clear();
        deck2Waveform.clear();

        // Default values
        deck1Position = 0.0;
        deck2Position = 0.0;
        deck1Length = 0.0;
        deck2Length = 0.0;

        zoomFactor = 1.0;
        viewStart = 0.0;

        isDragging = false;
        dragDeck = -1;

        syncMode = SyncMode::None;

        // Start timer for smooth playback position updates
        startTimerHz(60); // 60 FPS

        setMouseClickGrabsKeyboardFocus(true);
        setWantsKeyboardFocus(true);
    }

    ~DualWaveformComponent() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Background
        g.fillAll(juce::Colour(0xff222222));

        // Split area for both decks
        auto deck1Area = bounds.removeFromTop(bounds.getHeight() / 2);
        auto deck2Area = bounds;

        // Draw separator line
        g.setColour(juce::Colours::grey);
        g.drawHorizontalLine(deck1Area.getBottom(), 0.0f, (float)getWidth());

        // Draw deck labels
        g.setColour(juce::Colours::lightgrey);
        g.setFont(12.0f);
        g.drawText("DECK A", deck1Area.removeFromLeft(60), juce::Justification::centred);
        g.drawText("DECK B", deck2Area.removeFromLeft(60), juce::Justification::centred);

        // Draw waveforms
        drawWaveform(g, deck1Area, deck1Waveform, deck1Position, deck1Length, juce::Colours::cyan, 0);
        drawWaveform(g, deck2Area, deck2Waveform, deck2Position, deck2Length, juce::Colours::orange, 1);

        // Draw sync indicators
        drawSyncIndicators(g, bounds);

        // Draw zoom and time info
        drawInfoOverlay(g);
    }

    void resized() override
    {
        // Component resized - might need to recalculate waveform display
    }

    // === WAVEFORM DATA ===
    void setWaveformData(int deck, const std::vector<float>& waveformData, double lengthInSeconds)
    {
        if (deck == 0)
        {
            deck1Waveform = waveformData;
            deck1Length = lengthInSeconds;
        }
        else if (deck == 1)
        {
            deck2Waveform = waveformData;
            deck2Length = lengthInSeconds;
        }
        repaint();
    }

    void clearWaveform(int deck)
    {
        if (deck == 0)
        {
            deck1Waveform.clear();
            deck1Length = 0.0;
            deck1Position = 0.0;
        }
        else if (deck == 1)
        {
            deck2Waveform.clear();
            deck2Length = 0.0;
            deck2Position = 0.0;
        }
        repaint();
    }

    // === PLAYBACK POSITION ===
    void setPlaybackPosition(int deck, double positionInSeconds)
    {
        if (deck == 0)
        {
            deck1Position = positionInSeconds;
        }
        else if (deck == 1)
        {
            deck2Position = positionInSeconds;
        }

        // Auto-scroll to keep playback position visible
        autoScrollToPosition(deck, positionInSeconds);

        repaint();
    }

    double getPlaybackPosition(int deck) const
    {
        return (deck == 0) ? deck1Position : deck2Position;
    }

    // === ZOOM CONTROLS ===
    void setZoom(double newZoomFactor)
    {
        zoomFactor = juce::jlimit(0.1, 10.0, newZoomFactor);
        repaint();
    }

    void zoomIn()
    {
        setZoom(zoomFactor * 1.5);
    }

    void zoomOut()
    {
        setZoom(zoomFactor / 1.5);
    }

    void zoomToFit()
    {
        double maxLength = juce::jmax(deck1Length, deck2Length);
        if (maxLength > 0.0)
        {
            zoomFactor = 1.0;
            viewStart = 0.0;
            repaint();
        }
    }

    // === SYNC FEATURES ===
    void setSyncMode(SyncMode mode)
    {
        syncMode = mode;
        repaint();
    }

    SyncMode getSyncMode() const
    {
        return syncMode;
    }

    // === MOUSE HANDLING ===
    void mouseDown(const juce::MouseEvent& e) override
    {
        auto bounds = getLocalBounds();
        auto deck1Area = bounds.removeFromTop(bounds.getHeight() / 2);
        auto deck2Area = bounds;

        // Remove label areas
        deck1Area.removeFromLeft(60);
        deck2Area.removeFromLeft(60);

        int clickedDeck = -1;
        double clickPosition = 0.0;

        if (deck1Area.contains(e.getPosition()))
        {
            clickedDeck = 0;
            clickPosition = pixelToTime(e.x - 60, deck1Length);
        }
        else if (deck2Area.contains(e.getPosition()))
        {
            clickedDeck = 1;
            clickPosition = pixelToTime(e.x - 60, deck2Length);
        }

        if (clickedDeck >= 0)
        {
            if (e.mods.isRightButtonDown())
            {
                // Right click - set cue point or other actions
                showContextMenu(clickedDeck, clickPosition);
            }
            else
            {
                // Left click - start scrubbing
                isDragging = true;
                dragDeck = clickedDeck;
                lastDragPosition = clickPosition;

                if (onPositionClicked)
                {
                    onPositionClicked(clickedDeck, clickPosition);
                }
            }
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (isDragging && dragDeck >= 0)
        {
            auto bounds = getLocalBounds();
            if (dragDeck == 1)
            {
                bounds.removeFromTop(bounds.getHeight() / 2);
            }
            else
            {
                bounds.removeFromBottom(bounds.getHeight() / 2);
            }
            bounds.removeFromLeft(60);

            double length = (dragDeck == 0) ? deck1Length : deck2Length;
            double newPosition = pixelToTime(e.x - 60, length);

            newPosition = juce::jlimit(0.0, length, newPosition);

            if (dragDeck == 0)
            {
                deck1Position = newPosition;
            }
            else
            {
                deck2Position = newPosition;
            }

            if (onScrubbed)
            {
                onScrubbed(dragDeck, newPosition);
            }

            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        isDragging = false;
        dragDeck = -1;
    }

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (e.mods.isCtrlDown())
        {
            // Zoom with Ctrl+Wheel
            double zoomDelta = wheel.deltaY * 0.1;
            setZoom(zoomFactor + zoomDelta);
        }
        else
        {
            // Horizontal scroll
            double maxLength = juce::jmax(deck1Length, deck2Length);
            double scrollDelta = wheel.deltaY * maxLength * 0.1;
            viewStart = juce::jlimit(0.0, maxLength - (maxLength / zoomFactor), viewStart - scrollDelta);
            repaint();
        }
    }

    // === KEYBOARD SHORTCUTS ===
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::spaceKey)
        {
            // Toggle play/pause - would need callback to main component
            return true;
        }
        else if (key.getKeyCode() == '+' || key.getKeyCode() == '=')
        {
            zoomIn();
            return true;
        }
        else if (key.getKeyCode() == '-')
        {
            zoomOut();
            return true;
        }
        else if (key.getKeyCode() == '0')
        {
            zoomToFit();
            return true;
        }

        return false;
    }

    // === TIMER CALLBACK ===
    void timerCallback() override
    {
        // Smooth updates for playback position
        repaint();
    }

    // === CHANGE LISTENER ===
    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        // Handle audio format reader updates or other changes
        repaint();
    }

private:
    // Waveform data
    std::vector<float> deck1Waveform;
    std::vector<float> deck2Waveform;

    // Playback positions and lengths
    double deck1Position, deck2Position;
    double deck1Length, deck2Length;

    // View control
    double zoomFactor;
    double viewStart;

    // Interaction state
    bool isDragging;
    int dragDeck;
    double lastDragPosition;

    // Sync features
    SyncMode syncMode;

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> area,
        const std::vector<float>& waveform, double position,
        double length, juce::Colour colour, int deckIndex)
    {
        if (waveform.empty() || length <= 0.0) return;

        // Background for waveform area
        g.setColour(juce::Colour(0xff333333));
        g.fillRect(area);

        // Calculate visible time range
        double visibleLength = length / zoomFactor;
        double startTime = viewStart;
        double endTime = startTime + visibleLength;

        // Draw waveform
        g.setColour(colour.withAlpha(0.8f));

        int waveformSize = (int)waveform.size();
        float width = (float)area.getWidth();
        float height = (float)area.getHeight();
        float centerY = area.getY() + height * 0.5f;

        juce::Path waveformPath;
        bool pathStarted = false;

        for (int x = 0; x < area.getWidth(); ++x)
        {
            double time = startTime + (x / width) * visibleLength;
            if (time >= 0.0 && time < length)
            {
                int sampleIndex = (int)((time / length) * waveformSize);
                if (sampleIndex >= 0 && sampleIndex < waveformSize)
                {
                    float sample = waveform[sampleIndex];
                    float y = centerY - (sample * height * 0.4f);

                    if (!pathStarted)
                    {
                        waveformPath.startNewSubPath((float)(area.getX() + x), y);
                        pathStarted = true;
                    }
                    else
                    {
                        waveformPath.lineTo((float)(area.getX() + x), y);
                    }
                }
            }
        }

        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));

        // Draw playback position marker
        drawPlaybackMarker(g, area, position, startTime, endTime, colour);

        // Draw time grid
        drawTimeGrid(g, area, startTime, endTime);
    }

    void drawPlaybackMarker(juce::Graphics& g, juce::Rectangle<int> area,
        double position, double startTime, double endTime,
        juce::Colour colour)
    {
        if (position >= startTime && position <= endTime)
        {
            double relativePos = (position - startTime) / (endTime - startTime);
            int x = area.getX() + (int)(relativePos * area.getWidth());

            g.setColour(colour);
            g.drawVerticalLine(x, (float)area.getY(), (float)area.getBottom());

            // Draw position indicator
            g.setColour(colour.brighter());
            g.fillRect(x - 1, area.getY(), 3, 6);
            g.fillRect(x - 1, area.getBottom() - 6, 3, 6);
        }
    }

    void drawTimeGrid(juce::Graphics& g, juce::Rectangle<int> area,
        double startTime, double endTime)
    {
        g.setColour(juce::Colours::grey.withAlpha(0.3f));

        double duration = endTime - startTime;
        double gridInterval = 1.0; // 1 second intervals

        // Adjust grid interval based on zoom
        if (duration < 10.0) gridInterval = 1.0;
        else if (duration < 60.0) gridInterval = 5.0;
        else if (duration < 300.0) gridInterval = 15.0;
        else gridInterval = 30.0;

        double firstGrid = std::ceil(startTime / gridInterval) * gridInterval;

        for (double time = firstGrid; time < endTime; time += gridInterval)
        {
            double relativePos = (time - startTime) / duration;
            int x = area.getX() + (int)(relativePos * area.getWidth());

            g.drawVerticalLine(x, (float)area.getY(), (float)area.getBottom());

            // Draw time labels
            g.setColour(juce::Colours::lightgrey);
            g.setFont(10.0f);
            juce::String timeStr = formatTime(time);
            g.drawText(timeStr, x + 2, area.getY(), 50, 12, juce::Justification::left);
        }
    }

    void drawSyncIndicators(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (syncMode == SyncMode::None) return;

        g.setColour(juce::Colours::yellow);
        g.setFont(12.0f);

        juce::String syncText;
        switch (syncMode)
        {
        case SyncMode::BPMSync: syncText = "BPM SYNC"; break;
        case SyncMode::PhaseSync: syncText = "PHASE SYNC"; break;
        default: break;
        }

        g.drawText(syncText, bounds.getWidth() - 100, 5, 95, 20, juce::Justification::right);
    }

    void drawInfoOverlay(juce::Graphics& g)
    {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(11.0f);

        juce::String zoomInfo = "Zoom: " + juce::String(zoomFactor, 1) + "x";
        g.drawText(zoomInfo, 10, getHeight() - 20, 100, 15, juce::Justification::left);

        juce::String controlInfo = "Ctrl+Wheel: Zoom | Wheel: Scroll | Right-click: Menu";
        g.drawText(controlInfo, getWidth() - 300, getHeight() - 20, 295, 15, juce::Justification::right);
    }

    double pixelToTime(int pixel, double totalLength) const
    {
        if (totalLength <= 0.0) return 0.0;

        double visibleLength = totalLength / zoomFactor;
        double relativePos = (double)pixel / (getWidth() - 60);
        return viewStart + relativePos * visibleLength;
    }

    int timeToPixel(double time, double totalLength) const
    {
        if (totalLength <= 0.0) return 0;

        double visibleLength = totalLength / zoomFactor;
        double relativePos = (time - viewStart) / visibleLength;
        return 60 + (int)(relativePos * (getWidth() - 60));
    }

    void autoScrollToPosition(int deck, double position)
    {
        double length = (deck == 0) ? deck1Length : deck2Length;
        if (length <= 0.0) return;

        double visibleLength = length / zoomFactor;

        // Check if position is outside visible area
        if (position < viewStart || position > viewStart + visibleLength)
        {
            // Center the position in the view
            viewStart = position - visibleLength * 0.5;
            viewStart = juce::jlimit(0.0, length - visibleLength, viewStart);
        }
    }

    juce::String formatTime(double seconds) const
    {
        int mins = (int)(seconds / 60.0);
        int secs = (int)(seconds) % 60;
        return juce::String::formatted("%d:%02d", mins, secs);
    }

    void showContextMenu(int deck, double position)
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Set Cue Point");
        menu.addItem(2, "Set Loop In");
        menu.addItem(3, "Set Loop Out");
        menu.addSeparator();
        menu.addItem(4, "Sync to Other Deck");
        menu.addItem(5, "Reset Zoom");

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this, deck, position](int result) {
                switch (result)
                {
                case 1:
                    // Set cue point - would need callback
                    break;
                case 2:
                    // Set loop in
                    break;
                case 3:
                    // Set loop out
                    break;
                case 4:
                    // Sync decks
                    if (syncMode == SyncMode::None)
                        setSyncMode(SyncMode::BPMSync);
                    else
                        setSyncMode(SyncMode::None);
                    break;
                case 5:
                    zoomToFit();
                    break;
                }
            });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualWaveformComponent)
};
/*==============================================================================
  DualWaveformComponent.h
  Created: 26 Aug 2025
  Author:  mpue

  Enhanced Dual-Deck Waveform Display mit echtem Scrubbing und Sync
==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "WaveformGenerator.h"

class DualWaveformComponent : public juce::Component,
    public juce::Timer,
    public juce::ChangeListener
{
public:
    // Enhanced callbacks für bessere Integration
    std::function<void(int deck, double position, bool isPlaying)> onPositionChanged;
    std::function<void(int deck, double position)> onScrubStart;
    std::function<void(int deck, double position)> onScrubEnd;
    std::function<void(int deck, double position)> onPositionClicked;
    std::function<double()> getCurrentTime; // Für smooth playback updates

    enum class SyncMode
    {
        None,
        BPMSync,
        PhaseSync
    };

    DualWaveformComponent()
    {
        setSize(800, 300);

        // Default values
        deck1Position = 0.0;
        deck2Position = 0.0;
        deck1Length = 0.0;
        deck2Length = 0.0;

        // Playback states
        deck1Playing = false;
        deck2Playing = false;
        deck1Speed = 1.0;
        deck2Speed = 1.0;

        zoomFactor = 1.0;
        viewStart = 0.0;

        // Scrubbing state
        isDragging = false;
        dragDeck = -1;
        wasScrubbing = false;
        scrubStartPosition = 0.0;

        // Wave alignment for beatmatching
        isAligning = false;
        alignmentOffset = 0.0;
        lastAlignmentOffset = 0.0;

        syncMode = SyncMode::None;

        // Start timer for smooth updates
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

        // Draw deck labels with play indicators
        drawDeckLabels(g, deck1Area, deck2Area);

        // Remove label areas from waveform drawing
        deck1Area.removeFromLeft(60);
        deck2Area.removeFromLeft(60);

        WaveformGenerator::drawWaveform(g, deck1Waveform, juce::Rectangle<float>(deck1Area.toFloat()),juce::Colours::grey.withAlpha(0.7f),zoomFactor);
        WaveformGenerator::drawWaveform(g, deck2Waveform, juce::Rectangle<float>(deck2Area.toFloat()), juce::Colours::grey.withAlpha(0.7f), zoomFactor);

        // Draw playback position marker
        


        // Draw alignment indicators if in alignment mode
        if (isAligning)
        {
            drawAlignmentIndicators(g, deck1Area, 0);
            drawAlignmentIndicators(g, deck2Area, 1);
        }

        // Draw sync indicators
        drawSyncIndicators(g, bounds);

        // Draw zoom and time info
        drawInfoOverlay(g);


    }

    void resized() override
    {
        // Component resized - might need to recalculate waveform display
    }

    // === ENHANCED WAVEFORM DATA ===
    void setWaveformData(int deck, WaveformGenerator::WaveformData data)
    {


        if (deck == 0)
        {
            deck1Waveform = data;
			deck1Length = data.duration;
        }
        else if (deck == 1)
        {
            deck2Waveform = data;
            deck2Length = data.duration;
        }
        repaint();
    }

    void clearWaveform(int deck)
    {
        if (deck == 0)
        {

            deck1Length = 0.0;
            deck1Position = 0.0;
            deck1Playing = false;
        }
        else if (deck == 1)
        {

            deck2Length = 0.0;
            deck2Position = 0.0;
            deck2Playing = false;
        }
        repaint();
    }

    // === ENHANCED PLAYBACK CONTROL ===
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

        // Auto-scroll nur wenn nicht gerade gescrubbed wird
        if (!isDragging || dragDeck != deck)
        {
            autoScrollToPosition(deck, positionInSeconds);
        }

        repaint();
    }

    void setPlaybackState(int deck, bool isPlaying, double speed = 1.0)
    {
        if (deck == 0)
        {
            deck1Playing = isPlaying;
            deck1Speed = speed;
        }
        else if (deck == 1)
        {
            deck2Playing = isPlaying;
            deck2Speed = speed;
        }
        repaint();
    }

    bool isPlaying(int deck) const
    {
        return (deck == 0) ? deck1Playing : deck2Playing;
    }

    double getPlaybackPosition(int deck) const
    {
        return (deck == 0) ? deck1Position : deck2Position;
    }

    double getSpeed(int deck) const
    {
        return (deck == 0) ? deck1Speed : deck2Speed;
    }

    // === ZOOM CONTROLS ===
    void setZoom(double newZoomFactor)
    {
        zoomFactor = juce::jlimit(0.1, 20.0, newZoomFactor);
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

    // === BPM CONTROL ===
    void setBPM(int deck, double bpm)
    {
        if (deck == 0)
        {
            deck1BPM = bpm;
        }
        else if (deck == 1)
        {
            deck2BPM = bpm;
        }
        repaint();
    }

    double getBPM(int deck) const
    {
        return (deck == 0) ? deck1BPM : deck2BPM;
    }

    // === ENHANCED MOUSE HANDLING ===
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
                showContextMenu(clickedDeck, clickPosition);
            }
            else
            {
                // Start scrubbing
                startScrubbing(clickedDeck, clickPosition);
            }
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (isDragging && dragDeck >= 0)
        {
            double length = (dragDeck == 0) ? deck1Length : deck2Length;
            double newPosition = pixelToTime(e.x - 60, length);
            newPosition = juce::jlimit(0.0, length, newPosition);

            // Update position immediately for responsive scrubbing
            if (dragDeck == 0)
            {
                deck1Position = newPosition;
            }
            else
            {
                deck2Position = newPosition;
            }

            // Notify about position change during scrubbing
            if (onPositionChanged)
            {
                onPositionChanged(dragDeck, newPosition, false); // Not playing during scrub
            }

            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (isDragging && dragDeck >= 0)
        {
            endScrubbing(dragDeck);
        }
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
        // Update positions for smooth playback only when not scrubbing
        if (!isDragging)
        {
            repaint();
        }
    }

    // === CHANGE LISTENER ===
    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        repaint();
    }

private:
    // Waveform data
    WaveformGenerator::WaveformData deck1Waveform;
    WaveformGenerator::WaveformData deck2Waveform;

    // Playback positions and lengths
    double deck1Position, deck2Position;
    double deck1Length, deck2Length;

    // Playback states
    bool deck1Playing, deck2Playing;
    double deck1Speed, deck2Speed;

    // View control
    double zoomFactor;
    double viewStart;

    // Enhanced scrubbing state
    bool isDragging;
    int dragDeck;
    bool wasScrubbing;
    double scrubStartPosition;

    // Wave alignment for beatmatching
    bool isAligning;
    double alignmentOffset;
    double lastAlignmentOffset;
    juce::Point<int> alignmentStartPos;

    // Sync features
    SyncMode syncMode;

    // BPM values for display
    double deck1BPM = 0.0;
    double deck2BPM = 0.0;

    void startScrubbing(int deck, double position)
    {
        isDragging = true;
        dragDeck = deck;
        scrubStartPosition = position;
        wasScrubbing = (deck == 0) ? deck1Playing : deck2Playing;

        // Set position immediately
        if (deck == 0)
        {
            deck1Position = position;
        }
        else
        {
            deck2Position = position;
        }

        // Notify scrub start
        if (onScrubStart)
        {
            onScrubStart(deck, position);
        }

        if (onPositionClicked)
        {
            onPositionClicked(deck, position);
        }

        repaint();
    }

    void endScrubbing(int deck)
    {
        double finalPosition = (deck == 0) ? deck1Position : deck2Position;

        isDragging = false;
        dragDeck = -1;

        // Notify scrub end
        if (onScrubEnd)
        {
            onScrubEnd(deck, finalPosition);
        }

        // Resume playback state if it was playing before scrubbing
        if (onPositionChanged)
        {
            onPositionChanged(deck, finalPosition, wasScrubbing);
        }

        wasScrubbing = false;
    }

    // === WAVE ALIGNMENT FOR BEATMATCHING ===
    void startWaveAlignment(juce::Point<int> startPos)
    {
        isAligning = true;
        alignmentStartPos = startPos;
        lastAlignmentOffset = alignmentOffset;

        // Visual feedback
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        repaint();
    }

    void handleWaveAlignment(const juce::MouseEvent& e)
    {
        if (!isAligning) return;

        // Calculate horizontal drag distance
        int dragDistance = e.getPosition().x - alignmentStartPos.x;

        // Convert pixels to time offset (sensitivity factor)
        double maxLength = juce::jmax(deck1Length, deck2Length);
        double visibleLength = maxLength / zoomFactor;
        double timePerPixel = visibleLength / (getWidth() - 60);

        // Update alignment offset
        alignmentOffset = lastAlignmentOffset + (dragDistance * timePerPixel);

        // Limit alignment range (±30 seconds)
        alignmentOffset = juce::jlimit(-30.0, 30.0, alignmentOffset);

        repaint();
    }

    void endWaveAlignment()
    {
        isAligning = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);

        // Apply the alignment offset to the audio engine
        if (onPositionChanged && alignmentOffset != 0.0)
        {
            // Notify about alignment change - could be used to adjust one deck's position
            // The receiving component can decide which deck to adjust
            if (std::abs(alignmentOffset) > 0.01) // Minimum threshold
            {
                // Could trigger a nudge/slip on the appropriate deck
                // Implementation depends on your audio engine
            }
        }
    }

    void resetAlignment()
    {
        alignmentOffset = 0.0;
        lastAlignmentOffset = 0.0;
        repaint();
    }

    double getAlignmentOffset() const
    {
        return alignmentOffset;
    }

    void drawDeckLabels(juce::Graphics& g, juce::Rectangle<int> deck1Area, juce::Rectangle<int> deck2Area)
    {
        auto label1Area = deck1Area.removeFromLeft(60);
        auto label2Area = deck2Area.removeFromLeft(60);

        // Deck A Label
        g.setColour(deck1Playing ? juce::Colours::cyan : juce::Colours::lightgrey);
        g.setFont(12.0f);
        g.drawText("DECK A", label1Area, juce::Justification::centred);

        // Play indicator
        if (deck1Playing)
        {
            g.setColour(juce::Colours::cyan);
            g.fillEllipse(label1Area.getX() + 5, label1Area.getCentreY() - 3, 6, 6);
        }

        // Deck B Label  
        g.setColour(deck2Playing ? juce::Colours::orange : juce::Colours::lightgrey);
        g.drawText("DECK B", label2Area, juce::Justification::centred);

        // Play indicator
        if (deck2Playing)
        {
            g.setColour(juce::Colours::orange);
            g.fillEllipse(label2Area.getX() + 5, label2Area.getCentreY() - 3, 6, 6);
        }
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> area,
        const std::vector<float>& waveform, double position,
        double length, juce::Colour colour, int deckIndex, bool isPlaying)
    {
        
    }

    void drawPlaybackMarker(juce::Graphics& g, juce::Rectangle<int> area,
        double position, double startTime, double endTime,
        juce::Colour colour, bool isPlaying, double timeOffset = 0.0)
    {
        double adjustedPosition = position + timeOffset;
        if (adjustedPosition >= startTime && adjustedPosition <= endTime)
        {
            double relativePos = (adjustedPosition - startTime) / (endTime - startTime);
            int x = area.getX() + (int)(relativePos * area.getWidth());

            // Enhanced marker for playing state
            if (isPlaying)
            {
                g.setColour(colour.brighter());
                g.drawVerticalLine(x, (float)area.getY(), (float)area.getBottom());

                // Animated pulse effect for playing marker
                float pulseAlpha = 0.3f + 0.7f * (0.5f + 0.5f * std::sin(juce::Time::getMillisecondCounter() * 0.01f));
                g.setColour(colour.withAlpha(pulseAlpha));
                g.drawVerticalLine(x - 1, (float)area.getY(), (float)area.getBottom());
                g.drawVerticalLine(x + 1, (float)area.getY(), (float)area.getBottom());
            }
            else
            {
                g.setColour(colour);
                g.drawVerticalLine(x, (float)area.getY(), (float)area.getBottom());
            }

            // Position indicator triangles
            g.setColour(colour.brighter());
            juce::Path marker;
            marker.addTriangle(x - 4, area.getY(), x + 4, area.getY(), x, area.getY() + 8);
            marker.addTriangle(x - 4, area.getBottom(), x + 4, area.getBottom(), x, area.getBottom() - 8);
            g.fillPath(marker);
        }
    }

    void drawTimeGrid(juce::Graphics& g, juce::Rectangle<int> area,
        double startTime, double endTime)
    {
        g.setColour(juce::Colours::grey.withAlpha(0.3f));

        double duration = endTime - startTime;
        double gridInterval = 1.0;

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

            // Time labels
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

        juce::String controlInfo = "Left: Scrub | Middle: Align Waves | Ctrl+Wheel: Zoom";
        g.drawText(controlInfo, getWidth() - 300, getHeight() - 20, 295, 15, juce::Justification::right);

        // Show alignment offset if active
        if (std::abs(alignmentOffset) > 0.01 || isAligning)
        {
            g.setColour(juce::Colours::yellow);
            g.setFont(12.0f);
            juce::String offsetText = "Offset: " + juce::String(alignmentOffset, 3) + "s";
            if (isAligning) offsetText += " (Aligning...)";
            g.drawText(offsetText, getWidth() / 2 - 75, getHeight() - 40, 150, 15, juce::Justification::centred);
        }

        // Enhanced position display
        drawPositionDisplay(g);
    }

    void drawAlignmentIndicators(juce::Graphics& g, juce::Rectangle<int> area, int deckIndex)
    {
        // Draw alignment guides and offset indicators
        g.setColour(juce::Colours::yellow.withAlpha(0.7f));

        // Center line for reference
        int centerX = area.getCentreX();
        g.drawVerticalLine(centerX, (float)area.getY(), (float)area.getBottom());

        // Offset indicator
        if (deckIndex == 1 && std::abs(alignmentOffset) > 0.001) // Only show on deck B
        {
            double maxLength = juce::jmax(deck1Length, deck2Length);
            double visibleLength = maxLength / zoomFactor;
            double pixelsPerSecond = (getWidth() - 60) / visibleLength;
            int offsetPixels = (int)(alignmentOffset * pixelsPerSecond);

            int offsetX = centerX + offsetPixels;
            g.setColour(juce::Colours::red.withAlpha(0.8f));
            g.drawVerticalLine(offsetX, (float)area.getY(), (float)area.getBottom());

            // Draw arrow indicating direction
            juce::Path arrow;
            if (alignmentOffset > 0)
            {
                arrow.addTriangle(offsetX + 5, area.getCentreY() - 5,
                    offsetX + 5, area.getCentreY() + 5,
                    offsetX + 15, area.getCentreY());
            }
            else
            {
                arrow.addTriangle(offsetX - 5, area.getCentreY() - 5,
                    offsetX - 5, area.getCentreY() + 5,
                    offsetX - 15, area.getCentreY());
            }
            g.fillPath(arrow);
        }
    }

    void drawPositionDisplay(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();
        int deckHeight = bounds.getHeight() / 2;

        // Deck A Position Display
        drawDeckPositionDisplay(g, juce::Rectangle<int>(0, 0, getWidth(), deckHeight),
            deck1Position, deck1Length, juce::Colours::cyan, "A", deck1Playing, deck1Speed);

        // Deck B Position Display  
        drawDeckPositionDisplay(g, juce::Rectangle<int>(0, deckHeight, getWidth(), deckHeight),
            deck2Position, deck2Length, juce::Colours::orange, "B", deck2Playing, deck2Speed);
    }

    void drawDeckPositionDisplay(juce::Graphics& g, juce::Rectangle<int> area,
        double position, double length, juce::Colour colour,
        const juce::String& deckName, bool isPlaying, double speed)
    {
        if (length <= 0.0) return;

        // Enhanced display area
        auto displayArea = juce::Rectangle<int>(area.getRight() - 180, area.getY() + 5, 175, 30);

        // Background with glow effect for playing state
        if (isPlaying)
        {
            g.setColour(colour.withAlpha(0.1f));
            g.fillRoundedRectangle(displayArea.expanded(2).toFloat(), 4.0f);
        }

        g.setColour(juce::Colour(0xff1a1a1a).withAlpha(0.9f));
        g.fillRoundedRectangle(displayArea.toFloat(), 3.0f);

        // Enhanced border
        float borderWidth = isPlaying ? 2.0f : 1.0f;
        g.setColour(isPlaying ? colour.brighter() : colour);
        g.drawRoundedRectangle(displayArea.toFloat(), 3.0f, borderWidth);

        // Position Text with speed indicator
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain));

        juce::String positionText = formatTime(position) + " / " + formatTime(length);
        if (speed != 1.0)
        {
            positionText += " (" + juce::String(speed, 2) + "x)";
        }

        auto textArea = displayArea.reduced(5).removeFromTop(15);
        g.drawText(positionText, textArea, juce::Justification::centred);

        // Deck Label
        g.setColour(colour);
        g.setFont(10.0f);
        g.drawText(deckName, displayArea.getX() - 15, displayArea.getY() + 2, 12, 12, juce::Justification::centred);

        // Enhanced progress bar
        drawProgressBar(g, displayArea, position, length, colour, isPlaying);

        // BPM Display
        drawBPMDisplay(g, displayArea, deckName == "A" ? deck1BPM : deck2BPM, colour, isPlaying);
    }

    void drawProgressBar(juce::Graphics& g, juce::Rectangle<int> displayArea,
        double position, double length, juce::Colour colour, bool isPlaying)
    {
        auto progressArea = juce::Rectangle<int>(displayArea.getX() + 5,
            displayArea.getBottom() - 10,
            displayArea.getWidth() - 10, 4);

        // Background
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(progressArea.toFloat(), 2.0f);

        // Progress with enhanced visuals for playing state
        if (length > 0.0)
        {
            double progress = juce::jlimit(0.0, 1.0, position / length);
            int progressWidth = (int)(progress * progressArea.getWidth());

            auto filledArea = progressArea.withWidth(progressWidth);

            if (isPlaying)
            {
                // Gradient for playing state
                juce::ColourGradient gradient(colour.darker(), filledArea.getX(), filledArea.getY(),
                    colour.brighter(), filledArea.getRight(), filledArea.getY(), false);
                g.setGradientFill(gradient);
            }
            else
            {
                g.setColour(colour);
            }

            g.fillRoundedRectangle(filledArea.toFloat(), 2.0f);
        }
    }

    void drawBPMDisplay(juce::Graphics& g, juce::Rectangle<int> displayArea,
        double bpm, juce::Colour colour, bool isPlaying)
    {
        if (bpm <= 0.0) return;

        auto bpmArea = juce::Rectangle<int>(displayArea.getX() - 55, displayArea.getY(), 50, 12);

        g.setColour(isPlaying ? colour : colour.withAlpha(0.8f));
        g.setFont(9.0f);
        juce::String bpmText = juce::String(bpm, 1) + " BPM";
        g.drawText(bpmText, bpmArea, juce::Justification::centred);
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
        int ms = (int)((seconds - (int)seconds) * 100);
        return juce::String::formatted("%d:%02d.%02d", mins, secs, ms);
    }

    void showContextMenu(int deck, double position)
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Set Cue Point");
        menu.addItem(2, "Set Loop In");
        menu.addItem(3, "Set Loop Out");
        menu.addSeparator();
        menu.addItem(4, "Jump to Position");
        menu.addItem(5, isPlaying(deck) ? "Pause" : "Play");
        menu.addSeparator();
        menu.addItem(6, "Sync to Other Deck");
        menu.addItem(7, "Reset Zoom");
        menu.addItem(8, "Fit to View");
        menu.addSeparator();
        menu.addItem(9, "Reset Wave Alignment");
        menu.addItem(10, "Fine Nudge Left");
        menu.addItem(11, "Fine Nudge Right");

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this, deck, position](int result) {
                switch (result)
                {
                case 1:
                    // Set cue point - implementiert in der Hauptkomponente
                    if (onPositionClicked)
                        onPositionClicked(deck, position);
                    break;
                case 2:
                    // Set loop in
                    break;
                case 3:
                    // Set loop out
                    break;
                case 4:
                    // Jump to position
                    setPlaybackPosition(deck, position);
                    if (onPositionChanged)
                        onPositionChanged(deck, position, isPlaying(deck));
                    break;
                case 5:
                    // Toggle play/pause
                    setPlaybackState(deck, !isPlaying(deck));
                    if (onPositionChanged)
                        onPositionChanged(deck, getPlaybackPosition(deck), isPlaying(deck));
                    break;
                case 6:
                    // Sync decks
                    if (syncMode == SyncMode::None)
                        setSyncMode(SyncMode::BPMSync);
                    else
                        setSyncMode(SyncMode::None);
                    break;
                case 7:
                    zoomToFit();
                    break;
                case 8:
                    // Fit current playing position to center
                    if (isPlaying(deck))
                    {
                        double pos = getPlaybackPosition(deck);
                        double length = (deck == 0) ? deck1Length : deck2Length;
                        viewStart = pos - (length / zoomFactor) * 0.5;
                        viewStart = juce::jlimit(0.0, length - (length / zoomFactor), viewStart);
                        repaint();
                    }
                    break;
                case 9:
                    // Reset wave alignment
                    resetAlignment();
                    break;
                case 10:
                    // Fine nudge left (0.01 seconds)
                    alignmentOffset -= 0.01;
                    alignmentOffset = juce::jlimit(-30.0, 30.0, alignmentOffset);
                    repaint();
                    break;
                case 11:
                    // Fine nudge right (0.01 seconds)
                    alignmentOffset += 0.01;
                    alignmentOffset = juce::jlimit(-30.0, 30.0, alignmentOffset);
                    repaint();
                    break;
                }
            });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualWaveformComponent);

 };

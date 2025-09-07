/*
  ==============================================================================

    CyberpunkDJLookAndFeel.h
    Created: 29 Aug 2025 12:53:49pm
    Author:  mpue

  ==============================================================================
*/

/*
  ==============================================================================

    CyberpunkDJLookAndFeel.h
    Created: Neural Audio Matrix Interface
    Author:  mpue - Quantum Audio Engineer

  ==============================================================================
*/

// CyberpunkDJLookAndFeel.h - Futuristic Neural Audio Interface für DJ-Applikation
#pragma once
#include <JuceHeader.h>

class CyberpunkDJLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CyberpunkDJLookAndFeel()
    {
        // === NEURAL AUDIO MATRIX FARBPALETTE ===
        // Quantum Background Colors
        primaryBg = juce::Colour(0xff020510);      // Ultra-dunkles Neural-Blau
        secondaryBg = juce::Colour(0xff0a1020);    // Mittleres Neural-Blau
        tertiaryBg = juce::Colour(0xff132030);     // Helles Neural-Blau

        // Quantum Accent Colors - Audio Matrix
        accentColor = juce::Colour(0xff00ff88);    // Neural-Grün (Primary)
        secondaryAccent = juce::Colour(0xff00ccff); // Quantum-Cyan
        tertiaryAccent = juce::Colour(0xff88ffaa);  // Holo-Grün

        // Audio Frequency Colors
        audioFreqLow = juce::Colour(0xffff0080);    // Bass = Pink
        audioFreqMid = juce::Colour(0xff00ff88);    // Mid = Grün
        audioFreqHigh = juce::Colour(0xff0088ff);   // High = Blau

        // Status Colors
        warningColor = juce::Colour(0xffff3366);    // Neural-Rot für Warnings
        successColor = juce::Colour(0xff00ff44);    // Quantum-Grün für Success

        // Text Colors - Neural Matrix
        primaryText = juce::Colour(0xffffffff);     // Weiß
        secondaryText = juce::Colour(0xffaaffcc);   // Holo-Grün-Weiß
        mutedText = juce::Colour(0xff66ccaa);       // Gedämpftes Neural-Grün
        borderColor = juce::Colour(0xff2a5544);     // Neural-Border

        // Glow Effect
        glowColor = juce::Colour(0x4400ff88);       // Transparenter Glow

        // === STANDARD JUCE FARBEN SETZEN ===
        setColour(juce::ResizableWindow::backgroundColourId, primaryBg);
        setColour(juce::DocumentWindow::backgroundColourId, primaryBg);
        setColour(juce::DocumentWindow::textColourId, primaryText);

        // Slider - Neural Quantum Colors
        setColour(juce::Slider::backgroundColourId, secondaryBg);
        setColour(juce::Slider::thumbColourId, accentColor);
        setColour(juce::Slider::trackColourId, tertiaryBg);
        setColour(juce::Slider::rotarySliderFillColourId, accentColor);
        setColour(juce::Slider::rotarySliderOutlineColourId, borderColor);

        // Button - Cyberpunk Style
        setColour(juce::TextButton::buttonColourId, secondaryBg);
        setColour(juce::TextButton::buttonOnColourId, accentColor);
        setColour(juce::TextButton::textColourOffId, primaryText);
        setColour(juce::TextButton::textColourOnId, primaryBg);

        // ComboBox - Neural Interface
        setColour(juce::ComboBox::backgroundColourId, secondaryBg);
        setColour(juce::ComboBox::textColourId, primaryText);
        setColour(juce::ComboBox::outlineColourId, borderColor);
        setColour(juce::ComboBox::arrowColourId, accentColor);

        // Label - Quantum Text
        setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, primaryText);

        // PopupMenu - Neural Matrix
        setColour(juce::PopupMenu::backgroundColourId, secondaryBg);
        setColour(juce::PopupMenu::textColourId, primaryText);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accentColor);
        setColour(juce::PopupMenu::highlightedTextColourId, primaryBg);

        // Additional Components
        setColour(juce::GroupComponent::textColourId, primaryText);
        setColour(juce::GroupComponent::outlineColourId, borderColor);
        setColour(juce::ListBox::backgroundColourId, secondaryBg);
    }

    void CyberpunkDJLookAndFeel::drawRotarySlider(Graphics& g,
        int     x,
        int     y,
        int     width,
        int     height,
        float     sliderPosProportional,
        float     rotaryStartAngle,
        float     rotaryEndAngle,
        Slider& slider) override
    {
        //LookAndFeel_V2::drawRotarySlider(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider);

        // This is the binary image data that uses very little CPU when rotating
        Image myStrip = ImageCache::getFromMemory(BinaryData::Knob_64_png, BinaryData::Knob_64_pngSize);

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float radius = jmin(width / 2.0f, height / 2.0f);
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius - 1.0f;
        const float ry = centreY - radius - 1.0f;
        const float rw = radius * 2.0f;
        const float thickness = 0.9f;

        // const double fractRotation = (slider.getValue() - slider.getMinimum())  /
        //(slider.getMaximum() - slider.getMinimum()); //value between 0 and 1 for current amount of rotation

        const int nFrames = myStrip.getHeight() / myStrip.getWidth(); // number of frames for vertical film strip
        const int frameIdx = (int)ceil(sliderPosProportional * ((double)nFrames - 1.0)); // current index from 0 --> nFrames-1

        // Logger::getCurrentLogger()->writeToLog("==========" +String(sliderPosProportional));

        g.setColour(Colours::darkorange);
        {
            Path filledArc;

            float dist = angle - rotaryStartAngle;

            for (float a = rotaryStartAngle; a < dist + rotaryStartAngle; a += 0.15) {
                filledArc.addPieSegment(rx + 1, ry + 1, rw - 0.5, rw - 0.5, a, a + 0.1, thickness);
            }

            // filledArc.addPieSegment (rx+1, ry+1, rw-0.5, rw-0.5, rotaryStartAngle, angle, thickness);

            g.fillPath(filledArc);
        }

        g.drawImage(myStrip,
            (int)rx,
            (int)ry,
            2 * (int)radius,
            2 * (int)radius,   //Dest
            0,
            frameIdx * myStrip.getWidth(),
            myStrip.getWidth(),
            myStrip.getWidth()); //Source

        
    }

    // === NEURAL AUDIO SLIDER SYSTEM ===
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackBounds = juce::Rectangle<int>(x, y, width, height);

        if (style == juce::Slider::LinearVertical)
        {
            drawNeuralChannelFader(g, trackBounds, sliderPos, slider);
        }
        else if (style == juce::Slider::LinearHorizontal)
        {
            if (slider.getName().contains("crossfader") || slider.getName().contains("cross"))
            {
                drawQuantumCrossfader(g, trackBounds, sliderPos, slider);
            }
            else
            {
                drawNeuralHorizontalSlider(g, trackBounds, sliderPos, slider);
            }
        }
    }

    // === NEURAL CHANNEL FADER (Quantum Vertical Slider) ===
    void drawNeuralChannelFader(juce::Graphics& g, const juce::Rectangle<int>& bounds, float sliderPos, juce::Slider& slider)
    {
        auto trackWidth = 14;  // Breitere Neural-Spur
        auto trackArea = bounds.withSizeKeepingCentre(trackWidth, bounds.getHeight() - 50).toFloat();

        // === NEURAL FADER TRACK MIT QUANTUM GLOW ===
        // Outer Quantum Shadow
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.fillRoundedRectangle(trackArea.expanded(3.0f), 3.0f);

        // Neural Track Base
        g.setColour(primaryBg);
        g.fillRoundedRectangle(trackArea.expanded(1.0f), 2.0f);

        // Inner Neural Channel
        juce::ColourGradient trackGradient(
            secondaryBg,
            trackArea.getCentreX(), trackArea.getY(),
            primaryBg.darker(0.3f),
            trackArea.getCentreX(), trackArea.getBottom(),
            false
        );
        g.setGradientFill(trackGradient);
        g.fillRoundedRectangle(trackArea, 2.0f);

        // Neural Grid Lines
        g.setColour(borderColor.withAlpha(0.5f));
        for (float y = trackArea.getY(); y < trackArea.getBottom(); y += 20.0f)
        {
            g.drawLine(trackArea.getX() + 2, y, trackArea.getRight() - 2, y, 0.5f);
        }

        // Quantum Glow Border
        g.setColour(accentColor.withAlpha(0.3f));
        g.drawRoundedRectangle(trackArea.expanded(1.0f), 2.0f, 1.5f);

        // === NEURAL SCALE MARKINGS ===
        drawNeuralFaderScale(g, trackArea, slider);

        // === QUANTUM FADER KNOB ===
        drawNeuralChannelFaderKnob(g, trackArea, sliderPos, slider);
    }

    // === QUANTUM CROSSFADER (Neural Horizontal Control) ===
    void drawQuantumCrossfader(juce::Graphics& g, const juce::Rectangle<int>& bounds, float sliderPos, juce::Slider& slider)
    {
        auto trackHeight = 12;
        auto trackArea = bounds.withSizeKeepingCentre(bounds.getWidth() - 80, trackHeight).toFloat();

        // === QUANTUM CROSSFADER TRACK ===
        // Outer Neural Shadow
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(trackArea.expanded(0, 3.0f), 3.0f);

        // Neural Base Track
        juce::ColourGradient crossGradient(
            primaryBg,
            trackArea.getX(), trackArea.getCentreY(),
            secondaryBg,
            trackArea.getRight(), trackArea.getCentreY(),
            false
        );
        g.setGradientFill(crossGradient);
        g.fillRoundedRectangle(trackArea, 2.0f);

        // === QUANTUM CROSSFADER ZONES ===
        auto centerX = trackArea.getCentreX();
        auto currentPos = (sliderPos - trackArea.getX()) / trackArea.getWidth();

        // Zone A (Left) - Neural Pink
        auto zoneA = trackArea.withTrimmedRight(trackArea.getWidth() * 0.5f);
        if (currentPos < 0.5f)
        {
            g.setColour(audioFreqLow.withAlpha(0.3f));
            g.fillRoundedRectangle(zoneA, 2.0f);
        }

        // Zone B (Right) - Neural Blue  
        auto zoneB = trackArea.withTrimmedLeft(trackArea.getWidth() * 0.5f);
        if (currentPos > 0.5f)
        {
            g.setColour(audioFreqHigh.withAlpha(0.3f));
            g.fillRoundedRectangle(zoneB, 2.0f);
        }

        // === NEURAL CENTER LINE ===
        g.setColour(accentColor);
        g.drawLine(centerX, trackArea.getY() - 10, centerX, trackArea.getBottom() + 10, 2.0f);

        // Quantum Center Glow
        g.setColour(accentColor.withAlpha(0.3f));
        g.drawLine(centerX, trackArea.getY() - 8, centerX, trackArea.getBottom() + 8, 4.0f);

        // === NEURAL DECK LABELS ===
        g.setColour(secondaryText);
        g.setFont(juce::Font("Electrolize", 10.0f, juce::Font::bold));

        // Deck A Label
        g.setColour(audioFreqLow);
        g.drawText("DECK.A", trackArea.getX() - 35, trackArea.getY() - 18, 30, 12, juce::Justification::centred);

        // Deck B Label
        g.setColour(audioFreqHigh);
        g.drawText("DECK.B", trackArea.getRight() + 5, trackArea.getY() - 18, 30, 12, juce::Justification::centred);

        // === QUANTUM CROSSFADER KNOB ===
        drawQuantumCrossfaderKnob(g, trackArea, sliderPos, slider);
    }

    // === NEURAL CHANNEL FADER KNOB ===
    void drawNeuralChannelFaderKnob(juce::Graphics& g, const juce::Rectangle<float>& trackArea,
        float sliderPos, juce::Slider& slider)
    {
        auto knobWidth = 24.0f;
        auto knobHeight = 18.0f;
        auto knobArea = juce::Rectangle<float>(
            trackArea.getCentreX() - knobWidth / 2,
            sliderPos - knobHeight / 2,
            knobWidth, knobHeight
        );

        // === QUANTUM SHADOW SYSTEM ===
        // Primary Shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(knobArea.translated(2.0f, 3.0f), 3.0f);

        // Neural Glow Shadow
        g.setColour(accentColor.withAlpha(0.2f));
        g.fillRoundedRectangle(knobArea.translated(1.0f, 2.0f).expanded(2.0f), 4.0f);

        // === NEURAL KNOB BODY ===
        // Main Body Gradient
        juce::ColourGradient knobGradient(
            tertiaryBg.brighter(0.2f),  // Lighter top
            knobArea.getCentreX(), knobArea.getY(),
            secondaryBg.darker(0.2f),   // Darker bottom
            knobArea.getCentreX(), knobArea.getBottom(),
            false
        );
        g.setGradientFill(knobGradient);
        g.fillRoundedRectangle(knobArea, 3.0f);

        // === QUANTUM BORDER SYSTEM ===
        // Outer Neural Border
        g.setColour(borderColor.brighter(0.3f));
        g.drawRoundedRectangle(knobArea.reduced(0.5f), 2.5f, 1.5f);

        // Inner Quantum Line
        g.setColour(accentColor.withAlpha(0.7f));
        g.drawRoundedRectangle(knobArea.reduced(2.0f), 1.5f, 1.0f);

        // === NEURAL GRIP PATTERN ===
        g.setColour(primaryBg.darker(0.4f));
        for (int i = 0; i < 4; ++i)
        {
            auto lineX = knobArea.getX() + 5 + i * 3.5f;
            g.drawLine(lineX, knobArea.getY() + 4, lineX, knobArea.getBottom() - 4, 1.0f);
        }

        // === QUANTUM ACTIVITY INDICATORS ===
        if (slider.isMouseOverOrDragging())
        {
            // Neural Activity Glow
            g.setColour(accentColor.withAlpha(0.4f));
            g.drawRoundedRectangle(knobArea.expanded(2.0f), 5.0f, 2.0f);

            // Quantum Pulse Effect
            g.setColour(secondaryAccent.withAlpha(0.3f));
            g.drawRoundedRectangle(knobArea.expanded(4.0f), 6.0f, 1.0f);
        }

        // Level Indicator Light
        auto value = slider.getValue() / slider.getMaximum();
        if (value > 0.8f)
        {
            g.setColour(warningColor);
        }
        else if (value > 0.6f)
        {
            g.setColour(accentColor);
        }
        else
        {
            g.setColour(mutedText);
        }

        auto indicator = juce::Rectangle<float>(knobArea.getRight() - 3, knobArea.getY() + 2, 2, 4);
        g.fillRoundedRectangle(indicator, 1.0f);
    }

    // === QUANTUM CROSSFADER KNOB ===
    void drawQuantumCrossfaderKnob(juce::Graphics& g, const juce::Rectangle<float>& trackArea,
        float sliderPos, juce::Slider& slider)
    {
        auto knobWidth = 30.0f;
        auto knobHeight = 24.0f;
        auto knobArea = juce::Rectangle<float>(
            sliderPos - knobWidth / 2,
            trackArea.getCentreY() - knobHeight / 2,
            knobWidth, knobHeight
        );

        // === QUANTUM CROSSFADER SHADOW SYSTEM ===
        // Primary Neural Shadow
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(knobArea.translated(2.5f, 3.0f), 4.0f);

        // Quantum Aura
        auto currentPos = (sliderPos - trackArea.getX()) / trackArea.getWidth();
        juce::Colour auraColour;
        if (currentPos < 0.4f)
            auraColour = audioFreqLow;  // Deck A
        else if (currentPos > 0.6f)
            auraColour = audioFreqHigh; // Deck B
        else
            auraColour = accentColor;   // Center

        g.setColour(auraColour.withAlpha(0.2f));
        g.fillRoundedRectangle(knobArea.expanded(3.0f), 6.0f);

        // === NEURAL CROSSFADER BODY ===
        // Main Body with special Crossfader gradient
        juce::ColourGradient crossKnobGradient(
            tertiaryBg.brighter(0.3f),
            knobArea.getCentreX(), knobArea.getY(),
            primaryBg.darker(0.2f),
            knobArea.getCentreX(), knobArea.getBottom(),
            false
        );
        g.setGradientFill(crossKnobGradient);
        g.fillRoundedRectangle(knobArea, 4.0f);

        // === QUANTUM CROSSFADER BORDERS ===
        // Outer Neural Frame
        g.setColour(borderColor.brighter(0.4f));
        g.drawRoundedRectangle(knobArea.reduced(1.0f), 3.0f, 2.0f);

        // Inner Quantum Ring
        g.setColour(auraColour.withAlpha(0.8f));
        g.drawRoundedRectangle(knobArea.reduced(3.0f), 2.0f, 1.5f);

        // === NEURAL CROSSFADER GRIP SYSTEM ===
        // Central grip area
        auto gripArea = knobArea.reduced(8.0f, 5.0f);
        g.setColour(primaryBg.darker(0.5f));
        g.fillRoundedRectangle(gripArea, 1.5f);

        // Crossfader grip lines
        g.setColour(borderColor);
        for (int i = 0; i < 3; ++i)
        {
            auto lineY = gripArea.getY() + 2 + i * 3;
            g.drawLine(gripArea.getX() + 2, lineY, gripArea.getRight() - 2, lineY, 1.0f);
        }

        // === QUANTUM POSITION INDICATORS ===
        // Position Status Light
        g.setColour(auraColour);
        auto statusLight = juce::Rectangle<float>(knobArea.getCentreX() - 2, knobArea.getY() + 2, 4, 6);
        g.fillRoundedRectangle(statusLight, 2.0f);

        // Neural Position Display
        g.setColour(auraColour.withAlpha(0.6f));
        auto posDisplay = juce::Rectangle<float>(knobArea.getCentreX() - 1, knobArea.getBottom() - 4, 2, 2);
        g.fillEllipse(posDisplay);

        // === QUANTUM INTERACTION FEEDBACK ===
        if (slider.isMouseOverOrDragging())
        {
            // Neural Interaction Glow
            g.setColour(auraColour.withAlpha(0.4f));
            g.drawRoundedRectangle(knobArea.expanded(3.0f), 7.0f, 2.0f);

            // Quantum Pulse Ring
            g.setColour(secondaryAccent.withAlpha(0.3f));
            g.drawRoundedRectangle(knobArea.expanded(6.0f), 9.0f, 1.0f);
        }
    }

    // === NEURAL FADER SCALE SYSTEM ===
    void drawNeuralFaderScale(juce::Graphics& g, const juce::Rectangle<float>& trackArea, juce::Slider& slider)
    {
        g.setColour(mutedText);
        g.setFont(juce::Font("Electrolize", 8.0f, juce::Font::bold));

        // Neural Scale Marks
        auto range = slider.getRange();
        auto numTicks = 11; // 0-100 in 10er Schritten

        for (int i = 0; i <= numTicks; ++i)
        {
            auto proportion = i / float(numTicks);
            auto tickY = trackArea.getBottom() - (proportion * trackArea.getHeight());

            // Major ticks für 0, 50, 100
            bool isMajorTick = (i == 0 || i == 5 || i == numTicks);
            auto tickLength = isMajorTick ? 10.0f : 5.0f;
            auto tickThickness = isMajorTick ? 2.0f : 1.0f;

            // Neural Tick Color
            juce::Colour tickColor = isMajorTick ? accentColor : mutedText;

            // Main tick line
            g.setColour(tickColor);
            g.drawLine(trackArea.getRight() + 3, tickY,
                trackArea.getRight() + 3 + tickLength, tickY, tickThickness);

            // Quantum glow for major ticks
            if (isMajorTick)
            {
                g.setColour(accentColor.withAlpha(0.3f));
                g.drawLine(trackArea.getRight() + 3, tickY,
                    trackArea.getRight() + 3 + tickLength, tickY, tickThickness + 2.0f);
            }

            // Neural Scale Numbers
            if (isMajorTick && i <= numTicks)
            {
                auto value = juce::String(int(proportion * 100));
                g.setColour(isMajorTick ? accentColor : mutedText);
                g.drawText(value, trackArea.getRight() + 15, tickY - 5, 25, 10,
                    juce::Justification::left);
            }
        }

        // Neural "dB" Label
        g.setFont(juce::Font("Electrolize", 7.0f, juce::Font::bold));
        g.setColour(secondaryAccent);
        g.drawText("dB", trackArea.getRight() + 15, trackArea.getY() - 18, 20, 12,
            juce::Justification::left);
    }

    // === NEURAL HORIZONTAL SLIDER SYSTEM ===
    void drawNeuralHorizontalSlider(juce::Graphics& g, const juce::Rectangle<int>& bounds,
        float sliderPos, juce::Slider& slider)
    {
        auto trackHeight = 8;
        auto trackArea = bounds.withSizeKeepingCentre(bounds.getWidth() - 30, trackHeight).toFloat();

        // === NEURAL HORIZONTAL TRACK ===
        // Outer Neural Shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(trackArea.expanded(0, 2.0f), 4.0f);

        // Neural Track Base
        juce::ColourGradient hTrackGradient(
            primaryBg,
            trackArea.getX(), trackArea.getCentreY(),
            secondaryBg,
            trackArea.getRight(), trackArea.getCentreY(),
            false
        );
        g.setGradientFill(hTrackGradient);
        g.fillRoundedRectangle(trackArea, 4.0f);

        // === NEURAL CENTER MARKING (for EQ/Filter controls) ===
        if (slider.getRange().getStart() < 0 && slider.getRange().getEnd() > 0)
        {
            auto centerX = trackArea.getCentreX();

            // Neural Center Line
            g.setColour(accentColor.withAlpha(0.7f));
            g.drawLine(centerX, trackArea.getY() - 5, centerX, trackArea.getBottom() + 5, 2.0f);

            // Quantum Center Point
            g.setColour(accentColor);
            g.fillEllipse(centerX - 2, trackArea.getCentreY() - 2, 4, 4);

            // Center Glow
            g.setColour(accentColor.withAlpha(0.3f));
            g.fillEllipse(centerX - 3, trackArea.getCentreY() - 3, 6, 6);
        }

        // === NEURAL SCALE MARKS ===
        if (slider.getName().contains("eq") || slider.getName().contains("filter"))
        {
            drawNeuralHorizontalScale(g, trackArea, slider);
        }

        // === NEURAL HORIZONTAL KNOB ===
        drawNeuralHorizontalKnob(g, trackArea, sliderPos, slider);
    }

    // === NEURAL HORIZONTAL SCALE SYSTEM ===
    void drawNeuralHorizontalScale(juce::Graphics& g, const juce::Rectangle<float>& trackArea, juce::Slider& slider)
    {
        g.setColour(mutedText.withAlpha(0.8f));
        g.setFont(juce::Font("Electrolize", 7.0f, juce::Font::bold));

        // Neural EQ Scale Points
        auto centerX = trackArea.getCentreX();
        auto quarterLeft = trackArea.getX() + trackArea.getWidth() * 0.25f;
        auto quarterRight = trackArea.getX() + trackArea.getWidth() * 0.75f;

        // Scale tick marks
        g.setColour(mutedText);
        g.drawLine(quarterLeft, trackArea.getBottom() + 3, quarterLeft, trackArea.getBottom() + 8, 1.5f);

        g.setColour(accentColor);
        g.drawLine(centerX, trackArea.getBottom() + 3, centerX, trackArea.getBottom() + 10, 2.0f);

        g.setColour(mutedText);
        g.drawLine(quarterRight, trackArea.getBottom() + 3, quarterRight, trackArea.getBottom() + 8, 1.5f);

        // Neural Scale Labels
        g.setColour(secondaryAccent);
        g.drawText("-12", quarterLeft - 8, trackArea.getBottom() + 12, 16, 8, juce::Justification::centred);

        g.setColour(accentColor);
        g.drawText("0", centerX - 6, trackArea.getBottom() + 12, 12, 8, juce::Justification::centred);

        g.setColour(secondaryAccent);
        g.drawText("+12", quarterRight - 8, trackArea.getBottom() + 12, 16, 8, juce::Justification::centred);
    }

    // === NEURAL HORIZONTAL KNOB ===
    void drawNeuralHorizontalKnob(juce::Graphics& g, const juce::Rectangle<float>& trackArea,
        float sliderPos, juce::Slider& slider)
    {
        auto thumbRadius = 10.0f;
        auto thumbArea = juce::Rectangle<float>(
            sliderPos - thumbRadius,
            trackArea.getCentreY() - thumbRadius,
            thumbRadius * 2.0f,
            thumbRadius * 2.0f
        );

        // === QUANTUM KNOB SHADOW ===
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(thumbArea.translated(2.0f, 2.0f));

        // Neural Glow
        g.setColour(accentColor.withAlpha(0.2f));
        g.fillEllipse(thumbArea.expanded(3.0f));

        // === NEURAL KNOB BODY ===
        juce::ColourGradient thumbGradient(
            tertiaryBg.brighter(0.3f),
            thumbArea.getCentreX(), thumbArea.getY(),
            primaryBg.darker(0.2f),
            thumbArea.getCentreX(), thumbArea.getBottom(),
            false
        );
        g.setGradientFill(thumbGradient);
        g.fillEllipse(thumbArea);

        // === QUANTUM KNOB BORDERS ===
        // Outer Neural Ring
        g.setColour(borderColor.brighter(0.4f));
        g.drawEllipse(thumbArea.reduced(1.0f), 2.0f);

        // Inner Quantum Ring
        g.setColour(accentColor.withAlpha(0.7f));
        g.drawEllipse(thumbArea.reduced(3.0f), 1.5f);

        // === NEURAL CENTER INDICATOR ===
        g.setColour(accentColor);
        auto centerIndicator = thumbArea.reduced(6.0f);
        g.fillEllipse(centerIndicator);

        // Quantum Center Glow
        g.setColour(accentColor.withAlpha(0.6f));
        g.fillEllipse(centerIndicator.expanded(1.0f));

        // === NEURAL INTERACTION FEEDBACK ===
        if (slider.isMouseOverOrDragging())
        {
            g.setColour(secondaryAccent.withAlpha(0.4f));
            g.drawEllipse(thumbArea.expanded(2.0f), 2.0f);
        }
    }

    // === QUANTUM BUTTON SYSTEM ===
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 8.0f;

        // Neural Button Color Logic
        juce::Colour baseColour = secondaryBg;
        juce::Colour glowColour = accentColor;

        if (button.getToggleState())
        {
            baseColour = accentColor;
            glowColour = secondaryAccent;
        }
        else if (shouldDrawButtonAsDown)
        {
            baseColour = tertiaryBg;
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            baseColour = secondaryBg.brighter(0.2f);
        }

        // === QUANTUM BUTTON SHADOW ===
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds.translated(2.0f, 3.0f), cornerSize);

        // Neural Glow Effect
        if (button.getToggleState() || shouldDrawButtonAsHighlighted)
        {
            g.setColour(glowColour.withAlpha(0.3f));
            g.fillRoundedRectangle(bounds.expanded(2.0f), cornerSize + 2.0f);
        }

        // === NEURAL BUTTON BODY ===
        juce::ColourGradient buttonGradient(
            baseColour.brighter(0.2f),
            bounds.getCentreX(), bounds.getY(),
            baseColour.darker(0.2f),
            bounds.getCentreX(), bounds.getBottom(),
            false
        );
        g.setGradientFill(buttonGradient);
        g.fillRoundedRectangle(bounds, cornerSize);

        // === QUANTUM BUTTON BORDER ===
        g.setColour(borderColor.brighter(0.3f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), cornerSize, 2.0f);

        // Active state inner glow
        if (button.getToggleState())
        {
            g.setColour(primaryBg.withAlpha(0.8f));
            g.drawRoundedRectangle(bounds.reduced(3.0f), cornerSize - 2.0f, 1.0f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto font = juce::Font("Electrolize", button.getHeight() * 0.5f, juce::Font::bold);
        g.setFont(font);

        juce::Colour textColour = primaryText;

        if (button.getToggleState())
            textColour = primaryBg;
        else if (!button.isEnabled())
            textColour = mutedText;
        else if (shouldDrawButtonAsHighlighted)
            textColour = accentColor;

        g.setColour(textColour);

        // Neural text with subtle glow
        if (button.getToggleState() || shouldDrawButtonAsHighlighted)
        {
            g.setColour(textColour.withAlpha(0.3f));
            auto textBounds = button.getLocalBounds().translated(1, 1);
            g.drawFittedText(button.getButtonText(), textBounds,
                juce::Justification::centred, 1);
        }

        g.setColour(textColour);
        auto textBounds = button.getLocalBounds();
        g.drawFittedText(button.getButtonText(), textBounds,
            juce::Justification::centred, 1);
    }

    // === NEURAL COMBOBOX SYSTEM ===
    void drawComboBox(juce::Graphics& g, int width, int height,
        bool isButtonDown, int buttonX, int buttonY,
        int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto cornerSize = 6.0f;
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        // === NEURAL COMBOBOX BACKGROUND ===
        // Shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(bounds.translated(1.0f, 2.0f), cornerSize);

        // Main background
        juce::ColourGradient comboGradient(
            secondaryBg.brighter(0.1f),
            bounds.getCentreX(), bounds.getY(),
            primaryBg.darker(0.1f),
            bounds.getCentreX(), bounds.getBottom(),
            false
        );
        g.setGradientFill(comboGradient);
        g.fillRoundedRectangle(bounds, cornerSize);

        // === QUANTUM COMBOBOX BORDER ===
        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds.reduced(1.0f), cornerSize, 1.5f);

        // Inner neural glow
        g.setColour(accentColor.withAlpha(0.2f));
        g.drawRoundedRectangle(bounds.reduced(2.0f), cornerSize - 1.0f, 1.0f);

        // === NEURAL ARROW INDICATOR ===
        auto arrowBounds = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH).reduced(6.0f);

        //juce::Path neuralArrow;
        //neuralArrow.addTriangle(
        //    arrowBounds.getX() + 2, arrowBounds.getY() + 2,
        //    arrowBounds.getRight() - 2, arrowBounds.getY() + 2,
        //    arrowBounds.getCentreX(), arrowBounds.getBottom() - 2
        //);

        //g.setColour(accentColor);
        //g.fillPath(neuralArrow);

        //// Arrow glow
        //g.setColour(accentColor.withAlpha(0.4f));
        //g.strokePath(neuralArrow, juce::PathStrokeType(2.0f));
    }

    // === NEURAL FONT SYSTEM ===
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font("Electrolize", juce::jmin(16.0f, buttonHeight * 0.6f), juce::Font::bold);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font("Electrolize", 14.0f, juce::Font::FontStyleFlags::plain);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font("Electrolize", 13.0f, juce::Font::FontStyleFlags::plain);
    }

    // === NEURAL AUDIO DECK COLOR SYSTEM ===
    juce::Colour getDeckAColour() const { return audioFreqLow; }      // Neural Pink
    juce::Colour getDeckBColour() const { return audioFreqHigh; }     // Neural Blue
    juce::Colour getMidFreqColour() const { return audioFreqMid; }    // Neural Green

    juce::Colour getSyncActiveColour() const { return successColor; }
    juce::Colour getMidiLearnColour() const { return warningColor; }
    juce::Colour getErrorColour() const { return warningColor; }
    juce::Colour getQuantumGlow() const { return glowColor; }

    // === QUANTUM COLOR GETTERS ===
    juce::Colour getPrimaryBg() const { return primaryBg; }
    juce::Colour getSecondaryBg() const { return secondaryBg; }
    juce::Colour getTertiaryBg() const { return tertiaryBg; }
    juce::Colour getAccentColor() const { return accentColor; }
    juce::Colour getSecondaryAccent() const { return secondaryAccent; }
    juce::Colour getTertiaryAccent() const { return tertiaryAccent; }

private:
    // === NEURAL AUDIO MATRIX COLOR PALETTE ===
    juce::Colour primaryBg, secondaryBg, tertiaryBg;
    juce::Colour accentColor, secondaryAccent, tertiaryAccent;
    juce::Colour audioFreqLow, audioFreqMid, audioFreqHigh;
    juce::Colour warningColor, successColor;
    juce::Colour primaryText, secondaryText, mutedText, borderColor;
    juce::Colour glowColor;
};

// === USAGE IN MAINCOMPONENT ===
/*
Integration in MainComponent:

// In Header (.h):
std::unique_ptr<CyberpunkDJLookAndFeel> neuralLookAndFeel;

// In Constructor:
neuralLookAndFeel = std::make_unique<CyberpunkDJLookAndFeel>();
setLookAndFeel(neuralLookAndFeel.get());

// Spezielle Slider Setup für Neural Audio Interface:
channelFaderA.setSliderStyle(juce::Slider::LinearVertical);
channelFaderA.setName("channelFaderA");

crossfader.setSliderStyle(juce::Slider::LinearHorizontal);
crossfader.setName("crossfader");

eqHighA.setSliderStyle(juce::Slider::LinearHorizontal);
eqHighA.setName("eq_high_A");

// In Destructor:
setLookAndFeel(nullptr);

// Spezielle Neural Farben verwenden:
playButtonA.setColour(juce::TextButton::buttonOnColourId,
    neuralLookAndFeel->getDeckAColour());

playButtonB.setColour(juce::TextButton::buttonOnColourId,
    neuralLookAndFeel->getDeckBColour());
*/
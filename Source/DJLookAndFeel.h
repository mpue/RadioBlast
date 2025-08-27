/*
  ==============================================================================

    DJLookAndFeel.h
    Created: 26 Aug 2025 11:48:31am
    Author:  mpue

  ==============================================================================
*/

// DJLookAndFeel.h - Professionelles Look&Feel für DJ-Applikation
#pragma once
#include <JuceHeader.h>

class DJLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DJLookAndFeel()
    {
        // === FARBPALETTE DEFINIEREN ===
        // Hauptfarben
        darkBackground = juce::Colour(0xff1a1a1e);      // Sehr dunkles Grau
        mediumBackground = juce::Colour(0xff2d2d35);    // Mittleres Grau
        lightBackground = juce::Colour(0xff3a3a42);     // Helles Grau

        // Akzentfarben - Professionelle Blautöne
        primaryBlue = juce::Colour(0xff4a9eff);         // Helles Blau
        secondaryBlue = juce::Colour(0xff2e5cb8);       // Mittleres Blau
        darkBlue = juce::Colour(0xff1e3a5f);           // Dunkles Blau

        // Status-Farben
        successGreen = juce::Colour(0xff00c851);        // Grün für Erfolg
        warningOrange = juce::Colour(0xffff8800);       // Orange für Warnung
        errorRed = juce::Colour(0xffff4444);            // Rot für Fehler

        // Text-Farben
        primaryText = juce::Colour(0xffe8e8e8);         // Haupttext
        secondaryText = juce::Colour(0xffb8b8b8);       // Sekundärtext
        mutedText = juce::Colour(0xff888888);           // Gedämpfter Text

        // Standard-Farben setzen
        setColour(juce::ResizableWindow::backgroundColourId, darkBackground);
        setColour(juce::DocumentWindow::backgroundColourId, darkBackground);
        setColour(DocumentWindow::backgroundColourId, Colour(0xff333333));
        setColour(DocumentWindow::textColourId, juce::Colours::white);

        // Slider-Farben
        setColour(juce::Slider::backgroundColourId, mediumBackground);
        setColour(juce::Slider::thumbColourId, primaryBlue);
        setColour(juce::Slider::trackColourId, secondaryBlue);
        setColour(juce::Slider::rotarySliderFillColourId, primaryBlue);
        setColour(juce::Slider::rotarySliderOutlineColourId, lightBackground);

        // Button-Farben
        setColour(juce::TextButton::buttonColourId, mediumBackground);
        setColour(juce::TextButton::buttonOnColourId, primaryBlue);
        setColour(juce::TextButton::textColourOffId, primaryText);
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        // ComboBox-Farben
        setColour(juce::ComboBox::backgroundColourId, mediumBackground);
        setColour(juce::ComboBox::textColourId, primaryText);
        setColour(juce::ComboBox::outlineColourId, lightBackground);
        setColour(juce::ComboBox::arrowColourId, primaryBlue);

        // Label-Farben
        setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, primaryText);

        // PopupMenu-Farben
        setColour(juce::PopupMenu::backgroundColourId, mediumBackground);
        setColour(juce::PopupMenu::textColourId, primaryText);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, primaryBlue);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

        setColour(juce::GroupComponent::textColourId, juce::Colours::white);
        setColour(juce::GroupComponent::outlineColourId, juce::Colours::grey);
        setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff333333));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff444444));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff444444));
    }

    // === CUSTOM SLIDER DRAWING ===
 // === ERWEITERTE SLIDER-FUNKTIONEN FÜR AUTHENTISCHES MISCHPULT-DESIGN ===

// Füge diese Methoden zu deiner DJLookAndFeel Klasse hinzu:

// === ERWEITERTE SLIDER-FUNKTIONEN FÜR AUTHENTISCHES MISCHPULT-DESIGN ===

// Füge diese Methoden zu deiner DJLookAndFeel Klasse hinzu:

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackBounds = juce::Rectangle<int>(x, y, width, height);

        if (style == juce::Slider::LinearVertical)
        {
            drawChannelFader(g, trackBounds, sliderPos, slider);
        }
        else if (style == juce::Slider::LinearHorizontal)
        {
            // Unterscheide zwischen Crossfader und EQ/Gain Knöpfen
            if (slider.getName().contains("crossfader") || slider.getName().contains("cross"))
            {
                drawCrossfader(g, trackBounds, sliderPos, slider);
            }
            else
            {
                drawHorizontalSlider(g, trackBounds, sliderPos, slider);
            }
        }
    }

    // === CHANNEL FADER (Vertikale Volume-Slider) ===
    void drawChannelFader(juce::Graphics& g, const juce::Rectangle<int>& bounds, float sliderPos, juce::Slider& slider)
    {
        auto trackWidth = 12;  // Breitere Spur wie bei echten Fadern
        auto trackArea = bounds.withSizeKeepingCentre(trackWidth, bounds.getHeight() - 40).toFloat();

        // === FADER-SPUR MIT VERTIEFUNG ===
        // Äußerer Rahmen (Vertiefung)
        g.setColour(juce::Colour(0xff0a0a0a));  // Sehr dunkel für Tiefe
        g.fillRoundedRectangle(trackArea.expanded(2.0f), 2.0f);

        // Innere Spur
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(trackArea, 1.0f);

        // Seitliche Führungslinien
        g.setColour(juce::Colour(0xff2a2a2a));
        g.drawLine(trackArea.getX(), trackArea.getY(), trackArea.getX(), trackArea.getBottom(), 1.0f);
        g.drawLine(trackArea.getRight(), trackArea.getY(), trackArea.getRight(), trackArea.getBottom(), 1.0f);

        // === SKALIERUNG/MARKIERUNGEN ===
        drawFaderScale(g, trackArea, slider);

        // === PROFESSIONELLER FADER-KNOB ===
        drawChannelFaderKnob(g, trackArea, sliderPos, slider);
    }

    // === CROSSFADER (Horizontaler DJ-Crossfader) ===
    void drawCrossfader(juce::Graphics& g, const juce::Rectangle<int>& bounds, float sliderPos, juce::Slider& slider)
    {
        auto trackHeight = 10;
        auto trackArea = bounds.withSizeKeepingCentre(bounds.getWidth() - 60, trackHeight).toFloat();

        // === CROSSFADER-SPUR ===
        // Äußere Vertiefung
        g.setColour(juce::Colour(0xff0a0a0a));
        g.fillRoundedRectangle(trackArea.expanded(0, 2.0f), 2.0f);

        // Innere Spur
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(trackArea, 1.0f);

        // === CURVE-MARKIERUNGEN (A-Mitte-B) ===
        auto centerX = trackArea.getCentreX();

        // Mittellinie
        g.setColour(primaryBlue.withAlpha(0.6f));
        g.drawLine(centerX, trackArea.getY() - 8, centerX, trackArea.getBottom() + 8, 2.0f);

        // A/B Markierungen
        g.setColour(mutedText);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("A", trackArea.getX() - 20, trackArea.getY() - 15, 15, 10, juce::Justification::centred);
        g.drawText("B", trackArea.getRight() + 5, trackArea.getY() - 15, 15, 10, juce::Justification::centred);

        // === CROSSFADER-KNOB ===
        drawCrossfaderKnob(g, trackArea, sliderPos, slider);
    }

    // === CHANNEL FADER KNOB ===
    void drawChannelFaderKnob(juce::Graphics& g, const juce::Rectangle<float>& trackArea, float sliderPos, juce::Slider& slider)
    {
        auto knobWidth = 20.0f;
        auto knobHeight = 15.0f;
        auto knobArea = juce::Rectangle<float>(
            trackArea.getCentreX() - knobWidth / 2,
            sliderPos - knobHeight / 2,
            knobWidth, knobHeight
        );

        // === 3D-EFFEKT ===
        // Schatten
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(knobArea.translated(1.0f, 2.0f), 2.0f);

        // Hauptkörper - Gradient für 3D-Look
        juce::ColourGradient bodyGradient(
            juce::Colour(0xff4a4a4a),  // Hell oben
            knobArea.getCentreX(), knobArea.getY(),
            juce::Colour(0xff2a2a2a),  // Dunkel unten
            knobArea.getCentreX(), knobArea.getBottom(),
            false
        );
        g.setGradientFill(bodyGradient);
        g.fillRoundedRectangle(knobArea, 2.0f);

        // === METALLISCHER RAHMEN ===
        g.setColour(juce::Colour(0xff6a6a6a));
        g.drawRoundedRectangle(knobArea.reduced(0.5f), 2.0f, 1.0f);

        // === GRIFFMUSTER (Vertikale Linien) ===
        g.setColour(juce::Colour(0xff1a1a1a));
        for (int i = 0; i < 3; ++i)
        {
            auto lineX = knobArea.getX() + 4 + i * 4;
            g.drawLine(lineX, knobArea.getY() + 3, lineX, knobArea.getBottom() - 3, 1.0f);
        }

        // === HIGHLIGHT FÜR AKTIVEN ZUSTAND ===
        if (slider.isMouseOverOrDragging())
        {
            g.setColour(primaryBlue.withAlpha(0.3f));
            g.drawRoundedRectangle(knobArea.expanded(1.0f), 3.0f, 2.0f);
        }
    }

    // === CROSSFADER KNOB ===
    void drawCrossfaderKnob(juce::Graphics& g, const juce::Rectangle<float>& trackArea, float sliderPos, juce::Slider& slider)
    {
        auto knobWidth = 25.0f;
        auto knobHeight = 20.0f;
        auto knobArea = juce::Rectangle<float>(
            sliderPos - knobWidth / 2,
            trackArea.getCentreY() - knobHeight / 2,
            knobWidth, knobHeight
        );

        // === CROSSFADER-SPEZIFISCHES DESIGN ===
        // Schatten
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(knobArea.translated(1.5f, 2.0f), 3.0f);

        // Hauptkörper mit speziellem Crossfader-Design
        juce::ColourGradient crossGradient(
            juce::Colour(0xff5a5a5a),
            knobArea.getCentreX(), knobArea.getY(),
            juce::Colour(0xff2a2a2a),
            knobArea.getCentreX(), knobArea.getBottom(),
            false
        );
        g.setGradientFill(crossGradient);
        g.fillRoundedRectangle(knobArea, 3.0f);

        // Crossfader-spezifische Markierungen
        g.setColour(juce::Colour(0xff7a7a7a));
        g.drawRoundedRectangle(knobArea.reduced(1.0f), 2.0f, 1.5f);

        // Zentrale Griffmulde
        auto gripArea = knobArea.reduced(6.0f, 4.0f);
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(gripArea, 1.0f);

        // Position-Indikator
        auto currentPos = (sliderPos - trackArea.getX()) / trackArea.getWidth();
        juce::Colour posColour;
        if (currentPos < 0.4f)
            posColour = juce::Colour(0xffff6b35); // Orange für A-Seite
        else if (currentPos > 0.6f)
            posColour = juce::Colour(0xff9c27b0); // Lila für B-Seite
        else
            posColour = primaryBlue; // Blau für Mitte

        g.setColour(posColour);
        auto indicator = juce::Rectangle<float>(knobArea.getCentreX() - 1, knobArea.getY() + 2, 2, 4);
        g.fillRoundedRectangle(indicator, 1.0f);
    }

    // === FADER-SKALIERUNG ===
    void drawFaderScale(juce::Graphics& g, const juce::Rectangle<float>& trackArea, juce::Slider& slider)
    {
        g.setColour(mutedText);
        g.setFont(juce::Font(8.0f));

        // Skalierungsstriche für Volume-Fader
        auto range = slider.getRange();
        auto numTicks = 11; // 0, 10, 20, ... 100

        for (int i = 0; i <= numTicks; ++i)
        {
            auto proportion = i / float(numTicks);
            auto tickY = trackArea.getBottom() - (proportion * trackArea.getHeight());

            // Hauptstriche bei 0, 50, 100
            bool isMajorTick = (i == 0 || i == 5 || i == numTicks);
            auto tickLength = isMajorTick ? 8.0f : 4.0f;
            auto tickThickness = isMajorTick ? 1.5f : 1.0f;

            // Tick-Linie
            g.drawLine(trackArea.getRight() + 2, tickY,
                trackArea.getRight() + 2 + tickLength, tickY, tickThickness);

            // Zahlen nur bei Hauptstrichen
            if (isMajorTick && i <= numTicks)
            {
                auto value = juce::String(int(proportion * 100));
                g.drawText(value, trackArea.getRight() + 12, tickY - 4, 20, 8,
                    juce::Justification::left);
            }
        }

        // "dB" Label am oberen Ende
        g.setFont(juce::Font(7.0f));
        g.drawText("dB", trackArea.getRight() + 12, trackArea.getY() - 15, 20, 10,
            juce::Justification::left);
    }

    // === ZUSÄTZLICHE HILFSMETHODEN ===

    // Spezielle Methode für Gain-Knöpfe (wenn als horizontale Slider implementiert)
    void drawGainSlider(juce::Graphics& g, const juce::Rectangle<int>& bounds, float sliderPos, juce::Slider& slider)
    {
        // Ähnlich wie Crossfader, aber mit Gain-spezifischen Markierungen
        auto trackHeight = 8;
        auto trackArea = bounds.withSizeKeepingCentre(bounds.getWidth() - 40, trackHeight).toFloat();

        // Unity-Gain Position (meist bei 12 Uhr bzw. Mitte)
        auto unityPos = trackArea.getCentreX();

        // Track
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(trackArea, 1.0f);

        // Unity-Markierung
        g.setColour(warningOrange);
        g.drawLine(unityPos, trackArea.getY() - 5, unityPos, trackArea.getBottom() + 5, 2.0f);

        // Kleiner Unity-Indikator
        g.setFont(juce::Font(8.0f, juce::Font::bold));
        g.drawText("U", unityPos - 4, trackArea.getY() - 18, 8, 10, juce::Justification::centred);

        // Standard horizontaler Knob
        drawHorizontalSliderKnob(g, trackArea, sliderPos, slider);
    }

    // === STANDARD HORIZONTALER SLIDER (für EQ, Filter, etc.) ===
    void drawHorizontalSlider(juce::Graphics& g, const juce::Rectangle<int>& bounds, float sliderPos, juce::Slider& slider)
    {
        auto trackHeight = 6;
        auto trackArea = bounds.withSizeKeepingCentre(bounds.getWidth() - 20, trackHeight).toFloat();

        // === TRACK MIT VERTIEFUNG ===
        // Äußere Vertiefung
        g.setColour(juce::Colour(0xff0a0a0a));
        g.fillRoundedRectangle(trackArea.expanded(0, 1.0f), 3.0f);

        // Innere Spur
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(trackArea, 3.0f);

        // === MITTELMARKIERUNG (falls Slider symmetrisch ist) ===
        if (slider.getRange().getStart() < 0 && slider.getRange().getEnd() > 0)
        {
            auto centerX = trackArea.getCentreX();
            g.setColour(mutedText.withAlpha(0.7f));
            g.drawLine(centerX, trackArea.getY() - 3, centerX, trackArea.getBottom() + 3, 1.5f);

            // Kleiner Mittelpunkt
            g.setColour(primaryBlue.withAlpha(0.5f));
            g.fillEllipse(centerX - 1, trackArea.getCentreY() - 1, 2, 2);
        }

        // === SKALIERUNGSSTRICHE (optional für wichtige Slider) ===
        if (slider.getName().contains("eq") || slider.getName().contains("filter"))
        {
            drawHorizontalSliderScale(g, trackArea, slider);
        }

        // === STANDARD KNOB ===
        drawHorizontalSliderKnob(g, trackArea, sliderPos, slider);
    }

    // === SKALIERUNG FÜR HORIZONTALE SLIDER ===
    void drawHorizontalSliderScale(juce::Graphics& g, const juce::Rectangle<float>& trackArea, juce::Slider& slider)
    {
        g.setColour(mutedText.withAlpha(0.6f));

        // Für EQ: -12, 0, +12 dB Markierungen
        auto range = slider.getRange();
        auto centerX = trackArea.getCentreX();
        auto quarterLeft = trackArea.getX() + trackArea.getWidth() * 0.25f;
        auto quarterRight = trackArea.getX() + trackArea.getWidth() * 0.75f;

        // Striche
        g.drawLine(quarterLeft, trackArea.getBottom() + 2, quarterLeft, trackArea.getBottom() + 6, 1.0f);
        g.drawLine(centerX, trackArea.getBottom() + 2, centerX, trackArea.getBottom() + 8, 1.5f);
        g.drawLine(quarterRight, trackArea.getBottom() + 2, quarterRight, trackArea.getBottom() + 6, 1.0f);

        // Labels
        g.setFont(juce::Font(7.0f));
        g.drawText("-", quarterLeft - 3, trackArea.getBottom() + 8, 6, 8, juce::Justification::centred);
        g.drawText("0", centerX - 3, trackArea.getBottom() + 8, 6, 8, juce::Justification::centred);
        g.drawText("+", quarterRight - 3, trackArea.getBottom() + 8, 6, 8, juce::Justification::centred);
    }

    // Standard horizontaler Slider-Knob
    void drawHorizontalSliderKnob(juce::Graphics& g, const juce::Rectangle<float>& trackArea,
        float sliderPos, juce::Slider& slider)
    {
        auto thumbRadius = 8.0f;
        auto thumbArea = juce::Rectangle<float>(
            sliderPos - thumbRadius,
            trackArea.getCentreY() - thumbRadius,
            thumbRadius * 2.0f,
            thumbRadius * 2.0f
        );

        // Schatten
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillEllipse(thumbArea.translated(1.0f, 1.0f));

        // Hauptkörper
        juce::ColourGradient thumbGradient(
            juce::Colour(0xff4a4a4a),
            thumbArea.getCentreX(), thumbArea.getY(),
            juce::Colour(0xff2a2a2a),
            thumbArea.getCentreX(), thumbArea.getBottom(),
            false
        );
        g.setGradientFill(thumbGradient);
        g.fillEllipse(thumbArea);

        // Metallischer Rand
        g.setColour(juce::Colour(0xff6a6a6a));
        g.drawEllipse(thumbArea.reduced(1.0f), 1.0f);

        // Zentrale Markierung
        g.setColour(primaryBlue);
        auto centerDot = thumbArea.reduced(5.0f);
        g.fillEllipse(centerDot);
    }
    // === CUSTOM BUTTON DRAWING ===
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 6.0f;

        // Basis-Farbe bestimmen
        juce::Colour baseColour = mediumBackground;

        if (button.getToggleState())
        {
            baseColour = primaryBlue;
        }
        else if (shouldDrawButtonAsDown)
        {
            baseColour = lightBackground;
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            baseColour = mediumBackground.brighter(0.1f);
        }

        // Button-Schatten
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(bounds.translated(1.0f, 1.0f), cornerSize);

        // Button-Hintergrund mit Gradient
        juce::ColourGradient gradient(baseColour.brighter(0.1f),
            bounds.getCentreX(), bounds.getY(),
            baseColour.darker(0.1f),
            bounds.getCentreX(), bounds.getBottom(),
            false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, cornerSize);

        // Button-Rahmen
        g.setColour(lightBackground);
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);

        juce::Colour textColour = primaryText;

        if (button.getToggleState())
            textColour = juce::Colours::white;
        else if (!button.isEnabled())
            textColour = mutedText;

        g.setColour(textColour);

        auto textBounds = button.getLocalBounds();
        g.drawFittedText(button.getButtonText(), textBounds,
            juce::Justification::centred, 1);
    }

    // === CUSTOM COMBOBOX DRAWING ===
    void drawComboBox(juce::Graphics& g, int width, int height,
        bool isButtonDown, int buttonX, int buttonY,
        int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto cornerSize = 4.0f;
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        // Hintergrund
        g.setColour(mediumBackground);
        g.fillRoundedRectangle(bounds, cornerSize);

        // Rahmen
        g.setColour(lightBackground);
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);

        //// Pfeil
        //auto arrowBounds = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH).reduced(4.0f);
        //g.setColour(primaryBlue);

        //juce::Path arrow;
        //arrow.addTriangle(arrowBounds.getX(), arrowBounds.getY(),
        //    arrowBounds.getRight(), arrowBounds.getY(),
        //    arrowBounds.getCentreX(), arrowBounds.getBottom());
        //g.fillPath(arrow);
    }

    // === FONT CUSTOMIZATION ===
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font(juce::jmin(15.0f, buttonHeight * 0.6f), juce::Font::bold);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(14.0f);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(13.0f);
    }

    // === SPEZIELLE FARBEN FÜR DJ-KOMPONENTEN ===
    juce::Colour getDeckAColour() const { return primaryBlue; }
    juce::Colour getDeckBColour() const { return juce::Colour(0xff9c27b0); } // Lila für Deck B
    juce::Colour getSyncActiveColour() const { return successGreen; }
    juce::Colour getMidiLearnColour() const { return warningOrange; }
    juce::Colour getErrorColour() const { return errorRed; }

    // Getter für Hauptfarben
    juce::Colour getDarkBackground() const { return darkBackground; }
    juce::Colour getMediumBackground() const { return mediumBackground; }
    juce::Colour getLightBackground() const { return lightBackground; }
    juce::Colour getPrimaryBlue() const { return primaryBlue; }

private:
    // Farbpalette
    juce::Colour darkBackground, mediumBackground, lightBackground;
    juce::Colour primaryBlue, secondaryBlue, darkBlue;
    juce::Colour successGreen, warningOrange, errorRed;
    juce::Colour primaryText, secondaryText, mutedText;
};

// === USAGE IN MAINCOMPONENT ===
/*
Füge diese Zeilen in deinen MainComponent Constructor ein:

// Look&Feel anwenden
djLookAndFeel = std::make_unique<DJLookAndFeel>();
setLookAndFeel(djLookAndFeel.get());

Und im Destructor:
setLookAndFeel(nullptr);
*/
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
    }

    // === CUSTOM SLIDER DRAWING ===
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackBounds = juce::Rectangle<int>(x, y, width, height);

        if (style == juce::Slider::LinearVertical)
        {
            // Vertikaler Slider (Volume)
            auto trackWidth = juce::jmin(6, width / 2);
            auto trackArea = trackBounds.withSizeKeepingCentre(trackWidth, height - 20).toFloat();

            // Track-Hintergrund
            g.setColour(lightBackground);
            g.fillRoundedRectangle(trackArea, 3.0f);

            // Aktiver Track (gefüllter Bereich)
            auto activeHeight = sliderPos - trackArea.getY();
            auto activeArea = juce::Rectangle<float>(trackArea.getX(),
                trackArea.getBottom() - activeHeight,
                trackArea.getWidth(),
                activeHeight);

            // Gradient für aktiven Bereich
            juce::ColourGradient gradient(primaryBlue.brighter(0.3f),
                activeArea.getCentreX(), activeArea.getBottom(),
                primaryBlue.darker(0.2f),
                activeArea.getCentreX(), activeArea.getY(),
                false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(activeArea, 3.0f);

            // Thumb (Schieberegler)
            auto thumbRadius = 8.0f;
            auto thumbArea = juce::Rectangle<float>(trackArea.getCentreX() - thumbRadius,
                sliderPos - thumbRadius,
                thumbRadius * 2.0f,
                thumbRadius * 2.0f);

            // Thumb-Schatten
            g.setColour(juce::Colours::black.withAlpha(0.3f));
            g.fillEllipse(thumbArea.translated(1.0f, 1.0f));

            // Thumb-Hauptfarbe
            g.setColour(slider.isMouseOverOrDragging() ? primaryBlue.brighter(0.2f) : primaryBlue);
            g.fillEllipse(thumbArea);

            // Thumb-Highlight
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillEllipse(thumbArea.reduced(2.0f));
        }
        else if (style == juce::Slider::LinearHorizontal)
        {
            // Horizontaler Slider (Pitch, Crossfader)
            auto trackHeight = juce::jmin(6, height / 2);
            auto trackArea = trackBounds.withSizeKeepingCentre(width - 20, trackHeight).toFloat();

            // Track-Hintergrund
            g.setColour(lightBackground);
            g.fillRoundedRectangle(trackArea, 3.0f);

            // Mittellinie für Pitch-Slider (Null-Position)
            if (slider.getRange().getStart() < 0 && slider.getRange().getEnd() > 0)
            {
                auto centerX = trackArea.getCentreX();
                g.setColour(mutedText);
                g.drawLine(centerX, trackArea.getY() - 2, centerX, trackArea.getBottom() + 2, 1.0f);
            }

            // Thumb
            auto thumbRadius = 10.0f;
            auto thumbArea = juce::Rectangle<float>(sliderPos - thumbRadius,
                trackArea.getCentreY() - thumbRadius,
                thumbRadius * 2.0f,
                thumbRadius * 2.0f);

            // Thumb-Schatten
            g.setColour(juce::Colours::black.withAlpha(0.3f));
            g.fillEllipse(thumbArea.translated(1.0f, 1.0f));

            // Thumb-Hauptfarbe
            g.setColour(slider.isMouseOverOrDragging() ? primaryBlue.brighter(0.2f) : primaryBlue);
            g.fillEllipse(thumbArea);

            // Thumb-Highlight
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillEllipse(thumbArea.reduced(2.0f));
        }
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

        // Pfeil
        auto arrowBounds = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH).reduced(4.0f);
        g.setColour(primaryBlue);

        juce::Path arrow;
        arrow.addTriangle(arrowBounds.getX(), arrowBounds.getY(),
            arrowBounds.getRight(), arrowBounds.getY(),
            arrowBounds.getCentreX(), arrowBounds.getBottom());
        g.fillPath(arrow);
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
#pragma once

#include <JuceHeader.h>

//==============================================================================
class ModernKnob : public juce::Slider
{
public:
    ModernKnob()
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4a9eff));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a2a2a));
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto centre = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 15.0f; // Mehr Platz für Text

        // Background circle
        g.setColour(juce::Colour(0xff333333)); // Einheitliche Farbe
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

        // Outline
        g.setColour(juce::Colour(0xff3a3a3a));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);

        // Value arc
        auto toAngle = juce::MathConstants<float>::pi * 1.5f + (getValue() - getMinimum()) / (getMaximum() - getMinimum()) * juce::MathConstants<float>::pi * 1.5f;
        auto fromAngle = juce::MathConstants<float>::pi * 0.75f;

        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, radius - 3, radius - 3, 0.0f, fromAngle, toAngle, true);

        g.setColour(juce::Colour(0xff4a9eff));
        g.strokePath(valueArc, juce::PathStrokeType(3.0f));

        // Center dot
        g.setColour(juce::Colours::white);
        g.fillEllipse(centre.x - 2, centre.y - 2, 4, 4);

    }
};

//==============================================================================
class FXSection : public juce::Component
{
public:
    FXSection(const juce::String& title) : sectionTitle(title)
    {
        addAndMakeVisible(titleLabel);
        titleLabel.setText(title, juce::dontSendNotification);
        titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        titleLabel.setJustificationType(juce::Justification::centred);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Background - Einheitliche Farbe
        g.setColour(juce::Colour(0xff333333));
        g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

        // Border
        g.setColour(juce::Colour(0xff4a4a4a));
        g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);

        // Title bar
        auto titleArea = bounds.removeFromTop(30);
        g.setColour(juce::Colour(0xff333333)); // Gleiche Farbe wie Background
        g.fillRoundedRectangle(titleArea.toFloat(), 8.0f);
        g.fillRect(titleArea.removeFromBottom(8));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        titleLabel.setBounds(bounds.removeFromTop(30));
    }

protected:
    juce::String sectionTitle;
    juce::Label titleLabel;
};

//==============================================================================
class FilterSection : public FXSection
{
public:
    FilterSection() : FXSection("Filter")
    {
        setupKnob(cutoffKnob, "Cutoff", 20.0f, 20000.0f, 1000.0f);
        setupKnob(resonanceKnob, "Resonance", 0.1f, 10.0f, 0.7f);
        setupKnob(driveKnob, "Drive", 1.0f, 5.0f, 1.0f);

        addAndMakeVisible(typeCombo);
        typeCombo.addItem("Low Pass", 1);
        typeCombo.addItem("High Pass", 2);
        typeCombo.addItem("Band Pass", 3);
        typeCombo.addItem("Notch", 4);
        typeCombo.setSelectedId(1);
        typeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
        typeCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        typeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4a4a4a));

        addAndMakeVisible(typeLabel);
        typeLabel.setText("Type", juce::dontSendNotification);
        typeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
        typeLabel.setJustificationType(juce::Justification::centred);
    }

    void resized() override
    {
        FXSection::resized();
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(35); // Title space

        auto topRow = bounds.removeFromTop(80);
        cutoffKnob.setBounds(topRow.removeFromLeft(80));
        resonanceKnob.setBounds(topRow.removeFromLeft(80));
        driveKnob.setBounds(topRow.removeFromLeft(80));

        bounds.removeFromTop(10);
        typeLabel.setBounds(bounds.removeFromTop(20));
        typeCombo.setBounds(bounds.removeFromTop(25).reduced(20, 0));
    }

    ModernKnob cutoffKnob, resonanceKnob, driveKnob;
    juce::ComboBox typeCombo;
    juce::Label typeLabel;

private:
    void setupKnob(ModernKnob& knob, const juce::String& suffix, float min, float max, float defaultVal)
    {
        addAndMakeVisible(knob);
        knob.setRange(min, max);
        knob.setValue(defaultVal);
        knob.setTextValueSuffix(suffix == "Cutoff" ? " Hz" : "");
        knob.setNumDecimalPlacesToDisplay(suffix == "Cutoff" ? 0 : 1);
    }
};

//==============================================================================
class EQSection : public FXSection
{
public:
    EQSection() : FXSection("3-Band EQ")
    {
        setupKnob(lowKnob, "Low", -12.0f, 12.0f, 0.0f);
        setupKnob(midKnob, "Mid", -12.0f, 12.0f, 0.0f);
        setupKnob(highKnob, "High", -12.0f, 12.0f, 0.0f);
        setupKnob(lowFreqKnob, "Low Freq", 50.0f, 500.0f, 200.0f);
        setupKnob(highFreqKnob, "High Freq", 2000.0f, 15000.0f, 8000.0f);
    }

    void resized() override
    {
        FXSection::resized();
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(35);

        auto topRow = bounds.removeFromTop(80);
        lowKnob.setBounds(topRow.removeFromLeft(80));
        midKnob.setBounds(topRow.removeFromLeft(80));
        highKnob.setBounds(topRow.removeFromLeft(80));

        bounds.removeFromTop(10);
        auto bottomRow = bounds.removeFromTop(80);
        bottomRow.removeFromLeft(20); // Center the freq knobs
        lowFreqKnob.setBounds(bottomRow.removeFromLeft(80));
        bottomRow.removeFromLeft(40);
        highFreqKnob.setBounds(bottomRow.removeFromLeft(80));
    }

    ModernKnob lowKnob, midKnob, highKnob;
    ModernKnob lowFreqKnob, highFreqKnob;

private:
    void setupKnob(ModernKnob& knob, const juce::String& name, float min, float max, float defaultVal)
    {
        addAndMakeVisible(knob);
        knob.setRange(min, max);
        knob.setValue(defaultVal);
        if (name.contains("Freq"))
            knob.setTextValueSuffix(" Hz");
        else
            knob.setTextValueSuffix(" dB");
        knob.setNumDecimalPlacesToDisplay(name.contains("Freq") ? 0 : 1);
    }
};

//==============================================================================
class ChorusSection : public FXSection
{
public:
    ChorusSection() : FXSection("Chorus")
    {
        setupKnob(rateKnob, "Rate", 0.1f, 10.0f, 2.0f);
        setupKnob(depthKnob, "Depth", 0.0f, 1.0f, 0.5f);
        setupKnob(feedbackKnob, "Feedback", 0.0f, 0.95f, 0.3f);
        setupKnob(mixKnob, "Mix", 0.0f, 1.0f, 0.4f);
    }

    void resized() override
    {
        FXSection::resized();
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(35);

        auto topRow = bounds.removeFromTop(80);
        rateKnob.setBounds(topRow.removeFromLeft(80));
        depthKnob.setBounds(topRow.removeFromLeft(80));

        bounds.removeFromTop(10);
        auto bottomRow = bounds.removeFromTop(80);
        feedbackKnob.setBounds(bottomRow.removeFromLeft(80));
        mixKnob.setBounds(bottomRow.removeFromLeft(80));
    }

    ModernKnob rateKnob, depthKnob, feedbackKnob, mixKnob;

private:
    void setupKnob(ModernKnob& knob, const juce::String& name, float min, float max, float defaultVal)
    {
        addAndMakeVisible(knob);
        knob.setRange(min, max);
        knob.setValue(defaultVal);

        if (name == "Rate")
            knob.setTextValueSuffix(" Hz");
        else if (name == "Mix" || name == "Depth" || name == "Feedback")
            knob.setTextValueSuffix("%");

        knob.setNumDecimalPlacesToDisplay(1);
    }
};

//==============================================================================
class ReverbSection : public FXSection
{
public:
    ReverbSection() : FXSection("Reverb")
    {
        setupKnob(roomSizeKnob, "Room Size", 0.0f, 1.0f, 0.5f);
        setupKnob(dampingKnob, "Damping", 0.0f, 1.0f, 0.5f);
        setupKnob(wetKnob, "Wet", 0.0f, 1.0f, 0.3f);
        setupKnob(dryKnob, "Dry", 0.0f, 1.0f, 1.0f);
        setupKnob(widthKnob, "Width", 0.0f, 1.0f, 1.0f);
    }

    void resized() override
    {
        FXSection::resized();
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(35);

        auto topRow = bounds.removeFromTop(80);
        roomSizeKnob.setBounds(topRow.removeFromLeft(80));
        dampingKnob.setBounds(topRow.removeFromLeft(80));
        wetKnob.setBounds(topRow.removeFromLeft(80));

        bounds.removeFromTop(10);
        auto bottomRow = bounds.removeFromTop(80);
        bottomRow.removeFromLeft(20);
        dryKnob.setBounds(bottomRow.removeFromLeft(80));
        bottomRow.removeFromLeft(40);
        widthKnob.setBounds(bottomRow.removeFromLeft(80));
    }

    ModernKnob roomSizeKnob, dampingKnob, wetKnob, dryKnob, widthKnob;

private:
    void setupKnob(ModernKnob& knob, const juce::String& name, float min, float max, float defaultVal)
    {
        addAndMakeVisible(knob);
        knob.setRange(min, max);
        knob.setValue(defaultVal);
        knob.setTextValueSuffix("%");
        knob.setNumDecimalPlacesToDisplay(1);
    }
};

//==============================================================================
class FXComponent : public juce::Component
{
public:
    FXComponent()
    {
        addAndMakeVisible(filterSection);
        addAndMakeVisible(eqSection);
        addAndMakeVisible(chorusSection);
        addAndMakeVisible(reverbSection);

        setSize(800, 600);
    }

    void paint(juce::Graphics& g) override
    {
        // Background - Einheitliche Farbe
        juce::ColourGradient bg(juce::Colour(0xff222222), 0, 0,
            juce::Colour(0xff222222), 0, getHeight(), false);
        g.setGradientFill(bg);
        g.fillAll();

        // Main title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("Audio FX Suite", getLocalBounds().removeFromTop(50),
            juce::Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20);
        bounds.removeFromTop(60); // Title space

        auto topRow = bounds.removeFromTop(200);
        filterSection.setBounds(topRow.removeFromLeft(280));
        topRow.removeFromLeft(20);
        eqSection.setBounds(topRow.removeFromLeft(280));

        bounds.removeFromTop(20);
        auto bottomRow = bounds.removeFromTop(200);
        chorusSection.setBounds(bottomRow.removeFromLeft(180));
        bottomRow.removeFromLeft(20);
        reverbSection.setBounds(bottomRow.removeFromLeft(260));
    }

    // Public access to sections for parameter binding
    FilterSection filterSection;
    EQSection eqSection;
    ChorusSection chorusSection;
    ReverbSection reverbSection;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXComponent)
};

//==============================================================================
// Erweiterte FX-Komponente mit zusätzlichen Features
//==============================================================================
class AdvancedFXComponent : public FXComponent
{
public:
    AdvancedFXComponent() : FXComponent()
    {
        // Preset System
        addAndMakeVisible(presetCombo);
        presetCombo.addItem("Init", 1);
        presetCombo.addItem("Warm Filter", 2);
        presetCombo.addItem("Bright EQ", 3);
        presetCombo.addItem("Chorus Space", 4);
        presetCombo.addItem("Hall Reverb", 5);
        presetCombo.addItem("Vintage Combo", 6);
        presetCombo.setSelectedId(1);
        presetCombo.onChange = [this] { loadPreset(); };
        presetCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
        presetCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);

        addAndMakeVisible(presetLabel);
        presetLabel.setText("Presets", juce::dontSendNotification);
        presetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        presetLabel.setFont(juce::Font(14.0f, juce::Font::bold));

        // Bypass Buttons
        setupBypassButton(filterBypass, "Filter");
        setupBypassButton(eqBypass, "EQ");
        setupBypassButton(chorusBypass, "Chorus");
        setupBypassButton(reverbBypass, "Reverb");

        // Master Controls
        addAndMakeVisible(masterVolumeKnob);
        masterVolumeKnob.setRange(-60.0f, 6.0f);
        masterVolumeKnob.setValue(0.0f);
        masterVolumeKnob.setTextValueSuffix(" dB");
        masterVolumeKnob.setNumDecimalPlacesToDisplay(1);

        addAndMakeVisible(masterVolumeLabel);
        masterVolumeLabel.setText("Master", juce::dontSendNotification);
        masterVolumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        masterVolumeLabel.setJustificationType(juce::Justification::centred);

        setSize(950, 700); // Größer für besseres Layout
    }

    void paint(juce::Graphics& g) override
    {
        FXComponent::paint(g);

        // Status LEDs rechts neben Bypass Buttons
        auto bounds = getLocalBounds();
        int ledX = bounds.getWidth() - 90;

        drawStatusLED(g, juce::Point<int>(ledX, 120), !filterBypass.getToggleState(), "Filter");
        drawStatusLED(g, juce::Point<int>(ledX, 160), !eqBypass.getToggleState(), "EQ");
        drawStatusLED(g, juce::Point<int>(ledX, 200), !chorusBypass.getToggleState(), "Chorus");
        drawStatusLED(g, juce::Point<int>(ledX, 240), !reverbBypass.getToggleState(), "Reverb");
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20);

        // Titel-Bereich
        bounds.removeFromTop(60);

        // Preset und Master Controls oben - BESSER SPACING
        auto topControls = bounds.removeFromTop(100);

        auto presetArea = topControls.removeFromLeft(250);
        presetLabel.setBounds(presetArea.removeFromTop(20));
        presetArea.removeFromTop(5);
        presetCombo.setBounds(presetArea.removeFromTop(30));

        auto masterArea = topControls.removeFromRight(250);
        masterVolumeLabel.setBounds(masterArea.removeFromTop(25));
        masterArea.removeFromTop(10);
        masterVolumeKnob.setBounds(masterArea.removeFromTop(90));

        // Bypass Buttons rechts - VERBESSERTES LAYOUT
        auto rightPanel = bounds.removeFromRight(150);
        rightPanel.removeFromTop(10);

        // Bypass Buttons vertikal mit mehr Platz
        auto filterRow = rightPanel.removeFromTop(40);
        filterBypass.setBounds(filterRow.removeFromLeft(50));

        auto eqRow = rightPanel.removeFromTop(40);
        eqBypass.setBounds(eqRow.removeFromLeft(50));

        auto chorusRow = rightPanel.removeFromTop(40);
        chorusBypass.setBounds(chorusRow.removeFromLeft(50));

        auto reverbRow = rightPanel.removeFromTop(40);
        reverbBypass.setBounds(reverbRow.removeFromLeft(50));

        bounds.removeFromRight(30);
        bounds.removeFromTop(20);

        // FX Sections Layout - MEHR PLATZ
        auto topRow = bounds.removeFromTop(200);
        filterSection.setBounds(topRow.removeFromLeft(300));
        topRow.removeFromLeft(30);
        eqSection.setBounds(topRow.removeFromLeft(300));

        bounds.removeFromTop(20);
        auto bottomRow = bounds.removeFromTop(200);
        chorusSection.setBounds(bottomRow.removeFromLeft(200));
        bottomRow.removeFromLeft(30);
        reverbSection.setBounds(bottomRow.removeFromLeft(280));
    }

    // Öffentliche Member für Parameter Binding
    juce::ToggleButton filterBypass, eqBypass, chorusBypass, reverbBypass;
    ModernKnob masterVolumeKnob;
    juce::ComboBox presetCombo;

private:
    juce::Label presetLabel, masterVolumeLabel;

    void setupBypassButton(juce::ToggleButton& button, const juce::String& name)
    {
        addAndMakeVisible(button);
        button.setButtonText("BYP");  // Kurzer Text
        button.setSize(40, 25);       // Größer

        // Bessere Toggle Button Styling
        button.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        button.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff00ff00));        // Grün wenn AN
        button.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xffff4444)); // Rot wenn AUS

        // Callback für State-Synchronisation
        button.onClick = [this, &button]()
            {
                repaint(); // LED Update
            };
    }

    void drawStatusLED(juce::Graphics& g, juce::Point<int> position, bool isActive, const juce::String& label)
    {
        auto ledBounds = juce::Rectangle<float>(position.x, position.y, 15, 15);

        // LED Background
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillEllipse(ledBounds);

        // LED Color - Invertiert: Bypass = Rot, Aktiv = Grün
        if (isActive)
        {
            g.setColour(juce::Colour(0xff00ff00)); // Grün = FX ist aktiv
            g.fillEllipse(ledBounds.reduced(3));

            // Glow Effect
            g.setColour(juce::Colour(0x4000ff00));
            g.fillEllipse(ledBounds.expanded(2));
        }
        else
        {
            g.setColour(juce::Colour(0xffff0000)); // Rot = FX ist bypassed
            g.fillEllipse(ledBounds.reduced(3));

            // Glow Effect
            g.setColour(juce::Colour(0x40ff0000));
            g.fillEllipse(ledBounds.expanded(2));
        }

        // Border
        g.setColour(juce::Colour(0xff666666));
        g.drawEllipse(ledBounds, 1.5f);

        // Label rechts neben LED
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        auto textBounds = juce::Rectangle<float>(position.x + 25, position.y - 2, 80, 18);
        g.drawText(label, textBounds, juce::Justification::centredLeft);
    }

    void loadPreset()
    {
        auto presetId = presetCombo.getSelectedId();

        switch (presetId)
        {
        case 1: loadInitPreset(); break;
        case 2: loadWarmFilterPreset(); break;
        case 3: loadBrightEQPreset(); break;
        case 4: loadChorusSpacePreset(); break;
        case 5: loadHallReverbPreset(); break;
        case 6: loadVintageComboPreset(); break;
        }
    }

    void loadInitPreset()
    {
        filterSection.cutoffKnob.setValue(1000.0f);
        filterSection.resonanceKnob.setValue(0.7f);
        filterSection.driveKnob.setValue(1.0f);
        filterSection.typeCombo.setSelectedId(1);

        eqSection.lowKnob.setValue(0.0f);
        eqSection.midKnob.setValue(0.0f);
        eqSection.highKnob.setValue(0.0f);

        chorusSection.rateKnob.setValue(2.0f);
        chorusSection.depthKnob.setValue(0.5f);
        chorusSection.mixKnob.setValue(0.4f);

        reverbSection.roomSizeKnob.setValue(0.5f);
        reverbSection.wetKnob.setValue(0.3f);

        masterVolumeKnob.setValue(0.0f);
    }

    void loadWarmFilterPreset()
    {
        filterSection.cutoffKnob.setValue(800.0f);
        filterSection.resonanceKnob.setValue(1.2f);
        filterSection.driveKnob.setValue(2.0f);
        filterSection.typeCombo.setSelectedId(1);

        eqSection.lowKnob.setValue(2.0f);
        eqSection.midKnob.setValue(-1.0f);
        eqSection.highKnob.setValue(-2.0f);

        masterVolumeKnob.setValue(-1.0f);
    }

    void loadBrightEQPreset()
    {
        eqSection.lowKnob.setValue(-1.0f);
        eqSection.midKnob.setValue(1.0f);
        eqSection.highKnob.setValue(3.0f);
        eqSection.highFreqKnob.setValue(10000.0f);

        filterSection.cutoffKnob.setValue(15000.0f);
        masterVolumeKnob.setValue(0.0f);
    }

    void loadChorusSpacePreset()
    {
        chorusSection.rateKnob.setValue(1.5f);
        chorusSection.depthKnob.setValue(0.7f);
        chorusSection.feedbackKnob.setValue(0.4f);
        chorusSection.mixKnob.setValue(0.6f);

        reverbSection.roomSizeKnob.setValue(0.7f);
        reverbSection.wetKnob.setValue(0.4f);
        reverbSection.widthKnob.setValue(1.0f);

        masterVolumeKnob.setValue(0.0f);
    }

    void loadHallReverbPreset()
    {
        reverbSection.roomSizeKnob.setValue(0.9f);
        reverbSection.dampingKnob.setValue(0.3f);
        reverbSection.wetKnob.setValue(0.5f);
        reverbSection.widthKnob.setValue(1.0f);

        eqSection.highKnob.setValue(1.0f);
        masterVolumeKnob.setValue(-2.0f);
    }

    void loadVintageComboPreset()
    {
        filterSection.cutoffKnob.setValue(1200.0f);
        filterSection.driveKnob.setValue(3.0f);

        eqSection.lowKnob.setValue(2.0f);
        eqSection.midKnob.setValue(1.5f);
        eqSection.highKnob.setValue(-1.0f);

        chorusSection.rateKnob.setValue(0.8f);
        chorusSection.mixKnob.setValue(0.3f);

        reverbSection.roomSizeKnob.setValue(0.6f);
        reverbSection.dampingKnob.setValue(0.7f);
        reverbSection.wetKnob.setValue(0.2f);

        masterVolumeKnob.setValue(-1.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancedFXComponent)
};


/*
  ==============================================================================

    BaseComponent.cpp
    Created: 29 Aug 2025 6:23:38pm
    Author:  mpue

  ==============================================================================
*/

#include "BaseComponent.h"

BaseComponent::BaseComponent()
{
    // Label für die Größenanzeige erstellen (initial versteckt)
    sizeLabel = std::make_unique<juce::Label>();
    sizeLabel->setJustificationType(juce::Justification::centred);
    sizeLabel->setFont(juce::Font(14.0f, juce::Font::bold));
    sizeLabel->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.7f));
    sizeLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    sizeLabel->setVisible(false);
    addAndMakeVisible(*sizeLabel);
}

void BaseComponent::setSizeDisplayEnabled(bool enabled)
{
    sizeDisplayEnabled = enabled;

    if (!enabled)
        hideSizeDisplay();
}

bool BaseComponent::isSizeDisplayEnabled() const
{
    return sizeDisplayEnabled;
}

void BaseComponent::resized()
{
    // Erst die abgeleitete Klasse ihre Arbeit machen lassen
    onResized();

    // Dann die Größenanzeige aktualisieren falls aktiviert
    if (sizeDisplayEnabled)
    {
        showSizeDisplay();

        // Timer starten um das Label nach 2 Sekunden zu verstecken
        juce::Timer::callAfterDelay(2000, [this]()
            {
                hideSizeDisplay();
            });
    }
}

void BaseComponent::showSizeDisplay()
{
    if (!sizeLabel)
        return;

    auto bounds = getLocalBounds();
    juce::String sizeText = juce::String(bounds.getWidth()) + " x " + juce::String(bounds.getHeight());

    sizeLabel->setText(sizeText, juce::dontSendNotification);

    // Label in der Mitte der Komponente positionieren
    auto labelBounds = juce::Rectangle<int>(0, 0, 100, 30);
    labelBounds.setCentre(bounds.getCentre());
    sizeLabel->setBounds(labelBounds);

    sizeLabel->setVisible(true);
    sizeLabel->toFront(false);
}

void BaseComponent::hideSizeDisplay()
{
    if (sizeLabel)
        sizeLabel->setVisible(false);
}
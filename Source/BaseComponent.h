/*
  ==============================================================================

    BaseComponent.h
    Created: 29 Aug 2025 6:23:38pm
    Author:  mpue

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * Basiskomponente für alle anderen Komponenten.
 * Zeigt optional die aktuelle Größe an, wenn resized() aufgerufen wird.
 */
class BaseComponent : public juce::Component
{
public:
    BaseComponent();
    virtual ~BaseComponent() = default;

    // Aktiviert/deaktiviert die Größenanzeige
    void setSizeDisplayEnabled(bool enabled);
    bool isSizeDisplayEnabled() const;

    // Überschreibt resized() um die Größenanzeige zu handhaben
    void resized() override;

protected:
    // Diese Methode sollten abgeleitete Klassen überschreiben
    // anstatt resized() direkt zu überschreiben
    virtual void onResized() {}

private:
    bool sizeDisplayEnabled = false;
    std::unique_ptr<juce::Label> sizeLabel;

    void showSizeDisplay();
    void hideSizeDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseComponent)
};
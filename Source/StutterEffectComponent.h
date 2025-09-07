#pragma once

#include <JuceHeader.h>

class StutterEffectComponent : public juce::Component,
    public juce::Timer,
    private juce::Button::Listener
{
public:
    StutterEffectComponent();
    ~StutterEffectComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Audio processing - vereinfachte Version für direkte Buffer-Bearbeitung
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processAudioBuffer(juce::AudioBuffer<float>& buffer);
    void releaseResources();

    // Check if stutter is currently active
    bool isStutterActive() const { return stutterState.isActive; }

private:
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    enum StutterType
    {
        CLASSIC_STUTTER = 0,    // Klassisches wiederholendes Stottern
        GATE_STUTTER,           // Gate-artiges Ein/Aus
        REVERSE_STUTTER,        // Rückwärts-Stutter
        PITCHED_STUTTER         // Pitch-moduliertes Stutter
    };

    // UI Components
    juce::TextButton stutterButton1, stutterButton2, stutterButton3, stutterButton4;
    juce::Label titleLabel;

    // Parameter Controls
    juce::Slider lengthSlider, intensitySlider, feedbackSlider, mixSlider;
    juce::Label lengthLabel, intensityLabel, feedbackLabel, mixLabel;

    // Stutter parameters
    struct StutterParams
    {
        bool isActive = false;
        int subdivisionSamples = 0;
        int currentPosition = 0;
        float fadeCounter = 0.0f;
        StutterType type = CLASSIC_STUTTER;
        bool bufferCaptured = false;

        // User controllable parameters
        float length = 0.5f;        // 0.1 - 2.0 (multiplier for subdivision length)
        float intensity = 0.8f;     // 0.0 - 1.0 (effect strength)
        float feedback = 0.3f;      // 0.0 - 0.9 (for repeating effects)
        float mix = 1.0f;           // 0.0 - 1.0 (dry/wet mix)
    };

    StutterParams stutterState;

    // Audio buffer for storing stuttered audio
    juce::AudioBuffer<float> stutterBuffer;
    juce::AudioBuffer<float> dryBuffer;  // For dry/wet mixing

    // Audio parameters
    double currentSampleRate = 44100.0;
    int currentBufferSize = 512;

    // Stutter timing (16th notes, 8th notes, etc.)
    const int stutterSubdivisions[4] = { 16, 8, 4, 2 }; // 16tel, 8tel, 4tel, halbe Noten

    // Helper functions
    void startStutter(StutterType type, int subdivision);
    void stopStutter();
    void updateParameters();
    void processClassicStutter(juce::AudioBuffer<float>& buffer);
    void processGateStutter(juce::AudioBuffer<float>& buffer);
    void processReverseStutter(juce::AudioBuffer<float>& buffer);
    void processPitchedStutter(juce::AudioBuffer<float>& buffer);
    void applyDryWetMix(juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StutterEffectComponent)
};
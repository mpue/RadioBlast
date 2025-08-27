#include "FXComponent.h"

//==============================================================================
// Beispiel für die Integration in deine Audio-Applikation
class AudioFXProcessor : public juce::AudioProcessor
{
public:
    AudioFXProcessor()
    {
        // Filter Parameter
        addParameter(filterCutoff = new juce::AudioParameterFloat("filterCutoff", "Filter Cutoff",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));
        addParameter(filterResonance = new juce::AudioParameterFloat("filterResonance", "Filter Resonance",
            juce::NormalisableRange<float>(0.1f, 10.0f), 0.7f));
        addParameter(filterDrive = new juce::AudioParameterFloat("filterDrive", "Filter Drive",
            juce::NormalisableRange<float>(1.0f, 5.0f), 1.0f));
        addParameter(filterType = new juce::AudioParameterInt("filterType", "Filter Type", 1, 4, 1));

        // EQ Parameter
        addParameter(eqLow = new juce::AudioParameterFloat("eqLow", "EQ Low",
            juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f));
        addParameter(eqMid = new juce::AudioParameterFloat("eqMid", "EQ Mid",
            juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f));
        addParameter(eqHigh = new juce::AudioParameterFloat("eqHigh", "EQ High",
            juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f));
        addParameter(eqLowFreq = new juce::AudioParameterFloat("eqLowFreq", "EQ Low Freq",
            juce::NormalisableRange<float>(50.0f, 500.0f), 200.0f));
        addParameter(eqHighFreq = new juce::AudioParameterFloat("eqHighFreq", "EQ High Freq",
            juce::NormalisableRange<float>(2000.0f, 15000.0f), 8000.0f));

        // Chorus Parameter
        addParameter(chorusRate = new juce::AudioParameterFloat("chorusRate", "Chorus Rate",
            juce::NormalisableRange<float>(0.1f, 10.0f), 2.0f));
        addParameter(chorusDepth = new juce::AudioParameterFloat("chorusDepth", "Chorus Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
        addParameter(chorusFeedback = new juce::AudioParameterFloat("chorusFeedback", "Chorus Feedback",
            juce::NormalisableRange<float>(0.0f, 0.95f), 0.3f));
        addParameter(chorusMix = new juce::AudioParameterFloat("chorusMix", "Chorus Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.4f));

        // Reverb Parameter
        addParameter(reverbRoomSize = new juce::AudioParameterFloat("reverbRoomSize", "Reverb Room Size",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
        addParameter(reverbDamping = new juce::AudioParameterFloat("reverbDamping", "Reverb Damping",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
        addParameter(reverbWet = new juce::AudioParameterFloat("reverbWet", "Reverb Wet",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
        addParameter(reverbDry = new juce::AudioParameterFloat("reverbDry", "Reverb Dry",
            juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
        addParameter(reverbWidth = new juce::AudioParameterFloat("reverbWidth", "Reverb Width",
            juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        // Filter initialisieren
        for (auto& filter : filters)
        {
            filter.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });
            filter.reset();
        }

        // EQ initialisieren
        lowShelfFilter.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });
        peakingFilter.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });
        highShelfFilter.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });

        // Chorus initialisieren
        chorus.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });

        // Reverb initialisieren
        reverb.prepare({ sampleRate, (juce::uint32)samplesPerBlock, 2 });

        currentSampleRate = sampleRate;
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // Audio Processing Context erstellen
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        // Filter Processing
        updateFilter();
        for (auto& filter : filters)
            filter.process(context);

        // EQ Processing
        updateEQ();
        lowShelfFilter.process(context);
        peakingFilter.process(context);
        highShelfFilter.process(context);

        // Chorus Processing
        updateChorus();
        chorus.process(context);

        // Reverb Processing
        updateReverb();
        reverb.process(context);
    }

    // Notwendige AudioProcessor Methoden (vereinfacht)
    const juce::String getName() const override { return "AudioFX"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

private:
    // Audio Parameter
    juce::AudioParameterFloat* filterCutoff;
    juce::AudioParameterFloat* filterResonance;
    juce::AudioParameterFloat* filterDrive;
    juce::AudioParameterInt* filterType;

    juce::AudioParameterFloat* eqLow;
    juce::AudioParameterFloat* eqMid;
    juce::AudioParameterFloat* eqHigh;
    juce::AudioParameterFloat* eqLowFreq;
    juce::AudioParameterFloat* eqHighFreq;

    juce::AudioParameterFloat* chorusRate;
    juce::AudioParameterFloat* chorusDepth;
    juce::AudioParameterFloat* chorusFeedback;
    juce::AudioParameterFloat* chorusMix;

    juce::AudioParameterFloat* reverbRoomSize;
    juce::AudioParameterFloat* reverbDamping;
    juce::AudioParameterFloat* reverbWet;
    juce::AudioParameterFloat* reverbDry;
    juce::AudioParameterFloat* reverbWidth;

    // DSP Objekte
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> filters;
    juce::dsp::IIR::Filter<float> lowShelfFilter, peakingFilter, highShelfFilter;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;

    double currentSampleRate = 44100.0;

    void updateFilter()
    {
        auto cutoff = filterCutoff->get();
        auto resonance = filterResonance->get();
        auto drive = filterDrive->get();
        auto type = filterType->get();

        for (auto& filter : filters)
        {
            filter.setCutoffFrequency(cutoff);
            filter.setResonance(resonance);

            switch (type)
            {
            case 1: filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); break;
            case 2: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
            case 3: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
            
            }
        }
    }

    void updateEQ()
    {
        // Low Shelf Filter
        auto lowGain = juce::Decibels::decibelsToGain(eqLow->get());
        auto lowFreq = eqLowFreq->get();
        lowShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            currentSampleRate, lowFreq, 0.707f, lowGain);

        // Peaking Filter (Mid)
        auto midGain = juce::Decibels::decibelsToGain(eqMid->get());
        peakingFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, 1000.0f, 1.0f, midGain);

        // High Shelf Filter
        auto highGain = juce::Decibels::decibelsToGain(eqHigh->get());
        auto highFreq = eqHighFreq->get();
        highShelfFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate, highFreq, 0.707f, highGain);
    }

    void updateChorus()
    {
        chorus.setRate(chorusRate->get());
        chorus.setDepth(chorusDepth->get());
        chorus.setFeedback(chorusFeedback->get());
        chorus.setMix(chorusMix->get());
        chorus.setCentreDelay(7.0f);
    }

    void updateReverb()
    {
        juce::dsp::Reverb::Parameters reverbParams;
        reverbParams.roomSize = reverbRoomSize->get();
        reverbParams.damping = reverbDamping->get();
        reverbParams.wetLevel = reverbWet->get();
        reverbParams.dryLevel = reverbDry->get();
        reverbParams.width = reverbWidth->get();
        reverbParams.freezeMode = 0.0f;

        reverb.setParameters(reverbParams);
    }
};

//==============================================================================
class FXAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FXAudioProcessorEditor(AudioFXProcessor& p) : AudioProcessorEditor(&p), processor(p)
    {
        addAndMakeVisible(fxComponent);

        // Parameter Attachments erstellen
        setupAttachments();

        setSize(800, 600);
    }

    void paint(juce::Graphics&) override {}

    void resized() override
    {
        fxComponent.setBounds(getLocalBounds());
    }

private:
    AudioFXProcessor& processor;
    FXComponent fxComponent;

    // Parameter Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;

    // EQ Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqMidAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighFreqAttachment;

    // Chorus Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusMixAttachment;

    // Reverb Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbRoomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbWidthAttachment;

    void setupAttachments()
    {
        // Hinweis: Du benötigst ein AudioProcessorValueTreeState in deinem Processor
        // Hier ist ein vereinfachtes Beispiel wie du die Parameter verbinden würdest:

        /*
        // Filter
        filterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "filterCutoff", fxComponent.filterSection.cutoffKnob);
        filterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "filterResonance", fxComponent.filterSection.resonanceKnob);
        filterDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "filterDrive", fxComponent.filterSection.driveKnob);
        filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.parameters, "filterType", fxComponent.filterSection.typeCombo);

        // EQ
        eqLowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "eqLow", fxComponent.eqSection.lowKnob);
        eqMidAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "eqMid", fxComponent.eqSection.midKnob);
        eqHighAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "eqHigh", fxComponent.eqSection.highKnob);
        eqLowFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "eqLowFreq", fxComponent.eqSection.lowFreqKnob);
        eqHighFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "eqHighFreq", fxComponent.eqSection.highFreqKnob);

        // Chorus
        chorusRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "chorusRate", fxComponent.chorusSection.rateKnob);
        chorusDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "chorusDepth", fxComponent.chorusSection.depthKnob);
        chorusFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "chorusFeedback", fxComponent.chorusSection.feedbackKnob);
        chorusMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "chorusMix", fxComponent.chorusSection.mixKnob);

        // Reverb
        reverbRoomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "reverbRoomSize", fxComponent.reverbSection.roomSizeKnob);
        reverbDampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "reverbDamping", fxComponent.reverbSection.dampingKnob);
        reverbWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "reverbWet", fxComponent.reverbSection.wetKnob);
        reverbDryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "reverbDry", fxComponent.reverbSection.dryKnob);
        reverbWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.parameters, "reverbWidth", fxComponent.reverbSection.widthKnob);
        */
    }
};

// Editor Factory Methode
juce::AudioProcessorEditor* AudioFXProcessor::createEditor()
{
    return new FXAudioProcessorEditor(*this);
}


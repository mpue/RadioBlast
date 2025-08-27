#pragma once

#include <JuceHeader.h>

//==============================================================================
// Forward Declarations
//==============================================================================

//==============================================================================
class FXParameterHelper
{
public:
    // Parameter zu AudioProcessorValueTreeState hinzufügen
    static void addFXParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const juce::String& prefix = "")
    {
        juce::String p = prefix.isEmpty() ? "" : prefix + "";

        // Filter Parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "FilterCutoff", p + " Filter Cutoff",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " Hz"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "FilterResonance", p + " Filter Resonance",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 0.7f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "FilterDrive", p + " Filter Drive",
            juce::NormalisableRange<float>(1.0f, 5.0f, 0.01f), 1.0f));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            p + "FilterType", p + " Filter Type",
            juce::StringArray{ "Low Pass", "High Pass", "Band Pass", "Notch" }, 0));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            p + "FilterBypass", p + " Filter Bypass", false));

        // EQ Parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "EqLow", p + " EQ Low",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "EqMid", p + " EQ Mid",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "EqHigh", p + " EQ High",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "EqLowFreq", p + " EQ Low Freq",
            juce::NormalisableRange<float>(50.0f, 500.0f, 1.0f), 200.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " Hz"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "EqHighFreq", p + " EQ High Freq",
            juce::NormalisableRange<float>(2000.0f, 15000.0f, 1.0f), 8000.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " Hz"; }));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            p + "EqBypass", p + " EQ Bypass", false));

        // Chorus Parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ChorusRate", p + " Chorus Rate",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 2.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " Hz"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ChorusDepth", p + " Chorus Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ChorusFeedback", p + " Chorus Feedback",
            juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.3f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ChorusMix", p + " Chorus Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            p + "ChorusBypass", p + " Chorus Bypass", false));

        // Reverb Parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ReverbRoomSize", p + " Reverb Room Size",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ReverbDamping", p + " Reverb Damping",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ReverbWet", p + " Reverb Wet",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ReverbDry", p + " Reverb Dry",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "ReverbWidth", p + " Reverb Width",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            p + "ReverbBypass", p + " Reverb Bypass", false));

        // Master Parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            p + "MasterVolume", p + " Master Volume",
            juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }));
    }
};

//==============================================================================
// Preset Manager für die FX Suite
//==============================================================================
class FXPresetManager
{
public:
    struct PresetData
    {
        juce::String name;
        std::map<juce::String, float> parameters;
    };

    FXPresetManager()
    {
        createFactoryPresets();
    }

    const std::vector<PresetData>& getPresets() const { return presets; }

    void loadPreset(const PresetData& preset, juce::AudioProcessorValueTreeState& parameters)
    {
        for (const auto& param : preset.parameters)
        {
            if (auto* parameter = parameters.getParameter(param.first))
            {
                parameter->setValueNotifyingHost(parameter->convertTo0to1(param.second));
            }
        }
    }

    PresetData getCurrentPreset(juce::AudioProcessorValueTreeState& parameters)
    {
        PresetData currentPreset;
        currentPreset.name = "Current";

        for (auto* param : parameters.processor.getParameters())
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                currentPreset.parameters[rangedParam->paramID] = rangedParam->convertFrom0to1(rangedParam->getValue());
            }
        }

        return currentPreset;
    }

    void savePreset(const juce::String& name, juce::AudioProcessorValueTreeState& parameters)
    {
        PresetData newPreset = getCurrentPreset(parameters);
        newPreset.name = name;
        userPresets.push_back(newPreset);

        // Optional: In Datei speichern
        savePresetsToFile();
    }

    void loadPresetsFromFile()
    {
        auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        auto presetFile = appDataDir.getChildFile("AudioFXSuite").getChildFile("presets.xml");

        if (presetFile.existsAsFile())
        {
            auto xml = juce::XmlDocument::parse(presetFile);
            if (xml != nullptr)
            {
                userPresets.clear();
                for (auto* presetXml : xml->getChildIterator())
                {
                    PresetData preset;
                    preset.name = presetXml->getStringAttribute("name");

                    for (auto* paramXml : presetXml->getChildIterator())
                    {
                        auto paramName = paramXml->getStringAttribute("id");
                        auto paramValue = (float)paramXml->getDoubleAttribute("value");
                        preset.parameters[paramName] = paramValue;
                    }

                    userPresets.push_back(preset);
                }
            }
        }
    }

    void savePresetsToFile()
    {
        auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        auto presetDir = appDataDir.getChildFile("AudioFXSuite");
        presetDir.createDirectory();

        auto presetFile = presetDir.getChildFile("presets.xml");

        juce::XmlElement xml("presets");

        for (const auto& preset : userPresets)
        {
            auto* presetXml = xml.createNewChildElement("preset");
            presetXml->setAttribute("name", preset.name);

            for (const auto& param : preset.parameters)
            {
                auto* paramXml = presetXml->createNewChildElement("parameter");
                paramXml->setAttribute("id", param.first);
                paramXml->setAttribute("value", param.second);
            }
        }

        xml.writeTo(presetFile);
    }

private:
    std::vector<PresetData> presets;
    std::vector<PresetData> userPresets;

    void createFactoryPresets()
    {
        // Init Preset
        presets.push_back({ "Init", {
            {"filterCutoff", 1000.0f}, {"filterResonance", 0.7f}, {"filterDrive", 1.0f}, {"filterType", 0.0f},
            {"eqLow", 0.0f}, {"eqMid", 0.0f}, {"eqHigh", 0.0f}, {"eqLowFreq", 200.0f}, {"eqHighFreq", 8000.0f},
            {"chorusRate", 2.0f}, {"chorusDepth", 0.5f}, {"chorusFeedback", 0.3f}, {"chorusMix", 0.4f},
            {"reverbRoomSize", 0.5f}, {"reverbDamping", 0.5f}, {"reverbWet", 0.3f}, {"reverbDry", 1.0f}, {"reverbWidth", 1.0f},
            {"masterVolume", 0.0f}
        } });

        // Warm Filter Preset
        presets.push_back({ "Warm Filter", {
            {"filterCutoff", 800.0f}, {"filterResonance", 1.2f}, {"filterDrive", 2.0f}, {"filterType", 0.0f},
            {"eqLow", 2.0f}, {"eqMid", -1.0f}, {"eqHigh", -2.0f}, {"eqLowFreq", 200.0f}, {"eqHighFreq", 8000.0f},
            {"chorusRate", 2.0f}, {"chorusDepth", 0.3f}, {"chorusFeedback", 0.2f}, {"chorusMix", 0.2f},
            {"reverbRoomSize", 0.4f}, {"reverbDamping", 0.6f}, {"reverbWet", 0.2f}, {"reverbDry", 1.0f}, {"reverbWidth", 0.8f},
            {"masterVolume", -1.0f}
        } });

        // Bright EQ Preset
        presets.push_back({ "Bright EQ", {
            {"filterCutoff", 15000.0f}, {"filterResonance", 0.7f}, {"filterDrive", 1.0f}, {"filterType", 0.0f},
            {"eqLow", -1.0f}, {"eqMid", 1.0f}, {"eqHigh", 3.0f}, {"eqLowFreq", 200.0f}, {"eqHighFreq", 10000.0f},
            {"chorusRate", 2.0f}, {"chorusDepth", 0.4f}, {"chorusFeedback", 0.3f}, {"chorusMix", 0.3f},
            {"reverbRoomSize", 0.3f}, {"reverbDamping", 0.3f}, {"reverbWet", 0.1f}, {"reverbDry", 1.0f}, {"reverbWidth", 1.0f},
            {"masterVolume", 0.0f}
        } });

        // Chorus Space Preset
        presets.push_back({ "Chorus Space", {
            {"filterCutoff", 5000.0f}, {"filterResonance", 0.8f}, {"filterDrive", 1.2f}, {"filterType", 0.0f},
            {"eqLow", 0.0f}, {"eqMid", 0.5f}, {"eqHigh", 1.0f}, {"eqLowFreq", 200.0f}, {"eqHighFreq", 8000.0f},
            {"chorusRate", 1.5f}, {"chorusDepth", 0.7f}, {"chorusFeedback", 0.4f}, {"chorusMix", 0.6f},
            {"reverbRoomSize", 0.7f}, {"reverbDamping", 0.4f}, {"reverbWet", 0.4f}, {"reverbDry", 0.8f}, {"reverbWidth", 1.0f},
            {"masterVolume", 0.0f}
        } });

        // Hall Reverb Preset
        presets.push_back({ "Hall Reverb", {
            {"filterCutoff", 8000.0f}, {"filterResonance", 0.5f}, {"filterDrive", 1.0f}, {"filterType", 0.0f},
            {"eqLow", 0.0f}, {"eqMid", 0.0f}, {"eqHigh", 1.0f}, {"eqLowFreq", 200.0f}, {"eqHighFreq", 8000.0f},
            {"chorusRate", 2.0f}, {"chorusDepth", 0.3f}, {"chorusFeedback", 0.2f}, {"chorusMix", 0.2f},
            {"reverbRoomSize", 0.9f}, {"reverbDamping", 0.3f}, {"reverbWet", 0.5f}, {"reverbDry", 0.8f}, {"reverbWidth", 1.0f},
            {"masterVolume", -2.0f}
        } });

        // Vintage Combo Preset
        presets.push_back({ "Vintage Combo", {
            {"filterCutoff", 1200.0f}, {"filterResonance", 1.0f}, {"filterDrive", 3.0f}, {"filterType", 0.0f},
            {"eqLow", 2.0f}, {"eqMid", 1.5f}, {"eqHigh", -1.0f}, {"eqLowFreq", 200.0f}, {"eqHighFreq", 6000.0f},
            {"chorusRate", 0.8f}, {"chorusDepth", 0.4f}, {"chorusFeedback", 0.3f}, {"chorusMix", 0.3f},
            {"reverbRoomSize", 0.6f}, {"reverbDamping", 0.7f}, {"reverbWet", 0.2f}, {"reverbDry", 1.0f}, {"reverbWidth", 0.8f},
            {"masterVolume", -1.0f}
        } });
    }
};

//==============================================================================
// Spectrum Analyzer Komponente
//==============================================================================
class SpectrumAnalyzer : public juce::Component, private juce::Timer
{
public:
    SpectrumAnalyzer()
    {
        startTimerHz(30); // 30 FPS für bessere Performance

        for (int i = 0; i < fftSize; ++i)
            window[i] = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1));
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xff0a0a0a));
        g.fillRoundedRectangle(bounds, 4.0f);

        // Grid
        g.setColour(juce::Colour(0xff2a2a2a));
        for (int i = 1; i < 10; ++i)
        {
            auto x = bounds.getX() + bounds.getWidth() * i / 10.0f;
            g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
        }

        for (int i = 1; i < 5; ++i)
        {
            auto y = bounds.getY() + bounds.getHeight() * i / 5.0f;
            g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
        }

        // Spectrum
        if (!spectrum.empty())
        {
            juce::Path spectrumPath;
            spectrumPath.startNewSubPath(bounds.getX(), bounds.getBottom());

            for (size_t i = 1; i < spectrum.size(); ++i)
            {
                auto x = bounds.getX() + bounds.getWidth() * std::log(i) / std::log(spectrum.size());
                auto y = bounds.getBottom() - bounds.getHeight() * spectrum[i];

                if (i == 1)
                    spectrumPath.lineTo(x, y);
                else
                    spectrumPath.lineTo(x, y);
            }

            spectrumPath.lineTo(bounds.getRight(), bounds.getBottom());
            spectrumPath.closeSubPath();

            // Gradient fill
            juce::ColourGradient gradient(juce::Colour(0x804a9eff), bounds.getX(), bounds.getBottom(),
                juce::Colour(0x004a9eff), bounds.getX(), bounds.getY(), false);
            g.setGradientFill(gradient);
            g.fillPath(spectrumPath);

            // Outline
            g.setColour(juce::Colour(0xff4a9eff));
            g.strokePath(spectrumPath, juce::PathStrokeType(1.0f));
        }

        // Border
        g.setColour(juce::Colour(0xff4a4a4a));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Labels
        g.setColour(juce::Colours::lightgrey);
        g.setFont(10.0f);
        g.drawText("20Hz", bounds.getX() + 5, bounds.getBottom() - 15, 40, 12, juce::Justification::centredLeft);
        g.drawText("20kHz", bounds.getRight() - 45, bounds.getBottom() - 15, 40, 12, juce::Justification::centredRight);
    }

    void processAudioBuffer(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() == 0)
            return;

        auto* channelData = buffer.getReadPointer(0);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            fifoBuffer[fifoIndex] = channelData[i];
            fifoIndex = (fifoIndex + 1) % fftSize;
        }

        newDataAvailable = true;
    }

private:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT fft{ fftOrder };
    std::array<float, fftSize * 2> fftData;
    std::array<float, fftSize> fifoBuffer;
    std::array<float, fftSize> window;
    std::vector<float> spectrum;

    int fifoIndex = 0;
    bool newDataAvailable = false;

    void timerCallback() override
    {
        if (newDataAvailable)
        {
            // Copy FIFO to FFT buffer and apply window
            for (int i = 0; i < fftSize; ++i)
            {
                fftData[i] = fifoBuffer[(fifoIndex + i) % fftSize] * window[i];
                fftData[i + fftSize] = 0.0f;
            }

            // Perform FFT
            fft.performFrequencyOnlyForwardTransform(fftData.data());

            // Convert to spectrum
            if (spectrum.empty())
                spectrum.resize(fftSize / 2);

            for (int i = 0; i < fftSize / 2; ++i)
            {
                auto magnitude = std::sqrt(fftData[i] * fftData[i] + fftData[i + fftSize] * fftData[i + fftSize]);
                spectrum[i] = juce::jlimit(0.0f, 1.0f, magnitude * 4.0f);
            }

            newDataAvailable = false;
            repaint();
        }
    }
};


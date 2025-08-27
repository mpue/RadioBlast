#pragma once

#include <JuceHeader.h>

//==============================================================================
struct MidiEvent
{
    juce::Time timestamp;
    juce::String channel;
    juce::String type;
    juce::String data1;
    juce::String data2;
    juce::String rawHex;

    MidiEvent(const juce::MidiMessage& message)
        : timestamp(juce::Time::getCurrentTime())
    {
        channel = juce::String(message.getChannel());

        if (message.isNoteOn())
        {
            type = "Note On";
            data1 = juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4);
            data2 = juce::String(message.getVelocity());
        }
        else if (message.isNoteOff())
        {
            type = "Note Off";
            data1 = juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4);
            data2 = juce::String(message.getVelocity());
        }
        else if (message.isController())
        {
            type = "CC";
            data1 = juce::String(message.getControllerNumber());
            data2 = juce::String(message.getControllerValue());
        }
        else if (message.isPitchWheel())
        {
            type = "Pitch Bend";
            data1 = juce::String(message.getPitchWheelValue());
            data2 = "";
        }
        else if (message.isProgramChange())
        {
            type = "Program Change";
            data1 = juce::String(message.getProgramChangeNumber());
            data2 = "";
        }
        else if (message.isAftertouch())
        {
            type = "Aftertouch";
            data1 = juce::String(message.getAfterTouchValue());
            data2 = "";
        }
        else if (message.isMidiClock())
        {
            type = "Clock";
            data1 = "";
            data2 = "";
        }
        else if (message.isMidiStart())
        {
            type = "Start";
            data1 = "";
            data2 = "";
        }
        else if (message.isMidiStop())
        {
            type = "Stop";
            data1 = "";
            data2 = "";
        }
        else if (message.isMidiContinue())
        {
            type = "Continue";
            data1 = "";
            data2 = "";
        }
        else
        {
            type = "Other";
            data1 = "";
            data2 = "";
        }

        // Raw hex data
        rawHex = "";
        for (int i = 0; i < message.getRawDataSize(); ++i)
        {
            if (i > 0) rawHex += " ";
            rawHex += juce::String::toHexString(message.getRawData()[i]).paddedLeft('0', 2);
        }
    }
};

//==============================================================================
class MidiMonitorComponent : public juce::Component,
    public juce::TableListBoxModel,
    private juce::Timer
{
public:
    MidiMonitorComponent()
        : maxEvents(1000), autoScroll(true)
    {
        // Setup table
        addAndMakeVisible(table);
        table.setModel(this);
        table.setMultipleSelectionEnabled(false);

        // Setup columns
        table.getHeader().addColumn("Time", 1, 100);
        table.getHeader().addColumn("Channel", 2, 60);
        table.getHeader().addColumn("Type", 3, 100);
        table.getHeader().addColumn("Data 1", 4, 80);
        table.getHeader().addColumn("Data 2", 5, 80);
        table.getHeader().addColumn("Raw Hex", 6, 120);

        table.getHeader().setStretchToFitActive(true);

        // Control buttons
        addAndMakeVisible(clearButton);
        clearButton.setButtonText("Clear");
        clearButton.onClick = [this] { clearEvents(); };

        addAndMakeVisible(pauseButton);
        pauseButton.setButtonText("Pause");
        pauseButton.setToggleable(true);
        pauseButton.onClick = [this] {
            isPaused = pauseButton.getToggleState();
            pauseButton.setButtonText(isPaused ? "Resume" : "Pause");
            };

        addAndMakeVisible(autoScrollButton);
        autoScrollButton.setButtonText("Auto Scroll");
        autoScrollButton.setToggleable(true);
        autoScrollButton.setToggleState(autoScroll, juce::dontSendNotification);
        autoScrollButton.onClick = [this] {
            autoScroll = autoScrollButton.getToggleState();
            };

        // Event counter label
        addAndMakeVisible(eventCountLabel);
        eventCountLabel.setText("Events: 0", juce::dontSendNotification);

        // Start timer for UI updates
        startTimer(50); // Update UI every 50ms
    }

    ~MidiMonitorComponent() override
    {
        stopTimer();
    }

    void addMidiEvent(const juce::MidiMessage& message)
    {
        if (isPaused) return;

        const juce::ScopedLock lock(eventLock);

        events.emplace_back(message);

        // Limit number of events to prevent memory issues
        while (events.size() > maxEvents)
            events.erase(events.begin());

        needsTableUpdate = true;
    }

    void clearEvents()
    {
        const juce::ScopedLock lock(eventLock);
        events.clear();
        needsTableUpdate = true;
    }

    void setMaxEvents(int max) { maxEvents = max; }

    // Component
    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("MIDI Monitor", getLocalBounds().removeFromTop(30), juce::Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(30); // Title space

        auto controlArea = bounds.removeFromTop(40);
        controlArea.removeFromLeft(10);

        clearButton.setBounds(controlArea.removeFromLeft(80));
        controlArea.removeFromLeft(10);
        pauseButton.setBounds(controlArea.removeFromLeft(80));
        controlArea.removeFromLeft(10);
        autoScrollButton.setBounds(controlArea.removeFromLeft(100));
        controlArea.removeFromLeft(20);
        eventCountLabel.setBounds(controlArea.removeFromLeft(150));

        bounds.removeFromTop(10);
        table.setBounds(bounds);
    }

    // TableListBoxModel
    int getNumRows() override
    {
        const juce::ScopedLock lock(eventLock);
        return static_cast<int>(events.size());
    }

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue.withAlpha(0.3f));
        else if (rowNumber % 2 == 0)
            g.fillAll(juce::Colours::lightgrey.withAlpha(0.1f));
    }

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        const juce::ScopedLock lock(eventLock);

        if (rowNumber >= events.size())
            return;

        const auto& event = events[rowNumber];
        juce::String text;

        switch (columnId)
        {
        case 1: text = event.timestamp.toString(false, true, true, true); break;
        case 2: text = event.channel; break;
        case 3: text = event.type; break;
        case 4: text = event.data1; break;
        case 5: text = event.data2; break;
        case 6: text = event.rawHex; break;
        }

        g.setColour(rowIsSelected ? juce::Colours::lightblue : juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }

private:
    juce::TableListBox table;
    std::vector<MidiEvent> events;
    juce::CriticalSection eventLock;

    juce::TextButton clearButton;
    juce::TextButton pauseButton;
    juce::TextButton autoScrollButton;
    juce::Label eventCountLabel;

    bool isPaused = false;
    bool autoScroll = true;
    bool needsTableUpdate = false;
    int maxEvents;

    void timerCallback() override
    {
        if (needsTableUpdate)
        {
            table.updateContent();

            // Update event counter
            eventCountLabel.setText("Events: " + juce::String(events.size()), juce::dontSendNotification);

            // Auto scroll to bottom
            if (autoScroll && !events.empty())
            {
                table.selectRow(static_cast<int>(events.size()) - 1);
                table.scrollToEnsureRowIsOnscreen(static_cast<int>(events.size()) - 1);
            }

            needsTableUpdate = false;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMonitorComponent)
};


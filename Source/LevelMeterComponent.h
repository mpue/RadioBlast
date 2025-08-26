/*
  ==============================================================================

    LevelMeterComponent.h
    Created: 26 Aug 2025 11:25:30am
    Author:  mpue

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LevelMeter : public juce::Component, public juce::Timer
{
public:
    LevelMeter()
    {
        setSize(20, 200);
        startTimerHz(30); // 30 FPS update rate
        reset();
    }

    ~LevelMeter() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds, 2.0f);

        // Border
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);

        // Level bars
        const int numSegments = 40;
        const float segmentHeight = (bounds.getHeight() - 4.0f) / numSegments;
        const float segmentWidth = bounds.getWidth() - 4.0f;

        for (int i = 0; i < numSegments; ++i)
        {
            float segmentTop = bounds.getBottom() - 2.0f - (i + 1) * segmentHeight;
            float threshold = (float)i / numSegments;

            juce::Rectangle<float> segmentRect(bounds.getX() + 2.0f, segmentTop, segmentWidth, segmentHeight - 1.0f);

            if (currentLevel > threshold)
            {
                // Color coding: Green -> Yellow -> Red
                juce::Colour segmentColor;
                if (i < 12) // Green zone (0-60%)
                    segmentColor = juce::Colours::green;
                else if (i < 16) // Yellow zone (60-80%)
                    segmentColor = juce::Colours::yellow;
                else // Red zone (80-100%)
                    segmentColor = juce::Colours::red;

                g.setColour(segmentColor);
                g.fillRoundedRectangle(segmentRect, 1.0f);
            }
            else
            {
                // Inactive segment
                g.setColour(juce::Colour(0xff333333));
                g.fillRoundedRectangle(segmentRect, 1.0f);
            }
        }

        // Peak hold indicator
        if (peakLevel > 0.0f)
        {
            int peakSegment = (int)(peakLevel * numSegments);
            if (peakSegment < numSegments)
            {
                float peakTop = bounds.getBottom() - 2.0f - (peakSegment + 1) * segmentHeight;
                juce::Rectangle<float> peakRect(bounds.getX() + 2.0f, peakTop, segmentWidth, segmentHeight - 1.0f);

                g.setColour(juce::Colours::white);
                g.fillRoundedRectangle(peakRect, 1.0f);
            }
        }
    }

    void setLevel(float newLevel)
    {
        // Smooth level changes with decay
        float targetLevel = juce::jlimit(0.0f, 1.0f, newLevel);

        if (targetLevel > currentLevel)
        {
            currentLevel = targetLevel; // Fast attack
        }
        else
        {
            // Slower decay
            currentLevel = currentLevel * 0.95f + targetLevel * 0.05f;
        }

        // Update peak hold
        if (targetLevel > peakLevel)
        {
            peakLevel = targetLevel;
            peakHoldCounter = 0;
        }
    }

    void reset()
    {
        currentLevel = 0.0f;
        peakLevel = 0.0f;
        peakHoldCounter = 0;
    }

private:
    void timerCallback() override
    {
        // Peak hold decay
        peakHoldCounter++;
        if (peakHoldCounter > 20) // Hold peak for 2 seconds at 30fps
        {
            peakLevel *= 0.95f; // Slow decay
            if (peakLevel < 0.01f)
                peakLevel = 0.0f;
        }

        repaint();
    }

    float currentLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};
/*
  ==============================================================================

    LufsMeterComponent.h
    Created: 17 Mar 2026 10:20:50pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "LookAndFeel.h"

class LufsMeterComponent : public juce::Component
{
public:
    void setValue(float newValue)
    {
        value = newValue;
        repaint();
    }

    void setLabel(const juce::String& text)
    {
        label = text;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // VALUE (top)
        g.setColour(Colors::Button::backgroundToggled);
        g.setFont(Fonts::getFont(18.0f));
        g.drawText(formatLUFS(value),
            bounds.removeFromTop(30),
            juce::Justification::centred);

        // METER
        auto meterArea = bounds.removeFromTop(bounds.getHeight() - 20);

        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(meterArea.toFloat(), 4.0f);

        float norm = juce::jlimit(-60.0f, 0.0f, value);
        float height = juce::jmap(norm, -60.0f, 0.0f, 0.0f, (float)meterArea.getHeight());

        juce::Rectangle<float> bar(
            meterArea.getX(),
            meterArea.getBottom() - height,
            meterArea.getWidth(),
            height
        );

        g.setColour(Colors::Button::backgroundToggled);
        g.fillRoundedRectangle(bar, 4.0f);

        // LABEL (bottom)
        g.setColour(Colors::Group::label);
        g.setFont(Fonts::getFont(14.0f));
        g.drawText(label,
            bounds,
            juce::Justification::centred);
    }

private:
    float value = -100.0f;
    juce::String label = "LUFS";

    juce::String formatLUFS(float v)
    {
        if (v < -99.0f)
            return "-inf";
        return juce::String(v, 1);
    }
};


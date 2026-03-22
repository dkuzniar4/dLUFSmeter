/*
  ==============================================================================

    PeakMeterComponent.h
    Created: 18 Mar 2026 10:24:38pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "LookAndFeel.h"

class PeakMeterComponent : public juce::Component
{
public:
    void setValues(float left, float right, bool clipL, bool clipR)
    {
        peakL = left;
        peakR = right;
        isClipL = clipL;
        isClipR = clipR;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        auto top = bounds.removeFromTop(25);
        auto bottom = bounds.removeFromBottom(20);

        int gap = 6;
        int meterWidth = (bounds.getWidth() - gap) / 2;

        auto leftMeter = bounds.removeFromLeft(meterWidth);
        bounds.removeFromLeft(gap);
        auto rightMeter = bounds;

        // TOP VALUES
        g.setFont(Fonts::getFont(14.0f));
        g.setColour(Colors::Button::backgroundToggled);

        g.drawText(formatDB(peakL), leftMeter.withY(top.getY()).withHeight(top.getHeight()),
            juce::Justification::centred);

        g.drawText(formatDB(peakR), rightMeter.withY(top.getY()).withHeight(top.getHeight()),
            juce::Justification::centred);

        // METERS
        drawMeter(g, leftMeter, peakL, isClipL);
        drawMeter(g, rightMeter, peakR, isClipR);

        // LABELS
        g.setColour(Colors::Group::label);
        g.setFont(Fonts::getFont(13.0f));

        g.drawText("L", leftMeter.withY(bottom.getY()).withHeight(bottom.getHeight()),
            juce::Justification::centred);

        g.drawText("R", rightMeter.withY(bottom.getY()).withHeight(bottom.getHeight()),
            juce::Justification::centred);
    }

private:
    float peakL = 0.0f;
    float peakR = 0.0f;

    bool isClipL = false;
    bool isClipR = false;

    void drawMeter(juce::Graphics& g, juce::Rectangle<int> area, float peak, bool clip)
    {
        auto r = area.toFloat();

        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(r, 4.0f);

        float db = 20.0f * std::log10(peak + 1e-6f);
        db = juce::jlimit(-60.0f, 0.0f, db);

        float height = juce::jmap(db, -60.0f, 0.0f, 0.0f, r.getHeight());

        juce::Rectangle<float> bar(
            r.getX(),
            r.getBottom() - height,
            r.getWidth(),
            height
        );

        g.setColour(clip ? juce::Colours::red
            : Colors::Button::backgroundToggled);

        g.fillRoundedRectangle(bar, 3.0f);
    }

    juce::String formatDB(float peak)
    {
        float db = 20.0f * std::log10(peak + 1e-6f);
        if (db < -60.0f) return "-inf";
        return juce::String(db, 1);
    }
};


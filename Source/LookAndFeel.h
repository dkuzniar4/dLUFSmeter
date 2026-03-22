/*
  ==============================================================================

    LookAndFeel.h
    Created: 17 Mar 2026 9:39:50pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Colors
{
    const juce::Colour background{ 245, 240, 235 };
    const juce::Colour header{ 40, 40, 40 };

    namespace Group
    {
        const juce::Colour label{ 160, 155, 150 };
        const juce::Colour outline{ 235, 230, 225 };
    }

    namespace Button
    {
        const juce::Colour text{ 255, 255, 255 };
        const juce::Colour textToggled{ 0, 0, 0 };
        const juce::Colour background{ 0, 0, 0 };
        const juce::Colour backgroundToggled{ 78, 233, 7 };
        const juce::Colour outline{ 78, 233, 7 };
    }
}

class Fonts
{
public:
    Fonts() = delete;

    static juce::Font getFont(float height = 16.0f);

private:
    static const juce::Typeface::Ptr typeface;
};

class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MainLookAndFeel();

    juce::Font getLabelFont(juce::Label&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLookAndFeel)
};

class ButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ButtonLookAndFeel();

    static ButtonLookAndFeel* get()
    {
        static ButtonLookAndFeel instance;
        return &instance;
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButtonLookAndFeel)
};

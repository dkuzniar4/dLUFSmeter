/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LufsMeterComponent.h"
#include "PeakMeterComponent.h"

//==============================================================================
/**
*/
class DLUFSmeterAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    DLUFSmeterAudioProcessorEditor (DLUFSmeterAudioProcessor&);
    ~DLUFSmeterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    LufsMeterComponent momentaryMeter;
    LufsMeterComponent shortMeter;
    LufsMeterComponent integratedMeter;

    PeakMeterComponent peakMeter;

    juce::TextButton resetButton{ "Reset" };

    void timerCallback() override;

    MainLookAndFeel mainLNF;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DLUFSmeterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DLUFSmeterAudioProcessorEditor)
};

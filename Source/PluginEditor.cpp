/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DLUFSmeterAudioProcessorEditor::DLUFSmeterAudioProcessorEditor (DLUFSmeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(momentaryMeter);
    addAndMakeVisible(shortMeter);
    addAndMakeVisible(integratedMeter);

    addAndMakeVisible(peakMeter);

    momentaryMeter.setLabel("LUFS-M");
    shortMeter.setLabel("LUFS-S");
    integratedMeter.setLabel("LUFS-I");

    // RESET BUTTON
    addAndMakeVisible(resetButton);
    resetButton.setLookAndFeel(ButtonLookAndFeel::get());

    resetButton.onClick = [this]()
        {
            audioProcessor.lufs.reset();
        };

    // LookAndFeel
    setLookAndFeel(&mainLNF);

    startTimerHz(30); // 30 FPS GUI

    setSize(400, 300);
}

DLUFSmeterAudioProcessorEditor::~DLUFSmeterAudioProcessorEditor()
{
}

//==============================================================================
void DLUFSmeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto image = juce::ImageCache::getFromMemory(BinaryData::dkLogo_png, BinaryData::dkLogo_pngSize);

    g.drawImage(image,
        getWidth() - image.getWidth(), getHeight() - image.getHeight(), image.getWidth(), image.getHeight(),
        0, 0, image.getWidth(), image.getHeight());

    image = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);

    int x = (getWidth() - image.getWidth()) / 2;
    int y = 10;

    g.drawImage(image,
        x, y,
        image.getWidth(), image.getHeight(),
        0, 0,
        image.getWidth(), image.getHeight());
}

void DLUFSmeterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    auto topBar = area.removeFromTop(40);

    auto bottomBar = area.removeFromBottom(40);
    resetButton.setBounds(bottomBar.withSizeKeepingCentre(100, 30));

    auto mainArea = area.reduced(10);

    const int numMeters = 4; // Peak + M + S + I
    const int gap = 10;

    const int totalGap = gap * (numMeters - 1);
    const int meterWidth = (mainArea.getWidth() - totalGap) / numMeters;

    const int startX = mainArea.getX();
    const int y = mainArea.getY();
    const int h = mainArea.getHeight();

    for (int i = 0; i < numMeters; ++i)
    {
        int x = startX + i * (meterWidth + gap);

        juce::Rectangle<int> r(x, y, meterWidth, h);

        switch (i)
        {
        case 0: peakMeter.setBounds(r); break;
        case 1: momentaryMeter.setBounds(r); break;
        case 2: shortMeter.setBounds(r); break;
        case 3: integratedMeter.setBounds(r); break;
        }
    }
}

void DLUFSmeterAudioProcessorEditor::timerCallback()
{
    momentaryMeter.setValue(audioProcessor.lufs.getMomentaryLUFS());
    shortMeter.setValue(audioProcessor.lufs.getShortTermLUFS());
    integratedMeter.setValue(audioProcessor.lufs.getIntegratedLUFS());

    peakMeter.setValues(audioProcessor.peakL,
        audioProcessor.peakR,
        audioProcessor.clipL,
        audioProcessor.clipR);
}

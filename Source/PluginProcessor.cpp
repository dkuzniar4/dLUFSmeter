/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DLUFSmeterAudioProcessor::DLUFSmeterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DLUFSmeterAudioProcessor::~DLUFSmeterAudioProcessor()
{
}

//==============================================================================
const juce::String DLUFSmeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DLUFSmeterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DLUFSmeterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DLUFSmeterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DLUFSmeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DLUFSmeterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DLUFSmeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DLUFSmeterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DLUFSmeterAudioProcessor::getProgramName (int index)
{
    return {};
}

void DLUFSmeterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DLUFSmeterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lufs.prepare(sampleRate, getTotalNumInputChannels());
}

void DLUFSmeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DLUFSmeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DLUFSmeterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool isPlaying = false;
    double currentPos = lastPosition;

    // =========================
    // TRANSPORT DETECTION
    // =========================
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo pos;

        if (playHead->getCurrentPosition(pos))
        {
            isPlaying = pos.isPlaying;
            currentPos = pos.timeInSeconds;

            bool started = isPlaying && !wasPlaying;
            bool stopped = !isPlaying && wasPlaying;
            bool seeked = isPlaying && std::abs(currentPos - lastPosition) > 0.5;

            // DEBUG (na razie do testów)
            if (started)
            {
                lufs.reset();
                lufs.setIntegratedFreeze(false);
                DBG("START");
            }     

            if (stopped)
            {
                lufs.setIntegratedFreeze(true);
                DBG("STOP");
            }              

            if (seeked)
            {
                lufs.reset();
                lufs.setIntegratedFreeze(false);
                DBG("SEEK");
            }
                

            wasPlaying = isPlaying;
            lastPosition = currentPos;
        }
    }

    lufs.process(buffer.getArrayOfReadPointers(), buffer.getNumSamples());

    auto numSamples = buffer.getNumSamples();

    float localPeakL = 0.0f;
    float localPeakR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = std::abs(buffer.getSample(0, i));
        localPeakL = std::max(localPeakL, l);

        if (buffer.getNumChannels() > 1)
        {
            float r = std::abs(buffer.getSample(1, i));
            localPeakR = std::max(localPeakR, r);
        }
    }

    // smooth
    peakL = 0.8f * peakL + 0.2f * localPeakL;
    peakR = 0.8f * peakR + 0.2f * localPeakR;

    // clip detect
    clipL = localPeakL > 1.0f;
    clipR = localPeakR > 1.0f;
}

//==============================================================================
bool DLUFSmeterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DLUFSmeterAudioProcessor::createEditor()
{
    return new DLUFSmeterAudioProcessorEditor (*this);
}

//==============================================================================
void DLUFSmeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DLUFSmeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DLUFSmeterAudioProcessor();
}

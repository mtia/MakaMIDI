/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MidiProcessor.h"

int MidiEffectAudioProcessor::commasToPitchBend(String commas) {
    if (commas == "NaN")
        return std::numeric_limits<int>::quiet_NaN();

    int pitchWheelValue = commas.getIntValue() * 16384 / 9;
    if (pitchWheelValue >= 0 || pitchWheelValue < 16384)
       return commas.getIntValue();

    DBG("INVALID PITCH DATA: " << commas);
}

//==============================================================================
MidiEffectAudioProcessor::MidiEffectAudioProcessor()
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

MidiEffectAudioProcessor::~MidiEffectAudioProcessor()
{
}

//==============================================================================
const juce::String MidiEffectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidiEffectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidiEffectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidiEffectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidiEffectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiEffectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidiEffectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MidiEffectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MidiEffectAudioProcessor::getProgramName (int index)
{
    return {};
}

void MidiEffectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//============================================================================== 

void MidiEffectAudioProcessor::readScale(const juce::File& fileToRead)
{
    if (!fileToRead.existsAsFile()) {
        DBG("File not found");
        return;
    }

    juce::FileInputStream inputStream(fileToRead); 

    if (!inputStream.openedOk())
    {
        DBG("\nFailed to open file\n");
        return;
    }

    if (alterations.size() > 0)
        alterations.clear();

    for (int i = 0; i < 128; i++)
    {
        alterations.add(0);
    }

    while (!inputStream.isExhausted())
    {
        auto line = inputStream.readNextLine();

        int noteNumber = line.upToFirstOccurrenceOf(",", false, true).getIntValue();
        DBG(noteNumber);

        String altStr = line.fromFirstOccurrenceOf(",", false, true).upToFirstOccurrenceOf(",", false, true);
        alterations.set(noteNumber, commasToPitchBend(altStr));
    }

    /*for (int i = 0; i < 128; i++)
    {
        DBG("Note number: " << i << " \talteration " << alterations[i]);
    }*/

}

void MidiEffectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::File defaultFile("D:/Documents/JUCE\ Projex/MidiEffect/Saba.csv");
    readScale(defaultFile);
}

void MidiEffectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiEffectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MidiEffectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear(); // silence any possible disturbance
    midiProcessor.process(midiMessages, &pitchWheelValue, &pitchCorrection, alterations, &activeNoteNumber);
}

//==============================================================================
bool MidiEffectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiEffectAudioProcessor::createEditor()
{
    return new MidiEffectAudioProcessorEditor (*this);
}

//==============================================================================
void MidiEffectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MidiEffectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiEffectAudioProcessor();
}
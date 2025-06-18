/*
  ==============================================================================

    MakaMIDI
    Copyright (c) 2025 Mattia Vassena
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MidiProcessor.h"
#include "LowBox.h"

/* 
    @brief
    returns integer alteration [-9, 9] from the string value, if valid
*/
int MidiEffectAudioProcessor::parseCommas(String commas) {
    // if the field contains N, the alteration is NaN, thus the note is interpreted as not in the scale
    if (commas.indexOf("N")>=0)
        return std::numeric_limits<int>::max();

    int numCommas = commas.getIntValue();
    if (numCommas > -10 && numCommas < 10)
        return commas.getIntValue();
    else
    {
        DBG("INVALID PITCH DATA: " << commas);
        throw(std::range_error("Cannot convert commas to a valid pitch bend value"));
        return 0;
    }
}

void MidiEffectAudioProcessor::printAlterations()
{
    for (int i = 70; i < 100; i++)
        DBG("note " << i << ": " << alterations[i]);
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
    // Find VST's directory
    root = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory().getParentDirectory().getParentDirectory();
    alterations.insertMultiple(0, std::numeric_limits<int>::max(), 128);
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
        alterations.set(i, std::numeric_limits<int>::max());
        //DBG("DBG nan: " << alterations[i]);
    }

    while (!inputStream.isExhausted())
    {
        auto line = inputStream.readNextLine();

        int noteNumber = line.upToFirstOccurrenceOf(",", false, true).getIntValue();

        String altStr = line.fromFirstOccurrenceOf(",", false, true).upToFirstOccurrenceOf(",", false, true);
        
        DBG(altStr + " -> " + String(parseCommas(altStr)));
        alterations.set(noteNumber, parseCommas(altStr));
    }

}

void MidiEffectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    midiProcessor.exclusive = &exclusive;
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
    pitchCorrection = midiProcessor.process(midiMessages, &pitchWheelValue, &pitchCorrection, alterations, &activeNoteNumber);
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
void MidiEffectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);

    for (int i = 0; i < 128; ++i)
        stream.writeInt(alterations[i]);
}


void MidiEffectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);

    for (int i = 0; i < 128; ++i)
        alterations.set(i, stream.readInt());

}



juce::StringArray listAlterationsInCommas()
{
    StringArray output;
    for (int i = -9; i < 10; i++)
    {
        String* s = new String();
        if (i > 0)
            s->append("+", 1);
        s->append(String(i), 2);
        output.add(*s);
    }
    return output;
}

juce::StringArray listMIDINotes()
{
    std::vector<String> noteNames = {
        "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"
    };

    StringArray output;
    for (int i = 0; i < 115; i++) {
        output.add(noteNames[i % 12] + String(std::floor((i + 12) / 12)));
    }

    return output;
}

AudioProcessorValueTreeState::ParameterLayout MidiEffectAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 1; i <= 16; ++i)
    {
        String ts = String("Toggle ");
        ts.append(String(i), 2);
        String ns = String("Note ");
        ns.append(String(i), 2);
        String as = String("Alteration ");
        as.append(String(i), 2);
        layout.add(std::make_unique<AudioParameterBool>(ts, ts, false));
        layout.add(std::make_unique<AudioParameterChoice>(ns, ns, listMIDINotes(), 0));
        layout.add(std::make_unique<AudioParameterChoice>(as, as, listAlterationsInCommas(), 0));
    }

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiEffectAudioProcessor();
}

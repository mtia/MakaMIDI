/*
  ==============================================================================

    MakaMIDI
    Copyright (c) 2025 Mattia Vassena
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Changed from Projucer to CMake build system
//#include <juce_audio_processors/juce_audio_processors.h>
#include "MidiProcessor.h"


//==============================================================================

/**
*/
class MidiEffectAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MidiEffectAudioProcessor();
    ~MidiEffectAudioProcessor() override;

    //==============================================================================
    void readScale(const juce::File& fileToRead);
    void printAlterations();
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    int parseCommas(String commas);
    
    juce::File root, savedFile;
    int pitchWheelValue = 8192;
    int pitchCorrection = 0;
    int activeNoteNumber = -1;
    bool exclusive = false;
    bool readingScale = false;
    juce::Array<int> alterations;

private:
    MidiProcessor midiProcessor;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEffectAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

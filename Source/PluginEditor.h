/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MidiEffectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MidiEffectAudioProcessorEditor (MidiEffectAudioProcessor&);
    ~MidiEffectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MidiEffectAudioProcessor& audioProcessor;

    juce::TextButton loadBtn;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEffectAudioProcessorEditor)
};

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

// #include <JuceHeader.h>
// Changed from Projucer to CMake build system
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "LowBox.h"
#include "BinaryData.h"

//==============================================================================
struct Placeholder : Component
{
    Placeholder(); 

    void paint(juce::Graphics& g) override
    {
        g.fillAll(customColor);
    }
    juce::Colour customColor;
};

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
    void updateBoxes(MidiEffectAudioProcessor* p);
    void updateAlterations();
    void updateAlteration(int i);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MidiEffectAudioProcessor& audioProcessor;

    // GUI Components
    juce::TextButton loadBtn, exModeBtn;

    std::unique_ptr<juce::FileChooser> fileChooser;

    // labels on left side indicating rows of notes and alterations (2x2=4 rows)
    juce::Label noteLabel1, alterationLabel1, noteLabel2, alterationLabel2;

    // Upper Box contains image and two buttons, the two rows below contain the note controls
    juce::Component upperBox, firstControlRow, secondControlRow;

    // 16 control box (note and relative alteration)
    std::unique_ptr<LowBox> lowControls[16];

    // attachments for the 16 controls
    juce::AudioProcessorValueTreeState::ButtonAttachment* buttonAttachments[16];

    Image bgImg;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEffectAudioProcessorEditor)
};

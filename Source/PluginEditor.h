/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
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

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MidiEffectAudioProcessor& audioProcessor;

    // GUI Components
    juce::TextButton loadBtn, exModeBtn;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::Label noteLabel1, alterationLabel1, noteLabel2, alterationLabel2;

    juce::Component upperBox, firstButtonRow, secondButtonRow;

    std::unique_ptr<LowBox> lowButtons[16];

    juce::AudioProcessorValueTreeState::ButtonAttachment* buttonAttachments[16];

    // BG Image
    Image bgImg;

    //
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEffectAudioProcessorEditor)
};

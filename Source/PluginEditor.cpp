/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidiEffectAudioProcessorEditor::MidiEffectAudioProcessorEditor (MidiEffectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(loadBtn);
    loadBtn.setButtonText("Load scale");
    loadBtn.onClick = [this]{
        fileChooser = std::make_unique<juce::FileChooser>("Choose a file",
            audioProcessor.root,
            "*");

        const auto fileChooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectDirectories;

        fileChooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser) {
            juce::File chosenFile(chooser.getResult());
            if (chosenFile.getFileExtension() == ".csv") {
                audioProcessor.savedFile = chosenFile;
                audioProcessor.root = chosenFile.getParentDirectory().getFullPathName();
                audioProcessor.readScale(chosenFile);
            }
        });
    };
    setSize (600, 400);
}

MidiEffectAudioProcessorEditor::~MidiEffectAudioProcessorEditor()
{
}

//==============================================================================
void MidiEffectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void MidiEffectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const auto btnX = getWidth() * JUCE_LIVE_CONSTANT(0.25);
    const auto btnY = getHeight() * JUCE_LIVE_CONSTANT(0.5);
    const auto btnWidth = getWidth() * JUCE_LIVE_CONSTANT(0.1);
    const auto btnHeight = btnWidth * 0.5;

    loadBtn.setBounds(btnX, btnY, btnWidth, btnHeight);
}

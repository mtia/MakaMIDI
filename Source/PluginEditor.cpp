/*
  ==============================================================================

    MakaMIDI
    Copyright (c) 2025 Mattia Vassena
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    PluginEditor.cpp

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
MidiEffectAudioProcessorEditor::MidiEffectAudioProcessorEditor (MidiEffectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // setup "Load" button
    loadBtn.setButtonText("Load scale");

    loadBtn.onClick = [this](){

        audioProcessor.readingScale = true;

        fileChooser = std::make_unique<juce::FileChooser>("Choose a file",
            audioProcessor.root,
            "*");

        const auto fileChooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectDirectories;

        fileChooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser) {
            juce::File chosenFile(chooser.getResult());
            if (chosenFile.getFileExtension().toLowerCase() == ".csv") {
                audioProcessor.savedFile = chosenFile;
                audioProcessor.root = chosenFile.getParentDirectory().getFullPathName();
                audioProcessor.readScale(chosenFile);
                
                updateBoxes(&audioProcessor);

                for(int i=0; i<16; i++){
                    updateAlteration(i, true);
                }

                juce::Timer::callAfterDelay(100, [this] {
                    audioProcessor.readingScale = false;
                });

            }
            else{
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                 "Invalid file",
                                 "Please select a CSV file.");
            }
        });

    };

    // setup "Exclusive mode" button
    exModeBtn.setButtonText("Exclusive");
    exModeBtn.setToggleable(true);
    exModeBtn.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    exModeBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
    exModeBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    exModeBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkred);
    exModeBtn.setToggleState(false, false); // default off

    loadBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    loadBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::darkgoldenrod);

    exModeBtn.onClick = [this] {
        audioProcessor.exclusive = !audioProcessor.exclusive;
        exModeBtn.setToggleState(audioProcessor.exclusive, false);
        String s = audioProcessor.exclusive ? "ON" : "OFF";
        DBG("Exclusive mode: " << s);
    };


    addAndMakeVisible(firstControlRow);
    addAndMakeVisible(noteLabel1);
    addAndMakeVisible(alterationLabel1);
    addAndMakeVisible(noteLabel2);
    addAndMakeVisible(alterationLabel2);
    noteLabel1.setText("Note: ", juce::NotificationType::dontSendNotification);
    alterationLabel1.setText("Alteration (commas): ", juce::NotificationType::dontSendNotification);
    noteLabel2.setText("Note: ", juce::NotificationType::dontSendNotification);
    alterationLabel2.setText("Alteration (commas): ", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(upperBox);
    addAndMakeVisible(loadBtn);
    addAndMakeVisible(exModeBtn);
    
    for (int i = 0; i < 16; i++)
    {
        lowControls[i] = std::make_unique<LowBox>(audioProcessor.apvts, i + 1);
        lowControls[i]->note->onChange = [this, i] {
            updateAlteration(i, false);
        };
        lowControls[i]->alteration->onChange = [this, i] {
            updateAlteration(i, false);
        };
        lowControls[i]->toggle->onStateChange = [this, i] {
            updateAlteration(i, false);
        };

        addAndMakeVisible(*lowControls[i]);
    }
    
    bgImg = ImageFileFormat::loadFrom(BinaryData::Oud_png, BinaryData::Oud_pngSize);
    // bgImg = ImageCache::getFromFile(File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getChildFile("Oud.png"));
    DBG(File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getFullPathName());
    DBG(File::getCurrentWorkingDirectory().getFullPathName());
    setSize (800, 300);

    // for persistence of the GUI when the plugin window gets closed
    updateBoxes(&audioProcessor);
}

MidiEffectAudioProcessorEditor::~MidiEffectAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MidiEffectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // make whole background black
    g.fillAll(juce::Colours::black);
    // draw image in upper box
    g.drawImage(bgImg, upperBox.getLocalBounds().toFloat());
}

void MidiEffectAudioProcessorEditor::resized()
{
    int N = 16; // number of components in the lower area
    auto bounds = getLocalBounds();
    
    upperBox.setBounds(bounds.removeFromTop(100));
    firstControlRow.setBounds(bounds.removeFromTop(100));
    secondControlRow.setBounds(bounds);

    auto firstControlRowBounds = firstControlRow.getBounds();
    auto secondControlRowBounds = secondControlRow.getBounds();

    auto boxWidth = firstControlRow.getWidth()/(N/2+1);
    auto boxHeight = firstControlRow.getHeight() / 2.5;

    auto leftPanel1 = firstControlRowBounds.removeFromLeft(boxWidth);
    auto leftPanel2 = secondControlRowBounds.removeFromLeft(boxWidth);
    
    noteLabel1.setBounds(leftPanel1.removeFromTop(boxHeight));
    alterationLabel1.setBounds(leftPanel1.removeFromTop(boxHeight));
    noteLabel2.setBounds(leftPanel2.removeFromTop(boxHeight));
    alterationLabel2.setBounds(leftPanel2.removeFromTop(boxHeight));

    const auto btnX = getWidth() * (0.035);
    const auto btnY = getHeight() * (0.09);
    const auto btnWidth = getWidth() * (0.12);
    const auto btnHeight = btnWidth * 0.5;

    loadBtn.setBounds(btnX, btnY, btnWidth, btnHeight);
    exModeBtn.setBounds(getWidth()*(1-0.035) - btnWidth, btnY, btnWidth, btnHeight);

    for (int i = 0; i < N/2; i++) {
        lowControls[i]->setBounds(firstControlRowBounds.removeFromLeft(boxWidth));
    }

    for (int i = N/2; i < N; i++) {
        lowControls[i]->setBounds(secondControlRowBounds.removeFromLeft(boxWidth));
    }

}

// once a scale file has been read, this function updates the ComboBoxes on the GUI
void MidiEffectAudioProcessorEditor::updateBoxes(MidiEffectAudioProcessor* p)
{
    // boxnum counts how many notes the scale is using, a maximum of 16 is shown
    int boxnum = 0;

    // for each noteNumber
    for (int i = 0; i < 128; i++)
    {
        // if there is a record in the alterations
        if (p->alterations[i]!=std::numeric_limits<int>::max()) {
            DBG("Loading alteration[" << String(i) << "]: " + String(p->alterations[i]));
            //DBG("Alterations[" << String(i) << "]: " << String(p->alterations[i]));
            // fill a ComboBox with the corresponding couple note+alteration
            lowControls[boxnum]->setAlteration(i, p->alterations[i]);
            int temp = lowControls[boxnum]->alteration->getSelectedId()-11;
            DBG("Alteration set for note " << String(i) << ": " << String(temp));
            boxnum++;
        }

        // if more than 15 alterations are loaded, don't show the remaining ones
        if (boxnum > 15)
            break;
    }

    // reset unused boxes
    for (int i = boxnum; i <= 15; i++)
        lowControls[i]->reset();
}

/**
 * @brief Updates the alteration for the note controlled by LowBox at index i.
 *
 * Called when the user changes the toggle, note, or alteration controls.
 * Updates the corresponding entry in the processor's `alterations` array
 * if the controls are in a valid state.
 *
 * @param i Index of the LowBox (0-based).
 */
void MidiEffectAudioProcessorEditor::updateAlteration(int i, bool onLoad)
{
    if(!onLoad && audioProcessor.readingScale)
    {
        // If called on load, skip the update
        DBG("updateAlteration - Skipped during loading");
        return;
    }

    // Reset temporaneo della nota target
    int j = lowControls[i]->note->getSelectedId() + 10;

    if (lowControls[i]->toggle->getToggleState() &&
        lowControls[i]->note->getSelectedId() > 1 &&
        lowControls[i]->alteration->getSelectedId() > 1)
    {
        int alt = lowControls[i]->alteration->getSelectedId() - 11;
        audioProcessor.alterations.set(j, alt);
        DBG("updateAlteration - MIDInote: " << j << " alt: " << alt);
        DBG("\t\t\t\t: " << lowControls[i]->note->getText() << " alt: " << audioProcessor.alterations[j]);
    }
    else
    {
        // Disattiva se la combo o toggle è inattiva
        audioProcessor.alterations.set(j, std::numeric_limits<int>::max());
        DBG("updateAlteration - MIDInote: " << j << " reset");
    }
}

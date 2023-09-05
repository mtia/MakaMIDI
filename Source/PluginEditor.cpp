/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//#include "LowBox.h"

/*Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}*/


//==============================================================================
MidiEffectAudioProcessorEditor::MidiEffectAudioProcessorEditor (MidiEffectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    loadBtn.setButtonText("Load scale");
    loadBtn.onClick = [this](){
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
                
                updateBoxes(&audioProcessor);

            }
        });

    };

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
    };

    addAndMakeVisible(lowerBox);
    addAndMakeVisible(noteLabel);
    addAndMakeVisible(alterationLabel);
    noteLabel.setText("Note: ", juce::NotificationType::dontSendNotification);
    alterationLabel.setText("Alteration (commas): ", juce::NotificationType::dontSendNotification);


    addAndMakeVisible(upperBox);
    addAndMakeVisible(loadBtn);
    addAndMakeVisible(exModeBtn);
    
    for (int i = 0; i < 10; i++)
    {
        lowButtons[i] = (i, std::make_unique<LowBox>(audioProcessor.apvts, i + 1));
        lowButtons[i]->note->onChange = [this] { updateAlterations(); };
        lowButtons[i]->alteration->onChange = [this] { updateAlterations(); };
        lowButtons[i]->toggle->onStateChange = [this] { updateAlterations(); };
        addAndMakeVisible(*lowButtons[i]);
    }

    bgImg = ImageCache::getFromFile(File("D:/Documents/JUCE\ Projex/MidiEffect/Oud.png"));
    setSize (900, 200);
}

MidiEffectAudioProcessorEditor::~MidiEffectAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MidiEffectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.drawImage(bgImg, upperBox.getLocalBounds().toFloat());
}

void MidiEffectAudioProcessorEditor::resized()
{
    int N = 10; // number of components in the lower area
    auto bounds = getLocalBounds();
    
    upperBox.setBounds(bounds.removeFromTop(100));
    lowerBox.setBounds(bounds);
    auto lowBounds = lowerBox.getBounds();
    auto boxWidth = lowerBox.getWidth()/(N+1);
    auto boxHeight = lowerBox.getHeight() / 2.5;
    auto leftPanel = lowBounds.removeFromLeft(boxWidth);
    noteLabel.setBounds(leftPanel.removeFromTop(boxHeight));
    alterationLabel.setBounds(leftPanel.removeFromTop(boxHeight));
    
    const auto btnX = getWidth() * (0.035);
    const auto btnY = getHeight() * (0.09);
    const auto btnWidth = getWidth() * (0.12);
    const auto btnHeight = btnWidth * 0.5;

    loadBtn.setBounds(btnX, btnY, btnWidth, btnHeight);
    exModeBtn.setBounds(getWidth()*(1-0.035) - btnWidth, btnY, btnWidth, btnHeight);

    for (int i = 0; i < N; i++) {
        lowButtons[i]->setBounds(lowBounds.removeFromLeft(boxWidth));
    }

}

// once a scale file has been read, this function updates the ComboBoxes on the GUI
void MidiEffectAudioProcessorEditor::updateBoxes(MidiEffectAudioProcessor* p)
{
    // boxnum counts how many notes the scale is using, a maximum of 10 is shown
    int boxnum = 0;

    // for each noteNumber
    for (int i = 0; i < 128; i++)
    {
        // if there is a record in the alterations
        if (p->alterations[i]!=std::numeric_limits<int>::max()) {
            //DBG("Alterations[" << String(i) << "]: " << String(p->alterations[i]));
            // fill a ComboBox with the corresponding couple note+alteration
            //lowButtons[boxnum]->setAlteration(LowBox::noteNumberToName(i), p->alterations[i]);
            lowButtons[boxnum]->setAlteration(i, p->alterations[i]);
            boxnum++;
        }

        // if more than 9 alterations are loaded, don't show the remaining ones
        if (boxnum > 9)
            break;
    }

    // reset unused boxes
    for (int i = boxnum; i <= 9; i++)
        lowButtons[i]->reset();
}

void MidiEffectAudioProcessorEditor::updateAlterations()
{

    for (int i = 0; i < 128; i++)
    {
        audioProcessor.alterations.set(i, std::numeric_limits<int>::max());
    }

    // for each control box
    for (int i = 0; i < 10; i++)
    {
        // if the toggle is active
        if (lowButtons[i]->toggle->getToggleState())
        {
            // if the combobox are not in default position
            if (lowButtons[i]->note->getSelectedId() && lowButtons[i]->alteration->getSelectedId())
            {
                // C0 = MIDI note #12, alterations starts from 0
                int j = lowButtons[i]->note->getSelectedId()+11;
                int alt = lowButtons[i]->alteration->getSelectedId() - 10;
                audioProcessor.alterations.set(j, alt);
                // ComboBox starts from C1 (#24)
                DBG("updateAlterations - MIDInote: " << j << " alt: " << alt);
            }
        }
    }
}
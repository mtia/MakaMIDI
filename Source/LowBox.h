/*
  ==============================================================================

    LowBox.h
    Created: 21 Aug 2023 4:51:04pm
    Author:  Mattia

  ==============================================================================
*/

#pragma once
// #include "JuceHeader.h"
// Changed from Projucer to CMake build system
#include <juce_audio_processors/juce_audio_processors.h>

class LowBox : public juce::Component
{
public:

    std::unique_ptr<juce::ComboBox> note;
    std::unique_ptr<juce::ToggleButton> toggle;
    std::unique_ptr<juce::ComboBox> alteration;

    LowBox(juce::AudioProcessorValueTreeState &apvts, int i)
    {

        note = std::make_unique<ComboBox>();
        alteration = std::make_unique<ComboBox>();
        toggle = std::make_unique<ToggleButton>();

        /*DBG(midiNotes.joinIntoString(","));
        DBG(alterationsInCommas.joinIntoString(","));*/

        note->setEnabled(false);
        alteration->setEnabled(false);
        note->addItemList(midiNotes, 1);
        alteration->addItemList(alterationsInCommas,1);
        note->setSelectedId(0, juce::NotificationType::dontSendNotification);
        alteration->setSelectedId(0, juce::NotificationType::dontSendNotification);
        note->setTextWhenNothingSelected("");
        alteration->setTextWhenNothingSelected("");

        toggle->setClickingTogglesState(true);
        toggle->onClick = [this] () { toggleClicked(); };
        toggle->setToggleState(false, juce::NotificationType::dontSendNotification);

        String ts = String("Toggle ");
        ts.append(String(i), 2);
        String ns = String("Note ");
        ns.append(String(i), 2);
        String as = String("Alteration ");
        as.append(String(i), 2);

        toggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ts, *toggle);
        noteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, ns, *note);
        alterationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, as, *alteration);
        
        addAndMakeVisible(*note);
        addAndMakeVisible(*alteration);
        addAndMakeVisible(*toggle);
    };

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto h = bounds.getHeight() / 5;
        auto rx = 2;// JUCE_LIVE_CONSTANT(3);
        auto ry = 9;// JUCE_LIVE_CONSTANT(10);
        note->setBounds(bounds.removeFromTop(h*2).reduced(rx, ry));
        alteration->setBounds(bounds.removeFromTop(h*2).reduced(rx, ry));
        toggle->setBounds(bounds);
    };

    void toggleClicked()
    {
        if (toggle->getToggleState())
        {
            note->setEnabled(true);
            alteration->setEnabled(true);
        }
        else
        {
            note->setEnabled(false);
            alteration->setEnabled(false);
        }
    };

    // change the content of the box
    void setAlteration(int noteNum, int newAlteration)
    {
        note->setSelectedId(noteNum-10);

        String * stringAlt = new String();
        if (newAlteration > 0)
            stringAlt->append("+", 1);
        stringAlt->append(String(newAlteration), 2);
        alteration->setText(*stringAlt);
        toggle->setToggleState(true, juce::NotificationType::dontSendNotification);
        note->setEnabled(true);
        alteration->setEnabled(true);
    }

    static std::vector<String> getNoteNames()
    {
        return {
            "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"
        };
    }

    static String noteNumberToName(int number)
    {
        std::vector<String> noteNames = {
            "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"
        };
        return noteNames[number % 12] + String(std::floor((number-12)/ 12));
    }

    void reset()
    {
        note->setEnabled(false);
        note->setSelectedId(0, juce::NotificationType::dontSendNotification);
        alteration->setEnabled(false);
        alteration->setSelectedId(0, juce::NotificationType::dontSendNotification);
        toggle->setToggleState(false, juce::NotificationType::dontSendNotification);
    }


private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> noteAttachment;
    std::unique_ptr < juce::AudioProcessorValueTreeState::ComboBoxAttachment> alterationAttachment; 
    std::unique_ptr < juce::AudioProcessorValueTreeState::ButtonAttachment> toggleAttachment;

    int index;

    const juce::StringArray midiNotes = listMIDINotes();
    const juce::StringArray alterationsInCommas = listAlterationsInCommas();

    juce::StringArray listAlterationsInCommas()
    {
        StringArray output;
        output.add("");
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
        output.add("");
        for (int i = 0; i < 116; i++) {
            output.add(noteNames[i % 12] + String(std::floor(i / 12)));
        }

        return output;
    }

};
/*
  ==============================================================================

    LowBox.h
    Created: 21 Aug 2023 4:51:04pm
    Author:  Mattia

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

struct LowBox : juce::Component
{
    LowBox()
    {
        note = new ComboBox();
        alteration = new ComboBox();
        toggle = new ToggleButton();

        /*DBG(midiNotes.joinIntoString(","));
        DBG(alterationsInCommas.joinIntoString(","));*/

        note->setEnabled(false);
        alteration->setEnabled(false);

        note->addItemList(midiNotes,1);
        alteration->addItemList(alterationsInCommas,1);
        toggle->setClickingTogglesState(true);
        toggle->setToggleState(false, juce::NotificationType::dontSendNotification);
        toggle->onClick = [this] { toggleClicked(); };
        
        addAndMakeVisible(note);
        addAndMakeVisible(alteration);
        addAndMakeVisible(toggle);
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


private:
    juce::ComboBox* note;
    juce::ComboBox* alteration;
    juce::ToggleButton* toggle;

    const juce::StringArray midiNotes = listMIDINotes();
    const juce::StringArray alterationsInCommas = listAlterationsInCommas();

    juce::StringArray listMIDINotes()
    {
        StringArray names = {
            "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"
        };

        StringArray output;
        for (int i = 0; i < 115; i++) {
            output.add(names[i % 12] + String(std::floor((i + 12) / 12)));
        }

        return output;
    }

    juce::StringArray listAlterationsInCommas()
    {
        StringArray output;
        for (int i = -9; i < 10; i++)
        {
            String * s = new String();
            if (i > 0)
                s->append("+",1);
            s->append(String(i),2);
            output.add(*s);
        }
        return output;
    }
};
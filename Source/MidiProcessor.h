/*
  ==============================================================================

    MakaMIDI
    Copyright (c) 2025 Mattia Vassena
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.

    MidiProcessor.h
    Created: 2 Aug 2023 12:16:46pm
    Author:  Mattia

  ==============================================================================
*/

#pragma once

// #include "JuceHeader.h"
// Changed from Projucer to CMake build system
#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;

class MidiProcessor
{
public:
    int process(MidiBuffer& midiMessages, int *pitchWheelValue, int *pitchCorrection, const juce::Array<int> &alterations, int *activeNoteNumber)
    {
        processedBuffer.clear();
        processMidiInput(midiMessages, pitchWheelValue, pitchCorrection, alterations, activeNoteNumber);
        midiMessages.swapWith(processedBuffer);
        return *pitchCorrection;
    }

    int getPitchCorrection(int noteNumber, const juce::Array<int> &alterations)
    {
        if (alterations[noteNumber]==std::numeric_limits<int>::max())
            return 0;
        return juce::roundToInt(alterations[noteNumber]*8192/9);
    }

    bool isValidPitchValue(int pitchWheelValue)
    {
        if (pitchWheelValue < 0 || pitchWheelValue >= 16384) {
            DBG("INVALID PITCH" << pitchWheelValue);
            return false;
        }
        return true;
    }

    int clipPitch(int pitchValue)
    {
        return juce::jmin(16383, juce::jmax(0, pitchValue));
    }

    void suppressNote(int channel, int suppressingNoteNumber, int samplePos, int *pitchCorrection, int *pitchWheelValue, const juce::Array<int>& alterations)
    {
        *pitchCorrection = getPitchCorrection(suppressingNoteNumber, alterations);

        // if the suppressing note was altered
        if (*pitchCorrection != 0) {
            // restore pitchWheel value removing the note alteration contribution
            MidiMessage pitchMessage = MidiMessage::pitchWheel(channel, *pitchWheelValue);
            processedBuffer.addEvent(pitchMessage, samplePos);
            DBG("NOTE OFF: Pitch wheel " << *pitchWheelValue << " +  correction " << *pitchCorrection);
        }

        // reset pitch correction for future notes
        *pitchCorrection = 0;
    }

    void processMidiInput(const MidiBuffer& midiMessages, int *pitchWheelValue, int *pitchCorrection, const juce::Array<int> &alterations, int *activeNoteNumber)
    {
        MidiBuffer::Iterator it(midiMessages);
        MidiMessage currentMessage;
        int samplePos;

        while (it.getNextEvent(currentMessage, samplePos))
        {
            
            // DBG("MSG # " << samplePos);

            // safety check
            if (!isValidPitchValue(*pitchWheelValue))
            {
                DBG("INVALID PITCH VALUE");
                break;
            }

            int currentChannel = currentMessage.getChannel();

            // PitchWheel message
            if (currentMessage.isPitchWheel())
            {
                // store user's pitch alteration
                *pitchWheelValue = currentMessage.getPitchWheelValue();
                currentMessage = MidiMessage::pitchWheel(currentChannel, clipPitch(*pitchWheelValue + *pitchCorrection));
                
                // forward modified pitchwheel message
                processedBuffer.addEvent(currentMessage, samplePos);
            }

            // Keypress Message
            else if (currentMessage.isNoteOn())
            {
                // stops all playing notes (MONOPHONIC FUNCTION)
                if (*activeNoteNumber != -1)
                {
                    // reset pitch value for the suppressing note
                    suppressNote(currentChannel, *activeNoteNumber, samplePos, pitchCorrection, pitchWheelValue, alterations);
                    // generate NoteOff message to suppress note
                    MidiMessage cleanMessage = MidiMessage::noteOff(currentChannel, *activeNoteNumber, 0.0f);
                    processedBuffer.addEvent(cleanMessage, samplePos);
                }

                // get alteration for the current note
                int noteNumber = currentMessage.getNoteNumber();

                // do nothing if playing an excluded note in exclusive mode (+inf means excluded note)
                if (alterations[noteNumber] != std::numeric_limits<int>::max() || !*exclusive)
                {
                    *pitchCorrection = getPitchCorrection(noteNumber, alterations);

                    if (*pitchCorrection != 0) {

                        // create a pitch message summing the wheel alteration and the note alteration
                        MidiMessage pitchMessage = MidiMessage::pitchWheel(currentChannel, clipPitch(*pitchWheelValue + *pitchCorrection));
                        DBG("NOTE ON: Pitch wheel " << *pitchWheelValue << " +  correction " << *pitchCorrection);

                        processedBuffer.addEvent(pitchMessage, samplePos);
                    }
                    *activeNoteNumber = currentMessage.getNoteNumber();
                    // forward noteOn
                    processedBuffer.addEvent(currentMessage, samplePos);
                }
                else
                {
                    if(*exclusive)
                    {
                        DBG("Skipped note: " << noteNumber << " exclusive mode ON");
                    }
                    else
                    {
                        DBG("Skipped note: " << noteNumber << " exclusive mode OFF");
                        DBG("With Alteration: " << alterations[noteNumber]);
                    }
                }
            }

            //  key release (note not suppressed by the monophonic function)
            else if (currentMessage.isNoteOff() && currentMessage.getNoteNumber() == *activeNoteNumber)
            {
                // suppress corresponding note
                suppressNote(currentChannel, currentMessage.getNoteNumber(), samplePos, pitchCorrection, pitchWheelValue, alterations);
                // no notes are now active
                *activeNoteNumber = -1;

                // forward noteOff
                processedBuffer.addEvent(currentMessage, samplePos);
            }
        }
    }

    MidiBuffer processedBuffer;
    bool* exclusive;
};
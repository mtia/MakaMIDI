/*
  ==============================================================================

    MidiProcessor.h
    Created: 2 Aug 2023 12:16:46pm
    Author:  Mattia

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

// const int interval = 3;

using namespace juce;

class MidiProcessor
{
public:
    void process(MidiBuffer& midiMessages)
    {
        processedBuffer.clear();
        processMidiInput(midiMessages);
        midiMessages.swapWith(processedBuffer);
    }

    int getPitchCorrection(int noteNumber)
    {
        if (noteNumber == 72) {
            return 2500;
        }
        return 0;
    }

    void processMidiInput(const MidiBuffer& midiMessages)
    {
        MidiBuffer::Iterator it(midiMessages);
        MidiMessage currentMessage;
        int samplePos;

        // Value of the user's pitch wheel
        int pitchWheelValue = 0;

        while (it.getNextEvent(currentMessage, samplePos))
        {
            int currentChannel = currentMessage.getChannel();

            /*if (currentMessage.isNoteOnOrOff())
            {
                // auto transposedMessage = currentMessage;
                // auto oldNoteNumber = transposedMessage.getNoteNumber();
                // transposedMessage.setNoteNumber(oldNoteNumber + interval);
                // processedBuffer.addEvent(transposedMessage, samplePos);
            }*/

            if (currentMessage.isPitchWheel())
            {
                // store user's pitch alteration
                pitchWheelValue = currentMessage.getPitchWheelValue();
            }
            if (currentMessage.isNoteOn())
            {
                int noteNumber = currentMessage.getNoteNumber();
                int pitchCorrection = getPitchCorrection(noteNumber);

                if (pitchCorrection != 0) {
                    // create a pitch message summing the wheel alteration and the note alteration
                    MidiMessage pitchMessage = MidiMessage::pitchWheel(currentChannel, pitchCorrection + pitchWheelValue);
                    // stops all playing notes (MONOPHONIC FUNCTION)
                    MidiMessage cleanMessage = MidiMessage::allNotesOff(currentChannel);
                    processedBuffer.addEvent(cleanMessage, samplePos);
                    processedBuffer.addEvent(pitchMessage, samplePos);
                }
            }
            if (currentMessage.isNoteOff())
            {
                int noteNumber = currentMessage.getNoteNumber();
                int pitchCorrection = getPitchCorrection(noteNumber);

                if (pitchCorrection != 0) {
                    // restore pitchWheel value removing the note alteration contribution
                    MidiMessage pitchMessage = MidiMessage::pitchWheel(currentChannel, pitchWheelValue);
                    processedBuffer.addEvent(pitchMessage, samplePos);
                }
                
                // reset pitch correction for future notes
                pitchCorrection = 0;
            }

            processedBuffer.addEvent(currentMessage, samplePos);
        }
    }

    MidiBuffer processedBuffer;
};
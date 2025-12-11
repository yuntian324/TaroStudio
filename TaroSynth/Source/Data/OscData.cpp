#include "OscData.h"

void OscData::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    resetAll();
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    
    prepare (spec);
    fmOsc.prepare (spec);
    gain.prepare (spec);    
}

void OscData::setType (const int oscSelection)
{
    switch (oscSelection)
    {
        // Sine
        case 0:
            initialise ([](float x) { return std::sin (x); });
            break;
            
        // Saw(sawtooth wave)锯齿波
        case 1:
            initialise ([] (float x) { return x / juce::MathConstants<float>::pi; });
            break;
          
        // Square
        case 2:
            initialise ([] (float x) { return x < 0.0f ? -1.0f : 1.0f; });
            break;

        // 三角波
        // x范围：-pi ~ pi
        case 3:
            initialise ([](float x) {
                float v = std::fmod(x, juce::MathConstants<float>::pi) / juce::MathConstants<float>::pi;
                v = v > 0 ? v : (-v);
                v -= 0.5f;
                return v * 2.0f;
                }
            );          
            break;

        default:
            // You shouldn't be here!
            jassertfalse;
            break;
    }
}

void OscData::setGain (const float levelInDecibels)
{
    gain.setGainDecibels (levelInDecibels);
}

void OscData::setOscPitch (const int pitch)
{
    lastPitch = pitch;
    setFrequency (juce::MidiMessage::getMidiNoteInHertz ((lastMidiNote + lastPitch) + fmModulator));

}

void OscData::setFreq (const int midiNoteNumber)
{
    setFrequency (juce::MidiMessage::getMidiNoteInHertz ((midiNoteNumber + lastPitch) + fmModulator));
    lastMidiNote = midiNoteNumber;
}

void OscData::setFmOsc (const float freq, const float depth)
{
    fmDepth = depth;
    fmOsc.setFrequency (freq);
    setFrequency (juce::MidiMessage::getMidiNoteInHertz ((lastMidiNote + lastPitch) + fmModulator));
}

void OscData::renderNextBlock (juce::dsp::AudioBlock<float>& audioBlock)
{
    jassert (audioBlock.getNumSamples() > 0);
    process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
}

float OscData::processNextSample (float input)
{
    fmModulator = fmOsc.processSample (input) * fmDepth;
    return gain.processSample (processSample (input));
}

void OscData::setParams (const int oscChoice, const float oscGain, const int oscPitch, const float fmFreq, const float fmDepth)
{
    setType (oscChoice);
    setGain (oscGain);
    setOscPitch (oscPitch);
    setFmOsc (fmFreq, fmDepth);
}

void OscData::resetAll()
{
    reset();
    fmOsc.reset();
    gain.reset();
}

#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"
#include "ModulationMatrix.h"

class SalekSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SalekVoice : public juce::SynthesiserVoice
{
public:
    SalekVoice (juce::AudioProcessorValueTreeState& apvts, ModulationMatrix& matrix);
    ~SalekVoice() override = default;

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void prepare (double sampleRate, int samplesPerBlock);

private:
    void updateParameters();
    void applyModulation (float* destOffsets);

    float noteHz = 440.0f;
    float velocity = 0.0f;
    int currentNote = 60;

    juce::AudioProcessorValueTreeState& apvts;
    ModulationMatrix& modMatrix;

    // 3 main oscs + sub (unison handled by detune/voices later)
    Oscillator osc1, osc2, osc3;
    Oscillator subOsc;

    MultiFilter filter;
    Envelope ampEnv, filterEnv, modEnv;
    LFO lfo1, lfo2, lfo3, lfo4;

    juce::Random noiseRandom;
    float noiseLevel = 0.0f;

    // Simple unison (up to 5 voices per osc for performance)
    static constexpr int maxUnison = 5;
    std::array<float, maxUnison> unisonDetune {};
    int unisonVoices = 1;
    float unisonDetuneAmt = 0.12f;

    double sr = 44100.0;
    bool isPrepared = false;

    // Mod source buffer
    float modSources[ModulationMatrix::numSources] {};
    float modDests[ModulationMatrix::numDests] {};
};

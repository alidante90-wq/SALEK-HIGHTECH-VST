#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "../Oscillators/WavetableOscillator.h"
#include "../Wavetables/WavetableData.h"
#include "../Filters/StateVariableFilter.h"
#include "../Modulation/LFO.h"
#include "../DSP/SmoothedValue.h"

namespace salek
{

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice();

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override;

    void prepareToPlay (double sampleRate, int samplesPerBlock);

    // Oscillator basic
    void setOsc1TablePos (float v) { osc1.setTablePosition (v); }
    void setOsc2TablePos (float v) { osc2.setTablePosition (v); }
    void setOsc3TablePos (float v) { osc3.setTablePosition (v); }
    void setOsc1Level (float v) { osc1.setLevel (v); }
    void setOsc2Level (float v) { osc2.setLevel (v); }
    void setOsc3Level (float v) { osc3.setLevel (v); }
    void setOsc1Octave (int oct) { osc1Octave = oct; updateFrequencies(); }
    void setOsc2Octave (int oct) { osc2Octave = oct; updateFrequencies(); }
    void setOsc3Octave (int oct) { osc3Octave = oct; updateFrequencies(); }
    void setOsc1Semi (int semi) { osc1Semi = semi; updateFrequencies(); }
    void setOsc2Semi (int semi) { osc2Semi = semi; updateFrequencies(); }
    void setOsc3Semi (int semi) { osc3Semi = semi; updateFrequencies(); }
    void setOsc1Fine (float cents) { osc1Fine = cents; updateFrequencies(); }
    void setOsc2Fine (float cents) { osc2Fine = cents; updateFrequencies(); }
    void setOsc3Fine (float cents) { osc3Fine = cents; updateFrequencies(); }
    void setOsc1Detune (float cents) { osc1.setDetuneCents (cents); }
    void setOsc2Detune (float cents) { osc2.setDetuneCents (cents); }
    void setOsc3Detune (float cents) { osc3.setDetuneCents (cents); }
    void setOsc1Warp (float v) { osc1.setWarp (v); }
    void setOsc2Warp (float v) { osc2.setWarp (v); }
    void setOsc3Warp (float v) { osc3.setWarp (v); }
    void setOsc1Fold (float v) { osc1.setFold (v); }
    void setOsc2Fold (float v) { osc2.setFold (v); }
    void setOsc3Fold (float v) { osc3.setFold (v); }
    void setOsc1Drive (float v) { osc1.setDrive (v); }
    void setOsc2Drive (float v) { osc2.setDrive (v); }
    void setOsc3Drive (float v) { osc3.setDrive (v); }
    void setOsc1Phase (float v) { osc1.setPhaseOffset (v); }
    void setOsc2Phase (float v) { osc2.setPhaseOffset (v); }
    void setOsc3Phase (float v) { osc3.setPhaseOffset (v); }

    // Cross modulation amounts (real)
    void setFm2to1 (float v) { fm2to1 = v; }
    void setFm3to1 (float v) { fm3to1 = v; }
    void setFm3to2 (float v) { fm3to2 = v; }
    void setPm2to1 (float v) { pm2to1 = v; }
    void setPm3to1 (float v) { pm3to1 = v; }
    void setAm2to1 (float v) { am2to1 = v; }
    void setRm2to1 (float v) { rm2to1 = v; }

    void setFilterCutoff (float hz) { filter.setCutoff (hz); }
    void setFilterResonance (float r) { filter.setResonance (r); }
    void setFilterDrive (float d) { filter.setDrive (d); }
    void setFilterMode (int m) {
        using M = StateVariableFilter::Mode;
        filter.setMode (m == 1 ? M::HighPass : m == 2 ? M::BandPass : m == 3 ? M::Notch : M::LowPass);
    }
    void setFilterEnvAmt (float a) { filterEnvAmt = a; }
    void setFilterBaseCutoff (float hz) { baseCutoff = hz; filter.setCutoff (hz); }

    void setAmpAttack (float s)  { adsrParams.attack  = juce::jmax (0.001f, s); adsr.setParameters (adsrParams); }
    void setAmpDecay (float s)   { adsrParams.decay   = juce::jmax (0.001f, s); adsr.setParameters (adsrParams); }
    void setAmpSustain (float s) { adsrParams.sustain = juce::jlimit (0.0f, 1.0f, s); adsr.setParameters (adsrParams); }
    void setAmpRelease (float s) { adsrParams.release = juce::jmax (0.001f, s); adsr.setParameters (adsrParams); }

    void setLfoRate (float hz) { lfo.setRate (hz); }
    void setLfoAmount (float a) { lfo.setAmount (a); }
    void setLfoWave (int w) {
        using W = LFO::Wave;
        lfo.setWave (w == 1 ? W::Triangle : w == 2 ? W::Saw : w == 3 ? W::Square : w == 4 ? W::SAndH : W::Sine);
    }

    Wavetable& getSharedWavetable() { return sharedWavetable; }

    void labRegenerate() { sharedWavetable.generateDefaultTables(); }
    void labProcessFrames (float fold, float drive) {
        for (int i = 0; i < sharedWavetable.getNumFrames(); ++i)
            sharedWavetable.processFrame (i, fold, drive);
    }
    void labMorph (float t) {
        // morph frame 0 and 4 into frame 2 as example
        if (sharedWavetable.getNumFrames() >= 5)
            sharedWavetable.morphFrames (0, 4, 2, t);
    }

private:
    void updateFrequencies();

    WavetableOscillator osc1, osc2, osc3;
    Wavetable sharedWavetable;

    double currentSampleRate = 44100.0;
    float currentVelocity = 0.0f;
    int currentMidiNote = 60;

    int osc1Octave = 0, osc2Octave = 0, osc3Octave = -1;
    int osc1Semi = 0, osc2Semi = 0, osc3Semi = 0;
    float osc1Fine = 0.0f, osc2Fine = 0.0f, osc3Fine = 0.0f;

    // Modulation depths
    float fm2to1 = 0.0f, fm3to1 = 0.0f, fm3to2 = 0.0f;
    float pm2to1 = 0.0f, pm3to1 = 0.0f;
    float am2to1 = 0.0f, rm2to1 = 0.0f;

    StateVariableFilter filter;
    float filterEnvAmt = 0.5f;
    float baseCutoff = 8000.0f;
    LFO lfo;
    SmoothedValue cutoffSmoother;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams { 0.01f, 0.1f, 0.8f, 0.2f };

    bool isNoteOn = false;
};

} // namespace salek

#pragma once
#include <JuceHeader.h>
#include "SynthVoice.h"
#include "SynthSound.h"

namespace salek
{

class SynthEngine
{
public:
    SynthEngine();
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);
    void allNotesOff() { synth.allNotesOff (0, true); }

    void setOsc1TablePos (float v); void setOsc2TablePos (float v); void setOsc3TablePos (float v);
    void setOsc1Level (float v); void setOsc2Level (float v); void setOsc3Level (float v);
    void setOsc1Octave (int v); void setOsc2Octave (int v); void setOsc3Octave (int v);
    void setOsc1Semi (int v); void setOsc2Semi (int v); void setOsc3Semi (int v);
    void setOsc1Fine (float v); void setOsc2Fine (float v); void setOsc3Fine (float v);
    void setOsc1Detune (float v); void setOsc2Detune (float v); void setOsc3Detune (float v);
    void setOsc1Warp (float v); void setOsc2Warp (float v); void setOsc3Warp (float v);
    void setOsc1Fold (float v); void setOsc2Fold (float v); void setOsc3Fold (float v);
    void setOsc1Drive (float v); void setOsc2Drive (float v); void setOsc3Drive (float v);
    void setOsc1Phase (float v); void setOsc2Phase (float v); void setOsc3Phase (float v);
    void setUnison (int voices); void setUnisonDetune (float cents); void setUnisonSpread (float s);
    void setScaleMode (int m); void setKoronCents (float c);

    void setFm2to1 (float v); void setFm3to1 (float v); void setFm3to2 (float v);
    void setPm2to1 (float v); void setPm3to1 (float v);
    void setAm2to1 (float v); void setRm2to1 (float v);

    void setFilterCutoff (float hz); void setFilterResonance (float r);
    void setFilterDrive (float d); void setFilterMode (int mode); void setFilterEnvAmt (float a);

    void setAmpAttack (float v); void setAmpDecay (float v);
    void setAmpSustain (float v); void setAmpRelease (float v);

    void setLfoRate (float hz); void setLfoAmount (float a); void setLfoWave (int w);

    void labRegenerate(); void labProcessFrames (float fold, float drive); void labMorph (float t);

private:
    static constexpr int maxVoices = 8;
    juce::Synthesiser synth;
    template <typename Fn>
    void forEachVoice (Fn&& fn)
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
            if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
                fn (*v);
    }
};

} // namespace salek

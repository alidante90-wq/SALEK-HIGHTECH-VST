#include "SynthEngine.h"

namespace salek
{

SynthEngine::SynthEngine()
{
    for (int i = 0; i < maxVoices; ++i)
        synth.addVoice (new SynthVoice());

    synth.addSound (new SynthSound());
}

void SynthEngine::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    forEachVoice ([&] (SynthVoice& v) { v.prepareToPlay (sampleRate, samplesPerBlock); });
}

void SynthEngine::releaseResources()
{
}

void SynthEngine::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
}

void SynthEngine::setOsc1TablePos (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1TablePos (v); }); }
void SynthEngine::setOsc2TablePos (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2TablePos (v); }); }
void SynthEngine::setOsc3TablePos (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3TablePos (v); }); }

void SynthEngine::setOsc1Level (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Level (v); }); }
void SynthEngine::setOsc2Level (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Level (v); }); }
void SynthEngine::setOsc3Level (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Level (v); }); }

void SynthEngine::setOsc1Octave (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Octave (v); }); }
void SynthEngine::setOsc2Octave (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Octave (v); }); }
void SynthEngine::setOsc3Octave (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Octave (v); }); }

void SynthEngine::setOsc1Semi (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Semi (v); }); }
void SynthEngine::setOsc2Semi (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Semi (v); }); }
void SynthEngine::setOsc3Semi (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Semi (v); }); }

void SynthEngine::setOsc1Fine (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Fine (v); }); }
void SynthEngine::setOsc2Fine (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Fine (v); }); }
void SynthEngine::setOsc3Fine (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Fine (v); }); }

void SynthEngine::setOsc1Detune (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Detune (v); }); }
void SynthEngine::setOsc2Detune (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Detune (v); }); }
void SynthEngine::setOsc3Detune (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Detune (v); }); }

void SynthEngine::setOsc1Warp (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Warp (v); }); }
void SynthEngine::setOsc2Warp (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Warp (v); }); }
void SynthEngine::setOsc3Warp (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Warp (v); }); }
void SynthEngine::setOsc1Fold (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Fold (v); }); }
void SynthEngine::setOsc2Fold (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Fold (v); }); }
void SynthEngine::setOsc3Fold (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Fold (v); }); }
void SynthEngine::setOsc1Drive (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Drive (v); }); }
void SynthEngine::setOsc2Drive (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Drive (v); }); }
void SynthEngine::setOsc3Drive (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Drive (v); }); }

void SynthEngine::setOsc1Phase (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc1Phase (v); }); }
void SynthEngine::setOsc2Phase (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc2Phase (v); }); }
void SynthEngine::setOsc3Phase (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setOsc3Phase (v); }); }

} // namespace salek

void SynthEngine::setFm2to1 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFm2to1 (v); }); }
void SynthEngine::setFm3to1 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFm3to1 (v); }); }
void SynthEngine::setFm3to2 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFm3to2 (v); }); }
void SynthEngine::setPm2to1 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setPm2to1 (v); }); }
void SynthEngine::setPm3to1 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setPm3to1 (v); }); }
void SynthEngine::setAm2to1 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setAm2to1 (v); }); }
void SynthEngine::setRm2to1 (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setRm2to1 (v); }); }
void SynthEngine::setFilterCutoff (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFilterBaseCutoff (v); }); }
void SynthEngine::setFilterResonance (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFilterResonance (v); }); }
void SynthEngine::setFilterDrive (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFilterDrive (v); }); }
void SynthEngine::setFilterMode (int v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFilterMode (v); }); }
void SynthEngine::setFilterEnvAmt (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setFilterEnvAmt (v); }); }
void SynthEngine::setAmpAttack (float v)  { forEachVoice ([&] (SynthVoice& voice) { voice.setAmpAttack (v); }); }
void SynthEngine::setAmpDecay (float v)   { forEachVoice ([&] (SynthVoice& voice) { voice.setAmpDecay (v); }); }
void SynthEngine::setAmpSustain (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setAmpSustain (v); }); }
void SynthEngine::setAmpRelease (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setAmpRelease (v); }); }
void SynthEngine::setLfoRate (float v)   { forEachVoice ([&] (SynthVoice& voice) { voice.setLfoRate (v); }); }
void SynthEngine::setLfoAmount (float v) { forEachVoice ([&] (SynthVoice& voice) { voice.setLfoAmount (v); }); }
void SynthEngine::setLfoWave (int v)     { forEachVoice ([&] (SynthVoice& voice) { voice.setLfoWave (v); }); }
void SynthEngine::labRegenerate() { forEachVoice ([&] (SynthVoice& v) { v.labRegenerate(); }); }
void SynthEngine::labProcessFrames (float fold, float drive) { forEachVoice ([&] (SynthVoice& v) { v.labProcessFrames (fold, drive); }); }
void SynthEngine::labMorph (float t) { forEachVoice ([&] (SynthVoice& v) { v.labMorph (t); }); }

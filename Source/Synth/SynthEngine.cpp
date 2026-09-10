#include "SynthEngine.h"
namespace salek {
SynthEngine::SynthEngine() {
    for (int i = 0; i < maxVoices; ++i) synth.addVoice(new SynthVoice());
    synth.addSound(new SynthSound());
}
void SynthEngine::prepareToPlay(double sr, int spb) {
    synth.setCurrentPlaybackSampleRate(sr);
    forEachVoice([&](SynthVoice& v){ v.prepareToPlay(sr, spb); });
}
void SynthEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
}
#define FV(name, call) void SynthEngine::name { forEachVoice([&](SynthVoice& voice){ voice.call; }); }
FV(setOsc1TablePos(float v), setOsc1TablePos(v))
FV(setOsc2TablePos(float v), setOsc2TablePos(v))
FV(setOsc3TablePos(float v), setOsc3TablePos(v))
FV(setOsc1Level(float v), setOsc1Level(v))
FV(setOsc2Level(float v), setOsc2Level(v))
FV(setOsc3Level(float v), setOsc3Level(v))
FV(setOsc1Octave(int v), setOsc1Octave(v))
FV(setOsc2Octave(int v), setOsc2Octave(v))
FV(setOsc3Octave(int v), setOsc3Octave(v))
FV(setOsc1Semi(int v), setOsc1Semi(v))
FV(setOsc2Semi(int v), setOsc2Semi(v))
FV(setOsc3Semi(int v), setOsc3Semi(v))
FV(setOsc1Fine(float v), setOsc1Fine(v))
FV(setOsc2Fine(float v), setOsc2Fine(v))
FV(setOsc3Fine(float v), setOsc3Fine(v))
FV(setOsc1Detune(float v), setOsc1Detune(v))
FV(setOsc2Detune(float v), setOsc2Detune(v))
FV(setOsc3Detune(float v), setOsc3Detune(v))
FV(setOsc1Warp(float v), setOsc1Warp(v))
FV(setOsc2Warp(float v), setOsc2Warp(v))
FV(setOsc3Warp(float v), setOsc3Warp(v))
FV(setOsc1Fold(float v), setOsc1Fold(v))
FV(setOsc2Fold(float v), setOsc2Fold(v))
FV(setOsc3Fold(float v), setOsc3Fold(v))
FV(setOsc1Drive(float v), setOsc1Drive(v))
FV(setOsc2Drive(float v), setOsc2Drive(v))
FV(setOsc3Drive(float v), setOsc3Drive(v))
FV(setOsc1Phase(float v), setOsc1Phase(v))
FV(setOsc2Phase(float v), setOsc2Phase(v))
FV(setOsc3Phase(float v), setOsc3Phase(v))
FV(setFm2to1(float v), setFm2to1(v))
FV(setFm3to1(float v), setFm3to1(v))
FV(setFm3to2(float v), setFm3to2(v))
FV(setPm2to1(float v), setPm2to1(v))
FV(setPm3to1(float v), setPm3to1(v))
FV(setAm2to1(float v), setAm2to1(v))
FV(setRm2to1(float v), setRm2to1(v))
FV(setFilterCutoff(float v), setFilterBaseCutoff(v))
FV(setFilterResonance(float v), setFilterResonance(v))
FV(setFilterDrive(float v), setFilterDrive(v))
FV(setFilterMode(int v), setFilterMode(v))
FV(setFilterEnvAmt(float v), setFilterEnvAmt(v))
FV(setAmpAttack(float v), setAmpAttack(v))
FV(setAmpDecay(float v), setAmpDecay(v))
FV(setAmpSustain(float v), setAmpSustain(v))
FV(setAmpRelease(float v), setAmpRelease(v))
FV(setLfoRate(float v), setLfoRate(v))
FV(setLfoAmount(float v), setLfoAmount(v))
FV(setLfoWave(int v), setLfoWave(v))
FV(labRegenerate(), labRegenerate())
FV(labProcessFrames(float fold, float drive), labProcessFrames(fold, drive))
FV(labMorph(float t), labMorph(t))
#undef FV
}

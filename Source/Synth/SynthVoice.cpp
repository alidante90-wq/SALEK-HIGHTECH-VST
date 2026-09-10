#include "SynthVoice.h"
namespace salek {
void SynthVoice::updateFrequencies() {
    if (currentSampleRate <= 0) return;
    auto noteToHz = [](int note, int oct, int semi, float fine) {
        float midi = float(note + oct*12 + semi) + fine/100.f;
        return 440.f * std::pow(2.f, (midi - 69.f)/12.f);
    };
    osc1.setFrequency(noteToHz(currentMidiNote, osc1Octave, osc1Semi, osc1Fine));
    osc2.setFrequency(noteToHz(currentMidiNote, osc2Octave, osc2Semi, osc2Fine));
    osc3.setFrequency(noteToHz(currentMidiNote, osc3Octave, osc3Semi, osc3Fine));
}
void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!isVoiceActive()) return;
    auto* left = outputBuffer.getWritePointer(0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;
    for (int i = 0; i < numSamples; ++i) {
        if (!adsr.isActive() && !isNoteOn) { clearCurrentNote(); break; }
        float s3 = osc3.processSample(0.f, 1.f);
        float s2 = osc2.processSample(s3 * fm3to2 * 0.5f, 1.f);
        float pm = s2 * (fm2to1 * 2.5f + pm2to1) + s3 * (fm3to1 * 2.5f + pm3to1);
        float am = am2to1 > 1e-4f ? 1.f + s2 * am2to1 : 1.f;
        float s1 = osc1.processSample(pm, am);
        float sample = s1;
        if (rm2to1 > 1e-4f) sample = s1 * (1.f - rm2to1) + (s1 * s2) * rm2to1;
        sample += s2 * 0.15f + s3 * 0.12f;
        float env = adsr.getNextSample();
        float lfoVal = lfo.process();
        float modCutoff = baseCutoff * std::pow(2.f, (env * filterEnvAmt + lfoVal) * 3.f - 1.5f);
        cutoffSmoother.setTarget(modCutoff);
        filter.setCutoff(cutoffSmoother.getNext());
        sample = filter.process(sample);
        sample *= env * currentVelocity * 0.28f;
        left[i] += sample;
        if (right) right[i] += sample;
    }
    if (!adsr.isActive()) { clearCurrentNote(); isNoteOn = false; }
}
}

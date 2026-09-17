#include "SynthVoice.h"

namespace salek {

void SynthVoice::updateFrequencies()
{
    osc1.setFrequency (noteToHz (currentMidiNote, osc1Octave, osc1Semi, osc1Fine));
    osc2.setFrequency (noteToHz (currentMidiNote, osc2Octave, osc2Semi, osc2Fine));
    osc3.setFrequency (noteToHz (currentMidiNote, osc3Octave, osc3Semi, osc3Fine));
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    auto* left  = outputBuffer.getWritePointer (0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1
                    ? outputBuffer.getWritePointer (1, startSample) : nullptr;

    const int nUni = juce::jmax (1, unisonVoices);
    const float invN = 1.0f / std::sqrt ((float) nUni);

    for (int i = 0; i < numSamples; ++i)
    {
        if (! adsr.isActive() && ! isNoteOn)
        {
            clearCurrentNote();
            break;
        }

        // --- OSC3 with unison (modulator) ---
        float s3L = 0.f, s3R = 0.f, s3Mono = 0.f;
        for (int u = 0; u < nUni; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (nUni > 1)
            {
                const float t = ((float) u / (float) (nUni - 1)) * 2.0f - 1.0f;
                det = t * unisonDetune * 0.85f; // slightly less detune on mod oscs
                pan = 0.5f + 0.5f * t * unisonSpread;
            }
            uniOsc3[u].setFrequency (noteToHz (currentMidiNote, osc3Octave, osc3Semi, osc3Fine + det));
            const float s = uniOsc3[u].processSample (0.0f, 1.0f);
            s3L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s3R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
            s3Mono += s;
        }
        s3L *= invN; s3R *= invN; s3Mono *= invN;
        (void) osc3.processSample (0.0f, 1.0f);

        // --- OSC2 with unison (FM from OSC3) ---
        float s2L = 0.f, s2R = 0.f, s2Mono = 0.f;
        const float pmFor2 = s3Mono * fm3to2 * 0.5f;
        for (int u = 0; u < nUni; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (nUni > 1)
            {
                const float t = ((float) u / (float) (nUni - 1)) * 2.0f - 1.0f;
                det = t * unisonDetune * 0.9f;
                pan = 0.5f + 0.5f * t * unisonSpread;
            }
            uniOsc2[u].setFrequency (noteToHz (currentMidiNote, osc2Octave, osc2Semi, osc2Fine + det));
            const float s = uniOsc2[u].processSample (pmFor2, 1.0f);
            s2L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s2R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
            s2Mono += s;
        }
        s2L *= invN; s2R *= invN; s2Mono *= invN;
        (void) osc2.processSample (pmFor2, 1.0f);

        // --- OSC1 with unison (carrier, FM/PM/AM from 2&3) ---
        const float fmIndex2 = fm2to1 * 2.5f;
        const float fmIndex3 = fm3to1 * 2.5f;
        const float pmFrom2  = s2Mono * (fmIndex2 + pm2to1);
        const float pmFrom3  = s3Mono * (fmIndex3 + pm3to1);
        float am = 1.0f;
        if (am2to1 > 1.0e-4f)
            am = 1.0f + (s2Mono * am2to1);
        const float pmCarrier = pmFrom2 + pmFrom3;

        float s1L = 0.f, s1R = 0.f;
        for (int u = 0; u < nUni; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (nUni > 1)
            {
                const float t = ((float) u / (float) (nUni - 1)) * 2.0f - 1.0f;
                det = t * unisonDetune;
                pan = 0.5f + 0.5f * t * unisonSpread;
            }
            uniOsc1[u].setFrequency (noteToHz (currentMidiNote, osc1Octave, osc1Semi, osc1Fine + det));
            const float s = uniOsc1[u].processSample (pmCarrier, am);
            s1L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s1R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
        }
        s1L *= invN; s1R *= invN;
        (void) osc1.processSample (pmCarrier, am);

        float sampleL = s1L, sampleR = s1R;
        if (rm2to1 > 1.0e-4f)
        {
            sampleL = s1L * (1.0f - rm2to1) + (s1L * s2Mono) * rm2to1;
            sampleR = s1R * (1.0f - rm2to1) + (s1R * s2Mono) * rm2to1;
        }
        // blend OSC2/OSC3 stereo images
        sampleL += s2L * 0.22f + s3L * 0.18f;
        sampleR += s2R * 0.22f + s3R * 0.18f;

        const float env = adsr.getNextSample();
        const float lfoVal = lfo.process();
        const float modCutoff = baseCutoff * std::pow (2.0f, (env * filterEnvAmt + lfoVal) * 3.0f - 1.5f);
        cutoffSmoother.setTarget (modCutoff);
        filter.setCutoff (cutoffSmoother.getNext());
        float mid = filter.process (0.5f * (sampleL + sampleR));
        float side = 0.5f * (sampleL - sampleR) * 1.15f;
        mid = std::tanh (mid * 1.25f);
        const float g = env * currentVelocity * 0.42f;
        sampleL = (mid + side) * g;
        sampleR = (mid - side) * g;

        left[i] += sampleL;
        if (right != nullptr)
            right[i] += sampleR;
    }

    if (! adsr.isActive())
    {
        clearCurrentNote();
        isNoteOn = false;
    }
}

} // namespace salek

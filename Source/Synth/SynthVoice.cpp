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

    const int nUni1 = juce::jmax (1, uniVoices1);
    const int nUni2 = juce::jmax (1, uniVoices2);
    const int nUni3 = juce::jmax (1, uniVoices3);
    const float invN1 = 1.0f / std::sqrt ((float) nUni1);
    const float invN2 = 1.0f / std::sqrt ((float) nUni2);
    const float invN3 = 1.0f / std::sqrt ((float) nUni3);

    for (int i = 0; i < numSamples; ++i)
    {
        if (! adsr.isActive() && ! isNoteOn)
        {
            clearCurrentNote();
            break;
        }

        // --- OSC3 with unison (modulator) ---
        float s3L = 0.f, s3R = 0.f, s3Mono = 0.f;
        for (int u = 0; u < nUni3; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (nUni3 > 1)
            {
                const float t = ((float) u / (float) (nUni3 - 1)) * 2.0f - 1.0f;
                det = t * uniDet3 * 0.85f;
                pan = 0.5f + 0.5f * t * uniSpr3;
            }
            uniOsc3[u].setFrequency (noteToHz (currentMidiNote, osc3Octave, osc3Semi, osc3Fine + det));
            const float s = uniOsc3[u].processSample (0.0f, 1.0f);
            s3L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s3R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
            s3Mono += s;
        }
        s3L *= invN3; s3R *= invN3; s3Mono *= invN3;
        (void) osc3.processSample (0.0f, 1.0f);

        // --- OSC2 with unison (FM from OSC3) ---
        float s2L = 0.f, s2R = 0.f, s2Mono = 0.f;
        const float pmFor2 = s3Mono * fm3to2 * 0.5f;
        for (int u = 0; u < nUni2; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (nUni2 > 1)
            {
                const float t = ((float) u / (float) (nUni2 - 1)) * 2.0f - 1.0f;
                det = t * uniDet2 * 0.9f;
                pan = 0.5f + 0.5f * t * uniSpr2;
            }
            uniOsc2[u].setFrequency (noteToHz (currentMidiNote, osc2Octave, osc2Semi, osc2Fine + det));
            const float s = uniOsc2[u].processSample (pmFor2, 1.0f);
            s2L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s2R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
            s2Mono += s;
        }
        s2L *= invN2; s2R *= invN2; s2Mono *= invN2;
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
        for (int u = 0; u < nUni1; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (nUni1 > 1)
            {
                const float t = ((float) u / (float) (nUni1 - 1)) * 2.0f - 1.0f;
                det = t * uniDet1;
                pan = 0.5f + 0.5f * t * uniSpr1;
            }
            uniOsc1[u].setFrequency (noteToHz (currentMidiNote, osc1Octave, osc1Semi, osc1Fine + det));
            const float s = uniOsc1[u].processSample (pmCarrier, am);
            s1L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s1R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
        }
        s1L *= invN1; s1R *= invN1;
        (void) osc1.processSample (pmCarrier, am);

        // Per-osc levels before filter route
        float o1L = s1L, o1R = s1R;
        float o2L = s2L, o2R = s2R;
        float o3L = s3L, o3R = s3R;
        if (rm2to1 > 1.0e-4f)
        {
            o1L = s1L * (1.0f - rm2to1) + (s1L * s2Mono) * rm2to1;
            o1R = s1R * (1.0f - rm2to1) + (s1R * s2Mono) * rm2to1;
        }
        // Sub oscillator (square-ish at -1 oct)
        float subL = 0.f, subR = 0.f;
        if (subLevel > 1.0e-4f)
        {
            const float subHz = noteToHz (currentMidiNote, -1, 0, 0);
            subPhase += subHz / (float) getSampleRate();
            if (subPhase >= 1.f) subPhase -= 1.f;
            const float sq = subPhase < 0.5f ? 1.f : -1.f;
            subL = subR = sq * subLevel * 0.35f;
        }
        // Noise
        float nL = 0.f, nR = 0.f;
        if (noiseLevel > 1.0e-4f)
        {
            const float n = (noiseRng.nextFloat() * 2.f - 1.f) * noiseLevel * 0.25f;
            nL = nR = n;
        }

        // Filter route: which oscs go through filter
        const bool f1 = (filterRoute == 0 || filterRoute == 1 || filterRoute == 4 || filterRoute == 5);
        const bool f2 = (filterRoute == 0 || filterRoute == 2 || filterRoute == 4 || filterRoute == 6);
        const bool f3 = (filterRoute == 0 || filterRoute == 3 || filterRoute == 5 || filterRoute == 6);
        float thruL = 0.f, thruR = 0.f, dryL = 0.f, dryR = 0.f;
        if (f1) { thruL += o1L; thruR += o1R; } else { dryL += o1L; dryR += o1R; }
        if (f2) { thruL += o2L * 0.22f; thruR += o2R * 0.22f; } else { dryL += o2L * 0.22f; dryR += o2R * 0.22f; }
        if (f3) { thruL += o3L * 0.18f; thruR += o3R * 0.18f; } else { dryL += o3L * 0.18f; dryR += o3R * 0.18f; }
        // sub + noise always filtered when route=All, else dry
        if (filterRoute == 0) { thruL += subL + nL; thruR += subR + nR; }
        else { dryL += subL + nL; dryR += subR + nR; }

        const float env = adsr.getNextSample();
        const float lfoVal = lfo.process();
        const float modCutoff = baseCutoff * std::pow (2.0f, (env * filterEnvAmt + lfoVal) * 3.0f - 1.5f);
        cutoffSmoother.setTarget (modCutoff);
        filter.setCutoff (cutoffSmoother.getNext());
        float mid = filter.process (0.5f * (thruL + thruR));
        float side = 0.5f * (thruL - thruR) * 1.15f;
        mid = std::tanh (mid * 1.25f);
        float sampleL = mid + side + dryL;
        float sampleR = mid - side + dryR;
        const float g = env * currentVelocity * 0.42f;
        sampleL *= g;
        sampleR *= g;

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

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

    for (int i = 0; i < numSamples; ++i)
    {
        if (! adsr.isActive() && ! isNoteOn)
        {
            clearCurrentNote();
            break;
        }

        const float s3 = osc3.processSample (0.0f, 1.0f);
        const float pmFor2 = s3 * fm3to2 * 0.5f;
        const float s2 = osc2.processSample (pmFor2, 1.0f);

        const float fmIndex2 = fm2to1 * 2.5f;
        const float fmIndex3 = fm3to1 * 2.5f;
        const float pmFrom2  = s2 * (fmIndex2 + pm2to1);
        const float pmFrom3  = s3 * (fmIndex3 + pm3to1);
        float am = 1.0f;
        if (am2to1 > 1.0e-4f)
            am = 1.0f + (s2 * am2to1);

        const float pmCarrier = pmFrom2 + pmFrom3;

        float sL = 0.0f, sR = 0.0f;
        const int nUni = juce::jmax (1, unisonVoices);
        const float invN = 1.0f / std::sqrt ((float) nUni);
        for (int u = 0; u < nUni; ++u)
        {
            float det = 0.0f, pan = 0.5f;
            if (nUni > 1)
            {
                const float t = ((float) u / (float) (nUni - 1)) * 2.0f - 1.0f;
                det = t * unisonDetune;
                pan = 0.5f + 0.5f * t * unisonSpread;
            }
            uniOsc[u].setFrequency (noteToHz (currentMidiNote, osc1Octave, osc1Semi, osc1Fine + det));
            const float s = uniOsc[u].processSample (pmCarrier, am);
            sL += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            sR += s * std::sin (pan * juce::MathConstants<float>::halfPi);
        }
        sL *= invN; sR *= invN;
        (void) osc1.processSample (pmCarrier, am);

        float sampleL = sL, sampleR = sR;
        if (rm2to1 > 1.0e-4f)
        {
            sampleL = sL * (1.0f - rm2to1) + (sL * s2) * rm2to1;
            sampleR = sR * (1.0f - rm2to1) + (sR * s2) * rm2to1;
        }
        sampleL += s2 * 0.22f + s3 * 0.18f;
        sampleR += s2 * 0.22f + s3 * 0.18f;

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

#include "SynthVoice.h"

namespace salek {

void SynthVoice::updateFrequencies()
{
    osc1.setFrequency (noteToHz (currentMidiNote, osc1Octave, osc1Semi, osc1Fine));
    osc2.setFrequency (noteToHz (currentMidiNote, osc2Octave, osc2Semi, osc2Fine));
    osc3.setFrequency (noteToHz (currentMidiNote, osc3Octave, osc3Semi, osc3Fine));
    uniFreqDirty = true;
}

void SynthVoice::refreshUnisonTuning() noexcept
{
    // Soft budget: if all three oscs max unison, scale down to protect CPU
    int v1 = juce::jmax (1, uniVoices1);
    int v2 = juce::jmax (1, uniVoices2);
    int v3 = juce::jmax (1, uniVoices3);
    // Silent oscs cost 0 toward budget
    const int c1 = osc1Level > 1e-5f ? v1 : 0;
    const int c2 = osc2Level > 1e-5f ? v2 : 0;
    const int c3 = osc3Level > 1e-5f ? v3 : 0;
    int total = c1 + c2 + c3;
    if (total > maxUniBudget && total > 0)
    {
        const float scale = (float) maxUniBudget / (float) total;
        if (c1 > 0) v1 = juce::jmax (1, (int) std::lround (v1 * scale));
        if (c2 > 0) v2 = juce::jmax (1, (int) std::lround (v2 * scale));
        if (c3 > 0) v3 = juce::jmax (1, (int) std::lround (v3 * scale));
    }
    nUniEff1 = osc1Level > 1e-5f ? v1 : 0;
    nUniEff2 = osc2Level > 1e-5f ? v2 : 0;
    nUniEff3 = osc3Level > 1e-5f ? v3 : 0;

    auto fill = [&] (int n, float detAmt, float spr, float* detTbl, float* panTbl,
                     WavetableOscillator* oscs, int oct, int semi, float fine)
    {
        for (int u = 0; u < maxUnison; ++u)
        {
            float det = 0.f, pan = 0.5f;
            if (n > 1 && u < n)
            {
                const float t = ((float) u / (float) (n - 1)) * 2.0f - 1.0f;
                det = t * detAmt;
                pan = 0.5f + 0.5f * t * spr;
            }
            detTbl[u] = det;
            panTbl[u] = pan;
            if (u < n)
                oscs[u].setFrequency (noteToHz (currentMidiNote, oct, semi, fine + det));
        }
    };
    fill (nUniEff1, uniDet1, uniSpr1, uniDetTbl1, uniPanTbl1, uniOsc1, osc1Octave, osc1Semi, osc1Fine);
    fill (nUniEff2, uniDet2 * 0.9f, uniSpr2, uniDetTbl2, uniPanTbl2, uniOsc2, osc2Octave, osc2Semi, osc2Fine);
    fill (nUniEff3, uniDet3 * 0.85f, uniSpr3, uniDetTbl3, uniPanTbl3, uniOsc3, osc3Octave, osc3Semi, osc3Fine);
    uniFreqDirty = false;
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    auto* left  = outputBuffer.getWritePointer (0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1
                    ? outputBuffer.getWritePointer (1, startSample) : nullptr;

    if (uniFreqDirty)
        refreshUnisonTuning();

    const int nUni1 = juce::jmax (0, nUniEff1);
    const int nUni2 = juce::jmax (0, nUniEff2);
    const int nUni3 = juce::jmax (0, nUniEff3);
    const float invN1 = nUni1 > 0 ? 1.0f / std::sqrt ((float) nUni1) : 0.f;
    const float invN2 = nUni2 > 0 ? 1.0f / std::sqrt ((float) nUni2) : 0.f;
    const float invN3 = nUni3 > 0 ? 1.0f / std::sqrt ((float) nUni3) : 0.f;

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
            const float pan = uniPanTbl3[u];
            const float s = uniOsc3[u].processSample (0.0f, 1.0f);
            s3L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s3R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
            s3Mono += s;
        }
        if (nUni3 > 0) { s3L *= invN3; s3R *= invN3; s3Mono *= invN3; }

        // --- OSC2 with unison (FM from OSC3) ---
        float s2L = 0.f, s2R = 0.f, s2Mono = 0.f;
        const float pmFor2 = s3Mono * fm3to2 * 0.5f;
        for (int u = 0; u < nUni2; ++u)
        {
            const float pan = uniPanTbl2[u];
            const float s = uniOsc2[u].processSample (pmFor2, 1.0f);
            s2L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s2R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
            s2Mono += s;
        }
        if (nUni2 > 0) { s2L *= invN2; s2R *= invN2; s2Mono *= invN2; }

        // --- OSC1 with unison (carrier, FM/PM/AM from 2&3) ---
        // FM index: exponential feel — fine control low, aggressive high (Serum-like)
        const float fmIndex2 = fm2to1 * fm2to1 * 5.5f;
        const float fmIndex3 = fm3to1 * fm3to1 * 4.8f;
        const float pmFrom2  = s2Mono * (fmIndex2 + pm2to1 * 1.25f);
        const float pmFrom3  = s3Mono * (fmIndex3 + pm3to1 * 1.1f);
        float am = 1.0f;
        if (am2to1 > 1.0e-4f)
            am = 1.0f + (s2Mono * am2to1);
        const float pmCarrier = pmFrom2 + pmFrom3;

        float s1L = 0.f, s1R = 0.f;
        for (int u = 0; u < nUni1; ++u)
        {
            const float pan = uniPanTbl1[u];
            const float s = uniOsc1[u].processSample (pmCarrier, am);
            s1L += s * std::cos (pan * juce::MathConstants<float>::halfPi);
            s1R += s * std::sin (pan * juce::MathConstants<float>::halfPi);
        }
        if (nUni1 > 0) { s1L *= invN1; s1R *= invN1; }

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


        // Per-oscillator pan (0=L .. 1=R), preserves unison width
        auto applyPan = [] (float& L, float& R, float pan)
        {
            const float b = (pan - 0.5f) * 2.f; // -1..1
            if (b < 0.f) R *= (1.f + b);
            else         L *= (1.f - b);
        };
        applyPan (o1L, o1R, osc1Pan);
        applyPan (o2L, o2R, osc2Pan);
        applyPan (o3L, o3R, osc3Pan);

        // Filter route: which oscs go through filter
        const bool f1 = (filterRoute == 0 || filterRoute == 1 || filterRoute == 4 || filterRoute == 5);
        const bool f2 = (filterRoute == 0 || filterRoute == 2 || filterRoute == 4 || filterRoute == 6);
        const bool f3 = (filterRoute == 0 || filterRoute == 3 || filterRoute == 5 || filterRoute == 6);
        float thruL = 0.f, thruR = 0.f, dryL = 0.f, dryR = 0.f;
        // Full per-osc level (already applied in WavetableOscillator::processSample)
        if (f1) { thruL += o1L; thruR += o1R; } else { dryL += o1L; dryR += o1R; }
        if (f2) { thruL += o2L; thruR += o2R; } else { dryL += o2L; dryR += o2R; }
        if (f3) { thruL += o3L; thruR += o3R; } else { dryL += o3L; dryR += o3R; }
        // sub + noise always filtered when route=All, else dry
        if (filterRoute == 0) { thruL += subL + nL; thruR += subR + nR; }
        else { dryL += subL + nL; dryR += subR + nR; }

        const float env = adsr.getNextSample();
        const float lfoVal = lfo.process();
        // Filter-env opens relative to base; keep closed when base is near min
        float modCutoff = baseCutoff * std::pow (2.0f, (env * filterEnvAmt + lfoVal * 0.5f) * 2.5f);
        modCutoff = juce::jlimit (10.f, 20000.f, modCutoff);
        cutoffSmoother.setTarget (modCutoff);
        const float cutHz = cutoffSmoother.getNext();
        filter.setCutoff (cutHz);
        // Process L/R independently so stereo NEVER bypasses the filter
        float fL = filter.process (thruL);
        float fR = filter.process (thruR);
        // Extra close attenuation for LP-family when cutoff is very low
        if (cutHz < 80.f)
        {
            const float close = cutHz / 80.f; // 0 at 0Hz .. 1 at 80Hz
            const float close2 = close * close;
            fL *= close2;
            fR *= close2;
        }
        fL = std::tanh (fL * 1.15f);
        fR = std::tanh (fR * 1.15f);
        float sampleL = fL + dryL;
        float sampleR = fR + dryR;
        // Hard mute if all levels essentially off (noise/sub included)
        const float g = env * currentVelocity * 0.38f; // slightly softer headroom for modern stack
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

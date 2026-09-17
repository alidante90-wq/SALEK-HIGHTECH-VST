#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>
namespace salek {
/** Tempo-style delay with fractional read, stereo offset, feedback tone.
 *  TIME: 1 ms .. 1.8 s | FB: 0..0.95 | MIX: dry/wet */
class SimpleDelay {
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        const int maxS = (int) (sr * 2.0) + maxBlock + 16;
        bufL.assign ((size_t) maxS, 0.f);
        bufR.assign ((size_t) maxS, 0.f);
        writePos = 0;
        lpL = lpR = 0.f;
    }

    void setTimeMs (float ms) noexcept
    {
        targetDelay = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
    }
    void setFeedback (float fb) noexcept { feedback = juce::jlimit (0.f, 0.95f, fb); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int bs = (int) bufL.size();
        if (bs < 8) return;

        // smooth time changes (anti-zipper)
        const float smooth = 0.0015f;
        delaySamples += smooth * (targetDelay - delaySamples);

        // feedback damping: longer times slightly darker
        const float tone = juce::jlimit (0.25f, 0.9f, 0.82f - delaySamples / float (sr) * 0.3f);

        // stereo: R slightly longer for width (ping-ish)
        const float delayR = delaySamples * 1.068f;

        for (int i = 0; i < n; ++i)
        {
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;

            float dL = readFrac (bufL, delaySamples);
            float dR = readFrac (bufR, delayR);

            // tone on wet before feedback
            lpL += tone * (dL - lpL);
            lpR += tone * (dR - lpR);
            dL = lpL;
            dR = lpR;

            buffer.setSample (0, i, inL * (1.f - mix) + dL * mix);
            if (ch > 1) buffer.setSample (1, i, inR * (1.f - mix) + dR * mix);

            // cross-feed slight ping-pong character at high FB
            float cross = feedback * 0.15f;
            bufL[(size_t) writePos] = inL + dL * (feedback - cross) + dR * cross;
            bufR[(size_t) writePos] = inR + dR * (feedback - cross) + dL * cross;

            writePos = (writePos + 1) % bs;
        }
    }

private:
    float readFrac (const std::vector<float>& buf, float delaySamps) const noexcept
    {
        const int bs = (int) buf.size();
        float rp = (float) writePos - delaySamps;
        while (rp < 0.f) rp += (float) bs;
        int i0 = ((int) rp) % bs;
        int i1 = (i0 + 1) % bs;
        float f = rp - std::floor (rp);
        // hermite-ish soft
        return buf[(size_t) i0] * (1.f - f) + buf[(size_t) i1] * f;
    }

    double sr = 44100.0;
    std::vector<float> bufL, bufR;
    int writePos = 0;
    float delaySamples = 300.f, targetDelay = 300.f;
    float feedback = 0.3f, mix = 0.f;
    float lpL = 0.f, lpR = 0.f;
};
}

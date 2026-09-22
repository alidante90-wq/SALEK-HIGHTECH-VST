#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>
namespace salek {
/** Stereo / Ping-Pong / Mono delay with independent L/R times. */
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
        targetDelayL = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
        targetDelayR = targetDelayL * 1.07f;
    }
    void setTimeMsL (float ms) noexcept
    {
        targetDelayL = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
    }
    void setTimeMsR (float ms) noexcept
    {
        targetDelayR = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
    }
    void setFeedback (float fb) noexcept { feedback = juce::jlimit (0.f, 0.95f, fb); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setWow (float w) noexcept { wowAmt = juce::jlimit (0.f, 1.f, w); }
    /** 0 Stereo, 1 PingPong, 2 Mono, 3 MultiTap */
    void setMode (int m) noexcept { mode = juce::jlimit (0, 3, m); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1e-4f) return;
        const float wet = std::sqrt (mix); // more audible at mid settings
        const float dry = 1.f - wet * 0.85f;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int bs = (int) bufL.size();
        if (bs < 8) return;

        const float smooth = 0.002f;
        delaySamplesL += smooth * (targetDelayL - delaySamplesL);
        delaySamplesR += smooth * (targetDelayR - delaySamplesR);

        const float tone = juce::jlimit (0.25f, 0.9f, 0.82f - delaySamplesL / float (sr) * 0.3f);

        for (int i = 0; i < n; ++i)
        {
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;
            if (mode == 2) // Mono: average input
            {
                const float m = 0.5f * (inL + inR);
                inL = inR = m;
            }

            wowPhase += 0.0007f;
            const float wow = 1.f + wowAmt * 0.015f * std::sin (wowPhase * 6.283185f);
            float dL, dR;
            if (mode == 3) // Multi-tap: 4 taps on L/R spread
            {
                dL = 0.f; dR = 0.f;
                const float taps[4] = { 1.f, 1.37f, 1.71f, 2.15f };
                const float gains[4] = { 1.f, 0.7f, 0.5f, 0.35f };
                for (int t = 0; t < 4; ++t)
                {
                    dL += readFrac (bufL, delaySamplesL * taps[t] * wow) * gains[t];
                    dR += readFrac (bufR, delaySamplesR * taps[t] * (2.f - wow)) * gains[t];
                }
                dL *= 0.55f; dR *= 0.55f;
            }
            else
            {
                dL = readFrac (bufL, delaySamplesL * wow);
                dR = readFrac (bufR, delaySamplesR * (2.f - wow));
            }

            lpL += tone * (dL - lpL);
            lpR += tone * (dR - lpR);
            dL = lpL;
            dR = lpR;

            buffer.setSample (0, i, inL * dry + dL * wet);
            if (ch > 1) buffer.setSample (1, i, inR * dry + dR * wet);

            if (mode == 1) // PingPong: cross feedback
            {
                bufL[(size_t) writePos] = inL + dR * feedback;
                bufR[(size_t) writePos] = inR + dL * feedback;
            }
            else if (mode == 2) // Mono: shared
            {
                const float monoIn = 0.5f * (inL + inR);
                const float monoD = 0.5f * (dL + dR);
                bufL[(size_t) writePos] = monoIn + monoD * feedback;
                bufR[(size_t) writePos] = bufL[(size_t) writePos];
            }
            else // Stereo: light cross
            {
                const float cross = feedback * 0.12f;
                bufL[(size_t) writePos] = inL + dL * (feedback - cross) + dR * cross;
                bufR[(size_t) writePos] = inR + dR * (feedback - cross) + dL * cross;
            }

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
        return buf[(size_t) i0] * (1.f - f) + buf[(size_t) i1] * f;
    }

    double sr = 44100.0;
    std::vector<float> bufL, bufR;
    int writePos = 0;
    float delaySamplesL = 300.f, delaySamplesR = 320.f;
    float targetDelayL = 300.f, targetDelayR = 320.f;
    float feedback = 0.3f, mix = 0.f;
    float wowAmt = 0.12f, wowPhase = 0.f;
    float lpL = 0.f, lpR = 0.f;
    int mode = 0;
};
}

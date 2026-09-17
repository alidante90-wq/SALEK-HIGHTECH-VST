#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace salek
{
/** Multi-model reverb with working SIZE (scales comb lengths) + damping.
 *  Modes: 0 Room 1 Hall 2 Plate 3 Chamber 4 Spring */
class SimpleReverb
{
public:
    void prepare (double sampleRate, int)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        // allocate max hall-sized combs once
        for (int i = 0; i < 4; ++i)
        {
            const int maxLen = juce::jmax (64, (int) (0.12 * sr)); // up to ~120ms
            combL[i].assign ((size_t) maxLen, 0.f);
            combR[i].assign ((size_t) maxLen, 0.f);
            combPos[i] = 0;
            combLen[i] = maxLen / 2;
        }
        for (int i = 0; i < 2; ++i)
        {
            const int maxLen = juce::jmax (16, (int) (0.02 * sr));
            apL[i].assign ((size_t) maxLen, 0.f);
            apR[i].assign ((size_t) maxLen, 0.f);
            apPos[i] = 0;
            apLen[i] = maxLen / 2;
        }
        preDelay.assign ((size_t) juce::jmax (64, (int) (0.08 * sr)), 0.f);
        prePos = 0;
        updateLengths();
        dampStateL.fill (0.f); dampStateR.fill (0.f);
    }

    void setSize (float s) noexcept
    {
        size = juce::jlimit (0.0f, 1.0f, s);
        updateLengths();
    }
    void setDecay (float d) noexcept { decay = juce::jlimit (0.05f, 0.98f, d); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); }
    void setDamping (float d) noexcept { damping = juce::jlimit (0.0f, 1.0f, d); }
    void setMode (int m) noexcept
    {
        const int nm = juce::jlimit (0, 4, m);
        if (nm != mode) { mode = nm; updateLengths(); }
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();

        // feedback strongly follows decay; size already set lengths
        float fbBase = 0.52f;
        if (mode == 1) fbBase = 0.58f;      // hall longer
        if (mode == 2) fbBase = 0.50f;      // plate
        if (mode == 3) fbBase = 0.48f;      // chamber
        if (mode == 4) fbBase = 0.60f;      // spring
        const float fb = juce::jlimit (0.1f, 0.96f, fbBase + decay * 0.42f);
        const float dampCoeff = 0.15f + damping * 0.75f; // higher = darker

        // pre-delay samples from size (small rooms less pre)
        const int preSamps = juce::jlimit (0, (int) preDelay.size() - 1,
                                           (int) ((0.005f + size * 0.055f) * (float) sr));

        for (int i = 0; i < n; ++i)
        {
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;

            // predelay write/read
            preDelay[(size_t) prePos] = 0.5f * (inL + inR);
            int rp = prePos - preSamps;
            if (rp < 0) rp += (int) preDelay.size();
            float pre = preDelay[(size_t) rp];
            prePos = (prePos + 1) % (int) preDelay.size();

            float wetL = 0.f, wetR = 0.f;
            for (int c = 0; c < 4; ++c)
            {
                auto& bL = combL[c];
                auto& bR = combR[c];
                const int len = juce::jmax (8, combLen[c]);
                if ((int) bL.size() < len) continue;
                int p = combPos[c] % len;
                float oL = bL[(size_t) p];
                float oR = bR[(size_t) p];
                // per-comb damping (one-pole)
                dampStateL[c] += dampCoeff * (oL - dampStateL[c]);
                dampStateR[c] += dampCoeff * (oR - dampStateR[c]);
                float fbL = dampStateL[c] * fb;
                float fbR = dampStateR[c] * fb;
                bL[(size_t) p] = pre + inL * 0.35f + fbL;
                bR[(size_t) p] = pre + inR * 0.35f + fbR;
                combPos[c] = (p + 1) % len;
                // stereo spread by size
                float spread = 0.15f + size * 0.35f;
                wetL += oL * (1.f - spread * 0.3f) + oR * spread * 0.3f;
                wetR += oR * (1.f - spread * 0.3f) + oL * spread * 0.3f;
            }
            wetL *= 0.22f;
            wetR *= 0.22f;

            for (int a = 0; a < 2; ++a)
            {
                if (apL[a].empty()) continue;
                const int len = juce::jmax (4, apLen[a]);
                auto processAP = [&] (std::vector<float>& buf, int& pos, float x) -> float
                {
                    int p = pos % len;
                    float y = buf[(size_t) p];
                    float z = x + y * 0.5f;
                    buf[(size_t) p] = z;
                    pos = (p + 1) % len;
                    return y - z * 0.5f;
                };
                wetL = processAP (apL[a], apPos[a], wetL);
                wetR = processAP (apR[a], apPos[a], wetR);
            }

            if (mode == 4) // spring metal
            {
                wetL = std::tanh (wetL * 1.2f);
                wetR = std::tanh (wetR * 1.2f);
            }

            buffer.setSample (0, i, inL * (1.f - mix) + wetL * mix);
            if (ch > 1) buffer.setSample (1, i, inR * (1.f - mix) + wetR * mix);
        }
    }

private:
    void updateLengths()
    {
        // base ms per mode
        static constexpr float roomC[4]    = { 29.7f, 37.1f, 41.1f, 43.7f };
        static constexpr float hallC[4]    = { 41.2f, 53.8f, 67.4f, 79.1f };
        static constexpr float plateC[4]   = { 17.1f, 23.3f, 31.7f, 39.5f };
        static constexpr float chamberC[4] = { 22.4f, 28.9f, 34.2f, 40.1f };
        static constexpr float springC[4]  = { 14.3f, 19.7f, 26.5f, 33.1f };
        static constexpr float roomA[2]    = { 5.0f, 1.7f };
        static constexpr float hallA[2]    = { 8.3f, 3.1f };
        static constexpr float plateA[2]   = { 3.2f, 1.1f };
        static constexpr float chamberA[2] = { 4.1f, 1.9f };
        static constexpr float springA[2]  = { 2.4f, 0.9f };
        const float* cm = roomC; const float* am = roomA;
        if (mode == 1) { cm = hallC; am = hallA; }
        else if (mode == 2) { cm = plateC; am = plateA; }
        else if (mode == 3) { cm = chamberC; am = chamberA; }
        else if (mode == 4) { cm = springC; am = springA; }

        // SIZE scales 0.45x .. 1.8x of base — clearly audible
        const float scale = 0.45f + size * 1.35f;
        for (int i = 0; i < 4; ++i)
        {
            int len = juce::jmax (16, (int) (cm[i] * 0.001f * scale * (float) sr));
            len = juce::jmin (len, (int) combL[i].size());
            combLen[i] = len;
        }
        for (int i = 0; i < 2; ++i)
        {
            int len = juce::jmax (8, (int) (am[i] * 0.001f * scale * (float) sr));
            len = juce::jmin (len, (int) apL[i].size());
            apLen[i] = len;
        }
    }

    double sr = 44100.0;
    float size = 0.5f, decay = 0.5f, mix = 0.0f, damping = 0.35f;
    int mode = 0;
    std::array<std::vector<float>, 4> combL, combR;
    std::array<std::vector<float>, 2> apL, apR;
    std::array<int, 4> combPos {}, combLen {};
    std::array<int, 2> apPos {}, apLen {};
    std::vector<float> preDelay;
    int prePos = 0;
    std::array<float, 4> dampStateL {}, dampStateR {};
};
}

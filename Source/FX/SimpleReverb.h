#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace salek
{
/** Schroeder–Moorer style reverb (Freeverb family).
 *  SIZE scales comb/AP delay lengths (audible room scale).
 *  DECAY maps to feedback (RT60-ish). Damping = HF absorption.
 *  Modes change base delay sets + feedback character. */
class SimpleReverb
{
public:
    static constexpr int kCombs = 8;
    static constexpr int kAPs   = 4;

    void prepare (double sampleRate, int)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        // max ~150 ms comb @ any size scale
        const int maxComb = juce::jmax (128, (int) (0.15 * sr));
        const int maxAP   = juce::jmax (32,  (int) (0.025 * sr));
        for (int i = 0; i < kCombs; ++i)
        {
            combL[i].assign ((size_t) maxComb, 0.f);
            combR[i].assign ((size_t) maxComb, 0.f);
            combPos[i] = 0;
            combLen[i] = maxComb / 2;
            dampL[i] = dampR[i] = 0.f;
        }
        for (int i = 0; i < kAPs; ++i)
        {
            apL[i].assign ((size_t) maxAP, 0.f);
            apR[i].assign ((size_t) maxAP, 0.f);
            apPosL[i] = apPosR[i] = 0;
            apLen[i] = maxAP / 2;
        }
        // early reflections taps
        earlyBuf.assign ((size_t) juce::jmax (256, (int) (0.1 * sr)), 0.f);
        earlyPos = 0;
        updateLengths();
    }

    void setSize (float s) noexcept { size = juce::jlimit (0.f, 1.f, s); updateLengths(); }
    void setDecay (float d) noexcept { decay = juce::jlimit (0.05f, 0.98f, d); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setDamping (float d) noexcept { damping = juce::jlimit (0.f, 1.f, d); }
    void setMode (int m) noexcept
    {
        int nm = juce::jlimit (0, 4, m);
        if (nm != mode) { mode = nm; updateLengths(); }
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();

        // RT-ish feedback: decay 0→0.98 maps to stable fb
        float fbBase = 0.55f;
        if (mode == 1) fbBase = 0.62f; // Hall
        if (mode == 2) fbBase = 0.50f; // Plate
        if (mode == 3) fbBase = 0.48f; // Chamber
        if (mode == 4) fbBase = 0.58f; // Spring
        const float fb = juce::jlimit (0.15f, 0.97f, fbBase * (0.35f + decay * 0.75f));

        // damping coeff: 0 = bright, 1 = dark (more LP in feedback)
        const float dampAmt = 0.1f + damping * 0.85f;

        // early reflection gain
        const float earlyG = 0.35f + size * 0.25f;

        for (int i = 0; i < n; ++i)
        {
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;
            float mono = 0.5f * (inL + inR);

            // --- early reflections (4 taps, size-scaled) ---
            earlyBuf[(size_t) earlyPos] = mono;
            float early = 0.f;
            static const float earlyMs[4] = { 12.f, 21.f, 35.f, 48.f };
            static const float earlyW[4]  = { 0.5f, 0.35f, 0.25f, 0.15f };
            for (int t = 0; t < 4; ++t)
            {
                int off = (int) (earlyMs[t] * (0.5f + size) * 0.001f * (float) sr);
                off = juce::jlimit (1, (int) earlyBuf.size() - 1, off);
                int rp = earlyPos - off;
                if (rp < 0) rp += (int) earlyBuf.size();
                early += earlyBuf[(size_t) rp] * earlyW[t];
            }
            earlyPos = (earlyPos + 1) % (int) earlyBuf.size();

            // --- parallel combs (diffusion body) ---
            float wetL = 0.f, wetR = 0.f;
            for (int c = 0; c < kCombs; ++c)
            {
                const int len = juce::jmax (16, combLen[c]);
                int p = combPos[c] % len;
                float oL = combL[c][(size_t) p];
                float oR = combR[c][(size_t) p];

                // HF damping in loop
                dampL[c] += dampAmt * (oL - dampL[c]);
                dampR[c] += dampAmt * (oR - dampR[c]);

                // slight L/R detune of input for width
                float injL = mono + inL * 0.25f + early * earlyG * ((c & 1) ? 0.6f : 1.f);
                float injR = mono + inR * 0.25f + early * earlyG * ((c & 1) ? 1.f : 0.6f);
                combL[c][(size_t) p] = injL + dampL[c] * fb;
                combR[c][(size_t) p] = injR + dampR[c] * fb;
                combPos[c] = (p + 1) % len;

                wetL += oL;
                wetR += oR;
            }
            wetL *= (1.f / (float) kCombs);
            wetR *= (1.f / (float) kCombs);

            // --- series all-passes (smear / density) ---
            for (int a = 0; a < kAPs; ++a)
            {
                const int len = juce::jmax (8, apLen[a]);
                const float g = (mode == 4) ? 0.6f : 0.5f; // spring more metallic
                auto ap = [&] (std::vector<float>& buf, int& pos, float x) -> float
                {
                    int p = pos % len;
                    float bufOut = buf[(size_t) p];
                    float in = x + bufOut * g;
                    buf[(size_t) p] = in;
                    pos = (p + 1) % len;
                    return bufOut - in * g;
                };
                wetL = ap (apL[a], apPosL[a], wetL);
                wetR = ap (apR[a], apPosR[a], wetR);
            }

            // mode coloration
            if (mode == 2) // plate: brighter, slight tanh
            {
                wetL = std::tanh (wetL * 1.15f);
                wetR = std::tanh (wetR * 1.15f);
            }
            else if (mode == 4) // spring: resonant grit
            {
                wetL = std::tanh (wetL * 1.35f);
                wetR = std::tanh (wetR * 1.35f);
            }

            // stereo width from size
            float w = 0.2f + size * 0.45f;
            float mid = 0.5f * (wetL + wetR);
            float side = 0.5f * (wetL - wetR) * (1.f + w);
            wetL = mid + side;
            wetR = mid - side;

            // Blend early + body; slight wet boost so mix knob is more audible
            const float wetBoost = 1.15f;
            wetL = (wetL + early * earlyG) * wetBoost;
            wetR = (wetR + early * earlyG * 0.92f) * wetBoost;
            buffer.setSample (0, i, inL * (1.f - mix) + wetL * mix);
            if (ch > 1) buffer.setSample (1, i, inR * (1.f - mix) + wetR * mix);
        }
    }

private:
    void updateLengths()
    {
        // Prime-ish base delays (ms) — classic Freeverb-style ratios
        // Mode selects which table; SIZE scales 0.5x..2.0x
        static constexpr float combMs[5][kCombs] = {
            // Room
            { 25.3f, 26.9f, 28.9f, 30.7f, 32.3f, 34.1f, 36.7f, 38.9f },
            // Hall
            { 36.1f, 39.3f, 42.7f, 46.1f, 49.9f, 53.3f, 57.1f, 61.7f },
            // Plate
            { 18.1f, 20.3f, 22.7f, 25.1f, 27.9f, 30.3f, 33.1f, 36.7f },
            // Chamber
            { 22.1f, 24.7f, 27.3f, 29.9f, 32.9f, 35.7f, 39.1f, 42.3f },
            // Spring
            { 14.3f, 16.7f, 19.1f, 21.9f, 24.7f, 28.1f, 31.9f, 35.3f },
        };
        static constexpr float apMs[5][kAPs] = {
            { 5.0f, 1.7f, 12.5f, 3.1f },
            { 8.3f, 3.1f, 15.1f, 4.7f },
            { 3.2f, 1.1f, 9.3f,  2.3f },
            { 4.1f, 1.9f, 11.1f, 2.9f },
            { 2.4f, 0.9f, 7.7f,  1.5f },
        };

        const float scale = 0.50f + size * 1.50f; // 0.5 .. 2.0
        for (int i = 0; i < kCombs; ++i)
        {
            int len = juce::jmax (16, (int) (combMs[mode][i] * 0.001f * scale * (float) sr));
            combLen[i] = juce::jmin (len, (int) combL[i].size());
        }
        for (int i = 0; i < kAPs; ++i)
        {
            int len = juce::jmax (8, (int) (apMs[mode][i] * 0.001f * scale * (float) sr));
            apLen[i] = juce::jmin (len, (int) apL[i].size());
        }
    }

    double sr = 44100.0;
    float size = 0.5f, decay = 0.55f, mix = 0.f, damping = 0.35f;
    int mode = 0;

    std::array<std::vector<float>, kCombs> combL, combR;
    std::array<std::vector<float>, kAPs>   apL, apR;
    std::array<int, kCombs> combPos {}, combLen {};
    std::array<int, kAPs>   apPosL {}, apPosR {}, apLen {};
    std::array<float, kCombs> dampL {}, dampR {};

    std::vector<float> earlyBuf;
    int earlyPos = 0;
};
}

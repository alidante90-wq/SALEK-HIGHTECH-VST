#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace salek
{
/** Multi-model reverb: 0 Room 1 Hall 2 Plate 3 Chamber 4 Spring */
class SimpleReverb
{
public:
    void prepare (double sampleRate, int)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        rebuild (0);
    }
    void setSize (float s) noexcept { size = juce::jlimit (0.0f, 1.0f, s); }
    void setDecay (float d) noexcept { decay = juce::jlimit (0.1f, 0.95f, d); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); }
    void setMode (int m) noexcept
    {
        const int nm = juce::jlimit (0, 4, m);
        if (nm != mode) { mode = nm; rebuild (mode); }
    }
    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        // spring: more metallic feedback; chamber: tighter
        float fbBase = 0.55f;
        if (mode == 4) fbBase = 0.62f;
        if (mode == 3) fbBase = 0.48f;
        const float fb = fbBase + decay * 0.4f * (0.5f + 0.5f * size);
        for (int i = 0; i < n; ++i) {
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;
            float wetL = 0.0f, wetR = 0.0f;
            for (int c = 0; c < 4; ++c) {
                auto& bL = combL[c]; auto& bR = combR[c];
                if (bL.empty()) continue;
                int p = combPos[c];
                float oL = bL[(size_t) p]; float oR = bR[(size_t) p];
                bL[(size_t) p] = inL + oL * fb;
                bR[(size_t) p] = inR + oR * fb;
                combPos[c] = (p + 1) % (int) bL.size();
                wetL += oL; wetR += oR;
            }
            wetL *= 0.25f; wetR *= 0.25f;
            for (int a = 0; a < 2; ++a) {
                if (apL[a].empty()) continue;
                auto processAP = [](std::vector<float>& buf, int& pos, float x) {
                    float y = buf[(size_t) pos];
                    float z = x + y * 0.5f;
                    buf[(size_t) pos] = z;
                    pos = (pos + 1) % (int) buf.size();
                    return y - z * 0.5f;
                };
                wetL = processAP (apL[a], apPos[a], wetL);
                wetR = processAP (apR[a], apPos[a], wetR);
            }
            // spring coloration
            if (mode == 4)
            {
                wetL = std::tanh (wetL * 1.15f);
                wetR = std::tanh (wetR * 1.15f);
            }
            buffer.setSample (0, i, inL * (1.0f - mix) + wetL * mix);
            if (ch > 1) buffer.setSample (1, i, inR * (1.0f - mix) + wetR * mix);
        }
    }
private:
    void rebuild (int m)
    {
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
        if (m == 1) { cm = hallC; am = hallA; }
        else if (m == 2) { cm = plateC; am = plateA; }
        else if (m == 3) { cm = chamberC; am = chamberA; }
        else if (m == 4) { cm = springC; am = springA; }
        for (int i = 0; i < 4; ++i) {
            int len = juce::jmax (16, (int) (cm[i] * 0.001 * sr));
            combL[i].assign ((size_t) len, 0.0f);
            combR[i].assign ((size_t) len, 0.0f);
            combPos[i] = 0;
        }
        for (int i = 0; i < 2; ++i) {
            int len = juce::jmax (8, (int) (am[i] * 0.001 * sr));
            apL[i].assign ((size_t) len, 0.0f);
            apR[i].assign ((size_t) len, 0.0f);
            apPos[i] = 0;
        }
    }
    double sr = 44100.0;
    float size = 0.5f, decay = 0.5f, mix = 0.0f;
    int mode = 0;
    std::array<std::vector<float>, 4> combL, combR;
    std::array<std::vector<float>, 2> apL, apR;
    std::array<int, 4> combPos {};
    std::array<int, 2> apPos {};
};
}

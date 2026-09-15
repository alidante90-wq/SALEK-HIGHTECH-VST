#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace salek
{
/** Multi-model reverb: 0=Room 1=Hall 2=Plate */
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
        const int nm = juce::jlimit (0, 2, m);
        if (nm != mode) { mode = nm; rebuild (mode); }
    }
    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const float fb = 0.55f + decay * 0.4f * (0.5f + 0.5f * size);
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
            buffer.setSample (0, i, inL * (1.0f - mix) + wetL * mix);
            if (ch > 1) buffer.setSample (1, i, inR * (1.0f - mix) + wetR * mix);
        }
    }
private:
    void rebuild (int m)
    {
        // Room / Hall / Plate delay sets (ms)
        static constexpr float roomC[4]  = { 29.7f, 37.1f, 41.1f, 43.7f };
        static constexpr float hallC[4]  = { 41.2f, 53.8f, 67.4f, 79.1f };
        static constexpr float plateC[4] = { 17.1f, 23.3f, 31.7f, 39.5f };
        static constexpr float roomA[2]  = { 5.0f, 1.7f };
        static constexpr float hallA[2]  = { 8.3f, 3.1f };
        static constexpr float plateA[2] = { 3.2f, 1.1f };
        const float* cm = (m == 1) ? hallC : (m == 2) ? plateC : roomC;
        const float* am = (m == 1) ? hallA : (m == 2) ? plateA : roomA;
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

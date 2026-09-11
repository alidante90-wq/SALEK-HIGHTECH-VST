#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace salek
{
class SimpleReverb
{
public:
    void prepare (double sampleRate, int)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        for (int i = 0; i < 4; ++i) {
            int len = (int) (combMs[i] * 0.001 * sr);
            if (len < 16) len = 16;
            combL[i].assign ((size_t) len, 0.0f);
            combR[i].assign ((size_t) len, 0.0f);
            combPos[i] = 0;
        }
        for (int i = 0; i < 2; ++i) {
            int len = (int) (apMs[i] * 0.001 * sr);
            if (len < 8) len = 8;
            apL[i].assign ((size_t) len, 0.0f);
            apR[i].assign ((size_t) len, 0.0f);
            apPos[i] = 0;
        }
    }
    void setSize (float s) noexcept { size = juce::jlimit (0.0f, 1.0f, s); }
    void setDecay (float d) noexcept { decay = juce::jlimit (0.1f, 0.95f, d); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); }
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
                int p = combPos[c];
                float oL = bL[(size_t) p]; float oR = bR[(size_t) p];
                bL[(size_t) p] = inL + oL * fb;
                bR[(size_t) p] = inR + oR * fb;
                combPos[c] = (p + 1) % (int) bL.size();
                wetL += oL; wetR += oR;
            }
            wetL *= 0.25f; wetR *= 0.25f;
            for (int a = 0; a < 2; ++a) {
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
    double sr = 44100.0;
    float size = 0.5f, decay = 0.5f, mix = 0.0f;
    static constexpr float combMs[4] = { 29.7f, 37.1f, 41.1f, 43.7f };
    static constexpr float apMs[2] = { 5.0f, 1.7f };
    std::array<std::vector<float>, 4> combL, combR;
    std::array<std::vector<float>, 2> apL, apR;
    std::array<int, 4> combPos {};
    std::array<int, 2> apPos {};
};
}

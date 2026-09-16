#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <vector>

namespace salek
{
class SimpleChorus
{
public:
    void prepare (double sampleRate, int)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        const int size = (int) (sr * 0.05) + 8;
        delayL.assign ((size_t) size, 0.0f);
        delayR.assign ((size_t) size, 0.0f);
        writePos = 0; phase = 0.0f;
    }
    void setRate (float hz) noexcept { rate = juce::jlimit (0.05f, 5.0f, hz); }
    void setDepth (float d) noexcept { depth = juce::jlimit (0.0f, 1.0f, d); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); }
    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f || delayL.empty()) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int size = (int) delayL.size();
        const float maxDelay = (float) size * 0.45f;
        const float baseDelay = maxDelay * 0.35f;
        for (int i = 0; i < n; ++i)
        {
            phase += rate / (float) sr;
            if (phase >= 1.0f) phase -= 1.0f;
            const float mod = std::sin (phase * juce::MathConstants<float>::twoPi);
            const float dL = baseDelay + mod * depth * maxDelay * 0.4f;
            const float dR = baseDelay - mod * depth * maxDelay * 0.4f;
            auto read = [&](std::vector<float>& buf, float delaySamples) -> float {
                float rp = (float) writePos - delaySamples;
                while (rp < 0.0f) rp += (float) size;
                int i0 = ((int) rp) % size;
                int i1 = (i0 + 1) % size;
                float f = rp - std::floor (rp);
                return buf[(size_t) i0] * (1.0f - f) + buf[(size_t) i1] * f;
            };
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;
            delayL[(size_t) writePos] = inL;
            delayR[(size_t) writePos] = inR;
            float wetL = read (delayL, dL);
            float wetR = read (delayR, dR);
            buffer.setSample (0, i, inL * (1.0f - mix) + wetL * mix);
            if (ch > 1) buffer.setSample (1, i, inR * (1.0f - mix) + wetR * mix);
            writePos = (writePos + 1) % size;
        }
    }
private:
    double sr = 44100.0;
    float rate = 0.35f, depth = 0.5f, mix = 0.0f, phase = 0.0f;
    std::vector<float> delayL, delayR;
    int writePos = 0;
};
}

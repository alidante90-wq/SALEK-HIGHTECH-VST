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
        mixSmooth.reset (sr, 0.008); rateSmooth.reset (sr, 0.008); depthSmooth.reset (sr, 0.008);
        mixSmooth.setCurrentAndTargetValue (0.f); rateSmooth.setCurrentAndTargetValue (rate); depthSmooth.setCurrentAndTargetValue (depth);
    }
    void setRate (float hz) noexcept { rate = juce::jlimit (0.05f, 5.0f, hz); rateSmooth.setTargetValue (rate); }
    void setDepth (float d) noexcept { depth = juce::jlimit (0.0f, 1.0f, d); depthSmooth.setTargetValue (depth); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); mixSmooth.setTargetValue (mix); }
    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mixSmooth.getTargetValue() < 1e-4f && mixSmooth.getCurrentValue() < 1e-4f || delayL.empty()) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int size = (int) delayL.size();
        const float maxDelay = (float) size * 0.45f;
        const float baseDelay = maxDelay * 0.35f;
        for (int i = 0; i < n; ++i)
        {
            const float mixNow = mixSmooth.getNextValue();
            const float rateNow = rateSmooth.getNextValue();
            const float depthNow = depthSmooth.getNextValue();
            phase += rateNow / (float) sr;
            if (phase >= 1.0f) phase -= 1.0f;
            const float mod = std::sin (phase * juce::MathConstants<float>::twoPi);
            const float dL = baseDelay + mod * depthNow * maxDelay * 0.4f;
            const float dR = baseDelay - mod * depthNow * maxDelay * 0.4f;
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
            // second voice (BBD-ish offset)
            const float mod2 = std::sin ((phase * 0.73f + 0.25f) * juce::MathConstants<float>::twoPi);
            const float dL2 = baseDelay * 1.35f + mod2 * depthNow * maxDelay * 0.25f;
            const float dR2 = baseDelay * 1.35f - mod2 * depthNow * maxDelay * 0.25f;
            float wetL = read (delayL, dL) * 0.65f + read (delayL, dL2) * 0.35f;
            float wetR = read (delayR, dR) * 0.65f + read (delayR, dR2) * 0.35f;
            // analog soft sat + slight HF roll on wet
            wetL = std::tanh (wetL * 1.2f);
            wetR = std::tanh (wetR * 1.2f);
            buffer.setSample (0, i, inL * (1.0f - mixNow) + wetL * mixNow);
            if (ch > 1) buffer.setSample (1, i, inR * (1.0f - mixNow) + wetR * mixNow);
            writePos = (writePos + 1) % size;
        }
    }
private:
    double sr = 44100.0;
    float rate = 0.35f, depth = 0.5f, mix = 0.0f, phase = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmooth, rateSmooth, depthSmooth;
    std::vector<float> delayL, delayR;
    int writePos = 0;
};
}

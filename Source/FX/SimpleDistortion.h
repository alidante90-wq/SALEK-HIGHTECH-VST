#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class SimpleDistortion {
public:
    void prepare (double, int) {}
    void setDrive (float d) noexcept { drive = juce::jlimit (0.f, 1.f, d); }
    void setBitcrush (float b) noexcept { bits = juce::jlimit (0.f, 1.f, b); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void process (juce::AudioBuffer<float>& buf) noexcept {
        if (mix < 1e-4f && drive < 1e-4f && bits < 1e-4f) return;
        const int n = buf.getNumSamples();
        const int ch = buf.getNumChannels();
        const float gain = 1.f + drive * 12.f;
        const float levels = bits > 0.01f ? std::pow (2.f, 4.f + (1.f - bits) * 12.f) : 0.f;
        for (int c = 0; c < ch; ++c) {
            float* d = buf.getWritePointer (c);
            for (int i = 0; i < n; ++i) {
                float x = d[i];
                float y = x * gain;
                y = std::tanh (y);
                if (levels > 1.f)
                    y = std::floor (y * levels + 0.5f) / levels;
                d[i] = x * (1.f - mix) + y * mix;
            }
        }
    }
private:
    float drive = 0.f, bits = 0.f, mix = 0.f;
};
}

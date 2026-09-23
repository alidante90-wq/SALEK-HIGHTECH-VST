#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class SimplePhaser {
public:
    void prepare (double sampleRate, int) {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        for (auto& s : stages) s = 0.f;
        lfoPhase = 0.f;
        mixSmooth.reset (sr, 0.02);
        mixSmooth.setCurrentAndTargetValue (0.f);
    }
    void setRate (float r) noexcept { rate = juce::jlimit (0.05f, 8.f, r); }
    void setDepth (float d) noexcept { depth = juce::jlimit (0.f, 1.f, d); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); mixSmooth.setTargetValue (mix); }
    void process (juce::AudioBuffer<float>& buf) noexcept {
        if (mix < 1e-4f && mixSmooth.getCurrentValue() < 1e-4f) return;
        const int n = buf.getNumSamples();
        const int ch = juce::jmin (2, buf.getNumChannels());
        const float inc = float (juce::MathConstants<double>::twoPi * rate / sr);
        for (int i = 0; i < n; ++i) {
            const float mixNow = mixSmooth.getNextValue();
            float lfo = 0.5f + 0.5f * std::sin (lfoPhase);
            lfoPhase += inc;
            if (lfoPhase > juce::MathConstants<float>::twoPi) lfoPhase -= juce::MathConstants<float>::twoPi;
            float fc = 200.f + depth * 1800.f * lfo;
            float tanw = std::tan (juce::MathConstants<float>::pi * fc / float (sr));
            float a = (tanw - 1.f) / (tanw + 1.f);
            for (int c = 0; c < ch; ++c) {
                float* d = buf.getWritePointer (c);
                float x = d[i];
                float y = x;
                for (int s = 0; s < 4; ++s) {
                    float out = a * y + stages[size_t (s * 2 + c)];
                    stages[size_t (s * 2 + c)] = y - a * out;
                    y = out;
                }
                d[i] = x * (1.f - mixNow) + y * mixNow;
            }
        }
    }
private:
    double sr = 44100.0;
    float rate = 0.4f, depth = 0.6f, mix = 0.f, lfoPhase = 0.f;
    juce::SmoothedValue<float> mixSmooth { 0.f };
    float stages[8] {};
};
}

#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class SimpleCompressor {
public:
    void prepare (double sr, int) { sampleRate = sr; env = 0.f; }
    void setThresholdDb (float db) { threshold = juce::Decibels::decibelsToGain (db); }
    void setRatio (float r) { ratio = juce::jmax (1.0f, r); }
    void setMix (float m) { mix = juce::jlimit (0.f, 1.f, m); }
    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f) return;
        const float atk = 1.0f - std::exp (-1.0f / (0.005f * (float) sampleRate));
        const float rel = 1.0f - std::exp (-1.0f / (0.080f * (float) sampleRate));
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float peak = 0.f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                peak = juce::jmax (peak, std::abs (buffer.getSample (ch, i)));
            if (peak > env) env += atk * (peak - env);
            else            env += rel * (peak - env);
            float gain = 1.0f;
            if (env > threshold && threshold > 1e-6f)
            {
                float over = env / threshold;
                gain = std::pow (over, 1.0f / ratio - 1.0f);
            }
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                float dry = buffer.getSample (ch, i);
                buffer.setSample (ch, i, dry * (1.0f - mix) + dry * gain * mix);
            }
        }
    }
private:
    double sampleRate = 44100.0;
    float threshold = 0.5f, ratio = 4.0f, mix = 0.0f, env = 0.0f;
};
}

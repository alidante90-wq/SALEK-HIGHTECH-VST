#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class SimpleEQ {
public:
    void prepare (double sr, int) { sampleRate = sr; update(); }
    void setLowGainDb (float db)  { lowDb = db;  update(); }
    void setMidGainDb (float db)  { midDb = db;  update(); }
    void setHighGainDb (float db) { highDb = db; update(); }
    void process (juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            float& z1 = z1L[ch > 0 ? 1 : 0];
            float& z2 = z2L[ch > 0 ? 1 : 0];
            float& z3 = z3L[ch > 0 ? 1 : 0];
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = d[i];
                z1 += lowA * (x - z1);
                float low = z1 * lowG + x * (1.f - lowMix);
                float hp = x - z2; z2 += highA * (x - z2);
                float high = hp * highG + x * (1.f - highMix);
                float mid = x + (x - z3) * midG; z3 += 0.15f * (x - z3);
                d[i] = 0.34f * low + 0.33f * mid + 0.33f * high;
            }
        }
    }
private:
    void update()
    {
        lowG  = juce::Decibels::decibelsToGain (lowDb);
        midG  = juce::jmap (midDb, -12.f, 12.f, -0.5f, 0.5f);
        highG = juce::Decibels::decibelsToGain (highDb);
        lowMix = juce::jlimit (0.f, 1.f, std::abs (lowDb) / 12.f);
        highMix = juce::jlimit (0.f, 1.f, std::abs (highDb) / 12.f);
        lowA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 250.0f / (float) sampleRate);
        highA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 4000.0f / (float) sampleRate);
    }
    double sampleRate = 44100.0;
    float lowDb = 0, midDb = 0, highDb = 0;
    float lowG = 1, midG = 0, highG = 1, lowMix = 0, highMix = 0, lowA = 0.1f, highA = 0.1f;
    float z1L[2] {}, z2L[2] {}, z3L[2] {};
};
}

#pragma once
#include <JuceHeader.h>
#include <cmath>

namespace salek
{
// Musical three-band EQ: low/high shelves with a broad peaking mid band.
// Coefficients are recalculated at block boundaries; sample processing is allocation-free.
class SimpleEQ
{
public:
    void prepare (double sr, int) { sampleRate = sr > 1000.0 ? sr : 44100.0; reset(); update(); }
    void reset() noexcept
    {
        lowState[0] = lowState[1] = highState[0] = highState[1] = 0.0f;
        z1[0] = z1[1] = z2[0] = z2[1] = 0.0f;
    }
    void setLowGainDb (float db)  { lowDb = juce::jlimit (-12.0f, 12.0f, db); update(); }
    void setMidGainDb (float db)  { midDb = juce::jlimit (-12.0f, 12.0f, db); update(); }
    void setHighGainDb (float db) { highDb = juce::jlimit (-12.0f, 12.0f, db); update(); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = data[i];
                lowState[ch] += lowA * (x - lowState[ch]);
                float lowShelved = x + (lowGain - 1.0f) * lowState[ch];
                highState[ch] += highA * (x - highState[ch]);
                float highPassed = x - highState[ch];
                float highShelved = lowShelved + (highGain - 1.0f) * highPassed;

                const float midOut = b0 * highShelved + z1[ch];
                z1[ch] = b1 * highShelved - a1 * midOut + z2[ch];
                z2[ch] = b2 * highShelved - a2 * midOut;
                data[i] = std::isfinite (midOut) ? midOut : 0.0f;
            }
        }
    }

private:
    void update() noexcept
    {
        lowGain = juce::Decibels::decibelsToGain (lowDb);
        highGain = juce::Decibels::decibelsToGain (highDb);
        const float sr = (float) sampleRate;
        lowA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 180.0f / sr);
        highA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 6500.0f / sr);

        // RBJ peaking filter, centre 1.2 kHz, broad Q=0.72.
        const float A = std::sqrt (juce::Decibels::decibelsToGain (midDb));
        const float w = 2.0f * juce::MathConstants<float>::pi * 1200.0f / sr;
        const float c = std::cos (w), alpha = std::sin (w) / (2.0f * 0.72f);
        const float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * c) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * c) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    double sampleRate = 44100.0;
    float lowDb = 0.0f, midDb = 0.0f, highDb = 0.0f;
    float lowGain = 1.0f, highGain = 1.0f, lowA = 0.02f, highA = 0.5f;
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float lowState[2] {}, highState[2] {}, z1[2] {}, z2[2] {};
};
}

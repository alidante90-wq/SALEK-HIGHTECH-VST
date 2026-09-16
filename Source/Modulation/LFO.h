#pragma once
#include <JuceHeader.h>
#include <cmath>

namespace salek
{

class LFO
{
public:
    enum class Wave { Sine, Triangle, Saw, Square, Random, SAndH };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        phase = 0.0;
    }

    void reset() noexcept { phase = 0.0; lastSH = 0.0f; }

    void setRate (float hz) noexcept
    {
        rate = juce::jlimit (0.01f, 40.0f, hz);
        phaseInc = static_cast<double> (rate) / sr;
    }

    void setWave (Wave w) noexcept { wave = w; }
    void setAmount (float a) noexcept { amount = juce::jlimit (0.0f, 1.0f, a); }

    /** Returns bipolar -1..1 * amount */
    float process() noexcept
    {
        float v = 0.0f;
        const float p = static_cast<float> (phase);

        switch (wave)
        {
            case Wave::Sine:     v = std::sin (p * juce::MathConstants<float>::twoPi); break;
            case Wave::Triangle: v = 1.0f - 4.0f * std::abs (p - 0.5f); break;
            case Wave::Saw:      v = 2.0f * p - 1.0f; break;
            case Wave::Square:   v = p < 0.5f ? 1.0f : -1.0f; break;
            case Wave::Random:
            case Wave::SAndH:
                if (phase < phaseInc)
                    lastSH = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                v = lastSH;
                break;
        }

        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;

        return v * amount;
    }

private:
    double sr = 44100.0;
    double phase = 0.0;
    double phaseInc = 0.0;
    float rate = 1.0f;
    float amount = 0.0f;
    float lastSH = 0.0f;
    Wave wave = Wave::Sine;
};

} // namespace salek

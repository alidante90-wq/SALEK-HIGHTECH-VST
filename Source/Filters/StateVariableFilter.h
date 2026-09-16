#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace salek
{

/** Simple, stable state-variable filter (LP / HP / BP / Notch). Real-time safe. */
class StateVariableFilter
{
public:
    enum class Mode { LowPass, HighPass, BandPass, Notch };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept
    {
        ic1eq = ic2eq = 0.0f;
    }

    void setCutoff (float hz) noexcept
    {
        cutoff = juce::jlimit (20.0f, static_cast<float> (sr * 0.45), hz);
        update();
    }

    void setResonance (float res) noexcept
    {
        // 0..1 → mild to near self-osc
        resonance = juce::jlimit (0.0f, 1.0f, res);
        update();
    }

    void setMode (Mode m) noexcept { mode = m; }

    void setDrive (float d) noexcept
    {
        drive = juce::jlimit (0.0f, 1.0f, d);
    }

    float process (float x) noexcept
    {
        // Pre-drive
        if (drive > 1.0e-4f)
        {
            const float g = 1.0f + drive * 3.0f;
            x = std::tanh (x * g);
        }

        // SVF (Andrew Simper style, simplified)
        const float v3 = x - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        switch (mode)
        {
            case Mode::LowPass:  return v2;
            case Mode::HighPass: return x - k * v1 - v2;
            case Mode::BandPass: return v1;
            case Mode::Notch:    return x - k * v1;
            default:             return v2;
        }
    }

private:
    void update() noexcept
    {
        const float g = std::tan (juce::MathConstants<float>::pi * cutoff / static_cast<float> (sr));
        k = 2.0f - 1.8f * resonance; // resonance mapping
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    double sr = 44100.0;
    float cutoff = 1000.0f;
    float resonance = 0.2f;
    float drive = 0.0f;
    Mode mode = Mode::LowPass;

    float ic1eq = 0.0f, ic2eq = 0.0f;
    float g = 0.0f, k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
};

} // namespace salek

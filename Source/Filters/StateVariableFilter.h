#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace salek
{

/** Aggressive SVF with Virus-style resonance behaviour + saturation.
    Near self-oscillation, soft-clip output, pre/post drive. Real-time safe. */
class StateVariableFilter
{
public:
    enum class Mode { LowPass, HighPass, BandPass, Notch };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept { ic1eq = ic2eq = 0.0f; }

    void setCutoff (float hz) noexcept
    {
        cutoff = juce::jlimit (20.0f, static_cast<float> (sr * 0.45), hz);
        update();
    }

    void setResonance (float res) noexcept
    {
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
        if (drive > 1.0e-4f)
        {
            const float g = 1.0f + drive * 5.0f;
            x = std::tanh (x * g) * (1.0f / std::tanh (g * 0.65f + 0.35f));
        }

        const float v3 = x - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        float y = v2;
        switch (mode)
        {
            case Mode::LowPass:  y = v2; break;
            case Mode::HighPass: y = x - k * v1 - v2; break;
            case Mode::BandPass: y = v1; break;
            case Mode::Notch:    y = x - k * v1; break;
        }

        if (resonance > 0.4f)
        {
            const float boost = 1.0f + (resonance - 0.4f) * 1.8f;
            y *= boost;
        }
        y = std::tanh (y * (1.15f + drive * 0.8f));
        return y;
    }

private:
    void update() noexcept
    {
        const float g = std::tan (juce::MathConstants<float>::pi * cutoff / static_cast<float> (sr));
        k = 2.0f - 1.92f * resonance;
        if (k < 0.08f) k = 0.08f;
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
    float k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
};

} // namespace salek

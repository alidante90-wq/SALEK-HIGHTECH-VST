#pragma once
#include <JuceHeader.h>
#include "../Wavetables/WavetableData.h"
#include "../SHAE/SHAECore.h"

namespace salek {

/** SHAE wavetable oscillator: cubic table interpolation plus adaptive internal
    oversampling around nonlinear stages. The normal wavetable path stays 1x. */
class WavetableOscillator {
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        updatePhaseInc();
    }

    void reset() noexcept { phase = 0.0; dcState = 0.0f; analogPhase = 0.0; }

    void setFrequency(float hz) noexcept { frequency = juce::jmax(0.f, hz); updatePhaseInc(); }
    void setWavetable(const Wavetable* wt) noexcept { wavetable = wt; }
    void setTablePosition(float pos) noexcept { tablePos = juce::jlimit(0.f, 1.f, pos); }
    void setLevel(float lvl) noexcept { level = juce::jlimit(0.f, 1.f, lvl); }
    void setPhaseOffset(float o) noexcept { phaseOffset = juce::jlimit(0.f, 1.f, o); }
    void setDetuneCents(float c) noexcept { detuneCents = c; updatePhaseInc(); }
    void setWarp(float w) noexcept { warp = juce::jlimit(0.f, 1.f, w); }
    void setFold(float f) noexcept { fold = juce::jlimit(0.f, 1.f, f); }
    void setDrive(float d) noexcept { drive = juce::jlimit(0.f, 1.f, d); }

    float processSample(float pm = 0.f, float am = 1.f) noexcept
    {
        if (wavetable == nullptr) return 0.0f;

        const float nonLinear = juce::jmax (fold, drive);
        // Nonlinear synthesis is where alias energy is created. Average 2/4
        // phase samples only when needed, keeping clean patches at 1x CPU.
        const int os = nonLinear > 0.72f ? 4 : (nonLinear > 0.12f ? 2 : 1);
        const double inc = phaseInc * (1.0 + 0.00015 * std::sin (analogPhase));
        float sum = 0.0f;

        for (int k = 0; k < os; ++k)
        {
            const double p = wrapPhase (phase + inc * (double(k) / double(os))
                                      + double (phaseOffset) + double (pm));
            sum += shapeSample (float (p));
        }

        analogPhase += 0.000012 + 0.000004 * std::sin (analogPhase * 0.37);
        phase = wrapPhase (phase + inc);
        float sample = sum / static_cast<float> (os);

        // Very small one-pole DC removal prevents asymmetric waveshapers and
        // aggressive FM from accumulating offset between later filter stages.
        dcState += 0.001f * (sample - dcState);
        sample -= dcState;

        return shae::safeSample (sample * level * am, 0.995f);
    }

private:
    static double wrapPhase (double p) noexcept
    {
        p -= std::floor (p);
        return p < 0.0 ? p + 1.0 : p;
    }

    float shapeSample (float p) const noexcept
    {
        // Smooth phase distortion. Hard discontinuities are avoided in the
        // normal path; the adaptive nonlinear oversampling above handles the
        // deliberately aggressive fold/drive region.
        if (warp > 1.0e-4f)
        {
            const float amount = 1.0f + warp * 3.4f;
            float warped = std::pow (juce::jmax (1.0e-6f, p), amount);
            if (warp > 0.42f)
            {
                const float mirrorAmt = juce::jlimit (0.f, 1.f, (warp - 0.42f) * 1.4f);
                if (warped > 0.5f)
                    warped = 0.5f + (warped - 0.5f) * (1.f - mirrorAmt * 0.55f);
                if (warp > 0.75f)
                    warped = std::fmod (warped * (1.f + (warp - 0.75f) * 3.2f), 1.f);
            }
            p = juce::jlimit (0.f, 0.99999f, warped);
        }

        float sample = wavetable->getSample (tablePos, p);

        if (fold > 1.0e-4f)
        {
            const float f = fold * fold;
            const float gain = 1.f + f * 7.5f;
            const float x = sample * gain;
            float hard = x;
            for (int i = 0; i < 3; ++i)
            {
                if (hard > 1.f) hard = 2.f - hard;
                else if (hard < -1.f) hard = -2.f - hard;
                else break;
            }
            const float folded = std::sin (x * juce::MathConstants<float>::halfPi
                                           * (0.8f + f * 0.9f));
            sample = (folded * (1.f - f * 0.45f) + hard * (f * 0.45f))
                   / (1.f + f * 0.9f);
        }

        if (drive > 1.0e-4f)
        {
            const float g = 1.f + drive * 5.8f;
            float y = sample * g;
            y = std::tanh (y * (1.f + drive * 0.28f)) - 0.07f * drive * y * y;
            sample = y / (0.88f + 0.12f * std::tanh (g));
        }

        return sample;
    }

    void updatePhaseInc() noexcept
    {
        if (sr <= 0) { phaseInc = 0; return; }
        phaseInc = (double(frequency) * std::pow(2.0, double(detuneCents) / 1200.0)) / sr;
    }

    const Wavetable* wavetable = nullptr;
    double sr = 44100.0, phase = 0.0, phaseInc = 0.0, analogPhase = 0.0;
    float frequency = 440.f, tablePos = 0.f, level = 1.f, phaseOffset = 0.f, detuneCents = 0.f;
    float warp = 0.f, fold = 0.f, drive = 0.f, dcState = 0.f;
};
} // namespace salek

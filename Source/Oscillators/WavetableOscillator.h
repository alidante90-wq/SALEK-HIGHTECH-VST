#pragma once

#include <JuceHeader.h>
#include "../Wavetables/WavetableData.h"

namespace salek
{

class WavetableOscillator
{
public:
    WavetableOscillator() = default;

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        updatePhaseInc();
    }

    void reset() noexcept { phase = 0.0; }

    void setFrequency (float hz) noexcept
    {
        frequency = juce::jmax (0.0f, hz);
        updatePhaseInc();
    }

    void setWavetable (const Wavetable* wt) noexcept { wavetable = wt; }
    void setTablePosition (float pos) noexcept { tablePos = juce::jlimit (0.0f, 1.0f, pos); }
    void setLevel (float lvl) noexcept { level = juce::jlimit (0.0f, 1.0f, lvl); }
    void setPhaseOffset (float offset) noexcept { phaseOffset = juce::jlimit (0.0f, 1.0f, offset); }
    void setDetuneCents (float cents) noexcept { detuneCents = cents; updatePhaseInc(); }
    void setWarp (float w) noexcept { warp = juce::jlimit (0.0f, 1.0f, w); }
    void setFold (float f) noexcept { fold = juce::jlimit (0.0f, 1.0f, f); }
    void setDrive (float d) noexcept { drive = juce::jlimit (0.0f, 1.0f, d); }

    /** Process one sample with optional external modulation.
        pm     : phase modulation in cycles (-1..1 typical)
        am     : amplitude modulation multiplier (usually 0..2, 1 = none)
        Returns shaped sample * level * am.
    */
    float processSample (float pm = 0.0f, float am = 1.0f) noexcept
    {
        if (wavetable == nullptr)
            return 0.0f;

        double p = phase + static_cast<double> (phaseOffset) + static_cast<double> (pm);
        p -= std::floor (p);

        // Phase warp
        if (warp > 1.0e-4f)
        {
            const float pw = static_cast<float> (p);
            const float amount = 1.0f + warp * 3.5f;
            float warped = std::pow (pw, amount);
            if (warp > 0.55f && warped > 0.5f)
            {
                const float foldAmt = (warp - 0.55f) * 2.2f;
                warped = warped - foldAmt * (warped - 0.5f);
            }
            p = static_cast<double> (juce::jlimit (0.0f, 0.99999f, warped));
        }

        float sample = wavetable->getSample (tablePos, static_cast<float> (p));

        // Wavefold
        if (fold > 1.0e-4f)
        {
            const float thresh = 1.0f - fold * 0.85f;
            const float gain = 1.0f + fold * 4.0f;
            float x = sample * gain;
            for (int i = 0; i < 3; ++i)
            {
                if (x > thresh)       x = thresh - (x - thresh);
                else if (x < -thresh) x = -thresh - (x + thresh);
                else break;
            }
            sample = x * (1.0f / (1.0f + fold * 1.5f));
        }

        // Drive
        if (drive > 1.0e-4f)
        {
            const float g = 1.0f + drive * 6.0f;
            sample = std::tanh (sample * g) * (1.0f / std::tanh (g * 0.7f + 0.3f));
        }

        phase += phaseInc;
        if (phase >= 1.0) phase -= std::floor (phase);
        else if (phase < 0.0) phase += 1.0 - std::floor (phase);

        return sample * level * am;
    }

    float getPhase() const noexcept { return static_cast<float> (phase); }
    float getLastSample() const noexcept { return lastSample; }

    // For FM ratio style we expose current phase increment indirectly via frequency
    float getFrequency() const noexcept { return frequency; }

private:
    void updatePhaseInc() noexcept
    {
        if (sr <= 0.0) { phaseInc = 0.0; return; }
        const double detuneRatio = std::pow (2.0, static_cast<double> (detuneCents) / 1200.0);
        phaseInc = (static_cast<double> (frequency) * detuneRatio) / sr;
    }

    const Wavetable* wavetable = nullptr;
    double sr = 44100.0;
    double phase = 0.0;
    double phaseInc = 0.0;
    float frequency = 440.0f;
    float tablePos = 0.0f;
    float level = 1.0f;
    float phaseOffset = 0.0f;
    float detuneCents = 0.0f;
    float warp = 0.0f;
    float fold = 0.0f;
    float drive = 0.0f;
    float lastSample = 0.0f;
};

} // namespace salek

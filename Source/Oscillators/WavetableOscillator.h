#pragma once
#include <JuceHeader.h>
#include "../Wavetables/WavetableData.h"
#include "../DSP/SHAE.h"

namespace salek {

/** High-tech wavetable osc — crystal-clear interpolation path + modern fold/drive */
class WavetableOscillator {
public:
    void prepare(double sampleRate) { sr = sampleRate > 0 ? sampleRate : 44100; updatePhaseInc(); }
    void reset() noexcept { phase = 0; dcState = 0; }
    void setFrequency(float hz) noexcept { frequency = juce::jmax(0.f, hz); updatePhaseInc(); }
    void setWavetable(const Wavetable* wt) noexcept { wavetable = wt; }
    void setTablePosition(float pos) noexcept { tablePos = juce::jlimit(0.f, 1.f, pos); }
    void setLevel(float lvl) noexcept { level = juce::jlimit(0.f, 1.f, lvl); }
    void setPhaseOffset(float o) noexcept { phaseOffset = juce::jlimit(0.f, 1.f, o); }
    void setDetuneCents(float c) noexcept { detuneCents = c; updatePhaseInc(); }
    void setWarp(float w) noexcept { warp = juce::jlimit(0.f, 1.f, w); }
    void setFold(float f) noexcept { fold = juce::jlimit(0.f, 1.f, f); }
    void setDrive(float d) noexcept { drive = juce::jlimit(0.f, 1.f, d); }

    float processSample(float pm = 0.f, float am = 1.f) noexcept {
        if (!wavetable) return 0;
        double p = phase + double(phaseOffset) + double(pm);
        p -= std::floor(p);

        // Phase warp — higher quality PD + soft sync hybrid (less alias, more musical)
        if (warp > 1e-4f) {
            float pw = float(p);
            // dual-curve: soft PD at low, stronger formant at high
            float amount = 1.f + warp * 3.4f;
            float warped = std::pow (juce::jmax (1.0e-6f, pw), amount);
            // soft mirror for metallic high-end without harsh steps
            if (warp > 0.42f)
            {
                float mirrorAmt = (warp - 0.42f) * 1.4f;
                if (warped > 0.5f)
                    warped = 0.5f + (warped - 0.5f) * (1.f - mirrorAmt * 0.55f);
                // mild 2x sync flavour at extreme
                if (warp > 0.75f)
                    warped = std::fmod (warped * (1.f + (warp - 0.75f) * 3.2f), 1.f);
            }
            p = double (juce::jlimit (0.f, 0.99999f, warped));
        }

        float sample = wavetable->getSample (tablePos, float (p));
        // SHAE anti-alias assist: PolyBLEP residual when phase near discontinuity (bright tables)
        if (tablePos > 0.45f && fold < 0.05f)
        {
            const float dt = (float) juce::jmin (0.5, phaseInc);
            float t = float (p);
            float blep = shae::polyBLEP (t, dt);
            // mix small correction — reduces harsh digital edges on bright frames
            sample -= blep * (tablePos - 0.45f) * 0.35f;
        }

        // Wavefold — soft onset, glassy mid, aggressive high (professional response)
        if (fold > 1e-4f) {
            const float f = fold * fold; // perceptual curve
            float gain = 1.f + f * 7.5f;
            float x = sample * gain;
            float folded = std::sin (x * juce::MathConstants<float>::halfPi * (0.8f + f * 0.9f));
            float hard = x;
            for (int i = 0; i < 3; ++i) {
                if (hard > 1.f) hard = 2.f - hard;
                else if (hard < -1.f) hard = -2.f - hard;
                else break;
            }
            // low fold = almost pure; high = more hard edges
            sample = folded * (1.f - f * 0.45f) + hard * (f * 0.45f);
            sample /= (1.f + f * 0.9f);
        }

        // Drive — transparent tube → plastic saturation with better headroom
        if (drive > 1e-4f) {
            float g = 1.f + drive * 5.8f;
            float y = sample * g;
            // soft-clip + mild even harmonics
            y = std::tanh (y * (1.f + drive * 0.28f)) - 0.07f * drive * y * y;
            sample = y / (0.88f + 0.12f * std::tanh (g));
        }

        // tiny DC block for clarity at high resonance chains
        dcState += 0.001f * (sample - dcState);
        sample -= dcState;

        // Subtle analog-style phase drift (Moog/Virus-ish thickness when detuned/unison)
        analogPhase += 0.000012 + 0.000004 * std::sin (analogPhase * 0.37);
        phase += phaseInc * (1.0 + 0.00015 * std::sin (analogPhase));
        if (phase >= 1.0) phase -= std::floor (phase);
        else if (phase < 0.0) phase += 1.0 - std::floor (phase);
        return sample * level * am;
    }
private:
    void updatePhaseInc() noexcept {
        if (sr <= 0) { phaseInc = 0; return; }
        phaseInc = (double(frequency) * std::pow(2.0, double(detuneCents) / 1200.0)) / sr;
    }
    const Wavetable* wavetable = nullptr;
    double sr = 44100, phase = 0, phaseInc = 0, analogPhase = 0;
    float frequency = 440, tablePos = 0, level = 1, phaseOffset = 0, detuneCents = 0;
    float warp = 0, fold = 0, drive = 0, dcState = 0;
};
} // namespace salek

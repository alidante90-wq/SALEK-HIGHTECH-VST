#pragma once
#include <JuceHeader.h>
#include "../Wavetables/WavetableData.h"

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

        // Phase warp — plastic / formant-ish character without alias mess
        if (warp > 1e-4f) {
            float pw = float(p);
            float amount = 1.f + warp * 2.8f;
            float warped = std::pow (juce::jmax (1.0e-6f, pw), amount);
            // mild mirror for metallic edge at high warp
            if (warp > 0.5f && warped > 0.5f)
                warped = 0.5f + (warped - 0.5f) * (1.f - (warp - 0.5f) * 0.6f);
            p = double (juce::jlimit (0.f, 0.99999f, warped));
        }

        float sample = wavetable->getSample (tablePos, float (p));

        // Wavefold — crystal / metallic harmonics (softer asymptote)
        if (fold > 1e-4f) {
            float gain = 1.f + fold * 5.5f;
            float x = sample * gain;
            // sine-fold hybrid = glassy high-tech
            float folded = std::sin (x * juce::MathConstants<float>::halfPi * (0.7f + fold * 0.9f));
            float hard = x;
            for (int i = 0; i < 2; ++i) {
                if (hard > 1.f) hard = 2.f - hard;
                else if (hard < -1.f) hard = -2.f - hard;
                else break;
            }
            sample = folded * (0.55f + fold * 0.25f) + hard * (0.45f - fold * 0.25f);
            sample /= (1.f + fold * 0.8f);
        }

        // Drive — transparent tube → plastic saturation
        if (drive > 1e-4f) {
            float g = 1.f + drive * 5.5f;
            // asymmetric for subtle even harmonics (metal body)
            float y = sample * g;
            y = std::tanh (y * (1.f + drive * 0.3f)) - 0.08f * drive * y * y;
            sample = y / (0.85f + 0.15f * std::tanh (g));
        }

        // tiny DC block for clarity at high resonance chains
        dcState += 0.001f * (sample - dcState);
        sample -= dcState;

        phase += phaseInc;
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
    double sr = 44100, phase = 0, phaseInc = 0;
    float frequency = 440, tablePos = 0, level = 1, phaseOffset = 0, detuneCents = 0;
    float warp = 0, fold = 0, drive = 0, dcState = 0;
};
} // namespace salek

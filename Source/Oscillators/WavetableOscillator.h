#pragma once
#include <JuceHeader.h>
#include "../Wavetables/WavetableData.h"

namespace salek {

class WavetableOscillator {
public:
    void prepare(double sampleRate) { sr = sampleRate > 0 ? sampleRate : 44100; updatePhaseInc(); }
    void reset() noexcept { phase = 0; }
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
        if (warp > 1e-4f) {
            float pw = float(p), amount = 1 + warp * 3.5f;
            float warped = std::pow(pw, amount);
            if (warp > 0.55f && warped > 0.5f) warped -= (warp - 0.55f) * 2.2f * (warped - 0.5f);
            p = double(juce::jlimit(0.f, 0.99999f, warped));
        }
        float sample = wavetable->getSample(tablePos, float(p));
        if (fold > 1e-4f) {
            float thresh = 1 - fold * 0.85f, gain = 1 + fold * 4, x = sample * gain;
            for (int i = 0; i < 3; ++i) {
                if (x > thresh) x = thresh - (x - thresh);
                else if (x < -thresh) x = -thresh - (x + thresh);
                else break;
            }
            sample = x / (1 + fold * 1.5f);
        }
        if (drive > 1e-4f) {
            float g = 1 + drive * 6;
            sample = std::tanh(sample * g) / std::tanh(g * 0.7f + 0.3f);
        }
        phase += phaseInc;
        if (phase >= 1) phase -= std::floor(phase);
        else if (phase < 0) phase += 1 - std::floor(phase);
        return sample * level * am;
    }
private:
    void updatePhaseInc() noexcept {
        if (sr <= 0) { phaseInc = 0; return; }
        phaseInc = (double(frequency) * std::pow(2.0, double(detuneCents) / 1200.0)) / sr;
    }
    const Wavetable* wavetable = nullptr;
    double sr = 44100, phase = 0, phaseInc = 0;
    float frequency = 440, tablePos = 0, level = 1, phaseOffset = 0, detuneCents = 0, warp = 0, fold = 0, drive = 0;
};
} // namespace salek

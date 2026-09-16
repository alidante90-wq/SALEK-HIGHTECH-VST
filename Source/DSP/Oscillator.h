#pragma once
#include <JuceHeader.h>
#include "Wavetable.h"

class Oscillator
{
public:
    enum class WarpMode { None, Fold, PD, Sync, Mirror, Crush };

    Oscillator() = default;

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0;
        lastSample = 0.0f;
    }

    void setFrequency (float hz)
    {
        frequency = juce::jmax (0.01f, hz);
        phaseInc = frequency / (float) sr;
    }

    void setWavetablePos (float pos) { wtPos = juce::jlimit (0.0f, 1.0f, pos); }
    void setMorph (float m) { morph = juce::jlimit (0.0f, 1.0f, m); }
    void setWarp (float w) { warpAmt = juce::jlimit (0.0f, 1.0f, w); }
    void setWarpMode (WarpMode m) { warpMode = m; }
    void setFM (float amount) { fmAmount = amount; }
    void setAM (float amount) { amAmount = juce::jlimit (0.0f, 1.0f, amount); }
    void setRM (float amount) { rmAmount = juce::jlimit (0.0f, 1.0f, amount); }
    void setLevel (float lv) { level = lv; }
    void setPhaseMod (float mod) { phaseMod = mod; }
    void setRingModInput (float s) { ringIn = s; }

    float process()
    {
        float p = (float) phase + phaseMod * fmAmount * 10.0f;
        p = p - std::floor (p);

        float warpedPhase = p;
        if (warpAmt > 0.001f)
        {
            switch (warpMode)
            {
                case WarpMode::PD:
                    warpedPhase = std::pow (p, 1.0f - warpAmt * 0.85f);
                    break;
                case WarpMode::Sync:
                    warpedPhase = std::fmod (p * (1.0f + warpAmt * 7.0f), 1.0f);
                    break;
                case WarpMode::Mirror:
                {
                    float m = p * (1.0f + warpAmt * 3.0f);
                    warpedPhase = m - std::floor (m);
                    if (((int) std::floor (m)) % 2 == 1)
                        warpedPhase = 1.0f - warpedPhase;
                    break;
                }
                default: break;
            }
        }

        float sample = wavetable.getSample (wtPos + morph * 0.02f, warpedPhase);

        if (warpAmt > 0.001f && (warpMode == WarpMode::Fold || warpMode == WarpMode::None))
        {
            float drive = 1.0f + warpAmt * 8.0f;
            sample = std::sin (sample * drive * juce::MathConstants<float>::pi * 0.5f);
            if (warpAmt > 0.6f)
                sample = std::sin (sample * (1.0f + (warpAmt - 0.6f) * 6.0f));
        }
        if (warpMode == WarpMode::Crush && warpAmt > 0.01f)
        {
            float bits = 3.0f + (1.0f - warpAmt) * 12.0f;
            sample = std::floor (sample * bits + 0.5f) / bits;
        }

        if (amAmount > 0.001f)
            sample *= (1.0f - amAmount) + amAmount * (0.5f + 0.5f * std::sin ((float) phase * juce::MathConstants<float>::twoPi * 2.0f));

        if (rmAmount > 0.001f)
            sample = sample * (1.0f - rmAmount) + (sample * ringIn) * rmAmount;

        phase += phaseInc;
        if (phase >= 1.0) phase -= std::floor (phase);

        lastSample = sample * level;
        return lastSample;
    }

    void reset() { phase = 0.0; lastSample = 0.0f; }
    float getPhase() const { return (float) phase; }
    float getLastSample() const { return lastSample; }

    static Wavetable& getSharedWavetable()
    {
        static Wavetable wt;
        return wt;
    }

private:
    Wavetable& wavetable = getSharedWavetable();
    double sr = 44100.0;
    double phase = 0.0;
    float phaseInc = 0.0f;
    float frequency = 440.0f;
    float wtPos = 0.0f;
    float morph = 0.0f;
    float warpAmt = 0.0f;
    WarpMode warpMode = WarpMode::Fold;
    float fmAmount = 0.0f;
    float amAmount = 0.0f;
    float rmAmount = 0.0f;
    float phaseMod = 0.0f;
    float ringIn = 0.0f;
    float level = 1.0f;
    float lastSample = 0.0f;
};
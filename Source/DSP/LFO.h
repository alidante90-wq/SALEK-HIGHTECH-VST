#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

class LFO
{
public:
    enum class Shape { Sine, Triangle, Saw, Square, SampleHold, Chaos };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0;
        random.setSeedRandomly();
    }

    void setRate (float hz) { rate = juce::jmax (0.01f, hz); }
    void setDepth (float d) { depth = juce::jlimit (0.0f, 1.0f, d); }
    void setShape (Shape s) { shape = s; }
    void setSync (bool s) { sync = s; }
    void setOneShot (bool o) { oneShot = o; }

    // For tempo sync later: setRateInBeats etc.
    void reset() { phase = 0.0; heldValue = 0.0f; }

    float process()
    {
        if (oneShot && phase >= 1.0)
            return 0.0f;

        float out = 0.0f;
        float p = (float) phase;

        switch (shape)
        {
            case Shape::Sine:
                out = std::sin (p * juce::MathConstants<float>::twoPi);
                break;
            case Shape::Triangle:
                out = 1.0f - 4.0f * std::abs (std::round (p - 0.25f) - (p - 0.25f));
                break;
            case Shape::Saw:
                out = 2.0f * p - 1.0f;
                break;
            case Shape::Square:
                out = p < 0.5f ? 1.0f : -1.0f;
                break;
            case Shape::SampleHold:
                if (phase < lastPhase) // wrapped
                    heldValue = random.nextFloat() * 2.0f - 1.0f;
                out = heldValue;
                break;
            case Shape::Chaos:
                // Simple chaotic map
                chaosState = 3.7f * chaosState * (1.0f - chaosState);
                out = chaosState * 2.0f - 1.0f;
                break;
        }

        lastPhase = phase;
        phase += rate / sr;
        if (phase >= 1.0)
        {
            if (oneShot)
                phase = 1.0;
            else
                phase -= 1.0;
        }

        return out * depth;
    }

    // Drawable shape support (future): load custom table
    void setCustomShape (const std::vector<float>& table)
    {
        custom = table;
        useCustom = !custom.empty();
    }

private:
    double sr = 44100.0;
    double phase = 0.0;
    double lastPhase = 0.0;
    float rate = 1.0f;
    float depth = 1.0f;
    Shape shape = Shape::Sine;
    bool sync = false;
    bool oneShot = false;
    float heldValue = 0.0f;
    float chaosState = 0.5f;
    juce::Random random;
    std::vector<float> custom;
    bool useCustom = false;
};
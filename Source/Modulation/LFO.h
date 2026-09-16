#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <array>
namespace salek {
class LFO {
public:
    enum class Wave { Sine, Triangle, Saw, Square, SAndH, Custom };
    static constexpr int TableSize = 16;

    void prepare(double sampleRate){ sr=sampleRate>0?sampleRate:44100; phase=0; }
    void reset() noexcept { phase=0; lastSH=0; }
    void setRate(float hz) noexcept { rate=juce::jlimit(0.01f,40.f,hz); phaseInc=double(rate)/sr; }
    void setWave(Wave w) noexcept { wave=w; }
    void setAmount(float a) noexcept { amount=juce::jlimit(0.f,1.f,a); }
    void setCustomPoint (int i, float v) noexcept
    {
        if (i >= 0 && i < TableSize)
            customTable[(size_t) i] = juce::jlimit (-1.f, 1.f, v);
    }
    float getCustomPoint (int i) const noexcept
    {
        return (i >= 0 && i < TableSize) ? customTable[(size_t) i] : 0.f;
    }
    std::array<float, TableSize>& getTable() noexcept { return customTable; }
    const std::array<float, TableSize>& getTable() const noexcept { return customTable; }

    /** Serum-inspired shape bank (0..15) written into custom table */
    void loadPresetShape (int preset) noexcept
    {
        const float twoPi = juce::MathConstants<float>::twoPi;
        for (int i = 0; i < TableSize; ++i)
        {
            float t = (float) i / (float) (TableSize - 1);
            float v = 0.f;
            switch (preset)
            {
                case 0:  // sine
                    v = std::sin (t * twoPi); break;
                case 1:  // triangle
                    v = 1.f - 4.f * std::abs (t - 0.5f); break;
                case 2:  // saw up
                    v = 2.f * t - 1.f; break;
                case 3:  // saw down
                    v = 1.f - 2.f * t; break;
                case 4:  // square 50%
                    v = t < 0.5f ? 1.f : -1.f; break;
                case 5:  // pulse 25%
                    v = t < 0.25f ? 1.f : -1.f; break;
                case 6:  // pulse 12%
                    v = t < 0.12f ? 1.f : -1.f; break;
                case 7:  // exponential rise (pluck / filter snap)
                    v = 2.f * (1.f - std::exp (-4.f * t)) - 1.f; break;
                case 8:  // exponential fall
                    v = 2.f * std::exp (-4.f * t) - 1.f; break;
                case 9:  // log / slow attack curve
                    v = 2.f * std::log (1.f + 9.f * t) / std::log (10.f) - 1.f; break;
                case 10: // cosine bell (smooth hump)
                    v = 0.5f - 0.5f * std::cos (t * twoPi); break;
                case 11: // double peak (wobble)
                    v = std::sin (t * twoPi * 2.f) * (1.f - 0.35f * t); break;
                case 12: // chaos-ish stepped
                    v = (i % 3 == 0) ? 1.f : ((i % 3 == 1) ? -0.6f : 0.15f); break;
                case 13: // ramp hold (gate style)
                    v = t < 0.15f ? (t / 0.15f) * 2.f - 1.f
                        : (t < 0.7f ? 1.f : 1.f - (t - 0.7f) / 0.3f * 2.f); break;
                case 14: // bipolar S-curve
                    v = std::tanh ((t - 0.5f) * 6.f); break;
                default: // noise-ish sparse spikes
                    v = (i == 2 || i == 9 || i == 13) ? 1.f
                        : ((i == 5 || i == 11) ? -1.f : 0.f); break;
            }
            customTable[(size_t) i] = juce::jlimit (-1.f, 1.f, v);
        }
    }

    float process() noexcept {
        float v=0, p=float(phase);
        switch(wave){
            case Wave::Sine: v=std::sin(p*juce::MathConstants<float>::twoPi); break;
            case Wave::Triangle: v=1-4*std::abs(p-0.5f); break;
            case Wave::Saw: v=2*p-1; break;
            case Wave::Square: v=p<0.5f?1.f:-1.f; break;
            case Wave::SAndH:
                if(phase<phaseInc) lastSH=juce::Random::getSystemRandom().nextFloat()*2-1;
                v=lastSH; break;
            case Wave::Custom:
            {
                float idx = p * (float) (TableSize - 1);
                int i0 = juce::jlimit (0, TableSize - 1, (int) idx);
                int i1 = juce::jmin (TableSize - 1, i0 + 1);
                float f = idx - (float) i0;
                v = customTable[(size_t) i0] * (1.f - f) + customTable[(size_t) i1] * f;
                break;
            }
        }
        phase+=phaseInc; if(phase>=1) phase-=1;
        return v*amount;
    }

    float getPhase() const noexcept { return (float) phase; }

private:
    double sr=44100, phase=0, phaseInc=0; float rate=1, amount=0, lastSH=0;
    Wave wave=Wave::Sine;
    std::array<float, TableSize> customTable {{
        0,0.4f,0.8f,1,0.8f,0.4f,0,-0.4f,-0.8f,-1,-0.8f,-0.4f,0,0.3f,0.6f,0.9f
    }};
};
}

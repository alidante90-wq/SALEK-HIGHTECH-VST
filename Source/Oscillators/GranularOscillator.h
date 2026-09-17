#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek {

class GranularOscillator
{
public:
    static constexpr int kMaxGrains   = 32;
    static constexpr int kBufferSize  = 1 << 16;
    static constexpr int kBufferMask  = kBufferSize - 1;

    struct Grain
    {
        bool  active   = false;
        float pos      = 0.f;
        float phase    = 0.f;
        float phaseInc = 0.f;
        float pitch    = 1.f;
        float pan      = 0.5f;
        float amp      = 1.f;
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        writePos = 0;
        std::fill (buffer.begin(), buffer.end(), 0.f);
        for (auto& g : grains) g.active = false;
        updateDerived();
    }

    void reset() noexcept
    {
        writePos = 0;
        for (auto& g : grains) g.active = false;
        densityPhase = 0.f;
    }

    void setPosition   (float p)  noexcept { position  = juce::jlimit (0.f, 1.f, p); }
    void setDensity    (float d)  noexcept { density   = juce::jlimit (0.05f, 1.f, d); updateDerived(); }
    void setGrainSize  (float s)  noexcept { grainSize = juce::jlimit (0.01f, 1.f, s); updateDerived(); }
    void setSpray      (float s)  noexcept { spray     = juce::jlimit (0.f, 1.f, s); }
    void setPitch      (float p)  noexcept { pitch     = juce::jlimit (0.25f, 4.f, p); }
    void setPitchSpray (float s)  noexcept { pitchSpray = juce::jlimit (0.f, 1.f, s); }
    void setFeedback   (float f)  noexcept { feedback  = juce::jlimit (0.f, 0.95f, f); }
    void setMix        (float m)  noexcept { mix       = juce::jlimit (0.f, 1.f, m); }
    void setFreeze     (bool f)   noexcept { freeze    = f; }

    void writeInput (const float* left, const float* right, int numSamples)
    {
        if (freeze) return;
        for (int i = 0; i < numSamples; ++i)
        {
            const float mono = 0.5f * (left[i] + (right ? right[i] : left[i]));
            buffer[static_cast<size_t> (writePos & kBufferMask)] = mono;
            ++writePos;
        }
    }

    void process (juce::AudioBuffer<float>& audio)
    {
        const int n = audio.getNumSamples();
        auto* L = audio.getWritePointer (0);
        auto* R = audio.getNumChannels() > 1 ? audio.getWritePointer (1) : nullptr;
        writeInput (L, R, n);

        for (int i = 0; i < n; ++i)
        {
            densityPhase += densityRate;
            while (densityPhase >= 1.f)
            {
                densityPhase -= 1.f;
                spawnGrain();
            }

            float outL = 0.f, outR = 0.f;
            for (auto& g : grains)
            {
                if (! g.active) continue;
                const float env = 0.5f - 0.5f * std::cos (g.phase * juce::MathConstants<float>::twoPi);
                const float sample = readBuffer (g.pos) * env * g.amp;
                outL += sample * (1.f - g.pan);
                outR += sample * g.pan;
                g.pos   += g.pitch;
                g.phase += g.phaseInc;
                if (g.phase >= 1.f) g.active = false;
            }

            if (feedback > 1e-4f && ! freeze)
            {
                const int idx = (writePos - n + i) & kBufferMask;
                buffer[static_cast<size_t> (idx)] =
                    juce::jlimit (-1.5f, 1.5f,
                        buffer[static_cast<size_t> (idx)] * (1.f - feedback * 0.3f)
                        + (outL + outR) * 0.5f * feedback);
            }

            const float dryL = L[i];
            const float dryR = R ? R[i] : dryL;
            L[i] = dryL * (1.f - mix) + outL * mix;
            if (R) R[i] = dryR * (1.f - mix) + outR * mix;
        }
    }

private:
    void updateDerived()
    {
        densityRate = (2.f + density * 78.f) / static_cast<float> (sr);
        grainSamples = juce::jmax (32, static_cast<int> ((0.005f + grainSize * 0.245f) * sr));
        grainPhaseInc = 1.f / static_cast<float> (grainSamples);
    }

    void spawnGrain()
    {
        for (auto& g : grains)
        {
            if (g.active) continue;
            g.active   = true;
            const float sprayOffset = (rng.nextFloat() - 0.5f) * spray * static_cast<float> (kBufferSize) * 0.4f;
            const float basePos = static_cast<float> (writePos) - position * static_cast<float> (kBufferSize) * 0.9f;
            g.pos      = basePos + sprayOffset;
            g.phase    = 0.f;
            g.phaseInc = grainPhaseInc;
            g.pitch    = pitch * (1.f + (rng.nextFloat() - 0.5f) * pitchSpray * 1.5f);
            g.pan      = 0.5f + (rng.nextFloat() - 0.5f) * 0.7f;
            g.amp      = 0.6f + rng.nextFloat() * 0.4f;
            return;
        }
    }

    float readBuffer (float pos) const noexcept
    {
        const int i0 = static_cast<int> (pos) & kBufferMask;
        const int i1 = (i0 + 1) & kBufferMask;
        const float frac = pos - std::floor (pos);
        return buffer[static_cast<size_t> (i0)] * (1.f - frac)
             + buffer[static_cast<size_t> (i1)] * frac;
    }

    double sr = 44100.0;
    int    writePos = 0;
    float  position = 0.5f, density = 0.4f, grainSize = 0.25f, spray = 0.2f;
    float  pitch = 1.f, pitchSpray = 0.1f, feedback = 0.f, mix = 0.5f;
    bool   freeze = false;
    float  densityRate = 0.001f, densityPhase = 0.f, grainPhaseInc = 1.f / 2048.f;
    int    grainSamples = 2048;
    std::array<float, kBufferSize> buffer {};
    std::array<Grain, kMaxGrains>  grains {};
    juce::Random rng { 0x5A1E4C };
};

} // namespace salek

#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <vector>

namespace salek {
namespace shae {

/** SALEK HIGHTECH AUDIO ENGINE — quality / oversampling modes */
enum class Quality : int { Eco = 0, Normal = 1, High = 2, Ultra = 3 };

inline int oversampleFactor (Quality q) noexcept
{
    switch (q) {
        case Quality::Eco:    return 1;
        case Quality::Normal: return 2;
        case Quality::High:   return 4;
        case Quality::Ultra:  return 8;
    }
    return 1;
}

/** PolyBLEP for bandlimited naive waveforms (anti-alias) */
inline float polyBLEP (float t, float dt) noexcept
{
    if (t < dt)
    {
        t = t / dt;
        return t + t - t * t - 1.f;
    }
    if (t > 1.f - dt)
    {
        t = (t - 1.f) / dt;
        return t * t + t + t + 1.f;
    }
    return 0.f;
}

/** BLAMP for triangle / integrated BLEP */
inline float polyBLAMP (float t, float dt) noexcept
{
    if (dt < 1e-8f) return 0.f;
    if (t < dt)
    {
        t = t / dt - 1.f;
        return -1.f / 3.f * t * t * t;
    }
    if (t > 1.f - dt)
    {
        t = (t - 1.f) / dt + 1.f;
        return 1.f / 3.f * t * t * t;
    }
    return 0.f;
}

/** SALEK Distortion Matrix modes */
enum class DistMode : int {
    Soft = 0, Tube, Tape, HardClip, WaveShaper, WaveFold,
    Asymmetric, Rectifier, BitCrush, Downsample, Digital, Metal, Neuro, Hitech,
    NumModes
};

inline const char* distModeName (int m) noexcept
{
    static const char* n[] = {
        "Soft","Tube","Tape","HardClip","Shaper","Fold",
        "Asym","Rect","Crush","Down","Digital","Metal","Neuro","Hitech"
    };
    return n[juce::jlimit (0, (int) DistMode::NumModes - 1, m)];
}

inline float processDistSample (float x, int mode, float drive) noexcept
{
    const float g = 1.f + drive * 6.f;
    x *= g;
    switch (mode)
    {
        case 0: // Soft
            return std::tanh (x) / (0.9f + 0.1f * drive);
        case 1: // Tube (even harmonics bias)
            return std::tanh (x + 0.15f * drive * x * x) * 0.95f;
        case 2: // Tape
            return std::tanh (x * (1.f + 0.3f * drive)) - 0.08f * drive * x * x * x;
        case 3: // HardClip
            return juce::jlimit (-1.f, 1.f, x * (0.7f + 0.3f * drive));
        case 4: // WaveShaper
            return std::sin (x * juce::MathConstants<float>::halfPi * (0.8f + drive * 0.4f));
        case 5: // WaveFold
        {
            float y = x;
            for (int i = 0; i < 3; ++i)
            {
                if (y > 1.f) y = 2.f - y;
                else if (y < -1.f) y = -2.f - y;
                else break;
            }
            return y / (1.f + drive * 0.5f);
        }
        case 6: // Asymmetric
            return x >= 0.f ? std::tanh (x) : std::tanh (x * (1.2f + drive));
        case 7: // Rectifier
            return std::tanh (std::abs (x) * 1.2f) * (x >= 0.f ? 1.f : -0.6f);
        case 8: // BitCrush
        {
            float bits = 3.f + (1.f - drive) * 9.f;
            float step = std::pow (2.f, -bits);
            return std::floor (x / step + 0.5f) * step;
        }
        case 9: // Downsample (handled externally mostly)
            return std::tanh (x);
        case 10: // Digital
            return juce::jlimit (-1.f, 1.f, x * 1.4f) * (1.f - 0.2f * drive)
                 + 0.15f * drive * std::copysign (1.f, x);
        case 11: // Metal
            return std::tanh (x * 2.f) + 0.3f * drive * std::sin (x * 8.f);
        case 12: // Neuro
            return std::tanh (x + drive * 0.5f * std::sin (x * 12.f)) * 0.9f;
        case 13: // Hitech
            return std::sin (std::tanh (x * (1.5f + drive)) * juce::MathConstants<float>::pi * 0.5f);
        default:
            return std::tanh (x);
    }
}

/** 3-band formant (A E I O U morph 0..1) — SALEK vocal/alien */
struct FormantFilter
{
    void prepare (double sampleRate) noexcept
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        for (auto& s : st) s = 0.f;
    }

    void setMorph (float m) noexcept { morph = juce::jlimit (0.f, 1.f, m); }
    void setAmount (float a) noexcept { amount = juce::jlimit (0.f, 1.f, a); }

    // Vowel center freqs (approx F1,F2,F3)
    void process (float& L, float& R) noexcept
    {
        if (amount < 1e-4f) return;
        // A E I O U tables
        static const float F1[5] = { 800.f,  500.f,  300.f,  450.f,  325.f };
        static const float F2[5] = { 1200.f, 1800.f, 2300.f, 800.f,  700.f };
        static const float F3[5] = { 2500.f, 2500.f, 3000.f, 2800.f, 2600.f };

        float t = morph * 4.f;
        int i0 = juce::jlimit (0, 3, (int) t);
        int i1 = i0 + 1;
        float f = t - (float) i0;
        float f1 = F1[i0] + (F1[i1] - F1[i0]) * f;
        float f2 = F2[i0] + (F2[i1] - F2[i0]) * f;
        float f3 = F3[i0] + (F3[i1] - F3[i0]) * f;

        auto bp = [&] (float x, float freq, float& z1, float& z2) -> float
        {
            float w = 2.f * juce::MathConstants<float>::pi * freq / (float) sr;
            float q = 6.f + amount * 8.f;
            float a = std::sin (w) / (2.f * q);
            float y = x - z1 * (2.f * std::cos (w) * (1.f - a)) - z2 * (1.f - 2.f * a);
            // simplified resonator
            float r = 0.96f + amount * 0.03f;
            float out = x + r * (std::sin (w) * z1);
            z2 = z1;
            z1 = out * 0.15f + x * 0.85f;
            juce::ignoreUnused (a, y);
            // band emphasis via 1-pole peak
            float c = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * freq / (float) sr);
            float lp = z1 + c * (x - z1);
            z1 = lp;
            return (x + (x - lp) * amount * 2.5f);
        };

        // Use three cascaded peak-ish stages
        float yL = L, yR = R;
        // crude formant: sum of bandpassed resonances
        auto res = [&] (float x, float freq, float& s) -> float
        {
            float c = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * (freq * 0.5f) / (float) sr);
            s += c * (x - s);
            float hp = x - s;
            float c2 = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * freq / (float) sr);
            // band
            float b = s; // low
            float mid = hp;
            juce::ignoreUnused (c2, b);
            return x + mid * amount * 1.8f * (freq / 2000.f);
        };

        yL = res (yL, f1, st[0]);
        yL = res (yL, f2, st[1]);
        yL = res (yL, f3, st[2]);
        yR = res (yR, f1, st[3]);
        yR = res (yR, f2, st[4]);
        yR = res (yR, f3, st[5]);
        L = L * (1.f - amount * 0.7f) + yL * (amount * 0.7f);
        R = R * (1.f - amount * 0.7f) + yR * (amount * 0.7f);
        juce::ignoreUnused (bp);
    }

    double sr = 44100.0;
    float morph = 0.f, amount = 0.f;
    float st[6] {};
};

/** Simple 2x oversampler (upsample linear, process, downsample) for hot path stages */
struct Oversampler2x
{
    void reset() noexcept { zUpL = zUpR = zDnL = zDnR = 0.f; }

    template <typename Fn>
    void process (juce::AudioBuffer<float>& buffer, Fn&& processSample)
    {
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        if (ch < 1) return;
        for (int i = 0; i < n; ++i)
        {
            float L0 = buffer.getSample (0, i);
            float R0 = ch > 1 ? buffer.getSample (1, i) : L0;
            // upsample: hold + interpolate mid
            float L1 = 0.5f * (L0 + zUpL);
            float R1 = 0.5f * (R0 + zUpR);
            zUpL = L0; zUpR = R0;
            processSample (L1, R1);
            processSample (L0, R0);
            // simple downsample average + mild LP
            float oL = 0.5f * (L1 + L0);
            float oR = 0.5f * (R1 + R0);
            zDnL += 0.55f * (oL - zDnL);
            zDnR += 0.55f * (oR - zDnR);
            buffer.setSample (0, i, zDnL);
            if (ch > 1) buffer.setSample (1, i, zDnR);
        }
    }

    float zUpL = 0, zUpR = 0, zDnL = 0, zDnR = 0;
};

} // namespace shae
} // namespace salek

#pragma once
/**
 * SHAE — SALEK HIGHTECH AUDIO ENGINE
 * Custom realtime-safe DSP (original implementation).
 * License of this file: same as the GITI project (proprietary to SALEK HIGHTECH unless stated otherwise).
 * Algorithms inspired by public DSP literature (PolyBLEP, ZDF SVF, waveshaping) — no third-party code copied.
 */
#include <JuceHeader.h>
#include <cmath>
#include <array>
#include <vector>

namespace salek {
namespace shae {

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

inline float flushDenormal (float x) noexcept
{
    return (std::abs (x) < 1.0e-15f) ? 0.f : x;
}

inline bool isFinite (float x) noexcept
{
    return std::isfinite (x);
}

/** Fast tanh approx (Pade) — realtime friendly */
inline float fastTanh (float x) noexcept
{
    if (x < -3.f) return -1.f;
    if (x >  3.f) return  1.f;
    const float x2 = x * x;
    return x * (27.f + x2) / (27.f + 9.f * x2);
}

/** Soft clip with approximate gain compensation */
inline float softClipComp (float x, float drive) noexcept
{
    const float g = 1.f + drive * 4.f;
    const float y = fastTanh (x * g);
    // compensate average gain so drive ≠ automatic loudness
    const float comp = 1.f / (0.75f + 0.25f * fastTanh (g * 0.5f));
    return y * comp;
}

/** PolyBLEP residual (bandlimited step) */
inline float polyBLEP (float t, float dt) noexcept
{
    if (dt < 1e-8f) return 0.f;
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

inline float polyBLAMP (float t, float dt) noexcept
{
    if (dt < 1e-8f) return 0.f;
    if (t < dt)
    {
        t = t / dt - 1.f;
        return (-1.f / 3.f) * t * t * t;
    }
    if (t > 1.f - dt)
    {
        t = (t - 1.f) / dt + 1.f;
        return (1.f / 3.f) * t * t * t;
    }
    return 0.f;
}

/** DC blocker (1-pole HPF ~20Hz) */
struct DCBlocker
{
    void prepare (double sampleRate) noexcept
    {
        const float hz = 20.f;
        const float x = std::exp (-2.f * juce::MathConstants<float>::pi * hz / (float) sampleRate);
        R = x;
        xz = yz = 0.f;
    }
    float process (float x) noexcept
    {
        float y = x - xz + R * yz;
        xz = x;
        yz = y;
        return flushDenormal (y);
    }
    float R = 0.995f, xz = 0.f, yz = 0.f;
};

/** One-pole lowpass for mono-bass extraction */
struct OnePoleLP
{
    void setCutoff (float hz, double sr) noexcept
    {
        a = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * hz / (float) sr);
    }
    float process (float x) noexcept
    {
        z += a * (x - z);
        return flushDenormal (z);
    }
    float a = 0.1f, z = 0.f;
};

// ---------- Distortion Matrix ----------
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
    const float d = juce::jlimit (0.f, 1.f, drive);
    // Pre-gain with compensation path
    const float g = 1.f + d * 5.5f;
    x *= g;
    float y = x;
    switch (mode)
    {
        case 0: y = softClipComp (x / g, d); break; // already compensated
        case 1: // Tube
            y = fastTanh (x + 0.12f * d * x * x);
            y *= 1.f / (0.8f + 0.2f * d);
            break;
        case 2: // Tape
            y = fastTanh (x * (1.f + 0.25f * d)) - 0.06f * d * x * x * x;
            y *= 0.95f;
            break;
        case 3: // HardClip
            y = juce::jlimit (-1.f, 1.f, x * (0.65f + 0.35f * d));
            break;
        case 4: // WaveShaper
            y = std::sin (x * juce::MathConstants<float>::halfPi * (0.75f + d * 0.45f));
            break;
        case 5: // WaveFold
        {
            float t = x;
            for (int i = 0; i < 3; ++i)
            {
                if (t > 1.f) t = 2.f - t;
                else if (t < -1.f) t = -2.f - t;
                else break;
            }
            y = t / (1.f + d * 0.6f);
            break;
        }
        case 6: // Asymmetric
            y = x >= 0.f ? fastTanh (x) : fastTanh (x * (1.15f + d));
            break;
        case 7: // Rectifier
            y = fastTanh (std::abs (x) * 1.15f) * (x >= 0.f ? 1.f : -0.55f);
            break;
        case 8: // BitCrush
        {
            float bits = 3.f + (1.f - d) * 9.f;
            float step = std::pow (2.f, -bits);
            y = std::floor (x / step + 0.5f) * step;
            break;
        }
        case 9: // Downsample placeholder (caller may hold samples)
            y = softClipComp (x / g, d);
            break;
        case 10: // Digital
            y = juce::jlimit (-1.f, 1.f, x * 1.35f);
            y = y * (1.f - 0.15f * d) + 0.12f * d * std::copysign (1.f, y);
            break;
        case 11: // Metal
            y = fastTanh (x * 1.8f) + 0.28f * d * std::sin (x * 7.5f);
            y *= 0.85f;
            break;
        case 12: // Neuro
            y = fastTanh (x + d * 0.45f * std::sin (x * 11.f)) * 0.9f;
            break;
        case 13: // Hitech
            y = std::sin (fastTanh (x * (1.4f + d)) * juce::MathConstants<float>::halfPi);
            break;
        default:
            y = softClipComp (x / g, d);
            break;
    }
    if (! isFinite (y)) y = 0.f;
    return y;
}

// ---------- Formant ----------
struct FormantFilter
{
    void prepare (double sampleRate) noexcept
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        for (auto& s : st) s = 0.f;
    }
    void setMorph (float m) noexcept { morph = juce::jlimit (0.f, 1.f, m); }
    void setAmount (float a) noexcept { amount = juce::jlimit (0.f, 1.f, a); }

    void process (float& L, float& R) noexcept
    {
        if (amount < 1e-4f) return;
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

        auto peak = [&] (float x, float freq, float& s) -> float
        {
            float c = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * (freq * 0.45f) / (float) sr);
            s += c * (x - s);
            s = flushDenormal (s);
            float hp = x - s;
            return x + hp * amount * 1.6f * juce::jlimit (0.3f, 1.4f, freq / 1800.f);
        };

        float yL = peak (peak (peak (L, f1, st[0]), f2, st[1]), f3, st[2]);
        float yR = peak (peak (peak (R, f1, st[3]), f2, st[4]), f3, st[5]);
        L = L * (1.f - amount * 0.65f) + yL * (amount * 0.65f);
        R = R * (1.f - amount * 0.65f) + yR * (amount * 0.65f);
        if (! isFinite (L)) L = 0.f;
        if (! isFinite (R)) R = 0.f;
    }

    double sr = 44100.0;
    float morph = 0.f, amount = 0.f;
    float st[6] {};
};

/** Lightweight 2x oversampler for nonlinear stages */
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
            float L1 = 0.5f * (L0 + zUpL);
            float R1 = 0.5f * (R0 + zUpR);
            zUpL = L0; zUpR = R0;
            processSample (L1, R1);
            processSample (L0, R0);
            float oL = 0.5f * (L1 + L0);
            float oR = 0.5f * (R1 + R0);
            zDnL += 0.5f * (oL - zDnL);
            zDnR += 0.5f * (oR - zDnR);
            buffer.setSample (0, i, flushDenormal (zDnL));
            if (ch > 1) buffer.setSample (1, i, flushDenormal (zDnR));
        }
    }

    float zUpL = 0, zUpR = 0, zDnL = 0, zDnR = 0;
};

} // namespace shae
} // namespace salek

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


/** Karplus-ish / metal-glass resonator (exciter in, body out) */
struct Resonator
{
    void prepare (double sampleRate, int maxBlock = 512) noexcept
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        const int n = juce::jmax (64, (int) (sr * 0.05)); // ~50ms max
        buf.assign ((size_t) n, 0.f);
        pos = 0;
        filterZ = 0.f;
    }
    void setFrequency (float hz) noexcept
    {
        hz = juce::jlimit (40.f, 4000.f, hz);
        delaySamps = juce::jlimit (2.f, (float) buf.size() - 2.f, (float) (sr / hz));
    }
    void setDecay (float d) noexcept { decay = juce::jlimit (0.8f, 0.9995f, 0.85f + d * 0.149f); }
    void setBrightness (float b) noexcept { bright = juce::jlimit (0.1f, 0.95f, b); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    /** 0 metal 1 glass 2 steel 3 cyber */
    void setMaterial (int m) noexcept { material = juce::jlimit (0, 3, m); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1e-4f || buf.empty()) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int bs = (int) buf.size();
        float fb = decay;
        if (material == 1) fb *= 0.998f; // glass longer
        if (material == 3) fb *= 0.992f; // cyber shorter metallic

        for (int i = 0; i < n; ++i)
        {
            float in = buffer.getSample (0, i);
            if (ch > 1) in = 0.5f * (in + buffer.getSample (1, i));

            float rp = (float) pos - delaySamps;
            while (rp < 0.f) rp += (float) bs;
            int i0 = ((int) rp) % bs;
            int i1 = (i0 + 1) % bs;
            float frac = rp - std::floor (rp);
            float y = buf[(size_t) i0] * (1.f - frac) + buf[(size_t) i1] * frac;

            // damping LP
            filterZ += bright * (y - filterZ);
            y = filterZ;
            if (material == 2) // steel: extra harmonic grit
                y = fastTanh (y * 1.4f);
            if (material == 3)
                y = y + 0.15f * std::sin (y * 9.f);

            buf[(size_t) pos] = in * 0.35f + y * fb;
            pos = (pos + 1) % bs;

            float outL = buffer.getSample (0, i) * (1.f - mix) + y * mix;
            buffer.setSample (0, i, flushDenormal (outL));
            if (ch > 1)
            {
                float outR = buffer.getSample (1, i) * (1.f - mix) + y * mix * 0.96f;
                buffer.setSample (1, i, flushDenormal (outR));
            }
        }
    }

    double sr = 44100.0;
    std::vector<float> buf;
    int pos = 0;
    float delaySamps = 100.f, decay = 0.97f, bright = 0.6f, mix = 0.f, filterZ = 0.f;
    int material = 0;
};

/** Lightweight spectral smear: parallel tuned comb stack (no FFT, realtime-safe) */
/** Advanced spectral colour without heavy FFT:
 *  - parallel tuned resonators (partials)
 *  - freeze (hold resonator state)
 *  - shift (scale delay lengths → pitch-ish smear)
 *  - gate (noise floor / spectral gate approx)
 */
struct SpectralSmear
{
    static constexpr int kCombs = 8;
    void prepare (double sampleRate) noexcept
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        rebuildDelays (1.f);
        for (int i = 0; i < kCombs; ++i) { cpos[i] = 0; dampZ[i] = 0.f; }
    }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setAmount (float a) noexcept { amount = juce::jlimit (0.f, 1.f, a); }
    void setFreeze (bool f) noexcept { freeze = f; }
    void setShift (float s) noexcept
    {
        // -1..+1 → delay scale 0.5..2
        shift = juce::jlimit (-1.f, 1.f, s);
        rebuildDelays (std::pow (2.f, shift));
    }
    void setGate (float g) noexcept { gate = juce::jlimit (0.f, 1.f, g); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const float fb = 0.5f + amount * 0.45f;
        const float gateThresh = gate * gate * 0.08f;

        for (int i = 0; i < n; ++i)
        {
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;
            float mono = 0.5f * (inL + inR);
            float env = std::abs (mono);
            envFollow += 0.02f * (env - envFollow);
            const float gateGain = (gate < 1e-3f) ? 1.f
                : juce::jlimit (0.f, 1.f, (envFollow - gateThresh) / (gateThresh + 1e-4f));

            float wet = 0.f;
            for (int c = 0; c < kCombs; ++c)
            {
                auto& b = comb[c];
                const int len = (int) b.size();
                if (len < 4) continue;
                int p = cpos[c] % len;
                float o = b[(size_t) p];
                // mild damping LP inside loop
                dampZ[c] += (0.25f + amount * 0.5f) * (o - dampZ[c]);
                o = dampZ[c];
                if (! freeze)
                    b[(size_t) p] = mono * gateGain + o * fb;
                cpos[c] = (p + 1) % len;
                // odd combs → L bias, even → R
                wet += o * (0.7f + 0.3f * ((c & 1) ? 1.f : 0.7f));
            }
            wet *= (1.f / (float) kCombs);
            wet = fastTanh (wet * (1.1f + amount * 0.6f));
            const float w = mix * (0.55f + amount * 0.45f);
            buffer.setSample (0, i, inL * (1.f - w) + wet * w);
            if (ch > 1)
                buffer.setSample (1, i, inR * (1.f - w) + wet * w * 0.96f);
        }
    }

    void rebuildDelays (float scale) noexcept
    {
        // Inharmonic partial ratios for alien/metallic spectra
        static const float ratios[kCombs] = { 1.f, 1.47f, 2.12f, 2.76f, 3.51f, 4.33f, 5.18f, 6.07f };
        for (int i = 0; i < kCombs; ++i)
        {
            float sec = 0.028f / ratios[i] * scale;
            int n = juce::jmax (16, (int) (sr * sec));
            n = juce::jmin (n, (int) (sr * 0.08));
            comb[i].assign ((size_t) n, 0.f);
            cpos[i] = 0;
        }
    }

    double sr = 44100.0;
    std::vector<float> comb[kCombs];
    int cpos[kCombs] {};
    float dampZ[kCombs] {};
    float mix = 0.f, amount = 0.5f, shift = 0.f, gate = 0.f, envFollow = 0.f;
    bool freeze = false;
};

/** Engineered multi-segment envelope (MSEG)
 *  - up to 8 points with per-segment curve (lin / exp / log / s-curve)
 *  - loop or one-shot
 *  - bipolar or unipolar output
 *  - shape presets for production use
 */
struct MSEG
{
    static constexpr int maxPts = 8;
    enum class Curve : int { Linear = 0, Exp, Log, Smooth };

    void prepare (double sampleRate) noexcept { sr = sampleRate > 0 ? sampleRate : 44100.0; phase = 0; }
    void reset() noexcept { phase = 0.0; finished = false; }
    void trigger() noexcept { phase = 0.0; finished = false; }
    void setRateHz (float hz) noexcept { rate = juce::jlimit (0.01f, 40.f, hz); }
    void setLoop (bool l) noexcept { loop = l; }
    void setBipolar (bool b) noexcept { bipolar = b; }
    void setCurve (Curve c) noexcept { curve = c; }
    void setPoint (int i, float t, float v) noexcept
    {
        if (i < 0 || i >= maxPts) return;
        ptsT[i] = juce::jlimit (0.f, 1.f, t);
        ptsV[i] = juce::jlimit (-1.f, 1.f, v);
        nPts = juce::jmax (nPts, i + 1);
        sortPoints();
    }
    void setNumPoints (int n) noexcept { nPts = juce::jlimit (2, maxPts, n); }

    float processBlock (int numSamples) noexcept
    {
        if (finished && ! loop) return bipolar ? 0.f : ptsV[juce::jmax (0, nPts - 1)];
        float v = eval ((float) phase);
        phase += ((double) rate / sr) * (double) juce::jmax (1, numSamples);
        if (loop)
        {
            while (phase >= 1.0) phase -= 1.0;
        }
        else if (phase >= 1.0)
        {
            phase = 1.0;
            finished = true;
            v = ptsV[juce::jmax (0, nPts - 1)];
        }
        // bipolar: leave -1..1; unipolar shapes already use 0..1 points
        if (bipolar)
            ; // keep signed
        return flushDenormal (v);
    }

    float eval (float t) const noexcept
    {
        if (nPts < 2) return 0.f;
        t = juce::jlimit (0.f, 1.f, t);
        for (int i = 0; i < nPts - 1; ++i)
        {
            float t0 = ptsT[i], t1 = ptsT[i + 1];
            if (t1 <= t0) continue;
            if (t >= t0 && (t <= t1 || i == nPts - 2))
            {
                float f = (t - t0) / (t1 - t0);
                f = juce::jlimit (0.f, 1.f, f);
                switch (curve)
                {
                    case Curve::Exp:    f = f * f; break;
                    case Curve::Log:    f = 1.f - (1.f - f) * (1.f - f); break;
                    case Curve::Smooth: f = f * f * (3.f - 2.f * f); break;
                    default: break;
                }
                return ptsV[i] + (ptsV[i + 1] - ptsV[i]) * f;
            }
        }
        return ptsV[nPts - 1];
    }

    void sortPoints() noexcept
    {
        // simple insertion keep time ordered
        for (int i = 1; i < nPts; ++i)
        {
            float t = ptsT[i], v = ptsV[i];
            int j = i;
            while (j > 0 && ptsT[j - 1] > t)
            {
                ptsT[j] = ptsT[j - 1];
                ptsV[j] = ptsV[j - 1];
                --j;
            }
            ptsT[j] = t; ptsV[j] = v;
        }
        ptsT[0] = 0.f;
        ptsT[nPts - 1] = 1.f;
    }

    void loadADSRShape() noexcept
    {
        nPts = 4; bipolar = false; curve = Curve::Smooth;
        ptsT[0] = 0.f;   ptsV[0] = 0.f;
        ptsT[1] = 0.12f; ptsV[1] = 1.f;
        ptsT[2] = 0.35f; ptsV[2] = 0.55f;
        ptsT[3] = 1.f;   ptsV[3] = 0.f;
    }
    void loadRampUp() noexcept
    {
        nPts = 2; bipolar = false; curve = Curve::Exp;
        ptsT[0] = 0.f; ptsV[0] = 0.f;
        ptsT[1] = 1.f; ptsV[1] = 1.f;
    }
    void loadTriangle() noexcept
    {
        nPts = 3; bipolar = true; curve = Curve::Linear;
        ptsT[0] = 0.f;  ptsV[0] = -1.f;
        ptsT[1] = 0.5f; ptsV[1] = 1.f;
        ptsT[2] = 1.f;  ptsV[2] = -1.f;
    }
    void loadHitechBurst() noexcept
    {
        nPts = 6; bipolar = false; curve = Curve::Smooth;
        ptsT[0] = 0.f;   ptsV[0] = 0.f;
        ptsT[1] = 0.05f; ptsV[1] = 1.f;
        ptsT[2] = 0.12f; ptsV[2] = 0.2f;
        ptsT[3] = 0.2f;  ptsV[3] = 0.9f;
        ptsT[4] = 0.45f; ptsV[4] = 0.35f;
        ptsT[5] = 1.f;   ptsV[5] = 0.f;
    }

    double sr = 44100.0, phase = 0;
    float rate = 1.f;
    float ptsT[maxPts] { 0, 0.33f, 0.66f, 1.f };
    float ptsV[maxPts] { 0, 1.f, 0.5f, 0.f };
    int nPts = 4;
    bool loop = true, bipolar = false, finished = false;
    Curve curve = Curve::Smooth;
};

} // namespace shae
} // namespace salek

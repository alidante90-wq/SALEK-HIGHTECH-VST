#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>

namespace salek
{

/** Multi-mode filter bank — SVF core + cascade / comb / formant approximations.
    Real-time safe. Modes 0..15 selectable from MAIN UI. */
class StateVariableFilter
{
public:
    enum class Mode
    {
        LowPass12 = 0,
        LowPass24,
        HighPass12,
        HighPass24,
        BandPass,
        Notch,
        Peak,
        AllPass,
        AcidLP,       // high-res LP with extra drive
        LadderSoft,   // softer 24dB-ish
        Comb,
        Formant,      // dual-peak vocal-ish
        BandReject,
        LowShelf,
        HighShelf,
        PhaserNotch   // moving-ish dual notch character
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept
    {
        ic1eq = ic2eq = 0.0f;
        ic1b = ic2b = 0.0f;
        combPos = 0;
        for (auto& s : combBuf) s = 0.f;
    }

    void setCutoff (float hz) noexcept
    {
        cutoff = juce::jlimit (20.0f, static_cast<float> (sr * 0.45), hz);
        update();
    }

    void setResonance (float res) noexcept
    {
        resonance = juce::jlimit (0.0f, 1.0f, res);
        update();
    }

    void setMode (Mode m) noexcept { mode = m; }
    void setModeIndex (int m) noexcept
    {
        mode = static_cast<Mode> (juce::jlimit (0, 15, m));
    }

    void setDrive (float d) noexcept
    {
        drive = juce::jlimit (0.0f, 1.0f, d);
    }

    float process (float x) noexcept
    {
        // Pre-drive (stronger for Acid / Ladder)
        float preDrive = drive;
        if (mode == Mode::AcidLP || mode == Mode::LadderSoft)
            preDrive = juce::jmin (1.f, drive + 0.25f);

        if (preDrive > 1.0e-4f)
        {
            const float g = 1.0f + preDrive * 5.0f;
            x = std::tanh (x * g) * (1.0f / std::tanh (g * 0.65f + 0.35f));
        }

        if (mode == Mode::Comb)
            return processComb (x);

        // Primary SVF tick
        float y = tickSvf (x, ic1eq, ic2eq, a1, a2, a3, k);

        switch (mode)
        {
            case Mode::LowPass12:
            case Mode::AcidLP:
                y = v2last; break;
            case Mode::LowPass24:
            case Mode::LadderSoft:
            {
                // cascade second stage at slightly higher cutoff
                float y2 = tickSvf (v2last, ic1b, ic2b, a1b, a2b, a3b, k);
                y = (mode == Mode::LadderSoft) ? (v2last * 0.35f + y2 * 0.65f) : y2;
                break;
            }
            case Mode::HighPass12:
                y = x - k * v1last - v2last; break;
            case Mode::HighPass24:
            {
                float hp1 = x - k * v1last - v2last;
                float y2 = tickSvf (hp1, ic1b, ic2b, a1b, a2b, a3b, k);
                y = hp1 - k * v1lastb - y2; // approx
                juce::ignoreUnused (y2);
                y = hp1; // stable HP cascade approx
                float hp2 = tickSvf (hp1, ic1b, ic2b, a1b, a2b, a3b, k);
                y = hp1 - k * v1lastb - hp2 * 0.0f;
                y = hp1 - (k * 0.5f) * v1lastb; // keep stable
                break;
            }
            case Mode::BandPass:
                y = v1last; break;
            case Mode::Notch:
            case Mode::BandReject:
                y = x - k * v1last; break;
            case Mode::Peak:
                y = x + v1last * (0.5f + resonance); break;
            case Mode::AllPass:
                y = x - 2.f * k * v1last - 2.f * v2last + v2last; // simple AP
                y = x - 2.f * v1last * (0.3f + resonance * 0.5f);
                break;
            case Mode::Formant:
            {
                // dual band peaks around cut and 1.6*cut
                float p1 = v1last;
                float y2 = tickSvf (x, ic1b, ic2b, a1b, a2b, a3b, k);
                y = (p1 + v1lastb) * (0.55f + resonance * 0.4f);
                juce::ignoreUnused (y2);
                break;
            }
            case Mode::LowShelf:
                y = x * (1.f - 0.4f * resonance) + v2last * (0.5f + resonance * 0.5f);
                break;
            case Mode::HighShelf:
                y = x * (0.5f + resonance * 0.4f) + (x - v2last) * 0.5f;
                break;
            case Mode::PhaserNotch:
                y = x - k * v1last - 0.35f * v2last;
                break;
            default:
                y = v2last; break;
        }

        if (resonance > 0.4f && mode != Mode::AllPass)
        {
            const float boost = 1.0f + (resonance - 0.4f) * 1.8f;
            y *= boost;
        }
        y = std::tanh (y * (1.15f + drive * 0.8f));
        return y;
    }

private:
    float tickSvf (float x, float& i1, float& i2,
                   float aa1, float aa2, float aa3, float kk) noexcept
    {
        const float v3 = x - i2;
        const float v1 = aa1 * i1 + aa2 * v3;
        const float v2 = i2 + aa2 * i1 + aa3 * v3;
        i1 = 2.0f * v1 - i1;
        i2 = 2.0f * v2 - i2;
        v1last = v1;
        v2last = v2;
        // secondary last for stage B tracked when using ic1b
        if (&i1 == &ic1b) { v1lastb = v1; v2lastb = v2; }
        juce::ignoreUnused (kk);
        return v2;
    }

    float processComb (float x) noexcept
    {
        const int maxD = (int) combBuf.size() - 1;
        int d = juce::jlimit (1, maxD, (int) (sr / juce::jmax (30.f, cutoff)));
        float delayed = combBuf[(size_t) ((combPos - d + (int) combBuf.size()) % (int) combBuf.size())];
        float fb = 0.2f + resonance * 0.75f;
        float y = x + delayed * fb;
        combBuf[(size_t) combPos] = y * 0.95f;
        combPos = (combPos + 1) % (int) combBuf.size();
        return std::tanh (y * (1.f + drive));
    }

    void update() noexcept
    {
        const float g = std::tan (juce::MathConstants<float>::pi * cutoff / static_cast<float> (sr));
        k = 2.0f - 1.92f * resonance;
        if (k < 0.08f) k = 0.08f;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;

        // second stage slightly offset for 24dB / formant
        float cut2 = juce::jmin (static_cast<float> (sr * 0.45), cutoff * 1.35f);
        const float g2 = std::tan (juce::MathConstants<float>::pi * cut2 / static_cast<float> (sr));
        a1b = 1.0f / (1.0f + g2 * (g2 + k));
        a2b = g2 * a1b;
        a3b = g2 * a2b;
    }

    double sr = 44100.0;
    float cutoff = 1000.0f;
    float resonance = 0.2f;
    float drive = 0.0f;
    Mode mode = Mode::LowPass12;
    float ic1eq = 0.0f, ic2eq = 0.0f;
    float ic1b = 0.0f, ic2b = 0.0f;
    float k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float a1b = 0.0f, a2b = 0.0f, a3b = 0.0f;
    float v1last = 0.f, v2last = 0.f, v1lastb = 0.f, v2lastb = 0.f;
    std::array<float, 2048> combBuf {};
    int combPos = 0;
};

} // namespace salek

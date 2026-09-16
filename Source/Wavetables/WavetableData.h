#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include <cmath>

namespace salek
{

/** Single wavetable frame (one cycle). Power-of-two size for fast masking. */
struct WavetableFrame
{
    static constexpr int tableSize = 2048;
    static constexpr int tableMask = tableSize - 1;
    std::array<float, tableSize> samples {};

    /** Linear interpolation. phase in [0, 1). */
    float getSample (float phase) const noexcept
    {
        const float pos = phase * static_cast<float> (tableSize);
        const int i0 = static_cast<int> (pos) & tableMask;
        const int i1 = (i0 + 1) & tableMask;
        const float frac = pos - static_cast<float> (static_cast<int> (pos));
        return samples[static_cast<size_t> (i0)] * (1.0f - frac)
             + samples[static_cast<size_t> (i1)] * frac;
    }

    void normalize() noexcept
    {
        float peak = 0.0f;
        for (float s : samples)
            peak = juce::jmax (peak, std::abs (s));
        if (peak > 1.0e-6f)
        {
            const float inv = 1.0f / peak;
            for (float& s : samples)
                s *= inv;
        }
    }
};

/** Multi-frame wavetable with continuous morph / position. */
class Wavetable
{
public:
    static constexpr int maxFrames = 64;

    Wavetable()
    {
        generateDefaultTables();
    }

    void setNumFrames (int n)
    {
        n = juce::jlimit (1, maxFrames, n);
        frames.resize (static_cast<size_t> (n));
    }

    int getNumFrames() const noexcept { return static_cast<int> (frames.size()); }

    WavetableFrame& getFrame (int index)
    {
        return frames[static_cast<size_t> (juce::jlimit (0, getNumFrames() - 1, index))];
    }

    const WavetableFrame& getFrame (int index) const
    {
        return frames[static_cast<size_t> (juce::jlimit (0, getNumFrames() - 1, index))];
    }

    /** Interpolated sample. tablePos [0,1], phase [0,1). */
    float getSample (float tablePos, float phase) const noexcept
    {
        const int n = getNumFrames();
        if (n <= 0)
            return 0.0f;
        if (n == 1)
            return frames[0].getSample (phase);

        const float scaled = juce::jlimit (0.0f, 1.0f, tablePos) * static_cast<float> (n - 1);
        const int f0 = static_cast<int> (scaled);
        const int f1 = juce::jmin (f0 + 1, n - 1);
        const float frac = scaled - static_cast<float> (f0);

        const float s0 = frames[static_cast<size_t> (f0)].getSample (phase);
        const float s1 = frames[static_cast<size_t> (f1)].getSample (phase);
        return s0 + frac * (s1 - s0);
    }

    /** Clear a frame to silence. */
    void clearFrame (int index) noexcept
    {
        if (index < 0 || index >= getNumFrames()) return;
        frames[static_cast<size_t>(index)].samples.fill (0.0f);
    }

    /** Set a single sample in a frame (for drawing). */
    void setSample (int frameIdx, int sampleIdx, float value) noexcept
    {
        if (frameIdx < 0 || frameIdx >= getNumFrames()) return;
        sampleIdx = sampleIdx & WavetableFrame::tableMask;
        frames[static_cast<size_t>(frameIdx)].samples[static_cast<size_t>(sampleIdx)] =
            juce::jlimit (-1.0f, 1.0f, value);
    }

    /** Generate frame from harmonic amplitudes (0 = fundamental). Real additive synthesis. */
    void generateFromHarmonics (int frameIdx, const float* amps, int numHarmonics) noexcept
    {
        if (frameIdx < 0 || frameIdx >= getNumFrames() || amps == nullptr || numHarmonics <= 0)
            return;

        auto& frame = frames[static_cast<size_t>(frameIdx)];
        for (int i = 0; i < WavetableFrame::tableSize; ++i)
        {
            const float phase = static_cast<float>(i) / static_cast<float>(WavetableFrame::tableSize)
                                * juce::MathConstants<float>::twoPi;
            float s = 0.0f;
            for (int h = 0; h < numHarmonics; ++h)
            {
                if (std::abs (amps[h]) > 1.0e-6f)
                    s += amps[h] * std::sin (phase * static_cast<float>(h + 1));
            }
            frame.samples[static_cast<size_t>(i)] = s;
        }
        frame.normalize();
    }

    /** Morph two frames into a third (or overwrite). */
    void morphFrames (int srcA, int srcB, int dest, float t) noexcept
    {
        const int n = getNumFrames();
        if (srcA < 0 || srcA >= n || srcB < 0 || srcB >= n || dest < 0 || dest >= n) return;
        t = juce::jlimit (0.0f, 1.0f, t);
        auto& a = frames[static_cast<size_t>(srcA)];
        auto& b = frames[static_cast<size_t>(srcB)];
        auto& d = frames[static_cast<size_t>(dest)];
        for (int i = 0; i < WavetableFrame::tableSize; ++i)
            d.samples[static_cast<size_t>(i)] =
                a.samples[static_cast<size_t>(i)] * (1.0f - t) + b.samples[static_cast<size_t>(i)] * t;
        d.normalize();
    }

    /** Soft saturate / wavefold an entire frame in-place. */
    void processFrame (int frameIdx, float foldAmt, float driveAmt) noexcept
    {
        if (frameIdx < 0 || frameIdx >= getNumFrames()) return;
        auto& frame = frames[static_cast<size_t>(frameIdx)];
        for (float& s : frame.samples)
        {
            if (foldAmt > 1.0e-4f)
            {
                float x = s * (1.0f + foldAmt * 3.0f);
                const float thr = 1.0f - foldAmt * 0.7f;
                for (int k = 0; k < 2; ++k)
                {
                    if (x > thr) x = thr - (x - thr);
                    else if (x < -thr) x = -thr - (x + thr);
                    else break;
                }
                s = x;
            }
            if (driveAmt > 1.0e-4f)
                s = std::tanh (s * (1.0f + driveAmt * 4.0f));
        }
        frame.normalize();
    }

    /** Rebuild factory tables optimised for hi-tech / psy / alien / metallic. */
    void generateDefaultTables()
    {
        frames.resize (8);

        for (int i = 0; i < WavetableFrame::tableSize; ++i)
        {
            const float t = static_cast<float> (i) / static_cast<float> (WavetableFrame::tableSize);
            const float phase = t * juce::MathConstants<float>::twoPi;
            const size_t idx = static_cast<size_t> (i);

            // 0 — Clean Saw (additive, limited harmonics)
            {
                float s = 0.0f;
                for (int h = 1; h <= 32; ++h)
                    s += std::sin (static_cast<float> (h) * phase) / static_cast<float> (h);
                frames[0].samples[idx] = s * 0.55f;
            }

            // 1 — Soft Square / Pulse
            {
                float s = 0.0f;
                for (int h = 1; h <= 31; h += 2)
                    s += std::sin (static_cast<float> (h) * phase) / static_cast<float> (h);
                frames[1].samples[idx] = s * 0.7f;
            }

            // 2 — Triangle (odd harmonics 1/h²)
            {
                float s = 0.0f;
                for (int h = 1; h <= 15; h += 2)
                {
                    const float sign = ((h - 1) / 2) % 2 == 0 ? 1.0f : -1.0f;
                    s += sign * std::sin (static_cast<float> (h) * phase)
                         / static_cast<float> (h * h);
                }
                frames[2].samples[idx] = s * 0.9f;
            }

            // 3 — Sine + strong 2nd/3rd (warm fundamental)
            {
                float s = std::sin (phase);
                s += 0.45f * std::sin (2.0f * phase);
                s += 0.22f * std::sin (3.0f * phase);
                frames[3].samples[idx] = s * 0.75f;
            }

            // 4 — Metallic / inharmonic-ish (odd + selected high)
            {
                float s = std::sin (phase);
                s += 0.4f * std::sin (3.0f * phase);
                s += 0.25f * std::sin (5.0f * phase);
                s += 0.18f * std::sin (7.0f * phase);
                s += 0.12f * std::sin (11.0f * phase);
                s += 0.08f * std::sin (13.0f * phase);
                frames[4].samples[idx] = s * 0.65f;
            }

            // 5 — Alien / formant-ish (clustered harmonics)
            {
                float s = std::sin (phase);
                s += 0.55f * std::sin (4.0f * phase);
                s += 0.35f * std::sin (5.0f * phase);
                s += 0.2f  * std::sin (8.0f * phase);
                s += 0.15f * std::sin (9.0f * phase);
                s += 0.1f  * std::sin (12.0f * phase);
                frames[5].samples[idx] = s * 0.6f;
            }

            // 6 — Aggressive / screech potential (bright)
            {
                float s = 0.0f;
                for (int h = 1; h <= 48; ++h)
                {
                    const float amp = 1.0f / (1.0f + 0.15f * static_cast<float> (h));
                    s += amp * std::sin (static_cast<float> (h) * phase);
                }
                frames[6].samples[idx] = s * 0.35f;
            }

            // 7 — Cyber / digital edge (saw + pulse mix flavour)
            {
                float saw = 0.0f;
                for (int h = 1; h <= 24; ++h)
                    saw += std::sin (static_cast<float> (h) * phase) / static_cast<float> (h);
                float pulse = 0.0f;
                for (int h = 1; h <= 31; h += 2)
                    pulse += std::sin (static_cast<float> (h) * phase) / static_cast<float> (h);
                frames[7].samples[idx] = (0.6f * saw + 0.4f * pulse) * 0.5f;
            }
        }

        for (auto& f : frames)
            f.normalize();
    }

private:
    std::vector<WavetableFrame> frames;
};

} // namespace salek

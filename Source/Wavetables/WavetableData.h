#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include <cmath>

namespace salek
{

struct WavetableFrame
{
    static constexpr int tableSize = 2048;
    static constexpr int tableMask = tableSize - 1;
    std::array<float, tableSize> samples {};

    float getSample (float phase) const noexcept
    {
        // Catmull-Rom cubic — cleaner high-end than linear (hi-tech / crystal clarity)
        const float pos = phase * static_cast<float> (tableSize);
        const int i1 = static_cast<int> (pos) & tableMask;
        const int i0 = (i1 - 1) & tableMask;
        const int i2 = (i1 + 1) & tableMask;
        const int i3 = (i1 + 2) & tableMask;
        const float frac = pos - std::floor (pos);
        const float y0 = samples[static_cast<size_t> (i0)];
        const float y1 = samples[static_cast<size_t> (i1)];
        const float y2 = samples[static_cast<size_t> (i2)];
        const float y3 = samples[static_cast<size_t> (i3)];
        const float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
        const float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float a2 = -0.5f * y0 + 0.5f * y2;
        const float a3 = y1;
        return ((a0 * frac + a1) * frac + a2) * frac + a3;
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

class Wavetable
{
public:
    static constexpr int maxFrames = 64;

    Wavetable() { generateDefaultTables(); }

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

    float getSample (float tablePos, float phase) const noexcept
    {
        const int n = getNumFrames();
        if (n <= 0) return 0.0f;
        if (n == 1) return frames[0].getSample (phase);

        const float scaled = juce::jlimit (0.0f, 1.0f, tablePos) * static_cast<float> (n - 1);
        const int f0 = static_cast<int> (scaled);
        const int f1 = juce::jmin (f0 + 1, n - 1);
        const float frac = scaled - static_cast<float> (f0);
        const float s0 = frames[static_cast<size_t> (f0)].getSample (phase);
        const float s1 = frames[static_cast<size_t> (f1)].getSample (phase);
        return s0 * (1.0f - frac) + s1 * frac;
    }

    void clearFrame (int index)
    {
        if (index < 0 || index >= getNumFrames()) return;
        frames[static_cast<size_t>(index)].samples.fill (0.0f);
    }

    void setSample (int frameIdx, int sampleIdx, float value)
    {
        if (frameIdx < 0 || frameIdx >= getNumFrames()) return;
        frames[static_cast<size_t>(frameIdx)].samples[static_cast<size_t>(sampleIdx & WavetableFrame::tableMask)] = value;
    }

    void generateFromHarmonics (int frameIdx, const float* amps, int numHarmonics) noexcept
    {
        if (frameIdx < 0 || frameIdx >= getNumFrames() || amps == nullptr || numHarmonics <= 0)
            return;
        auto& frame = frames[static_cast<size_t>(frameIdx)];
        frame.samples.fill (0.0f);
        for (int i = 0; i < WavetableFrame::tableSize; ++i)
        {
            const float phase = static_cast<float> (i) / static_cast<float> (WavetableFrame::tableSize)
                              * juce::MathConstants<float>::twoPi;
            float s = 0.0f;
            for (int h = 0; h < numHarmonics; ++h)
                s += amps[h] * std::sin (phase * static_cast<float> (h + 1));
            frame.samples[static_cast<size_t>(i)] = s;
        }
        frame.normalize();
    }

    void morphFrames (int srcA, int srcB, int dest, float t) noexcept
    {
        const int n = getNumFrames();
        if (srcA < 0 || srcB < 0 || dest < 0 || srcA >= n || srcB >= n || dest >= n) return;
        t = juce::jlimit (0.0f, 1.0f, t);
        auto& a = frames[static_cast<size_t>(srcA)];
        auto& b = frames[static_cast<size_t>(srcB)];
        auto& d = frames[static_cast<size_t>(dest)];
        for (int i = 0; i < WavetableFrame::tableSize; ++i)
            d.samples[static_cast<size_t>(i)] = a.samples[static_cast<size_t>(i)] * (1.0f - t)
                                              + b.samples[static_cast<size_t>(i)] * t;
        d.normalize();
    }

    void processFrame (int frameIdx, float foldAmt, float driveAmt) noexcept
    {
        if (frameIdx < 0 || frameIdx >= getNumFrames()) return;
        auto& frame = frames[static_cast<size_t>(frameIdx)];
        for (float& s : frame.samples)
        {
            if (foldAmt > 1e-4f)
            {
                float thresh = 1.0f - foldAmt * 0.85f;
                float x = s * (1.0f + foldAmt * 4.0f);
                for (int k = 0; k < 3; ++k)
                {
                    if (x > thresh) x = thresh - (x - thresh);
                    else if (x < -thresh) x = -thresh - (x + thresh);
                    else break;
                }
                s = x / (1.0f + foldAmt * 1.5f);
            }
            if (driveAmt > 1e-4f)
                s = std::tanh (s * (1.0f + driveAmt * 6.0f));
        }
        frame.normalize();
    }

    void generateDefaultTables()
    {
        // Big bank: 128 frames — analog-style morph (sine→tri→sqr→saw→complex→metallic)
        frames.resize (128); // wide bank
        for (int i = 0; i < WavetableFrame::tableSize; ++i)
        {
            const float t = static_cast<float> (i) / static_cast<float> (WavetableFrame::tableSize);
            const float phase = t * juce::MathConstants<float>::twoPi;
            const size_t idx = static_cast<size_t> (i);
            auto addSaw = [&](int harm) {
                float s = 0.0f;
                for (int h = 1; h <= harm; ++h)
                    s += std::sin (static_cast<float> (h) * phase) / static_cast<float> (h);
                return s;
            };
            auto addSqr = [&](int harm) {
                float s = 0.0f;
                for (int h = 1; h <= harm; h += 2)
                    s += std::sin (static_cast<float> (h) * phase) / static_cast<float> (h);
                return s;
            };
            auto addTri = [&](int harm) {
                float s = 0.0f;
                for (int h = 1; h <= harm; h += 2) {
                    float sign = ((h - 1) / 2) % 2 == 0 ? 1.0f : -1.0f;
                    s += sign * std::sin ((float) h * phase) / (float) (h * h);
                }
                return s;
            };

            // 0–15 classic morph (kept + refined)
            frames[0].samples[idx]  = std::sin (phase);
            frames[1].samples[idx]  = addTri (15) * 0.95f;
            frames[2].samples[idx]  = addSqr (15) * 0.9f;
            frames[3].samples[idx]  = addSaw (32) * 0.85f;
            frames[4].samples[idx]  = addSaw (48) * 0.7f;
            frames[5].samples[idx]  = (addSqr (31) * 0.7f + addSaw (16) * 0.3f) * 0.85f;
            frames[6].samples[idx]  = (std::sin (phase) + 0.35f * std::sin (3.f * phase) + 0.2f * std::sin (5.f * phase)) * 0.75f;
            frames[7].samples[idx]  = (std::sin (phase) + 0.28f * std::sin (4.f * phase) + 0.2f * std::sin (8.f * phase)) * 0.7f;
            frames[8].samples[idx]  = std::floor (addSaw (20) * 6.f) / 6.f * 0.8f;
            frames[9].samples[idx]  = addSqr (41) * 0.75f;
            frames[10].samples[idx] = (std::sin (phase) * 0.85f + 0.4f * std::sin (2.f * phase) + 0.15f * std::sin (3.f * phase));
            {
                float s = 0.f;
                for (int h = 1; h <= 64; ++h)
                    s += (1.f / (1.f + 0.12f * (float) h)) * std::sin ((float) h * phase);
                frames[11].samples[idx] = s * 0.4f;
            }
            frames[12].samples[idx] = (std::sin (phase) + 0.5f * std::sin (6.f * phase) + 0.3f * std::sin (7.f * phase)) * 0.7f;
            frames[13].samples[idx] = (std::sin (phase) - 0.6f * std::sin (2.f * phase) + 0.3f * std::sin (5.f * phase)) * 0.75f;
            frames[14].samples[idx] = (0.55f * addSaw (24) + 0.45f * addSqr (31)) * 0.85f;
            frames[15].samples[idx] = (addSaw (8) + 0.15f * std::sin (23.f * phase) + 0.1f * std::sin (29.f * phase)) * 0.7f;

            // 16–31 Virus-style / analog fat morphs
            frames[16].samples[idx] = addSaw (12) * 0.9f;                          // warm saw
            frames[17].samples[idx] = addSaw (24) * 0.85f;
            frames[18].samples[idx] = addSaw (64) * 0.65f;                          // bright saw
            frames[19].samples[idx] = addSqr (9) * 0.9f;                            // soft square
            frames[20].samples[idx] = addSqr (25) * 0.8f;
            frames[21].samples[idx] = (addSaw (20) * 0.6f + addSqr (15) * 0.4f) * 0.85f;
            frames[22].samples[idx] = addTri (31) * 0.9f;
            frames[23].samples[idx] = (addTri (15) * 0.5f + addSaw (16) * 0.5f) * 0.85f;
            // pulse-ish
            frames[24].samples[idx] = (t < 0.25f ? 0.9f : (t < 0.5f ? -0.3f : (t < 0.75f ? 0.5f : -0.9f)));
            frames[25].samples[idx] = (t < 0.125f ? 1.f : -0.35f) * 0.85f;         // narrow pulse
            // formant-ish
            frames[26].samples[idx] = (std::sin (phase) + 0.55f * std::sin (3.f * phase) + 0.35f * std::sin (5.f * phase)
                                       + 0.2f * std::sin (7.f * phase)) * 0.65f;
            frames[27].samples[idx] = (std::sin (phase) + 0.4f * std::sin (5.f * phase) + 0.3f * std::sin (9.f * phase)) * 0.7f;
            // metallic / partial clusters
            frames[28].samples[idx] = (std::sin (phase) + 0.45f * std::sin (2.7f * phase) + 0.3f * std::sin (4.1f * phase)
                                       + 0.2f * std::sin (6.3f * phase)) * 0.65f;
            frames[29].samples[idx] = (std::sin (phase) + 0.5f * std::sin (3.3f * phase) + 0.35f * std::sin (7.1f * phase)) * 0.6f;
            // bit-crushed morph
            frames[30].samples[idx] = std::floor (addSaw (32) * 4.f) / 4.f * 0.85f;
            frames[31].samples[idx] = std::floor (addSqr (21) * 8.f) / 8.f * 0.8f;

            // 32–47 hi-tech / alien / screech material
            for (int f = 32; f < 48; ++f)
            {
                float morph = (float) (f - 32) / 15.f;
                float s = 0.f;
                int harm = 8 + (int) (morph * 48.f);
                for (int h = 1; h <= harm; ++h)
                {
                    float amp = 1.f / (1.f + 0.08f * h * (1.f + morph));
                    float det = 1.f + morph * 0.15f * std::sin ((float) h);
                    s += amp * std::sin ((float) h * det * phase);
                }
                // add odd harmonics for scream
                if (morph > 0.4f)
                    s += 0.3f * morph * std::sin (11.f * phase) + 0.2f * morph * std::sin (13.f * phase);
                frames[(size_t) f].samples[idx] = s * (0.55f - morph * 0.15f);
            }

            // 48–63 darkpsy / supersaw-ish / folded
            for (int f = 48; f < 64; ++f)
            {
                float morph = (float) (f - 48) / 15.f;
                float s = addSaw (16 + (int) (morph * 40.f));
                s = std::tanh (s * (1.2f + morph * 2.5f));
                s += 0.12f * morph * std::sin (2.f * phase);
                s += 0.08f * morph * std::sin (4.f * phase);
                frames[(size_t) f].samples[idx] = s * 0.7f;
            }
            // 64–95 formant / vowel / vocal-ish
            for (int f = 64; f < 96; ++f)
            {
                float morph = (float) (f - 64) / 31.f;
                float f1 = 0.8f + morph * 2.5f;
                float f2 = 2.2f + morph * 4.f;
                float f3 = 5.0f + morph * 6.f;
                float s = std::sin (phase)
                        + 0.55f * std::sin (f1 * phase)
                        + 0.35f * std::sin (f2 * phase)
                        + 0.2f * std::sin (f3 * phase);
                s *= (0.55f + 0.2f * std::sin (3.f * phase + morph));
                frames[(size_t) f].samples[idx] = s * 0.55f;
            }
            // 96–127 metallic / sync / FM-ish / glassy
            for (int f = 96; f < 128; ++f)
            {
                float morph = (float) (f - 96) / 31.f;
                float mod = std::sin ((2.f + morph * 8.f) * phase);
                float s = std::sin (phase + morph * 2.5f * mod);
                s += 0.25f * morph * std::sin ((7.f + morph * 11.f) * phase);
                s = std::tanh (s * (1.1f + morph));
                frames[(size_t) f].samples[idx] = s * 0.6f;
            }
        }
        for (auto& f : frames)
            f.normalize();
    }

private:
    std::vector<WavetableFrame> frames;
};

} // namespace salek

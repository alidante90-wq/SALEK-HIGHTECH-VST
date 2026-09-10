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
        frames.resize (16);
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
            frames[0].samples[idx] = (std::sin(phase) + 0.25f*std::sin(2.f*phase)) * 0.9f;
            {
                float s = 0.0f;
                for (int h = 1; h <= 15; h += 2) {
                    float sign = ((h - 1) / 2) % 2 == 0 ? 1.0f : -1.0f;
                    s += sign * std::sin((float)h * phase) / (float)(h * h);
                }
                frames[1].samples[idx] = s * 0.95f;
            }
            frames[2].samples[idx] = addSqr(15) * 0.9f;
            frames[3].samples[idx] = addSaw(32) * 0.85f;
            frames[4].samples[idx] = addSaw(48) * 0.7f;
            frames[5].samples[idx] = (addSqr(31) * 0.7f + addSaw(16) * 0.3f) * 0.85f;
            {
                float s = std::sin(phase);
                for (int h : {3,5,7,11,13,17})
                    s += (0.35f / (float)(h/2+1)) * std::sin((float)h * phase);
                frames[6].samples[idx] = s * 0.7f;
            }
            {
                float s = std::sin(phase);
                for (int h : {4,5,8,9,12,16})
                    s += 0.28f * std::sin((float)h * phase);
                frames[7].samples[idx] = s * 0.65f;
            }
            {
                float s = addSaw(20);
                s = std::floor(s * 6.0f) / 6.0f;
                frames[8].samples[idx] = s * 0.8f;
            }
            frames[9].samples[idx] = addSqr(41) * 0.75f;
            frames[10].samples[idx] = (std::sin(phase)*0.85f + 0.4f*std::sin(2.f*phase) + 0.15f*std::sin(3.f*phase));
            {
                float s = 0.0f;
                for (int h = 1; h <= 64; ++h)
                    s += (1.0f / (1.0f + 0.12f*(float)h)) * std::sin((float)h * phase);
                frames[11].samples[idx] = s * 0.4f;
            }
            {
                float s = std::sin(phase);
                s += 0.5f * std::sin(6.f*phase) + 0.3f*std::sin(7.f*phase) + 0.2f*std::sin(11.f*phase);
                frames[12].samples[idx] = s * 0.7f;
            }
            {
                float s = std::sin(phase) - 0.6f*std::sin(2.f*phase) + 0.3f*std::sin(5.f*phase);
                frames[13].samples[idx] = s * 0.75f;
            }
            frames[14].samples[idx] = (0.55f*addSaw(24) + 0.45f*addSqr(31)) * 0.85f;
            {
                float s = addSaw(8);
                s += 0.15f * std::sin(23.f*phase) + 0.1f*std::sin(29.f*phase) + 0.08f*std::sin(31.f*phase);
                frames[15].samples[idx] = s * 0.7f;
            }
        }
        for (auto& f : frames)
            f.normalize();
    }

private:
    std::vector<WavetableFrame> frames;
};

} // namespace salek

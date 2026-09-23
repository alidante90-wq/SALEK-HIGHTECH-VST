#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <limits>

namespace shae
{
/** Lightweight realtime-safe output safety stage used by GITI.
    No heap allocation, locks, filesystem or external proprietary DSP.
*/
class SafetyStage
{
public:
    void prepare (double sampleRate, int channels)
    {
        juce::ignoreUnused (channels);
        fs = sampleRate > 1.0 ? sampleRate : 44100.0;
        x1L = x1R = y1L = y1R = 0.0f;
        const double rc = 1.0 / (juce::MathConstants<double>::twoPi * 18.0);
        hpA = static_cast<float> (rc / (rc + 1.0 / fs));
    }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int n = buffer.getNumSamples();
        const int chs = buffer.getNumChannels();
        if (chs <= 0 || n <= 0) return;

        auto processChannel = [this, n] (float* d, float& x1, float& y1) noexcept
        {
            for (int i = 0; i < n; ++i)
            {
                float x = d[i];
                if (!std::isfinite (x)) x = 0.0f;

                // First-order DC blocker: y[n] = x[n]-x[n-1] + a*y[n-1]
                const float y = x - x1 + hpA * y1;
                x1 = x;
                y1 = y;

                // Never allow a NaN/Inf or an out-of-range sample to escape.
                float z = std::isfinite (y) ? y : 0.0f;
                const float a = std::abs (z);
                if (a > 0.97f)
                    z = std::copysign (0.97f + 0.025f * std::tanh ((a - 0.97f) * 8.0f), z);
                d[i] = juce::jlimit (-0.995f, 0.995f, z);
            }
        };

        processChannel (buffer.getWritePointer (0), x1L, y1L);
        if (chs > 1)
            processChannel (buffer.getWritePointer (1), x1R, y1R);
        for (int ch = 2; ch < chs; ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
                if (!std::isfinite (d[i])) d[i] = 0.0f;
        }
    }

private:
    double fs = 44100.0;
    float hpA = 0.997f;
    float x1L = 0.0f, x1R = 0.0f;
    float y1L = 0.0f, y1R = 0.0f;
};
}

#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
/** Multi-model distortion (Serum-inspired intensity):
 *  0 Tube  1 Sat  2 Hard  3 Fold  4 Bit  5 Rect */
class SimpleDistortion {
public:
    void prepare (double, int) {}
    void setDrive (float d) noexcept { drive = juce::jlimit (0.f, 1.f, d); }
    void setBitcrush (float b) noexcept { bits = juce::jlimit (0.f, 1.f, b); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setMode (int m) noexcept { mode = juce::jlimit (0, 5, m); }
    void process (juce::AudioBuffer<float>& buf) noexcept {
        // Auto wet: if drive is up but mix is 0, still hear colour (Serum-like feel)
        const float wet = juce::jmax (mix, drive * 0.55f);
        if (wet < 1e-4f && bits < 1e-4f) return;
        const int n = buf.getNumSamples();
        const int ch = buf.getNumChannels();
        const float gain = 1.f + drive * 14.f;
        const float bitAmt = (mode == 4) ? juce::jmax (bits, 0.25f + drive * 0.55f) : bits;
        const float levels = bitAmt > 0.01f ? std::pow (2.f, 3.f + (1.f - bitAmt) * 13.f) : 0.f;
        for (int c = 0; c < ch; ++c) {
            float* d = buf.getWritePointer (c);
            for (int i = 0; i < n; ++i) {
                float x = d[i];
                float y = x * gain;
                switch (mode)
                {
                    case 1: // soft sat
                        y = y / (1.f + std::abs (y) * (0.35f + drive * 0.7f));
                        y = std::tanh (y * 1.35f);
                        break;
                    case 2: // hard
                        y = juce::jlimit (-0.95f, 0.95f, y);
                        y = std::tanh (y * 1.1f);
                        break;
                    case 3: // fold
                        y = std::sin (y * juce::MathConstants<float>::halfPi * (1.f + drive * 2.2f));
                        break;
                    case 4: // bit
                        y = std::tanh (y);
                        break;
                    case 5: // rect
                        y = std::abs (std::tanh (y));
                        y = (x >= 0.f) ? y : -y * 0.35f;
                        break;
                    default: // tube
                        y = std::tanh (y);
                        y = y - 0.15f * y * y * y;
                        break;
                }
                if (levels > 1.f)
                    y = std::floor (y * levels + 0.5f) / levels;
                d[i] = x * (1.f - wet) + y * wet;
            }
        }
    }
private:
    float drive = 0.f, bits = 0.f, mix = 0.f;
    int mode = 0;
};
}

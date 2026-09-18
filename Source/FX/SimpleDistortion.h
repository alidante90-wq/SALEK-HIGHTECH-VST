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
        const float wet = juce::jlimit (0.f, 1.f, juce::jmax (mix, drive * 0.85f));
        if (wet < 1e-4f && bits < 1e-4f) return;
        const int n = buf.getNumSamples();
        const int ch = buf.getNumChannels();
        const float gain = 1.f + drive * 22.f;
        const float bitAmt = (mode == 4) ? juce::jmax (bits, 0.2f + drive * 0.7f) : bits;
        const float levels = bitAmt > 0.01f ? std::pow (2.f, 2.5f + (1.f - bitAmt) * 13.5f) : 0.f;
        for (int c = 0; c < ch; ++c) {
            float* d = buf.getWritePointer (c);
            for (int i = 0; i < n; ++i) {
                float x = d[i];
                float y = x * gain;
                switch (mode)
                {
                    case 1:
                        y = y / (1.f + std::abs (y) * (0.25f + drive * 0.9f));
                        y = std::tanh (y * 1.55f);
                        break;
                    case 2:
                        y = juce::jlimit (-0.9f, 0.9f, y * (1.f + drive));
                        y = std::tanh (y * 1.25f);
                        break;
                    case 3:
                        y = std::sin (y * juce::MathConstants<float>::halfPi * (1.2f + drive * 3.0f));
                        break;
                    case 4:
                        y = std::tanh (y);
                        break;
                    case 5:
                        y = std::abs (std::tanh (y));
                        y = (x >= 0.f) ? y : -y * 0.4f;
                        break;
                    default:
                        y = std::tanh (y);
                        y = y - 0.18f * y * y * y;
                        y = std::tanh (y * (1.1f + drive * 0.5f));
                        break;
                }
                if (levels > 1.f)
                    y = std::floor (y * levels + 0.5f) / levels;
                y *= (1.f + drive * 0.35f);
                d[i] = x * (1.f - wet) + y * wet;
            }
        }
    }
private:
    float drive = 0.f, bits = 0.f, mix = 0.f;
    int mode = 0;
};
}

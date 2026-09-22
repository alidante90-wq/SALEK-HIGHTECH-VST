#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "../DSP/SHAE.h"
namespace salek {
/** SALEK Distortion Matrix — 14 modes, optional 2x oversample on hot path */
class SimpleDistortion {
public:
    void prepare (double, int) { os.reset(); }
    void setDrive (float d) noexcept { drive = juce::jlimit (0.f, 1.f, d); }
    void setBitcrush (float b) noexcept { bits = juce::jlimit (0.f, 1.f, b); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setMode (int m) noexcept { mode = juce::jlimit (0, (int) shae::DistMode::NumModes - 1, m); }
    void setOversample (bool on) noexcept { oversample = on; }

    void process (juce::AudioBuffer<float>& buf) noexcept
    {
        if (mix < 1e-4f && drive < 1e-4f) return;
        const float wet = juce::jlimit (0.f, 1.f, juce::jmax (mix, drive * 0.5f));
        const int n = buf.getNumSamples();
        const int ch = buf.getNumChannels();

        auto processOne = [&] (float& L, float& R)
        {
            float yL = shae::processDistSample (L, mode, drive);
            float yR = shae::processDistSample (R, mode, drive);
            if (bits > 0.01f && mode != 8)
            {
                float levels = std::pow (2.f, 2.5f + (1.f - bits) * 13.5f);
                yL = std::floor (yL * levels + 0.5f) / levels;
                yR = std::floor (yR * levels + 0.5f) / levels;
            }
            L = L * (1.f - wet) + yL * wet;
            R = R * (1.f - wet) + yR * wet;
        };

        if (oversample)
        {
            os.process (buf, processOne);
            return;
        }
        for (int i = 0; i < n; ++i)
        {
            float L = buf.getSample (0, i);
            float R = ch > 1 ? buf.getSample (1, i) : L;
            processOne (L, R);
            buf.setSample (0, i, L);
            if (ch > 1) buf.setSample (1, i, R);
        }
    }
private:
    float drive = 0.f, bits = 0.f, mix = 0.f;
    int mode = 0;
    bool oversample = false;
    shae::Oversampler2x os;
};
}

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

namespace salek
{

class SimpleDelay
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate;
        const int maxSamples = static_cast<int> (sampleRate * 2.0) + maxBlock;
        buffer.resize (static_cast<size_t> (maxSamples * 2), 0.0f);
        writePos = 0;
    }

    void setTimeMs (float ms) noexcept
    {
        delaySamples = juce::jlimit (1.0f, static_cast<float> (sr * 1.8), ms * 0.001f * static_cast<float> (sr));
    }

    void setFeedback (float fb) noexcept { feedback = juce::jlimit (0.0f, 0.95f, fb); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1.0e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int bufSize = static_cast<int> (buffer.size() / 2);

        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < juce::jmin (2, ch); ++c)
            {
                float* data = buffer.getWritePointer (c);
                const int readPos = (writePos - static_cast<int> (delaySamples) + bufSize) % bufSize;
                const float delayed = buffer[static_cast<size_t> (readPos * 2 + c)];
                const float in = data[i];
                const float out = in * (1.0f - mix) + delayed * mix;
                data[i] = out;
                buffer[static_cast<size_t> (writePos * 2 + c)] = in + delayed * feedback;
            }
            writePos = (writePos + 1) % bufSize;
        }
    }

private:
    double sr = 44100.0;
    std::vector<float> buffer;
    int writePos = 0;
    float delaySamples = 300.0f;
    float feedback = 0.3f;
    float mix = 0.0f;
};

} // namespace salek

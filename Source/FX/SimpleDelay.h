#pragma once
#include <JuceHeader.h>
#include <vector>
namespace salek {
class SimpleDelay {
public:
    void prepare(double sampleRate, int maxBlock) {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        const int maxS = (int) (sr * 2.0) + maxBlock + 8;
        buffer.assign ((size_t) maxS * 2, 0.f);
        writePos = 0;
        lpL = lpR = 0.f;
    }
    void setTimeMs(float ms) noexcept { delaySamples = juce::jlimit(1.f, float(sr * 1.8), ms * 0.001f * float(sr)); }
    void setFeedback(float fb) noexcept { feedback = juce::jlimit(0.f, 0.95f, fb); }
    void setMix(float m) noexcept { mix = juce::jlimit(0.f, 1.f, m); }
    void process(juce::AudioBuffer<float>& buf) noexcept {
        if (mix < 1e-4f) return;
        int n = buf.getNumSamples(), ch = buf.getNumChannels();
        int bs = (int) buffer.size() / 2;
        if (bs < 4) return;
        // tone on feedback: longer times darker
        const float tone = juce::jlimit (0.2f, 0.85f, 0.75f - delaySamples / float (sr) * 0.25f);
        for (int i = 0; i < n; ++i) {
            for (int c = 0; c < juce::jmin(2, ch); ++c) {
                float* d = buf.getWritePointer(c);
                float ds = delaySamples * (c == 1 ? 1.07f : 1.f);
                int rp = (writePos - (int) ds + bs * 4) % bs;
                float delayed = buffer[(size_t)(rp * 2 + c)];
                // smooth feedback path
                if (c == 0) { lpL += tone * (delayed - lpL); delayed = lpL; }
                else        { lpR += tone * (delayed - lpR); delayed = lpR; }
                float in = d[i];
                d[i] = in * (1.f - mix) + delayed * mix;
                buffer[(size_t)(writePos * 2 + c)] = in + delayed * feedback;
            }
            writePos = (writePos + 1) % bs;
        }
    }
private:
    double sr = 44100;
    std::vector<float> buffer;
    int writePos = 0;
    float delaySamples = 300, feedback = 0.3f, mix = 0;
    float lpL = 0.f, lpR = 0.f;
};
}

#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <vector>
#include <cmath>
namespace salek {
/** Stereo / Ping-Pong / Mono delay with independent L/R times. */
class SimpleDelay {
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        const int maxS = (int) (sr * 2.0) + maxBlock + 16;
        bufL.assign ((size_t) maxS, 0.f);
        bufR.assign ((size_t) maxS, 0.f);
        writePos = 0;
        lpL = lpR = 0.f;
        lowCutL = lowCutR = 0.f;
        delaySamplesL = targetDelayL;
        delaySamplesR = targetDelayR;
        mixSmoothValue = mix;
        feedbackSmoothValue = feedback;
    }

    void reset() noexcept
    {
        std::fill (bufL.begin(), bufL.end(), 0.0f);
        std::fill (bufR.begin(), bufR.end(), 0.0f);
        writePos = 0;
        lpL = lpR = lowCutL = lowCutR = wowPhase = 0.0f;
    }

    void setTimeMs (float ms) noexcept
    {
        targetDelayL = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
        targetDelayR = targetDelayL * 1.07f;
    }
    void setTimeMsL (float ms) noexcept
    {
        targetDelayL = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
    }
    void setTimeMsR (float ms) noexcept
    {
        targetDelayR = juce::jlimit (1.f, float (sr * 1.8), ms * 0.001f * float (sr));
    }
    void setFeedback (float fb) noexcept { feedback = juce::jlimit (0.f, 0.95f, fb); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setTone (float t) noexcept { toneAmount = juce::jlimit (0.f, 1.f, t); }
    void setWow (float w) noexcept { wowAmt = juce::jlimit (0.f, 1.f, w); }
    /** 0 Stereo, 1 PingPong, 2 Mono */
    void setMode (int m) noexcept { mode = juce::jlimit (0, 2, m); }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const int bs = (int) bufL.size();
        if (bs < 8) return;

        const float timeSmooth = 1.0f - std::exp (-1.0f / (0.025f * (float) sr));
        const float mixSmooth = 1.0f - std::exp (-1.0f / (0.012f * (float) sr));
        const float toneHz = juce::jmap (toneAmount, 700.0f, 16000.0f);
        const float tone = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * toneHz / (float) sr);
        const float highPass = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 90.0f / (float) sr);

        for (int i = 0; i < n; ++i)
        {
            delaySamplesL += timeSmooth * (targetDelayL - delaySamplesL);
            delaySamplesR += timeSmooth * (targetDelayR - delaySamplesR);
            mixSmoothValue += mixSmooth * (mix - mixSmoothValue);
            feedbackSmoothValue += timeSmooth * (feedback - feedbackSmoothValue);
            float inL = buffer.getSample (0, i);
            float inR = ch > 1 ? buffer.getSample (1, i) : inL;
            if (mode == 2) // Mono: average input
            {
                const float m = 0.5f * (inL + inR);
                inL = inR = m;
            }

            wowPhase += 0.0007f;
            const float wow = 1.f + wowAmt * 0.015f * std::sin (wowPhase * 6.283185f);
            float dL = readFrac (bufL, delaySamplesL * wow);
            float dR = readFrac (bufR, delaySamplesR * (2.f - wow));

            lpL += tone * (dL - lpL);
            lpR += tone * (dR - lpR);
            dL = lpL;
            dR = lpR;
            lowCutL += highPass * (dL - lowCutL);
            lowCutR += highPass * (dR - lowCutR);
            const float fbL = dL - lowCutL, fbR = dR - lowCutR;

            buffer.setSample (0, i, inL * (1.f - mixSmoothValue) + dL * mixSmoothValue);
            if (ch > 1) buffer.setSample (1, i, inR * (1.f - mixSmoothValue) + dR * mixSmoothValue);

            if (mode == 1) // PingPong: cross feedback
            {
                bufL[(size_t) writePos] = inL + fbR * feedbackSmoothValue;
                bufR[(size_t) writePos] = inR + fbL * feedbackSmoothValue;
            }
            else if (mode == 2) // Mono: shared
            {
                const float monoIn = 0.5f * (inL + inR);
                const float monoD = 0.5f * (fbL + fbR);
                bufL[(size_t) writePos] = monoIn + monoD * feedbackSmoothValue;
                bufR[(size_t) writePos] = bufL[(size_t) writePos];
            }
            else // Stereo: light cross
            {
                const float cross = feedbackSmoothValue * 0.12f;
                bufL[(size_t) writePos] = inL + fbL * (feedbackSmoothValue - cross) + fbR * cross;
                bufR[(size_t) writePos] = inR + fbR * (feedbackSmoothValue - cross) + fbL * cross;
            }

            writePos = (writePos + 1) % bs;
        }
    }

private:
    float readFrac (const std::vector<float>& buf, float delaySamps) const noexcept
    {
        const int bs = (int) buf.size();
        float rp = (float) writePos - delaySamps;
        while (rp < 0.f) rp += (float) bs;
        int i0 = ((int) rp) % bs;
        int i1 = (i0 + 1) % bs;
        float f = rp - std::floor (rp);
        return buf[(size_t) i0] * (1.f - f) + buf[(size_t) i1] * f;
    }

    double sr = 44100.0;
    std::vector<float> bufL, bufR;
    int writePos = 0;
    float delaySamplesL = 300.f, delaySamplesR = 320.f;
    float targetDelayL = 300.f, targetDelayR = 320.f;
    float feedback = 0.3f, mix = 0.f;
    float wowAmt = 0.0f, wowPhase = 0.f;
    float lpL = 0.f, lpR = 0.f;
    float lowCutL = 0.f, lowCutR = 0.f;
    float toneAmount = 0.72f, mixSmoothValue = 0.0f, feedbackSmoothValue = 0.0f;
    int mode = 0;
};
}

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

namespace salek {

/** Kaossilator-style XY effect engine.
 *  Modes: 0 Loop, 1 Glitch, 2 Flanger+Reverb, 3 Psychedelic */
class MagicEngine
{
public:
    void prepare (double sampleRate, int /*maxBlock*/)
    {
        sr = sampleRate > 0 ? sampleRate : 44100.0;
        const int maxSamps = (int) (sr * 2.5); // ~2.5s loop buffer
        bufL.assign ((size_t) maxSamps, 0.f);
        bufR.assign ((size_t) maxSamps, 0.f);
        writePos = 0;
        bufSize = maxSamps;
        freezeLen = juce::jmax (256, (int) (sr * 0.25));
        phase = 0.0;
        flangerPhase = 0.0;
        active = false;
    }

    void setMode (int m) noexcept { mode = juce::jlimit (0, 3, m); }
    void setXY (float nx, float ny) noexcept
    {
        x = juce::jlimit (0.f, 1.f, nx);
        y = juce::jlimit (0.f, 1.f, ny);
    }
    void setActive (bool a) noexcept
    {
        if (a && ! active) captureGate = true; // start fresh capture on touch
        active = a;
    }
    bool isActive() const noexcept { return active; }
    float getX() const noexcept { return x; }
    float getY() const noexcept { return y; }
    int getMode() const noexcept { return mode; }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (! active || bufSize < 64) return;
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        float* L = buffer.getWritePointer (0);
        float* R = ch > 1 ? buffer.getWritePointer (1) : L;

        switch (mode)
        {
            case 0: processLoop (L, R, n); break;
            case 1: processGlitch (L, R, n); break;
            case 2: processFlangerVerb (L, R, n); break;
            default: processPsychedelic (L, R, n); break;
        }
    }

private:
    double sr = 44100.0;
    std::vector<float> bufL, bufR;
    int writePos = 0, bufSize = 0, freezeLen = 1024;
    int readPos = 0;
    float x = 0.5f, y = 0.5f;
    int mode = 0;
    bool active = false, captureGate = false;
    double phase = 0.0, flangerPhase = 0.0;

    void writeSample (float l, float r) noexcept
    {
        bufL[(size_t) writePos] = l;
        bufR[(size_t) writePos] = r;
        writePos = (writePos + 1) % bufSize;
    }

    void processLoop (float* L, float* R, int n) noexcept
    {
        // Y = loop length (50ms..1.5s), X = playback rate / position scrub
        freezeLen = juce::jlimit (256, bufSize - 1, (int) (sr * (0.05 + y * 1.45)));
        const float rate = 0.25f + x * 1.75f; // 0.25x .. 2x

        for (int i = 0; i < n; ++i)
        {
            writeSample (L[i], R[i]);

            // loop from writePos - freezeLen
            float rp = (float) ((writePos - freezeLen + bufSize) % bufSize) + phase;
            while (rp >= (float) bufSize) rp -= (float) bufSize;
            while (rp < 0.f) rp += (float) bufSize;
            int i0 = (int) rp % bufSize;
            int i1 = (i0 + 1) % bufSize;
            float f = rp - (float) (int) rp;
            float ol = bufL[(size_t) i0] * (1.f - f) + bufL[(size_t) i1] * f;
            float or_ = bufR[(size_t) i0] * (1.f - f) + bufR[(size_t) i1] * f;
            phase += rate;
            if (phase >= (double) freezeLen) phase -= (double) freezeLen;

            const float wet = 0.55f + y * 0.4f;
            L[i] = L[i] * (1.f - wet) + ol * wet;
            R[i] = R[i] * (1.f - wet) + or_ * wet;
        }
    }

    void processGlitch (float* L, float* R, int n) noexcept
    {
        // X = stutter rate, Y = intensity / crush
        const int slice = juce::jlimit (32, 4096, (int) (sr * (0.005 + (1.f - x) * 0.12)));
        const float intensity = y;

        for (int i = 0; i < n; ++i)
        {
            writeSample (L[i], R[i]);

            int src = (writePos - 1 - (readPos % slice) + bufSize * 4) % bufSize;
            float gl = bufL[(size_t) src];
            float gr = bufR[(size_t) src];
            readPos++;

            // bitcrush-ish
            if (intensity > 0.3f)
            {
                float steps = 2.f + (1.f - intensity) * 30.f;
                gl = std::floor (gl * steps) / steps;
                gr = std::floor (gr * steps) / steps;
            }
            // hard gate slices
            if (intensity > 0.6f && ((readPos / juce::jmax (8, slice / 4)) & 1))
            {
                gl *= 0.05f;
                gr *= 0.05f;
            }

            L[i] = L[i] * (1.f - intensity) + gl * intensity;
            R[i] = R[i] * (1.f - intensity) + gr * intensity;
        }
    }

    void processFlangerVerb (float* L, float* R, int n) noexcept
    {
        // X = flanger rate/depth, Y = reverb amount
        const float flRate = 0.1f + x * 4.f;
        const float flDepth = 0.002f + x * 0.006f; // seconds
        const float verb = y;

        for (int i = 0; i < n; ++i)
        {
            writeSample (L[i], R[i]);

            flangerPhase += (double) flRate / sr;
            if (flangerPhase > 1.0) flangerPhase -= 1.0;
            float mod = 0.5f + 0.5f * std::sin ((float) flangerPhase * juce::MathConstants<float>::twoPi);
            int dly = juce::jlimit (1, bufSize - 1, (int) (sr * flDepth * mod));
            int rp = (writePos - dly + bufSize) % bufSize;
            float fl = bufL[(size_t) rp];
            float fr = bufR[(size_t) rp];

            // simple reverb-ish multi-tap
            int d1 = (writePos - (int)(sr * 0.029f) + bufSize) % bufSize;
            int d2 = (writePos - (int)(sr * 0.037f) + bufSize) % bufSize;
            int d3 = (writePos - (int)(sr * 0.053f) + bufSize) % bufSize;
            float vl = (bufL[(size_t)d1] + bufL[(size_t)d2] + bufL[(size_t)d3]) * 0.33f;
            float vr = (bufR[(size_t)d1] + bufR[(size_t)d2] + bufR[(size_t)d3]) * 0.33f;

            float outL = L[i] + fl * 0.55f + vl * verb * 0.7f;
            float outR = R[i] + fr * 0.55f + vr * verb * 0.7f;
            L[i] = std::tanh (outL);
            R[i] = std::tanh (outR);
        }
    }

    void processPsychedelic (float* L, float* R, int n) noexcept
    {
        // X = pitch-ish delay / ring, Y = chaos filter + feedback
        const float ringHz = 20.f + x * 800.f;
        const float feedback = 0.2f + y * 0.7f;
        const float delayMs = 20.f + x * 180.f;
        int dly = juce::jlimit (1, bufSize - 1, (int) (sr * delayMs * 0.001f));

        for (int i = 0; i < n; ++i)
        {
            phase += (double) ringHz / sr;
            if (phase > 1.0) phase -= 1.0;
            float ring = std::sin ((float) phase * juce::MathConstants<float>::twoPi);

            writeSample (L[i], R[i]);
            int rp = (writePos - dly + bufSize) % bufSize;
            float dl = bufL[(size_t) rp];
            float dr = bufR[(size_t) rp];

            // cross feedback weirdness
            float wetL = dl * feedback * (0.5f + 0.5f * ring) + dr * feedback * 0.3f * (1.f - x);
            float wetR = dr * feedback * (0.5f - 0.5f * ring) + dl * feedback * 0.3f * x;

            // soft fold
            wetL = std::tanh (wetL * (1.f + y * 2.f));
            wetR = std::tanh (wetR * (1.f + y * 2.f));

            const float mix = 0.4f + y * 0.5f;
            L[i] = L[i] * (1.f - mix) + wetL * mix;
            R[i] = R[i] * (1.f - mix) + wetR * mix;
        }
    }
};

} // namespace salek

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

namespace salek {

/** Kaossilator-style XY effect engine (v2).
 *  Modes: 0 Loop (true freeze), 1 Glitch, 2 Flanger+Reverb, 3 Psychedelic. */
class MagicEngine
{
public:
    void prepare (double sampleRate, int /*maxBlock*/)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        const int maxSamps = (int) (sr * 2.5);
        bufL.assign ((size_t) maxSamps, 0.f);
        bufR.assign ((size_t) maxSamps, 0.f);
        freezeL.assign ((size_t) maxSamps, 0.f);
        freezeR.assign ((size_t) maxSamps, 0.f);
        writePos = 0;
        bufSize = maxSamps;
        freezeLen = juce::jmax (256, (int) (sr * 0.25));
        const float smoothMs = 8.f;
        xyCoeff = std::exp (-1.f / (float) (sr * smoothMs * 0.001f));
        const float rampMs = 5.f;
        rampInc = 1.f / juce::jmax (1.f, (float) (sr * rampMs * 0.001f));
        xSmoothed = ySmoothed = 0.5f;
        activeGain = 0.f;
        activeTarget = false;
        captureGate = false;
        resetModeState();
    }

    void setMode (int m) noexcept
    {
        const int nm = juce::jlimit (0, 3, m);
        if (nm != mode) { mode = nm; resetModeState(); }
    }

    void setXY (float nx, float ny) noexcept
    {
        xTarget = juce::jlimit (0.f, 1.f, nx);
        yTarget = juce::jlimit (0.f, 1.f, ny);
    }

    void setActive (bool a) noexcept
    {
        if (a && ! activeTarget) captureGate = true;
        activeTarget = a;
    }

    bool isActive() const noexcept { return activeTarget || activeGain > 1e-4f; }
    float getX() const noexcept { return xSmoothed; }
    float getY() const noexcept { return ySmoothed; }
    int getMode() const noexcept { return mode; }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int n = buffer.getNumSamples();
        const int chs = buffer.getNumChannels();
        if (chs < 1 || n < 1) return;

        for (int i = 0; i < n; ++i)
        {
            xSmoothed += (1.f - xyCoeff) * (xTarget - xSmoothed);
            ySmoothed += (1.f - xyCoeff) * (yTarget - ySmoothed);

            if (activeTarget && activeGain < 1.f) activeGain = juce::jmin (1.f, activeGain + rampInc);
            else if (! activeTarget && activeGain > 0.f) activeGain = juce::jmax (0.f, activeGain - rampInc);

            float inL = buffer.getSample (0, i);
            float inR = chs > 1 ? buffer.getSample (1, i) : inL;

            bufL[(size_t) writePos] = inL;
            bufR[(size_t) writePos] = inR;

            if (captureGate && activeGain > 0.5f)
            {
                captureFreeze();
                captureGate = false;
            }

            float wetL = inL, wetR = inR;
            if (activeGain > 1e-4f)
            {
                switch (mode)
                {
                    case 0: processLoopSample (wetL, wetR); break;
                    case 1: processGlitchSample (wetL, wetR); break;
                    case 2: processFlangerVerbSample (wetL, wetR); break;
                    case 3: processPsychedelicSample (wetL, wetR); break;
                    default: break;
                }
            }

            const float g = activeGain;
            buffer.setSample (0, i, inL * (1.f - g) + wetL * g);
            if (chs > 1)
                buffer.setSample (1, i, inR * (1.f - g) + wetR * g);

            writePos = (writePos + 1) % bufSize;
        }
    }

private:
    void resetModeState() noexcept
    {
        loopPhase = 0.0;
        glitchPos = 0;
        flPhase = 0.0;
        psychPhase = 0.0;
        psychFbL = psychFbR = 0.f;
        verbFbL = verbFbR = 0.f;
        verbLpL = verbLpR = 0.f;
    }

    void captureFreeze() noexcept
    {
        freezeCapLen = juce::jlimit (256, bufSize,
            (int) (sr * (0.08 + ySmoothed * 1.6)));
        for (int i = 0; i < freezeCapLen; ++i)
        {
            const int rp = (writePos - freezeCapLen + i + bufSize) % bufSize;
            freezeL[(size_t) i] = bufL[(size_t) rp];
            freezeR[(size_t) i] = bufR[(size_t) rp];
        }
        freezeLen = freezeCapLen;
        loopPhase = 0.0;
    }

#include "MagicEngine_Loop.inl"

    void processGlitchSample (float& outL, float& outR) noexcept
    {
        const int slice = juce::jlimit (32, 4096,
            (int) (sr * (0.005 + (1.f - xSmoothed) * 0.12)));
        const float intensity = ySmoothed;
        glitchPos = (glitchPos + 1) % juce::jmax (1, slice);
        const int rp = (writePos - 1 - glitchPos + bufSize) % bufSize;
        float sL = bufL[(size_t) rp];
        float sR = bufR[(size_t) rp];
        const float env = 1.f - (float) glitchPos / (float) juce::jmax (1, slice);
        const float crush = 4.f + intensity * 28.f;
        sL = std::floor (sL * crush) / crush;
        sR = std::floor (sR * crush) / crush;
        outL = sL * (0.4f + intensity * 0.6f) * (0.5f + 0.5f * env);
        outR = sR * (0.4f + intensity * 0.6f) * (0.5f + 0.5f * env);
    }

    void processFlangerVerbSample (float& outL, float& outR) noexcept
    {
        flPhase += (0.1 + xSmoothed * 2.5) / sr;
        if (flPhase > 1.0) flPhase -= 1.0;
        const float md = 0.001f + ySmoothed * 0.008f;
        const float lfo = std::sin ((float) flPhase * juce::MathConstants<float>::twoPi);
        const int d1 = juce::jlimit (1, bufSize - 1, (int) (sr * (md * (1.f + 0.7f * lfo))));
        const int rp = (writePos - d1 + bufSize) % bufSize;
        float fl = bufL[(size_t) rp];
        float fr = bufR[(size_t) rp];
        const float fbAmt = 0.2f + ySmoothed * 0.55f;
        const float verbAmt = 0.25f + ySmoothed * 0.6f;
        const float lpCoeff = 0.15f + (1.f - xSmoothed) * 0.5f;
        float tapL = fl + verbFbL * fbAmt;
        float tapR = fr + verbFbR * fbAmt;
        verbLpL += lpCoeff * (tapL - verbLpL);
        verbLpR += lpCoeff * (tapR - verbLpR);
        verbFbL = verbLpL; verbFbR = verbLpR;
        const float dryL = bufL[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        const float dryR = bufR[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        outL = dryL + fl * 0.5f + verbLpL * verbAmt * 0.85f;
        outR = dryR + fr * 0.5f + verbLpR * verbAmt * 0.85f;
    }

    void processPsychedelicSample (float& outL, float& outR) noexcept
    {
        const float ringHz = 18.f + xSmoothed * 720.f;
        const float feedback = 0.18f + ySmoothed * 0.62f;
        const float delayMs = 18.f + xSmoothed * 160.f;
        const int dly = juce::jlimit (1, bufSize - 1, (int) (sr * delayMs * 0.001f));
        psychPhase += (double) ringHz / sr;
        if (psychPhase >= 1.0) psychPhase -= 1.0;
        const float ring = std::sin ((float) psychPhase * juce::MathConstants<float>::twoPi);
        const int rp = (writePos - dly + bufSize) % bufSize;
        float dl = bufL[(size_t) rp] + psychFbL * feedback;
        float dr = bufR[(size_t) rp] + psychFbR * feedback;
        float wetL = dl * (0.55f + 0.45f * ring) + dr * 0.28f * (1.f - xSmoothed);
        float wetR = dr * (0.55f - 0.45f * ring) + dl * 0.28f * xSmoothed;
        const float drive = 1.f + ySmoothed * 1.8f;
        wetL = std::tanh (wetL * drive);
        wetR = std::tanh (wetR * drive);
        psychFbL = juce::jlimit (-1.f, 1.f, wetL * feedback * 0.9f);
        psychFbR = juce::jlimit (-1.f, 1.f, wetR * feedback * 0.9f);
        const float mix = 0.35f + ySmoothed * 0.55f;
        const float dryL = bufL[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        const float dryR = bufR[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        outL = dryL * (1.f - mix) + wetL * mix;
        outR = dryR * (1.f - mix) + wetR * mix;
    }

    double sr = 44100.0;
    int bufSize = 0, writePos = 0, freezeLen = 0, freezeCapLen = 0;
    std::vector<float> bufL, bufR, freezeL, freezeR;
    float xTarget = 0.5f, yTarget = 0.5f, xSmoothed = 0.5f, ySmoothed = 0.5f;
    float xyCoeff = 0.9f, rampInc = 0.01f, activeGain = 0.f;
    bool activeTarget = false, captureGate = false;
    int mode = 0;
    double loopPhase = 0.0, flPhase = 0.0, psychPhase = 0.0;
    int glitchPos = 0;
    float psychFbL = 0.f, psychFbR = 0.f;
    float verbFbL = 0.f, verbFbR = 0.f, verbLpL = 0.f, verbLpR = 0.f;
};

} // namespace salek

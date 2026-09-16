#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

namespace salek {

/** Kaossilator-style XY effect engine (v2).
 *  Modes: 0 Loop (true freeze), 1 Glitch (slice env), 2 Flanger+Reverb (FB+LPF),
 *         3 Psychedelic (isolated state + soft limiter).
 *  Features: one-pole X/Y smoothing, active gain ramp, click-free transitions. */
class MagicEngine
{
public:
    void prepare (double sampleRate, int /*maxBlock*/)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        const int maxSamps = (int) (sr * 2.5); // ~2.5s buffer
        bufL.assign ((size_t) maxSamps, 0.f);
        bufR.assign ((size_t) maxSamps, 0.f);
        freezeL.assign ((size_t) maxSamps, 0.f);
        freezeR.assign ((size_t) maxSamps, 0.f);
        writePos = 0;
        bufSize = maxSamps;
        freezeLen = juce::jmax (256, (int) (sr * 0.25));

        // ~8 ms smoothing at any SR
        const float smoothMs = 8.f;
        xyCoeff = std::exp (-1.f / (float) (sr * smoothMs * 0.001f));
        // ~5 ms active ramp
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
        if (nm != mode)
        {
            mode = nm;
            resetModeState();
        }
    }

    void setXY (float nx, float ny) noexcept
    {
        xTarget = juce::jlimit (0.f, 1.f, nx);
        yTarget = juce::jlimit (0.f, 1.f, ny);
    }

    void setActive (bool a) noexcept
    {
        if (a && ! activeTarget)
            captureGate = true; // snapshot on rising edge
        activeTarget = a;
    }

    bool isActive() const noexcept { return activeTarget || activeGain > 1e-4f; }
    float getX() const noexcept { return xSmoothed; }
    float getY() const noexcept { return ySmoothed; }
    int getMode() const noexcept { return mode; }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (bufSize < 64) return;

        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        float* L = buffer.getWritePointer (0);
        float* R = ch > 1 ? buffer.getWritePointer (1) : L;

        for (int i = 0; i < n; ++i)
        {
            // --- continuous ring-buffer capture (always, for freeze snapshot) ---
            writeSample (L[i], R[i]);

            // --- smooth XY ---
            xSmoothed += (1.f - xyCoeff) * (xTarget - xSmoothed);
            ySmoothed += (1.f - xyCoeff) * (yTarget - ySmoothed);

            // --- active ramp ---
            if (activeTarget)
                activeGain = juce::jmin (1.f, activeGain + rampInc);
            else
                activeGain = juce::jmax (0.f, activeGain - rampInc);

            if (activeGain < 1e-5f)
                continue; // fully dry, keep writing buffer only

            // rising-edge freeze capture
            if (captureGate)
            {
                captureFreeze();
                captureGate = false;
            }

            float wetL = 0.f, wetR = 0.f;
            switch (mode)
            {
                case 0: processLoopSample (wetL, wetR); break;
                case 1: processGlitchSample (wetL, wetR); break;
                case 2: processFlangerVerbSample (wetL, wetR); break;
                default: processPsychedelicSample (wetL, wetR); break;
            }

            // soft limiter on wet path
            wetL = softLimit (wetL);
            wetR = softLimit (wetR);

            const float g = activeGain;
            L[i] = L[i] * (1.f - g) + wetL * g;
            R[i] = R[i] * (1.f - g) + wetR * g;
        }
    }

private:
    double sr = 44100.0;
    std::vector<float> bufL, bufR;       // continuous ring
    std::vector<float> freezeL, freezeR; // snapshot for LOOP
    int writePos = 0, bufSize = 0, freezeLen = 1024;
    float xTarget = 0.5f, yTarget = 0.5f;
    float xSmoothed = 0.5f, ySmoothed = 0.5f;
    float xyCoeff = 0.99f;
    float activeGain = 0.f, rampInc = 0.001f;
    int mode = 0;
    bool activeTarget = false, captureGate = false;

    // ---- per-mode isolated state ----
    // LOOP
    double loopPhase = 0.0;
    int freezeCapLen = 0;

    // GLITCH
    int glitchRead = 0;
    float glitchEnv = 1.f;

    // FLANGE+VERB
    double flangerPhase = 0.0;
    float verbFbL = 0.f, verbFbR = 0.f;
    float verbLpL = 0.f, verbLpR = 0.f;

    // PSYCHEDELIC
    double psychPhase = 0.0;
    float psychFbL = 0.f, psychFbR = 0.f;

    static float softLimit (float x) noexcept
    {
        // tanh-ish soft clip, cheaper polynomial approximation near 0
        const float ax = std::abs (x);
        if (ax < 0.7f) return x;
        const float s = x >= 0.f ? 1.f : -1.f;
        return s * (0.7f + 0.3f * std::tanh ((ax - 0.7f) * 3.5f));
    }

    void resetModeState() noexcept
    {
        loopPhase = 0.0;
        glitchRead = 0;
        glitchEnv = 1.f;
        flangerPhase = 0.0;
        verbFbL = verbFbR = 0.f;
        verbLpL = verbLpR = 0.f;
        psychPhase = 0.0;
        psychFbL = psychFbR = 0.f;
    }

    void writeSample (float l, float r) noexcept
    {
        bufL[(size_t) writePos] = l;
        bufR[(size_t) writePos] = r;
        writePos = (writePos + 1) % bufSize;
    }

    void captureFreeze() noexcept
    {
        // Y at capture moment decides initial freeze length (refined live by Y later)
        freezeCapLen = juce::jlimit (256, bufSize - 1,
                                     (int) (sr * (0.05 + ySmoothed * 1.45)));
        const int start = (writePos - freezeCapLen + bufSize * 2) % bufSize;
        for (int i = 0; i < freezeCapLen; ++i)
        {
            const int src = (start + i) % bufSize;
            freezeL[(size_t) i] = bufL[(size_t) src];
            freezeR[(size_t) i] = bufR[(size_t) src];
        }
        // zero rest to avoid garbage if length shrinks then grows
        for (int i = freezeCapLen; i < bufSize; ++i)
        {
            freezeL[(size_t) i] = 0.f;
            freezeR[(size_t) i] = 0.f;
        }
        loopPhase = 0.0;
        freezeLen = freezeCapLen;
    }

    // ---------- LOOP: true freeze playback ----------
    void processLoopSample (float& outL, float& outR) noexcept
    {
        // Y morphs effective loop length within captured region
        const int len = juce::jlimit (64, juce::jmax (64, freezeCapLen),
                                      (int) (freezeCapLen * (0.15 + ySmoothed * 0.85)));
        freezeLen = len;
        const float rate = 0.25f + xSmoothed * 1.75f; // 0.25x .. 2x

        if (freezeCapLen < 64)
        {
            // fallback: live ring if never captured
            const int liveLen = juce::jlimit (256, bufSize - 1,
                                              (int) (sr * (0.05 + ySmoothed * 1.45)));
            float rp = (float) ((writePos - liveLen + bufSize) % bufSize) + (float) loopPhase;
            while (rp >= (float) bufSize) rp -= (float) bufSize;
            while (rp < 0.f) rp += (float) bufSize;
            const int i0 = ((int) rp) % bufSize;
            const int i1 = (i0 + 1) % bufSize;
            const float f = rp - (float) (int) rp;
            outL = bufL[(size_t) i0] * (1.f - f) + bufL[(size_t) i1] * f;
            outR = bufR[(size_t) i0] * (1.f - f) + bufR[(size_t) i1] * f;
            loopPhase += (double) rate;
            if (loopPhase >= (double) liveLen) loopPhase -= (double) liveLen;
            return;
        }

        float rp = (float) loopPhase;
        while (rp >= (float) len) rp -= (float) len;
        while (rp < 0.f) rp += (float) len;
        const int i0 = ((int) rp) % len;
        const int i1 = (i0 + 1) % len;
        const float f = rp - (float) (int) rp;
        outL = freezeL[(size_t) i0] * (1.f - f) + freezeL[(size_t) i1] * f;
        outR = freezeR[(size_t) i0] * (1.f - f) + freezeR[(size_t) i1] * f;

        loopPhase += (double) rate;
        if (loopPhase >= (double) len) loopPhase -= (double) len;
        if (loopPhase < 0.0) loopPhase += (double) len;
    }

    // ---------- GLITCH: stutter + bitcrush + slice envelope ----------
    void processGlitchSample (float& outL, float& outR) noexcept
    {
        const int slice = juce::jlimit (32, 4096,
            (int) (sr * (0.005 + (1.f - xSmoothed) * 0.12)));
        const float intensity = ySmoothed;

        const int src = (writePos - 1 - (glitchRead % slice) + bufSize * 4) % bufSize;
        float gl = bufL[(size_t) src];
        float gr = bufR[(size_t) src];
        const int posInSlice = glitchRead % slice;
        glitchRead++;

        // raised-cosine envelope near slice edges (anti-click)
        const int fade = juce::jmax (4, slice / 16);
        float env = 1.f;
        if (posInSlice < fade)
            env = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * (float) posInSlice / (float) fade);
        else if (posInSlice > slice - fade)
            env = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * (float) (slice - posInSlice) / (float) fade);
        glitchEnv = env;
        gl *= env;
        gr *= env;

        // bitcrush
        if (intensity > 0.25f)
        {
            const float t = (intensity - 0.25f) / 0.75f;
            const float steps = 2.f + (1.f - t) * 28.f;
            gl = std::floor (gl * steps + 0.5f) / steps;
            gr = std::floor (gr * steps + 0.5f) / steps;
        }

        // hard gate every other micro-slice when intense
        if (intensity > 0.55f)
        {
            const int micro = juce::jmax (8, slice / 4);
            if (((glitchRead / micro) & 1) != 0)
            {
                gl *= 0.04f;
                gr *= 0.04f;
            }
        }

        outL = gl * intensity + bufL[(size_t) ((writePos - 1 + bufSize) % bufSize)] * (1.f - intensity);
        outR = gr * intensity + bufR[(size_t) ((writePos - 1 + bufSize) % bufSize)] * (1.f - intensity);
    }

    // ---------- FLANGE + REVERB with feedback + LPF ----------
    void processFlangerVerbSample (float& outL, float& outR) noexcept
    {
        const float flRate = 0.08f + xSmoothed * 3.5f;
        const float flDepth = 0.0015f + xSmoothed * 0.007f; // seconds
        const float verbAmt = ySmoothed;
        const float fbAmt = 0.25f + ySmoothed * 0.55f;
        // LPF coeff ~ softer at high Y (darker reverb)
        const float lpCoeff = 0.15f + (1.f - ySmoothed) * 0.55f;

        flangerPhase += (double) flRate / sr;
        if (flangerPhase >= 1.0) flangerPhase -= 1.0;
        const float mod = 0.5f + 0.5f * std::sin ((float) flangerPhase * juce::MathConstants<float>::twoPi);
        const int dly = juce::jlimit (1, bufSize - 1, (int) (sr * flDepth * mod));
        const int rp = (writePos - dly + bufSize) % bufSize;

        // linear interp flanger read
        float fl = bufL[(size_t) rp];
        float fr = bufR[(size_t) rp];

        // multi-tap + feedback recirculation through one-pole LPF
        const int d1 = (writePos - (int) (sr * 0.029f) + bufSize) % bufSize;
        const int d2 = (writePos - (int) (sr * 0.037f) + bufSize) % bufSize;
        const int d3 = (writePos - (int) (sr * 0.053f) + bufSize) % bufSize;
        const int d4 = (writePos - (int) (sr * 0.079f) + bufSize) % bufSize;

        float tapL = (bufL[(size_t) d1] + bufL[(size_t) d2] + bufL[(size_t) d3] + bufL[(size_t) d4]) * 0.25f;
        float tapR = (bufR[(size_t) d1] + bufR[(size_t) d2] + bufR[(size_t) d3] + bufR[(size_t) d4]) * 0.25f;

        // inject feedback
        tapL += verbFbR * fbAmt * 0.7f; // cross FB for width
        tapR += verbFbL * fbAmt * 0.7f;

        // one-pole lowpass damping
        verbLpL += lpCoeff * (tapL - verbLpL);
        verbLpR += lpCoeff * (tapR - verbLpR);
        verbFbL = verbLpL;
        verbFbR = verbLpR;

        // flanger with mild feedback
        const float flFb = 0.35f + xSmoothed * 0.25f;
        float flOutL = fl + verbFbL * flFb * 0.15f;
        float flOutR = fr + verbFbR * flFb * 0.15f;

        const float dryL = bufL[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        const float dryR = bufR[(size_t) ((writePos - 1 + bufSize) % bufSize)];

        outL = dryL + flOutL * 0.5f + verbLpL * verbAmt * 0.85f;
        outR = dryR + flOutR * 0.5f + verbLpR * verbAmt * 0.85f;
    }

    // ---------- PSYCHEDELIC: isolated state + soft limit path ----------
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

        // cross-modulated feedback
        float wetL = dl * (0.55f + 0.45f * ring) + dr * 0.28f * (1.f - xSmoothed);
        float wetR = dr * (0.55f - 0.45f * ring) + dl * 0.28f * xSmoothed;

        // soft fold driven by Y
        const float drive = 1.f + ySmoothed * 1.8f;
        wetL = std::tanh (wetL * drive);
        wetR = std::tanh (wetR * drive);

        // store limited feedback (prevent runaway)
        psychFbL = softLimit (wetL * feedback * 0.9f);
        psychFbR = softLimit (wetR * feedback * 0.9f);

        const float mix = 0.35f + ySmoothed * 0.55f;
        const float dryL = bufL[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        const float dryR = bufR[(size_t) ((writePos - 1 + bufSize) % bufSize)];
        outL = dryL * (1.f - mix) + wetL * mix;
        outR = dryR * (1.f - mix) + wetR * mix;
    }
};

} // namespace salek

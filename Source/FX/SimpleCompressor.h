#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <array>

namespace salek {

/** Multi-band compressor with INDEPENDENT LO / MID / HI thresholds. */
class SimpleCompressor
{
public:
    void prepare (double sampleRate, int /*block*/)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        setCrossover (200.f, 2500.f);
        for (auto& e : env) e = 0.f;
        for (auto& g : grSmooth) g = 1.f;
        for (auto& m : grMeter) m = 0.f;
        // default band thresholds (dB)
        bandThrDb[0] = -18.f; // LO
        bandThrDb[1] = -12.f; // MID
        bandThrDb[2] = -8.f;  // HI
    }

    /** Global threshold still shifts all bands together */
    void setThresholdDb (float db) noexcept
    {
        thresholdDb = juce::jlimit (-60.f, 0.f, db);
        // keep relative offsets if user only moves the main knob
        if (! perBandOverride)
        {
            bandThrDb[0] = juce::jlimit (-60.f, 0.f, thresholdDb - 6.f);
            bandThrDb[1] = juce::jlimit (-60.f, 0.f, thresholdDb);
            bandThrDb[2] = juce::jlimit (-60.f, 0.f, thresholdDb + 4.f);
        }
    }

    /** Independent per-band threshold (0=LO 1=MID 2=HI) */
    void setBandThresholdDb (int band, float db) noexcept
    {
        band = juce::jlimit (0, 2, band);
        bandThrDb[(size_t) band] = juce::jlimit (-60.f, 0.f, db);
        perBandOverride = true;
    }

    float getBandThresholdDb (int band) const noexcept
    {
        band = juce::jlimit (0, 2, band);
        return bandThrDb[(size_t) band];
    }

    void setRatio (float r) noexcept { ratio = juce::jlimit (1.f, 20.f, r); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.f, 1.f, m); }
    void setAttackMs (float ms) noexcept { attackMs = juce::jmax (0.1f, ms); updateCoeffs(); }
    void setReleaseMs (float ms) noexcept { releaseMs = juce::jmax (1.f, ms); updateCoeffs(); }
    void setDepth (float d) noexcept { depth = juce::jlimit (0.f, 1.f, d); }

    float getBandGR (int band) const noexcept
    {
        band = juce::jlimit (0, 2, band);
        return grMeter[(size_t) band];
    }
    float getGainReduction() const noexcept
    {
        return 0.34f * grMeter[0] + 0.33f * grMeter[1] + 0.33f * grMeter[2];
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (mix < 1e-4f) return;
        const int n = buffer.getNumSamples();
        const int chs = buffer.getNumChannels();
        if (chs < 1) return;

        updateCoeffs();
        const float invRatioBase = 1.f / ratio;
        const float ratioMul[3] = { 1.3f, 1.f, 0.85f };
        const float upAmt = depth * 0.7f;
        const float downAmt = 0.45f + depth * 0.7f;

        for (int i = 0; i < n; ++i)
        {
            float L = buffer.getSample (0, i);
            float R = chs > 1 ? buffer.getSample (1, i) : L;

            lp1L += aLo * (L - lp1L); lp1R += aLo * (R - lp1R);
            float lowL = lp1L, lowR = lp1R;
            float midHiL = L - lowL, midHiR = R - lowR;
            lp2L += aHi * (midHiL - lp2L); lp2R += aHi * (midHiR - lp2R);
            float midL = lp2L, midR = lp2R;
            float highL = midHiL - midL, highR = midHiR - midR;

            float bandsL[3] = { lowL, midL, highL };
            float bandsR[3] = { lowR, midR, highR };
            float outL = 0.f, outR = 0.f;

            for (int b = 0; b < 3; ++b)
            {
                float xL = bandsL[b], xR = bandsR[b];
                float level = 0.5f * (std::abs (xL) + std::abs (xR));

                float coeff = (level > env[(size_t) b]) ? atkCoeff : relCoeff;
                env[(size_t) b] += coeff * (level - env[(size_t) b]);
                float e = juce::jmax (1.0e-6f, env[(size_t) b]);

                const float thresh = juce::Decibels::decibelsToGain (bandThrDb[(size_t) b]);
                const float invRatio = invRatioBase / ratioMul[b];

                float grDown = 1.f;
                if (e > thresh)
                {
                    float over = e / thresh;
                    float compressed = std::pow (over, invRatio - 1.f);
                    grDown = juce::jlimit (0.04f, 1.f, compressed);
                    grDown = 1.f - (1.f - grDown) * downAmt;
                }

                float grUp = 1.f;
                if (e < thresh && upAmt > 1e-4f)
                {
                    float under = e / thresh;
                    grUp = 1.f + (1.f - under) * upAmt * 1.6f;
                    grUp = juce::jmin (grUp, 3.0f);
                }

                float gr = grDown * grUp;
                grSmooth[(size_t) b] += 0.15f * (gr - grSmooth[(size_t) b]);
                float g = grSmooth[(size_t) b];

                float red = juce::jlimit (0.f, 1.f, 1.f - juce::jmin (1.f, grDown));
                grMeter[(size_t) b] += 0.12f * (red - grMeter[(size_t) b]);

                outL += xL * g;
                outR += xR * g;
            }

            const float makeup = 1.f + depth * 0.28f;
            outL *= makeup; outR *= makeup;
            buffer.setSample (0, i, L * (1.f - mix) + outL * mix);
            if (chs > 1)
                buffer.setSample (1, i, R * (1.f - mix) + outR * mix);
        }
    }

private:
    void setCrossover (float loHz, float hiHz)
    {
        aLo = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * loHz / (float) sr);
        aHi = 1.f - std::exp (-2.f * juce::MathConstants<float>::pi * hiHz / (float) sr);
    }
    void updateCoeffs()
    {
        atkCoeff = 1.f - std::exp (-1.f / (0.001f * attackMs * (float) sr));
        relCoeff = 1.f - std::exp (-1.f / (0.001f * releaseMs * (float) sr));
    }

    double sr = 44100.0;
    float thresholdDb = -12.f, ratio = 4.f, mix = 0.f, depth = 0.55f;
    float attackMs = 8.f, releaseMs = 80.f;
    float atkCoeff = 0.1f, relCoeff = 0.01f;
    float aLo = 0.05f, aHi = 0.2f;
    float lp1L = 0, lp1R = 0, lp2L = 0, lp2R = 0;
    std::array<float, 3> bandThrDb { -18.f, -12.f, -8.f };
    bool perBandOverride = false;
    std::array<float, 3> env {}, grSmooth { 1.f, 1.f, 1.f }, grMeter {};
};

} // namespace salek

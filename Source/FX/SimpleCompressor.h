#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{
/** Lightweight single-band compressor: musical, predictable and cheap.
    Three meters are retained for the UI, but the audio gain is one coherent
    stereo-linked envelope rather than three crossovers fighting each other. */
class SimpleCompressor
{
public:
    void prepare (double sampleRate, int) noexcept
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        env = 0.0f;
        gainSmooth = 1.0f;
        grMeter.fill (0.0f);
        updateCoeffs();
    }

    void setThresholdDb (float db) noexcept { thresholdDb = juce::jlimit (-60.0f, 0.0f, db); }
    void setBandThresholdDb (int band, float db) noexcept
    {
        band = juce::jlimit (0, 2, band);
        bandThrDb[(size_t) band] = juce::jlimit (-60.0f, 0.0f, db);
    }
    float getBandThresholdDb (int band) const noexcept
    {
        return bandThrDb[(size_t) juce::jlimit (0, 2, band)];
    }
    void setRatio (float r) noexcept { ratio = juce::jlimit (1.0f, 20.0f, r); }
    void setMix (float m) noexcept { mix = juce::jlimit (0.0f, 1.0f, m); }
    void setAttackMs (float ms) noexcept { attackMs = juce::jlimit (0.5f, 100.0f, ms); updateCoeffs(); }
    void setReleaseMs (float ms) noexcept { releaseMs = juce::jlimit (5.0f, 1000.0f, ms); updateCoeffs(); }
    void setDepth (float d) noexcept { depth = juce::jlimit (0.0f, 1.0f, d); }
    void setMakeupDb (float db) noexcept { makeupDb = juce::jlimit (-12.0f, 18.0f, db); }

    float getBandGR (int band) const noexcept { return grMeter[(size_t) juce::jlimit (0, 2, band)]; }
    float getGainReduction() const noexcept { return gainReductionDb; }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        if (mix < 1.0e-4f || buffer.getNumChannels() == 0) return;

        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        const float makeup = juce::Decibels::decibelsToGain (makeupDb);

        for (int i = 0; i < n; ++i)
        {
            float l = buffer.getSample (0, i);
            float r = ch > 1 ? buffer.getSample (1, i) : l;
            const float detector = juce::jmax (1.0e-6f, 0.5f * (std::abs (l) + std::abs (r)));

            const float coeff = detector > env ? attackCoeff : releaseCoeff;
            env += coeff * (detector - env);

            const float levelDb = juce::Decibels::gainToDecibels (env, -120.0f);
            const float overDb = levelDb - thresholdDb;
            float targetGainDb = 0.0f;
            if (overDb > 0.0f)
                targetGainDb = -(overDb - overDb / ratio) * depth;

            const float targetGain = juce::Decibels::decibelsToGain (targetGainDb);
            // Separate short gain smoothing avoids zippering without adding lookahead.
            gainSmooth += 0.15f * (targetGain - gainSmooth);

            const float wetGain = gainSmooth * makeup;
            const float outL = l * wetGain;
            const float outR = r * wetGain;
            buffer.setSample (0, i, l * (1.0f - mix) + outL * mix);
            if (ch > 1) buffer.setSample (1, i, r * (1.0f - mix) + outR * mix);

            gainReductionDb = juce::jmax (0.0f, -targetGainDb);
            const float gr = juce::jlimit (0.0f, 1.0f, gainReductionDb / 24.0f);
            // UI bands are weighted indicators, not separate compressors.
            grMeter[0] += 0.12f * (gr * 0.95f - grMeter[0]);
            grMeter[1] += 0.12f * (gr - grMeter[1]);
            grMeter[2] += 0.12f * (gr * 0.75f - grMeter[2]);
        }
    }

private:
    void updateCoeffs() noexcept
    {
        atkCoeff = 1.0f - std::exp (-1.0f / (0.001f * attackMs * (float) sr));
        releaseCoeff = 1.0f - std::exp (-1.0f / (0.001f * releaseMs * (float) sr));
    }

    double sr = 44100.0;
    float thresholdDb = -12.0f, ratio = 4.0f, mix = 0.0f, depth = 1.0f, makeupDb = 0.0f;
    float attackMs = 8.0f, releaseMs = 80.0f;
    float atkCoeff = 0.02f, releaseCoeff = 0.001f;
    float env = 0.0f, gainSmooth = 1.0f, gainReductionDb = 0.0f;
    std::array<float, 3> bandThrDb { -18.0f, -12.0f, -8.0f }, grMeter {};
};
}

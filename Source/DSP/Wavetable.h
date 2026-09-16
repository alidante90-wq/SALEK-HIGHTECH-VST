#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

// SALEK HIGHTECH Wavetable Engine
// Built for 170-190+ BPM Hi-Tech / Darkpsy / FM Bass / Screech / Metallic / Alien / Cyberpunk
class Wavetable
{
public:
    static constexpr int tableSize = 2048;
    static constexpr int numFrames = 128;

    Wavetable()
    {
        tables.resize (numFrames);
        for (auto& t : tables)
            t.resize (tableSize + 4);
        generateHighTechTables();
    }

    void generateHighTechTables()
    {
        for (int f = 0; f < numFrames; ++f)
        {
            float morph = (float) f / (float)(numFrames - 1);
            float morph2 = morph * morph;

            for (int i = 0; i < tableSize; ++i)
            {
                float phase = (float) i / (float) tableSize;
                float twopi = juce::MathConstants<float>::twoPi;

                float sine = std::sin (twopi * phase);
                float saw  = 2.0f * phase - 1.0f;
                float tri  = 1.0f - 4.0f * std::abs (std::round (phase - 0.25f) - (phase - 0.25f));

                // Metallic / inharmonic partials (alien, laser, screech)
                float metallic = 0.0f;
                for (int h = 1; h <= 12; ++h)
                {
                    float harm = (float) h + 0.07f * h * morph;
                    float amp = 1.0f / (h * (1.0f + morph * 0.8f));
                    metallic += amp * std::sin (twopi * phase * harm);
                }
                metallic *= 0.45f;

                // Digital / cyber character
                float digital = sine;
                if (morph > 0.3f)
                {
                    float bits = 4.0f + morph * 8.0f;
                    digital = std::floor (sine * bits) / bits;
                }

                // Strong wavefold (core of hi-tech edge)
                float folded = sine;
                float foldDrive = 1.0f + morph2 * 7.0f;
                folded = std::sin (sine * foldDrive * juce::MathConstants<float>::pi * 0.5f);
                if (morph > 0.55f)
                    folded = std::sin (folded * (1.5f + (morph - 0.55f) * 5.0f));

                float wave;
                if (morph < 0.25f)
                {
                    float t = morph / 0.25f;
                    wave = (1.0f - t) * sine + t * (0.7f * saw + 0.3f * tri);
                }
                else if (morph < 0.55f)
                {
                    float t = (morph - 0.25f) / 0.3f;
                    wave = (1.0f - t) * (0.6f * saw + 0.4f * tri) + t * (0.5f * metallic + 0.5f * folded);
                }
                else
                {
                    float t = (morph - 0.55f) / 0.45f;
                    wave = (1.0f - t) * (0.4f * metallic + 0.6f * folded)
                         + t * (0.35f * metallic + 0.4f * folded + 0.25f * digital);
                    wave += 0.15f * t * std::sin (twopi * phase * (9.0f + morph * 6.0f));
                }

                wave = juce::jlimit (-1.2f, 1.2f, wave);
                tables[f][i] = wave * 0.65f;
            }
            tables[f][tableSize]     = tables[f][0];
            tables[f][tableSize + 1] = tables[f][1];
            tables[f][tableSize + 2] = tables[f][2];
            tables[f][tableSize + 3] = tables[f][3];
        }
    }

    float getSample (float framePos, float phase) const
    {
        framePos = juce::jlimit (0.0f, 1.0f, framePos);
        float frameF = framePos * (numFrames - 1.0001f);
        int f0 = (int) frameF;
        int f1 = std::min (f0 + 1, numFrames - 1);
        float fFrac = frameF - (float) f0;

        phase = phase - std::floor (phase);
        float idx = phase * tableSize;
        int i0 = (int) idx;
        float iFrac = idx - (float) i0;

        auto catmull = [](float p0, float p1, float p2, float p3, float t)
        {
            float a = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
            float b = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
            float c = -0.5f * p0 + 0.5f * p2;
            float d = p1;
            return ((a * t + b) * t + c) * t + d;
        };

        auto sampleAt = [&](int frame, int ii) -> float
        {
            ii = ii % tableSize;
            if (ii < 0) ii += tableSize;
            return tables[frame][ii];
        };

        float s0 = catmull (sampleAt (f0, i0 - 1), sampleAt (f0, i0), sampleAt (f0, i0 + 1), sampleAt (f0, i0 + 2), iFrac);
        float s1 = catmull (sampleAt (f1, i0 - 1), sampleAt (f1, i0), sampleAt (f1, i0 + 1), sampleAt (f1, i0 + 2), iFrac);

        return s0 + fFrac * (s1 - s0);
    }

    std::vector<std::vector<float>>& getTables() { return tables; }

private:
    std::vector<std::vector<float>> tables;
};
#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <vector>
namespace salek {
/** HRTF-lite binaural: ILD + ITD + pinna shelf + distance air absorption. */
class SimpleSpatial {
public:
    void prepare (double sr, int) {
        sampleRate = sr;
        maxDelay = juce::jmax (2, (int) (0.00065 * sr) + 1);
        delayL.assign ((size_t) maxDelay + 4, 0.f);
        delayR.assign ((size_t) maxDelay + 4, 0.f);
        writePos = 0;
    }
    void setAzimuth (float deg) { azimuth = juce::jlimit (-180.f, 180.f, deg); }
    void setDistance (float d) { distance = juce::jlimit (0.f, 1.f, d); }
    void setSize (float s) { size = juce::jlimit (0.f, 1.f, s); }
    void setElevation (float e) { elevation = juce::jlimit (-90.f, 90.f, e); }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2 || maxDelay < 2) return;
        const float azRad = azimuth * juce::MathConstants<float>::pi / 180.f;
        const float ild = std::sin (azRad);
        const float gL = juce::Decibels::decibelsToGain (-6.0f * juce::jmax (0.f,  ild));
        const float gR = juce::Decibels::decibelsToGain (-6.0f * juce::jmax (0.f, -ild));
        const float itdNorm = std::sin (azRad);
        const int dL = juce::jmax (0, (int) std::round (juce::jmax (0.f,  itdNorm) * (float) maxDelay));
        const int dR = juce::jmax (0, (int) std::round (juce::jmax (0.f, -itdNorm) * (float) maxDelay));
        const float elevBright = 1.0f + elevation / 180.f;
        const float distAtten = 1.0f / (1.0f + distance * 4.0f);
        const float air = 0.15f + distance * 0.55f;
        const float width = 0.35f + size * 0.65f;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float l = buffer.getSample (0, i);
            float r = buffer.getSample (1, i);
            float mid = 0.5f * (l + r);
            float side = 0.5f * (l - r) * width;
            float srcL = mid + side;
            float srcR = mid - side;

            delayL[(size_t) writePos] = srcL;
            delayR[(size_t) writePos] = srcR;
            int rL = writePos - dL; if (rL < 0) rL += maxDelay;
            int rR = writePos - dR; if (rR < 0) rR += maxDelay;
            float outL = delayL[(size_t) rL] * gL * distAtten;
            float outR = delayR[(size_t) rR] * gR * distAtten;

            float hpL = outL - zHpL; zHpL += 0.25f * (outL - zHpL);
            float hpR = outR - zHpR; zHpR += 0.25f * (outR - zHpR);
            outL += hpL * (elevBright - 1.0f) * 0.5f;
            outR += hpR * (elevBright - 1.0f) * 0.5f;

            zAirL += air * (outL - zAirL);
            zAirR += air * (outR - zAirR);
            outL = outL * (1.f - distance * 0.5f) + zAirL * distance * 0.5f;
            outR = outR * (1.f - distance * 0.5f) + zAirR * distance * 0.5f;

            buffer.setSample (0, i, outL);
            buffer.setSample (1, i, outR);
            writePos = (writePos + 1) % maxDelay;
        }
    }
private:
    double sampleRate = 44100.0;
    float azimuth = 0.f, distance = 0.f, size = 0.5f, elevation = 0.f;
    int maxDelay = 32, writePos = 0;
    std::vector<float> delayL, delayR;
    float zHpL = 0, zHpR = 0, zAirL = 0, zAirR = 0;
};
}

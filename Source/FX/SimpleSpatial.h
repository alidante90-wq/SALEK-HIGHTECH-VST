#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class SimpleSpatial {
public:
    void prepare (double sr, int) { sampleRate = sr; }
    void setAzimuth (float deg) { azimuth = juce::jlimit (-180.f, 180.f, deg); }
    void setDistance (float d) { distance = juce::jlimit (0.f, 1.f, d); }
    void setSize (float s) { size = juce::jlimit (0.f, 1.f, s); }
    void process (juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2) return;
        const float pan = std::sin (azimuth * juce::MathConstants<float>::pi / 180.f);
        const float gL = 0.5f * (1.0f - pan);
        const float gR = 0.5f * (1.0f + pan);
        const float atten = 1.0f / (1.0f + distance * 3.0f);
        const float width = 0.5f + size * 0.5f;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float l = buffer.getSample (0, i);
            float r = buffer.getSample (1, i);
            float mid = 0.5f * (l + r);
            float side = 0.5f * (l - r) * width;
            float outL = (mid + side) * gL * 2.f * atten;
            float outR = (mid - side) * gR * 2.f * atten;
            zL += (0.05f + distance * 0.25f) * (outL - zL);
            zR += (0.05f + distance * 0.25f) * (outR - zR);
            buffer.setSample (0, i, outL * (1.f - distance * 0.4f) + zL * distance * 0.4f);
            buffer.setSample (1, i, outR * (1.f - distance * 0.4f) + zR * distance * 0.4f);
        }
    }
private:
    double sampleRate = 44100.0;
    float azimuth = 0.f, distance = 0.f, size = 0.5f;
    float zL = 0.f, zR = 0.f;
};
}

#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class MultiFilter
{
public:
    enum class Type { Lowpass, Highpass, Bandpass, Notch, Ladder };

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        ladder.prepare (spec);
        svf.prepare (spec);
        ladder.setEnabled (true);
        svf.setEnabled (true);
        reset();
    }

    void reset()
    {
        ladder.reset();
        svf.reset();
    }

    void setType (Type t) { type = t; }

    void setCutoff (float hz)
    {
        cutoff = juce::jlimit (20.0f, 20000.0f, hz);
        update();
    }

    void setResonance (float r)
    {
        res = juce::jlimit (0.0f, 1.0f, r);
        update();
    }

    void setDrive (float d) { drive = juce::jlimit (0.0f, 1.0f, d); }

    float processSample (float x)
    {
        // Pre drive for character
        if (drive > 0.01f)
            x = std::tanh (x * (1.0f + drive * 4.0f));

        float y = x;
        if (type == Type::Ladder)
        {
            y = ladder.processSample (0, x);
        }
        else
        {
            // StateVariableTPTFilter mono-friendly path
            float outL = 0.0f, outR = 0.0f;
            svf.processSample (0, x, outL, outR);
            y = outL;
        }
        return y;
    }

    // Stereo process helper
    void process (juce::AudioBuffer<float>& buffer)
    {
        // For voice we usually process mono then pan, but keep simple
    }

private:
    void update()
    {
        float q = 0.1f + res * 10.0f;
        ladder.setCutoffFrequencyHz (cutoff);
        ladder.setResonance (res);

        switch (type)
        {
            case Type::Lowpass:  svf.setType (juce::dsp::StateVariableTPTFilterType::lowpass); break;
            case Type::Highpass: svf.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
            case Type::Bandpass: svf.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
            case Type::Notch:    svf.setType (juce::dsp::StateVariableTPTFilterType::notch); break;
            default: break;
        }
        svf.setCutoffFrequency (cutoff);
        svf.setResonance (res);
    }

    Type type = Type::Lowpass;
    float cutoff = 1000.0f;
    float res = 0.2f;
    float drive = 0.0f;
    double sampleRate = 44100.0;

    juce::dsp::LadderFilter<float> ladder;
    juce::dsp::StateVariableTPTFilter<float> svf;
};
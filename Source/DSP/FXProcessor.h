#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

// Lightweight but real FX chain for SALEK HIGHTECH
// Distortion -> Delay -> Reverb (order fixed for now, expandable)
class FXProcessor
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        dist.prepare (spec);
        delay.prepare (spec);
        reverb.prepare (spec);
        dryWetDelay.reset (sampleRate, 0.02);
        dryWetReverb.reset (sampleRate, 0.02);
        reset();
    }

    void reset()
    {
        dist.reset();
        delay.reset();
        reverb.reset();
    }

    void setDistortionDrive (float d)
    {
        distDrive = juce::jlimit (0.0f, 1.0f, d);
        // Simple waveshaper gain
    }

    void setDelayTime (float seconds)
    {
        delayTime = juce::jlimit (0.001f, 2.0f, seconds);
        delay.setDelay (delayTime * (float) sampleRate);
    }

    void setDelayFeedback (float fb)
    {
        delayFeedback = juce::jlimit (0.0f, 0.95f, fb);
    }

    void setDelayMix (float m)
    {
        dryWetDelay.setTargetValue (juce::jlimit (0.0f, 1.0f, m));
    }

    void setReverbSize (float s)
    {
        reverbParams.roomSize = juce::jlimit (0.0f, 1.0f, s);
        reverb.setParameters (reverbParams);
    }

    void setReverbMix (float m)
    {
        dryWetReverb.setTargetValue (juce::jlimit (0.0f, 1.0f, m));
    }

    void setReverbDamping (float d)
    {
        reverbParams.damping = juce::jlimit (0.0f, 1.0f, d);
        reverb.setParameters (reverbParams);
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // Distortion (simple but effective for hi-tech)
        if (distDrive > 0.01f)
        {
            float gain = 1.0f + distDrive * 12.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* data = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    float x = data[i] * gain;
                    // asymmetric soft clip + fold for aggressive character
                    x = std::tanh (x);
                    if (distDrive > 0.5f)
                        x = std::sin (x * (1.0f + (distDrive - 0.5f) * 2.5f));
                    data[i] = x * (1.0f / (1.0f + distDrive * 0.4f)); // makeup
                }
            }
        }

        // Delay
        if (dryWetDelay.getTargetValue() > 0.001f || dryWetDelay.getCurrentValue() > 0.001f)
        {
            juce::dsp::AudioBlock<float> block (buffer);
            auto context = juce::dsp::ProcessContextReplacing<float> (block);

            // We use a simple custom delay for feedback control
            // For brevity using juce DelayLine style
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float mix = dryWetDelay.getNextValue();
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    float dry = buffer.getSample (ch, i);
                    float delayed = delay.popSample (ch);
                    float fbIn = dry + delayed * delayFeedback;
                    delay.pushSample (ch, fbIn);
                    buffer.setSample (ch, i, dry * (1.0f - mix) + delayed * mix);
                }
            }
        }

        // Reverb
        if (dryWetReverb.getTargetValue() > 0.001f || dryWetReverb.getCurrentValue() > 0.001f)
        {
            juce::AudioBuffer<float> dryBuf;
            dryBuf.makeCopyOf (buffer);

            juce::dsp::AudioBlock<float> block (buffer);
            reverb.process (juce::dsp::ProcessContextReplacing<float> (block));

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float mix = dryWetReverb.getNextValue();
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    float dry = dryBuf.getSample (ch, i);
                    float wet = buffer.getSample (ch, i);
                    buffer.setSample (ch, i, dry * (1.0f - mix) + wet * mix);
                }
            }
        }
    }

private:
    double sampleRate = 44100.0;
    float distDrive = 0.0f;
    float delayTime = 0.3f;
    float delayFeedback = 0.4f;

    juce::dsp::WaveShaper<float> dist; // placeholder
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 192000 };
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::SmoothedValue<float> dryWetDelay { 0.0f };
    juce::SmoothedValue<float> dryWetReverb { 0.0f };
};
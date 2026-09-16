#include "SynthVoice.h"

namespace salek
{

SynthVoice::SynthVoice()
{
    osc1.setWavetable (&sharedWavetable);
    osc2.setWavetable (&sharedWavetable);
    osc3.setWavetable (&sharedWavetable);

    osc1.setLevel (0.7f);
    osc2.setLevel (0.5f);
    osc3.setLevel (0.4f);
    osc3Octave = -1;
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    osc1.prepare (sampleRate);
    osc2.prepare (sampleRate);
    osc3.prepare (sampleRate);
    filter.prepare (sampleRate);
    lfo.prepare (sampleRate);
    cutoffSmoother.reset (baseCutoff);
    cutoffSmoother.setTimeMs (5.0f, sampleRate);
    filter.setCutoff (baseCutoff);
    adsr.setSampleRate (sampleRate);
    adsr.setParameters (adsrParams);
}

void SynthVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound* /*sound*/,
                            int /*currentPitchWheelPosition*/)
{
    currentMidiNote = midiNoteNumber;
    currentVelocity = velocity;
    isNoteOn = true;

    osc1.reset();
    osc2.reset();
    osc3.reset();

    updateFrequencies();
    adsr.noteOn();
}

void SynthVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        clearCurrentNote();
        adsr.reset();
        isNoteOn = false;
    }
}

void SynthVoice::pitchWheelMoved (int) {}
void SynthVoice::controllerMoved (int, int) {}

void SynthVoice::updateFrequencies()
{
    if (currentSampleRate <= 0.0)
        return;

    auto noteToHz = [] (int note, int octave, int semi, float fine) -> float
    {
        const float midi = static_cast<float> (note + octave * 12 + semi) + fine / 100.0f;
        return 440.0f * std::pow (2.0f, (midi - 69.0f) / 12.0f);
    };

    osc1.setFrequency (noteToHz (currentMidiNote, osc1Octave, osc1Semi, osc1Fine));
    osc2.setFrequency (noteToHz (currentMidiNote, osc2Octave, osc2Semi, osc2Fine));
    osc3.setFrequency (noteToHz (currentMidiNote, osc3Octave, osc3Semi, osc3Fine));
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                  int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    auto* left  = outputBuffer.getWritePointer (0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1
                    ? outputBuffer.getWritePointer (1, startSample) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        if (! adsr.isActive() && ! isNoteOn)
        {
            clearCurrentNote();
            break;
        }

        // --- Modulators first (no incoming PM for simplicity of order) ---
        // OSC3 can FM into OSC2
        const float s3 = osc3.processSample (0.0f, 1.0f);

        // OSC2 receives FM from OSC3
        const float pmFor2 = s3 * fm3to2 * 0.5f; // scale to reasonable phase deviation
        const float s2 = osc2.processSample (pmFor2, 1.0f);

        // --- Carrier OSC1 receives from both ---
        // FM = frequency modulation via phase increment (approximated as phase modulation scaled)
        // For classic FM we add to phase; index is in radians-ish scaled.
        const float fmIndex2 = fm2to1 * 2.5f;
        const float fmIndex3 = fm3to1 * 2.5f;
        const float pmFrom2  = s2 * (fmIndex2 + pm2to1);
        const float pmFrom3  = s3 * (fmIndex3 + pm3to1);

        // AM
        float am = 1.0f;
        if (am2to1 > 1.0e-4f)
            am = 1.0f + (s2 * am2to1); // bipolar-ish around 1

        const float s1 = osc1.processSample (pmFrom2 + pmFrom3, am);

        // Ring modulation: OSC1 * OSC2
        float sample = s1;
        if (rm2to1 > 1.0e-4f)
            sample = s1 * (1.0f - rm2to1) + (s1 * s2) * rm2to1;

        // Mix in the modulators themselves (their levels already applied)
        sample += s2 * 0.15f + s3 * 0.12f;

        const float env = adsr.getNextSample();
        // Filter with envelope amount
        const float lfoVal = lfo.process();
        const float modCutoff = baseCutoff * std::pow (2.0f, (env * filterEnvAmt + lfoVal) * 3.0f - 1.5f);
        cutoffSmoother.setTarget (modCutoff);
        filter.setCutoff (cutoffSmoother.getNext());
        sample = filter.process (sample);

        sample *= env * currentVelocity * 0.28f;

        left[i] += sample;
        if (right != nullptr)
            right[i] += sample;
    }

    if (! adsr.isActive())
    {
        clearCurrentNote();
        isNoteOn = false;
    }
}

} // namespace salek

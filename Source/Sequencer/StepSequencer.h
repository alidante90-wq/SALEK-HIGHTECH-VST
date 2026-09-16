#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

/** Real 16-step sequencer. Outputs note events + per-step modulation. */
class StepSequencer
{
public:
    static constexpr int NumSteps = 16;

    struct Step
    {
        int   noteOffset = 0;   // relative to root (-24..+24)
        float velocity   = 0.8f;
        float gate       = 0.7f;
        float probability = 1.0f;
        float modValue   = 0.0f; // -1..1 extra modulation
        bool  active     = true;
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        updateTiming();
    }

    void setBpm (double b) noexcept
    {
        bpm = juce::jlimit (20.0, 300.0, b);
        updateTiming();
    }

    void setRateDivisor (int div) noexcept
    {
        rateDivisor = juce::jlimit (1, 16, div);
        updateTiming();
    }

    void setEnabled (bool e) noexcept
    {
        enabled = e;
        if (!e)
        {
            if (currentNote >= 0)
                pendingNoteOff = currentNote;
            currentNote = -1;
        }
    }

    void setRootNote (int note) noexcept { rootNote = juce::jlimit (0, 127, note); }
    void setNumSteps (int n) noexcept { numSteps = juce::jlimit (1, NumSteps, n); }

    Step& getStep (int i) noexcept
    {
        return steps[static_cast<size_t> (juce::jlimit (0, NumSteps - 1, i))];
    }

    const Step& getStep (int i) const noexcept
    {
        return steps[static_cast<size_t> (juce::jlimit (0, NumSteps - 1, i))];
    }

    float getCurrentModValue() const noexcept { return currentMod; }
    int   getCurrentStep() const noexcept { return currentStep; }

    /** Process block. Appends MIDI note on/off. */
    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        if (! enabled)
            return;

        for (int i = 0; i < numSamples; ++i)
        {
            if (pendingNoteOff >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, pendingNoteOff), i);
                pendingNoteOff = -1;
            }

            if (sampleCounter <= 0)
            {
                // Finish previous note
                if (currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    currentNote = -1;
                }

                // Advance step
                currentStep = (currentStep + 1) % numSteps;
                const auto& st = steps[static_cast<size_t> (currentStep)];

                currentMod = st.modValue;

                if (st.active)
                {
                    // Probability
                    if (juce::Random::getSystemRandom().nextFloat() <= st.probability)
                    {
                        int note = rootNote + st.noteOffset;
                        note = juce::jlimit (0, 127, note);
                        outMidi.addEvent (juce::MidiMessage::noteOn (1, note, st.velocity), i);
                        currentNote = note;
                        gateSamplesLeft = static_cast<int> (samplesPerStep * st.gate);
                    }
                }

                sampleCounter = samplesPerStep;
            }

            if (currentNote >= 0 && gateSamplesLeft > 0)
            {
                --gateSamplesLeft;
                if (gateSamplesLeft <= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    currentNote = -1;
                }
            }

            --sampleCounter;
        }
    }

    void initDefaultPattern()
    {
        // Simple psy-ish pattern
        const int offsets[16] = { 0, 0, 7, 0, 12, 0, 7, 5, 0, 0, 7, 0, 12, 7, 5, 3 };
        for (int i = 0; i < NumSteps; ++i)
        {
            steps[static_cast<size_t>(i)].noteOffset = offsets[i];
            steps[static_cast<size_t>(i)].velocity = 0.7f + 0.3f * ((i % 4) == 0 ? 1.0f : 0.6f);
            steps[static_cast<size_t>(i)].gate = 0.5f + 0.3f * (i % 3 == 0 ? 1.0f : 0.0f);
            steps[static_cast<size_t>(i)].probability = 1.0f;
            steps[static_cast<size_t>(i)].modValue = (i % 4 == 0) ? 0.4f : 0.0f;
            steps[static_cast<size_t>(i)].active = true;
        }
    }

private:
    void updateTiming() noexcept
    {
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
        if (samplesPerStep < 32) samplesPerStep = 32;
    }

    double sr = 44100.0;
    double bpm = 175.0;
    int rateDivisor = 4;
    int samplesPerStep = 1000;
    int sampleCounter = 0;
    int currentStep = -1;
    int numSteps = 16;
    int rootNote = 36; // low C for bass
    int currentNote = -1;
    int pendingNoteOff = -1;
    int gateSamplesLeft = 0;
    float currentMod = 0.0f;
    bool enabled = false;
    std::array<Step, NumSteps> steps {};
};

} // namespace salek

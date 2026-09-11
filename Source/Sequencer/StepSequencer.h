#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

/** Real 16-step sequencer. Outputs note events + per-step modulation.
    Never auto-plays: requires hasRoot from a user note-on while enabled. */
class StepSequencer
{
public:
    static constexpr int NumSteps = 16;

    struct Step
    {
        int   noteOffset = 0;
        float velocity   = 0.8f;
        float gate       = 0.7f;
        float probability = 1.0f;
        float modValue   = 0.0f;
        bool  active     = false; // silent until user arms pattern
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
            hasRoot = false; // stop auto-play when disabled
        }
    }

    void setRootNote (int note) noexcept
    {
        rootNote = juce::jlimit (0, 127, note);
        hasRoot = true; // only user MIDI arms the sequencer
    }

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

    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        // CRITICAL: no sound until user provides a root note while seq is on
        if (! enabled || ! hasRoot)
            return;

        for (int i = 0; i < numSamples; ++i)
        {
            if (pendingNoteOff >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, pendingNoteOff), i);
                pendingNoteOff = -1;
            }

            samplesUntilNext -= 1.0;
            if (samplesUntilNext <= 0.0)
            {
                if (currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    currentNote = -1;
                }

                const auto& st = steps[static_cast<size_t> (currentStep)];
                if (st.active)
                {
                    const float r = (float) std::rand() / (float) RAND_MAX;
                    if (r <= st.probability)
                    {
                        int note = juce::jlimit (0, 127, rootNote + st.noteOffset);
                        float vel = juce::jlimit (0.01f, 1.0f, st.velocity);
                        outMidi.addEvent (juce::MidiMessage::noteOn (1, note, vel), i);
                        currentNote = note;
                        gateSamplesLeft = (int) (st.gate * samplesPerStep);
                        currentMod = st.modValue;
                    }
                }

                currentStep = (currentStep + 1) % numSteps;
                samplesUntilNext += samplesPerStep;
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
        }
    }

    void initDefaultPattern()
    {
        for (int i = 0; i < NumSteps; ++i)
        {
            steps[static_cast<size_t>(i)].noteOffset = 0;
            steps[static_cast<size_t>(i)].velocity = 0.8f;
            steps[static_cast<size_t>(i)].gate = 0.6f;
            steps[static_cast<size_t>(i)].probability = 1.0f;
            steps[static_cast<size_t>(i)].modValue = 0.0f;
            steps[static_cast<size_t>(i)].active = (i % 4 == 0); // pattern ready, silent until armed
        }
        currentStep = 0;
        hasRoot = false;
    }

private:
    void updateTiming()
    {
        const double beatsPerStep = 1.0 / (double) juce::jmax (1, rateDivisor);
        samplesPerStep = (sr * 60.0 / bpm) * beatsPerStep;
        if (samplesUntilNext <= 0.0 || samplesUntilNext > samplesPerStep * 2.0)
            samplesUntilNext = samplesPerStep;
    }

    double sr = 44100.0;
    double bpm = 140.0;
    int rateDivisor = 4;
    double samplesPerStep = 1000.0;
    double samplesUntilNext = 0.0;
    int currentStep = 0;
    int numSteps = NumSteps;
    int currentNote = -1;
    int pendingNoteOff = -1;
    int gateSamplesLeft = 0;
    float currentMod = 0.0f;
    bool enabled = false;
    int rootNote = 36;
    bool hasRoot = false; // no auto-play until user note
    std::array<Step, NumSteps> steps {};
};

} // namespace salek

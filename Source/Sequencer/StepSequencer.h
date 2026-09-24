#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

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
        bool  active     = false;
        bool  accent     = false;
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
    void setSwing (float s) noexcept { swing = juce::jlimit (0.f, 1.f, s); }
    void setGateScale (float g) noexcept { gateScale = juce::jlimit (0.05f, 1.f, g); }
    void setNumSteps (int n) noexcept { numStepsActive = juce::jlimit (8, NumSteps, n); }

    /** DAW transport: only advance while host is playing. */
    void setTransportPlaying (bool playing) noexcept
    {
        if (transportPlaying && ! playing)
        {
            // stopping — force note-offs next process
            stopRequested = true;
            samplesUntilNext = 0.0;
            lastPpqStep = -1;
        }
        transportPlaying = playing;
    }

    /** Host PPQ position for sample-accurate step lock (-1 = free-run). */
    void setHostPpq (double ppq) noexcept
    {
        hostPpq = ppq;
        usePpqSync = (ppq >= 0.0);
    }

    bool isEnabled() const noexcept { return enabled; }

    void setEnabled (bool e) noexcept
    {
        enabled = e;
        if (! e) { hasRoot = false; currentNote = -1; pendingNoteOff = -1; lastPpqStep = -1; }
    }

    void setRootNote (int n) noexcept { rootNote = juce::jlimit (0, 127, n); hasRoot = true; }

    Step& getStep (int i) noexcept
    {
        return steps[static_cast<size_t> (juce::jlimit (0, NumSteps - 1, i))];
    }
    const Step& getStep (int i) const noexcept
    {
        return steps[static_cast<size_t> (juce::jlimit (0, NumSteps - 1, i))];
    }

    void setStepMod (int i, float v) noexcept
    {
        getStep(i).modValue = juce::jlimit (0.f, 1.f, v);
    }
    int getCurrentStep() const noexcept { return currentStep; }
    float getCurrentMod() const noexcept { return currentMod; }

    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        auto allNotesOff = [&] (int samplePos)
        {
            if (pendingNoteOff >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, pendingNoteOff), samplePos);
                pendingNoteOff = -1;
            }
            if (currentNote >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), samplePos);
                currentNote = -1;
            }
            gateSamplesLeft = 0;
        };

        if (! enabled || ! hasRoot || ! transportPlaying || stopRequested)
        {
            allNotesOff (0);
            stopRequested = false;
            return;
        }

        // PPQ-locked step (sync with DAW timeline)
        if (usePpqSync && hostPpq >= 0.0)
        {
            const double stepsPerBeat = 4.0 * (double) juce::jmax (1, rateDivisor);
            const int nAct = juce::jmax (1, numStepsActive);
            const int targetStep = ((int) std::floor (hostPpq * stepsPerBeat)) % nAct;
            const int safeStep = (targetStep + nAct) % nAct;

            // Gate handling across block
            for (int i = 0; i < numSamples; ++i)
            {
                if (gateSamplesLeft > 0)
                {
                    --gateSamplesLeft;
                    if (gateSamplesLeft == 0 && currentNote >= 0)
                    {
                        outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                        pendingNoteOff = -1;
                        currentNote = -1;
                    }
                }
            }

            if (safeStep != lastPpqStep)
            {
                // close previous note at block start if still open
                if (currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), 0);
                    currentNote = -1;
                    pendingNoteOff = -1;
                    gateSamplesLeft = 0;
                }
                currentStep = safeStep;
                lastPpqStep = safeStep;
                auto& st = steps[static_cast<size_t> (currentStep)];
                currentMod = st.modValue;
                if (st.active && juce::Random::getSystemRandom().nextFloat() <= st.probability)
                {
                    int note = juce::jlimit (0, 127, rootNote + st.noteOffset);
                    float vel = juce::jlimit (0.0f, 1.0f, st.accent ? juce::jmin (1.0f, st.velocity * 1.35f) : st.velocity);
                    outMidi.addEvent (juce::MidiMessage::noteOn (1, note, vel), 0);
                    currentNote = note;
                    pendingNoteOff = note;
                    gateSamplesLeft = (int) (samplesPerStep * juce::jlimit (0.05f, 1.0f, st.gate * gateScale));
                }
            }
            return;
        }

        // Free-run (standalone / no PPQ)
        for (int i = 0; i < numSamples; ++i)
        {
            if (gateSamplesLeft > 0)
            {
                --gateSamplesLeft;
                if (gateSamplesLeft == 0 && currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    pendingNoteOff = -1;
                    currentNote = -1;
                }
            }

            samplesUntilNext -= 1.0;
            if (samplesUntilNext <= 0.0)
            {
                if (currentNote >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                    currentNote = -1;
                    pendingNoteOff = -1;
                }
                currentStep = (currentStep + 1) % juce::jmax (1, numStepsActive);
                samplesUntilNext += samplesPerStep;

                auto& st = steps[static_cast<size_t> (currentStep)];
                currentMod = st.modValue;

                if (st.active && juce::Random::getSystemRandom().nextFloat() <= st.probability)
                {
                    int note = juce::jlimit (0, 127, rootNote + st.noteOffset);
                    float vel = juce::jlimit (0.0f, 1.0f, st.accent ? juce::jmin (1.0f, st.velocity * 1.35f) : st.velocity);
                    outMidi.addEvent (juce::MidiMessage::noteOn (1, note, vel), i);
                    currentNote = note;
                    pendingNoteOff = note;
                    gateSamplesLeft = (int) (samplesPerStep * juce::jlimit (0.05f, 1.0f, st.gate * gateScale));
                }
            }
        }
    }

    void initDefaultPattern() { reset(); }

    void reset()
    {
        for (int i = 0; i < NumSteps; ++i)
        {
            steps[static_cast<size_t>(i)].active = false;
            steps[static_cast<size_t>(i)].noteOffset = 0;
            steps[static_cast<size_t>(i)].velocity = 0.8f;
            steps[static_cast<size_t>(i)].accent = false;
            steps[static_cast<size_t>(i)].gate = 0.7f;
            steps[static_cast<size_t>(i)].probability = 1.0f;
            steps[static_cast<size_t>(i)].modValue = (float) i / (float) juce::jmax (1, NumSteps - 1);
        }
        currentStep = 0;
        samplesUntilNext = 0.0;
        currentNote = -1;
        pendingNoteOff = -1;
        gateSamplesLeft = 0;
        lastPpqStep = -1;
    }

private:
    void updateTiming()
    {
        const double beatsPerSec = juce::jmax (1.0, bpm) / 60.0;
        const double stepsPerBeat = 4.0 * (double) juce::jmax (1, rateDivisor);
        samplesPerStep = sr / (beatsPerSec * stepsPerBeat);
        samplesPerStep = juce::jmax (1.0, samplesPerStep);
    }

    std::array<Step, NumSteps> steps {};
    double sr = 44100.0;
    double bpm = 120.0;
    int rateDivisor = 1;
    double samplesPerStep = 5512.5;
    double samplesUntilNext = 0.0;
    int currentStep = 0;
    int currentNote = -1;
    int pendingNoteOff = -1;
    int gateSamplesLeft = 0;
    int rootNote = 60;
    bool hasRoot = false;
    bool enabled = false;
    float currentMod = 0.0f;
    float swing = 0.f;
    float gateScale = 0.85f;
    int numStepsActive = 16;
    bool transportPlaying = true; // true until host reports stop
    bool stopRequested = false;
    bool usePpqSync = false;
    double hostPpq = -1.0;
    int lastPpqStep = -1;
};

} // namespace salek

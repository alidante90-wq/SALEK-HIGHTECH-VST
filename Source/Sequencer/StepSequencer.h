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

    bool isEnabled() const noexcept { return enabled; }

    void setEnabled (bool e) noexcept
    {
        enabled = e;
        if (! e) { hasRoot = false; currentNote = -1; pendingNoteOff = -1; }
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

    int getCurrentStep() const noexcept { return currentStep; }
    float getCurrentMod() const noexcept { return currentMod; }

    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        if (! enabled || ! hasRoot)
        {
            if (pendingNoteOff >= 0)
            {
                outMidi.addEvent (juce::MidiMessage::noteOff (1, pendingNoteOff), 0);
                pendingNoteOff = -1;
                currentNote = -1;
            }
            return;
        }

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
                }

                const auto& st = steps[static_cast<size_t> (currentStep)];
                if (st.active)
                {
                    int note = juce::jlimit (0, 127, rootNote + st.noteOffset);
                    float vel = juce::jlimit (0.0f, 1.0f, st.velocity);
                    outMidi.addEvent (juce::MidiMessage::noteOn (1, note, vel), i);
                    currentNote = note;
                    pendingNoteOff = note;
                    gateSamplesLeft = (int) (samplesPerStep * juce::jlimit (0.05f, 1.0f, st.gate));
                    currentMod = st.modValue;
                }
                currentStep = (currentStep + 1) % NumSteps;
                samplesUntilNext += samplesPerStep;
            }
        }
    }

    void initDefaultPattern() { reset(); }

    void reset()
    {
        for (int i = 0; i < NumSteps; ++i)
        {
            steps[static_cast<size_t>(i)].noteOffset = 0;
            steps[static_cast<size_t>(i)].velocity = 0.8f;
            steps[static_cast<size_t>(i)].gate = 0.6f;
            steps[static_cast<size_t>(i)].probability = 1.0f;
            steps[static_cast<size_t>(i)].modValue = 0.0f;
            steps[static_cast<size_t>(i)].active = (i % 4 == 0);
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
    int currentNote = -1;
    int pendingNoteOff = -1;
    int gateSamplesLeft = 0;
    float currentMod = 0.0f;
    bool enabled = false;
    int rootNote = 36;
    bool hasRoot = false;
    std::array<Step, NumSteps> steps {};
};

} // namespace salek

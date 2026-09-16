#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>

// Simple but real arpeggiator + step sequencer skeleton
class Arpeggiator
{
public:
    enum class Mode { Up, Down, UpDown, Random, AsPlayed, Chord };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        reset();
    }

    void setEnabled (bool e) { enabled = e; }
    void setRate (float hz) { rateHz = juce::jmax (0.1f, hz); }
    void setMode (Mode m) { mode = m; }
    void setOctaves (int o) { octaves = juce::jlimit (1, 4, o); }
    void setGate (float g) { gate = juce::jlimit (0.05f, 1.0f, g); }
    void setHold (bool h) { hold = h; }

    // Call from processBlock with current held notes
    void setHeldNotes (const std::vector<int>& notes)
    {
        heldNotes = notes;
        if (heldNotes.empty() && !hold)
            reset();
        else
            rebuildPattern();
    }

    // Returns true + note + velocity when a step triggers
    bool getNextEvent (int numSamples, int& outNote, float& outVelocity, int& outOffset)
    {
        if (!enabled || pattern.empty()) return false;

        samplesUntilNext -= numSamples;
        if (samplesUntilNext > 0) return false;

        // Trigger
        outNote = pattern[currentStep];
        outVelocity = 0.85f;
        outOffset = 0; // could distribute inside block later
        currentStep = (currentStep + 1) % (int) pattern.size();

        samplesUntilNext += (int) (sr / rateHz);
        return true;
    }

    void reset()
    {
        currentStep = 0;
        samplesUntilNext = 0;
        pattern.clear();
    }

    // Step sequencer style pattern (0 = rest, 1..127 note offset or absolute)
    void setStepPattern (const std::array<int, 16>& steps, int length = 16)
    {
        stepLength = juce::jlimit (1, 16, length);
        stepPattern = steps;
        useStepSeq = true;
        rebuildPattern();
    }

private:
    void rebuildPattern()
    {
        pattern.clear();
        if (heldNotes.empty()) return;

        std::vector<int> base = heldNotes;
        std::sort (base.begin(), base.end());

        if (mode == Mode::Down)
            std::reverse (base.begin(), base.end());
        else if (mode == Mode::Random)
            juce::Random::getSystemRandom().shuffle (base.begin(), base.end());

        for (int o = 0; o < octaves; ++o)
        {
            for (int n : base)
                pattern.push_back (n + o * 12);
            if (mode == Mode::UpDown && o < octaves - 1)
            {
                for (int i = (int) base.size() - 2; i >= 1; --i)
                    pattern.push_back (base[i] + o * 12);
            }
        }

        if (useStepSeq)
        {
            // Override with step pattern relative to lowest note
            int root = base.empty() ? 60 : base[0];
            pattern.clear();
            for (int i = 0; i < stepLength; ++i)
            {
                if (stepPattern[i] > 0)
                    pattern.push_back (root + (stepPattern[i] % 24));
                else
                    pattern.push_back (-1); // rest
            }
        }
    }

    double sr = 44100.0;
    bool enabled = false;
    float rateHz = 8.0f;
    Mode mode = Mode::Up;
    int octaves = 1;
    float gate = 0.6f;
    bool hold = false;
    bool useStepSeq = false;
    int stepLength = 16;
    std::array<int, 16> stepPattern {};
    std::vector<int> heldNotes;
    std::vector<int> pattern;
    int currentStep = 0;
    int samplesUntilNext = 0;
};
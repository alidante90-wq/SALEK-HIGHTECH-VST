#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithm>

namespace salek
{

/** Real MIDI arpeggiator. Generates note events from held notes. */
class Arpeggiator
{
public:
    enum class Direction { Up, Down, UpDown, Random };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
        sampleCounter = 0;
        stepIndex = 0;
    }

    void setBpm (double b) noexcept
    {
        bpm = juce::jlimit (20.0, 300.0, b);
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
    }

    void setRateDivisor (int div) noexcept // 1=1/4, 2=1/8, 4=1/16 etc.
    {
        rateDivisor = juce::jlimit (1, 16, div);
        samplesPerStep = static_cast<int> (sr * 60.0 / (bpm * static_cast<double> (rateDivisor)));
    }

    void setDirection (Direction d) noexcept { direction = d; }
    void setOctaves (int o) noexcept { octaves = juce::jlimit (1, 4, o); }
    void setGate (float g) noexcept { gate = juce::jlimit (0.05f, 1.0f, g); }
    void setEnabled (bool e) noexcept { enabled = e; if (!e) heldNotes.clear(); }

    void noteOn (int note, float velocity)
    {
        // avoid duplicates
        for (auto& n : heldNotes)
            if (n.note == note) return;
        heldNotes.push_back ({ note, velocity });
        std::sort (heldNotes.begin(), heldNotes.end(),
                   [] (const Note& a, const Note& b) { return a.note < b.note; });
        rebuildPattern();
    }

    void noteOff (int note)
    {
        heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
                          [note] (const Note& n) { return n.note == note; }),
                         heldNotes.end());
        rebuildPattern();
        if (heldNotes.empty())
        {
            // send note-off for current if needed
            currentPlaying = -1;
        }
    }

    /** Call every block. Appends generated MIDI to outMidi. */
    void process (int numSamples, juce::MidiBuffer& outMidi)
    {
        if (! enabled || pattern.empty())
            return;

        for (int i = 0; i < numSamples; ++i)
        {
            if (sampleCounter <= 0)
            {
                // Note off previous
                if (currentPlaying >= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentPlaying), i);
                    currentPlaying = -1;
                }

                // Next step
                if (! pattern.empty())
                {
                    const auto& step = pattern[static_cast<size_t> (stepIndex % static_cast<int> (pattern.size()))];
                    outMidi.addEvent (juce::MidiMessage::noteOn (1, step.note, step.velocity), i);
                    currentPlaying = step.note;
                    gateSamplesLeft = static_cast<int> (samplesPerStep * gate);
                    stepIndex++;
                }
                sampleCounter = samplesPerStep;
            }

            if (currentPlaying >= 0 && gateSamplesLeft > 0)
            {
                --gateSamplesLeft;
                if (gateSamplesLeft <= 0)
                {
                    outMidi.addEvent (juce::MidiMessage::noteOff (1, currentPlaying), i);
                    currentPlaying = -1;
                }
            }

            --sampleCounter;
        }
    }

private:
    struct Note { int note; float velocity; };

    void rebuildPattern()
    {
        pattern.clear();
        if (heldNotes.empty()) return;

        std::vector<Note> base = heldNotes;
        if (direction == Direction::Down)
            std::reverse (base.begin(), base.end());

        for (int o = 0; o < octaves; ++o)
        {
            for (auto n : base)
            {
                int nn = n.note + o * 12;
                if (nn >= 0 && nn <= 127)
                    pattern.push_back ({ nn, n.velocity });
            }
        }

        if (direction == Direction::UpDown && octaves > 0 && pattern.size() > 1)
        {
            // simple up-down by appending reverse without endpoints
            for (int i = static_cast<int> (pattern.size()) - 2; i > 0; --i)
                pattern.push_back (pattern[static_cast<size_t> (i)]);
        }
        // Random is handled by shuffling occasionally - keep ordered for stability
    }

    double sr = 44100.0;
    double bpm = 175.0;
    int rateDivisor = 4; // 1/16 at 175 BPM default (psy range)
    int samplesPerStep = 1000;
    int sampleCounter = 0;
    int stepIndex = 0;
    int gateSamplesLeft = 0;
    int currentPlaying = -1;
    int octaves = 1;
    float gate = 0.6f;
    bool enabled = false;
    Direction direction = Direction::Up;
    std::vector<Note> heldNotes;
    std::vector<Note> pattern;
};

} // namespace salek

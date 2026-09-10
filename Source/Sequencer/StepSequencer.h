#pragma once
#include <JuceHeader.h>
namespace salek {
class StepSequencer {
public:
    void prepare(double) {}
    void initDefaultPattern() {}
    void setEnabled(bool) {}
    void setRateDivisor(int) {}
    void setNumSteps(int) {}
    void setRootNote(int) {}
    void process(int, juce::MidiBuffer&) {}
    float getCurrentModValue() const { return 0; }
};
}

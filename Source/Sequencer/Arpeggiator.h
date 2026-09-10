#pragma once
#include <JuceHeader.h>
namespace salek {
class Arpeggiator {
public:
    void prepare(double) {}
    void setEnabled(bool) {}
    void setRateDivisor(int) {}
    void setOctaves(int) {}
    void setGate(float) {}
    void setDirection(int) {}
    void noteOn(int, float) {}
    void noteOff(int) {}
    void process(int, juce::MidiBuffer&) {}
};
}

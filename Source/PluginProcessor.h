#pragma once
#include <JuceHeader.h>
#include "Synth/SynthEngine.h"
#include "Modulation/ModMatrix.h"
#include "FX/SimpleDelay.h"
#include "FX/SimpleChorus.h"
#include "FX/SimpleReverb.h"
#include "Sequencer/StepSequencer.h"
#include "Sequencer/Arpeggiator.h"
#include <map>
#include <vector>

class SalekHightechAudioProcessor : public juce::AudioProcessor
{
public:
    SalekHightechAudioProcessor();
    ~SalekHightechAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return (int) factoryPresets.size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    salek::ModMatrix& getModMatrix() { return modMatrix; }
    salek::StepSequencer& getStepSequencer() { return stepSequencer; }
    salek::Arpeggiator& getArpeggiator() { return arpeggiator; }
    float getOutputPeak() const { return outputPeak.load(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void initFactoryPresets();
    void loadFactoryPreset (int index);

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    salek::SynthEngine synthEngine;
    salek::ModMatrix modMatrix;
    salek::SimpleDelay delay;
    salek::SimpleChorus chorus;
    salek::SimpleReverb reverb;
    salek::Arpeggiator arpeggiator;
    salek::StepSequencer stepSequencer;

    struct FactoryPreset { juce::String name; std::map<juce::String, float> values; };
    std::vector<FactoryPreset> factoryPresets;
    int currentProgram = 0;
    std::atomic<float> outputPeak { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessor)
};

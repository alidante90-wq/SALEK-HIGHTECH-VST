#pragma once
#include <JuceHeader.h>
#include <map>
#include "Synth/SynthEngine.h"
#include "FX/SimpleDelay.h"
#include "Sequencer/Arpeggiator.h"

class SalekHightechAudioProcessor : public juce::AudioProcessor
{
public:
    SalekHightechAudioProcessor();
    ~SalekHightechAudioProcessor() override = default;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
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
    salek::SynthEngine& getSynthEngine() { return synthEngine; }
    void loadFactoryPreset (int index);
    juce::StringArray getPresetNames() const;
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyParamsToEngine();
    void initFactoryPresets();
    juce::AudioProcessorValueTreeState apvts;
    salek::SynthEngine synthEngine;
    salek::SimpleDelay delay;
    salek::Arpeggiator arpeggiator;
    struct Preset { juce::String name; std::map<juce::String, float> values; };
    std::vector<Preset> factoryPresets;
    int currentProgram = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessor)
};

#pragma once

#include <JuceHeader.h>
#include "Synth/SynthEngine.h"
#include "FX/SimpleDelay.h"
#include "Modulation/ModMatrix.h"
#include "Sequencer/Arpeggiator.h"
#include "Sequencer/StepSequencer.h"

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
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    salek::SynthEngine& getSynthEngine() { return synthEngine; }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    salek::SynthEngine synthEngine;
    salek::SimpleDelay delay;
    salek::Arpeggiator arpeggiator;
    salek::StepSequencer stepSequencer;
    float masterDrive = 0.0f;

    // Cached parameter pointers for real-time safety
    std::atomic<float>* osc1TablePos = nullptr;
    std::atomic<float>* osc2TablePos = nullptr;
    std::atomic<float>* osc3TablePos = nullptr;
    std::atomic<float>* osc1Level = nullptr;
    std::atomic<float>* osc2Level = nullptr;
    std::atomic<float>* osc3Level = nullptr;
    std::atomic<float>* osc1Octave = nullptr;
    std::atomic<float>* osc2Octave = nullptr;
    std::atomic<float>* osc3Octave = nullptr;
    std::atomic<float>* osc1Semi = nullptr;
    std::atomic<float>* osc2Semi = nullptr;
    std::atomic<float>* osc3Semi = nullptr;
    std::atomic<float>* osc1Fine = nullptr;
    std::atomic<float>* osc2Fine = nullptr;
    std::atomic<float>* osc3Fine = nullptr;
    std::atomic<float>* osc1Detune = nullptr;
    std::atomic<float>* osc2Detune = nullptr;
    std::atomic<float>* osc3Detune = nullptr;
    std::atomic<float>* osc1Warp = nullptr;
    std::atomic<float>* osc2Warp = nullptr;
    std::atomic<float>* osc3Warp = nullptr;
    std::atomic<float>* osc1Phase = nullptr;
    std::atomic<float>* osc2Phase = nullptr;
    std::atomic<float>* osc3Phase = nullptr;
    std::atomic<float>* osc1Fold = nullptr;
    std::atomic<float>* osc2Fold = nullptr;
    std::atomic<float>* osc3Fold = nullptr;
    std::atomic<float>* osc1Drive = nullptr;
    std::atomic<float>* osc2Drive = nullptr;
    std::atomic<float>* osc3Drive = nullptr;
    std::atomic<float>* fm2to1 = nullptr;
    std::atomic<float>* fm3to1 = nullptr;
    std::atomic<float>* fm3to2 = nullptr;
    std::atomic<float>* pm2to1 = nullptr;
    std::atomic<float>* pm3to1 = nullptr;
    std::atomic<float>* am2to1 = nullptr;
    std::atomic<float>* rm2to1 = nullptr;
    std::atomic<float>* filterCutoff = nullptr;
    std::atomic<float>* filterReso = nullptr;
    std::atomic<float>* filterDrive = nullptr;
    std::atomic<float>* filterMode = nullptr;
    std::atomic<float>* filterEnvAmt = nullptr;
    std::atomic<float>* ampAttack = nullptr;
    std::atomic<float>* ampDecay = nullptr;
    std::atomic<float>* ampSustain = nullptr;
    std::atomic<float>* ampRelease = nullptr;
    std::atomic<float>* lfoRate = nullptr;
    std::atomic<float>* lfoAmount = nullptr;
    std::atomic<float>* lfoWave = nullptr;
    std::atomic<float>* macro1 = nullptr;
    std::atomic<float>* macro2 = nullptr;
    std::atomic<float>* macro3 = nullptr;
    std::atomic<float>* macro4 = nullptr;
    std::atomic<float>* delayTime = nullptr;
    std::atomic<float>* delayFb = nullptr;
    std::atomic<float>* delayMix = nullptr;
    std::atomic<float>* masterDrive = nullptr;
    std::atomic<float>* arpEnabled = nullptr;
    std::atomic<float>* arpRate = nullptr;
    std::atomic<float>* arpOctaves = nullptr;
    std::atomic<float>* arpGate = nullptr;
    std::atomic<float>* arpDir = nullptr;
    std::atomic<float>* seqEnabled = nullptr;
    std::atomic<float>* seqRate = nullptr;
    std::atomic<float>* seqSteps = nullptr;
    std::atomic<float>* seqRoot = nullptr;
    std::atomic<float>* labMorph = nullptr;
    std::atomic<float>* labFold = nullptr;
    std::atomic<float>* labDrive = nullptr;
    std::atomic<float>* labRegen = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessor)
};

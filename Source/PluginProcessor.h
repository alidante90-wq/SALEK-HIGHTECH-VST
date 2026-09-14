#pragma once
#include <JuceHeader.h>
#include "Synth/SynthEngine.h"
#include "Effects/SimpleDelay.h"
#include "Effects/SimpleChorus.h"
#include "Effects/SimpleReverb.h"
#include "Effects/SimpleCompressor.h"
#include "Effects/SimpleEQ.h"
#include "Effects/SimpleSpatial.h"
#include "Effects/SimplePhaser.h"
#include "Effects/SimpleDistortion.h"
#include "Sequencer/Arpeggiator.h"
#include "Sequencer/StepSequencer.h"
#include "Modulation/ModMatrix.h"
#include "UI/VisualFifo.h"
#include <map>
#include <vector>
#include <atomic>

class SalekHightechAudioProcessor : public juce::AudioProcessor
{
public:
    SalekHightechAudioProcessor();
    ~SalekHightechAudioProcessor() override;

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
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    float getOutputPeak() const { return outputPeak.load(); }
    VisualFifo& getVisualFifo() { return visualFifo; }
    salek::StepSequencer& getStepSequencer() { return stepSequencer; }
    salek::Arpeggiator& getArpeggiator() { return arpeggiator; }
    salek::ModMatrix& getModMatrix() { return modMatrix; }

    juce::StringArray getPresetNames() const;
    int saveCurrentAsUserPreset (const juce::String& name);
    void loadUserPresetsFromDisk();
    void saveUserPresetsToDisk();
    /** Write current params as a shareable single-preset .salek.xml file */
    bool exportCurrentPresetToFile (const juce::File& file, const juce::String& displayName);
    /** Load a shareable preset file, apply it, add to USER bank. Returns program index or -1 */
    int importPresetFromFile (const juce::File& file);

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyParamsToEngine();
    void initFactoryPresets();
    void loadFactoryPreset (int index);

    juce::AudioProcessorValueTreeState apvts;
    salek::SynthEngine synthEngine;
    salek::SimpleDelay delay;
    salek::SimpleChorus chorus;
    salek::SimpleReverb reverb;
    salek::SimpleCompressor compressor;
    salek::SimpleEQ eq;
    salek::SimpleSpatial spatial;
    salek::SimplePhaser phaser;
    salek::SimpleDistortion distortion;
    salek::Arpeggiator arpeggiator;
    salek::StepSequencer stepSequencer;
    salek::ModMatrix modMatrix;
    juce::MidiKeyboardState keyboardState;
    VisualFifo visualFifo;
    std::atomic<float> outputPeak { 0.f };

    struct FactoryPreset { juce::String name; std::map<juce::String, float> values; };
    std::vector<FactoryPreset> factoryPresets;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessor)
};

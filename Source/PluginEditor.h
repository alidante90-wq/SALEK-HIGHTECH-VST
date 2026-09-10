#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SalekHightechAudioProcessor& processor;

    juce::Slider osc1Level, osc2Level, osc3Level;
    juce::Slider osc1Table, osc2Table, osc3Table;
    juce::Slider osc1Warp, osc2Warp, osc3Warp;
    juce::Slider osc1Fold, osc2Fold, osc3Fold;
    juce::Slider osc1Drive, osc2Drive, osc3Drive;
    juce::Slider filterCutoff, filterReso, filterDrive, filterEnv;
    juce::Slider lfoRate, lfoAmount;
    juce::Slider fm2to1, fm3to1, rm2to1;
    juce::Slider macro1, delayMix, masterDrive;

    using Att = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<Att>> atts;

    juce::Label titleLabel;
    juce::Label secOsc, secFilter, secMod, secFx;

    void addKnob (juce::Slider& s, const juce::String& paramId, juce::Colour accent);
    void styleLabel (juce::Label& l, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessorEditor)
};

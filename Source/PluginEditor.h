#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SalekHightechAudioProcessor& processor;

    // Oscillator section
    juce::Slider osc1Level, osc2Level, osc3Level;
    juce::Slider osc1Table, osc2Table, osc3Table;
    juce::Slider osc1Warp,  osc2Warp,  osc3Warp;
    juce::Slider osc1Fold,  osc2Fold,  osc3Fold;
    juce::Slider osc1Drive, osc2Drive, osc3Drive;

    // Modulation / Filter
    juce::Slider filterCutoff, filterReso, filterDrive, filterEnv;
    juce::Slider lfoRate, lfoAmount;
    juce::Slider fm2to1, fm3to1, rm2to1;

    // Macros + Lab + FX
    juce::Slider macro1, macro2, macro3, macro4;
    juce::Slider labMorph, labFold, delayMix, masterDrive;

    // Arp / Seq
    juce::Slider arpEnabled, seqEnabled;

    using Att = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<Att>> attachments;

    juce::Label titleLabel;
    juce::Label sectionLabels[6];

    void addSlider (juce::Slider& s, const juce::String& paramId);
    void styleSlider (juce::Slider& s, juce::Colour accent);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessorEditor)
};

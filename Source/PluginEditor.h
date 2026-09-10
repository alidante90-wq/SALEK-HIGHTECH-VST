#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit SalekHightechAudioProcessorEditor(SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    SalekHightechAudioProcessor& processor;
    juce::Slider sLevel1, sLevel2, sLevel3, sTable1, sWarp1, sFold1, sCutoff, sFm, sDelay;
    using Att = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<Att>> atts;
    juce::Label title;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

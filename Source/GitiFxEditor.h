#pragma once
#include <JuceHeader.h>
#include "GitiFxProcessor.h"

class GitiFxEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GitiFxEditor(GitiFxAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    GitiFxAudioProcessor& processor;
    juce::OwnedArray<juce::Slider> knobs;
    juce::OwnedArray<juce::Label> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> atts;
    juce::ComboBox modeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAtt;
    juce::TextButton freezeBtn{"FREEZE"};
    juce::Colour cyan{0xff00e8ff}, pink{0xffff2d9b}, purple{0xff9b4dff};
    void addKnob(const char* id,const char* text,int x,int y,int w=100);
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GitiFxEditor)
};

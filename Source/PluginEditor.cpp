#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor(SalekHightechAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setSize(700, 400);
    title.setText("SALEK HIGHTECH", juce::dontSendNotification);
    title.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::cyan);
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);
    auto setup = [this](juce::Slider& s, const char* id) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::magenta);
        addAndMakeVisible(s);
        atts.push_back(std::make_unique<Att>(processor.getAPVTS(), id, s));
    };
    setup(sLevel1,"osc1_level"); setup(sLevel2,"osc2_level"); setup(sLevel3,"osc3_level");
    setup(sTable1,"osc1_table"); setup(sWarp1,"osc1_warp"); setup(sFold1,"osc1_fold");
    setup(sCutoff,"filter_cutoff"); setup(sFm,"fm_2to1"); setup(sDelay,"delay_mix");
}

void SalekHightechAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff080810));
    g.setColour(juce::Colours::magenta.withAlpha(0.3f));
    g.drawRect(getLocalBounds(), 2);
}

void SalekHightechAudioProcessorEditor::resized() {
    auto a = getLocalBounds().reduced(10);
    title.setBounds(a.removeFromTop(40));
    a.removeFromTop(10);
    auto row = a.removeFromTop(120);
    int w = row.getWidth() / 9;
    juce::Slider* sliders[] = {&sLevel1,&sLevel2,&sLevel3,&sTable1,&sWarp1,&sFold1,&sCutoff,&sFm,&sDelay};
    for (auto* s : sliders) s->setBounds(row.removeFromLeft(w).reduced(4));
}

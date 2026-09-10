#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (960, 640);
    setResizable (true, true);
    setResizeLimits (800, 520, 1400, 900);

    titleLabel.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (36.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::cyan);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    styleLabel (secOsc, "OSCILLATORS  \u2014  Level \u00b7 Table \u00b7 Warp \u00b7 Fold \u00b7 Drive");
    styleLabel (secFilter, "FILTER + LFO");
    styleLabel (secMod, "CROSS-MOD  FM / RM");
    styleLabel (secFx, "MACRO \u00b7 DELAY \u00b7 DRIVE");

    auto cyan = juce::Colours::cyan;
    auto mag  = juce::Colours::magenta;
    auto org  = juce::Colours::orange;
    auto grn  = juce::Colours::limegreen;

    addKnob (osc1Level, "osc1_level", cyan); addKnob (osc2Level, "osc2_level", cyan); addKnob (osc3Level, "osc3_level", cyan);
    addKnob (osc1Table, "osc1_table", mag);  addKnob (osc2Table, "osc2_table", mag);  addKnob (osc3Table, "osc3_table", mag);
    addKnob (osc1Warp,  "osc1_warp",  org);  addKnob (osc2Warp,  "osc2_warp",  org);  addKnob (osc3Warp,  "osc3_warp",  org);
    addKnob (osc1Fold,  "osc1_fold",  mag);  addKnob (osc2Fold,  "osc2_fold",  mag);  addKnob (osc3Fold,  "osc3_fold",  mag);
    addKnob (osc1Drive, "osc1_drive", org);  addKnob (osc2Drive, "osc2_drive", org);  addKnob (osc3Drive, "osc3_drive", org);

    addKnob (filterCutoff, "filter_cutoff", cyan);
    addKnob (filterReso,   "filter_reso",   mag);
    addKnob (filterDrive,  "filter_drive",  org);
    addKnob (filterEnv,    "filter_env",    grn);
    addKnob (lfoRate,      "lfo_rate",      cyan);
    addKnob (lfoAmount,    "lfo_amount",    mag);

    addKnob (fm2to1, "fm_2to1", org);
    addKnob (fm3to1, "fm_3to1", org);
    addKnob (rm2to1, "rm_2to1", mag);

    addKnob (macro1,      "macro1",       cyan);
    addKnob (delayMix,    "delay_mix",    org);
    addKnob (masterDrive, "master_drive", mag);
}

void SalekHightechAudioProcessorEditor::styleLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setColour (juce::Label::textColourId, juce::Colours::violet);
    l.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (l);
}

void SalekHightechAudioProcessorEditor::addKnob (juce::Slider& s, const juce::String& paramId, juce::Colour accent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 54, 14);
    s.setColour (juce::Slider::rotarySliderFillColourId, accent);
    s.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, accent.darker (0.7f));
    addAndMakeVisible (s);
    atts.push_back (std::make_unique<Att> (processor.getAPVTS(), paramId, s));
}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff06060e));
    g.setColour (juce::Colours::magenta.withAlpha (0.4f));
    g.drawRect (getLocalBounds(), 3);
    g.setColour (juce::Colours::cyan.withAlpha (0.2f));
    g.drawRect (getLocalBounds().reduced (3), 1);
    g.setColour (juce::Colours::cyan.withAlpha (0.3f));
    g.fillRect (40, 48, getWidth() - 80, 2);
    auto bounds = getLocalBounds().reduced (12);
    bounds.removeFromTop (52);
    g.setColour (juce::Colour (0xff0c0c18));
    g.fillRoundedRectangle (bounds.toFloat(), 8.0f);
    g.setColour (juce::Colours::violet.withAlpha (0.15f));
    g.drawRoundedRectangle (bounds.toFloat(), 8.0f, 1.0f);
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    titleLabel.setBounds (area.removeFromTop (40));
    area.removeFromTop (12);

    auto placeRow = [&] (juce::Label& lab, std::initializer_list<juce::Slider*> sliders, int rowH = 100)
    {
        auto row = area.removeFromTop (rowH);
        lab.setBounds (row.removeFromTop (18));
        const int n = (int) sliders.size();
        if (n == 0) return;
        const int w = row.getWidth() / n;
        for (auto* s : sliders)
            s->setBounds (row.removeFromLeft (w).reduced (4));
    };

    placeRow (secOsc, { &osc1Level, &osc2Level, &osc3Level, &osc1Table, &osc2Table, &osc3Table }, 95);
    placeRow (secOsc, { &osc1Warp, &osc2Warp, &osc3Warp, &osc1Fold, &osc2Fold, &osc3Fold, &osc1Drive, &osc2Drive, &osc3Drive }, 95);
    placeRow (secFilter, { &filterCutoff, &filterReso, &filterDrive, &filterEnv, &lfoRate, &lfoAmount }, 100);
    placeRow (secMod, { &fm2to1, &fm3to1, &rm2to1 }, 100);
    placeRow (secFx, { &macro1, &delayMix, &masterDrive }, 100);
}

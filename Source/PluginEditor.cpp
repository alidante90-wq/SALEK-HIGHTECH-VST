#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (980, 720);
    setResizable (true, true);
    setResizeLimits (800, 560, 1400, 1000);

    titleLabel.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (36.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::cyan);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    const char* sections[] = {
        "OSCILLATORS  —  Level / Table / Warp / Fold / Drive",
        "FILTER + LFO",
        "CROSS-MOD  FM / RM",
        "MACROS + WAVETABLE LAB",
        "FX",
        "ARP / SEQ"
    };
    for (int i = 0; i < 6; ++i)
    {
        sectionLabels[i].setText (sections[i], juce::dontSendNotification);
        sectionLabels[i].setColour (juce::Label::textColourId, juce::Colours::violet);
        sectionLabels[i].setFont (juce::FontOptions (14.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabels[i]);
    }

    auto cyan = juce::Colours::cyan;
    auto mag  = juce::Colours::magenta;
    auto org  = juce::Colours::orange;
    auto grn  = juce::Colours::limegreen;

    styleSlider (osc1Level, cyan); styleSlider (osc2Level, cyan); styleSlider (osc3Level, cyan);
    styleSlider (osc1Table, mag);  styleSlider (osc2Table, mag);  styleSlider (osc3Table, mag);
    styleSlider (osc1Warp,  org);  styleSlider (osc2Warp,  org);  styleSlider (osc3Warp,  org);
    styleSlider (osc1Fold,  mag);  styleSlider (osc2Fold,  mag);  styleSlider (osc3Fold,  mag);
    styleSlider (osc1Drive, org);  styleSlider (osc2Drive, org);  styleSlider (osc3Drive, org);

    styleSlider (filterCutoff, cyan); styleSlider (filterReso, mag);
    styleSlider (filterDrive, org);   styleSlider (filterEnv, grn);
    styleSlider (lfoRate, cyan);      styleSlider (lfoAmount, mag);

    styleSlider (fm2to1, org); styleSlider (fm3to1, org); styleSlider (rm2to1, mag);

    styleSlider (macro1, cyan); styleSlider (macro2, mag);
    styleSlider (macro3, org);  styleSlider (macro4, grn);
    styleSlider (labMorph, cyan); styleSlider (labFold, mag);
    styleSlider (delayMix, org);  styleSlider (masterDrive, mag);

    styleSlider (arpEnabled, grn); styleSlider (seqEnabled, cyan);

    addSlider (osc1Level, "osc1_level"); addSlider (osc2Level, "osc2_level"); addSlider (osc3Level, "osc3_level");
    addSlider (osc1Table, "osc1_table"); addSlider (osc2Table, "osc2_table"); addSlider (osc3Table, "osc3_table");
    addSlider (osc1Warp,  "osc1_warp");  addSlider (osc2Warp,  "osc2_warp");  addSlider (osc3Warp,  "osc3_warp");
    addSlider (osc1Fold,  "osc1_fold");  addSlider (osc2Fold,  "osc2_fold");  addSlider (osc3Fold,  "osc3_fold");
    addSlider (osc1Drive, "osc1_drive"); addSlider (osc2Drive, "osc2_drive"); addSlider (osc3Drive, "osc3_drive");

    addSlider (filterCutoff, "filter_cutoff"); addSlider (filterReso, "filter_reso");
    addSlider (filterDrive, "filter_drive");   addSlider (filterEnv, "filter_env");
    addSlider (lfoRate, "lfo_rate");           addSlider (lfoAmount, "lfo_amount");

    addSlider (fm2to1, "fm_2to1"); addSlider (fm3to1, "fm_3to1"); addSlider (rm2to1, "rm_2to1");

    addSlider (macro1, "macro1"); addSlider (macro2, "macro2");
    addSlider (macro3, "macro3"); addSlider (macro4, "macro4");
    addSlider (labMorph, "lab_morph"); addSlider (labFold, "lab_fold");
    addSlider (delayMix, "delay_mix"); addSlider (masterDrive, "master_drive");

    addSlider (arpEnabled, "arp_enabled"); addSlider (seqEnabled, "seq_enabled");
}

void SalekHightechAudioProcessorEditor::styleSlider (juce::Slider& s, juce::Colour accent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 54, 14);
    s.setColour (juce::Slider::rotarySliderFillColourId, accent);
    s.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, accent.darker (0.6f));
    addAndMakeVisible (s);
}

void SalekHightechAudioProcessorEditor::addSlider (juce::Slider& s, const juce::String& paramId)
{
    attachments.push_back (std::make_unique<Att> (processor.getAPVTS(), paramId, s));
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor() {}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark cyberpunk background
    g.fillAll (juce::Colour (0xff06060e));

    // Neon border
    g.setColour (juce::Colours::magenta.withAlpha (0.35f));
    g.drawRect (getLocalBounds(), 3);
    g.setColour (juce::Colours::cyan.withAlpha (0.2f));
    g.drawRect (getLocalBounds().reduced (3), 1);

    // Title underline glow
    g.setColour (juce::Colours::cyan.withAlpha (0.25f));
    g.fillRect (40, 48, getWidth() - 80, 2);
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    titleLabel.setBounds (area.removeFromTop (44));
    area.removeFromTop (6);

    auto placeRow = [&] (juce::Label& lab, std::initializer_list<juce::Slider*> sliders, int rowH = 92)
    {
        auto row = area.removeFromTop (rowH);
        lab.setBounds (row.removeFromTop (16));
        const int n = static_cast<int> (sliders.size());
        if (n == 0) return;
        const int w = row.getWidth() / n;
        for (auto* s : sliders)
            s->setBounds (row.removeFromLeft (w).reduced (3));
    };

    placeRow (sectionLabels[0],
              { &osc1Level, &osc2Level, &osc3Level,
                &osc1Table, &osc2Table, &osc3Table,
                &osc1Warp,  &osc2Warp,  &osc3Warp }, 100);

    placeRow (sectionLabels[0], // second osc row (fold/drive)
              { &osc1Fold, &osc2Fold, &osc3Fold,
                &osc1Drive, &osc2Drive, &osc3Drive }, 90);

    placeRow (sectionLabels[1],
              { &filterCutoff, &filterReso, &filterDrive, &filterEnv, &lfoRate, &lfoAmount });

    placeRow (sectionLabels[2], { &fm2to1, &fm3to1, &rm2to1 });

    placeRow (sectionLabels[3],
              { &macro1, &macro2, &macro3, &macro4, &labMorph, &labFold });

    placeRow (sectionLabels[4], { &delayMix, &masterDrive });

    placeRow (sectionLabels[5], { &arpEnabled, &seqEnabled }, 80);
}

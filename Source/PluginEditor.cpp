#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&salekLnF);
    setSize (1100, 700);
    setResizable (true, true);
    setResizeLimits (900, 600, 1600, 1000);

    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    title.setFont (juce::FontOptions (42.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, juce::Colours::cyan);
    title.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (title);

    subtitle.setText ("HI-TECH  \u00b7  DARKPSY  \u00b7  FM  \u00b7  ACID  \u00b7  ALIEN", juce::dontSendNotification);
    subtitle.setFont (juce::FontOptions (13.0f));
    subtitle.setColour (juce::Label::textColourId, juce::Colours::magenta);
    subtitle.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (subtitle);

    auto header = [] (juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        l.setColour (juce::Label::textColourId, juce::Colours::violet);
    };
    header (hOsc, "OSCILLATORS");
    header (hFilter, "FILTER  /  LFO");
    header (hMod, "CROSS MODULATION");
    header (hFx, "OUTPUT");
    for (auto* l : { &hOsc, &hFilter, &hMod, &hFx })
        addAndMakeVisible (*l);

    auto cyan = juce::Colours::cyan;
    auto mag  = juce::Colours::magenta;
    auto org  = juce::Colours::orange;
    auto grn  = juce::Colours::lime;

    setupKnob (kOsc1Lvl, "osc1_level", "OSC1 LVL", cyan);
    setupKnob (kOsc2Lvl, "osc2_level", "OSC2 LVL", cyan);
    setupKnob (kOsc3Lvl, "osc3_level", "OSC3 LVL", cyan);
    setupKnob (kOsc1Tab, "osc1_table", "TABLE 1", mag);
    setupKnob (kOsc2Tab, "osc2_table", "TABLE 2", mag);
    setupKnob (kOsc3Tab, "osc3_table", "TABLE 3", mag);
    setupKnob (kOsc1Warp,"osc1_warp",  "WARP 1",  org);
    setupKnob (kOsc2Warp,"osc2_warp",  "WARP 2",  org);
    setupKnob (kOsc3Warp,"osc3_warp",  "WARP 3",  org);
    setupKnob (kOsc1Fold,"osc1_fold",  "FOLD 1",  mag);
    setupKnob (kOsc2Fold,"osc2_fold",  "FOLD 2",  mag);
    setupKnob (kOsc3Fold,"osc3_fold",  "FOLD 3",  mag);
    setupKnob (kOsc1Drv, "osc1_drive", "DRIVE 1", org);
    setupKnob (kOsc2Drv, "osc2_drive", "DRIVE 2", org);
    setupKnob (kOsc3Drv, "osc3_drive", "DRIVE 3", org);

    setupKnob (kCutoff, "filter_cutoff", "CUTOFF", cyan);
    setupKnob (kReso,   "filter_reso",   "RESO",   mag);
    setupKnob (kFDrive, "filter_drive",  "F-DRIVE", org);
    setupKnob (kFEnv,   "filter_env",    "F-ENV",  grn);
    setupKnob (kLfoRate,"lfo_rate",      "LFO RATE", cyan);
    setupKnob (kLfoAmt, "lfo_amount",    "LFO AMT",  mag);

    setupKnob (kFm21, "fm_2to1", "FM 2>1", org);
    setupKnob (kFm31, "fm_3to1", "FM 3>1", org);
    setupKnob (kRm21, "rm_2to1", "RM 2>1", mag);

    setupKnob (kMacro,  "macro1",       "MACRO",  cyan);
    setupKnob (kDelay,  "delay_mix",    "DELAY",  org);
    setupKnob (kMDrive, "master_drive", "MASTER", mag);

    filterModeBox.addItemList (juce::StringArray { "LowPass", "HighPass", "BandPass", "Notch" }, 1);
    addAndMakeVisible (filterModeBox);
    filterModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.getAPVTS(), "filter_mode", filterModeBox);

    lfoWaveBox.addItemList (juce::StringArray { "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
    addAndMakeVisible (lfoWaveBox);
    if (processor.getAPVTS().getParameter ("lfo_wave") != nullptr)
        lfoWaveAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo_wave", lfoWaveBox);
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SalekHightechAudioProcessorEditor::setupKnob (Knob& k, const juce::String& paramId,
                                                    const juce::String& name, juce::Colour c)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
    k.slider.setColour (juce::Slider::rotarySliderFillColourId, c);
    k.slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible (k.slider);
    atts.push_back (std::make_unique<SAtt> (processor.getAPVTS(), paramId, k.slider));

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    k.label.setColour (juce::Label::textColourId, c.brighter (0.2f));
    addAndMakeVisible (k.label);
}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff04040a));
    g.setColour (juce::Colours::magenta.withAlpha (0.55f));
    g.drawRect (getLocalBounds(), 3);
    g.setColour (juce::Colours::cyan.withAlpha (0.25f));
    g.drawRect (getLocalBounds().reduced (4), 1);
    g.setColour (juce::Colours::cyan.withAlpha (0.15f));
    g.fillRect (0, 0, getWidth(), 70);
    g.setColour (juce::Colours::cyan.withAlpha (0.4f));
    g.fillRect (60, 66, getWidth() - 120, 2);

    auto body = getLocalBounds().reduced (14);
    body.removeFromTop (78);
    auto top = body.removeFromTop (body.getHeight() * 55 / 100);
    auto bot = body;
    auto leftTop  = top.removeFromLeft (top.getWidth() * 62 / 100);
    auto rightTop = top;
    drawPanel (g, leftTop.reduced (4));
    drawPanel (g, rightTop.reduced (4));
    drawPanel (g, bot.removeFromLeft (bot.getWidth() / 2).reduced (4));
    drawPanel (g, bot.reduced (4));
}

void SalekHightechAudioProcessorEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> r)
{
    g.setColour (juce::Colour (0xff0a0a16));
    g.fillRoundedRectangle (r.toFloat(), 10.0f);
    g.setColour (juce::Colours::violet.withAlpha (0.35f));
    g.drawRoundedRectangle (r.toFloat(), 10.0f, 1.5f);
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    title.setBounds (area.removeFromTop (40));
    subtitle.setBounds (area.removeFromTop (22));
    area.removeFromTop (16);

    auto top = area.removeFromTop (area.getHeight() * 55 / 100);
    auto bot = area;
    auto oscArea = top.removeFromLeft (top.getWidth() * 62 / 100).reduced (10);
    auto filtArea = top.reduced (10);

    hOsc.setBounds (oscArea.removeFromTop (20));
    auto place = [] (juce::Rectangle<int>& row, Knob& k, int w)
    {
        auto cell = row.removeFromLeft (w);
        k.label.setBounds (cell.removeFromBottom (16));
        k.slider.setBounds (cell.reduced (2));
    };

    auto row1 = oscArea.removeFromTop (oscArea.getHeight() / 3);
    int w1 = row1.getWidth() / 6;
    for (auto* k : { &kOsc1Lvl, &kOsc2Lvl, &kOsc3Lvl, &kOsc1Tab, &kOsc2Tab, &kOsc3Tab })
        place (row1, *k, w1);

    auto row2 = oscArea.removeFromTop (oscArea.getHeight() / 2);
    int w2 = row2.getWidth() / 6;
    for (auto* k : { &kOsc1Warp, &kOsc2Warp, &kOsc3Warp, &kOsc1Fold, &kOsc2Fold, &kOsc3Fold })
        place (row2, *k, w2);

    auto row3 = oscArea;
    int w3 = row3.getWidth() / 3;
    for (auto* k : { &kOsc1Drv, &kOsc2Drv, &kOsc3Drv })
        place (row3, *k, w3);

    hFilter.setBounds (filtArea.removeFromTop (20));
    auto frow = filtArea.removeFromTop (filtArea.getHeight() * 55 / 100);
    int fw = frow.getWidth() / 4;
    for (auto* k : { &kCutoff, &kReso, &kFDrive, &kFEnv })
        place (frow, *k, fw);

    auto frow2 = filtArea.removeFromTop (filtArea.getHeight() * 60 / 100);
    int fw2 = frow2.getWidth() / 2;
    for (auto* k : { &kLfoRate, &kLfoAmt })
        place (frow2, *k, fw2);

    auto comboRow = filtArea;
    filterModeBox.setBounds (comboRow.removeFromLeft (comboRow.getWidth() / 2).reduced (6));
    lfoWaveBox.setBounds (comboRow.reduced (6));

    auto modArea = bot.removeFromLeft (bot.getWidth() / 2).reduced (10);
    auto fxArea  = bot.reduced (10);
    hMod.setBounds (modArea.removeFromTop (20));
    int mw = modArea.getWidth() / 3;
    for (auto* k : { &kFm21, &kFm31, &kRm21 })
        place (modArea, *k, mw);
    hFx.setBounds (fxArea.removeFromTop (20));
    int xw = fxArea.getWidth() / 3;
    for (auto* k : { &kMacro, &kDelay, &kMDrive })
        place (fxArea, *k, xw);
}

#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);
    setSize (1200, 760);
    setResizable (true, true);
    setResizeLimits (1000, 640, 1800, 1100);

    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    title.setFont (juce::FontOptions (44.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, juce::Colour (0xff00f0ff));
    title.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (title);

    tagline.setText ("SYNTHESIS ENGINE  //  HI-TECH  DARKPSY  FM  ACID  ALIEN", juce::dontSendNotification);
    tagline.setFont (juce::FontOptions (12.0f));
    tagline.setColour (juce::Label::textColourId, juce::Colour (0xffff00aa));
    tagline.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (tagline);

    brand.setText ("VST3  |  STANDALONE  |  SALEK SYSTEMS", juce::dontSendNotification);
    brand.setFont (juce::FontOptions (11.0f));
    brand.setColour (juce::Label::textColourId, juce::Colour (0xff6666aa));
    brand.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (brand);

    addAndMakeVisible (scope);

    auto C = juce::Colour (0xff00f0ff);
    auto M = juce::Colour (0xffff00aa);
    auto O = juce::Colour (0xffff8800);
    auto G = juce::Colour (0xff66ff99);

    setup (osc1L, "osc1_level", "LEVEL 1", C);
    setup (osc2L, "osc2_level", "LEVEL 2", C);
    setup (osc3L, "osc3_level", "LEVEL 3", C);
    setup (osc1T, "osc1_table", "TABLE 1", M);
    setup (osc2T, "osc2_table", "TABLE 2", M);
    setup (osc3T, "osc3_table", "TABLE 3", M);
    setup (osc1W, "osc1_warp",  "WARP 1",  O);
    setup (osc2W, "osc2_warp",  "WARP 2",  O);
    setup (osc3W, "osc3_warp",  "WARP 3",  O);
    setup (osc1F, "osc1_fold",  "FOLD 1",  M);
    setup (osc2F, "osc2_fold",  "FOLD 2",  M);
    setup (osc3F, "osc3_fold",  "FOLD 3",  M);
    setup (osc1D, "osc1_drive", "DRIVE 1", O);
    setup (osc2D, "osc2_drive", "DRIVE 2", O);
    setup (osc3D, "osc3_drive", "DRIVE 3", O);

    setup (cut,  "filter_cutoff", "CUTOFF",   C);
    setup (reso, "filter_reso",   "RESO",     M);
    setup (fDrv, "filter_drive",  "F DRIVE",  O);
    setup (fEnv, "filter_env",    "F ENV",    G);
    setup (lfoR, "lfo_rate",      "LFO RATE", C);
    setup (lfoA, "lfo_amount",    "LFO AMT",  M);

    setup (fm21,   "fm_2to1",      "FM 2>1", O);
    setup (fm31,   "fm_3to1",      "FM 3>1", O);
    setup (rm21,   "rm_2to1",      "RM 2>1", M);
    setup (macro,  "macro1",       "MACRO",  C);
    setup (delay,  "delay_mix",    "DELAY",  O);
    setup (master, "master_drive", "MASTER", M);

    filterMode.addItemList (juce::StringArray { "LOW PASS", "HIGH PASS", "BAND PASS", "NOTCH" }, 1);
    addAndMakeVisible (filterMode);
    filterAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.getAPVTS(), "filter_mode", filterMode);

    lfoWave.addItemList (juce::StringArray { "SINE", "TRIANGLE", "SAW", "SQUARE", "S&H" }, 1);
    addAndMakeVisible (lfoWave);
    if (processor.getAPVTS().getParameter ("lfo_wave") != nullptr)
        lfoAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo_wave", lfoWave);
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SalekHightechAudioProcessorEditor::setup (Knob& k, const char* id, const char* label, juce::Colour c)
{
    k.s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    k.s.setColour (juce::Slider::rotarySliderFillColourId, c);
    addAndMakeVisible (k.s);
    atts.push_back (std::make_unique<SAtt> (processor.getAPVTS(), id, k.s));
    k.name.setText (label, juce::dontSendNotification);
    k.name.setJustificationType (juce::Justification::centred);
    k.name.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    k.name.setColour (juce::Label::textColourId, c.brighter (0.15f));
    addAndMakeVisible (k.name);
}

void SalekHightechAudioProcessorEditor::paintPanel (juce::Graphics& g, juce::Rectangle<float> r,
                                                    const juce::String& header, juce::Colour accent)
{
    g.setColour (juce::Colour (0xff080812));
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (accent.withAlpha (0.7f));
    g.fillRoundedRectangle (r.getX() + 8, r.getY() + 2, r.getWidth() - 16, 2.5f, 1.0f);
    g.setColour (accent.withAlpha (0.25f));
    g.drawRoundedRectangle (r, 8.0f, 1.2f);
    g.setColour (accent.withAlpha (0.85f));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (header, r.getX() + 12, r.getY() + 8, r.getWidth() - 24, 16, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colour (0xff030308), 0, 0,
                             juce::Colour (0xff0a0518), 0, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colours::white.withAlpha (0.015f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    g.setColour (juce::Colour (0xffff00aa).withAlpha (0.5f));
    g.drawRect (getLocalBounds(), 2);
    g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.2f));
    g.drawRect (getLocalBounds().reduced (3), 1);

    g.setColour (juce::Colour (0xff0a0a16).withAlpha (0.9f));
    g.fillRect (0, 0, getWidth(), 78);
    g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.35f));
    g.fillRect (0, 76, getWidth(), 2);

    auto body = getLocalBounds().toFloat().reduced (12.0f);
    body.removeFromTop (82.0f);

    auto topH = body.getHeight() * 0.58f;
    auto top = body.removeFromTop (topH);
    auto bot = body;

    auto oscR = top.removeFromLeft (top.getWidth() * 0.58f).reduced (4.0f);
    auto right = top.reduced (4.0f);
    auto scopeR = right.removeFromTop (right.getHeight() * 0.38f);
    auto filtR = right;

    paintPanel (g, oscR, "OSCILLATORS  //  WAVETABLE ENGINE", juce::Colour (0xff00f0ff));
    paintPanel (g, scopeR, "SIGNAL SCOPE", juce::Colour (0xffff00aa));
    paintPanel (g, filtR, "FILTER  //  LFO", juce::Colour (0xffaa66ff));
    paintPanel (g, bot.removeFromLeft (bot.getWidth() * 0.5f).reduced (4.0f),
                "CROSS-MOD  //  FM  RM", juce::Colour (0xffff8800));
    paintPanel (g, bot.reduced (4.0f), "OUTPUT  //  MACRO  DELAY  DRIVE", juce::Colour (0xff66ff99));
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto a = getLocalBounds().reduced (14);
    auto header = a.removeFromTop (64);
    title.setBounds (header.removeFromLeft (420).removeFromTop (36));
    tagline.setBounds (header.removeFromLeft (480).withTrimmedTop (8).removeFromTop (22));
    brand.setBounds (header.withTrimmedTop (12));

    a.removeFromTop (18);
    auto top = a.removeFromTop ((int) (a.getHeight() * 0.58f));
    auto bot = a;

    auto osc = top.removeFromLeft ((int) (top.getWidth() * 0.58f)).reduced (10);
    auto right = top.reduced (10);
    auto scopeArea = right.removeFromTop ((int) (right.getHeight() * 0.38f));
    auto filt = right;

    osc.removeFromTop (28);
    scopeArea.removeFromTop (28);
    scope.setBounds (scopeArea.reduced (6));
    filt.removeFromTop (28);

    auto place = [] (juce::Rectangle<int>& row, Knob& k, int w)
    {
        auto cell = row.removeFromLeft (w);
        k.name.setBounds (cell.removeFromBottom (15));
        k.s.setBounds (cell.reduced (1));
    };

    auto r1 = osc.removeFromTop (osc.getHeight() / 3);
    int w = r1.getWidth() / 6;
    for (auto* k : { &osc1L, &osc2L, &osc3L, &osc1T, &osc2T, &osc3T }) place (r1, *k, w);

    auto r2 = osc.removeFromTop (osc.getHeight() / 2);
    w = r2.getWidth() / 6;
    for (auto* k : { &osc1W, &osc2W, &osc3W, &osc1F, &osc2F, &osc3F }) place (r2, *k, w);

    auto r3 = osc;
    w = r3.getWidth() / 3;
    for (auto* k : { &osc1D, &osc2D, &osc3D }) place (r3, *k, w);

    auto fr1 = filt.removeFromTop ((int) (filt.getHeight() * 0.5f));
    w = fr1.getWidth() / 4;
    for (auto* k : { &cut, &reso, &fDrv, &fEnv }) place (fr1, *k, w);

    auto fr2 = filt.removeFromTop ((int) (filt.getHeight() * 0.55f));
    w = fr2.getWidth() / 2;
    for (auto* k : { &lfoR, &lfoA }) place (fr2, *k, w);

    auto combos = filt;
    filterMode.setBounds (combos.removeFromLeft (combos.getWidth() / 2).reduced (4));
    lfoWave.setBounds (combos.reduced (4));

    auto mod = bot.removeFromLeft (bot.getWidth() / 2).reduced (10);
    auto fx  = bot.reduced (10);
    mod.removeFromTop (28);
    fx.removeFromTop (28);
    w = mod.getWidth() / 3;
    for (auto* k : { &fm21, &fm31, &rm21 }) place (mod, *k, w);
    w = fx.getWidth() / 3;
    for (auto* k : { &macro, &delay, &master }) place (fx, *k, w);
}

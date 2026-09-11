#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (24, 96);
    keyboard.setOctaveForMiddleC (4);
    addAndMakeVisible (keyboard);
    startTimerHz (8);
    setLookAndFeel (&lnf);
    setSize (1180, 800);
    setResizable (true, true);
    setResizeLimits (980, 700, 1600, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("NEBULA", juce::dontSendNotification);
    addAndMakeVisible (tagline);
    addAndMakeVisible (scope);
    wtDisplay = std::make_unique<WavetableDisplay> (processor.getAPVTS());
    addAndMakeVisible (*wtDisplay);
    addAndMakeVisible (tabs);
    tabs.setOutline (0);

    auto C = juce::Colour (0xff00d4ff);
    auto M = juce::Colour (0xffc77dff);
    auto O = juce::Colour (0xffff8800);
    auto G = juce::Colour (0xff66ff99);
    auto V = juce::Colour (0xffaa66ff);

    tabs.addTab ("OSC", juce::Colour (0xff0a0a16), &oscTab, false);
    {
        addKnob (oscTab, "osc1_level", "LVL 1", C); addKnob (oscTab, "osc1_table", "TABLE 1", M);
        addKnob (oscTab, "osc1_warp", "WARP 1", O); addKnob (oscTab, "osc1_fold", "FOLD 1", M);
        addKnob (oscTab, "osc1_drive", "DRIVE 1", O); addKnob (oscTab, "osc1_octave", "OCT 1", V);
        addKnob (oscTab, "osc1_semi", "SEMI 1", V); addKnob (oscTab, "osc1_detune", "DET 1", G);
        addKnob (oscTab, "osc2_level", "LVL 2", C); addKnob (oscTab, "osc2_table", "TABLE 2", M);
        addKnob (oscTab, "osc2_warp", "WARP 2", O); addKnob (oscTab, "osc2_fold", "FOLD 2", M);
        addKnob (oscTab, "osc2_drive", "DRIVE 2", O); addKnob (oscTab, "osc2_octave", "OCT 2", V);
        addKnob (oscTab, "osc2_semi", "SEMI 2", V); addKnob (oscTab, "osc2_detune", "DET 2", G);
        addKnob (oscTab, "osc3_level", "LVL 3", C); addKnob (oscTab, "osc3_table", "TABLE 3", M);
        addKnob (oscTab, "osc3_warp", "WARP 3", O); addKnob (oscTab, "osc3_fold", "FOLD 3", M);
        addKnob (oscTab, "osc3_drive", "DRIVE 3", O); addKnob (oscTab, "osc3_octave", "OCT 3", V);
        addKnob (oscTab, "osc3_semi", "SEMI 3", V); addKnob (oscTab, "osc3_detune", "DET 3", G);
        addKnob (oscTab, "unison_voices", "UNISON", C);
        addKnob (oscTab, "unison_detune", "U DET", M);
        addKnob (oscTab, "unison_spread", "SPREAD", O);
    }
    tabs.addTab ("FILTER", juce::Colour (0xff0a0a16), &filterTab, false);
    {
        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "F DRIVE", O);
        addKnob (filterTab, "filter_env", "F ENV", G);
        addKnob (filterTab, "amp_attack", "ATTACK", C);
        addKnob (filterTab, "amp_decay", "DECAY", M);
        addKnob (filterTab, "amp_sustain", "SUSTAIN", O);
        addKnob (filterTab, "amp_release", "RELEASE", G);
        addCombo (filterTab, filterMode, "filter_mode", {"LOW PASS","HIGH PASS","BAND PASS","NOTCH"});
    }
    tabs.addTab ("MOD", juce::Colour (0xff0a0a16), &modTab, false);
    {
        addKnob (modTab, "fm_2to1", "FM 2>1", O); addKnob (modTab, "fm_3to1", "FM 3>1", O);
        addKnob (modTab, "fm_3to2", "FM 3>2", O); addKnob (modTab, "pm_2to1", "PM 2>1", M);
        addKnob (modTab, "rm_2to1", "RM 2>1", M); addKnob (modTab, "am_2to1", "AM 2>1", V);
        addKnob (modTab, "lfo_rate", "LFO RATE", C); addKnob (modTab, "lfo_amount", "LFO AMT", M);
        addCombo (modTab, lfoWave, "lfo_wave", {"SINE","TRIANGLE","SAW","SQUARE","S&H"});
        addKnob (modTab, "macro1", "MACRO 1", C); addKnob (modTab, "macro2", "MACRO 2", M);
        addKnob (modTab, "macro3", "MACRO 3", O); addKnob (modTab, "macro4", "MACRO 4", G);
    }
    tabs.addTab ("FX", juce::Colour (0xff0a0a16), &fxTab, false);
    {
        addKnob (fxTab, "chorus_mix", "CHORUS", O);
        addKnob (fxTab, "chorus_rate", "C RATE", C);
        addKnob (fxTab, "chorus_depth", "C DEPTH", M);
        addKnob (fxTab, "delay_mix", "DELAY", O);
        addKnob (fxTab, "delay_time", "D TIME", C);
        addKnob (fxTab, "delay_fb", "D FB", M);
        addKnob (fxTab, "reverb_mix", "REVERB", O);
        addKnob (fxTab, "reverb_size", "R SIZE", C);
        addKnob (fxTab, "reverb_decay", "R DECAY", M);
        addKnob (fxTab, "master_drive", "DRIVE", M);
        addKnob (fxTab, "master_gain", "GAIN", G);
    }
    tabs.addTab ("SEQ", juce::Colour (0xff0a0a16), &seqTab, false);
    {
        seqTab.addAndMakeVisible (arpOn);
        seqTab.addAndMakeVisible (seqOn);
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "arp_on", arpOn));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "seq_on", seqOn));
        addKnob (seqTab, "arp_rate", "ARP RATE", C);
        addKnob (seqTab, "arp_octaves", "ARP OCT", M);
        addKnob (seqTab, "seq_rate", "SEQ RATE", O);
        stepGrid = std::make_unique<StepGridComponent> (processor.getStepSequencer());
        seqTab.addAndMakeVisible (*stepGrid);
    }
    tabs.addTab ("PRESETS", juce::Colour (0xff0a0a16), &presetTab, false);
    {
        presetList.setRowHeight (28);
        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00d4ff));
        presetLabel.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
        prevPreset.onClick = [this] {
            int i = processor.getCurrentProgram() - 1;
            if (i < 0) i = processor.getNumPrograms() - 1;
            processor.setCurrentProgram (i);
            presetLabel.setText (processor.getProgramName (i), juce::dontSendNotification);
            presetList.selectRow (i); presetList.repaint();
        };
        nextPreset.onClick = [this] {
            int i = (processor.getCurrentProgram() + 1) % processor.getNumPrograms();
            processor.setCurrentProgram (i);
            presetLabel.setText (processor.getProgramName (i), juce::dontSendNotification);
            presetList.selectRow (i); presetList.repaint();
        };
        initBtn.onClick = [this] {
            processor.setCurrentProgram (0);
            presetLabel.setText ("Init", juce::dontSendNotification);
            presetList.selectRow (0); presetList.repaint();
        };
        presetList.selectRow (processor.getCurrentProgram());
    }
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

SalekHightechAudioProcessorEditor::Knob& SalekHightechAudioProcessorEditor::addKnob (
    juce::Component& parent, const char* id, const char* label, juce::Colour c)
{
    auto k = std::make_unique<Knob>();
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    parent.addAndMakeVisible (k->s);
    atts.push_back (std::make_unique<SAtt> (processor.getAPVTS(), id, k->s));
    k->name.setText (label, juce::dontSendNotification);
    k->name.setJustificationType (juce::Justification::centred);
    k->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    k->name.setColour (juce::Label::textColourId, c.brighter (0.15f));
    parent.addAndMakeVisible (k->name);
    knobs.push_back (std::move (k));
    return *knobs.back();
}

void SalekHightechAudioProcessorEditor::addCombo (juce::Component& parent, juce::ComboBox& box,
                                                   const char* id, juce::StringArray items)
{
    box.addItemList (items, 1);
    parent.addAndMakeVisible (box);
    if (processor.getAPVTS().getParameter (id) != nullptr)
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), id, box));
}

int SalekHightechAudioProcessorEditor::getNumRows() { return processor.getNumPrograms(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected) g.fillAll (juce::Colour (0xff3a0066));
    g.setColour (selected ? juce::Colour (0xff00d4ff) : juce::Colour (0xffc8c8e0));
    g.setFont (juce::FontOptions (14.0f));
    g.drawText (processor.getProgramName (row), 12, 0, width - 20, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    processor.setCurrentProgram (row);
    presetLabel.setText (processor.getProgramName (row), juce::dontSendNotification);
}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff02020a));
    auto outer = getLocalBounds().toFloat().reduced (4.0f);
    juce::ColourGradient frame (juce::Colour (0xff0a1830), outer.getX(), outer.getY(),
                                juce::Colour (0xff120828), outer.getRight(), outer.getBottom(), false);
    g.setGradientFill (frame);
    g.fillRoundedRectangle (outer, 22.0f);
    g.setColour (juce::Colour (0xff00d4ff).withAlpha (0.45f));
    g.drawRoundedRectangle (outer, 22.0f, 2.0f);
    g.setColour (juce::Colour (0xff7b2cbf).withAlpha (0.25f));
    g.drawRoundedRectangle (outer.reduced (3.0f), 20.0f, 1.2f);
    auto inner = outer.reduced (10.0f);
    g.setColour (juce::Colour (0xff060614).withAlpha (0.92f));
    g.fillRoundedRectangle (inner, 16.0f);
    auto head = juce::Rectangle<float> (inner.getX() + 8, inner.getY() + 6, inner.getWidth() - 16, 50);
    juce::ColourGradient hg (juce::Colour (0xff0c1a32), head.getX(), head.getY(),
                             juce::Colour (0xff080818), head.getX(), head.getBottom(), false);
    g.setGradientFill (hg);
    g.fillRoundedRectangle (head, 12.0f);
    g.setColour (juce::Colour (0xff00d4ff).withAlpha (0.3f));
    g.drawRoundedRectangle (head, 12.0f, 1.0f);
    g.setColour (juce::Colour (0xff00e8ff));
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.reduced (14, 6).removeFromLeft (320), juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xffc77dff));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("NEBULA-CLASS  //  WT · FM · UNISON · VIRUS FILTER · CHORUS · REVERB",
                head.withTrimmedLeft (320).reduced (8, 12), juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 20) resized();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto a = getLocalBounds().reduced (14);
    keyboard.setBounds (a.removeFromBottom (70).reduced (2));
    a.removeFromBottom (6);
    auto header = a.removeFromTop (52);
    title.setVisible (false);
    tagline.setVisible (false);
    scope.setBounds (header.removeFromRight (140).reduced (4));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (220).reduced (4));
    a.removeFromTop (6);
    tabs.setBounds (a.reduced (4, 2));

    auto layoutTab = [] (juce::Component& tab)
    {
        auto bounds = tab.getLocalBounds().reduced (16);
        if (bounds.getWidth() < 50 || bounds.getHeight() < 50) return;
        juce::Array<juce::Component*> knobsArr, labelsArr, others;
        for (auto* c : tab.getChildren())
        {
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
            else others.add (c);
        }
        const int n = knobsArr.size();
        if (n == 0) { for (auto* o : others) o->setBounds (bounds.removeFromTop (36).reduced (4)); return; }
        const int cols = juce::jmin (8, juce::jmax (4, n));
        const int rows = (n + cols - 1) / cols;
        const int cellW = bounds.getWidth() / cols;
        const int cellH = juce::jmin (120, bounds.getHeight() / juce::jmax (1, rows));
        for (int i = 0; i < n; ++i)
        {
            int col = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (bounds.getX() + col * cellW, bounds.getY() + row * cellH, cellW, cellH).reduced (4);
            if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (16));
            knobsArr[i]->setBounds (cell);
        }
        auto bottom = bounds.withTrimmedTop (rows * cellH);
        for (auto* o : others) o->setBounds (bottom.removeFromTop (32).reduced (6));
    };
    for (auto* tab : { &oscTab, &filterTab, &modTab, &fxTab })
        layoutTab (*tab);
    {
        auto bounds = seqTab.getLocalBounds().reduced (16);
        juce::Array<juce::Component*> knobsArr, labelsArr, others;
        for (auto* c : seqTab.getChildren())
        {
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
            else if (c != stepGrid.get()) others.add (c);
        }
        const int n = knobsArr.size();
        auto top = bounds.removeFromTop (130);
        if (n > 0) {
            const int cellW = top.getWidth() / juce::jmax (1, n);
            for (int i = 0; i < n; ++i) {
                auto cell = top.withX (top.getX() + i * cellW).withWidth (cellW).reduced (4);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (16));
                knobsArr[i]->setBounds (cell);
            }
        }
        auto row = bounds.removeFromTop (36);
        for (auto* o : others) o->setBounds (row.removeFromLeft (120).reduced (4));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (4));
    }
    {
        auto b = presetTab.getLocalBounds().reduced (16);
        auto top = b.removeFromTop (40);
        prevPreset.setBounds (top.removeFromLeft (50).reduced (2));
        nextPreset.setBounds (top.removeFromLeft (50).reduced (2));
        initBtn.setBounds (top.removeFromLeft (80).reduced (2));
        presetLabel.setBounds (top.reduced (4));
        presetList.setBounds (b);
    }
}

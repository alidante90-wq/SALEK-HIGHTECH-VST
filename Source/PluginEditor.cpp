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
    setSize (1100, 720);
    setResizable (true, true);
    setResizeLimits (960, 640, 1600, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("WT", juce::dontSendNotification);
    addAndMakeVisible (tagline);
    addAndMakeVisible (scope);
    wtDisplay = std::make_unique<WavetableDisplay> (processor.getAPVTS());
    addAndMakeVisible (*wtDisplay);
    addAndMakeVisible (tabs);
    tabs.setOutline (0);

    auto C = juce::Colour (0xff4fc3f7);
    auto M = juce::Colour (0xff81d4fa);
    auto O = juce::Colour (0xffffab40);
    auto G = juce::Colour (0xffa5d6a7);
    auto V = juce::Colour (0xffce93d8);

    tabs.addTab ("OSC", juce::Colour (0xff121218), &oscTab, false);
    {
        addKnob (oscTab, "osc1_level", "LVL 1", C); addKnob (oscTab, "osc1_table", "TABLE", M);
        addKnob (oscTab, "osc1_warp", "WARP", O); addKnob (oscTab, "osc1_fold", "FOLD", M);
        addKnob (oscTab, "osc1_drive", "DRIVE", O); addKnob (oscTab, "osc1_octave", "OCT", V);
        addKnob (oscTab, "osc1_semi", "SEMI", V); addKnob (oscTab, "osc1_detune", "DET", G);
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
    tabs.addTab ("FILTER", juce::Colour (0xff121218), &filterTab, false);
    {
        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "DRIVE", O);
        addKnob (filterTab, "filter_env", "ENV AMT", G);
        addCombo (filterTab, filterMode, "filter_mode", {"LOW PASS","HIGH PASS","BAND PASS","NOTCH"});
    }
    tabs.addTab ("ENV", juce::Colour (0xff121218), &envTab, false);
    {
        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);
    }
    tabs.addTab ("MOD", juce::Colour (0xff121218), &modTab, false);
    {
        addKnob (modTab, "fm_2to1", "FM 2>1", O); addKnob (modTab, "fm_3to1", "FM 3>1", O);
        addKnob (modTab, "fm_3to2", "FM 3>2", O); addKnob (modTab, "pm_2to1", "PM 2>1", M);
        addKnob (modTab, "rm_2to1", "RM 2>1", M); addKnob (modTab, "am_2to1", "AM 2>1", V);
        addKnob (modTab, "lfo_rate", "LFO RATE", C); addKnob (modTab, "lfo_amount", "LFO AMT", M);
        addCombo (modTab, lfoWave, "lfo_wave", {"SINE","TRIANGLE","SAW","SQUARE","S&H"});
        addKnob (modTab, "macro1", "MACRO 1", C); addKnob (modTab, "macro2", "MACRO 2", M);
        addKnob (modTab, "macro3", "MACRO 3", O); addKnob (modTab, "macro4", "MACRO 4", G);
    }
    tabs.addTab ("FX", juce::Colour (0xff121218), &fxTab, false);
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
    tabs.addTab ("ARP", juce::Colour (0xff121218), &seqTab, false);
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
    tabs.addTab ("PRESET", juce::Colour (0xff121218), &presetTab, false);
    {
        presetList.setRowHeight (28);
        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff4fc3f7));
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
    k->name.setColour (juce::Label::textColourId, juce::Colour (0xffa0a0b0));
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
    if (selected) g.fillAll (juce::Colour (0xff1e3a50));
    g.setColour (selected ? juce::Colour (0xff4fc3f7) : juce::Colour (0xffc0c0d0));
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
    g.fillAll (juce::Colour (0xff08080c));
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient outerG (juce::Colour (0xff2a2a38), 0, 0,
                                 juce::Colour (0xff0a0a10), 0, bounds.getHeight(), false);
    g.setGradientFill (outerG);
    g.fillRoundedRectangle (bounds.reduced (3.0f), 14.0f);
    g.setColour (juce::Colour (0xff4fc3f7).withAlpha (0.35f));
    g.drawRoundedRectangle (bounds.reduced (3.0f), 14.0f, 1.5f);
    g.setColour (juce::Colour (0xff1a1a24));
    g.fillRoundedRectangle (bounds.reduced (8.0f), 12.0f);

    auto head = juce::Rectangle<float> (12.0f, 10.0f, bounds.getWidth() - 24.0f, 52.0f);
    juce::ColourGradient hg (juce::Colour (0xff222230), head.getX(), head.getY(),
                             juce::Colour (0xff101018), head.getX(), head.getBottom(), false);
    g.setGradientFill (hg);
    g.fillRoundedRectangle (head, 10.0f);
    g.setColour (juce::Colour (0xff3a3a48));
    g.drawRoundedRectangle (head, 10.0f, 1.0f);

    g.setColour (juce::Colour (0xff4fc3f7));
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.getX() + 14.0f, head.getY() + 8.0f, 300.0f, 28.0f,
                juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xff8890a8));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("WAVETABLE  ·  FM  ·  UNISON  ·  FX", head.getX() + 14.0f, head.getY() + 32.0f, 320.0f, 16.0f,
                juce::Justification::centredLeft);

    // Original anime-style cyber silhouette (NOT any existing character / IP)
    const float ax = head.getRight() - 200.0f;
    const float ay = head.getY() + 6.0f;
    juce::Path hair;
    hair.startNewSubPath (ax + 40, ay + 8);
    hair.cubicTo (ax + 20, ay + 5, ax + 10, ay + 20, ax + 18, ay + 38);
    hair.cubicTo (ax + 25, ay + 42, ax + 55, ay + 40, ax + 62, ay + 28);
    hair.cubicTo (ax + 70, ay + 12, ax + 55, ay + 4, ax + 40, ay + 8);
    g.setColour (juce::Colour (0xff7b2cbf).withAlpha (0.55f));
    g.fillPath (hair);
    g.setColour (juce::Colour (0xffc77dff).withAlpha (0.4f));
    g.strokePath (hair, juce::PathStrokeType (1.2f));
    g.setColour (juce::Colour (0xffe8d0f0).withAlpha (0.35f));
    g.fillEllipse (ax + 28, ay + 14, 22, 26);
    g.setColour (juce::Colour (0xff4fc3f7).withAlpha (0.7f));
    g.fillEllipse (ax + 32, ay + 22, 6, 4);
    g.fillEllipse (ax + 42, ay + 22, 6, 4);
    g.setColour (juce::Colour (0xff4fc3f7).withAlpha (0.25f));
    g.drawLine (ax + 70, ay + 18, ax + 110, ay + 18, 1.0f);
    g.drawLine (ax + 110, ay + 18, ax + 118, ay + 28, 1.0f);
    g.fillEllipse (ax + 116, ay + 26, 5, 5);
}

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 20) resized();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto a = getLocalBounds().reduced (12);
    keyboard.setBounds (a.removeFromBottom (68).reduced (2));
    a.removeFromBottom (4);
    title.setVisible (false);
    tagline.setVisible (false);
    auto header = a.removeFromTop (52);
    scope.setBounds (header.removeFromRight (150).reduced (4));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (240).reduced (4));
    a.removeFromTop (4);
    tabs.setBounds (a);

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
    for (auto* tab : { &oscTab, &filterTab, &envTab, &modTab, &fxTab })
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
        auto top = bounds.removeFromTop (120);
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

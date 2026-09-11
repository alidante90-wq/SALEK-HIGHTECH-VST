#include "PluginEditor.h"

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (24, 96);
    keyboard.setOctaveForMiddleC (4);
    addAndMakeVisible (keyboard);
    startTimerHz (10);
    setLookAndFeel (&lnf);
    setSize (1200, 780);
    setResizable (true, true);
    setResizeLimits (1000, 680, 1600, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("CYBER", juce::dontSendNotification);
    addAndMakeVisible (tagline);
    addAndMakeVisible (scope);
    wtDisplay = std::make_unique<WavetableDisplay> (processor.getAPVTS());
    addAndMakeVisible (*wtDisplay);
    adsrDisplay = std::make_unique<AdsrDisplay> (processor.getAPVTS());
    filterDisplay = std::make_unique<FilterCurveDisplay> (processor.getAPVTS());
    lfoDisplay = std::make_unique<LfoDisplay> (processor.getAPVTS());
    addAndMakeVisible (tabs);
    tabs.setOutline (0);

    auto C = juce::Colour (0xff00f0ff);
    auto M = juce::Colour (0xffff00aa);
    auto O = juce::Colour (0xffffab40);
    auto G = juce::Colour (0xff66ff99);
    auto V = juce::Colour (0xffce93d8);

    tabs.addTab ("OSC", juce::Colour (0xff0c0818), &oscTab, false);
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
    tabs.addTab ("FILTER", juce::Colour (0xff0c0818), &filterTab, false);
    {
        if (filterDisplay != nullptr) filterTab.addAndMakeVisible (*filterDisplay);
        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "DRIVE", O);
        addKnob (filterTab, "filter_env", "ENV AMT", G);
        addCombo (filterTab, filterMode, "filter_mode", {"LOW PASS","HIGH PASS","BAND PASS","NOTCH"});
    }
    tabs.addTab ("ENV", juce::Colour (0xff0c0818), &envTab, false);
    {
        if (adsrDisplay != nullptr) envTab.addAndMakeVisible (*adsrDisplay);
        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);
    }
    tabs.addTab ("MOD", juce::Colour (0xff0c0818), &modTab, false);
    {
        if (lfoDisplay != nullptr) modTab.addAndMakeVisible (*lfoDisplay);
        addKnob (modTab, "fm_2to1", "FM 2>1", O); addKnob (modTab, "fm_3to1", "FM 3>1", O);
        addKnob (modTab, "fm_3to2", "FM 3>2", O); addKnob (modTab, "pm_2to1", "PM 2>1", M);
        addKnob (modTab, "rm_2to1", "RM 2>1", M); addKnob (modTab, "am_2to1", "AM 2>1", V);
        addKnob (modTab, "lfo_rate", "LFO RATE", C); addKnob (modTab, "lfo_amount", "LFO AMT", M);
        addCombo (modTab, lfoWave, "lfo_wave", {"SINE","TRIANGLE","SAW","SQUARE","S&H"});
        addKnob (modTab, "macro1", "MACRO 1", C); addKnob (modTab, "macro2", "MACRO 2", M);
        addKnob (modTab, "macro3", "MACRO 3", O); addKnob (modTab, "macro4", "MACRO 4", G);
    }
    tabs.addTab ("FX", juce::Colour (0xff0c0818), &fxTab, false);
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
    tabs.addTab ("ARP", juce::Colour (0xff0c0818), &seqTab, false);
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
    tabs.addTab ("PRESET", juce::Colour (0xff0c0818), &presetTab, false);
    {
        presetList.setRowHeight (28);
        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00f0ff));
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
    k->name.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
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
    g.setColour (selected ? juce::Colour (0xff00f0ff) : juce::Colour (0xffd0c0e0));
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
    g.fillAll (juce::Colour (0xff06040e));
    auto bounds = getLocalBounds().toFloat();

    auto charPanel = juce::Rectangle<float> (8.0f, 8.0f, 150.0f, bounds.getHeight() - 16.0f);
    juce::ColourGradient cg (juce::Colour (0xff1a0a30), charPanel.getX(), charPanel.getY(),
                             juce::Colour (0xff0a0618), charPanel.getX(), charPanel.getBottom(), false);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (charPanel, 16.0f);
    g.setColour (juce::Colour (0xffff00aa).withAlpha (0.55f));
    g.drawRoundedRectangle (charPanel, 16.0f, 2.0f);

    const float cx = charPanel.getCentreX();
    const float top = charPanel.getY() + 40.0f;
    juce::Path hair;
    hair.startNewSubPath (cx, top);
    hair.cubicTo (cx - 55, top + 10, cx - 70, top + 80, cx - 45, top + 160);
    hair.cubicTo (cx - 20, top + 200, cx + 20, top + 200, cx + 45, top + 160);
    hair.cubicTo (cx + 70, top + 80, cx + 55, top + 10, cx, top);
    g.setColour (juce::Colour (0xff9b30ff).withAlpha (0.8f));
    g.fillPath (hair);
    g.setColour (juce::Colour (0xffff66cc).withAlpha (0.5f));
    g.strokePath (hair, juce::PathStrokeType (2.0f));

    g.setColour (juce::Colour (0xffffe0f0).withAlpha (0.6f));
    g.fillEllipse (cx - 28, top + 50, 56, 70);
    g.setColour (juce::Colour (0xff00f0ff));
    g.fillEllipse (cx - 16, top + 78, 12, 8);
    g.fillEllipse (cx + 4, top + 78, 12, 8);
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.fillEllipse (cx - 13, top + 80, 4, 3);
    g.fillEllipse (cx + 7, top + 80, 4, 3);

    g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.35f));
    for (int i = 0; i < 5; ++i)
    {
        float y = top + 150.0f + i * 28.0f;
        g.drawLine (cx - 40, y, cx + 40, y, 1.2f);
        g.fillEllipse (cx - 42, y - 3, 6, 6);
        g.fillEllipse (cx + 36, y - 3, 6, 6);
    }
    g.setColour (juce::Colour (0xffff00aa));
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText ("SALEK", charPanel.getX(), charPanel.getBottom() - 48, charPanel.getWidth(), 18, juce::Justification::centred);
    g.setColour (juce::Colour (0xff00f0ff));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("v0.4 CYBER", charPanel.getX(), charPanel.getBottom() - 30, charPanel.getWidth(), 16, juce::Justification::centred);

    auto main = bounds.withTrimmedLeft (162.0f).reduced (6.0f);
    juce::ColourGradient mg (juce::Colour (0xff14122a), main.getX(), main.getY(),
                             juce::Colour (0xff0a0814), main.getX(), main.getBottom(), false);
    g.setGradientFill (mg);
    g.fillRoundedRectangle (main, 14.0f);
    g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.4f));
    g.drawRoundedRectangle (main, 14.0f, 1.5f);

    auto head = main.removeFromTop (48.0f).reduced (8.0f, 6.0f);
    g.setColour (juce::Colour (0xff1c1830));
    g.fillRoundedRectangle (head, 10.0f);
    g.setColour (juce::Colour (0xff00f0ff));
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.getX() + 12, head.getY() + 6, 280, 24, juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xffff00aa));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("CYBER ANIME  ·  WT · FM · UNISON · FX", head.getX() + 12, head.getY() + 28, 360, 14, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 30) resized();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto a = getLocalBounds().reduced (12);
    a.removeFromLeft (150);
    keyboard.setBounds (a.removeFromBottom (68).reduced (2));
    a.removeFromBottom (4);
    title.setVisible (false);
    tagline.setVisible (false);
    auto header = a.removeFromTop (48);
    scope.setBounds (header.removeFromRight (130).reduced (4));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (260).reduced (2));
    a.removeFromTop (4);
    tabs.setBounds (a);

    auto layoutTab = [] (juce::Component& tab)
    {
        auto bounds = tab.getLocalBounds().reduced (14);
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
        const int cellH = juce::jmin (115, bounds.getHeight() / juce::jmax (1, rows));
        for (int i = 0; i < n; ++i)
        {
            int col = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (bounds.getX() + col * cellW, bounds.getY() + row * cellH, cellW, cellH).reduced (3);
            if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
            knobsArr[i]->setBounds (cell);
        }
        auto bottom = bounds.withTrimmedTop (rows * cellH);
        for (auto* o : others) o->setBounds (bottom.removeFromTop (30).reduced (4));
    };
    layoutTab (oscTab);
    layoutTab (fxTab);

    {
        auto bounds = filterTab.getLocalBounds().reduced (12);
        if (filterDisplay != nullptr)
            filterDisplay->setBounds (bounds.removeFromTop (130).reduced (4));
        juce::Array<juce::Component*> knobsArr, labelsArr, others;
        for (auto* c : filterTab.getChildren())
        {
            if (c == filterDisplay.get()) continue;
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
            else others.add (c);
        }
        const int n = knobsArr.size();
        if (n > 0)
        {
            const int cols = juce::jmin (5, n);
            const int cellW = bounds.getWidth() / cols;
            const int cellH = juce::jmin (105, bounds.getHeight());
            for (int i = 0; i < n; ++i)
            {
                auto cell = juce::Rectangle<int> (bounds.getX() + (i % cols) * cellW, bounds.getY(), cellW, cellH).reduced (3);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
            auto bottom = bounds.withTrimmedTop (cellH);
            for (auto* o : others) o->setBounds (bottom.removeFromTop (28).reduced (4));
        }
    }

    {
        auto bounds = envTab.getLocalBounds().reduced (12);
        if (adsrDisplay != nullptr)
            adsrDisplay->setBounds (bounds.removeFromTop (150).reduced (4));
        juce::Array<juce::Component*> knobsArr, labelsArr;
        for (auto* c : envTab.getChildren())
        {
            if (c == adsrDisplay.get()) continue;
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
        }
        const int n = knobsArr.size();
        if (n > 0)
        {
            const int cellW = bounds.getWidth() / n;
            for (int i = 0; i < n; ++i)
            {
                auto cell = bounds.withX (bounds.getX() + i * cellW).withWidth (cellW).reduced (6);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
        }
    }

    {
        auto bounds = modTab.getLocalBounds().reduced (12);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (bounds.removeFromTop (120).reduced (4));
        juce::Array<juce::Component*> knobsArr, labelsArr, others;
        for (auto* c : modTab.getChildren())
        {
            if (c == lfoDisplay.get()) continue;
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
            else others.add (c);
        }
        const int n = knobsArr.size();
        if (n > 0)
        {
            const int cols = juce::jmin (6, juce::jmax (4, n));
            const int rows = (n + cols - 1) / cols;
            const int cellW = bounds.getWidth() / cols;
            const int cellH = juce::jmin (100, bounds.getHeight() / juce::jmax (1, rows));
            for (int i = 0; i < n; ++i)
            {
                int col = i % cols, row = i / cols;
                auto cell = juce::Rectangle<int> (bounds.getX() + col * cellW, bounds.getY() + row * cellH, cellW, cellH).reduced (3);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
            auto bottom = bounds.withTrimmedTop (rows * cellH);
            for (auto* o : others) o->setBounds (bottom.removeFromTop (28).reduced (4));
        }
    }

    {
        auto bounds = seqTab.getLocalBounds().reduced (14);
        juce::Array<juce::Component*> knobsArr, labelsArr, others;
        for (auto* c : seqTab.getChildren())
        {
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
            else if (c != stepGrid.get()) others.add (c);
        }
        const int n = knobsArr.size();
        auto top = bounds.removeFromTop (110);
        if (n > 0) {
            const int cellW = top.getWidth() / juce::jmax (1, n);
            for (int i = 0; i < n; ++i) {
                auto cell = top.withX (top.getX() + i * cellW).withWidth (cellW).reduced (4);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
        }
        auto row = bounds.removeFromTop (36);
        for (auto* o : others) o->setBounds (row.removeFromLeft (120).reduced (4));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (4));
    }
    {
        auto b = presetTab.getLocalBounds().reduced (14);
        auto top = b.removeFromTop (40);
        prevPreset.setBounds (top.removeFromLeft (50).reduced (2));
        nextPreset.setBounds (top.removeFromLeft (50).reduced (2));
        initBtn.setBounds (top.removeFromLeft (80).reduced (2));
        presetLabel.setBounds (top.reduced (4));
        presetList.setBounds (b);
    }
}

#include "PluginEditor.h"
#include "AssetsData.h"
#include <cmath>

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (21, 108);
    keyboard.setOctaveForMiddleC (4);
    glBackdrop = std::make_unique<OpenGLGridBackdrop>();
    glBackdrop->setVisible (false);
    addAndMakeVisible (keyboard);
    startTimerHz (30);
    setLookAndFeel (&lnf);
    logoImg = SalekAssets::loadLogo();
    heroImg = SalekAssets::loadHero();
    setSize (1280, 820);
    setResizable (true, true);
    setResizeLimits (1020, 700, 1700, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("ALIEN", juce::dontSendNotification);
    addAndMakeVisible (tagline);
    addAndMakeVisible (scope);
    wtDisplay = std::make_unique<WavetableDisplay> (processor.getAPVTS());
    addAndMakeVisible (*wtDisplay);
    adsrDisplay = std::make_unique<AdsrDisplay> (processor.getAPVTS());
    filterDisplay = std::make_unique<FilterCurveDisplay> (processor.getAPVTS());
    lfoDisplay = std::make_unique<LfoDisplay> (processor.getAPVTS());
    matrixPanel = std::make_unique<ModMatrixPanel> (processor.getModMatrix());
    addAndMakeVisible (tabs);
    tabs.setOutline (0);
    tabs.setOpaque (true);

    auto C = juce::Colour (0xff00f0ff);
    auto M = juce::Colour (0xffff00aa);
    auto O = juce::Colour (0xffffab40);
    auto G = juce::Colour (0xff66ff99);
    auto V = juce::Colour (0xffce93d8);

    tabs.addTab ("MAIN", juce::Colour (0xff0c0818), &mainTab, false);
    {
        mainTab.addAndMakeVisible (oscTab);
        mainTab.addAndMakeVisible (filterTab);
        mainTab.addAndMakeVisible (envTab);
        mainTab.addAndMakeVisible (presetTab);
        if (filterDisplay != nullptr) filterTab.addAndMakeVisible (*filterDisplay);
        if (adsrDisplay != nullptr) envTab.addAndMakeVisible (*adsrDisplay);

        addKnob (oscTab, "osc1_level", "LVL 1", C); addKnob (oscTab, "osc1_table", "TABLE", M);
        addKnob (oscTab, "osc1_warp", "WARP", O); addKnob (oscTab, "osc1_fold", "FOLD", M);
        addKnob (oscTab, "osc1_drive", "DRIVE", O); addKnob (oscTab, "osc1_octave", "OCT", V);
        addKnob (oscTab, "osc2_level", "LVL 2", C); addKnob (oscTab, "osc2_table", "TABLE2", M);
        addKnob (oscTab, "osc2_warp", "WARP2", O); addKnob (oscTab, "osc2_fold", "FOLD2", M);
        addKnob (oscTab, "osc2_drive", "DRIVE2", O); addKnob (oscTab, "osc2_octave", "OCT2", V);
        addKnob (oscTab, "osc3_level", "LVL 3", C); addKnob (oscTab, "osc3_table", "TABLE3", M);
        addKnob (oscTab, "osc3_warp", "WARP3", O); addKnob (oscTab, "osc3_fold", "FOLD3", M);
        addKnob (oscTab, "unison_voices", "UNISON", C);
        addKnob (oscTab, "unison_detune", "U DET", M);
        addKnob (oscTab, "unison_spread", "SPREAD", O);

        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "F DRIVE", O);
        addKnob (filterTab, "filter_env", "F ENV", G);
        addCombo (filterTab, filterMode, "filter_mode", {"LOW PASS","HIGH PASS","BAND PASS","NOTCH"});

        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);

        presetList.setRowHeight (26);
        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00f0ff));
        presetLabel.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
        prevPreset.onClick = [this] {
            int i = processor.getCurrentProgram() - 1;
            if (i < 0) i = processor.getNumPrograms() - 1;
            processor.setCurrentProgram (i);
            presetLabel.setText (processor.getProgramName (i), juce::dontSendNotification);
            rebuildPresetRows(); presetList.updateContent(); presetList.repaint();
        };
        nextPreset.onClick = [this] {
            int i = (processor.getCurrentProgram() + 1) % processor.getNumPrograms();
            processor.setCurrentProgram (i);
            presetLabel.setText (processor.getProgramName (i), juce::dontSendNotification);
            rebuildPresetRows(); presetList.updateContent(); presetList.repaint();
        };
        initBtn.onClick = [this] {
            processor.setCurrentProgram (0);
            presetLabel.setText ("Init", juce::dontSendNotification);
            rebuildPresetRows(); presetList.updateContent(); presetList.repaint();
        };
        rebuildPresetRows();
        presetList.updateContent();
    }

    tabs.addTab ("MOD", juce::Colour (0xff0c0818), &modTab, false);
    {
        if (lfoDisplay != nullptr) modTab.addAndMakeVisible (*lfoDisplay);
        if (matrixPanel != nullptr) modTab.addAndMakeVisible (*matrixPanel);
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
        addKnob (fxTab, "comp_threshold", "C THR", O);
        addKnob (fxTab, "comp_ratio", "C RATIO", M);
        addKnob (fxTab, "comp_mix", "C MIX", C);
        addKnob (fxTab, "eq_low", "EQ LOW", G);
        addKnob (fxTab, "eq_mid", "EQ MID", O);
        addKnob (fxTab, "eq_high", "EQ HI", M);
        addKnob (fxTab, "spatial_azim", "AZIM", C);
        addKnob (fxTab, "spatial_dist", "DIST", O);
        addKnob (fxTab, "spatial_size", "SIZE", M);
        addKnob (fxTab, "spatial_elev", "ELEV", G);
        addKnob (fxTab, "input_mix", "IN MIX", V);
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

    themeBox.addItem ("Cyber Magenta", 1);
    themeBox.addItem ("Alien Green", 2);
    themeBox.addItem ("Ice Blue", 3);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (themeBox);
    themeBox.onChange = [this] { repaint(); };
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

void SalekHightechAudioProcessorEditor::rebuildPresetRows()
{
    presetRows.clear();
    juce::String lastCat;
    const int n = processor.getNumPrograms();
    for (int i = 0; i < n; ++i)
    {
        auto name = processor.getProgramName (i);
        juce::String cat = name.containsChar ('/') ? name.upToFirstOccurrenceOf ("/", false, false) : "Other";
        if (cat != lastCat)
        {
            PresetRow h; h.isHeader = true; h.label = cat.toUpperCase(); h.programIndex = -1;
            presetRows.add (h);
            lastCat = cat;
        }
        PresetRow r; r.isHeader = false;
        r.label = name.fromFirstOccurrenceOf ("/", false, false);
        if (r.label.isEmpty()) r.label = name;
        r.programIndex = i;
        presetRows.add (r);
    }
}

int SalekHightechAudioProcessorEditor::getNumRows() { return presetRows.size(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);
    if (pr.isHeader)
    {
        g.fillAll (juce::Colour (0xff12081c));
        g.setColour (juce::Colour (0xff00f0ff));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("  " + pr.label, 6, 0, width - 10, height, juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffff2d6a).withAlpha (0.5f));
        g.drawLine (4.0f, (float) height - 1.0f, (float) width - 4.0f, (float) height - 1.0f, 1.0f);
        return;
    }
    if (selected) g.fillAll (juce::Colour (0xff3a0066));
    else g.fillAll (juce::Colour (0xff0a0614));
    juce::Colour tc = selected ? juce::Colour (0xff00f0ff) : juce::Colour (0xffd0c0e0);
    auto full = processor.getProgramName (pr.programIndex);
    if (full.startsWith ("Acid")) tc = selected ? tc : juce::Colour (0xffffcc44);
    if (full.startsWith ("Bass")) tc = selected ? tc : juce::Colour (0xff66ff99);
    if (full.startsWith ("Lead")) tc = selected ? tc : juce::Colour (0xff00f0ff);
    if (full.startsWith ("Kick")) tc = selected ? tc : juce::Colour (0xffffab40);
    g.setColour (tc);
    g.setFont (juce::FontOptions (12.5f));
    g.drawText ("   " + pr.label, 8, 0, width - 12, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);
    if (pr.isHeader || pr.programIndex < 0) return;
    processor.setCurrentProgram (pr.programIndex);
    presetLabel.setText (processor.getProgramName (pr.programIndex), juce::dontSendNotification);
}

#include "PluginEditorPaint.inl"

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 40) resized();
    animPhase += 0.04f;
    if (glBackdrop != nullptr)
        glBackdrop->setPulse (processor.getOutputPeak());
    repaint();
}

void SalekHightechAudioProcessorEditor::resized()
{
    if (glBackdrop != nullptr)
        glBackdrop->setBounds (0, 0, 0, 0);

    auto full = getLocalBounds().reduced (6);
    keyboard.setBounds (full.removeFromBottom (72).reduced (2, 2));
    full.removeFromBottom (2);

    auto a = full;
    a.removeFromLeft (176);
    title.setVisible (false);
    tagline.setVisible (false);

    auto header = a.removeFromTop (44);
    themeBox.setBounds (header.removeFromRight (150).reduced (2));
    scope.setBounds (header.removeFromRight (130).reduced (3));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (260).reduced (2));

    a.removeFromTop (4);
    tabs.setBounds (a);

    {
        auto b = mainTab.getLocalBounds().reduced (8);
        presetTab.setBounds (b.removeFromLeft (220));
        envTab.setBounds (b.removeFromBottom (150));
        filterTab.setBounds (b.removeFromRight (280));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (6);
            auto top = pb.removeFromTop (36);
            prevPreset.setBounds (top.removeFromLeft (36).reduced (2));
            nextPreset.setBounds (top.removeFromLeft (36).reduced (2));
            initBtn.setBounds (top.removeFromLeft (56).reduced (2));
            presetLabel.setBounds (top.reduced (2));
            presetList.setBounds (pb);
        }

        auto layoutKnobs = [] (juce::Component& panel, juce::Component* skip, int topReserve)
        {
            auto bounds = panel.getLocalBounds().reduced (8);
            if (skip != nullptr)
                skip->setBounds (bounds.removeFromTop (topReserve).reduced (2));
            juce::Array<juce::Component*> knobsArr, labelsArr, others;
            for (auto* c : panel.getChildren())
            {
                if (c == skip) continue;
                if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
                else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
                else others.add (c);
            }
            const int n = knobsArr.size();
            if (n == 0) return;
            const int cols = juce::jmin (6, juce::jmax (3, n));
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
            for (auto* o : others) o->setBounds (bottom.removeFromTop (28).reduced (3));
        };
        layoutKnobs (oscTab, nullptr, 0);
        layoutKnobs (filterTab, filterDisplay.get(), 110);
        layoutKnobs (envTab, adsrDisplay.get(), 110);
    }

    {
        auto bounds = modTab.getLocalBounds().reduced (12);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (bounds.removeFromTop (100).reduced (4));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (bounds.removeFromTop (200).reduced (4));
        juce::Array<juce::Component*> knobsArr, labelsArr;
        for (auto* c : modTab.getChildren())
        {
            if (c == lfoDisplay.get() || c == matrixPanel.get()) continue;
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
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
                auto cell = juce::Rectangle<int> (bounds.getX() + (i % cols) * cellW, bounds.getY() + (i / cols) * cellH, cellW, cellH).reduced (3);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
        }
    }

    {
        auto bounds = fxTab.getLocalBounds().reduced (16);
        juce::Array<juce::Component*> knobsArr, labelsArr;
        for (auto* c : fxTab.getChildren())
        {
            if (dynamic_cast<juce::Slider*> (c)) knobsArr.add (c);
            else if (dynamic_cast<juce::Label*> (c)) labelsArr.add (c);
        }
        const int n = knobsArr.size();
        if (n > 0)
        {
            const int cols = juce::jmin (6, n);
            const int cellW = bounds.getWidth() / cols;
            const int cellH = juce::jmin (100, bounds.getHeight() / juce::jmax (1, (n + cols - 1) / cols));
            for (int i = 0; i < n; ++i)
            {
                auto cell = juce::Rectangle<int> (bounds.getX() + (i % cols) * cellW, bounds.getY() + (i / cols) * cellH, cellW, cellH).reduced (4);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
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
        if (n > 0)
        {
            const int cellW = top.getWidth() / juce::jmax (1, n);
            for (int i = 0; i < n; ++i)
            {
                auto cell = top.withX (top.getX() + i * cellW).withWidth (cellW).reduced (4);
                if (i < labelsArr.size()) labelsArr[i]->setBounds (cell.removeFromBottom (14));
                knobsArr[i]->setBounds (cell);
            }
        }
        auto row = bounds.removeFromTop (36);
        for (auto* o : others) o->setBounds (row.removeFromLeft (120).reduced (4));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (4));
    }
}

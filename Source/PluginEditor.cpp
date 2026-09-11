#include "PluginEditor.h"
#include "AssetsData.h"
#include <cmath>

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (24, 96);
    keyboard.setOctaveForMiddleC (4);
    addAndMakeVisible (keyboard);
    startTimerHz (30);
    setLookAndFeel (&lnf);
    logoImg = SalekAssets::loadLogo();
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

int SalekHightechAudioProcessorEditor::getNumRows() { return processor.getNumPrograms(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected) g.fillAll (juce::Colour (0xff3a0066));
    g.setColour (selected ? juce::Colour (0xff00f0ff) : juce::Colour (0xffd0c0e0));
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (processor.getProgramName (row), 8, 0, width - 12, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    processor.setCurrentProgram (row);
    presetLabel.setText (processor.getProgramName (row), juce::dontSendNotification);
}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int theme = themeBox.getSelectedId();
    juce::Colour bg1 (0xff04020a), bg2 (0xff10081a), accent (0xff00f0ff), accent2 (0xffff2d6a);
    if (theme == 2) { bg1 = juce::Colour (0xff020806); bg2 = juce::Colour (0xff081810); accent = juce::Colour (0xff66ff99); accent2 = juce::Colour (0xffc0ff00); }
    if (theme == 3) { bg1 = juce::Colour (0xff020810); bg2 = juce::Colour (0xff0a1420); accent = juce::Colour (0xff4fc3f7); accent2 = juce::Colour (0xff90caf9); }

    const float peak = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 2.8f);
    float cut = 0.5f;
    if (auto* p = processor.getAPVTS().getRawParameterValue ("filter_cutoff"))
        cut = juce::jmap (std::log (juce::jmax (20.0f, p->load())), std::log (20.0f), std::log (20000.0f), 0.0f, 1.0f);

    juce::ColourGradient bg (bg1, 0, 0, bg2, 0, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (accent.withAlpha (0.06f + pulse * 0.14f));
    const float y0 = (float) getHeight() * 0.58f;
    for (int i = 0; i < 14; ++i)
    {
        float t = (float) i / 13.0f;
        g.drawLine (0.0f, y0 + t * t * ((float) getHeight() - y0), (float) getWidth(), y0 + t * t * ((float) getHeight() - y0), 1.0f);
    }
    for (int i = 0; i < 30; ++i)
    {
        float seed = (float) i * 19.7f;
        float x = std::fmod (seed * 41.0f + animPhase * (10.0f + peak * 50.0f), (float) getWidth());
        float y = std::fmod (seed * 27.0f + animPhase * 6.0f + cut * 40.0f, (float) getHeight());
        g.setColour (accent2.withAlpha (0.2f + pulse * 0.5f));
        g.fillEllipse (x, y, 1.5f + pulse * 2.5f, 1.5f + pulse * 2.5f);
    }

    auto charPanel = juce::Rectangle<float> (8.0f, 52.0f, 168.0f, (float) getHeight() - 130.0f);
    {
        juce::ColourGradient cg (juce::Colour (0xff1a0a28), charPanel.getX(), charPanel.getY(),
                                 juce::Colour (0xff080410), charPanel.getX(), charPanel.getBottom(), false);
        g.setGradientFill (cg);
        g.fillRoundedRectangle (charPanel, 12.0f);
        g.setColour (accent2.withAlpha (0.45f + pulse * 0.4f));
        g.drawRoundedRectangle (charPanel, 12.0f, 1.5f + pulse);

        float cx = charPanel.getCentreX();
        float cy = charPanel.getCentreY() - 20.0f + (0.5f - cut) * 30.0f;
        for (int r = 3; r >= 0; --r)
        {
            float rad = 28.0f + r * 14.0f + pulse * 8.0f;
            g.setColour (accent.withAlpha (0.08f + pulse * 0.1f - r * 0.015f));
            g.drawEllipse (cx - rad, cy - rad, rad * 2, rad * 2, 1.5f);
        }
        g.setColour (juce::Colour (0xff2a1538));
        g.fillEllipse (cx - 36, cy - 42, 72, 84);
        g.setColour (accent.withAlpha (0.7f + pulse * 0.3f));
        g.drawEllipse (cx - 36, cy - 42, 72, 84, 1.5f);
        float eyeY = cy - 8.0f;
        float eyeOff = (cut - 0.5f) * 6.0f;
        g.setColour (accent);
        g.fillEllipse (cx - 16 + eyeOff, eyeY, 10, 7);
        g.fillEllipse (cx + 6 + eyeOff, eyeY, 10, 7);

        auto gval = [&](const char* id) -> float {
            if (auto* p = processor.getAPVTS().getRawParameterValue (id)) return p->load();
            return 0.f;
        };
        float ly = charPanel.getBottom() - 70.0f;
        auto drawLed = [&](float x, const char* label, bool on, juce::Colour c) {
            g.setColour (on ? c : juce::Colour (0xff222230));
            g.fillEllipse (x, ly, 10.0f, 10.0f);
            if (on) { g.setColour (c.withAlpha (0.35f)); g.fillEllipse (x - 3, ly - 3, 16, 16); }
            g.setColour (juce::Colour (0xffa0a0b8));
            g.setFont (juce::FontOptions (9.0f));
            g.drawText (label, x + 14, ly - 2, 50, 14, juce::Justification::centredLeft);
        };
        drawLed (charPanel.getX() + 12, "ARP", gval ("arp_on") > 0.5f, accent);
        ly += 16;
        drawLed (charPanel.getX() + 12, "SEQ", gval ("seq_on") > 0.5f, accent2);

        g.setColour (juce::Colour (0xff1a1020));
        g.fillRoundedRectangle (charPanel.getX() + 12, charPanel.getBottom() - 28, charPanel.getWidth() - 24, 10, 3.0f);
        g.setColour (accent.interpolatedWith (accent2, pulse));
        g.fillRoundedRectangle (charPanel.getX() + 12, charPanel.getBottom() - 28, (charPanel.getWidth() - 24) * pulse, 10, 3.0f);

        if (logoImg.isValid())
        {
            auto lr = juce::Rectangle<float> (charPanel.getX() + 40, charPanel.getY() + 8, 88, 88);
            g.setOpacity (0.85f + pulse * 0.15f);
            g.drawImage (logoImg, lr, juce::RectanglePlacement::centred);
            g.setOpacity (1.0f);
        }
    }

    auto outer = getLocalBounds().toFloat().reduced (3.0f);
    g.setColour (accent.withAlpha (0.2f + pulse * 0.35f));
    g.drawRoundedRectangle (outer, 14.0f, 2.0f + pulse);

    auto head = juce::Rectangle<float> (8.0f, 6.0f, (float) getWidth() - 16.0f, 42.0f);
    g.setColour (bg2.brighter (0.08f));
    g.fillRoundedRectangle (head, 10.0f);
    g.setColour (accent);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.getX() + 180.0f, head.getY() + 4.0f, 300.0f, 22.0f, juce::Justification::centredLeft);
    g.setColour (accent2);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("MOD MATRIX  ·  OPENGL READY  ·  v0.8", head.getX() + 180.0f, head.getY() + 24.0f, 360.0f, 14.0f, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 40) resized();
    animPhase += 0.04f;
    repaint();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto a = getLocalBounds().reduced (8);
    a.removeFromLeft (176);
    keyboard.setBounds (a.removeFromBottom (64).reduced (2));
    a.removeFromBottom (4);
    title.setVisible (false);
    tagline.setVisible (false);

    auto header = a.removeFromTop (44);
    themeBox.setBounds (header.removeFromLeft (140).reduced (2));
    scope.setBounds (header.removeFromRight (120).reduced (3));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (240).reduced (2));

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
        layoutKnobs (envTab, adsrDisplay.get(), 90);
    }

    {
        auto bounds = modTab.getLocalBounds().reduced (12);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (bounds.removeFromTop (90).reduced (4));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (bounds.removeFromTop (160).reduced (4));
        juce::Array<juce::Component*> knobsArr, labelsArr, others;
        for (auto* c : modTab.getChildren())
        {
            if (c == lfoDisplay.get() || c == matrixPanel.get()) continue;
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
            const int cellH = juce::jmin (90, bounds.getHeight() / juce::jmax (1, rows));
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
            const int cellH = juce::jmin (120, bounds.getHeight());
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

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    setSize (1180, 720);
    setResizable (true, true);
    setResizeLimits (980, 620, 1600, 1000);

    logoImg = salek::AssetsData::loadLogo();
    heroImg = salek::AssetsData::loadToronowla();
    faceImg = salek::AssetsData::loadFace();
    lianImg = salek::AssetsData::loadLian();
    cyanImg = salek::AssetsData::loadCyan();

    keyboard.setAvailableRange (36, 96);
    keyboard.setOctaveForMiddleC (4);
    addAndMakeVisible (keyboard);

    adsrDisplay = std::make_unique<AdsrDisplay> (processor.getAPVTS());
    filterDisplay = std::make_unique<FilterCurveDisplay> (processor.getAPVTS());
    lfoDisplay = std::make_unique<LfoDisplay> (processor.getAPVTS());
    matrixPanel = std::make_unique<ModMatrixPanel> (processor.getModMatrix());

    tabs.addTab ("MAIN", juce::Colours::transparentBlack, &mainTab, false);
    tabs.addTab ("MOD", juce::Colours::transparentBlack, &modTab, false);
    tabs.addTab ("FX", juce::Colours::transparentBlack, &fxTab, false);
    tabs.addTab ("SEQ", juce::Colours::transparentBlack, &seqTab, false);
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (28);
    tabs.setOpaque (false);
    tabs.setColour (juce::TabbedComponent::backgroundColourId, juce::Colour (0xff0a0614));
    tabs.setColour (juce::TabbedComponent::outlineColourId, juce::Colour (0xff1a1030));
    for (auto* p : { &mainTab, &modTab, &fxTab, &seqTab, &oscTab, &filterTab, &envTab, &presetTab })
        p->setOpaque (false);

    {
        mainTab.addAndMakeVisible (oscTab);
        mainTab.addAndMakeVisible (filterTab);
        mainTab.addAndMakeVisible (envTab);
        mainTab.addAndMakeVisible (presetTab);
        if (filterDisplay != nullptr) filterTab.addAndMakeVisible (*filterDisplay);
        if (adsrDisplay != nullptr) envTab.addAndMakeVisible (*adsrDisplay);

        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);

        addKnob (oscTab, "osc1_level", "LVL 1", C);
        addKnob (oscTab, "osc1_table", "TABLE", M);
        addKnob (oscTab, "osc1_warp", "WARP", O);
        addKnob (oscTab, "osc1_fold", "FOLD", M);
        addKnob (oscTab, "osc1_drive", "DRIVE", O);
        addKnob (oscTab, "osc1_octave", "OCT", V);
        addKnob (oscTab, "osc2_level", "LVL 2", C);
        addKnob (oscTab, "osc2_table", "TABLE2", M);
        addKnob (oscTab, "osc2_warp", "WARP2", O);
        addKnob (oscTab, "osc2_fold", "FOLD2", M);
        addKnob (oscTab, "osc2_drive", "DRIVE2", O);
        addKnob (oscTab, "osc2_octave", "OCT2", V);
        addKnob (oscTab, "osc3_level", "LVL 3", C);
        addKnob (oscTab, "osc3_table", "TABLE3", M);
        addKnob (oscTab, "osc3_warp", "WARP3", O);
        addKnob (oscTab, "osc3_fold", "FOLD3", M);
        addKnob (oscTab, "unison_voices", "UNISON", C);
        addKnob (oscTab, "unison_detune", "U DET", M);
        addKnob (oscTab, "unison_spread", "SPREAD", O);

        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "F DRIVE", O);
        addKnob (filterTab, "filter_env", "F ENV", G);

        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);

        presetList.setModel (this);
        presetList.setRowHeight (22);
        presetList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff0a0614));
        presetList.setColour (juce::ListBox::outlineColourId, juce::Colour (0xff1a1030));
        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        prevPreset.onClick = [this] {
            int i = processor.getCurrentProgram();
            if (i > 0) processor.setCurrentProgram (i - 1);
            presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
            presetList.repaint();
        };
        nextPreset.onClick = [this] {
            int i = processor.getCurrentProgram();
            if (i + 1 < processor.getNumPrograms()) processor.setCurrentProgram (i + 1);
            presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
            presetList.repaint();
        };
        initBtn.onClick = [this] {
            processor.setCurrentProgram (0);
            presetLabel.setText (processor.getProgramName (0), juce::dontSendNotification);
            presetList.repaint();
        };
        rebuildPresetRows();
        presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
    }

    {
        if (lfoDisplay != nullptr) modTab.addAndMakeVisible (*lfoDisplay);
        if (matrixPanel != nullptr) modTab.addAndMakeVisible (*matrixPanel);
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        addKnob (modTab, "fm_2to1", "FM 2>1", C);
        addKnob (modTab, "fm_3to1", "FM 3>1", M);
        addKnob (modTab, "fm_3to2", "FM 3>2", O);
        addKnob (modTab, "pm_2to1", "PM 2>1", G);
        addKnob (modTab, "rm_2to1", "RM 2>1", C);
        addKnob (modTab, "am_2to1", "AM 2>1", M);
        addKnob (modTab, "lfo_rate", "LFO RATE", O);
        addKnob (modTab, "lfo_amount", "LFO AMT", V);
        addKnob (modTab, "macro1", "MACRO 1", C);
        addKnob (modTab, "macro2", "MACRO 2", M);
        addKnob (modTab, "macro3", "MACRO 3", O);
        addKnob (modTab, "macro4", "MACRO 4", G);
    }

    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        addKnob (fxTab, "delay_mix", "DELAY", C);
        addKnob (fxTab, "delay_time", "D TIME", M);
        addKnob (fxTab, "delay_fb", "D FB", O);
        addKnob (fxTab, "chorus_mix", "CHORUS", G);
        addKnob (fxTab, "chorus_rate", "C RATE", C);
        addKnob (fxTab, "chorus_depth", "C DEPTH", M);
        addKnob (fxTab, "reverb_mix", "REVERB", O);
        addKnob (fxTab, "reverb_size", "R SIZE", V);
        addKnob (fxTab, "reverb_damp", "R DAMP", G);
        addKnob (fxTab, "comp_thresh", "C THR", C);
        addKnob (fxTab, "comp_ratio", "C RAT", M);
        addKnob (fxTab, "comp_attack", "C ATK", O);
        addKnob (fxTab, "comp_release", "C REL", G);
        addKnob (fxTab, "eq_low", "EQ LOW", C);
        addKnob (fxTab, "eq_mid", "EQ MID", M);
        addKnob (fxTab, "eq_high", "EQ HI", O);
        addKnob (fxTab, "spatial_mix", "SPATIAL", G);
        addKnob (fxTab, "spatial_width", "WIDTH", C);
        addKnob (fxTab, "master_drive", "DRIVE", M);
        addKnob (fxTab, "master_vol", "VOLUME", O);
        addKnob (fxTab, "master_pan", "PAN", V);
        addKnob (fxTab, "master_limit", "LIMIT", G);
    }

    {
        seqTab.addAndMakeVisible (arpOn);
        seqTab.addAndMakeVisible (seqOn);
        arpOn.setButtonText ("ARP");
        seqOn.setButtonText ("SEQ");
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "arp_on", arpOn));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "seq_on", seqOn));
        stepGrid = std::make_unique<StepGridComponent> (processor.getStepSequencer());
        seqTab.addAndMakeVisible (*stepGrid);
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        addKnob (seqTab, "arp_rate", "ARP RATE", C);
        addKnob (seqTab, "arp_octaves", "ARP OCT", M);
        addKnob (seqTab, "seq_rate", "SEQ RATE", O);
    }

    themeBox.addItem ("CYBER", 1);
    themeBox.addItem ("ACID", 2);
    themeBox.addItem ("NEON", 3);
    themeBox.setSelectedId (1);
    addAndMakeVisible (themeBox);

    if (wtDisplay != nullptr) addAndMakeVisible (*wtDisplay);
    addAndMakeVisible (scope);

    presetFilterBox.addItem ("ALL", 1);
    presetFilterBox.setSelectedId (1);
    presetFilterBox.onChange = [this] { rebuildPresetRows(); };

    startTimerHz (30);
    animPhase = 0.045f;

    if (auto* disp = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
        vblank = juce::VBlankAttachment (this, [this] { animPhase += 0.045f; repaint(); });
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
    vblank = {};
}

#include "PluginEditorAddKnob.inl"

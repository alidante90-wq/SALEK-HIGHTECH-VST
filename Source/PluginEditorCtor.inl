    setLookAndFeel (&lnf);
    logoImg   = SalekAssets::loadLogo();
    heroImg   = SalekAssets::loadToronowla();
    faceImg   = SalekAssets::loadFace();
    lianImg   = SalekAssets::loadLian();
    cyanImg   = SalekAssets::loadCyanGirl();
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
    tabs.addTab ("MAIN", juce::Colours::transparentBlack, &mainTab, false);
    tabs.addTab ("MOD", juce::Colours::transparentBlack, &modTab, false);
    tabs.addTab ("FX", juce::Colours::transparentBlack, &fxTab, false);
    tabs.addTab ("SEQ", juce::Colours::transparentBlack, &seqTab, false);
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (28);
    tabs.setOpaque (true);

    filterDisplay = std::make_unique<FilterCurveDisplay> (processor.getAPVTS());
    adsrDisplay = std::make_unique<AdsrDisplay> (processor.getAPVTS());
    lfoDisplay = std::make_unique<LfoDisplay> (processor.getAPVTS());
    matrixPanel = std::make_unique<salek::ModMatrixPanel>(processor.getModMatrix());
    stepGrid = std::make_unique<salek::StepGridComponent>(processor.getStepSequencer());

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
        addKnob (filterTab, "filter_drive", "F DRV", O);
        addKnob (filterTab, "filter_env", "F ENV", G);

        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);

        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetList.setModel (this);
        prevPreset.setButtonText ("<");
        nextPreset.setButtonText (">");
        initBtn.setButtonText ("INIT");
        prevPreset.onClick = [this] { int i = processor.getCurrentProgram(); if (i > 0) processor.setCurrentProgram (i - 1); presetList.selectRow (processor.getCurrentProgram()); };
        nextPreset.onClick = [this] { int i = processor.getCurrentProgram(); if (i + 1 < processor.getNumPrograms()) processor.setCurrentProgram (i + 1); presetList.selectRow (processor.getCurrentProgram()); };
        initBtn.onClick = [this] { processor.setCurrentProgram (0); presetList.selectRow (0); };
        presetLabel.setText ("PRESETS", juce::dontSendNotification);
    }

    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        if (lfoDisplay != nullptr) modTab.addAndMakeVisible (*lfoDisplay);
        if (matrixPanel != nullptr) modTab.addAndMakeVisible (*matrixPanel);
        addKnob (modTab, "fm_2to1", "FM 2>1", O); addKnob (modTab, "fm_3to1", "FM 3>1", O);
        addKnob (modTab, "fm_3to2", "FM 3>2", O); addKnob (modTab, "pm_2to1", "PM 2>1", M);
        addKnob (modTab, "rm_2to1", "RM 2>1", M); addKnob (modTab, "am_2to1", "AM 2>1", V);
        addKnob (modTab, "lfo_rate", "LFO RATE", C); addKnob (modTab, "lfo_amount", "LFO AMT", M);
        addKnob (modTab, "macro1", "MACRO 1", C); addKnob (modTab, "macro2", "MACRO 2", M);
        addKnob (modTab, "macro3", "MACRO 3", O); addKnob (modTab, "macro4", "MACRO 4", G);
    }

    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
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

    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        seqTab.addAndMakeVisible (arpOn);
        seqTab.addAndMakeVisible (seqOn);
        arpOn.setButtonText ("ARP");
        seqOn.setButtonText ("SEQ");
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "arp_on", arpOn));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "seq_on", seqOn));
        if (stepGrid != nullptr) seqTab.addAndMakeVisible (*stepGrid);
        addKnob (seqTab, "arp_rate", "ARP RATE", C);
        addKnob (seqTab, "arp_octaves", "ARP OCT", M);
        addKnob (seqTab, "seq_rate", "SEQ RATE", O);
    }

    themeBox.addItem ("CYBER", 1);
    themeBox.addItem ("ACID", 2);
    themeBox.addItem ("NEON", 3);
    themeBox.setSelectedId (1);
    addAndMakeVisible (themeBox);

    if (sonicCore != nullptr)
        sonicCore->toBack();

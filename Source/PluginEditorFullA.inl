SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (21, 108);
    keyboard.setOctaveForMiddleC (4);
    glBackdrop = nullptr;
    sonicCore = nullptr;

    addAndMakeVisible (keyboard);
    vblank = juce::VBlankAttachment (this, [this] (double) {
        animPhase += 0.085f;
        repaint();
    });
    startTimerHz (30);

    setLookAndFeel (&lnf);
    logoImg   = SalekAssets::loadLogo();
    faceImg   = SalekAssets::loadFace();
    lianImg   = SalekAssets::loadLian();
    cyanImg   = SalekAssets::loadCyanGirl();
    heroImg   = SalekAssets::loadLian();
    setSize (1280, 820);
    setResizable (true, true);
    setResizeLimits (1020, 700, 1700, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("ALIEN", juce::dontSendNotification);
    addAndMakeVisible (tagline);
    scope.setFifo (&processor.getVisualFifo());
    spectrum.setFifo (&processor.getVisualFifo());
    addAndMakeVisible (scope);
    addAndMakeVisible (spectrum);
    wtDisplay = std::make_unique<WavetableDisplay> (processor.getAPVTS());
    addAndMakeVisible (*wtDisplay);
    adsrDisplay = std::make_unique<AdsrDisplay> (processor.getAPVTS());
    filterDisplay = std::make_unique<FilterCurveDisplay> (processor.getAPVTS());
    lfoDisplay = std::make_unique<LfoDisplay> (processor.getAPVTS());
    matrixPanel = std::make_unique<ModMatrixPanel> (processor.getModMatrix());

    tabs.addTab ("MAIN", juce::Colours::transparentBlack, &mainTab, false);
    tabs.addTab ("MOD", juce::Colours::transparentBlack, &modTab, false);
    tabs.addTab ("LFO", juce::Colours::transparentBlack, &lfoTab, false);
    tabs.addTab ("FX", juce::Colours::transparentBlack, &fxTab, false);
    tabs.addTab ("MAGIC", juce::Colours::transparentBlack, &magicTab, false);
    tabs.addTab ("SEQ", juce::Colours::transparentBlack, &seqTab, false);
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (28);
    tabs.setOpaque (true);
    tabs.setColour (juce::TabbedComponent::backgroundColourId, juce::Colour (0xff0a0614));
    tabs.setColour (juce::TabbedComponent::outlineColourId, juce::Colour (0xff1a1030));
    // Opaque content prevents black voids after long sessions in some hosts
    for (auto* panel : { &mainTab, &modTab, &lfoTab, &fxTab, &magicTab, &seqTab, &oscTab, &filterTab, &envTab, &presetTab })
    {
        panel->setOpaque (true);
        panel->setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff0a0614));
    }

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

        addKnob (oscTab, "osc1_level", "LVL", C); addKnob (oscTab, "osc1_table", "TABLE", M);
        addKnob (oscTab, "osc1_warp", "WARP", O); addKnob (oscTab, "osc1_fold", "FOLD", M);
        addKnob (oscTab, "osc1_drive", "DRIVE", O); addKnob (oscTab, "osc1_octave", "OCT", V);
        addKnob (oscTab, "osc2_level", "LVL", C); addKnob (oscTab, "osc2_table", "TABLE", M);
        addKnob (oscTab, "osc2_warp", "WARP", O); addKnob (oscTab, "osc2_fold", "FOLD", M);
        addKnob (oscTab, "osc2_drive", "DRIVE", O); addKnob (oscTab, "osc2_octave", "OCT", V);
        addKnob (oscTab, "osc3_level", "LVL", C); addKnob (oscTab, "osc3_table", "TABLE", M);
        addKnob (oscTab, "osc3_warp", "WARP", O); addKnob (oscTab, "osc3_fold", "FOLD", M);
        addKnob (oscTab, "osc3_drive", "DRIVE", O); addKnob (oscTab, "osc3_octave", "OCT", V);
        addKnob (oscTab, "unison_voices", "UNISON", C);
        addKnob (oscTab, "unison_detune", "U DET", M);
        addKnob (oscTab, "unison_spread", "SPREAD", O);

        #include "PluginEditorOscMonitors.inl"

        addKnob (filterTab, "filter_cutoff", "CUTOFF", C);
        addKnob (filterTab, "filter_reso", "RESO", M);
        addKnob (filterTab, "filter_drive", "F DRIVE", O);
        addKnob (filterTab, "filter_env", "F ENV", G);
        // 16-mode filter bank
        addCombo (filterTab, filterMode, "filter_mode", {
            "LP12","LP24","HP12","HP24","BP","Notch","Peak","AllPass",
            "AcidLP","Ladder","Comb","Formant","BandRej","LoShelf","HiShelf","PhaserN",
            "DiodeLP","Sallen","Vocal","RingMod","BP-Q","LP+HP","Notch2","Wide"
        });
        filterMode.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        filterMode.setColour (juce::ComboBox::textColourId, juce::Colour (0xff00e8ff));

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
        presetTab.addAndMakeVisible (savePresetBtn);
        presetTab.addAndMakeVisible (loadPresetBtn);
        presetTab.addAndMakeVisible (bankBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetLabel.setFont (juce::FontOptions (11.0f));
        presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00e8ff));
        savePresetBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a1050));
        savePresetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffd700));
        loadPresetBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0a3050));
        loadPresetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
        bankBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff201040));
        bankBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffaa00));
        prevPreset.onClick = [this] {
            int i = processor.getCurrentProgram();
            if (i > 0) processor.setCurrentProgram (i - 1);
            presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
            presetList.updateContent();
            presetList.repaint();
        };
        nextPreset.onClick = [this] {
            int i = processor.getCurrentProgram();
            if (i + 1 < processor.getNumPrograms()) processor.setCurrentProgram (i + 1);
            presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
            presetList.updateContent();
            presetList.repaint();
        };
        initBtn.onClick = [this] {
            processor.setCurrentProgram (0);
            presetLabel.setText (processor.getProgramName (0), juce::dontSendNotification);
            presetList.updateContent();
            presetList.repaint();
        };
        #include "PluginEditorPresetIO.inl"
        rebuildPresetRows();
        presetLabel.setText (processor.getProgramName (processor.getCurrentProgram()), juce::dontSendNotification);
    }

    {
        if (matrixPanel != nullptr) modTab.addAndMakeVisible (*matrixPanel);
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        #include "PluginEditorModKnobs.inl"
    }

    {
        #include "PluginEditorLfoSetup.inl"
    }

    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        #include "PluginEditorFxSections.inl"
    }

    {
        #include "PluginEditorMagic.inl"
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
    themeBox.addItem ("PATINA", 4);
    themeBox.setSelectedId (4); // default soft pink/mint like reference
    themeBox.onChange = [this] { applyHeroFromTheme(); repaint(); };
    addAndMakeVisible (themeBox);

    // Collapse PRESET list only (hero stays visible)
    presetToggle.setButtonText ("<<");
    presetToggle.setClickingTogglesState (true);
    presetToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
    presetToggle.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff2a1040));
    presetToggle.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
    presetToggle.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffd700));
    presetToggle.setTooltip ("Collapse / expand preset list");
    presetToggle.onClick = [this]
    {
        presetCollapsed = presetToggle.getToggleState();
        presetToggle.setButtonText (presetCollapsed ? ">>" : "<<");
        resized();
        repaint();
    };
    addAndMakeVisible (presetToggle);

    // Bilingual UI: EN (default) <-> FA
    langToggle.setButtonText ("EN");
    langToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
    langToggle.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffd700));
    langToggle.setTooltip ("UI language EN / FA");
    langToggle.onClick = [this]
    {
        uiLangFa = ! uiLangFa;
        langToggle.setButtonText (uiLangFa ? "FA" : "EN");
        applyUiLanguage();
    };
    addAndMakeVisible (langToggle);

    applyHeroFromTheme();
    applyUiLanguage();
    rebuildPresetRows();
    // Start with categories folded — click [+] to expand (Serum/Vital style browser)
    {
        juce::StringArray cats;
        const int n = processor.getNumPrograms();
        for (int i = 0; i < n; ++i)
        {
            auto name = processor.getProgramName (i);
            auto cat = name.upToFirstOccurrenceOf ("/", false, false);
            if (cat == name) cat = "OTHER";
            if (! cats.contains (cat)) cats.add (cat);
        }
        collapsedCats = cats;
        rebuildPresetRows();
    }
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    glBackdrop = nullptr;
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
        auto cat = name.upToFirstOccurrenceOf ("/", false, false);
        if (cat == name) cat = "OTHER";
        if (cat != lastCat)
        {
            PresetRow h; h.isHeader = true; h.label = cat.toUpperCase(); h.programIndex = -1; h.category = cat;
            presetRows.add (h);
            lastCat = cat;
        }
        // Skip presets in collapsed categories
        if (collapsedCats.contains (cat))
            continue;
        PresetRow r; r.isHeader = false;
        r.label = name.fromFirstOccurrenceOf ("/", false, false);
        if (r.label.isEmpty()) r.label = name;
        r.programIndex = i;
        r.category = cat;
        presetRows.add (r);
    }
    presetList.updateContent();
    presetList.repaint();
}

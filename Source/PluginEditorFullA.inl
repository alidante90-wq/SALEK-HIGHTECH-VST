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

    tabs.addTab (juce::CharPointer_UTF8 ("\xd8\xa7\xd8\xb5\xd9\x84\xdb\x8c / MAIN"), juce::Colours::transparentBlack, &mainTab, false);
    tabs.addTab (juce::CharPointer_UTF8 ("\xd9\x85\xd8\xaf\xd9\x88\xd9\x84 / MOD"), juce::Colours::transparentBlack, &modTab, false);
    tabs.addTab (juce::CharPointer_UTF8 ("\xd8\xa7\xd9\x84\xd8\xa7\xd9\x81\xd8\xa7\xd9\x88 / LFO"), juce::Colours::transparentBlack, &lfoTab, false);
    tabs.addTab (juce::CharPointer_UTF8 ("\xd8\xa7\xd9\x81\xda\xa9\xd8\xaa / FX"), juce::Colours::transparentBlack, &fxTab, false);
    tabs.addTab (juce::CharPointer_UTF8 ("\xd9\x85\xd8\xac\xdb\x8c\xda\xa9 / MAGIC"), juce::Colours::transparentBlack, &magicTab, false);
    tabs.addTab (juce::CharPointer_UTF8 ("\xd8\xb3\xda\xa9\xd9\x88\xd9\x86\xd8\xb3\xd8\xb1 / SEQ"), juce::Colours::transparentBlack, &seqTab, false);
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (28);
    tabs.setOpaque (false);
    tabs.setColour (juce::TabbedComponent::backgroundColourId, juce::Colour (0xff0a0614));
    tabs.setColour (juce::TabbedComponent::outlineColourId, juce::Colour (0xff1a1030));
    for (auto* panel : { &mainTab, &modTab, &lfoTab, &fxTab, &magicTab, &seqTab, &oscTab, &filterTab, &envTab, &presetTab })
        panel->setOpaque (false);

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
        arpOn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xa2\xd8\xb1\xd9\xbe / ARP"));
        seqOn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xb3\xda\xa9\xd9\x88\xd8\xa7\xd9\x86\xd8\xb3 / SEQ"));
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
    themeBox.onChange = [this] { applyHeroFromTheme(); repaint(); };
    addAndMakeVisible (themeBox);

    // Collapsible left hero panel — frees horizontal space when closed
    sideToggle.setButtonText ("<<");
    sideToggle.setClickingTogglesState (true);
    sideToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
    sideToggle.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff2a1040));
    sideToggle.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
    sideToggle.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffd700));
    sideToggle.onClick = [this]
    {
        sideCollapsed = sideToggle.getToggleState();
        sideToggle.setButtonText (sideCollapsed ? ">>" : "<<");
        resized();
        repaint();
    };
    addAndMakeVisible (sideToggle);

    applyHeroFromTheme();

    rebuildPresetRows();
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
    presetList.updateContent();
    presetList.repaint();
}

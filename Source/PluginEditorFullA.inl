SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (21, 108);
    keyboard.setOctaveForMiddleC (4);
    keyboard.setWantsKeyboardFocus (true);
    // Keep computer-keyboard → MIDI working even while mouse is on knobs
    setWantsKeyboardFocus (true);
    glBackdrop = nullptr;
    sonicCore = nullptr;

    addAndMakeVisible (keyboard);
    // VBlank disabled — was causing 5-10 FPS lag with heavy BG paint
    // Animation driven by timer only at 12 Hz
    startTimerHz (4); // multi-instance friendly

    setLookAndFeel (&lnf);
    logoImg   = SalekAssets::loadLogoGiti();
    if (! logoImg.isValid()) logoImg = SalekAssets::loadLogo();
    logoAltImg = SalekAssets::loadLogoAlt();
    // face.png removed (was ~2MB) — BGs only
    lianImg   = SalekAssets::loadBgIsatis();
    cyanImg   = SalekAssets::loadBgSalek();
    heroImg   = lianImg;
    refreshCharCache(); // load char PNGs ONCE
    
    // Global LFO mod sources (Serum/Vital style) — visible all tabs
    auto styleModSrc = [this] (juce::TextButton& b, juce::Colour c, int src)
    {
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
        b.setColour (juce::TextButton::buttonOnColourId, c.withAlpha (0.55f));
        b.setColour (juce::TextButton::textColourOffId, c);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        b.setClickingTogglesState (true);
        b.setTooltip ("Vital-style: arm " + b.getButtonText() + " → click knob | Shift=100% | Alt=invert | Ctrl=25% | Right-click=clear");
        b.onClick = [this, src, &b]
        {
            const bool on = b.getToggleState();
            armedModSource = on ? src : -1;
            modSrcLfo1.setToggleState (src == 0 && on, juce::dontSendNotification);
            modSrcLfo2.setToggleState (src == 1 && on, juce::dontSendNotification);
            modSrcLfo3.setToggleState (src == 2 && on, juce::dontSendNotification);
            if (! on) armedModSource = -1;
        };
        addAndMakeVisible (b);
    };
    styleModSrc (modSrcLfo1, juce::Colour (0xff00e8ff), 0);
    styleModSrc (modSrcLfo2, juce::Colour (0xffff2d9b), 1);
    styleModSrc (modSrcLfo3, juce::Colour (0xff39ff14), 2);
    // Real drag-and-drop from LFO pills onto knobs
    struct LfoDragHook : public juce::MouseListener
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        int src = 0;
        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (ed == nullptr || e.getDistanceFromDragStart() < 6) return;
            ed->armedModSource = src;
            juce::String desc = "SALEK_LFO" + juce::String (src);
            if (auto* c = dynamic_cast<juce::DragAndDropContainer*> (ed))
                c->startDragging (desc, e.eventComponent);
        }
    };
    for (int i = 0; i < 3; ++i)
    {
        auto* hook = new LfoDragHook();
        hook->ed = this;
        hook->src = i;
        juce::TextButton* btns[3] = { &modSrcLfo1, &modSrcLfo2, &modSrcLfo3 };
        btns[i]->addMouseListener (hook, false);
        // leak-free store
        modHookListeners.push_back (std::unique_ptr<juce::MouseListener> (hook));
    }

    masterGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    masterGainSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterGainSlider.setTooltip ("Master Gain");
    addAndMakeVisible (masterGainSlider);
    masterGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), "master_gain", masterGainSlider);

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
    // Glass UI: transparent so animated theme shows through (no solid pink slab)
    tabs.setOpaque (false);
    tabs.setColour (juce::TabbedComponent::backgroundColourId, juce::Colours::transparentBlack);
    tabs.setColour (juce::TabbedComponent::outlineColourId, juce::Colour (0x55ffffff));
    for (auto* panel : { &mainTab, &modTab, &lfoTab, &magicTab, &seqTab, &oscTab, &filterTab, &envTab, &presetTab })
    {
        panel->setOpaque (false);
        panel->setColour (juce::ResizableWindow::backgroundColourId, juce::Colours::transparentBlack);
    }
    // FX: solid dark underlay so knobs readable (user: black under FX)
    fxTab.setOpaque (true);
    fxTab.setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff08060f));

    {
        mainTab.addAndMakeVisible (oscTab);
        mainTab.addAndMakeVisible (filterTab);
        mainTab.addAndMakeVisible (envTab);
        // presetTab is global left column — parent is editor (not mainTab)
        addAndMakeVisible (presetTab);
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
        // Per-osc unison row (each column gets its own UNI / DET / SPR)
        addKnob (oscTab, "osc1_unison", "UNI", C);
        addKnob (oscTab, "osc1_udet", "DET", M);
        addKnob (oscTab, "osc1_uspread", "SPR", O);
        addKnob (oscTab, "osc2_unison", "UNI", C);
        addKnob (oscTab, "osc2_udet", "DET", M);
        addKnob (oscTab, "osc2_uspread", "SPR", O);
        addKnob (oscTab, "osc3_unison", "UNI", C);
        addKnob (oscTab, "osc3_udet", "DET", M);
        addKnob (oscTab, "osc3_uspread", "SPR", O);
        // Phase + random phase (prevents locked static note phase)
        addKnob (oscTab, "osc1_phase", "PHASE", C); addKnob (oscTab, "osc1_rand", "RAND", M); addKnob (oscTab, "osc1_pan", "PAN", O);
        addKnob (oscTab, "osc2_phase", "PHASE", C); addKnob (oscTab, "osc2_rand", "RAND", M); addKnob (oscTab, "osc2_pan", "PAN", O);
        addKnob (oscTab, "osc3_phase", "PHASE", C); addKnob (oscTab, "osc3_rand", "RAND", M); addKnob (oscTab, "osc3_pan", "PAN", O);

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
        addCombo (filterTab, filterRouteBox, "filter_route", {
            "All","OSC1","OSC2","OSC3","1+2","1+3","2+3"
        });
        filterRouteBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        filterRouteBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffff66cc));
        addCombo (envTab, voiceModeBox, "voice_mode", { "Poly", "Mono", "Legato" });
        voiceModeBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        voiceModeBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xff00e8ff));

        addKnob (envTab, "amp_attack", "ATTACK", C);
        addKnob (envTab, "amp_decay", "DECAY", M);
        addKnob (envTab, "amp_sustain", "SUSTAIN", O);
        addKnob (envTab, "amp_release", "RELEASE", G);

        presetList.setModel (this);
        presetList.setRowHeight (24);
        presetList.setOpaque (false);
        presetTab.setOpaque (false);
        presetList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0x66080414));
        presetList.setColour (juce::ListBox::outlineColourId, juce::Colour (0xff00e8ff).withAlpha (0.35f));
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
        const auto G = juce::Colour (0xff7c4dff);
        addKnob (seqTab, "arp_rate", "ARP RATE", C);
        addKnob (seqTab, "arp_octaves", "ARP OCT", M);
        addKnob (seqTab, "seq_rate", "SEQ RATE", O);
        // Extra voice colour — registered last so indices of FX/MOD stay stable
        addKnob (envTab, "glide", "GLIDE", C);
        addKnob (envTab, "poly_voices", "VOICES", M);
        addKnob (envTab, "noise_level", "NOISE", O);
        addKnob (envTab, "sub_level", "SUB", G);
    }

    themeBox.addItem ("CYBER", 1);
    themeBox.addItem ("ACID", 2);
    themeBox.addItem ("NEON", 3);
    themeBox.addItem ("PATINA", 4);
    themeBox.setSelectedId (1); // default CYBER
    themeBox.onChange = [this] { applyHeroFromTheme(); repaint(); };
    addAndMakeVisible (themeBox);
    bgSwapBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
    bgSwapBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffffd700));
    bgSwapBtn.setTooltip ("Swap background (ISATIS man+cat / SALEK 2 girls)");
    bgSwapBtn.onClick = [this]
    {
        // toggle between CYBER (1) and NEON (3) → different BGs
        themeBox.setSelectedId (themeBox.getSelectedId() == 1 ? 3 : 1, juce::sendNotification);
    };
    addAndMakeVisible (bgSwapBtn);

    charCycleBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
    charCycleBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffff2d9b));
    charCycleBtn.setButtonText ("MODEL");
    charCycleBtn.setTooltip ("Change character model (left panel) — does NOT change background");
    charCycleBtn.onClick = [this]
    {
        charPortraitIdx = (charPortraitIdx + 1) % 10; // 3 models only
        refreshCharCache();
        repaint();
    };
    addAndMakeVisible (charCycleBtn);

    // Collapse PRESET list only (hero stays visible)
    presetToggle.setButtonText (juce::CharPointer_UTF8 ("â")); // ◀
    presetToggle.setClickingTogglesState (true);
    presetToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
    presetToggle.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff3a1060));
    presetToggle.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
    presetToggle.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffd700));
    presetToggle.setTooltip ("Collapse / expand presets");
    presetToggle.onClick = [this]
    {
        presetCollapsed = presetToggle.getToggleState();
        presetToggle.setButtonText (presetCollapsed
            ? juce::CharPointer_UTF8 ("â¶")   // ▶
            : juce::CharPointer_UTF8 ("â")); // ◀
        resized();
        repaint();
    };
    presetTab.addAndMakeVisible (presetToggle); // must be child of presetTab for correct coords

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
    k->paramId = id;
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
    k->s.setNumDecimalPlacesToDisplay (1);   // short numbers (fixes "هزار رقمی")
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    k->s.setScrollWheelEnabled (true);
    k->s.setMouseDragSensitivity (180);
    // Cleaner value text: max 2 decimals, integers without .00
    k->s.textFromValueFunction = [] (double v) {
        if (std::abs (v) >= 1000.0) return juce::String ((int) std::round (v));
        if (std::abs (v - std::round (v)) < 1e-4) return juce::String ((int) std::round (v));
        if (std::abs (v) >= 100.0) return juce::String (v, 0);
        if (std::abs (v) >= 10.0) return juce::String (v, 1);
        return juce::String (v, 2);
    };
    parent.addAndMakeVisible (k->s);
    atts.push_back (std::make_unique<SAtt> (processor.getAPVTS(), id, k->s));
    k->name.setText (label, juce::dontSendNotification);
    k->name.setJustificationType (juce::Justification::centred);
    k->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    k->name.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
    parent.addAndMakeVisible (k->name);

    // Vital-style: arm LFO1/2/3 then click knob name to assign
    struct ModHook : public juce::MouseListener
    {
        SalekHightechAudioProcessorEditor* ed = nullptr;
        juce::String pid;
        void mouseDown (const juce::MouseEvent& e) override
        {
            if (ed == nullptr) return;
            if (ed->armedModSource < 0) return;
            float amt = 0.5f;
            if (e.mods.isShiftDown()) amt = 1.0f;
            if (e.mods.isAltDown())   amt = -0.5f;
            if (e.mods.isCommandDown() || e.mods.isCtrlDown()) amt = 0.25f;
            if (e.mods.isRightButtonDown()) amt = 0.f;
            ed->assignModToParam (pid, amt);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            // Drop: if an LFO was armed via drag, assign on release over this knob
            if (ed == nullptr || ed->armedModSource < 0) return;
            // Only if this release is not a simple click already handled
            if (e.mouseWasDraggedSinceMouseDown())
                ed->assignModToParam (pid, 0.5f);
        }
    };
    auto hook = std::make_unique<ModHook>();
    hook->ed = this;
    hook->pid = id;
    k->name.addMouseListener (hook.get(), false);
    k->s.addMouseListener (hook.get(), false);
    modHookListeners.push_back (std::move (hook));

    knobs.push_back (std::move (k));
    return *knobs.back();
}

void SalekHightechAudioProcessorEditor::assignModToParam (const juce::String& paramId, float amount)
{
    if (armedModSource < 0 || armedModSource > 2) return;

    using D = salek::ModMatrix::Dest;
    using S = salek::ModMatrix::Source;
    D dest = D::NumDests;
    if      (paramId == "filter_cutoff") dest = D::FilterCutoff;
    else if (paramId == "filter_reso")   dest = D::FilterReso;
    else if (paramId == "osc1_level")    dest = D::Osc1Level;
    else if (paramId == "osc2_level")    dest = D::Osc2Level;
    else if (paramId == "osc3_level")    dest = D::Osc3Level;
    else if (paramId == "osc1_table")    dest = D::Osc1Table;
    else if (paramId == "osc2_table")    dest = D::Osc2Table;
    else if (paramId == "osc3_table")    dest = D::Osc3Table;
    else if (paramId == "osc1_warp")     dest = D::Osc1Warp;
    else if (paramId == "osc2_warp")     dest = D::Osc2Warp;
    else if (paramId == "osc3_warp")     dest = D::Osc3Warp;
    else if (paramId == "osc1_fold")     dest = D::Osc1Fold;
    else if (paramId == "osc2_fold")     dest = D::Osc2Fold;
    else if (paramId == "osc3_fold")     dest = D::Osc3Fold;
    else if (paramId == "osc1_pan")      dest = D::Osc1Pan;
    else if (paramId == "osc2_pan")      dest = D::Osc2Pan;
    else if (paramId == "osc3_pan")      dest = D::Osc3Pan;
    else if (paramId == "osc1_drive")    dest = D::Osc1Drive;
    else if (paramId == "osc2_drive")    dest = D::Osc2Drive;
    else if (paramId == "osc3_drive")    dest = D::Osc3Drive;
    else if (paramId == "fm_2to1")       dest = D::Fm2to1;
    else if (paramId == "fm_3to1")       dest = D::Fm3to1;
    else if (paramId == "delay_mix")     dest = D::DelayMix;
    else if (paramId == "reverb_mix")    dest = D::ReverbMix;
    else if (paramId == "dist_drive")    dest = D::DistDrive;
    else if (paramId == "chorus_mix")    dest = D::ChorusMix;
    else return;

    S src = (armedModSource == 0) ? S::LFO1 : (armedModSource == 1) ? S::LFO2 : S::LFO3;
    if (std::abs (amount) < 1e-4f)
        processor.getModMatrix().removeRoute (src, dest);
    else
        processor.getModMatrix().addRoute (src, dest, juce::jlimit (-1.f, 1.f, amount));

    // Visual flash on source
    if (armedModSource == 0) modSrcLfo1.setToggleState (false, juce::dontSendNotification);
    if (armedModSource == 1) modSrcLfo2.setToggleState (false, juce::dontSendNotification);
    if (armedModSource == 2) modSrcLfo3.setToggleState (false, juce::dontSendNotification);
    armedModSource = -1;

    if (matrixPanel != nullptr)
        matrixPanel->repaint();
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
    // Group by unique category (no duplicate headers)
    presetRows.clear();
    juce::StringArray cats;
    const int n = processor.getNumPrograms();
    for (int i = 0; i < n; ++i)
    {
        auto name = processor.getProgramName (i);
        auto cat = name.upToFirstOccurrenceOf ("/", false, false);
        if (cat == name || cat.isEmpty()) cat = "OTHER";
        if (! cats.contains (cat, false))
            cats.add (cat);
    }
    // Prefer order matching mockup
    const juce::StringArray preferred { "OTHER","KICK","BASS","ACID","LEAD","FM","PAD","RETRO","FX","SQUEEK","KEYS","PLUCK","ARP","SALEK","AMBIENT","USER" };
    juce::StringArray ordered;
    for (auto& p : preferred)
        if (cats.contains (p, false)) ordered.add (p);
    for (auto& c : cats)
        if (! ordered.contains (c, false)) ordered.add (c);

    for (auto& cat : ordered)
    {
        PresetRow h; h.isHeader = true; h.label = cat.toUpperCase(); h.programIndex = -1; h.category = cat;
        presetRows.add (h);
        if (collapsedCats.contains (cat))
            continue;
        for (int i = 0; i < n; ++i)
        {
            auto name = processor.getProgramName (i);
            auto c = name.upToFirstOccurrenceOf ("/", false, false);
            if (c == name || c.isEmpty()) c = "OTHER";
            if (c != cat) continue;
            PresetRow r; r.isHeader = false;
            r.label = name.fromFirstOccurrenceOf ("/", false, false);
            if (r.label.isEmpty()) r.label = name;
            r.programIndex = i;
            r.category = cat;
            presetRows.add (r);
        }
    }
    presetList.updateContent();
    presetList.repaint();
}

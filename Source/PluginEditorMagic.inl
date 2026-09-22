// MAGIC tab: pad + mode buttons each with its OWN HOLD latch
{
    magicPad = std::make_unique<MagicPad>();
    magicTab.addAndMakeVisible (*magicPad);
    magicPad->setModeColour (juce::Colour (0xff00e8ff));
    magicPad->setModeName ("LOOP");

    auto styleBtn = [] (juce::TextButton& b, juce::Colour c)
    {
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
        b.setColour (juce::TextButton::buttonOnColourId, c.darker (0.15f));
        b.setColour (juce::TextButton::textColourOffId, c);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        b.setClickingTogglesState (true);
    };
    styleBtn (magicLoopBtn,   juce::Colour (0xff00e8ff));
    styleBtn (magicGlitchBtn, juce::Colour (0xffff2d9b));
    styleBtn (magicFlangeBtn, juce::Colour (0xff39ff14));
    styleBtn (magicPsychBtn,  juce::Colour (0xffffd700));
    styleBtn (magicHold,      juce::Colour (0xffffaa00)); // kept as master indicator

    magicLoopBtn.setButtonText ("LOOP");
    magicGlitchBtn.setButtonText ("GLITCH");
    magicFlangeBtn.setButtonText ("FLANGE");
    magicPsychBtn.setButtonText ("PSY");
    magicHold.setButtonText ("HOLD");

    // Per-mode HOLD buttons (member-like via static editor-scoped arrays)
    static juce::TextButton holdBtn[4];
    static bool holdOn[4] = { false, false, false, false };
    static int activeMode = 0;
    const juce::Colour holdCols[4] = {
        juce::Colour (0xff00e8ff), juce::Colour (0xffff2d9b),
        juce::Colour (0xff39ff14), juce::Colour (0xffffd700)
    };
    const char* holdLabels[4] = { "H-LOOP", "H-GLITCH", "H-FLANGE", "H-PSY" };
    for (int i = 0; i < 4; ++i)
    {
        styleBtn (holdBtn[i], holdCols[i]);
        holdBtn[i].setButtonText (holdLabels[i]);
        holdBtn[i].setToggleState (false, juce::dontSendNotification);
        magicTab.addAndMakeVisible (holdBtn[i]);
    }

    magicLoopBtn.setToggleState (true, juce::dontSendNotification);
    magicTab.addAndMakeVisible (magicLoopBtn);
    magicTab.addAndMakeVisible (magicGlitchBtn);
    magicTab.addAndMakeVisible (magicFlangeBtn);
    magicTab.addAndMakeVisible (magicPsychBtn);
    magicTab.addAndMakeVisible (magicHold);

    auto applyActive = [this] ()
    {
        // Any mode HOLD keeps magic alive (multi-hold)
        const bool anyHold = holdOn[0] || holdOn[1] || holdOn[2] || holdOn[3];
        magicHold.setToggleState (anyHold, juce::dontSendNotification);
        if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
            pa->setValueNotifyingHost (anyHold ? 1.f : 0.f);
        processor.getMagic().setActive (anyHold);
        processor.getMagic().setMode (activeMode);
    };

    magicPad->onChange = [this] (float mx, float my, bool act)
    {
        if (auto* px = processor.getAPVTS().getParameter ("magic_x"))
            px->setValueNotifyingHost (px->convertTo0to1 (mx));
        if (auto* py = processor.getAPVTS().getParameter ("magic_y"))
            py->setValueNotifyingHost (py->convertTo0to1 (my));
        const bool anyHold = holdOn[0] || holdOn[1] || holdOn[2] || holdOn[3];
        const bool on = anyHold ? true : act;
        if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
            pa->setValueNotifyingHost (on ? 1.f : 0.f);
        processor.getMagic().setXY (mx, my);
        processor.getMagic().setActive (on);
    };

    for (int i = 0; i < 4; ++i)
    {
        holdBtn[i].onClick = [this, i, applyActive]
        {
            holdOn[i] = holdBtn[i].getToggleState();
            if (i == activeMode)
                applyActive();
        };
    }

    // Master HOLD mirrors current mode's hold
    magicHold.onClick = [this, applyActive]
    {
        holdOn[activeMode] = magicHold.getToggleState();
        holdBtn[activeMode].setToggleState (holdOn[activeMode], juce::dontSendNotification);
        applyActive();
    };

    auto setMode = [this, applyActive] (int m, juce::Colour col, const char* name)
    {
        // Keep previous holds; do NOT clear them when switching modes
        activeMode = m;
        magicLoopBtn.setToggleState   (m == 0, juce::dontSendNotification);
        magicGlitchBtn.setToggleState (m == 1, juce::dontSendNotification);
        magicFlangeBtn.setToggleState (m == 2, juce::dontSendNotification);
        magicPsychBtn.setToggleState  (m == 3, juce::dontSendNotification);
        // Sync hold button UI for the new mode without clearing others
        holdBtn[m].setToggleState (holdOn[m], juce::dontSendNotification);
        if (auto* p = processor.getAPVTS().getParameter ("magic_mode"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) m));
        processor.getMagic().setMode (m);
        applyActive(); // stays ON if any hold is latched
        if (magicPad != nullptr)
        {
            magicPad->setModeColour (col);
            magicPad->setModeName (name);
        }
    };
    magicLoopBtn.onClick   = [setMode] { setMode (0, juce::Colour (0xff00e8ff), "LOOP"); };
    magicGlitchBtn.onClick = [setMode] { setMode (1, juce::Colour (0xffff2d9b), "GLITCH"); };
    magicFlangeBtn.onClick = [setMode] { setMode (2, juce::Colour (0xff39ff14), "FLANGE+VERB"); };
    magicPsychBtn.onClick  = [setMode] { setMode (3, juce::Colour (0xffffd700), "PSYCHEDELIC"); };

    magicHint.setText ("H-LOOP / H-GLITCH / H-FLANGE / H-PSY = hold per effect",
                       juce::dontSendNotification);
    magicHint.setJustificationType (juce::Justification::centred);
    magicHint.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
    magicHint.setFont (juce::FontOptions (10.5f));
    magicTab.addAndMakeVisible (magicHint);

    // Layout helper stored for resized — place hold buttons under each mode
    // (resized block in FullB will position mode rail; we also position holds here on first show)
    magicPad->onChange = magicPad->onChange; // keep

    processor.getMagic().setMode (0);
}


// ---- 14 Magic combination presets (weird interesting blends) ----
{
    static juce::TextButton magicCombo[14];
    static const char* comboNames[14] = {
        "VOID", "STAB", "WARP", "SLICE", "DRONE", "ACID", "GLASS",
        "CRASH", "ECHOX", "PSY+X", "LOOPY", "NOISE", "ALIEN", "TORO"
    };
    static bool combosInited = false;
    if (! combosInited)
    {
        combosInited = true;
        for (int i = 0; i < 14; ++i)
        {
            magicCombo[i].setButtonText (comboNames[i]);
            magicCombo[i].setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a2e));
            magicCombo[i].setColour (juce::TextButton::textColourOffId, juce::Colour (0xffff66cc));
            magicTab.addAndMakeVisible (magicCombo[i]);
            magicCombo[i].onClick = [this, i]
            {
                auto setP = [this] (const char* id, float v)
                {
                    if (auto* p = processor.getAPVTS().getParameter (id))
                    {
                        p->beginChangeGesture();
                        p->setValueNotifyingHost (p->convertTo0to1 (v));
                        p->endChangeGesture();
                    }
                };
                // Always enable magic (bool + engine)
                if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
                {
                    pa->beginChangeGesture();
                    pa->setValueNotifyingHost (1.f);
                    pa->endChangeGesture();
                }
                // 0 Loop 1 Glitch 2 Flange 3 Psy 4 Void 5 Slice 6 Acid 7 Crash 8 Noise
                // VOID STAB WARP SLICE DRONE ACID GLASS CRASH ECHOX PSY+X LOOPY NOISE ALIEN TORO
                const int modes[14] = { 4, 0, 3, 5, 0, 6, 2, 7, 2, 3, 0, 8, 3, 1 };
                const float xs[14]  = { 0.85f,0.3f,0.7f,0.95f,0.2f,0.6f,0.4f,0.9f,0.55f,0.75f,0.35f,0.8f,0.65f,0.88f };
                const float ys[14]  = { 0.9f,0.5f,0.8f,0.7f,0.4f,0.85f,0.6f,0.95f,0.55f,0.9f,0.45f,0.75f,0.85f,0.92f };
                if (auto* pm = dynamic_cast<juce::AudioParameterChoice*> (processor.getAPVTS().getParameter ("magic_mode")))
                {
                    pm->beginChangeGesture();
                    pm->setValueNotifyingHost (pm->convertTo0to1 ((float) modes[i]));
                    pm->endChangeGesture();
                }
                setP ("magic_x", xs[i]);
                setP ("magic_y", ys[i]);
                processor.getMagic().setMode (modes[i]);
                processor.getMagic().setXY (xs[i], ys[i]);
                processor.getMagic().setActive (true);
                if (i == 0) { setP ("reverb_mix", 0.55f); setP ("delay_mix", 0.25f); }
                if (i == 1) { setP ("dist_mix", 0.4f); setP ("master_drive", 0.45f); }
                if (i == 3 || i == 7) { setP ("delay_mix", 0.45f); setP ("delay_fb", 0.55f); }
                if (i == 5) { setP ("filter_cutoff", 600.f); setP ("filter_reso", 0.85f); setP ("filter_env", 0.9f); }
                if (i == 8) { setP ("delay_mix", 0.6f); setP ("reverb_mix", 0.35f); }
                if (i == 9) { setP ("phaser_mix", 0.5f); setP ("chorus_mix", 0.3f); }
                if (i == 11) { setP ("noise_level", 0.45f); setP ("dist_mix", 0.55f); }
                if (i == 12) { setP ("fm_2to1", 0.7f); setP ("reverb_mix", 0.45f); }
                if (i == 13) { setP ("osc1_fold", 0.55f); setP ("delay_mix", 0.4f); setP ("reverb_mix", 0.3f); }
                if (magicPad != nullptr)
                    magicPad->setPosition (xs[i], ys[i], true);
            };
        }
    }
    // stash pointer for layout via component name
    for (int i = 0; i < 14; ++i)
        magicCombo[i].setComponentID ("magicCombo" + juce::String (i));
}

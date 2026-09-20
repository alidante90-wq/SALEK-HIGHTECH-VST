// FX section labels + monitors + bypass + KNOBS (must exist or FX page is empty)
{
    const char* names[] = {
        "CHORUS", "DELAY", "REVERB", "BASSIFY",
        "COMP", "EQ", "PHASER", "DIST"
    };
    const char* bypassIds[] = {
        "chorus_bypass", "delay_bypass", "reverb_bypass", "bassify_bypass",
        "comp_bypass", "eq_bypass", "phaser_bypass", "dist_bypass"
    };
    FxMonitor::Kind kinds[] = {
        FxMonitor::Chorus, FxMonitor::Delay, FxMonitor::Reverb, FxMonitor::Master,
        FxMonitor::Comp, FxMonitor::EQ, FxMonitor::Phaser, FxMonitor::Dist
    };
    juce::Colour monCols[] = {
        juce::Colour (0xff00e8ff), juce::Colour (0xffff2d9b), juce::Colour (0xff39ff14), juce::Colour (0xffffd700),
        juce::Colour (0xffff6600), juce::Colour (0xffb388ff), juce::Colour (0xff00ffcc), juce::Colour (0xffff3355)
    };

    for (int i = 0; i < 8; ++i)
    {
        auto* lab = fxSectionLabels.add (new juce::Label ("fx", names[i]));
        lab->setJustificationType (juce::Justification::centredLeft);
        lab->setFont (juce::FontOptions (11.0f, juce::Font::bold));
        lab->setColour (juce::Label::textColourId, monCols[i]);
        lab->setColour (juce::Label::backgroundColourId, juce::Colour (0xff1a0a30));
        fxTab.addAndMakeVisible (lab);
        auto* mon = fxMonitors.add (new FxMonitor());
        mon->setKind (kinds[i]);
        mon->setAccent (monCols[i]);
        mon->setLevel (0.4f);
        if (kinds[i] == FxMonitor::Comp)
        {
            if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (processor.getAPVTS().getParameter ("comp_threshold")))
                mon->bindThresholdParam (p);
            mon->bindCompressor (&processor.getCompressor());
        }
        fxTab.addAndMakeVisible (mon);

        fxBypass[i].setButtonText ("BYP");
        fxBypass[i].setClickingTogglesState (true);
        fxBypass[i].setColour (juce::ToggleButton::textColourId, monCols[i]);
        fxTab.addAndMakeVisible (fxBypass[i]);
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            processor.getAPVTS(), bypassIds[i], fxBypass[i]));
    }

    // ===== CREATE FX KNOBS (indices start at 58 after LFO 52-57) =====
    {
        const auto C = juce::Colour (0xff00e8ff);
        const auto M = juce::Colour (0xffff2d9b);
        const auto O = juce::Colour (0xff39ff14);
        const auto V = juce::Colour (0xffc0ff00);
        const auto G = juce::Colour (0xff7c4dff);
        // 58-60 CHORUS
        addKnob (fxTab, "chorus_mix", "CHORUS", O);
        addKnob (fxTab, "chorus_rate", "C RATE", C);
        addKnob (fxTab, "chorus_depth", "C DEPTH", M);
        // 61-63 DELAY
        addKnob (fxTab, "delay_mix", "DELAY", O);
        addKnob (fxTab, "delay_time", "D TIME", C);
        addKnob (fxTab, "delay_fb", "D FB", M);
        // 64-66 REVERB
        addKnob (fxTab, "reverb_mix", "REVERB", O);
        addKnob (fxTab, "reverb_size", "R SIZE", C);
        addKnob (fxTab, "reverb_decay", "R DECAY", M);
        // 67 BASSIFY
        addKnob (fxTab, "bassify", "BASSIFY", V);
        // 68-71 COMP (global) + independent LO/MID/HI thresholds
        addKnob (fxTab, "comp_threshold", "C THR", O);
        addKnob (fxTab, "comp_ratio", "C RATIO", M);
        addKnob (fxTab, "comp_mix", "C MIX", C);
        addKnob (fxTab, "comp_gain", "C GAIN", G);
        addKnob (fxTab, "comp_depth", "C DEPTH", G);
        addKnob (fxTab, "comp_thr_lo", "LO THR", C);
        addKnob (fxTab, "comp_thr_mid", "MID THR", M);
        addKnob (fxTab, "comp_thr_hi", "HI THR", O);
        // EQ
        addKnob (fxTab, "eq_low", "EQ LOW", G);
        addKnob (fxTab, "eq_mid", "EQ MID", O);
        addKnob (fxTab, "eq_high", "EQ HI", M);
        // 75-77 PHASER
        addKnob (fxTab, "phaser_mix", "PHASER", O);
        addKnob (fxTab, "phaser_rate", "P RATE", C);
        addKnob (fxTab, "phaser_depth", "P DEPTH", M);
        // 78-80 DIST
        addKnob (fxTab, "dist_mix", "DIST", O);
        addKnob (fxTab, "dist_drive", "DRIVE", M);
        addKnob (fxTab, "dist_crush", "CRUSH", C);
        addKnob (fxTab, "delay_time_l", "D TIME L", C);
        addKnob (fxTab, "delay_time_r", "D TIME R", M);
    }

    // Mode menus + APVTS attachment (were missing → modes never applied)
    distModeBox.clear (juce::dontSendNotification);
    distModeBox.addItemList ({ "Tube", "Sat", "Hard", "Fold", "Bit", "Rect" }, 1);
    distModeBox.setSelectedItemIndex (0, juce::dontSendNotification);
    reverbModeBox.clear (juce::dontSendNotification);
    reverbModeBox.addItemList ({ "Room", "Hall", "Plate", "Chamber", "Spring" }, 1);
    reverbModeBox.setSelectedItemIndex (0, juce::dontSendNotification);
    fxTab.addAndMakeVisible (distModeBox);
    fxTab.addAndMakeVisible (reverbModeBox);
    delayModeBox.clear (juce::dontSendNotification);
    delayModeBox.addItemList ({ "Stereo", "PingPong", "Mono" }, 1);
    delayModeBox.setSelectedItemIndex (0, juce::dontSendNotification);
    fxTab.addAndMakeVisible (delayModeBox);
    comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.getAPVTS(), "delay_mode", delayModeBox));
    comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.getAPVTS(), "dist_mode", distModeBox));
    comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.getAPVTS(), "reverb_mode", reverbModeBox));
}


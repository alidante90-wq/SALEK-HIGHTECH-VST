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
        addKnob (fxTab, "bassify", "BASSIFY", V);
        addKnob (fxTab, "comp_threshold", "C THR", O);
        addKnob (fxTab, "comp_ratio", "C RATIO", M);
        addKnob (fxTab, "comp_mix", "C MIX", C);
        addKnob (fxTab, "eq_low", "EQ LOW", G);
        addKnob (fxTab, "eq_mid", "EQ MID", O);
        addKnob (fxTab, "eq_high", "EQ HI", M);
        addKnob (fxTab, "phaser_mix", "PHASER", O);
        addKnob (fxTab, "phaser_rate", "P RATE", C);
        addKnob (fxTab, "phaser_depth", "P DEPTH", M);
        addKnob (fxTab, "dist_mix", "DIST", G);
        addKnob (fxTab, "dist_drive", "D DRV", M);
        addKnob (fxTab, "dist_crush", "CRUSH", O);

        // Mode selectors
        reverbModeBox.addItemList ({ "Room", "Hall", "Plate", "Chamber", "Spring" }, 1);
        distModeBox.addItemList ({ "Tube", "Sat", "Hard", "Fold", "Bit", "Rect" }, 1);
        fxTab.addAndMakeVisible (reverbModeBox);
        fxTab.addAndMakeVisible (distModeBox);
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "reverb_mode", reverbModeBox));
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "dist_mode", distModeBox));
        reverbModeBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        reverbModeBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xff7c4dff));
        distModeBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        distModeBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffff6b00));

        const char* secs[] = { "CHORUS", "DELAY", "REVERB", "MASTER", "COMP", "EQ", "PHASER", "DISTORT" };
        const FxMonitor::Kind kinds[] = {
            FxMonitor::Chorus, FxMonitor::Delay, FxMonitor::Reverb, FxMonitor::Master,
            FxMonitor::Comp, FxMonitor::EQ, FxMonitor::Phaser, FxMonitor::Dist
        };
        juce::Colour monCols[] = {
            juce::Colour (0xff39ff14), juce::Colour (0xff00e8ff), juce::Colour (0xff7c4dff),
            juce::Colour (0xffffd700), juce::Colour (0xff66ccff), juce::Colour (0xffff66aa),
            juce::Colour (0xffff2d9b), juce::Colour (0xffff6b00)
        };
        const char* bypassIds[] = {
            "chorus_bypass","delay_bypass","reverb_bypass","bassify_bypass",
            "comp_bypass","eq_bypass","phaser_bypass","dist_bypass"
        };
        for (int i = 0; i < 8; ++i)
        {
            auto* lab = fxSectionLabels.add (new juce::Label());
            lab->setText (secs[i], juce::dontSendNotification);
            lab->setJustificationType (juce::Justification::centred);
            lab->setFont (juce::FontOptions (11.0f, juce::Font::bold));
            lab->setColour (juce::Label::textColourId, monCols[i]);
            lab->setColour (juce::Label::backgroundColourId, juce::Colour (0xff1a0a30));
            fxTab.addAndMakeVisible (lab);
            auto* mon = fxMonitors.add (new FxMonitor());
            mon->setKind (kinds[i]);
            mon->setAccent (monCols[i]);
            mon->setLevel (0.4f);
            fxTab.addAndMakeVisible (mon);

            fxBypass[i].setButtonText ("BYP");
            fxBypass[i].setClickingTogglesState (true);
            fxBypass[i].setColour (juce::ToggleButton::textColourId, monCols[i]);
            fxTab.addAndMakeVisible (fxBypass[i]);
            btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
                processor.getAPVTS(), bypassIds[i], fxBypass[i]));
        }

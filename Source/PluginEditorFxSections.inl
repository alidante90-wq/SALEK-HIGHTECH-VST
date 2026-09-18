// FX section labels + monitors + bypass toggles (Serum-style rows)
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

    // Dist / Reverb mode boxes already created elsewhere — ensure visible on FX
    fxTab.addAndMakeVisible (distModeBox);
    fxTab.addAndMakeVisible (reverbModeBox);
}

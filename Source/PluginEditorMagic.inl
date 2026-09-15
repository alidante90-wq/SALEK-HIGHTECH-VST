// MAGIC tab: Kaossilator-style hex pad + mode buttons (v2)
{
    magicPad = std::make_unique<MagicPad>();
    magicTab.addAndMakeVisible (*magicPad);
    magicPad->setModeColour (juce::Colour (0xff00e8ff));
    magicPad->setModeName ("LOOP");
    magicPad->onChange = [this] (float mx, float my, bool act)
    {
        if (auto* px = processor.getAPVTS().getParameter ("magic_x"))
            px->setValueNotifyingHost (px->convertTo0to1 (mx));
        if (auto* py = processor.getAPVTS().getParameter ("magic_y"))
            py->setValueNotifyingHost (py->convertTo0to1 (my));
        if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
            pa->setValueNotifyingHost (act ? 1.f : 0.f);
        processor.getMagic().setXY (mx, my);
        processor.getMagic().setActive (act);
    };

    auto styleBtn = [] (juce::TextButton& b, juce::Colour c)
    {
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff12081c));
        b.setColour (juce::TextButton::buttonOnColourId, c.darker (0.15f));
        b.setColour (juce::TextButton::textColourOffId, c);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        b.setClickingTogglesState (true);
    };
    styleBtn (magicLoopBtn, juce::Colour (0xff00e8ff));
    styleBtn (magicGlitchBtn, juce::Colour (0xffff2d9b));
    styleBtn (magicFlangeBtn, juce::Colour (0xff39ff14));
    styleBtn (magicPsychBtn, juce::Colour (0xffffd700));
    magicLoopBtn.setButtonText ("LOOP");
    magicGlitchBtn.setButtonText ("GLITCH");
    magicFlangeBtn.setButtonText ("FLANGE+VERB");
    magicPsychBtn.setButtonText ("PSYCHEDELIC");
    magicLoopBtn.setToggleState (true, juce::dontSendNotification);
    magicTab.addAndMakeVisible (magicLoopBtn);
    magicTab.addAndMakeVisible (magicGlitchBtn);
    magicTab.addAndMakeVisible (magicFlangeBtn);
    magicTab.addAndMakeVisible (magicPsychBtn);

    auto setMode = [this] (int m, juce::Colour col, const char* name)
    {
        magicLoopBtn.setToggleState (m == 0, juce::dontSendNotification);
        magicGlitchBtn.setToggleState (m == 1, juce::dontSendNotification);
        magicFlangeBtn.setToggleState (m == 2, juce::dontSendNotification);
        magicPsychBtn.setToggleState (m == 3, juce::dontSendNotification);
        if (auto* p = processor.getAPVTS().getParameter ("magic_mode"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) m));
        processor.getMagic().setMode (m);
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

    magicHint.setText (
        "Touch hex = engage  |  X/Y morph effect  |  Release = smooth bypass  |  LOOP freezes on touch",
        juce::dontSendNotification);
    magicHint.setJustificationType (juce::Justification::centred);
    magicHint.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
    magicHint.setFont (juce::FontOptions (11.5f));
    magicTab.addAndMakeVisible (magicHint);

    // ensure engine mode matches default UI
    processor.getMagic().setMode (0);
}

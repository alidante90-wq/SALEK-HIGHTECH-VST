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
        const bool on = holdOn[activeMode];
        magicHold.setToggleState (on, juce::dontSendNotification);
        if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
            pa->setValueNotifyingHost (on ? 1.f : 0.f);
        processor.getMagic().setActive (on);
    };

    magicPad->onChange = [this] (float mx, float my, bool act)
    {
        if (auto* px = processor.getAPVTS().getParameter ("magic_x"))
            px->setValueNotifyingHost (px->convertTo0to1 (mx));
        if (auto* py = processor.getAPVTS().getParameter ("magic_y"))
            py->setValueNotifyingHost (py->convertTo0to1 (my));
        const bool on = holdOn[activeMode] ? true : act;
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
        activeMode = m;
        magicLoopBtn.setToggleState   (m == 0, juce::dontSendNotification);
        magicGlitchBtn.setToggleState (m == 1, juce::dontSendNotification);
        magicFlangeBtn.setToggleState (m == 2, juce::dontSendNotification);
        magicPsychBtn.setToggleState  (m == 3, juce::dontSendNotification);
        if (auto* p = processor.getAPVTS().getParameter ("magic_mode"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) m));
        processor.getMagic().setMode (m);
        applyActive();
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

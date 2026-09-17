// MAGIC tab: Kaossilator pad + HOLD (latch) + mode buttons
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
        // HOLD latches FX on; otherwise follow finger
        const bool on = magicHold.getToggleState() ? true : act;
        if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
            pa->setValueNotifyingHost (on ? 1.f : 0.f);
        processor.getMagic().setXY (mx, my);
        processor.getMagic().setActive (on);
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
    styleBtn (magicHold, juce::Colour (0xffffaa00));
    magicLoopBtn.setButtonText (juce::CharPointer_UTF8 ("\xd9\x84\xd9\x88\xd9\xbe / LOOP"));
    magicGlitchBtn.setButtonText (juce::CharPointer_UTF8 ("\xda\xaf\xd9\x84\xdb\x8c\xda\x86 / GLITCH"));
    magicFlangeBtn.setButtonText (juce::CharPointer_UTF8 ("\xd9\x81\xd9\x84\xd9\x86\xd8\xac / FLANGE"));
    magicPsychBtn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xb1\xd9\x88\xd8\xa7\xd9\x86\xdb\x8c / PSY"));
    magicHold.setButtonText (juce::CharPointer_UTF8 ("\xd9\x86\xda\xaf\xd9\x87\xd8\xaf\xd8\xa7\xd8\xb1 / HOLD"));
    magicLoopBtn.setToggleState (true, juce::dontSendNotification);
    magicTab.addAndMakeVisible (magicLoopBtn);
    magicTab.addAndMakeVisible (magicGlitchBtn);
    magicTab.addAndMakeVisible (magicFlangeBtn);
    magicTab.addAndMakeVisible (magicPsychBtn);
    magicTab.addAndMakeVisible (magicHold);

    magicHold.onClick = [this]
    {
        if (! magicHold.getToggleState())
        {
            if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
                pa->setValueNotifyingHost (0.f);
            processor.getMagic().setActive (false);
        }
        else
        {
            if (auto* pa = processor.getAPVTS().getParameter ("magic_on"))
                pa->setValueNotifyingHost (1.f);
            processor.getMagic().setActive (true);
        }
    };

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
        juce::CharPointer_UTF8 (
            "HOLD=\xd9\xab\xd8\xa7\xd8\xa8\xd8\xaa \xd8\xa8\xd8\xaf\xd9\x88\xd9\x86 \xd9\x82\xd8\xb7\xd8\xb9  |  "
            "XY=\xd9\x85\xd9\x88\xd8\xb1\xd9\x81  |  "
            "Modes exclusive / \xd9\x85\xd9\x88\xd8\xaf\xd9\x87\xd8\xa7 \xd8\xa7\xd9\x86\xd8\xad\xd8\xb5\xd8\xa7\xd8\xb1\xdb\x8c"),
        juce::dontSendNotification);
    magicHint.setJustificationType (juce::Justification::centred);
    magicHint.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
    magicHint.setFont (juce::FontOptions (11.5f));
    magicTab.addAndMakeVisible (magicHint);

    processor.getMagic().setMode (0);
}

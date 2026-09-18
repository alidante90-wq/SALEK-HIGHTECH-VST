void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    auto charA = SalekAssets::loadCharPortrait (0);
    auto charB = SalekAssets::loadCharPortrait (1);
    auto charC = SalekAssets::loadCharPortrait (2);

    if (id == 1 || id == 2)
    {
        if (charA.isValid())       heroImg = charA;
        else if (lianImg.isValid()) heroImg = lianImg;
        else                       heroImg = faceImg;
        themeAccent  = (id == 2) ? juce::Colour (0xff39ff14) : juce::Colour (0xff00e8ff);
        themeAccent2 = (id == 2) ? juce::Colour (0xffffab40) : juce::Colour (0xffffd700);
        themePanelBg = (id == 2) ? juce::Colour (0xff061408) : juce::Colour (0xff0a0614);
    }
    else if (id == 3)
    {
        if (charB.isValid())       heroImg = charB;
        else if (cyanImg.isValid()) heroImg = cyanImg;
        else                       heroImg = faceImg;
        themeAccent = juce::Colour (0xffff2d9b);
        themeAccent2 = juce::Colour (0xff00e8ff);
        themePanelBg = juce::Colour (0xff12081c);
    }
    else
    {
        if (charC.isValid())       heroImg = charC;
        else if (cyanImg.isValid()) heroImg = cyanImg;
        else                       heroImg = faceImg;
        themeAccent = juce::Colour (0xffb388ff);
        themeAccent2 = juce::Colour (0xffffd700);
        themePanelBg = juce::Colour (0xff0c0818);
    }
    repaint();
}

void SalekHightechAudioProcessorEditor::cycleHero()
{
    heroIndex = (heroIndex + 1) % 4;
    const int ids[4] = { 1, 2, 3, 4 };
    themeBox.setSelectedId (ids[heroIndex], juce::sendNotification);
}

void SalekHightechAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    juce::ignoreUnused (e);
}

void SalekHightechAudioProcessorEditor::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    juce::ignoreUnused (e, wheel);
}

void SalekHightechAudioProcessorEditor::applyUiLanguage()
{
    if (uiLangFa)
    {
        tabs.setTabName (0, "MAIN");
        tabs.setTabName (1, "MOD");
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, "FX");
        tabs.setTabName (4, "MAGIC");
        tabs.setTabName (5, "SEQ");
    }
    else
    {
        tabs.setTabName (0, "MAIN");
        tabs.setTabName (1, "MOD");
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, "FX");
        tabs.setTabName (4, "MAGIC");
        tabs.setTabName (5, "SEQ");
    }
    magicLoopBtn.setButtonText ("LOOP");
    magicGlitchBtn.setButtonText ("GLITCH");
    magicFlangeBtn.setButtonText ("FLANGE");
    magicPsychBtn.setButtonText ("PSY");
    magicHold.setButtonText ("HOLD");
    for (int i = 0; i < 8; ++i)
        fxBypass[i].setButtonText ("BYP");
    repaint();
}

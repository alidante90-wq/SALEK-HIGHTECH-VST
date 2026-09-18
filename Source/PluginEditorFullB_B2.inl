void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    // 2 official backgrounds from concept art
    // 1 CYBER -> bg_isatis (man+cat city), 3 NEON -> bg_salek (girl red city)
    // 2 ACID / 4 PATINA map to same two for now
    if (id == 1 || id == 2)
        heroImg = lianImg.isValid() ? lianImg : faceImg;
    else
        heroImg = cyanImg.isValid() ? cyanImg : faceImg;

    if (id == 1) { themeAccent = juce::Colour (0xff00e8ff); themeAccent2 = juce::Colour (0xffffd700); themePanelBg = juce::Colour (0xff0a0614); }
    else if (id == 2) { themeAccent = juce::Colour (0xff39ff14); themeAccent2 = juce::Colour (0xffffab40); themePanelBg = juce::Colour (0xff061408); }
    else if (id == 3) { themeAccent = juce::Colour (0xffff2d9b); themeAccent2 = juce::Colour (0xff00e8ff); themePanelBg = juce::Colour (0xff12081c); }
    else { themeAccent = juce::Colour (0xffb388ff); themeAccent2 = juce::Colour (0xffffd700); themePanelBg = juce::Colour (0xff0c0818); }
    repaint();
}

void SalekHightechAudioProcessorEditor::cycleHero()
{
    heroIndex = (heroIndex + 1) % 2;
    themeBox.setSelectedId (heroIndex == 0 ? 1 : 3, juce::sendNotification);
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
        tabs.setTabName (0, juce::CharPointer_UTF8 ("\xd8\xa7\xd8\xb5\xd9\x84\xdb\x8c"));
        tabs.setTabName (1, juce::CharPointer_UTF8 ("\xd9\x85\xd8\xaf\xd9\x88\xd9\x84"));
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, "FX");
        tabs.setTabName (4, juce::CharPointer_UTF8 ("\xd9\x85\xd8\xac\xdb\x8c\xda\xa9"));
        tabs.setTabName (5, juce::CharPointer_UTF8 ("\xd8\xb3\xda\xa9\xd9\x88\xd8\xa7\xd9\x86\xd8\xb3"));
        magicLoopBtn.setButtonText ("LOOP");
        magicGlitchBtn.setButtonText ("GLITCH");
        magicFlangeBtn.setButtonText ("FLANGE");
        magicPsychBtn.setButtonText ("PSY");
        magicHold.setButtonText ("HOLD");
    }
    else
    {
        tabs.setTabName (0, "MAIN");
        tabs.setTabName (1, "MOD");
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, "FX");
        tabs.setTabName (4, "MAGIC");
        tabs.setTabName (5, "SEQ");
        magicLoopBtn.setButtonText ("LOOP");
        magicGlitchBtn.setButtonText ("GLITCH");
        magicFlangeBtn.setButtonText ("FLANGE");
        magicPsychBtn.setButtonText ("PSY");
        magicHold.setButtonText ("HOLD");
    }
    for (int i = 0; i < 8; ++i)
        fxBypass[i].setButtonText ("BYP");
    repaint();
}

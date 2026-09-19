void SalekHightechAudioProcessorEditor::refreshCharCache()
{
    charImgL = SalekAssets::loadCharPortrait (charPortraitIdx);
    charImgR = SalekAssets::loadCharPortrait (charPortraitIdx + 1);
}

void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    // ONLY the 2 official backgrounds:
    // 1/2 -> bg_isatis (you + cat / city)
    // 3/4 -> bg_salek (2 girls)
    if (id == 1 || id == 2)
    {
        heroImg = lianImg.isValid() ? lianImg : faceImg;
        themeAccent  = (id == 2) ? juce::Colour (0xff39ff14) : juce::Colour (0xff00e8ff);
        themeAccent2 = (id == 2) ? juce::Colour (0xffffab40) : juce::Colour (0xffffd700);
        themePanelBg = (id == 2) ? juce::Colour (0xff061408) : juce::Colour (0xff0a0614);
    }
    else
    {
        heroImg = cyanImg.isValid() ? cyanImg : faceImg;
        themeAccent  = (id == 3) ? juce::Colour (0xffff2d9b) : juce::Colour (0xffb388ff);
        themeAccent2 = juce::Colour (0xff00e8ff);
        themePanelBg = (id == 3) ? juce::Colour (0xff12081c) : juce::Colour (0xff0c0818);
    }
    repaint();
}

void SalekHightechAudioProcessorEditor::cycleHero()
{
    // Toggle between the 2 official arts only
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

// Computer keyboard → MIDI even while mouse is on knobs / other controls
bool SalekHightechAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (keyboard.keyPressed (key))
        return true;
    return false;
}

bool SalekHightechAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    if (keyboard.keyStateChanged (isKeyDown))
        return true;
    return false;
}

void SalekHightechAudioProcessorEditor::applyUiLanguage()
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
    for (int i = 0; i < 8; ++i)
        fxBypass[i].setButtonText ("BYP");
    repaint();
}

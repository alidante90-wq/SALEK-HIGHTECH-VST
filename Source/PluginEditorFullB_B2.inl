void SalekHightechAudioProcessorEditor::refreshCharCache()
{
    // Downscale once — full-res PNGs destroy UI FPS
    auto loadScaled = [] (int idx, int maxH) -> juce::Image
    {
        auto src = SalekAssets::loadCharPortrait (idx);
        if (! src.isValid()) return {};
        if (src.getHeight() <= maxH) return src;
        const int h = maxH;
        const int w = juce::jmax (1, src.getWidth() * h / src.getHeight());
        return src.rescaled (w, h, juce::Graphics::mediumResamplingQuality);
    };
    charImgL = loadScaled (charPortraitIdx, 480); // larger model in left slot // single model, lighter cache
    charImgR = {}; // right portrait removed — saves RAM/CPU
}

void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    // ONLY the 2 official backgrounds:
    // 1/2 -> bg_isatis (you + cat / city)
    // 3/4 -> bg_salek (2 girls)
    if (id == 1 || id == 2)
    {
        heroImg = lianImg;
        themeAccent  = (id == 2) ? juce::Colour (0xff39ff14) : juce::Colour (0xff00e8ff);
        themeAccent2 = (id == 2) ? juce::Colour (0xffffab40) : juce::Colour (0xffffd700);
        themePanelBg = (id == 2) ? juce::Colour (0xff061408) : juce::Colour (0xff0a0614);
    }
    else
    {
        heroImg = cyanImg;
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
    // A fixed chromatic piano row is predictable across hosts and starts in C2,
    // so bass patches are immediately playable. [ and ] shift the whole row.
    const juce::String keys = "zsxdcvgbhnjm,l.;/";
    const auto ch = juce::CharacterFunctions::toLowerCase (key.getTextCharacter());
    if (ch == '[' || ch == ']')
    {
        computerKeyboardBase = juce::jlimit (24, 84, computerKeyboardBase + (ch == '[' ? -12 : 12));
        return true;
    }
    const int keyIndex = keys.indexOfChar (ch);
    if (keyIndex >= 0)
    {
        const int note = juce::jlimit (0, 127, computerKeyboardBase + keyIndex);
        if (! computerKeyboardHeldNotes.contains (note))
        {
            computerKeyboardHeldNotes.add (note);
            processor.getKeyboardState().noteOn (1, note, 0.92f);
        }
        return true;
    }
    if (keyboard.keyPressed (key))
        return true;
    return false;
}

bool SalekHightechAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    const juce::String keys = "zsxdcvgbhnjm,l.;/";
    for (int i = computerKeyboardHeldNotes.size(); --i >= 0;)
    {
        const int note = computerKeyboardHeldNotes.getUnchecked (i);
        const int index = note - computerKeyboardBase;
        if (index < 0 || index >= keys.length()
            || ! juce::KeyPress::isKeyCurrentlyDown (keys[index]))
        {
            processor.getKeyboardState().noteOff (1, note, 0.0f);
            computerKeyboardHeldNotes.remove (i);
        }
    }
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
        fxBypass[i].setButtonText (juce::CharPointer_UTF8 ("⏻"));
    repaint();
}

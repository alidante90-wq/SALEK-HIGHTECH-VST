void SalekHightechAudioProcessorEditor::refreshCharCache()
{
    // Downscale once — full-res PNGs destroy UI FPS
    auto loadScaled = [] (int idx, int maxH) -> juce::Image
    {
        auto src = SalekAssets::loadCharPortrait (idx, 480, 720);
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

    // Hue-shift every knob (and label) toward current theme accent
    const float refH = juce::Colour (0xff00e8ff).getHue();
    const float delta = themeAccent.getHue() - refH;
    auto hueShift = [delta] (juce::Colour c) -> juce::Colour
    {
        float h = c.getHue() + delta;
        if (h > 1.f) h -= 1.f;
        if (h < 0.f) h += 1.f;
        return juce::Colour::fromHSV (h,
                                      juce::jlimit (0.35f, 1.f, c.getSaturation() * 1.05f),
                                      juce::jlimit (0.45f, 1.f, c.getBrightness()),
                                      c.getFloatAlpha());
    };
    for (auto& kk : knobs)
    {
        if (kk == nullptr) continue;
        const auto shifted = hueShift (kk->baseCol);
        kk->s.setColour (juce::Slider::rotarySliderFillColourId, shifted);
        kk->name.setColour (juce::Label::textColourId, shifted.withMultipliedBrightness (0.9f));
        kk->s.repaint();
    }
    lnf.setColour (juce::Slider::rotarySliderFillColourId, themeAccent);
    lnf.setColour (juce::TextButton::textColourOffId, themeAccent);
    lnf.setColour (juce::ComboBox::outlineColourId, themeAccent2);
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
    const auto ch = juce::CharacterFunctions::toLowerCase (key.getTextCharacter());
    if (ch == '-' || ch == '=')
    {
        computerKeyboardBase = juce::jlimit (24, 84, computerKeyboardBase + (ch == '-' ? -12 : 12));
        return true;
    }
    const juce::String rows[] = { "zxcvbnm,./", "asdfghjkl;'", "qwertyuiop[]\\" };
    const int rowStarts[] = { 36, 48, 60 };
    const int whiteNoteOffsets[] = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21 };
    int note = -1;
    for (int row = 0; row < 3 && note < 0; ++row)
    {
        const int index = rows[row].indexOfChar (ch);
        if (index >= 0 && index < 13)
            note = juce::jlimit (0, 127, computerKeyboardBase + rowStarts[row] - 36 + whiteNoteOffsets[index]);
    }
    if (note >= 0)
    {
        if (! computerKeyboardHeldKeys.contains ((int) ch))
        {
            const bool noteAlreadyHeld = computerKeyboardHeldNotes.contains (note);
            computerKeyboardHeldNotes.add (note);
            computerKeyboardHeldKeys.add ((int) ch);
            if (! noteAlreadyHeld)
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
    for (int i = computerKeyboardHeldKeys.size(); --i >= 0;)
    {
        const int keyChar = computerKeyboardHeldKeys.getUnchecked (i);
        if (keyChar < 0 || ! juce::KeyPress::isKeyCurrentlyDown (keyChar))
        {
            const int note = computerKeyboardHeldNotes.getUnchecked (i);
            bool stillHeld = false;
            for (int j = 0; j < computerKeyboardHeldNotes.size(); ++j)
                if (j != i && computerKeyboardHeldNotes.getUnchecked (j) == note) { stillHeld = true; break; }
            if (! stillHeld)
                processor.getKeyboardState().noteOff (1, note, 0.0f);
            computerKeyboardHeldNotes.remove (i);
            computerKeyboardHeldKeys.remove (i);
        }
    }
    if (keyboard.keyStateChanged (isKeyDown))
        return true;
    return false;
}

void SalekHightechAudioProcessorEditor::applyUiLanguage()
{
    // Cache original English knob labels once
    struct EnCache {
        static std::map<juce::String, juce::String>& map (SalekHightechAudioProcessorEditor& ed)
        {
            static std::map<juce::String, juce::String> m;
            static bool init = false;
            if (! init)
            {
                for (auto& kk : ed.knobs)
                    if (kk != nullptr)
                        m[kk->paramId] = kk->name.getText();
                init = true;
            }
            return m;
        }
    };
    auto& enLabels = EnCache::map (*this);

    auto faFor = [] (const juce::String& id) -> juce::String
    {
        // Iranian Persian (fa-IR) production terms — not Arabic
        if (id.contains ("cutoff")) return juce::CharPointer_UTF8 ("\xda\xa9\xd8\xa7\xd8\xaa\xe2\x80\x8c\xd8\xa7\xd9\x81");
        if (id.contains ("reso"))   return juce::CharPointer_UTF8 ("\xd8\xb1\xd8\xb2\xd9\x88");
        if (id.contains ("attack")) return juce::CharPointer_UTF8 ("\xd8\xa7\xd8\xaa\xda\xa9");
        if (id.contains ("decay"))  return juce::CharPointer_UTF8 ("\xd8\xaf\xdb\x8c\xda\xa9\xdb\x8c");
        if (id.contains ("sustain"))return juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd8\xb3\xd8\xaa\xdb\x8c\xd9\x86");
        if (id.contains ("release"))return juce::CharPointer_UTF8 ("\xd8\xb1\xdb\x8c\xd9\x84\xdb\x8c\xd8\xb2");
        if (id.contains ("_rate") || id.endsWith ("rate")) return juce::CharPointer_UTF8 ("\xd8\xb1\xdb\x8c\xd8\xaa");
        if (id.contains ("amount")) return juce::CharPointer_UTF8 ("\xd9\x85\xdb\x8c\xd8\xb2\xd8\xa7\xd9\x86");
        if (id.contains ("drive"))  return juce::CharPointer_UTF8 ("\xd8\xaf\xd8\xb1\xd8\xa7\xdb\x8c\xd9\x88");
        if (id.contains ("warp"))   return juce::CharPointer_UTF8 ("\xd9\x88\xd8\xa7\xd8\xb1\xd9\xbe");
        if (id.contains ("fold"))   return juce::CharPointer_UTF8 ("\xd9\x81\xd9\x88\xd9\x84\xd8\xaf");
        if (id.contains ("semi"))   return juce::CharPointer_UTF8 ("\xd9\x86\xdb\x8c\xd9\x85");
        if (id.contains ("coarse") || id.contains ("pitch") && id.contains ("crs")) return juce::CharPointer_UTF8 ("\xd8\xae\xd8\xb4\xd9\x86");
        if (id.contains ("unison") || id == "uni") return juce::CharPointer_UTF8 ("\xdb\x8c\xd9\x88\xd9\x86\xdb\x8c");
        if (id.contains ("detune") || id.contains ("det")) return juce::CharPointer_UTF8 ("\xd8\xaf\xd8\xaa\xdb\x8c\xd9\x88\xd9\x86");
        if (id.contains ("pan"))    return juce::CharPointer_UTF8 ("\xd9\xbe\xd9\x86");
        if (id.contains ("mix"))    return juce::CharPointer_UTF8 ("\xd9\x85\xdb\x8c\xda\xa9\xd8\xb3");
        if (id.contains ("glide"))  return juce::CharPointer_UTF8 ("\xda\xaf\xd9\x84\xd8\xa7\xdb\x8c\xd8\xaf");
        if (id.contains ("noise"))  return juce::CharPointer_UTF8 ("\xd9\x86\xd9\x88\xdb\x8c\xd8\xb2");
        if (id.contains ("sub"))    return juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd8\xa8");
        if (id.contains ("table"))  return juce::CharPointer_UTF8 ("\xd8\xac\xd8\xaf\xd9\x88\xd9\x84");
        if (id.contains ("level") || id.contains ("lvl")) return juce::CharPointer_UTF8 ("\xd9\x84\xd9\x88\xd9\x84");
        if (id.contains ("filter")) return juce::CharPointer_UTF8 ("\xd9\x81\xdb\x8c\xd9\x84\xd8\xaa\xd8\xb1");
        return {};
    };

    if (uiLangFa)
    {
        tabs.setTabName (0, juce::CharPointer_UTF8 ("\xd8\xa7\xd8\xb5\xd9\x84\xdb\x8c"));
        tabs.setTabName (1, juce::CharPointer_UTF8 ("\xd9\x85\xd8\xa7\xd8\xaf"));
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, juce::CharPointer_UTF8 ("\xd8\xa7\xd9\x81\xda\xa9\xd8\xaa"));
        tabs.setTabName (4, juce::CharPointer_UTF8 ("\xd9\x85\xd8\xac\xdb\x8c\xda\xa9"));
        tabs.setTabName (5, juce::CharPointer_UTF8 ("\xd8\xb3\xda\xa9\xd9\x88\xd8\xa6\xd9\x86\xd8\xb3"));
        magicHold.setButtonText (juce::CharPointer_UTF8 ("\xd9\x86\xda\xaf\xd9\x87\xe2\x80\x8c\xd8\xaf\xd8\xa7\xd8\xb1"));
        savePresetBtn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xb0\xd8\xae\xdb\x8c\xd8\xb1\xd9\x87"));
        loadPresetBtn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xa8\xd8\xa7\xd8\xb1\xda\xaf\xd8\xb0\xd8\xa7\xd8\xb1\xdb\x8c"));
        initBtn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xa7\xd9\x88\xd9\x84\xdb\x8c\xd9\x87"));
        langToggle.setButtonText (juce::CharPointer_UTF8 ("\xd9\x81\xd8\xa7"));
        for (auto& kk : knobs)
        {
            if (kk == nullptr) continue;
            auto fa = faFor (kk->paramId);
            kk->name.setText (fa.isNotEmpty() ? fa : kk->name.getText(), juce::dontSendNotification);
            kk->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        }
    }
    else
    {
        tabs.setTabName (0, "MAIN");
        tabs.setTabName (1, "MOD");
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, "FX");
        tabs.setTabName (4, "MAGIC");
        tabs.setTabName (5, "SEQ");
        magicHold.setButtonText ("HOLD");
        savePresetBtn.setButtonText ("SAVE");
        loadPresetBtn.setButtonText ("LOAD");
        initBtn.setButtonText ("INIT");
        langToggle.setButtonText ("EN");
        for (auto& kk : knobs)
        {
            if (kk == nullptr) continue;
            if (auto it = enLabels.find (kk->paramId); it != enLabels.end())
                kk->name.setText (it->second, juce::dontSendNotification);
            kk->name.setFont (juce::FontOptions (10.5f, juce::Font::bold));
        }
    }
    magicLoopBtn.setButtonText ("LOOP");
    magicGlitchBtn.setButtonText ("GLITCH");
    magicFlangeBtn.setButtonText ("FLANGE");
    magicPsychBtn.setButtonText ("PSY");
    for (int i = 0; i < 8; ++i)
        fxBypass[i].setButtonText (juce::CharPointer_UTF8 ("\xe2\x8f\x8b"));
    repaint();
}


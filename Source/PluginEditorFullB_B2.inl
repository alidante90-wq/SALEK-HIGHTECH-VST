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
    // Persian (Farsi / فارسی) — not Arabic
    if (uiLangFa)
    {
        tabs.setTabName (0, juce::String::fromUTF8 ("اصلی"));
        tabs.setTabName (1, juce::String::fromUTF8 ("ماد"));
        tabs.setTabName (2, juce::String::fromUTF8 ("ال‌اف‌او"));
        tabs.setTabName (3, juce::String::fromUTF8 ("افکت"));
        tabs.setTabName (4, juce::String::fromUTF8 ("مجیک"));
        tabs.setTabName (5, juce::String::fromUTF8 ("سکوئنس"));
        magicLoopBtn.setButtonText (juce::String::fromUTF8 ("لوپ"));
        magicGlitchBtn.setButtonText (juce::String::fromUTF8 ("گلیچ"));
        magicFlangeBtn.setButtonText (juce::String::fromUTF8 ("فلنج"));
        magicPsychBtn.setButtonText (juce::String::fromUTF8 ("سایکی"));
        magicHold.setButtonText (juce::String::fromUTF8 ("هلد"));
        arpOn.setButtonText (juce::String::fromUTF8 ("آرپ"));
        seqOn.setButtonText (juce::String::fromUTF8 ("سکوئنس"));
        for (int i = 0; i < 8; ++i)
            fxBypass[i].setButtonText (juce::String::fromUTF8 ("بایپس"));
        langToggle.setTooltip (juce::String::fromUTF8 ("زبان: فارسی / English"));
        auto setKnob = [&] (const char* id, const char* faUtf8)
        {
            for (auto& k : knobs)
                if (k != nullptr && k->paramId == id)
                    k->name.setText (juce::String::fromUTF8 (faUtf8), juce::dontSendNotification);
        };
        setKnob ("arp_rate", "سرعت آرپ");
        setKnob ("arp_octaves", "اکتاو آرپ");
        setKnob ("arp_gate", "گیت آرپ");
        setKnob ("arp_direction", "جهت آرپ");
        setKnob ("seq_rate", "سرعت سک");
        setKnob ("seq_swing", "سویینگ");
        setKnob ("seq_gate", "گیت سک");
        setKnob ("seq_steps", "استپ");
        setKnob ("seq_magic_depth", "عمق مجیک");
        setKnob ("filter_cutoff", "کات‌آف");
        setKnob ("filter_reso", "رزو");
        setKnob ("amp_attack", "حمله");
        setKnob ("amp_decay", "افت");
        setKnob ("amp_sustain", "نگهداشت");
        setKnob ("amp_release", "رها");
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
        arpOn.setButtonText ("ARP");
        seqOn.setButtonText ("SEQ");
        for (int i = 0; i < 8; ++i)
            fxBypass[i].setButtonText ("BYP");
        langToggle.setTooltip ("Language: English / فارسی");
        auto setKnob = [&] (const char* id, const char* en)
        {
            for (auto& k : knobs)
                if (k != nullptr && k->paramId == id)
                    k->name.setText (en, juce::dontSendNotification);
        };
        setKnob ("arp_rate", "ARP RATE");
        setKnob ("arp_octaves", "ARP OCT");
        setKnob ("arp_gate", "ARP GATE");
        setKnob ("arp_direction", "ARP DIR");
        setKnob ("seq_rate", "SEQ RATE");
        setKnob ("seq_swing", "SWING");
        setKnob ("seq_gate", "SEQ GATE");
        setKnob ("seq_steps", "STEPS");
        setKnob ("seq_magic_depth", "SEQ→MAGIC");
        setKnob ("filter_cutoff", "CUTOFF");
        setKnob ("filter_reso", "RESO");
        setKnob ("amp_attack", "ATTACK");
        setKnob ("amp_decay", "DECAY");
        setKnob ("amp_sustain", "SUSTAIN");
        setKnob ("amp_release", "RELEASE");
    }
    repaint();
}


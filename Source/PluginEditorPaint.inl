void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int theme = themeBox.getSelectedId();
    juce::Colour bg1 (0xff06040e), bg2 (0xff12081c), accent (0xff00f0ff), accent2 (0xffff2d6a);
    if (theme == 2) { bg1 = juce::Colour (0xff040a06); bg2 = juce::Colour (0xff0a1810); accent = juce::Colour (0xff66ff99); accent2 = juce::Colour (0xffc0ff00); }
    if (theme == 3) { bg1 = juce::Colour (0xff040810); bg2 = juce::Colour (0xff0a1420); accent = juce::Colour (0xff4fc3f7); accent2 = juce::Colour (0xff90caf9); }
    const float peak = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 2.8f);
    auto gval = [&](const char* id, float d = 0.f) -> float {
        if (auto* p = processor.getAPVTS().getRawParameterValue (id)) return p->load();
        return d;
    };
    float cut = juce::jmap (std::log (juce::jmax (20.0f, gval ("filter_cutoff", 8000.f))),
                            std::log (20.0f), std::log (20000.0f), 0.0f, 1.0f);

    juce::ColourGradient bg (bg1, 0, 0, bg2, 0, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    const float y0 = (float) getHeight() * 0.55f;
    g.setColour (accent.withAlpha (0.06f + pulse * 0.12f));
    for (int i = 0; i < 14; ++i) {
        float t = (float) i / 13.0f;
        g.drawLine (0.0f, y0 + t * t * ((float) getHeight() - y0), (float) getWidth(), y0 + t * t * ((float) getHeight() - y0), 1.0f);
    }
    for (int i = 0; i < 28; ++i) {
        float seed = (float) i * 17.3f;
        float x = std::fmod (seed * 37.0f + animPhase * (8.0f + peak * 40.0f), (float) getWidth());
        float y = std::fmod (seed * 23.0f + animPhase * 5.0f + cut * 40.0f, (float) getHeight());
        g.setColour ((i % 2 ? accent : accent2).withAlpha (0.2f + pulse * 0.4f));
        g.fillEllipse (x, y, 1.5f + pulse * 2.0f, 1.5f + pulse * 2.0f);
    }

    auto charPanel = juce::Rectangle<float> (6.0f, 52.0f, 172.0f, (float) getHeight() - 128.0f);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (charPanel.translated (3, 4), 14.0f);
    juce::ColourGradient cg (juce::Colour (0xff1e0c30), charPanel.getX(), charPanel.getY(),
                             juce::Colour (0xff080412), charPanel.getX(), charPanel.getBottom(), false);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (charPanel, 14.0f);
    g.setColour (accent2.withAlpha (0.3f + pulse * 0.4f));
    g.drawRoundedRectangle (charPanel, 14.0f, 2.5f);
    g.setColour (accent.withAlpha (0.6f + pulse * 0.3f));
    g.drawRoundedRectangle (charPanel.reduced (2), 12.0f, 1.2f);

    if (logoImg.isValid()) {
        auto lr = juce::Rectangle<float> (charPanel.getX() + 42, charPanel.getY() + 10, 88, 72);
        g.setOpacity (0.95f); g.drawImage (logoImg, lr, juce::RectanglePlacement::centred); g.setOpacity (1.0f);
    }

    float cx = charPanel.getCentreX();
    float cy = charPanel.getY() + 175.0f + (0.5f - cut) * 28.0f;
    for (int r = 4; r >= 0; --r) {
        float rad = 22.0f + r * 12.0f + pulse * 10.0f;
        g.setColour (accent.withAlpha (0.07f + pulse * 0.08f));
        g.drawEllipse (cx - rad, cy - rad * 0.85f, rad * 2, rad * 1.7f, 1.4f);
    }
    auto headR = juce::Rectangle<float> (cx - 48, cy - 58, 96, 110);
    juce::ColourGradient hg (juce::Colour (0xff3a1a55), headR.getX(), headR.getY(),
                             juce::Colour (0xff100818), headR.getX(), headR.getBottom(), false);
    g.setGradientFill (hg);
    g.fillRoundedRectangle (headR, 22.0f);
    g.setColour (accent.withAlpha (0.7f + pulse * 0.25f));
    g.drawRoundedRectangle (headR, 22.0f, 2.0f);

    auto visor = juce::Rectangle<float> (cx - 42, cy - 18, 84, 28);
    g.setColour (accent2.withAlpha (0.35f + pulse * 0.4f));
    g.fillRoundedRectangle (visor, 8.0f);
    g.setColour (accent);
    g.drawRoundedRectangle (visor, 8.0f, 1.2f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", visor, juce::Justification::centred);
    float eyeOff = (cut - 0.5f) * 8.0f;
    for (int e = 0; e < 2; ++e) {
        float ex = cx + (e == 0 ? -18.0f : 10.0f) + eyeOff;
        g.setColour (accent.withAlpha (0.4f + pulse * 0.5f));
        g.fillEllipse (ex - 6, cy + 14, 14, 10);
        g.setColour (juce::Colours::white);
        g.fillEllipse (ex - 2, cy + 16, 4, 4);
    }
    auto mouth = juce::Rectangle<float> (cx - 28, cy + 36, 56, 8);
    g.setColour (juce::Colour (0xff1a1020)); g.fillRoundedRectangle (mouth, 3.0f);
    g.setColour (accent2.interpolatedWith (accent, pulse));
    g.fillRoundedRectangle (mouth.withWidth (mouth.getWidth() * juce::jlimit (0.05f, 1.0f, pulse)), 3.0f);

    float ly = charPanel.getBottom() - 88.0f;
    auto led = [&](const char* lab, bool on, juce::Colour c) {
        g.setColour (on ? c : juce::Colour (0xff222230));
        g.fillEllipse (charPanel.getX() + 14, ly, 11, 11);
        if (on) { g.setColour (c.withAlpha (0.35f)); g.fillEllipse (charPanel.getX() + 11, ly - 3, 17, 17); }
        g.setColour (juce::Colour (0xffb0b0c8));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText (lab, charPanel.getX() + 30, ly - 2, 80, 14, juce::Justification::centredLeft);
        ly += 18.0f;
    };
    led ("ARP", gval ("arp_on") > 0.5f, accent);
    led ("SEQ", gval ("seq_on") > 0.5f, accent2);
    led ("DRV", gval ("master_drive") > 0.15f, juce::Colour (0xffff00aa));
    auto meter = juce::Rectangle<float> (charPanel.getX() + 12, charPanel.getBottom() - 26, charPanel.getWidth() - 24, 12);
    g.setColour (juce::Colour (0xff1a1020)); g.fillRoundedRectangle (meter, 4.0f);
    g.setColour (accent.interpolatedWith (accent2, pulse));
    g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * pulse), 4.0f);

    auto outer = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (accent.withAlpha (0.18f + pulse * 0.3f));
    g.drawRoundedRectangle (outer, 16.0f, 2.5f + pulse);

    auto head = juce::Rectangle<float> (184.0f, 6.0f, (float) getWidth() - 192.0f, 42.0f);
    g.setColour (juce::Colour (0xdd140a22));
    g.fillRoundedRectangle (head, 10.0f);
    g.setColour (accent.withAlpha (0.55f + pulse * 0.3f));
    g.drawRoundedRectangle (head, 10.0f, 1.3f);
    g.setColour (accent);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.getX() + 14, head.getY() + 3, 280, 24, juce::Justification::centredLeft);
    g.setColour (accent2);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ALIEN  ·  3D REACTIVE  ·  MATRIX  ·  v1.0", head.getX() + 14, head.getY() + 24, 360, 14, juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xffc8c8e0));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    juce::String strip = "CUT " + juce::String ((int) gval ("filter_cutoff", 8000)) + "Hz  RES "
        + juce::String (gval ("filter_reso"), 2) + "  UNI " + juce::String ((int) gval ("unison_voices", 1))
        + "  DRV " + juce::String (gval ("master_drive"), 2);
    g.drawText (strip, head.getRight() - 400, head.getY() + 12, 390, 18, juce::Justification::centredRight);
}

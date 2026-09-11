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

    g.setColour (accent.withAlpha (0.04f + pulse * 0.08f));
    const float y0 = (float) getHeight() * 0.62f;
    for (int i = 0; i < 10; ++i) {
        float t = (float) i / 9.0f;
        float y = y0 + t * t * ((float) getHeight() - y0 - 80.0f);
        g.drawLine (180.0f, y, (float) getWidth() - 8.0f, y, 1.0f);
    }

    auto col = juce::Rectangle<float> (6.0f, 8.0f, 170.0f, (float) getHeight() - 90.0f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (col.translated (3, 4), 14.0f);
    juce::ColourGradient cg (juce::Colour (0xff1a0c28), col.getX(), col.getY(),
                             juce::Colour (0xff080412), col.getX(), col.getBottom(), false);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (col, 14.0f);
    g.setColour (accent.withAlpha (0.5f + pulse * 0.35f));
    g.drawRoundedRectangle (col, 14.0f, 2.0f);

    if (logoImg.isValid()) {
        auto lr = juce::Rectangle<float> (col.getX() + 35, col.getY() + 12, 100, 80);
        g.setOpacity (0.95f);
        g.drawImage (logoImg, lr, juce::RectanglePlacement::centred);
        g.setOpacity (1.0f);
    }

    float cx = col.getCentreX();
    float cy = col.getY() + 200.0f + (0.5f - cut) * 20.0f;
    for (int r = 3; r >= 0; --r) {
        float rad = 26.0f + r * 11.0f + pulse * 8.0f;
        g.setColour (accent.withAlpha (0.06f + pulse * 0.07f));
        g.drawEllipse (cx - rad, cy - rad * 0.85f, rad * 2.0f, rad * 1.7f, 1.3f);
    }
    auto headR = juce::Rectangle<float> (cx - 46, cy - 55, 92, 105);
    juce::ColourGradient hg (juce::Colour (0xff3a1a55), headR.getX(), headR.getY(),
                             juce::Colour (0xff100818), headR.getX(), headR.getBottom(), false);
    g.setGradientFill (hg);
    g.fillRoundedRectangle (headR, 20.0f);
    g.setColour (accent.withAlpha (0.75f + pulse * 0.2f));
    g.drawRoundedRectangle (headR, 20.0f, 2.0f);

    auto visor = juce::Rectangle<float> (cx - 40, cy - 16, 80, 26);
    g.setColour (accent2.withAlpha (0.4f + pulse * 0.35f));
    g.fillRoundedRectangle (visor, 7.0f);
    g.setColour (accent);
    g.drawRoundedRectangle (visor, 7.0f, 1.2f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", visor, juce::Justification::centred);

    float eyeOff = (cut - 0.5f) * 7.0f;
    for (int e = 0; e < 2; ++e) {
        float ex = cx + (e == 0 ? -16.0f : 8.0f) + eyeOff;
        g.setColour (accent.withAlpha (0.45f + pulse * 0.4f));
        g.fillEllipse (ex - 5, cy + 16, 12, 9);
        g.setColour (juce::Colours::white);
        g.fillEllipse (ex - 1, cy + 18, 3.5f, 3.5f);
    }
    auto mouth = juce::Rectangle<float> (cx - 26, cy + 36, 52, 7);
    g.setColour (juce::Colour (0xff1a1020));
    g.fillRoundedRectangle (mouth, 3.0f);
    g.setColour (accent2.interpolatedWith (accent, pulse));
    g.fillRoundedRectangle (mouth.withWidth (mouth.getWidth() * juce::jlimit (0.06f, 1.0f, pulse)), 3.0f);

    float ly = col.getBottom() - 100.0f;
    auto led = [&](const char* lab, bool on, juce::Colour c) {
        g.setColour (on ? c : juce::Colour (0xff222230));
        g.fillEllipse (col.getX() + 16, ly, 12, 12);
        if (on) { g.setColour (c.withAlpha (0.35f)); g.fillEllipse (col.getX() + 13, ly - 3, 18, 18); }
        g.setColour (juce::Colour (0xffc0c0d8));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (lab, col.getX() + 34, ly - 1, 90, 14, juce::Justification::centredLeft);
        ly += 20.0f;
    };
    led ("ARP", gval ("arp_on") > 0.5f, accent);
    led ("SEQ", gval ("seq_on") > 0.5f, accent2);
    led ("DRV", gval ("master_drive") > 0.15f, juce::Colour (0xffff00aa));

    auto meter = juce::Rectangle<float> (col.getX() + 14, col.getBottom() - 28, col.getWidth() - 28, 12);
    g.setColour (juce::Colour (0xff1a1020));
    g.fillRoundedRectangle (meter, 4.0f);
    g.setColour (accent.interpolatedWith (accent2, pulse));
    g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * pulse), 4.0f);
    g.setColour (accent.withAlpha (0.6f));
    g.drawRoundedRectangle (meter, 4.0f, 1.0f);

    g.setColour (accent.withAlpha (0.15f + pulse * 0.25f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 14.0f, 2.0f + pulse);

    auto head = juce::Rectangle<float> (184.0f, 8.0f, juce::jmax (120.0f, (float) getWidth() - 560.0f), 40.0f);
    g.setColour (juce::Colour (0xcc10081a));
    g.fillRoundedRectangle (head, 8.0f);
    g.setColour (accent.withAlpha (0.45f + pulse * 0.3f));
    g.drawRoundedRectangle (head, 8.0f, 1.2f);
    g.setColour (accent);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.reduced (10, 4), juce::Justification::centredLeft);
    g.setColour (accent2);
    g.setFont (juce::FontOptions (10.0f));
    g.drawText ("v1.1  ·  REACTIVE", head.getX() + 10, head.getY() + 22, 200, 14, juce::Justification::centredLeft);
}

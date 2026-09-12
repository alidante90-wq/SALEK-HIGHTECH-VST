// SALEK HIGHTECH v2 — cyber / anime / neon pro paint
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int theme = themeBox.getSelectedId();
    juce::Colour bgDeep (0xff03020a), bgMid (0xff0a0618), accent (0xff00e8ff), accent2 (0xffff2d9b), accent3 (0xffb44dff);
    if (theme == 2) { bgDeep = juce::Colour (0xff020a06); bgMid = juce::Colour (0xff06140c); accent = juce::Colour (0xff39ff14); accent2 = juce::Colour (0xffc0ff00); accent3 = juce::Colour (0xff00ffaa); }
    if (theme == 3) { bgDeep = juce::Colour (0xff02060e); bgMid = juce::Colour (0xff06101c); accent = juce::Colour (0xff4fc3f7); accent2 = juce::Colour (0xff7c4dff); accent3 = juce::Colour (0xff82b1ff); }

    const float peak = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 2.6f);
    const float t = animPhase;

    auto gval = [&](const char* id, float d = 0.f) -> float {
        if (auto* p = processor.getAPVTS().getRawParameterValue (id)) return p->load();
        return d;
    };

    juce::ColourGradient bg (bgDeep, 0, 0, bgMid, (float) getWidth() * 0.6f, (float) getHeight(), true);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (-getWidth() * 0.1f, getHeight() * 0.55f, getWidth() * 1.2f, getHeight() * 0.7f);

    for (int i = 0; i < 5; ++i)
    {
        float px = std::fmod (80.f + i * 210.f + std::sin (t * 0.4f + i) * 30.f, (float) getWidth());
        float py = 120.f + i * 95.f + std::cos (t * 0.35f + i * 1.2f) * 18.f;
        float rad = 40.f + pulse * 25.f + i * 8.f;
        auto c = (i % 2 == 0) ? accent : accent2;
        g.setColour (c.withAlpha (0.04f + pulse * 0.05f));
        g.fillEllipse (px - rad, py - rad * 0.6f, rad * 2.f, rad * 1.2f);
    }

    g.setColour (accent.withAlpha (0.03f + pulse * 0.04f));
    for (int y = 60; y < getHeight() - 90; y += 28)
        g.drawHorizontalLine (y, 12.f, (float) getWidth() - 12.f);
    for (int x = 12; x < getWidth() - 12; x += 48)
        g.drawVerticalLine (x, 60.f, (float) getHeight() - 90.f);

    {
        auto bar = juce::Rectangle<float> (8, 6, (float) getWidth() - 16, 48);
        juce::ColourGradient hg (juce::Colour (0xee0c0820), bar.getX(), bar.getY(),
                                 juce::Colour (0xee060412), bar.getX(), bar.getBottom(), false);
        g.setGradientFill (hg);
        g.fillRoundedRectangle (bar, 12.f);
        g.setColour (accent.withAlpha (0.45f + pulse * 0.35f));
        g.drawRoundedRectangle (bar, 12.f, 1.4f);

        if (logoImg.isValid())
        {
            auto lr = juce::Rectangle<float> (16, 8, 42, 42);
            g.setOpacity (0.95f);
            g.drawImage (logoImg, lr, juce::RectanglePlacement::centred);
            g.setOpacity (1.f);
            g.setColour (accent.withAlpha (0.3f + pulse * 0.4f));
            g.drawEllipse (lr.expanded (2.f + pulse * 2.f), 1.5f);
        }

        g.setColour (accent);
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawText ("SALEK", 64, 8, 120, 28, juce::Justification::centredLeft);
        g.setColour (accent2);
        g.drawText ("HIGHTECH", 148, 8, 160, 28, juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffa090c0));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("UFO LABORATORY  ·  185 BPM  ·  TORONOWLA", 64, 34, 320, 14, juce::Justification::centredLeft);
    }

    {
        auto col = juce::Rectangle<float> (10, 62, 168, (float) getHeight() - 148);
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (col.translated (3, 4), 16.f);
        juce::ColourGradient cg (juce::Colour (0xcc12082a), col.getX(), col.getY(),
                                 juce::Colour (0xcc060410), col.getX(), col.getBottom(), false);
        g.setGradientFill (cg);
        g.fillRoundedRectangle (col, 16.f);
        g.setColour (accent.withAlpha (0.5f + pulse * 0.4f));
        g.drawRoundedRectangle (col, 16.f, 1.8f + pulse);

        juce::Image art = heroImg.isValid() ? heroImg : faceImg;
        if (theme == 2 && lianImg.isValid()) art = lianImg;
        if (theme == 3 && faceImg.isValid()) art = faceImg;
        if (art.isValid())
        {
            auto ar = col.reduced (10, 12).withTrimmedBottom (70);
            juce::Path clip; clip.addRoundedRectangle (ar, 12.f);
            g.saveState();
            g.reduceClipRegion (clip);
            float sc = 1.0f + pulse * 0.02f;
            auto scaled = ar.withSizeKeepingCentre (ar.getWidth() * sc, ar.getHeight() * sc);
            g.setOpacity (0.92f + pulse * 0.08f);
            g.drawImage (art, scaled, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            g.restoreState();
            g.setColour (accent2.withAlpha (0.55f + pulse * 0.3f));
            g.drawRoundedRectangle (ar, 12.f, 1.6f);
            g.setColour (juce::Colours::white.withAlpha (0.1f + pulse * 0.12f));
            g.fillRoundedRectangle (ar.removeFromTop (22), 12.f);
        }

        float ly = col.getBottom() - 58;
        auto led = [&](const char* lab, bool on, juce::Colour c) {
            g.setColour (on ? c : juce::Colour (0xff1a1528));
            g.fillEllipse (col.getX() + 14, ly, 10, 10);
            if (on) {
                g.setColour (c.withAlpha (0.45f + pulse * 0.35f));
                g.fillEllipse (col.getX() + 10, ly - 4, 18, 18);
            }
            g.setColour (juce::Colour (0xffc8b8e0));
            g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
            g.drawText (lab, col.getX() + 30, ly - 2, 100, 14, juce::Justification::centredLeft);
            ly += 17;
        };
        led ("ARP", gval ("arp_on") > 0.5f, accent);
        led ("SEQ", gval ("seq_on") > 0.5f, accent2);
        led ("DRV", gval ("master_drive") > 0.15f, accent3);
    }

    {
        float cx = (float) getWidth() * 0.52f;
        float cy = (float) getHeight() * 0.42f;

        for (int r = 5; r >= 0; --r)
        {
            float rad = 55.f + r * 18.f + pulse * 22.f;
            g.setColour (accent.withAlpha (0.02f + pulse * 0.03f));
            g.fillEllipse (cx - rad, cy - rad * 0.85f, rad * 2.f, rad * 1.7f);
        }

        for (int i = 0; i < 6; ++i)
        {
            float rad = 28.f + i * 16.f + pulse * 10.f + std::sin (t * 2.f + i) * 3.f;
            float a = 0.15f + (1.f - i / 6.f) * 0.35f + pulse * 0.25f;
            auto c = accent.interpolatedWith (accent2, (float) i / 5.f);
            g.setColour (c.withAlpha (a));
            g.drawEllipse (cx - rad, cy - rad * 0.9f, rad * 2.f, rad * 1.8f, 1.5f + (i == 0 ? 1.f : 0.f));
        }

        float coreR = 18.f + pulse * 8.f;
        juce::ColourGradient core (accent.brighter (0.4f), cx, cy - coreR,
                                   accent2.darker (0.3f), cx, cy + coreR, false);
        g.setGradientFill (core);
        g.fillEllipse (cx - coreR, cy - coreR, coreR * 2, coreR * 2);
        g.setColour (juce::Colours::white.withAlpha (0.5f + pulse * 0.3f));
        g.fillEllipse (cx - coreR * 0.35f, cy - coreR * 0.55f, coreR * 0.55f, coreR * 0.4f);

        for (int i = 0; i < 12; ++i)
        {
            float ang = t * (1.2f + i * 0.05f) + i * juce::MathConstants<float>::twoPi / 12.f;
            float rr = 48.f + (i % 3) * 14.f + pulse * 12.f;
            float px = cx + std::cos (ang) * rr;
            float py = cy + std::sin (ang) * rr * 0.75f;
            g.setColour (accent2.withAlpha (0.4f + pulse * 0.4f));
            g.fillEllipse (px - 2.5f, py - 2.5f, 5.f, 5.f);
        }

        g.setColour (accent.withAlpha (0.85f));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("SONIC CORE", cx - 60, cy + 72, 120, 14, juce::Justification::centred);
        g.setColour (accent2.withAlpha (0.7f));
        g.setFont (juce::FontOptions (9.0f));
        g.drawText ("SALEK  ·  UFO BASS", cx - 70, cy + 86, 140, 12, juce::Justification::centred);
    }

    {
        auto vis = juce::Rectangle<float> (186, (float) getHeight() - 108, (float) getWidth() - 200, 26);
        g.setColour (juce::Colour (0x66060814));
        g.fillRoundedRectangle (vis, 6.f);
        g.setColour (accent.withAlpha (0.3f + pulse * 0.35f));
        g.drawRoundedRectangle (vis, 6.f, 1.f);
        const int bars = 64;
        float bw = (vis.getWidth() - 6.f) / bars;
        for (int i = 0; i < bars; ++i)
        {
            float env = 0.2f + 0.8f * pulse * (0.35f + 0.65f * std::sin (t * 3.5f + i * 0.28f));
            float h = env * (vis.getHeight() - 4.f);
            float x = vis.getX() + 3.f + i * bw;
            auto c = accent.interpolatedWith (accent2, (float) i / bars);
            g.setColour (c.withAlpha (0.5f + pulse * 0.4f));
            g.fillRoundedRectangle (x, vis.getBottom() - 2.f - h, bw * 0.65f, h, 1.5f);
        }
    }

    g.setColour (accent.withAlpha (0.12f + pulse * 0.22f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (3.f), 16.f, 2.f + pulse * 1.2f);
}

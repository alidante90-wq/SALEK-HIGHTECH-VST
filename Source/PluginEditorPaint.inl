// SALEK HIGHTECH — hyper-dynamic software graphics (no OpenGL)
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Theme-driven palette (CYBER / ACID / NEON)
    const int themeId = themeBox.getSelectedId();
    juce::Colour cyan, magenta, gold, purple, lime;
    if (themeId == 2) { // ACID — toxic green / yellow
        cyan    = juce::Colour (0xffb8ff00);
        magenta = juce::Colour (0xffff6b00);
        gold    = juce::Colour (0xffffee00);
        purple  = juce::Colour (0xff88ff00);
        lime    = juce::Colour (0xff39ff14);
    } else if (themeId == 3) { // NEON — hot pink / electric purple
        cyan    = juce::Colour (0xffff00dd);
        magenta = juce::Colour (0xff00ffff);
        gold    = juce::Colour (0xffff66ff);
        purple  = juce::Colour (0xffaa00ff);
        lime    = juce::Colour (0xffff2d9b);
    } else { // CYBER — cyan / magenta default
        cyan    = juce::Colour (0xff00e8ff);
        magenta = juce::Colour (0xffff2d9b);
        gold    = juce::Colour (0xffffd700);
        purple  = juce::Colour (0xffb44dff);
        lime    = juce::Colour (0xff39ff14);
    }

    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 5.0f);
    const float t     = animPhase;

    // --- deep cyber gradient with reactive shift ---
    {
        juce::Colour c0 (0xff03010e), c1 (0xff12062a);
        float shift = 0.5f + 0.5f * std::sin (t * 0.55f);
        juce::ColourGradient bg (c0, 0, 0,
                                 c1.interpolatedWith (purple.darker (0.35f), shift * 0.45f + pulse * 0.15f),
                                 (float) getWidth(), (float) getHeight(), false);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // --- animated perspective grid ---
    {
        const float off = std::fmod (t * 28.0f, 36.0f);
        g.setColour (cyan.withAlpha (0.055f + pulse * 0.06f));
        for (float x = -off; x < (float) getWidth() + 40; x += 36.0f)
            g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
        for (float y = -off * 0.65f; y < (float) getHeight() + 40; y += 36.0f)
            g.drawHorizontalLine ((int) y, 0.0f, (float) getWidth());
    }

    // --- scanlines ---
    {
        float scanY = std::fmod (t * 120.0f, (float) getHeight() + 40.0f) - 20.0f;
        g.setColour (cyan.withAlpha (0.06f + pulse * 0.05f));
        g.fillRect (0.0f, scanY, (float) getWidth(), 2.5f);
        g.setColour (magenta.withAlpha (0.03f));
        for (int y = 0; y < getHeight(); y += 3)
            g.drawHorizontalLine (y, 0.0f, (float) getWidth());
    }

    // --- floating particles ---
    for (int i = 0; i < 72; ++i)
    {
        float px = std::fmod (20.0f + i * 87.3f + t * (14.0f + (i % 9)), (float) getWidth());
        float py = std::fmod (10.0f + i * 53.1f + t * (8.0f + (i % 5)), (float) getHeight());
        float sz = 1.0f + (i % 6) * 0.9f + pulse * 1.2f;
        juce::Colour pc = (i % 4 == 0) ? cyan : ((i % 4 == 1) ? magenta : ((i % 4 == 2) ? gold : lime));
        g.setColour (pc.withAlpha (0.22f + pulse * 0.35f));
        g.fillEllipse (px, py, sz, sz);
        if ((i % 11) == 0)
        {
            g.setColour (pc.withAlpha (0.08f + pulse * 0.12f));
            g.fillEllipse (px - 3.0f, py - 3.0f, sz + 6.0f, sz + 6.0f);
        }
    }

    // --- title ---
    g.setColour (cyan);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 16, 6, 380, 32, juce::Justification::centredLeft, false);
    g.setColour (gold.withAlpha (0.55f + pulse * 0.45f));
    g.fillRect (16.0f, 38.0f, 200.0f + pulse * 80.0f, 2.2f);

    // --- left hero panel ---
    {
        auto left = juce::Rectangle<float> (8.0f, 46.0f, 214.0f, 280.0f);
        g.setColour (juce::Colour (0xff080414).withAlpha (0.96f));
        g.fillRoundedRectangle (left, 14.0f);
        g.setColour (gold.withAlpha (0.85f + pulse * 0.15f));
        g.drawRoundedRectangle (left, 14.0f, 2.2f);

        g.setColour (cyan.withAlpha (0.08f + pulse * 0.12f));
        g.drawRoundedRectangle (left.expanded (3.0f), 16.0f, 3.0f);

        auto& src = heroImg.isValid() ? heroImg : logoImg;
        if (src.isValid())
        {
            auto imgR = left.reduced (8.0f, 8.0f).withHeight (168.0f);
            g.saveState();
            juce::Path clip;
            clip.addRoundedRectangle (imgR, 10.0f);
            g.reduceClipRegion (clip);
            g.setOpacity (0.90f + pulse * 0.10f);
            g.drawImageWithin (src, (int) imgR.getX(), (int) imgR.getY(),
                               (int) imgR.getWidth(), (int) imgR.getHeight(),
                               juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
            g.restoreState();
            g.setOpacity (1.0f);
            g.setColour (cyan.withAlpha (0.50f + pulse * 0.35f));
            g.drawRoundedRectangle (imgR, 10.0f, 1.6f);
        }

        g.setColour (gold);
        g.setFont (juce::FontOptions (34.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    juce::Rectangle<float> (left.getX(), left.getY() + 178.0f, left.getWidth(), 36.0f),
                    juce::Justification::centred, false);
        g.setColour (cyan.withAlpha (0.95f));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("SALEK", juce::Rectangle<float> (left.getX(), left.getY() + 210.0f, left.getWidth(), 16.0f), juce::Justification::centred);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.setFont (juce::FontOptions (9.5f));
        g.drawText ("PERSIAN CYBER SONIC CORE", juce::Rectangle<float> (left.getX(), left.getY() + 226.0f, left.getWidth(), 14.0f), juce::Justification::centred);
        g.setColour (magenta.withAlpha (0.9f));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText ("SYNTHESIZER VST", juce::Rectangle<float> (left.getX(), left.getY() + 242.0f, left.getWidth(), 14.0f), juce::Justification::centred);

        float starA = 0.55f + 0.45f * std::sin (t * 2.4f);
        g.setColour (gold.withAlpha (starA));
        juce::Path star;
        star.addStar (juce::Point<float> (left.getCentreX(), left.getBottom() - 18.0f), 6, 9.0f + pulse * 3.0f, 3.5f);
        g.fillPath (star);
    }

    // --- central energy core ---
    {
        const float cx = (float) getWidth() * 0.58f;
        const float cy = (float) getHeight() * 0.42f;
        const float baseR = 90.0f + pulse * 55.0f;

        for (int ring = 0; ring < 5; ++ring)
        {
            float rad = baseR * (0.35f + ring * 0.22f) + 8.0f * std::sin (t * (1.2f + ring * 0.3f));
            juce::Colour c = (ring % 3 == 0) ? cyan : ((ring % 3 == 1) ? magenta : gold);
            g.setColour (c.withAlpha (0.08f + pulse * 0.14f - ring * 0.01f));
            g.drawEllipse (cx - rad, cy - rad * 0.70f, rad * 2.0f, rad * 1.40f, 1.5f + pulse * 1.4f);
        }

        {
            float r = 48.0f + pulse * 30.0f;
            juce::ColourGradient glow (cyan.withAlpha (0.22f + pulse * 0.28f), cx, cy,
                                       magenta.withAlpha (0.0f), cx + r, cy, true);
            g.setGradientFill (glow);
            g.fillEllipse (cx - r, cy - r * 0.75f, r * 2.0f, r * 1.5f);
        }

        for (int layer = 0; layer < 3; ++layer)
        {
            juce::Path wave;
            float amp = (20.0f + pulse * 55.0f) * (1.0f - layer * 0.20f);
            float phase = t * (2.8f + layer * 0.7f) + layer * 1.1f;
            for (int i = 0; i < 96; ++i)
            {
                float x = cx - 110.0f + i * (220.0f / 95.0f);
                float y = cy + std::sin (phase + i * 0.22f + layer * 0.4f) * amp
                              + std::sin (phase * 1.7f + i * 0.11f) * amp * 0.35f;
                if (i == 0) wave.startNewSubPath (x, y);
                else        wave.lineTo (x, y);
            }
            juce::Colour wc = (layer == 0) ? cyan : ((layer == 1) ? magenta : gold);
            g.setColour (wc.withAlpha (0.40f + pulse * 0.40f - layer * 0.08f));
            g.strokePath (wave, juce::PathStrokeType (layer == 0 ? 2.8f : 1.5f));
        }

        juce::Path star;
        star.addStar (juce::Point<float> (cx, cy), 8, 32.0f + pulse * 18.0f, 12.0f);
        auto rot = juce::AffineTransform::rotation (t * 0.6f, cx, cy);
        g.setColour (gold.withAlpha (0.28f + pulse * 0.32f));
        g.fillPath (star, rot);
        g.setColour (cyan.withAlpha (0.5f + pulse * 0.3f));
        g.strokePath (star, juce::PathStrokeType (1.2f), rot);
    }

    // --- bottom ISATIS branding ---
    g.setColour (magenta.withAlpha (0.85f + pulse * 0.15f));
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("ISATIS", getWidth() - 220, getHeight() - 48, 200, 32, juce::Justification::centredRight, false);
    g.setColour (cyan.withAlpha (0.35f + pulse * 0.25f));
    g.fillRect ((float) getWidth() - 180.0f, (float) getHeight() - 16.0f, 150.0f + pulse * 30.0f, 2.0f);
}

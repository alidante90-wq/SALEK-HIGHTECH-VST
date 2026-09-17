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

    // --- theme background gradient (stronger per-theme identity) ---
    {
        juce::Colour c0, c1;
        if (themeId == 2) { // ACID
            c0 = juce::Colour (0xff040a02);
            c1 = juce::Colour (0xff1a2e08);
        } else if (themeId == 3) { // NEON
            c0 = juce::Colour (0xff0c0210);
            c1 = juce::Colour (0xff2a0838);
        } else { // CYBER
            c0 = juce::Colour (0xff03010e);
            c1 = juce::Colour (0xff12062a);
        }
        float shift = 0.5f + 0.5f * std::sin (t * 0.55f);
        juce::ColourGradient bg (c0, 0, 0,
                                 c1.interpolatedWith (purple.darker (0.25f), shift * 0.5f + pulse * 0.2f),
                                 (float) getWidth(), (float) getHeight(), false);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // animated grid
    {
        g.setColour (cyan.withAlpha (0.055f + pulse * 0.06f));
        const float spacing = 28.0f;
        const float ox = std::fmod (t * 12.0f, spacing);
        const float oy = std::fmod (t * 9.0f, spacing);
        for (float x = -spacing + ox; x < (float) getWidth() + spacing; x += spacing)
            g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
        for (float y = -spacing + oy; y < (float) getHeight() + spacing; y += spacing)
            g.drawHorizontalLine ((int) y, 0.0f, (float) getWidth());
        g.setColour (cyan.withAlpha (0.06f + pulse * 0.05f));
        g.drawHorizontalLine ((int) (std::fmod (t * 40.0f, (float) getHeight())), 0.0f, (float) getWidth());
        g.setColour (magenta.withAlpha (0.03f));
        g.drawHorizontalLine ((int) (std::fmod (t * 27.0f + 90.0f, (float) getHeight())), 0.0f, (float) getWidth());
    }

    // particles
    for (int i = 0; i < 72; ++i)
    {
        float px = std::fmod (t * (8.0f + (i % 7)) + i * 37.0f, (float) getWidth());
        float py = std::fmod (t * (5.0f + (i % 5)) + i * 53.0f, (float) getHeight());
        juce::Colour pc = (i % 4 == 0) ? cyan : ((i % 4 == 1) ? magenta : ((i % 4 == 2) ? gold : lime));
        g.setColour (pc.withAlpha (0.22f + pulse * 0.35f));
        float sz = 1.2f + (i % 3) * 0.6f + pulse * 1.2f;
        g.fillEllipse (px, py, sz, sz);
        if (i % 11 == 0)
        {
            g.setColour (pc.withAlpha (0.08f + pulse * 0.12f));
            g.fillEllipse (px - 3.0f, py - 3.0f, sz + 6.0f, sz + 6.0f);
        }
    }

    // brand text
    g.setColour (cyan);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 12, 6, 280, 22, juce::Justification::centredLeft);
    g.setColour (gold.withAlpha (0.55f + pulse * 0.45f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ISATIS", getWidth() - 100, getHeight() - 90, 90, 16, juce::Justification::centredRight);

    // left hero panel (skipped when sideCollapsed to free space)
    if (! sideCollapsed)
    {
        auto left = juce::Rectangle<float> (8.0f, 46.0f, 210.0f, 250.0f);
        g.setColour (juce::Colour (0xff080414).withAlpha (0.96f));
        g.fillRoundedRectangle (left, 12.0f);
        g.setColour (gold.withAlpha (0.85f + pulse * 0.15f));
        g.drawRoundedRectangle (left, 12.0f, 2.0f);

        g.setColour (cyan.withAlpha (0.08f + pulse * 0.12f));
        g.fillRoundedRectangle (left.reduced (4.0f), 10.0f);

        if (heroImg.isValid())
        {
            auto imgArea = left.reduced (10.0f, 12.0f).withHeight (160.0f);
            g.setOpacity (0.95f);
            g.drawImageWithin (heroImg, (int) imgArea.getX(), (int) imgArea.getY(),
                               (int) imgArea.getWidth(), (int) imgArea.getHeight(),
                               juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            g.setOpacity (1.0f);
        }
        else
        {
            g.setColour (cyan.withAlpha (0.50f + pulse * 0.35f));
            g.fillEllipse (left.getCentreX() - 40.0f, left.getY() + 50.0f, 80.0f, 80.0f);
        }

        g.setColour (gold);
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    juce::Rectangle<float> (left.getX(), left.getY() + 178.0f, left.getWidth(), 28.0f),
                    juce::Justification::centred);
        g.setColour (cyan.withAlpha (0.95f));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("SALEK", juce::Rectangle<float> (left.getX(), left.getY() + 206.0f, left.getWidth(), 16.0f), juce::Justification::centred);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.setFont (juce::FontOptions (9.0f));
        g.drawText ("PERSIAN CYBER SONIC CORE", juce::Rectangle<float> (left.getX(), left.getY() + 226.0f, left.getWidth(), 14.0f), juce::Justification::centred);
    }
    else
    {
        g.setColour (cyan.withAlpha (0.35f + pulse * 0.2f));
        g.fillRoundedRectangle (4.0f, 46.0f, 28.0f, 80.0f, 6.0f);
    }
}

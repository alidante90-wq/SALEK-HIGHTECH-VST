// SALEK HIGHTECH — animated software graphics (no OpenGL)
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const juce::Colour cyan    (0xff00e8ff);
    const juce::Colour magenta (0xffff2d9b);
    const juce::Colour gold    (0xffffd700);
    const juce::Colour purple  (0xffb44dff);

    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 3.2f);
    const float t     = animPhase;

    // === Animated deep background ===
    {
        juce::Colour c0 (0xff06031a), c1 (0xff12082a);
        float shift = 0.5f + 0.5f * std::sin (t * 0.4f);
        juce::ColourGradient bg (c0, 0, 0, c1.interpolatedWith (purple.darker (0.6f), shift * 0.25f),
                                 (float) getWidth(), (float) getHeight(), false);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // Moving grid
    {
        const float off = std::fmod (t * 12.0f, 48.0f);
        g.setColour (cyan.withAlpha (0.035f + pulse * 0.03f));
        for (float x = -off; x < (float) getWidth() + 48; x += 48.0f)
            g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
        for (float y = -off * 0.6f; y < (float) getHeight() + 48; y += 48.0f)
            g.drawHorizontalLine ((int) y, 0.0f, (float) getWidth());
    }

    // Floating particles
    for (int i = 0; i < 28; ++i)
    {
        float px = std::fmod (40.0f + i * 97.3f + t * (8.0f + (i % 5)), (float) getWidth());
        float py = std::fmod (20.0f + i * 53.1f + t * (5.0f + (i % 3)), (float) getHeight());
        float sz = 1.5f + (i % 4);
        juce::Colour pc = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : gold);
        g.setColour (pc.withAlpha (0.15f + pulse * 0.2f));
        g.fillEllipse (px, py, sz, sz);
    }

    // === TOP-LEFT brand ===
    g.setColour (cyan);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 16, 6, 380, 32, juce::Justification::centredLeft, false);
    g.setColour (gold.withAlpha (0.55f + pulse * 0.35f));
    g.fillRect (16, 38, 260, 2);

    // === LEFT logo panel + images ===
    {
        auto left = juce::Rectangle<float> (10.0f, 48.0f, 210.0f, 210.0f);
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.95f));
        g.fillRoundedRectangle (left, 14.0f);
        g.setColour (gold.withAlpha (0.85f + pulse * 0.12f));
        g.drawRoundedRectangle (left, 14.0f, 2.0f);

        if (heroImg.isValid())
        {
            auto imgArea = left.reduced (12.0f).withHeight (90.0f);
            g.setOpacity (0.55f + pulse * 0.2f);
            g.drawImageWithin (heroImg, (int) imgArea.getX(), (int) imgArea.getY(),
                               (int) imgArea.getWidth(), (int) imgArea.getHeight(),
                               juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            g.setOpacity (1.0f);
        }
        else if (logoImg.isValid())
        {
            auto imgArea = left.reduced (30.0f).withHeight (70.0f);
            g.setOpacity (0.7f);
            g.drawImageWithin (logoImg, (int) imgArea.getX(), (int) imgArea.getY(),
                               (int) imgArea.getWidth(), (int) imgArea.getHeight(),
                               juce::RectanglePlacement::centred);
            g.setOpacity (1.0f);
        }

        g.setColour (gold);
        g.setFont (juce::FontOptions (36.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    left.getX(), left.getY() + 95.0f, left.getWidth(), 42.0f,
                    juce::Justification::centred, false);
        g.setColour (cyan.withAlpha (0.95f));
        g.setFont (juce::FontOptions (12.5f, juce::Font::bold));
        g.drawText ("SALEK", left.getX(), left.getY() + 140.0f, left.getWidth(), 16.0f, juce::Justification::centred);
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("PERSIAN CYBER SONIC CORE", left.getX(), left.getY() + 158.0f, left.getWidth(), 14.0f, juce::Justification::centred);
        g.setColour (magenta.withAlpha (0.9f));
        g.drawText ("SYNTHESIZER VST", left.getX(), left.getY() + 174.0f, left.getWidth(), 14.0f, juce::Justification::centred);

        juce::Path star;
        star.addStar (juce::Point<float> (left.getCentreX(), left.getBottom() - 18.0f), 8, 11.0f, 5.5f);
        g.setColour (gold.withAlpha (0.7f + pulse * 0.25f));
        g.fillPath (star);
    }

    // === CENTRAL animated visualizer ===
    {
        const float cx = (float) getWidth() * 0.58f;
        const float cy = (float) getHeight() * 0.40f;
        const float baseR = 70.0f + pulse * 28.0f;

        for (int i = 0; i < 6; ++i)
        {
            float rad = baseR + i * 22.0f + 8.0f * std::sin (t * 1.6f + i * 0.9f);
            juce::Colour c = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : gold);
            g.setColour (c.withAlpha (0.08f + pulse * 0.1f));
            g.drawEllipse (cx - rad, cy - rad * 0.75f, rad * 2.0f, rad * 1.5f, 1.5f + pulse);
        }

        juce::Path star;
        star.addStar (juce::Point<float> (cx, cy), 8, 40.0f + pulse * 10.0f, 16.0f);
        g.setColour (gold.withAlpha (0.25f + pulse * 0.2f));
        g.strokePath (star, juce::PathStrokeType (1.5f));

        juce::Path wave;
        const int N = 100;
        float amp = 14.0f + pulse * 32.0f;
        for (int i = 0; i < N; ++i)
        {
            float x = cx - 140.0f + 280.0f * (float) i / (N - 1);
            float ph = (float) i / (N - 1) * juce::MathConstants<float>::twoPi * 3.0f + t * 2.8f;
            float y = cy + std::sin (ph) * amp * (0.5f + 0.5f * std::sin (t + i * 0.04f));
            if (i == 0) wave.startNewSubPath (x, y); else wave.lineTo (x, y);
        }
        g.setColour (cyan.withAlpha (0.65f + pulse * 0.25f));
        g.strokePath (wave, juce::PathStrokeType (2.0f));
        g.setColour (magenta.withAlpha (0.3f));
        g.strokePath (wave, juce::PathStrokeType (4.5f));
    }

    if (faceImg.isValid() || cyanImg.isValid())
    {
        auto& img = faceImg.isValid() ? faceImg : cyanImg;
        auto area = juce::Rectangle<int> (getWidth() - 160, 50, 140, 140);
        g.setOpacity (0.18f + pulse * 0.12f);
        g.drawImageWithin (img, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity (1.0f);
    }

    g.setColour (cyan);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("ISATIS", getWidth() - 230, getHeight() - 50, 210, 34, juce::Justification::centredRight, false);
    g.setColour (gold.withAlpha (0.7f));
    g.setFont (juce::FontOptions (10.5f));
    g.drawText ("VOICE OF THE ANCIENT FUTURE", getWidth() - 250, getHeight() - 18, 230, 14, juce::Justification::centredRight);

    g.setColour (cyan.withAlpha (0.45f));
    g.setFont (juce::FontOptions (10.0f));
    g.drawText ("SALEK HIGHTECH  //  LIVE ENGINE  //  PERSIAN CYBER", 16, getHeight() - 16, 480, 14, juce::Justification::centredLeft);
}

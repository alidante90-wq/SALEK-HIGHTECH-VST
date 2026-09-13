// SALEK HIGHTECH — animated software graphics (no OpenGL)
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const juce::Colour cyan    (0xff00e8ff);
    const juce::Colour magenta (0xffff2d9b);
    const juce::Colour gold    (0xffffd700);
    const juce::Colour purple  (0xffb44dff);

    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 4.0f);
    const float t     = animPhase;

    {
        juce::Colour c0 (0xff050218), c1 (0xff100628);
        float shift = 0.5f + 0.5f * std::sin (t * 0.35f);
        juce::ColourGradient bg (c0, 0, 0,
                                 c1.interpolatedWith (purple.darker (0.5f), shift * 0.3f),
                                 (float) getWidth(), (float) getHeight(), false);
        g.setGradientFill (bg);
        g.fillAll();
    }

    {
        const float off = std::fmod (t * 18.0f, 40.0f);
        g.setColour (cyan.withAlpha (0.045f + pulse * 0.04f));
        for (float x = -off; x < (float) getWidth() + 40; x += 40.0f)
            g.drawVerticalLine ((int) x, 0.0f, (float) getHeight());
        for (float y = -off * 0.7f; y < (float) getHeight() + 40; y += 40.0f)
            g.drawHorizontalLine ((int) y, 0.0f, (float) getWidth());
    }

    for (int i = 0; i < 56; ++i)
    {
        float px = std::fmod (30.0f + i * 91.7f + t * (10.0f + (i % 7)), (float) getWidth());
        float py = std::fmod (15.0f + i * 47.3f + t * (6.0f + (i % 4)), (float) getHeight());
        float sz = 1.2f + (i % 5) * 0.8f;
        juce::Colour pc = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : gold);
        g.setColour (pc.withAlpha (0.18f + pulse * 0.25f));
        g.fillEllipse (px, py, sz, sz);
    }

    g.setColour (cyan);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 16, 6, 380, 32, juce::Justification::centredLeft, false);
    g.setColour (gold.withAlpha (0.55f + pulse * 0.4f));
    g.fillRect (16, 38, 280, 2);

    {
        auto left = juce::Rectangle<float> (8.0f, 46.0f, 214.0f, 280.0f);
        g.setColour (juce::Colour (0xff080414).withAlpha (0.96f));
        g.fillRoundedRectangle (left, 14.0f);
        g.setColour (gold.withAlpha (0.9f + pulse * 0.1f));
        g.drawRoundedRectangle (left, 14.0f, 2.2f);

        auto& src = heroImg.isValid() ? heroImg : logoImg;
        if (src.isValid())
        {
            auto imgR = left.reduced (10.0f, 10.0f).withHeight (150.0f);
            g.saveState();
            juce::Path clip;
            clip.addRoundedRectangle (imgR, 10.0f);
            g.reduceClipRegion (clip);
            g.setOpacity (0.92f + pulse * 0.08f);
            g.drawImageWithin (src, (int) imgR.getX(), (int) imgR.getY(),
                               (int) imgR.getWidth(), (int) imgR.getHeight(),
                               juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
            g.restoreState();
            g.setOpacity (1.0f);
            g.setColour (cyan.withAlpha (0.55f + pulse * 0.3f));
            g.drawRoundedRectangle (imgR, 10.0f, 1.5f);
        }
        else
        {
            auto imgR = left.reduced (40.0f, 20.0f).withHeight (100.0f);
            g.setColour (cyan.withAlpha (0.3f + pulse * 0.25f));
            g.drawEllipse (imgR, 3.0f);
            g.setColour (gold);
            g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
            g.drawText ("SALEK", imgR, juce::Justification::centred);
        }

        g.setColour (gold);
        g.setFont (juce::FontOptions (34.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    juce::Rectangle<float> (left.getX(), left.getY() + 162.0f, left.getWidth(), 40.0f),
                    juce::Justification::centred, false);
        g.setColour (cyan.withAlpha (0.95f));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText ("SALEK", juce::Rectangle<float> (left.getX(), left.getY() + 200.0f, left.getWidth(), 16.0f), juce::Justification::centred);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.setFont (juce::FontOptions (9.5f));
        g.drawText ("PERSIAN CYBER SONIC CORE", juce::Rectangle<float> (left.getX(), left.getY() + 218.0f, left.getWidth(), 14.0f), juce::Justification::centred);
        g.setColour (magenta.withAlpha (0.9f));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText ("SYNTHESIZER VST", juce::Rectangle<float> (left.getX(), left.getY() + 234.0f, left.getWidth(), 14.0f), juce::Justification::centred);

        juce::Path star;
        star.addStar (juce::Point<float> (left.getCentreX(), left.getBottom() - 16.0f), 8, 12.0f, 5.5f);
        g.setColour (gold.withAlpha (0.75f + pulse * 0.25f));
        g.fillPath (star);
    }

    {
        const float cx = (float) getWidth() * 0.55f;
        const float cy = (float) getHeight() * 0.42f;
        const float baseR = 95.0f + pulse * 45.0f;

        for (int i = 0; i < 10; ++i)
        {
            float rad = baseR + i * 18.0f + 10.0f * std::sin (t * 1.8f + i * 0.7f);
            juce::Colour c = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : gold);
            g.setColour (c.withAlpha (0.10f + pulse * 0.12f));
            g.drawEllipse (cx - rad, cy - rad * 0.72f, rad * 2.0f, rad * 1.44f, 1.8f + pulse * 1.2f);
        }

        {
            float r = 50.0f + pulse * 25.0f;
            juce::ColourGradient glow (cyan.withAlpha (0.18f + pulse * 0.2f), cx, cy,
                                       magenta.withAlpha (0.0f), cx + r, cy, true);
            g.setGradientFill (glow);
            g.fillEllipse (cx - r, cy - r * 0.75f, r * 2.0f, r * 1.5f);
        }

        for (int layer = 0; layer < 3; ++layer)
        {
            juce::Path wave;
            const int N = 120;
            float amp = (22.0f + pulse * 48.0f) * (1.0f - layer * 0.22f);
            float yOff = layer * 8.0f;
            for (int i = 0; i < N; ++i)
            {
                float x = cx - 160.0f + 320.0f * (float) i / (float) (N - 1);
                float ph = (float) i / (float) (N - 1) * juce::MathConstants<float>::twoPi * 3.5f
                           + t * (3.2f + layer * 0.4f);
                float y = cy + yOff + std::sin (ph) * amp * (0.55f + 0.45f * std::sin (t * 1.1f + i * 0.03f));
                if (i == 0) wave.startNewSubPath (x, y);
                else        wave.lineTo (x, y);
            }
            juce::Colour wc = (layer == 0) ? cyan : ((layer == 1) ? magenta : gold);
            g.setColour (wc.withAlpha (0.35f + pulse * 0.35f - layer * 0.08f));
            g.strokePath (wave, juce::PathStrokeType (layer == 0 ? 2.6f : 1.4f));
        }

        juce::Path star;
        star.addStar (juce::Point<float> (cx, cy), 8, 36.0f + pulse * 14.0f, 14.0f);
        g.setColour (gold.withAlpha (0.3f + pulse * 0.25f));
        g.strokePath (star, juce::PathStrokeType (1.6f));
    }

    if (faceImg.isValid() || cyanImg.isValid())
    {
        auto& img = faceImg.isValid() ? faceImg : cyanImg;
        auto area = juce::Rectangle<int> (getWidth() - 150, 48, 130, 130);
        g.setOpacity (0.28f + pulse * 0.15f);
        g.drawImageWithin (img, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity (1.0f);
    }

    g.setColour (cyan);
    g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    g.drawText ("ISATIS", getWidth() - 220, getHeight() - 48, 200, 32, juce::Justification::centredRight, false);
    g.setColour (gold.withAlpha (0.7f));
    g.setFont (juce::FontOptions (10.0f));
    g.drawText ("VOICE OF THE ANCIENT FUTURE", getWidth() - 240, getHeight() - 16, 220, 12, juce::Justification::centredRight);

    g.setColour (cyan.withAlpha (0.4f));
    g.setFont (juce::FontOptions (9.5f));
    g.drawText ("SALEK HIGHTECH  //  LIVE ENGINE  //  PERSIAN CYBER", 16, getHeight() - 14, 460, 12, juce::Justification::centredLeft);
}

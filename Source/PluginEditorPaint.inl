// SALEK HIGHTECH — Complete software graphics engine (no OpenGL)
// Persian Cyber Anime style — pure JUCE Graphics
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const juce::Colour bgDeep  (0xff02010a);
    const juce::Colour bgMid   (0xff0a0618);
    const juce::Colour cyan    (0xff00e8ff);
    const juce::Colour magenta (0xffff2d9b);
    const juce::Colour gold    (0xffffd700);
    const juce::Colour purple  (0xffb44dff);

    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 3.0f);
    const float t     = animPhase;

    // === Background ===
    juce::ColourGradient bg (bgDeep, 0, 0, bgMid, (float) getWidth() * 0.5f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    // Subtle grid
    g.setColour (cyan.withAlpha (0.03f + pulse * 0.02f));
    for (int x = 0; x < getWidth(); x += 52)
        g.drawVerticalLine (x, 0.0f, (float) getHeight());
    for (int y = 0; y < getHeight(); y += 52)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    // === TOP-LEFT: SALEK HIGHTECH ===
    {
        g.setColour (cyan);
        g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
        g.drawText ("SALEK HIGHTECH", 16, 6, 380, 32, juce::Justification::centredLeft, false);
        g.setColour (gold.withAlpha (0.6f + pulse * 0.3f));
        g.fillRect (16, 38, 260, 2);
    }

    // === LEFT LOGO PANEL ===
    {
        auto left = juce::Rectangle<float> (10.0f, 50.0f, 210.0f, 200.0f);
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.94f));
        g.fillRoundedRectangle (left, 14.0f);
        g.setColour (gold.withAlpha (0.8f + pulse * 0.15f));
        g.drawRoundedRectangle (left, 14.0f, 2.2f);
        g.setColour (cyan.withAlpha (0.4f));
        g.drawRoundedRectangle (left.reduced (5.0f), 10.0f, 1.0f);

        // Persian سالک
        g.setColour (gold);
        g.setFont (juce::FontOptions (40.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    left.getX(), left.getY() + 25, left.getWidth(), 50,
                    juce::Justification::centred, false);

        g.setColour (cyan.withAlpha (0.95f));
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText ("SALEK", left.getX(), left.getY() + 82, left.getWidth(), 18, juce::Justification::centred);

        g.setColour (juce::Colours::white.withAlpha (0.65f));
        g.setFont (juce::FontOptions (10.5f));
        g.drawText ("PERSIAN CYBER SONIC CORE", left.getX(), left.getY() + 105, left.getWidth(), 14, juce::Justification::centred);

        g.setColour (magenta.withAlpha (0.9f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("SYNTHESIZER VST", left.getX(), left.getY() + 125, left.getWidth(), 14, juce::Justification::centred);

        // geometric star
        juce::Path star;
        star.addStar (juce::Point<float> (left.getCentreX(), left.getBottom() - 35.0f), 8, 13.0f, 6.5f);
        g.setColour (gold.withAlpha (0.75f + pulse * 0.2f));
        g.fillPath (star);
    }

    // === CENTRAL SOFTWARE VISUALIZER (replaces broken OpenGL) ===
    {
        const float cx = (float) getWidth() * 0.55f;
        const float cy = (float) getHeight() * 0.42f;
        const float baseR = 95.0f + pulse * 22.0f;

        // Outer rotating rings
        for (int i = 0; i < 7; ++i)
        {
            float rad = baseR + i * 28.0f + 10.0f * std::sin (t * 1.4f + i * 0.8f);
            juce::Colour c = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : gold);
            g.setColour (c.withAlpha (0.10f + pulse * 0.12f));
            g.drawEllipse (cx - rad, cy - rad * 0.78f, rad * 2.0f, rad * 1.56f, 2.0f + pulse * 1.5f);
        }

        // Mandala star layers
        for (int layer = 0; layer < 3; ++layer)
        {
            juce::Path star;
            float r1 = 55.0f - layer * 14.0f + pulse * 8.0f;
            float r2 = 22.0f - layer * 5.0f;
            star.addStar (juce::Point<float> (cx, cy), 8 + layer * 2, r1, r2);
            juce::Colour sc = (layer == 0) ? gold : ((layer == 1) ? cyan : magenta);
            g.setColour (sc.withAlpha (0.35f + pulse * 0.25f));
            g.strokePath (star, juce::PathStrokeType (1.6f + layer * 0.4f));
        }

        // Center glow core
        g.setColour (cyan.withAlpha (0.15f + pulse * 0.25f));
        g.fillEllipse (cx - 28.0f, cy - 28.0f, 56.0f, 56.0f);
        g.setColour (gold.withAlpha (0.5f + pulse * 0.4f));
        g.fillEllipse (cx - 12.0f, cy - 12.0f, 24.0f, 24.0f);

        // Simple reactive waveform across center
        juce::Path wave;
        const int N = 120;
        float amp = 18.0f + pulse * 35.0f;
        for (int i = 0; i < N; ++i)
        {
            float x = cx - 160.0f + (320.0f * (float) i / (N - 1));
            float ph = (float) i / (N - 1) * juce::MathConstants<float>::twoPi * 3.0f + t * 2.5f;
            float y = cy + std::sin (ph) * amp * (0.6f + 0.4f * std::sin (t + (float) i * 0.05f));
            if (i == 0) wave.startNewSubPath (x, y);
            else        wave.lineTo (x, y);
        }
        g.setColour (cyan.withAlpha (0.7f + pulse * 0.25f));
        g.strokePath (wave, juce::PathStrokeType (2.2f));
        g.setColour (magenta.withAlpha (0.4f));
        g.strokePath (wave, juce::PathStrokeType (5.0f));
    }

    // === BOTTOM-RIGHT: ISATIS ===
    {
        g.setColour (cyan);
        g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
        g.drawText ("ISATIS", getWidth() - 240, getHeight() - 52, 220, 36, juce::Justification::centredRight, false);
        g.setColour (gold.withAlpha (0.75f));
        g.setFont (juce::FontOptions (11.0f));
        g.drawText ("VOICE OF THE ANCIENT FUTURE", getWidth() - 260, getHeight() - 20, 240, 14, juce::Justification::centredRight);
    }

    // Status
    g.setColour (cyan.withAlpha (0.5f));
    g.setFont (juce::FontOptions (10.5f));
    g.drawText ("SALEK HIGHTECH  //  SOFTWARE ENGINE  //  PERSIAN CYBER", 16, getHeight() - 18, 460, 14, juce::Justification::centredLeft);
}

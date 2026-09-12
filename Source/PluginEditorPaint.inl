// SALEK HIGHTECH — Final Persian Cyber Anime UI (ISATIS edition)
// Hyper-realistic dark cyber + neon cyan/magenta/gold + Persian geometric accents
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Final palette from approved concept
    const juce::Colour bgDeep   (0xff02010a);
    const juce::Colour bgMid    (0xff0a0618);
    const juce::Colour cyan     (0xff00e8ff);
    const juce::Colour magenta  (0xffff2d9b);
    const juce::Colour gold     (0xffffd700);
    const juce::Colour purple   (0xffb44dff);
    const juce::Colour panelBg  (0xff0a0614);

    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 2.8f);
    const float t     = animPhase;

    // Deep background gradient
    juce::ColourGradient bg (bgDeep, 0, 0, bgMid, (float) getWidth() * 0.55f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    // Subtle cyber grid
    g.setColour (cyan.withAlpha (0.035f + pulse * 0.025f));
    for (int x = 0; x < getWidth(); x += 48)
        g.drawVerticalLine (x, 0.0f, (float) getHeight());
    for (int y = 0; y < getHeight(); y += 48)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    // ========== TOP-LEFT: SALEK HIGHTECH (very large) ==========
    {
        g.setColour (cyan);
        g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
        g.drawText ("SALEK HIGHTECH", 18, 8, 420, 36, juce::Justification::centredLeft, false);

        // subtle gold underline accent
        g.setColour (gold.withAlpha (0.55f + pulse * 0.3f));
        g.fillRect (18, 42, 280, 2);
    }

    // ========== LEFT LOGO PANEL (سالک) ==========
    {
        auto left = juce::Rectangle<float> (12.0f, 55.0f, 210.0f, 210.0f);
        g.setColour (panelBg.withAlpha (0.92f));
        g.fillRoundedRectangle (left, 14.0f);

        // ornate border
        g.setColour (gold.withAlpha (0.75f + pulse * 0.2f));
        g.drawRoundedRectangle (left, 14.0f, 2.0f);
        g.setColour (cyan.withAlpha (0.35f));
        g.drawRoundedRectangle (left.reduced (4.0f), 11.0f, 1.0f);

        // Persian "سالک" big
        g.setColour (gold);
        g.setFont (juce::FontOptions (42.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"), left.getX(), left.getY() + 28, left.getWidth(), 55,
                    juce::Justification::centred, false);

        // small English + tagline
        g.setColour (cyan.withAlpha (0.9f));
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText ("SALEK", left.getX(), left.getY() + 88, left.getWidth(), 20, juce::Justification::centred);

        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.setFont (juce::FontOptions (11.0f));
        g.drawText ("PERSIAN CYBER SONIC CORE", left.getX(), left.getY() + 112, left.getWidth(), 16, juce::Justification::centred);

        g.setColour (magenta.withAlpha (0.85f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("SYNTHESIZER VST", left.getX(), left.getY() + 135, left.getWidth(), 14, juce::Justification::centred);

        // decorative geometric diamond
        juce::Path diamond;
        float dcx = left.getCentreX();
        float dcy = left.getBottom() - 38.0f;
        diamond.addStar (juce::Point<float>(dcx, dcy), 8, 14.0f, 7.0f);
        g.setColour (gold.withAlpha (0.7f + pulse * 0.25f));
        g.fillPath (diamond);
    }

    // ========== BOTTOM-RIGHT: ISATIS (very large, English only) ==========
    {
        g.setColour (cyan);
        g.setFont (juce::FontOptions (36.0f, juce::Font::bold));
        g.drawText ("ISATIS", getWidth() - 260, getHeight() - 58, 240, 42, juce::Justification::centredRight, false);

        g.setColour (gold.withAlpha (0.8f));
        g.setFont (juce::FontOptions (12.0f));
        g.drawText ("VOICE OF THE ANCIENT FUTURE", getWidth() - 280, getHeight() - 22, 260, 16, juce::Justification::centredRight);
    }

    // ========== CENTRAL VISUALIZER GLOW RINGS (fallback) ==========
    {
        auto cx = (float) getWidth() * 0.52f;
        auto cy = (float) getHeight() * 0.48f;

        for (int i = 0; i < 6; ++i)
        {
            float rad = 55.0f + i * 32.0f + pulse * 18.0f + 8.0f * std::sin (t * 1.3f + i * 0.7f);
            juce::Colour c = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : gold);
            g.setColour (c.withAlpha (0.07f + pulse * 0.09f));
            g.drawEllipse (cx - rad, cy - rad * 0.82f, rad * 2.0f, rad * 1.64f, 2.0f + pulse);
        }

        // inner mandala star
        juce::Path star;
        star.addStar (juce::Point<float>(cx, cy), 8, 38.0f + pulse * 6.0f, 18.0f);
        g.setColour (gold.withAlpha (0.45f + pulse * 0.25f));
        g.strokePath (star, juce::PathStrokeType (1.8f));
    }

    // bottom status line
    g.setColour (cyan.withAlpha (0.55f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("SALEK HIGHTECH  //  ISATIS ENGINE  //  PERSIAN CYBER", 18, getHeight() - 20, 480, 14, juce::Justification::centredLeft);
}

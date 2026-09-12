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

    juce::ColourGradient bg (bgDeep, 0, 0, bgMid, (float) getWidth() * 0.6f, (float) getHeight(), true);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (accent.withAlpha (0.04f + pulse * 0.03f));
    for (int x = 0; x < getWidth(); x += 40)
        g.drawVerticalLine (x, 0.0f, (float) getHeight());
    for (int y = 0; y < getHeight(); y += 40)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    auto left = juce::Rectangle<int> (8, 52, 170, getHeight() - 140);
    g.setColour (juce::Colour (0xff0a0614).withAlpha (0.85f));
    g.fillRoundedRectangle (left.toFloat(), 12.0f);
    g.setColour (accent.withAlpha (0.35f + pulse * 0.25f));
    g.drawRoundedRectangle (left.toFloat(), 12.0f, 1.5f);

    if (logoImg.isValid())
    {
        auto lr = juce::Rectangle<int> (left.getX() + 20, left.getY() + 10, 130, 50);
        g.drawImageWithin (logoImg, lr.getX(), lr.getY(), lr.getWidth(), lr.getHeight(),
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }

    auto art = left.reduced (10).withTrimmedTop (60).withTrimmedBottom (20);
    juce::Image hero = heroImg.isValid() ? heroImg : (faceImg.isValid() ? faceImg : lianImg);
    if (theme == 2 && lianImg.isValid()) hero = lianImg;
    if (theme == 3 && cyanImg.isValid()) hero = cyanImg;
    if (hero.isValid())
    {
        g.setOpacity (0.9f);
        g.drawImageWithin (hero, art.getX(), art.getY(), art.getWidth(), art.getHeight(),
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity (1.0f);
    }

    {
        auto cx = (float) getWidth() * 0.55f;
        auto cy = (float) getHeight() * 0.42f;
        for (int i = 0; i < 4; ++i)
        {
            float rad = 40.0f + i * 28.0f + pulse * 12.0f + 6.0f * std::sin (t + i);
            g.setColour ((i % 2 == 0 ? accent : accent2).withAlpha (0.08f + pulse * 0.12f));
            g.drawEllipse (cx - rad, cy - rad * 0.7f, rad * 2.0f, rad * 1.4f, 1.5f);
        }
    }

    g.setColour (accent.withAlpha (0.7f));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH  //  PRESET LIBRARY ON MAIN", 12, getHeight() - 22, 420, 16, juce::Justification::centredLeft);
}
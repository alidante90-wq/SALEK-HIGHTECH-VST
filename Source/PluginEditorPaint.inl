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

    {
        float glowA = 0.03f + pulse * 0.12f;
        g.setColour (accent.withAlpha (glowA));
        g.fillEllipse ((float) getWidth() * 0.55f - 180.f - pulse * 40.f,
                       (float) getHeight() * 0.35f - 120.f - pulse * 30.f,
                       360.f + pulse * 80.f, 240.f + pulse * 60.f);
        g.setColour (accent2.withAlpha (glowA * 0.7f));
        g.fillEllipse ((float) getWidth() * 0.25f - 100.f,
                       (float) getHeight() * 0.7f - 80.f,
                       200.f + pulse * 40.f, 160.f + pulse * 30.f);
    }

    g.setColour (accent.withAlpha (0.035f + pulse * 0.07f));
    const float y0 = (float) getHeight() * 0.55f;
    for (int i = 0; i < 14; ++i) {
        float t = (float) i / 13.0f;
        float y = y0 + t * t * ((float) getHeight() - y0 - 90.0f);
        g.drawLine (190.0f, y, (float) getWidth() - 8.0f, y, 1.0f);
    }

    auto col = juce::Rectangle<float> (6.0f, 8.0f, 178.0f, (float) getHeight() - 96.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (col.translated (3, 4), 14.0f);
    juce::ColourGradient cg (juce::Colour (0xff1a0c28), col.getX(), col.getY(),
                             juce::Colour (0xff080412), col.getX(), col.getBottom(), false);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (col, 14.0f);
    g.setColour (accent.withAlpha (0.55f + pulse * 0.4f));
    g.drawRoundedRectangle (col, 14.0f, 2.2f + pulse * 1.5f);

    if (logoImg.isValid())
    {
        auto lr = juce::Rectangle<float> (col.getX() + 24, col.getY() + 10, 130, 130);
        g.setOpacity (0.98f);
        g.drawImage (logoImg, lr, juce::RectanglePlacement::centred);
        g.setOpacity (1.0f);
        g.setColour (accent.withAlpha (0.15f + pulse * 0.25f));
        g.drawEllipse (lr.expanded (6.f + pulse * 4.f), 2.0f + pulse);
    }

    {
        juce::Image art = heroImg.isValid() ? heroImg : faceImg;
        if (theme == 2 && lianImg.isValid()) art = lianImg;
        if (theme == 3 && faceImg.isValid()) art = faceImg;

        if (art.isValid())
        {
            auto ar = juce::Rectangle<float> (col.getX() + 8, col.getY() + 148, col.getWidth() - 16, 210);
            juce::Path clip;
            clip.addRoundedRectangle (ar, 12.0f);
            g.saveState();
            g.reduceClipRegion (clip);
            g.setOpacity (0.92f + pulse * 0.08f);
            g.drawImage (art, ar, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            g.restoreState();
            g.setColour (accent2.withAlpha (0.5f + pulse * 0.35f));
            g.drawRoundedRectangle (ar, 12.0f, 1.8f);
            g.setColour (juce::Colours::white.withAlpha (0.08f + pulse * 0.12f));
            g.fillRoundedRectangle (ar.withHeight (28.f), 12.0f);
        }
        else
        {
            float cx = col.getCentreX();
            float cy = col.getY() + 220.0f + (0.5f - cut) * 18.0f;
            auto headR = juce::Rectangle<float> (cx - 48, cy - 58, 96, 110);
            juce::ColourGradient hg (juce::Colour (0xff3a1a55), headR.getX(), headR.getY(),
                                     juce::Colour (0xff100818), headR.getX(), headR.getBottom(), false);
            g.setGradientFill (hg);
            g.fillRoundedRectangle (headR, 22.0f);
            g.setColour (accent.withAlpha (0.8f + pulse * 0.2f));
            g.drawRoundedRectangle (headR, 22.0f, 2.0f);
        }
    }

    float ly = col.getBottom() - 92.0f;
    auto led = [&](const char* lab, bool on, juce::Colour c) {
        g.setColour (on ? c : juce::Colour (0xff222230));
        g.fillEllipse (col.getX() + 14, ly, 11, 11);
        if (on) {
            g.setColour (c.withAlpha (0.4f + pulse * 0.3f));
            g.fillEllipse (col.getX() + 10, ly - 4, 19, 19);
        }
        g.setColour (juce::Colour (0xffc0c0d8));
        g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
        g.drawText (lab, col.getX() + 32, ly - 1, 100, 14, juce::Justification::centredLeft);
        ly += 18.0f;
    };
    led ("ARP", gval ("arp_on") > 0.5f, accent);
    led ("SEQ", gval ("seq_on") > 0.5f, accent2);
    led ("DRV", gval ("master_drive") > 0.15f, juce::Colour (0xffff00aa));

    auto meter = juce::Rectangle<float> (col.getX() + 12, col.getBottom() - 26, col.getWidth() - 24, 11);
    g.setColour (juce::Colour (0xff1a1020));
    g.fillRoundedRectangle (meter, 4.0f);
    g.setColour (accent.interpolatedWith (accent2, pulse));
    g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * pulse), 4.0f);
    g.setColour (accent.withAlpha (0.65f));
    g.drawRoundedRectangle (meter, 4.0f, 1.0f);

    g.setColour (accent.withAlpha (0.12f + pulse * 0.28f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 14.0f, 2.0f + pulse * 1.5f);

    auto head = juce::Rectangle<float> (190.0f, 8.0f, juce::jmax (120.0f, (float) getWidth() - 560.0f), 40.0f);
    g.setColour (juce::Colour (0xcc10081a));
    g.fillRoundedRectangle (head, 8.0f);
    g.setColour (accent.withAlpha (0.5f + pulse * 0.35f));
    g.drawRoundedRectangle (head, 8.0f, 1.4f);
    g.setColour (accent);
    g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", head.reduced (10, 4), juce::Justification::centredLeft);
    g.setColour (accent2);
    g.setFont (juce::FontOptions (10.0f));
    g.drawText ("v1.2  ·  TORONOWLA  ·  REACTIVE", head.getX() + 10, head.getY() + 22, 280, 14, juce::Justification::centredLeft);

    {
        auto vis = juce::Rectangle<float> (190.0f, (float) getHeight() - 108.0f, (float) getWidth() - 200.0f, 28.0f);
        g.setColour (juce::Colour (0x66080814));
        g.fillRoundedRectangle (vis, 6.0f);
        g.setColour (accent.withAlpha (0.25f + pulse * 0.4f));
        g.drawRoundedRectangle (vis, 6.0f, 1.0f);
        const int bars = 48;
        float bw = (vis.getWidth() - 8.f) / bars;
        for (int i = 0; i < bars; ++i)
        {
            float h = (0.15f + 0.85f * pulse * (0.4f + 0.6f * std::sin (animPhase * 3.f + i * 0.35f))) * (vis.getHeight() - 6.f);
            float x = vis.getX() + 4.f + i * bw;
            g.setColour (accent.interpolatedWith (accent2, (float) i / bars).withAlpha (0.55f + pulse * 0.35f));
            g.fillRoundedRectangle (x, vis.getBottom() - 3.f - h, bw * 0.7f, h, 2.0f);
        }
    }
}

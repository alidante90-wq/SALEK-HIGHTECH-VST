// SALEK HIGHTECH — GIF-like living background (software animated, no OpenGL required)
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int themeId = themeBox.getSelectedId();
    juce::Colour cyan, magenta, gold, purple, lime;
    if (themeId == 2) {
        cyan = juce::Colour (0xffb8ff00); magenta = juce::Colour (0xffff6b00);
        gold = juce::Colour (0xffffee00); purple = juce::Colour (0xff88ff00); lime = juce::Colour (0xff39ff14);
    } else if (themeId == 3) {
        cyan = juce::Colour (0xffff00dd); magenta = juce::Colour (0xff00ffff);
        gold = juce::Colour (0xffff66ff); purple = juce::Colour (0xffaa00ff); lime = juce::Colour (0xffff2d9b);
    } else if (themeId == 4) {
        cyan = juce::Colour (0xff7ef9e0); magenta = juce::Colour (0xffff2d9b);
        gold = juce::Colour (0xffff66aa); purple = juce::Colour (0xffff80c0); lime = juce::Colour (0xffb8ff40);
    } else {
        cyan = juce::Colour (0xff00e8ff); magenta = juce::Colour (0xffff2d9b);
        gold = juce::Colour (0xffffd700); purple = juce::Colour (0xffb44dff); lime = juce::Colour (0xff39ff14);
    }

    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 5.0f);
    const float t     = animPhase;
    const float W = (float) getWidth();
    const float H = (float) getHeight();

    // ========== layer 0: deep animated gradient wash ==========
    {
        juce::Colour c0, c1, c2;
        if (themeId == 4) {
            c0 = juce::Colour (0xff1a0c18);
            c1 = juce::Colour (0xff3a1830);
            c2 = juce::Colour (0xff2a2040);
        } else if (themeId == 2) {
            c0 = juce::Colour (0xff040a02); c1 = juce::Colour (0xff1a2e08); c2 = juce::Colour (0xff0c1a04);
        } else if (themeId == 3) {
            c0 = juce::Colour (0xff0c0210); c1 = juce::Colour (0xff2a0838); c2 = juce::Colour (0xff1a0430);
        } else {
            c0 = juce::Colour (0xff03010e); c1 = juce::Colour (0xff12062a); c2 = juce::Colour (0xff0a0420);
        }
        float s1 = 0.5f + 0.5f * std::sin (t * 0.4f);
        float s2 = 0.5f + 0.5f * std::sin (t * 0.7f + 1.3f);
        juce::Colour mid = c1.interpolatedWith (c2, s1).interpolatedWith (purple.darker (0.3f), s2 * 0.35f + pulse * 0.15f);
        juce::ColourGradient bg (c0, 0, 0, mid, W, H, false);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // ========== layer 1: soft hero photo wash (moving / breathing) ==========
    if (heroImg.isValid())
    {
        const float breathe = 1.0f + 0.04f * std::sin (t * 0.8f);
        const float panX = 30.0f * std::sin (t * 0.25f);
        const float panY = 18.0f * std::cos (t * 0.18f);
        auto dest = juce::Rectangle<float> (-40.0f + panX, -30.0f + panY, W * breathe + 80.0f, H * breathe + 60.0f);
        g.setOpacity (0.14f + pulse * 0.08f);
        g.drawImage (heroImg, dest, juce::RectanglePlacement::fillDestination);
        g.setOpacity (1.0f);
        // pink/mint tint overlay for PATINA
        if (themeId == 4)
        {
            g.setColour (magenta.withAlpha (0.07f + pulse * 0.05f));
            g.fillAll();
            g.setColour (cyan.withAlpha (0.05f));
            g.fillRect (0.0f, H * 0.55f, W, H * 0.45f);
        }
    }

    // ========== layer 2: flowing plasma ribbons ==========
    {
        for (int r = 0; r < 5; ++r)
        {
            juce::Path ribbon;
            const float yBase = H * (0.15f + r * 0.16f);
            const float amp = 28.0f + r * 10.0f + pulse * 20.0f;
            const float speed = 0.55f + r * 0.12f;
            const float phase = t * speed + r * 1.7f;
            bool first = true;
            for (float x = 0; x <= W; x += 8.0f)
            {
                float y = yBase
                    + std::sin (x * 0.012f + phase) * amp
                    + std::sin (x * 0.031f - phase * 1.3f) * (amp * 0.4f);
                if (first) { ribbon.startNewSubPath (x, y); first = false; }
                else ribbon.lineTo (x, y);
            }
            juce::Colour rc = (r % 2 == 0) ? cyan : magenta;
            g.setColour (rc.withAlpha (0.10f + pulse * 0.08f));
            g.strokePath (ribbon, juce::PathStrokeType (3.0f + r * 0.5f));
            g.setColour (rc.withAlpha (0.04f));
            g.strokePath (ribbon, juce::PathStrokeType (10.0f));
        }
    }

    // ========== layer 3: animated grid (parallax) ==========
    {
        const float spacing = 32.0f;
        const float ox = std::fmod (t * 18.0f, spacing);
        const float oy = std::fmod (t * 14.0f, spacing);
        g.setColour (cyan.withAlpha (0.06f + pulse * 0.05f));
        for (float x = -spacing + ox; x < W + spacing; x += spacing)
            g.drawVerticalLine ((int) x, 0.0f, H);
        for (float y = -spacing + oy; y < H + spacing; y += spacing)
            g.drawHorizontalLine ((int) y, 0.0f, W);
        // scanning beam
        float scanY = std::fmod (t * 55.0f, H + 40.0f) - 20.0f;
        juce::ColourGradient scan (cyan.withAlpha (0.0f), 0, scanY - 18.0f,
                                   cyan.withAlpha (0.18f + pulse * 0.15f), 0, scanY, false);
        scan.addColour (1.0, cyan.withAlpha (0.0f));
        g.setGradientFill (scan);
        g.fillRect (0.0f, scanY - 18.0f, W, 36.0f);
    }

    // ========== layer 4: floating orbs / bubbles (GIF energy) ==========
    for (int i = 0; i < 28; ++i)
    {
        float speed = 12.0f + (i % 9) * 3.5f;
        float px = std::fmod (t * speed * 0.7f + i * 47.0f, W + 60.0f) - 30.0f;
        float py = std::fmod (t * (speed * 0.45f) + i * 71.0f, H + 60.0f) - 30.0f;
        // slight orbit
        px += 18.0f * std::sin (t * 0.9f + i);
        py += 12.0f * std::cos (t * 0.7f + i * 0.5f);
        float sz = 6.0f + (i % 5) * 4.0f + pulse * 8.0f;
        juce::Colour oc = (i % 4 == 0) ? magenta : ((i % 4 == 1) ? cyan : ((i % 4 == 2) ? gold : lime));
        // glow
        g.setColour (oc.withAlpha (0.06f + pulse * 0.08f));
        g.fillEllipse (px - sz, py - sz, sz * 3.0f, sz * 3.0f);
        g.setColour (oc.withAlpha (0.25f + pulse * 0.35f));
        g.fillEllipse (px, py, sz, sz);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.fillEllipse (px + sz * 0.2f, py + sz * 0.15f, sz * 0.3f, sz * 0.3f);
    }

    // ========== layer 5: spark particles ==========
    for (int i = 0; i < 90; ++i)
    {
        float px = std::fmod (t * (10.0f + (i % 11)) + i * 41.0f, W);
        float py = std::fmod (t * (7.0f + (i % 7)) + i * 59.0f, H);
        juce::Colour pc = (i % 4 == 0) ? cyan : ((i % 4 == 1) ? magenta : ((i % 4 == 2) ? gold : lime));
        float sz = 1.0f + (i % 3) * 0.7f + pulse * 1.5f;
        g.setColour (pc.withAlpha (0.3f + pulse * 0.4f));
        g.fillEllipse (px, py, sz, sz);
        if (i % 8 == 0)
        {
            g.setColour (pc.withAlpha (0.1f));
            g.drawLine (px - 6.0f, py, px + 6.0f, py, 1.0f);
            g.drawLine (px, py - 6.0f, px, py + 6.0f, 1.0f);
        }
    }

    // ========== layer 6: corner energy arcs ==========
    {
        juce::Path arc;
        float r = 80.0f + pulse * 40.0f;
        arc.addCentredArc (0, 0, r + 20.0f * std::sin (t), r, 0, 0.2f, 1.4f, true);
        g.setColour (magenta.withAlpha (0.2f + pulse * 0.15f));
        g.strokePath (arc, juce::PathStrokeType (2.5f));
        juce::Path arc2;
        arc2.addCentredArc (W, H, r * 1.2f, r * 1.1f, 0, 3.5f + 0.3f * std::sin (t * 0.6f), 5.2f, true);
        g.setColour (cyan.withAlpha (0.18f + pulse * 0.12f));
        g.strokePath (arc2, juce::PathStrokeType (2.0f));
    }

    // ========== brand text ==========
    g.setColour (cyan);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 16, 4, 320, 24, juce::Justification::centredLeft);
    g.setColour (gold.withAlpha (0.55f + pulse * 0.45f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ISATIS", W - 100.0f, H - 90.0f, 90.0f, 16.0f, juce::Justification::centredRight);

    // left hero card
    auto left = juce::Rectangle<float> (8.0f, 48.0f, 210.0f, 280.0f);
    {
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.55f));
        g.fillRoundedRectangle (left, 12.0f);
        g.setColour (cyan.withAlpha (0.55f + pulse * 0.3f));
        g.drawRoundedRectangle (left, 12.0f, 1.6f);

        if (heroImg.isValid())
        {
            auto imgArea = left.reduced (8.0f, 8.0f).withHeight (200.0f);
            // subtle Ken-Burns on card image
            float z = 1.0f + 0.03f * std::sin (t * 0.5f);
            auto src = heroImg.getBounds().toFloat();
            float sw = src.getWidth() / z, sh = src.getHeight() / z;
            float sx = (src.getWidth() - sw) * (0.5f + 0.15f * std::sin (t * 0.3f));
            float sy = (src.getHeight() - sh) * (0.5f + 0.1f * std::cos (t * 0.25f));
            g.drawImage (heroImg,
                         (int) imgArea.getX(), (int) imgArea.getY(), (int) imgArea.getWidth(), (int) imgArea.getHeight(),
                         (int) sx, (int) sy, (int) sw, (int) sh);
        }

        g.setColour (magenta);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    juce::Rectangle<float> (left.getX(), left.getY() + 206.0f, left.getWidth(), 18.0f),
                    juce::Justification::centred);
        g.setColour (cyan);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("SALEK", juce::Rectangle<float> (left.getX(), left.getY() + 222.0f, left.getWidth(), 14.0f),
                    juce::Justification::centred);
        g.setColour (gold.withAlpha (0.7f));
        g.setFont (juce::FontOptions (9.0f));
        g.drawText ("PERSIAN CYBER SONIC CORE",
                    juce::Rectangle<float> (left.getX(), left.getY() + 238.0f, left.getWidth(), 12.0f),
                    juce::Justification::centred);
    }
}

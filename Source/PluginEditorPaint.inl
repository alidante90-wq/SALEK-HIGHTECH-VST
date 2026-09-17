// SALEK HIGHTECH — advanced multi-layer parallax + GIF-like living theme
//
// Parallax layers (far → near), each with different scroll/orbit speed:
//   L0 deep gradient morph (slowest color drift)
//   L1 distant starfield / dust
//   L2 soft hero photo (slow ken-burns + weak mouse parallax)
//   L3 plasma ribbons (mid depth)
//   L4 floating orbs
//   L5 near sparks + scan beam (fastest)
//   L6 UI chrome (static relative to window)
//
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int themeId = themeBox.getSelectedId();
    const float W = (float) getWidth();
    const float H = (float) getHeight();
    const float t = animPhase;
    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 5.0f);

    // --- mouse parallax (advanced): normalized -0.5..0.5 ---
    const auto mouse = getMouseXYRelative();
    const float mx = W > 1.f ? ((float) mouse.x / W - 0.5f) : 0.f;
    const float my = H > 1.f ? ((float) mouse.y / H - 0.5f) : 0.f;

    // Theme palette with time-morph (theme itself is animated)
    auto lerpCol = [] (juce::Colour a, juce::Colour b, float x) {
        return a.interpolatedWith (b, juce::jlimit (0.f, 1.f, x));
    };
    const float morph = 0.5f + 0.5f * std::sin (t * 0.35f);

    juce::Colour cyan, magenta, gold, purple, lime;
    if (themeId == 2) {
        cyan    = lerpCol (juce::Colour (0xffb8ff00), juce::Colour (0xff39ff14), morph);
        magenta = lerpCol (juce::Colour (0xffff6b00), juce::Colour (0xffffaa00), morph);
        gold = juce::Colour (0xffffee00); purple = juce::Colour (0xff88ff00); lime = juce::Colour (0xff39ff14);
    } else if (themeId == 3) {
        cyan    = lerpCol (juce::Colour (0xffff00dd), juce::Colour (0xffff66ff), morph);
        magenta = lerpCol (juce::Colour (0xff00ffff), juce::Colour (0xff7c4dff), morph);
        gold = juce::Colour (0xffff66ff); purple = juce::Colour (0xffaa00ff); lime = juce::Colour (0xffff2d9b);
    } else if (themeId == 4) {
        cyan    = lerpCol (juce::Colour (0xff7ef9e0), juce::Colour (0xffffb0d0), morph);
        magenta = lerpCol (juce::Colour (0xffff2d9b), juce::Colour (0xffff80c0), 1.f - morph);
        gold = juce::Colour (0xffff66aa); purple = juce::Colour (0xffff80c0); lime = juce::Colour (0xffb8ff40);
    } else {
        cyan    = lerpCol (juce::Colour (0xff00e8ff), juce::Colour (0xff7c4dff), morph);
        magenta = lerpCol (juce::Colour (0xffff2d9b), juce::Colour (0xffffd700), morph);
        gold = juce::Colour (0xffffd700); purple = juce::Colour (0xffb44dff); lime = juce::Colour (0xff39ff14);
    }

    // ========== L0 far: deep animated gradient (slowest parallax) ==========
    {
        juce::Colour c0, c1;
        if (themeId == 4)      { c0 = juce::Colour (0xff1a0c18); c1 = juce::Colour (0xff3a1830); }
        else if (themeId == 2) { c0 = juce::Colour (0xff040a02); c1 = juce::Colour (0xff1a2e08); }
        else if (themeId == 3) { c0 = juce::Colour (0xff0c0210); c1 = juce::Colour (0xff2a0838); }
        else                   { c0 = juce::Colour (0xff03010e); c1 = juce::Colour (0xff12062a); }

        // gradient centre drifts slowly + weak mouse (far = * 8 px)
        float gx = W * 0.5f + 40.f * std::sin (t * 0.22f) + mx * 8.f;
        float gy = H * 0.5f + 30.f * std::cos (t * 0.18f) + my * 6.f;
        juce::Colour mid = c1.interpolatedWith (purple.darker (0.25f), morph * 0.4f + pulse * 0.2f);
        juce::ColourGradient bg (c0, gx - W * 0.4f, gy - H * 0.4f, mid, gx + W * 0.5f, gy + H * 0.5f, true);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // ========== L1 far-mid: drifting star dust ==========
    {
        g.setColour (cyan.withAlpha (0.15f + pulse * 0.1f));
        for (int i = 0; i < 48; ++i)
        {
            // far parallax factor 0.15
            float px = std::fmod (t * (3.f + i % 5) + i * 53.f + mx * 12.f, W);
            float py = std::fmod (t * (2.f + i % 3) + i * 37.f + my * 10.f, H);
            float sz = 0.8f + (i % 3) * 0.5f;
            g.fillEllipse (px, py, sz, sz);
        }
    }

    // ========== L2 mid-far: hero photo wash (parallax 0.35) ==========
    if (heroImg.isValid())
    {
        const float breathe = 1.0f + 0.05f * std::sin (t * 0.65f);
        const float panX = 40.f * std::sin (t * 0.2f) + mx * 28.f; // mouse parallax
        const float panY = 24.f * std::cos (t * 0.15f) + my * 20.f;
        auto dest = juce::Rectangle<float> (-50.f + panX, -40.f + panY, W * breathe + 100.f, H * breathe + 80.f);
        g.setOpacity (0.16f + pulse * 0.1f);
        g.drawImage (heroImg, dest, juce::RectanglePlacement::fillDestination);
        g.setOpacity (1.0f);
        if (themeId == 4)
        {
            g.setColour (magenta.withAlpha (0.08f + pulse * 0.05f));
            g.fillAll();
            g.setColour (cyan.withAlpha (0.05f));
            g.fillRect (0.f, H * 0.5f + my * 10.f, W, H * 0.5f);
        }
    }

    // ========== L3 mid: plasma ribbons (parallax 0.55) ==========
    {
        for (int r = 0; r < 6; ++r)
        {
            juce::Path ribbon;
            const float yBase = H * (0.12f + r * 0.14f) + my * (12.f + r * 3.f);
            const float amp = 30.f + r * 11.f + pulse * 24.f;
            const float speed = 0.6f + r * 0.13f;
            const float phase = t * speed + r * 1.5f;
            bool first = true;
            for (float x = -20.f; x <= W + 20.f; x += 6.f)
            {
                float y = yBase
                    + std::sin ((x + mx * 40.f) * 0.011f + phase) * amp
                    + std::sin ((x + mx * 20.f) * 0.028f - phase * 1.4f) * (amp * 0.45f);
                if (first) { ribbon.startNewSubPath (x, y); first = false; }
                else ribbon.lineTo (x, y);
            }
            juce::Colour rc = (r % 3 == 0) ? cyan : ((r % 3 == 1) ? magenta : lime);
            g.setColour (rc.withAlpha (0.11f + pulse * 0.09f));
            g.strokePath (ribbon, juce::PathStrokeType (2.5f + r * 0.4f));
            g.setColour (rc.withAlpha (0.04f));
            g.strokePath (ribbon, juce::PathStrokeType (12.f));
        }
    }

    // ========== L4 mid-near: grid + scan (parallax 0.7) ==========
    {
        const float spacing = 30.f;
        const float ox = std::fmod (t * 22.f + mx * 50.f, spacing);
        const float oy = std::fmod (t * 16.f + my * 40.f, spacing);
        g.setColour (cyan.withAlpha (0.07f + pulse * 0.05f));
        for (float x = -spacing + ox; x < W + spacing; x += spacing)
            g.drawVerticalLine ((int) x, 0.f, H);
        for (float y = -spacing + oy; y < H + spacing; y += spacing)
            g.drawHorizontalLine ((int) y, 0.f, W);

        float scanY = std::fmod (t * 70.f, H + 50.f) - 25.f + my * 15.f;
        juce::ColourGradient scan (cyan.withAlpha (0.f), 0, scanY - 20.f,
                                   cyan.withAlpha (0.22f + pulse * 0.18f), 0, scanY, false);
        g.setGradientFill (scan);
        g.fillRect (0.f, scanY - 20.f, W, 40.f);
    }

    // ========== L5 near: floating orbs (parallax 0.9) ==========
    for (int i = 0; i < 32; ++i)
    {
        float speed = 14.f + (i % 9) * 4.f;
        float px = std::fmod (t * speed * 0.65f + i * 47.f, W + 80.f) - 40.f + mx * 55.f;
        float py = std::fmod (t * (speed * 0.4f) + i * 71.f, H + 80.f) - 40.f + my * 45.f;
        px += 22.f * std::sin (t * 1.1f + i * 0.7f);
        py += 16.f * std::cos (t * 0.85f + i * 0.5f);
        float sz = 7.f + (i % 5) * 4.5f + pulse * 10.f;
        juce::Colour oc = (i % 4 == 0) ? magenta : ((i % 4 == 1) ? cyan : ((i % 4 == 2) ? gold : lime));
        g.setColour (oc.withAlpha (0.07f + pulse * 0.1f));
        g.fillEllipse (px - sz, py - sz, sz * 3.2f, sz * 3.2f);
        g.setColour (oc.withAlpha (0.28f + pulse * 0.35f));
        g.fillEllipse (px, py, sz, sz);
        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.fillEllipse (px + sz * 0.22f, py + sz * 0.15f, sz * 0.28f, sz * 0.28f);
    }

    // ========== L6 nearest: sparks + cross flares ==========
    for (int i = 0; i < 100; ++i)
    {
        float px = std::fmod (t * (12.f + (i % 13)) + i * 41.f + mx * 70.f, W);
        float py = std::fmod (t * (9.f + (i % 7)) + i * 59.f + my * 60.f, H);
        juce::Colour pc = (i % 4 == 0) ? cyan : ((i % 4 == 1) ? magenta : ((i % 4 == 2) ? gold : lime));
        float sz = 1.1f + (i % 3) * 0.8f + pulse * 1.8f;
        g.setColour (pc.withAlpha (0.35f + pulse * 0.4f));
        g.fillEllipse (px, py, sz, sz);
        if (i % 7 == 0)
        {
            g.setColour (pc.withAlpha (0.12f));
            g.drawLine (px - 8.f, py, px + 8.f, py, 1.f);
            g.drawLine (px, py - 8.f, px, py + 8.f, 1.f);
        }
    }

    // ========== corner energy arcs (react to mouse) ==========
    {
        juce::Path arc;
        float r = 90.f + pulse * 50.f + std::abs (mx) * 30.f;
        arc.addCentredArc (mx * 20.f, my * 15.f, r + 25.f * std::sin (t), r, 0, 0.15f, 1.5f, true);
        g.setColour (magenta.withAlpha (0.22f + pulse * 0.15f));
        g.strokePath (arc, juce::PathStrokeType (2.5f));
        juce::Path arc2;
        arc2.addCentredArc (W + mx * 15.f, H + my * 12.f, r * 1.25f, r * 1.1f, 0,
                            3.4f + 0.35f * std::sin (t * 0.7f), 5.3f, true);
        g.setColour (cyan.withAlpha (0.2f + pulse * 0.12f));
        g.strokePath (arc2, juce::PathStrokeType (2.2f));
    }

    // ========== brand + hero card (UI layer — almost no parallax) ==========
    g.setColour (cyan);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 16, 4, 320, 24, juce::Justification::centredLeft);
    g.setColour (gold.withAlpha (0.55f + pulse * 0.45f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ISATIS", W - 100.f, H - 90.f, 90.f, 16.f, juce::Justification::centredRight);

    auto left = juce::Rectangle<float> (8.f, 48.f, 210.f, 280.f);
    {
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.6f));
        g.fillRoundedRectangle (left, 12.f);
        g.setColour (cyan.withAlpha (0.55f + pulse * 0.3f));
        g.drawRoundedRectangle (left, 12.f, 1.6f);

        if (heroImg.isValid())
        {
            auto imgArea = left.reduced (8.f, 8.f).withHeight (200.f);
            float z = 1.f + 0.04f * std::sin (t * 0.55f);
            auto src = heroImg.getBounds().toFloat();
            float sw = src.getWidth() / z, sh = src.getHeight() / z;
            float sx = (src.getWidth() - sw) * (0.5f + 0.12f * std::sin (t * 0.28f) + mx * 0.05f);
            float sy = (src.getHeight() - sh) * (0.5f + 0.1f * std::cos (t * 0.22f) + my * 0.04f);
            g.drawImage (heroImg,
                         (int) imgArea.getX(), (int) imgArea.getY(), (int) imgArea.getWidth(), (int) imgArea.getHeight(),
                         (int) sx, (int) sy, (int) sw, (int) sh);
        }

        g.setColour (magenta);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText (juce::CharPointer_UTF8 ("\xd8\xb3\xd8\xa7\xd9\x84\xda\xa9"),
                    juce::Rectangle<float> (left.getX(), left.getY() + 206.f, left.getWidth(), 18.f),
                    juce::Justification::centred);
        g.setColour (cyan);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("SALEK", juce::Rectangle<float> (left.getX(), left.getY() + 222.f, left.getWidth(), 14.f),
                    juce::Justification::centred);
        g.setColour (gold.withAlpha (0.7f));
        g.setFont (juce::FontOptions (9.0f));
        g.drawText ("PERSIAN CYBER SONIC CORE",
                    juce::Rectangle<float> (left.getX(), left.getY() + 238.f, left.getWidth(), 12.f),
                    juce::Justification::centred);
    }
}

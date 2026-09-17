// SALEK HIGHTECH — optimized parallax + 3D light + Iranian geometric motifs
void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int themeId = themeBox.getSelectedId();
    const float W = (float) getWidth();
    const float H = (float) getHeight();
    const float t = animPhase;
    const float peak  = processor.getOutputPeak();
    const float pulse = juce::jlimit (0.0f, 1.0f, peak * 5.0f);
    const auto mouse = getMouseXYRelative();
    const float mx = W > 1.f ? ((float) mouse.x / W - 0.5f) : 0.f;
    const float my = H > 1.f ? ((float) mouse.y / H - 0.5f) : 0.f;
    const float morph = 0.5f + 0.5f * std::sin (t * 0.35f);

    auto lerpCol = [] (juce::Colour a, juce::Colour b, float x) {
        return a.interpolatedWith (b, juce::jlimit (0.f, 1.f, x));
    };

    juce::Colour cyan, magenta, gold, purple, lime;
    if (themeId == 2) {
        cyan = lerpCol (juce::Colour (0xffb8ff00), juce::Colour (0xff39ff14), morph);
        magenta = lerpCol (juce::Colour (0xffff6b00), juce::Colour (0xffffaa00), morph);
        gold = juce::Colour (0xffffee00); purple = juce::Colour (0xff88ff00); lime = juce::Colour (0xff39ff14);
    } else if (themeId == 3) {
        cyan = lerpCol (juce::Colour (0xffff00dd), juce::Colour (0xffff66ff), morph);
        magenta = lerpCol (juce::Colour (0xff00ffff), juce::Colour (0xff7c4dff), morph);
        gold = juce::Colour (0xffff66ff); purple = juce::Colour (0xffaa00ff); lime = juce::Colour (0xffff2d9b);
    } else if (themeId == 4) {
        cyan = lerpCol (juce::Colour (0xff7ef9e0), juce::Colour (0xffffb0d0), morph);
        magenta = lerpCol (juce::Colour (0xffff2d9b), juce::Colour (0xffff80c0), 1.f - morph);
        gold = juce::Colour (0xffff66aa); purple = juce::Colour (0xffff80c0); lime = juce::Colour (0xffb8ff40);
    } else {
        cyan = lerpCol (juce::Colour (0xff00e8ff), juce::Colour (0xff7c4dff), morph);
        magenta = lerpCol (juce::Colour (0xffff2d9b), juce::Colour (0xffffd700), morph);
        gold = juce::Colour (0xffffd700); purple = juce::Colour (0xffb44dff); lime = juce::Colour (0xff39ff14);
    }

    // Light source follows mouse (3D lighting key)
    const float lx = W * (0.5f + mx * 0.6f);
    const float ly = H * (0.35f + my * 0.4f);

    // ---- L0 far gradient ----
    {
        juce::Colour c0, c1;
        if (themeId == 4)      { c0 = juce::Colour (0xff1a0c18); c1 = juce::Colour (0xff3a1830); }
        else if (themeId == 2) { c0 = juce::Colour (0xff040a02); c1 = juce::Colour (0xff1a2e08); }
        else if (themeId == 3) { c0 = juce::Colour (0xff0c0210); c1 = juce::Colour (0xff2a0838); }
        else                   { c0 = juce::Colour (0xff03010e); c1 = juce::Colour (0xff12062a); }
        float gx = lx * 0.3f + W * 0.3f, gy = ly * 0.3f + H * 0.2f;
        juce::Colour mid = c1.interpolatedWith (purple.darker (0.2f), morph * 0.35f + pulse * 0.15f);
        juce::ColourGradient bg (c0, gx - W * 0.3f, gy - H * 0.3f, mid, gx + W * 0.5f, gy + H * 0.5f, true);
        g.setGradientFill (bg);
        g.fillAll();
    }

    // ---- Iranian girih motif (8-point star) — far layer ----
    {
        auto drawStar = [&] (float cx, float cy, float r, juce::Colour col, float alpha)
        {
            juce::Path star;
            for (int i = 0; i < 8; ++i)
            {
                float a0 = (float) i * juce::MathConstants<float>::twoPi / 8.f - juce::MathConstants<float>::halfPi;
                float a1 = a0 + juce::MathConstants<float>::pi / 8.f;
                float x0 = cx + std::cos (a0) * r;
                float y0 = cy + std::sin (a0) * r;
                float x1 = cx + std::cos (a1) * r * 0.45f;
                float y1 = cy + std::sin (a1) * r * 0.45f;
                if (i == 0) star.startNewSubPath (x0, y0);
                else star.lineTo (x0, y0);
                star.lineTo (x1, y1);
            }
            star.closeSubPath();
            g.setColour (col.withAlpha (alpha));
            g.strokePath (star, juce::PathStrokeType (1.2f));
        };
        float rot = t * 0.15f;
        for (int k = 0; k < 3; ++k)
        {
            float cx = W * (0.2f + k * 0.3f) + mx * 10.f + 20.f * std::sin (t * 0.2f + k);
            float cy = H * (0.25f + k * 0.2f) + my * 8.f;
            float r = 40.f + k * 18.f + pulse * 12.f;
            // rotate by drawing at offset angles via phase in draw - simple scale pulse
            drawStar (cx, cy, r * (1.f + 0.05f * std::sin (t + k)), gold, 0.08f + pulse * 0.06f);
            drawStar (cx, cy, r * 0.55f, cyan, 0.06f);
        }
        (void) rot;
    }

    // ---- soft hero wash ----
    if (heroImg.isValid())
    {
        float breathe = 1.f + 0.04f * std::sin (t * 0.6f);
        float panX = 35.f * std::sin (t * 0.18f) + mx * 24.f;
        float panY = 20.f * std::cos (t * 0.14f) + my * 18.f;
        auto dest = juce::Rectangle<float> (-40.f + panX, -30.f + panY, W * breathe + 80.f, H * breathe + 60.f);
        g.setOpacity (0.14f + pulse * 0.08f);
        g.drawImage (heroImg, dest, juce::RectanglePlacement::fillDestination);
        g.setOpacity (1.f);
        if (themeId == 4)
        {
            g.setColour (magenta.withAlpha (0.07f));
            g.fillAll();
        }
    }

    // ---- plasma ribbons (optimized: 4 not 6) ----
    for (int r = 0; r < 4; ++r)
    {
        juce::Path ribbon;
        float yBase = H * (0.18f + r * 0.18f) + my * (10.f + r * 2.f);
        float amp = 28.f + r * 12.f + pulse * 20.f;
        float phase = t * (0.55f + r * 0.12f) + r * 1.4f;
        bool first = true;
        for (float x = 0; x <= W; x += 10.f)
        {
            float y = yBase + std::sin ((x + mx * 30.f) * 0.012f + phase) * amp
                            + std::sin ((x) * 0.03f - phase * 1.2f) * amp * 0.4f;
            if (first) { ribbon.startNewSubPath (x, y); first = false; }
            else ribbon.lineTo (x, y);
        }
        juce::Colour rc = (r & 1) ? cyan : magenta;
        g.setColour (rc.withAlpha (0.12f + pulse * 0.08f));
        g.strokePath (ribbon, juce::PathStrokeType (2.8f));
    }

    // ---- 3D light bloom at mouse ----
    {
        float br = 120.f + pulse * 80.f;
        juce::ColourGradient bloom (juce::Colours::white.withAlpha (0.12f + pulse * 0.1f), lx, ly,
                                    cyan.withAlpha (0.f), lx + br, ly, true);
        g.setGradientFill (bloom);
        g.fillEllipse (lx - br, ly - br, br * 2.f, br * 2.f);
        g.setColour (magenta.withAlpha (0.08f + pulse * 0.06f));
        g.fillEllipse (lx - br * 0.4f, ly - br * 0.4f, br * 0.8f, br * 0.8f);
    }

    // ---- orbs with 3D specular (optimized count) ----
    for (int i = 0; i < 20; ++i)
    {
        float speed = 12.f + (i % 7) * 3.5f;
        float px = std::fmod (t * speed * 0.6f + i * 47.f, W + 60.f) - 30.f + mx * 40.f;
        float py = std::fmod (t * speed * 0.35f + i * 71.f, H + 60.f) - 30.f + my * 32.f;
        px += 16.f * std::sin (t + i);
        py += 12.f * std::cos (t * 0.8f + i * 0.5f);
        float sz = 8.f + (i % 4) * 4.f + pulse * 8.f;
        juce::Colour oc = (i % 3 == 0) ? magenta : ((i % 3 == 1) ? cyan : gold);
        // volume shade
        g.setColour (oc.darker (0.6f).withAlpha (0.15f));
        g.fillEllipse (px + 3.f, py + 4.f, sz, sz);
        g.setColour (oc.withAlpha (0.3f + pulse * 0.25f));
        g.fillEllipse (px, py, sz, sz);
        // specular highlight toward light
        float hx = px + sz * 0.25f + (lx - px) * 0.02f;
        float hy = py + sz * 0.2f + (ly - py) * 0.02f;
        g.setColour (juce::Colours::white.withAlpha (0.45f));
        g.fillEllipse (hx, hy, sz * 0.28f, sz * 0.28f);
    }

    // ---- near sparks ----
    for (int i = 0; i < 60; ++i)
    {
        float px = std::fmod (t * (11.f + i % 9) + i * 41.f + mx * 55.f, W);
        float py = std::fmod (t * (8.f + i % 5) + i * 59.f + my * 45.f, H);
        juce::Colour pc = (i % 3 == 0) ? cyan : ((i % 3 == 1) ? magenta : lime);
        g.setColour (pc.withAlpha (0.35f + pulse * 0.3f));
        g.fillEllipse (px, py, 1.5f + pulse, 1.5f + pulse);
    }

    // ---- Persian toranj medallion (corner) ----
    {
        float cx = W - 70.f + mx * 6.f, cy = 70.f + my * 4.f;
        float R = 36.f + pulse * 8.f;
        g.setColour (gold.withAlpha (0.2f + pulse * 0.15f));
        for (int ring = 0; ring < 3; ++ring)
        {
            float rr = R * (1.f - ring * 0.28f);
            g.drawEllipse (cx - rr, cy - rr, rr * 2.f, rr * 2.f, 1.2f);
        }
        // 12-petal toranj
        juce::Path petals;
        for (int i = 0; i < 12; ++i)
        {
            float a = (float) i * juce::MathConstants<float>::twoPi / 12.f + t * 0.2f;
            float x = cx + std::cos (a) * R * 0.7f;
            float y = cy + std::sin (a) * R * 0.7f;
            if (i == 0) petals.startNewSubPath (x, y);
            else petals.lineTo (x, y);
        }
        petals.closeSubPath();
        g.setColour (magenta.withAlpha (0.18f));
        g.strokePath (petals, juce::PathStrokeType (1.4f));
        g.setColour (cyan.withAlpha (0.25f));
        g.fillEllipse (cx - 5.f, cy - 5.f, 10.f, 10.f);
    }

    // ---- brand ----
    g.setColour (cyan);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("SALEK HIGHTECH", 20, 4, 340, 24, juce::Justification::centredLeft);
    g.setColour (gold.withAlpha (0.6f + pulse * 0.4f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ISATIS", W - 100.f, H - 90.f, 90.f, 16.f, juce::Justification::centredRight);

    // ---- hero card with 3D edge light ----
    auto left = juce::Rectangle<float> (8.f, 48.f, 210.f, 280.f);
    {
        // drop shadow
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (left.translated (4.f, 6.f), 12.f);
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.72f));
        g.fillRoundedRectangle (left, 12.f);
        // edge light toward mouse
        float edgeA = 0.4f + pulse * 0.3f + std::abs (mx) * 0.2f;
        g.setColour (cyan.withAlpha (edgeA));
        g.drawRoundedRectangle (left, 12.f, 1.8f);
        g.setColour (magenta.withAlpha (0.15f));
        g.drawRoundedRectangle (left.reduced (2.f), 10.f, 1.f);

        if (heroImg.isValid())
        {
            auto imgArea = left.reduced (8.f, 8.f).withHeight (200.f);
            float z = 1.f + 0.035f * std::sin (t * 0.5f);
            auto src = heroImg.getBounds().toFloat();
            float sw = src.getWidth() / z, sh = src.getHeight() / z;
            float sx = (src.getWidth() - sw) * (0.5f + 0.1f * std::sin (t * 0.25f) + mx * 0.04f);
            float sy = (src.getHeight() - sh) * (0.5f + 0.08f * std::cos (t * 0.2f) + my * 0.03f);
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
        g.setColour (gold.withAlpha (0.75f));
        g.setFont (juce::FontOptions (9.0f));
        g.drawText ("PERSIAN CYBER SONIC CORE",
                    juce::Rectangle<float> (left.getX(), left.getY() + 238.f, left.getWidth(), 12.f),
                    juce::Justification::centred);
    }
}

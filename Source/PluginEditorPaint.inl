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
        // Mockup cyber: hot pink + electric cyan (SALEK reference art)
        cyan = lerpCol (juce::Colour (0xff00f0ff), juce::Colour (0xff7cf0ff), morph);
        magenta = lerpCol (juce::Colour (0xffff2ec8), juce::Colour (0xffff66e0), morph);
        gold = juce::Colour (0xffffd700); purple = juce::Colour (0xffc44dff); lime = juce::Colour (0xff39ff14);
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
        else                   { c0 = juce::Colour (0xff0a0218); c1 = juce::Colour (0xff2a0a3a); } // deep cyber night
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

    // ---- background art (2 versions) — almost static, subtle mouse parallax only ----
    if (heroImg.isValid())
    {
        // very small pan with mouse only (no auto animation)
        const float panX = mx * 8.f;
        const float panY = my * 6.f;
        auto dest = juce::Rectangle<float> (-12.f + panX, -10.f + panY, W + 24.f, H + 20.f);
        g.setOpacity (0.90f); // BG opacity 0.9 — art clearly visible
        g.drawImage (heroImg, dest, juce::RectanglePlacement::fillDestination);
        g.setOpacity (1.f);
        // soft dark veil — UI readability over detailed art
        {
            juce::ColourGradient veil (juce::Colours::black.withAlpha (0.18f), W * 0.5f, H,
                                      juce::Colours::transparentBlack, W * 0.5f, H * 0.28f, false);
            g.setGradientFill (veil);
            g.fillAll();
            g.setColour (juce::Colours::black.withAlpha (0.06f));
            g.fillAll();
        }
    }

    // ---- plasma ribbons (optimized: 4 not 6) ----
    for (int r = 0; r < 4; ++r)
    {
        juce::Path ribbon;
        float yBase = H * (0.18f + r * 0.18f) + my * (10.f + r * 2.f);
        float amp = 34.f + r * 14.f + pulse * 28.f;
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
    for (int i = 0; i < 16; ++i)
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
    for (int i = 0; i < 40; ++i)
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
        g.setColour (magenta.withAlpha (0.32f));
        g.strokePath (petals, juce::PathStrokeType (1.4f));
        g.setColour (cyan.withAlpha (0.25f));
        g.fillEllipse (cx - 5.f, cy - 5.f, 10.f, 10.f);
    }

    // ---- brand ----
    g.setColour (cyan);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    // ---- cyber city horizon glow (mockup) ----
    {
        juce::ColourGradient horizon (magenta.withAlpha (0.18f + pulse * 0.08f), W * 0.5f, H * 0.72f,
                                     juce::Colours::transparentBlack, W * 0.5f, H * 0.45f, false);
        g.setGradientFill (horizon);
        g.fillRect (0.f, H * 0.45f, W, H * 0.4f);
        juce::ColourGradient cyanHorizon (cyan.withAlpha (0.08f), W * 0.7f, H * 0.85f,
                                         juce::Colours::transparentBlack, W * 0.7f, H * 0.55f, false);
        g.setGradientFill (cyanHorizon);
        g.fillRect (0.f, H * 0.55f, W, H * 0.35f);
    }

        g.drawText ("SALEK HIGHTECH", 20, 4, 340, 24, juce::Justification::centredLeft);
    g.setColour (gold.withAlpha (0.6f + pulse * 0.4f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ISATIS", W - 100.f, H - 90.f, 90.f, 16.f, juce::Justification::centredRight);

    // ---- left panel: only show branding card when preset sidebar is collapsed ----
    if (presetCollapsed)
    {
        auto left = juce::Rectangle<float> (8.f, 48.f, 28.f, 120.f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (left.translated (2.f, 3.f), 8.f);
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.45f));
        g.fillRoundedRectangle (left, 8.f);
        g.setColour (cyan.withAlpha (0.5f));
        g.drawRoundedRectangle (left, 8.f, 1.2f);
        g.setColour (magenta);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText ("S", left, juce::Justification::centred);
    }
    // when preset is open, no solid card — background art stays fully visible behind the list


        // Serum-like section frames on MAIN (osc / filter / env)
        if (tabs.getCurrentTabIndex() == 0)
        {
            auto frame = [] (juce::Graphics& g, juce::Component& c, juce::Colour col, const char* title)
            {
                if (! c.isVisible() || c.getWidth() < 8) return;
                auto b = c.getBounds().toFloat();
                // convert to editor coords
                auto topLeft = c.getParentComponent()->getLocalBounds(); // wrong
                juce::ignoreUnused (topLeft);
            };
            juce::ignoreUnused (frame);
            auto drawFrame = [&] (juce::Component& c, juce::Colour col, const juce::String& title)
            {
                if (c.getWidth() < 10) return;
                auto r = getLocalArea (&c, c.getLocalBounds()).toFloat().reduced (2.f);
                g.setColour (juce::Colour (0xff0a0614).withAlpha (0.22f));
                g.fillRoundedRectangle (r, 10.f);
                g.setColour (col.withAlpha (0.45f + pulse * 0.15f));
                g.drawRoundedRectangle (r, 10.f, 1.4f);
                g.setColour (col.withAlpha (0.85f));
                g.setFont (juce::FontOptions (10.f, juce::Font::bold));
                g.drawText (title, r.removeFromTop (14).reduced (8, 0), juce::Justification::centredLeft);
            };
            drawFrame (oscTab, cyan, "OSCILLATORS");
            drawFrame (filterTab, magenta, "FILTER");
            drawFrame (envTab, gold, "AMP ENV");
        }

}

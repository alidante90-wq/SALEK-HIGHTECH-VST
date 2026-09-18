int SalekHightechAudioProcessorEditor::getNumRows() { return presetRows.size(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);

    // ===== Category header with icon badge =====
    if (pr.isHeader)
    {
        auto cat = pr.category.toUpperCase();
        juce::Colour catCol (0xffff2ec8);
        juce::String icon = "◆";
        if (cat.contains ("BASS"))      { catCol = juce::Colour (0xff66ff99); icon = "B"; }
        else if (cat.contains ("KICK")) { catCol = juce::Colour (0xffffab40); icon = "K"; }
        else if (cat.contains ("LEAD")) { catCol = juce::Colour (0xff00e8ff); icon = "L"; }
        else if (cat.contains ("PAD"))  { catCol = juce::Colour (0xffb388ff); icon = "P"; }
        else if (cat.contains ("ACID")) { catCol = juce::Colour (0xffffcc44); icon = "A"; }
        else if (cat.contains ("FM"))   { catCol = juce::Colour (0xffff66cc); icon = "F"; }
        else if (cat.contains ("FX"))   { catCol = juce::Colour (0xffff5252); icon = "X"; }
        else if (cat.contains ("ARP"))  { catCol = juce::Colour (0xff69f0ae); icon = "↻"; }
        else if (cat.contains ("SALEK") || cat.contains ("GITI")) { catCol = juce::Colour (0xffffd700); icon = "★"; }
        else if (cat.contains ("USER")) { catCol = juce::Colour (0xffffaa00); icon = "U"; }
        else if (cat.contains ("RETRO")){ catCol = juce::Colour (0xffa0a0ff); icon = "R"; }
        else if (cat.contains ("OTHER")){ catCol = juce::Colour (0xff90a4ae); icon = "·"; }

        g.setColour (juce::Colour (0xdd0a0618));
        g.fillRoundedRectangle (1.f, 1.f, (float) width - 2.f, (float) height - 2.f, 6.f);
        g.setColour (catCol.withAlpha (0.55f));
        g.drawRoundedRectangle (1.5f, 1.5f, (float) width - 3.f, (float) height - 3.f, 6.f, 1.2f);

        // circular icon badge
        auto badge = juce::Rectangle<float> (6.f, (float) height * 0.15f, (float) height * 0.7f, (float) height * 0.7f);
        g.setColour (catCol.withAlpha (0.25f));
        g.fillEllipse (badge);
        g.setColour (catCol);
        g.drawEllipse (badge, 1.2f);
        g.setFont (juce::FontOptions (badge.getHeight() * 0.55f, juce::Font::bold));
        g.drawText (icon, badge.toNearestInt(), juce::Justification::centred);

        const bool folded = collapsedCats.contains (pr.category);
        g.setColour (catCol.brighter (0.2f));
        g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
        juce::String mark = folded ? juce::String (juce::CharPointer_UTF8 ("\xe2\x96\xb8 "))
                                   : juce::String (juce::CharPointer_UTF8 ("\xe2\x96\xbe "));
        // use simple ASCII for MSVC reliability
        mark = folded ? "> " : "v ";
        g.drawText (mark + pr.label, (int) badge.getRight() + 6, 0, width - (int) badge.getRight() - 10, height,
                    juce::Justification::centredLeft);
        return;
    }

    // ===== Preset row (mockup: cyan selected bar) =====
    if (selected)
    {
        g.setColour (juce::Colour (0xbb0a2848)); // translucent selected
        g.fillRect (0, 0, width, height);
        g.setColour (juce::Colour (0xff00e8ff));
        g.fillRect (0, 0, 4, height);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
        g.drawRect (0, 0, width, height, 1);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.15f));
        g.fillRect (4, 1, width - 5, height - 2);
    }
    else
    {
        g.setColour (juce::Colour (0x880a0614)); // see-through so BG shows
        g.fillRect (0, 0, width, height);
        if ((row & 1) == 0)
        {
            g.setColour (juce::Colour (0x5512081c));
            g.fillRect (0, 0, width, height);
        }
    }

    juce::Colour tc = selected ? juce::Colour (0xff00f0ff) : juce::Colour (0xffc8c0e8);
    auto fullName = processor.getProgramName (pr.programIndex);
    if (fullName.startsWith ("Acid"))  tc = selected ? tc : juce::Colour (0xffffcc44);
    if (fullName.startsWith ("Bass"))  tc = selected ? tc : juce::Colour (0xff66ff99);
    if (fullName.startsWith ("Lead"))  tc = selected ? tc : juce::Colour (0xff00f0ff);
    if (fullName.startsWith ("Kick"))  tc = selected ? tc : juce::Colour (0xffffab40);
    if (fullName.startsWith ("FM"))    tc = selected ? tc : juce::Colour (0xffff66cc);
    if (fullName.startsWith ("Retro")) tc = selected ? tc : juce::Colour (0xffa0a0ff);
    if (fullName.startsWith ("SALEK")) tc = selected ? tc : juce::Colour (0xffffd700);
    if (fullName.startsWith ("USER"))  tc = selected ? tc : juce::Colour (0xffffaa00);

    g.setColour (tc);
    g.setFont (juce::FontOptions (selected ? 13.0f : 12.2f, selected ? juce::Font::bold : juce::Font::plain));
    g.drawText (pr.label, 14, 0, width - 20, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto pr = presetRows.getReference (row); // copy
    if (pr.isHeader)
    {
        if (collapsedCats.contains (pr.category))
            collapsedCats.removeString (pr.category);
        else
            collapsedCats.add (pr.category);
        rebuildPresetRows();
        return;
    }
    if (pr.programIndex < 0) return;
    processor.setCurrentProgram (pr.programIndex);
    presetLabel.setText (processor.getProgramName (pr.programIndex), juce::dontSendNotification);
}

#include "PluginEditorPaint.inl"

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 4) resized();
    // BPM-synced animation when host provides tempo
    float bpmScale = 1.f;
    if (auto* ph = processor.getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (pos->getBpm())
                bpmScale = (float) (*pos->getBpm() / 120.0);
    }
    animPhase += 0.022f * juce::jlimit (0.5f, 1.8f, bpmScale);

    // Live FX monitor levels + multi-band GR (OTT style)
    auto g = [&](const char* id, float d=0.f) -> float {
        if (auto* p = processor.getAPVTS().getRawParameterValue (id)) return p->load();
        return d;
    };
    if (fxMonitors.size() >= 8)
    {
        fxMonitors[0]->setLevel (g ("chorus_mix"));
        fxMonitors[1]->setLevel (g ("delay_mix"));
        fxMonitors[2]->setLevel (g ("reverb_mix"));
        fxMonitors[3]->setLevel (g ("bassify"));
        fxMonitors[4]->setLevel (g ("comp_mix"));
        fxMonitors[4]->setThresholdNorm (juce::jmap (g ("comp_threshold", -12.f), -40.f, 0.f, 0.f, 1.f));
        fxMonitors[4]->setBandGR (
            processor.getCompressor().getBandGR (0),
            processor.getCompressor().getBandGR (1),
            processor.getCompressor().getBandGR (2));
        fxMonitors[5]->setLevel (juce::jmax (std::abs (g ("eq_low")), std::abs (g ("eq_mid")), std::abs (g ("eq_high"))) / 12.f);
        fxMonitors[5]->setEqBands (g ("eq_low") / 12.f, g ("eq_mid") / 12.f, g ("eq_high") / 12.f);
        fxMonitors[6]->setLevel (g ("phaser_mix"));
        fxMonitors[7]->setLevel (g ("dist_mix"));
    }
    repaint();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto full = getLocalBounds();

    auto place = [this] (juce::Rectangle<int> area, auto& arr, int start, int count, int cols, int maxCell = 0)
    {
        if (count <= 0 || cols <= 0) return;
        const int rows = (count + cols - 1) / cols;
        const int cw = juce::jmax (1, area.getWidth() / cols);
        const int rh = juce::jmax (1, area.getHeight() / rows);
        for (int i = 0; i < count; ++i)
        {
            const int idx = start + i;
            if (idx < 0 || idx >= (int) arr.size()) break;
            auto* k = arr[(size_t) idx].get();
            const int c = i % cols, r = i / cols;
            auto cell = juce::Rectangle<int> (area.getX() + c * cw, area.getY() + r * rh, cw, rh).reduced (2);
            if (maxCell > 0 && cell.getWidth() > maxCell)
            {
                const int pad = (cell.getWidth() - maxCell) / 2;
                cell = cell.withTrimmedLeft (pad).withTrimmedRight (pad);
            }
            k->name.setBounds (cell.removeFromBottom (14));
            k->name.setJustificationType (juce::Justification::centred);
            k->name.setVisible (true);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, cell.getWidth() - 8), 12);
            k->s.setBounds (cell);
            k->s.setVisible (true);
        }
    };

    {
        auto kb = full.removeFromBottom (68).reduced (2, 1);
        keyboard.setKeyWidth ((float) juce::jmax (10, kb.getWidth() / 52));
        keyboard.setBounds (kb);
    }
    full.removeFromBottom (4);

    // Far-left PRESET column (over background art) — not stretched full height
    {
        const int presetW = presetCollapsed ? 28 : 210;
        auto leftStrip = full.removeFromLeft (presetW);
        const int logoPad = 88; // room for large top logo
        const int maxH = juce::jmax (160, (int) ((leftStrip.getHeight() - logoPad) * 0.55f));
        presetTab.setBounds (leftStrip.getX(), leftStrip.getY() + logoPad, leftStrip.getWidth(), maxH);
        presetTab.toFront (false);
        auto pb = presetTab.getLocalBounds().reduced (2);
        auto top = pb.removeFromTop (22);
        presetToggle.setBounds (top.removeFromLeft (28).reduced (1, 0));
        if (! presetCollapsed)
        {
            prevPreset.setBounds (top.removeFromLeft (20).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (20).reduced (1));
            initBtn.setBounds (top.removeFromLeft (28).reduced (1));
            savePresetBtn.setBounds (top.removeFromLeft (30).reduced (1));
            loadPresetBtn.setBounds (top.removeFromLeft (30).reduced (1));
            bankBtn.setBounds (top.removeFromLeft (34).reduced (1));
            presetLabel.setBounds (pb.removeFromTop (18).reduced (2, 0));
            presetList.setBounds (pb);
            presetList.setOpaque (false);
            presetTab.setOpaque (false);
            presetList.setVisible (true);
            prevPreset.setVisible (true);
            nextPreset.setVisible (true);
            initBtn.setVisible (true);
            savePresetBtn.setVisible (true);
            loadPresetBtn.setVisible (true);
            bankBtn.setVisible (true);
            presetLabel.setVisible (true);
        }
        else
        {
            presetList.setVisible (false);
            prevPreset.setVisible (false);
            nextPreset.setVisible (false);
            initBtn.setVisible (false);
            savePresetBtn.setVisible (false);
            loadPresetBtn.setVisible (false);
            bankBtn.setVisible (false);
            presetLabel.setVisible (false);
        }
    }

    // Header
    auto header = full.removeFromTop (40);
    langToggle.setBounds (header.removeFromRight (36).reduced (2));
    themeBox.setBounds (header.removeFromRight (90).reduced (2));
    bgSwapBtn.setBounds (header.removeFromRight (40).reduced (2));
    if (knobs.size() > 0)
    {
        // master gain / drive if present in header elsewhere — skip
    }
    spectrum.setBounds (header.removeFromRight (80).reduced (2));
    scope.setBounds (header.removeFromRight (100).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (140).reduced (2));

    tabs.setBounds (full);
    tabs.toFront (false);

    // MAIN tab: osc columns | filter | env
    {
        auto b = mainTab.getLocalBounds().reduced (2);
        envTab.setBounds (b.removeFromBottom (120));
        filterTab.setBounds (b.removeFromRight (220));
        oscTab.setBounds (b);

        {
            // 3 columns: monitor + shape + params + unison
            auto oa = oscTab.getLocalBounds().reduced (2);
            const int colW = oa.getWidth() / 3;
            juce::Component* mons[3] = { oscMon1.get(), oscMon2.get(), oscMon3.get() };
            juce::ComboBox* shapes[3] = { &osc1ShapeBox, &osc2ShapeBox, &osc3ShapeBox };
            for (int c = 0; c < 3; ++c)
            {
                auto col = juce::Rectangle<int> (oa.getX() + c * colW, oa.getY(), colW, oa.getHeight()).reduced (3, 2);
                auto monH = juce::jlimit (40, 64, col.getHeight() / 6);
                if (mons[c] != nullptr)
                    mons[c]->setBounds (col.removeFromTop (monH).reduced (1));
                shapes[c]->setBounds (col.removeFromTop (20).reduced (1));
                auto paramArea = col.removeFromTop (juce::jmax (80, col.getHeight() * 2 / 3));
                place (paramArea, knobs, c * 6, 6, 2);
                place (col, knobs, 18 + c * 3, 3, 3);
            }
        }

        {
            auto fr = filterTab.getLocalBounds().reduced (3);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (juce::jlimit (50, 80, fr.getHeight() / 3)).reduced (2));
            auto modeRow = fr.removeFromTop (24);
            filterMode.setBounds (modeRow.removeFromLeft (modeRow.getWidth() / 2).reduced (1));
            filterRouteBox.setBounds (modeRow.reduced (1));
            place (fr, knobs, 27, 4, 2);
        }

        {
            auto er = envTab.getLocalBounds().reduced (2);
            if (adsrDisplay != nullptr)
            {
                auto curve = er.removeFromLeft (juce::jmin (160, er.getWidth() / 4));
                adsrDisplay->setBounds (curve.reduced (2));
            }
            auto adsrKnobs = er.removeFromLeft (juce::jmin (280, er.getWidth() / 2));
            place (adsrKnobs, knobs, 31, 4, 4);
            voiceModeBox.setBounds (er.removeFromTop (22).reduced (1));
            const int extraStart = (int) knobs.size() - 4;
            if (extraStart >= 0)
                place (er, knobs, extraStart, 4, 4);
        }
    }

    {
        auto r = modTab.getLocalBounds().reduced (4);
        // Right rail: FM block (top) + MACRO block (bottom) — fixed cell heights
        auto right = r.removeFromRight (juce::jmin (300, r.getWidth() * 34 / 100));
        auto macroBand = right.removeFromBottom (110); // fixed height: knob+label glued
        auto fmArea = right.reduced (2, 4);

        // FM/PM/RM/AM: 2x3 grid, max cell 92px
        place (fmArea, knobs, 35, 6, 3, 92);

        // MACROS: explicit layout so name is always glued under rotary (no floating labels)
        {
            const int start = 41;
            const int n = 4;
            const int cw = juce::jmax (1, macroBand.getWidth() / n);
            for (int i = 0; i < n; ++i)
            {
                if (start + i >= (int) knobs.size()) break;
                auto* k = knobs[(size_t) (start + i)].get();
                auto cell = juce::Rectangle<int> (macroBand.getX() + i * cw, macroBand.getY(),
                                                  cw, macroBand.getHeight()).reduced (3, 2);
                k->name.setBounds (cell.removeFromBottom (16));
                k->name.setJustificationType (juce::Justification::centred);
                k->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
                k->name.setVisible (true);
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, cw - 10), 12);
                k->s.setBounds (cell);
                k->s.setVisible (true);
            }
        }

        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.reduced (2));
    }

    {
        auto r = lfoTab.getLocalBounds().reduced (6);
        // Top: live LFO scopes
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (56).reduced (1));

        // Shape bank row
        auto presetRow = r.removeFromTop (24);
        const int bw = presetRow.getWidth() / 12;
        juce::TextButton* presets[] = {
            &lfoPresetSine, &lfoPresetTri, &lfoPresetSaw, &lfoPresetSqr,
            &lfoPresetPulse, &lfoPresetExp, &lfoPresetLog, &lfoPresetBell,
            &lfoPresetWob, &lfoPresetChaos, &lfoPresetGate, &lfoPresetCustom
        };
        for (auto* b : presets)
            b->setBounds (presetRow.removeFromLeft (bw).reduced (1));

        // Wave select + copy/slots
        auto waveRow = r.removeFromTop (24);
        lfo1WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 3).reduced (2));
        lfo2WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 2).reduced (2));
        lfo3WaveBox.setBounds (waveRow.reduced (2));

        // 32-shape bank selector
        auto bankRow = r.removeFromTop (24);
        lfoShapeBank.setBounds (bankRow.reduced (2));

        auto copyRow = r.removeFromTop (22);
        const int cw = copyRow.getWidth() / 9;
        lfoCopyTo1.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoCopyTo2.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoCopyTo3.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveA.setBounds   (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadA.setBounds   (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveB.setBounds   (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadB.setBounds   (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveC.setBounds   (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadC.setBounds   (copyRow.reduced (1));

        // Knobs at bottom, BIG shape editor takes remaining (Serum 2 style)
        auto knobArea = r.removeFromBottom (88);
        place (knobArea, knobs, 45, 6, 6);
        lfoShapeEditor.setBounds (r.reduced (2));
    }

    #include "PluginEditorFullB_FxLayout.inl"

    {
        auto r = magicTab.getLocalBounds().reduced (6);
        auto rail = r.removeFromRight (150);
        magicHint.setBounds (rail.removeFromBottom (36).reduced (2));
        const int bh = juce::jmax (28, rail.getHeight() / 5);
        magicLoopBtn.setBounds (rail.removeFromTop (bh).reduced (3));
        magicGlitchBtn.setBounds (rail.removeFromTop (bh).reduced (3));
        magicFlangeBtn.setBounds (rail.removeFromTop (bh).reduced (3));
        magicPsychBtn.setBounds (rail.removeFromTop (bh).reduced (3));
        magicHold.setBounds (rail.reduced (3));
        if (magicPad != nullptr)
        {
            magicPad->setBounds (r.reduced (4));
            magicPad->setAudioPeak (processor.getOutputPeak());
        }
    }

    {
        // SEQ: ARP/SEQ toggles + rate knobs (indices: last 7 = 3 seq + 4 env extras)
        auto bounds = seqTab.getLocalBounds().reduced (6);
        auto top = bounds.removeFromTop (88);
        auto toggles = top.removeFromLeft (120);
        arpOn.setBounds (toggles.removeFromTop (36).reduced (2));
        seqOn.setBounds (toggles.removeFromTop (36).reduced (2));
        const int seqKnobStart = juce::jmax (0, (int) knobs.size() - 7);
        place (top.reduced (4), knobs, seqKnobStart, 3, 3);
        bounds.removeFromTop (4);
        if (stepGrid != nullptr)
            stepGrid->setBounds (bounds);
    }
}

void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    // 2 official backgrounds from concept art
    if (! lianImg.isValid()) lianImg = SalekAssets::loadBgIsatis();  // BG1 man+cat
    if (! cyanImg.isValid()) cyanImg = SalekAssets::loadBgSalek();   // BG2 girl red city
    if (! faceImg.isValid()) faceImg = SalekAssets::loadFace();

    juce::Colour accent, accent2, panelBg, labelBg, labelTx;
    if (id == 2) // ACID
    {
        heroImg = lianImg.isValid() ? lianImg : cyanImg;
        heroIndex = 0;
        accent  = juce::Colour (0xffc0ff00);
        accent2 = juce::Colour (0xffff6b00);
        panelBg = juce::Colour (0xff0a1204);
        labelBg = juce::Colour (0xff1a2a08);
        labelTx = juce::Colour (0xffc0ff00);
    }
    else if (id == 3) // NEON → BG2 girl
    {
        heroImg = cyanImg.isValid() ? cyanImg : lianImg;
        heroIndex = 1;
        accent  = juce::Colour (0xffff2d9b);
        accent2 = juce::Colour (0xff00e8ff);
        panelBg = juce::Colour (0xff12061a);
        labelBg = juce::Colour (0xff2a0a30);
        labelTx = juce::Colour (0xffff66cc);
    }
    else if (id == 4) // PATINA → BG2
    {
        heroImg = cyanImg.isValid() ? cyanImg : lianImg;
        heroIndex = 1;
        accent  = juce::Colour (0xffff2d9b);
        accent2 = juce::Colour (0xff39ff14);
        panelBg = juce::Colour (0xff1a0f24);
        labelBg = juce::Colour (0xff2a1838);
        labelTx = juce::Colour (0xffff66aa);
    }
    else // CYBER → BG1 ISATIS
    {
        heroImg = lianImg.isValid() ? lianImg : cyanImg;
        heroIndex = 0;
        accent  = juce::Colour (0xff00e8ff);
        accent2 = juce::Colour (0xffff2ec8);
        panelBg = juce::Colour (0xff0a0614);
        labelBg = juce::Colour (0xff1a0a30);
        labelTx = juce::Colour (0xff00e8ff);
    }

    tabs.setColour (juce::TabbedComponent::backgroundColourId, juce::Colours::transparentBlack);
    tabs.setColour (juce::TabbedComponent::outlineColourId, accent.withAlpha (0.35f));
    for (int i = 0; i < fxSectionLabels.size(); ++i)
    {
        fxSectionLabels[i]->setColour (juce::Label::textColourId, labelTx);
        fxSectionLabels[i]->setColour (juce::Label::backgroundColourId, labelBg);
    }
    presetLabel.setColour (juce::Label::textColourId, accent);
    title.setColour (juce::Label::textColourId, accent);
    tagline.setColour (juce::Label::textColourId, accent2);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto c = (i % 2 == 0) ? accent : accent2;
        knobs[i]->s.setColour (juce::Slider::rotarySliderFillColourId, c);
        knobs[i]->s.setColour (juce::Slider::thumbColourId, c.brighter (0.2f));
        knobs[i]->name.setColour (juce::Label::textColourId, c.withAlpha (0.85f));
    }

    presetList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0x990a0614)); // semi-transparent
    presetList.setColour (juce::ListBox::outlineColourId, accent.withAlpha (0.3f));
    savePresetBtn.setColour (juce::TextButton::textColourOffId, accent2);
    loadPresetBtn.setColour (juce::TextButton::textColourOffId, accent);
    bankBtn.setColour (juce::TextButton::textColourOffId, accent2);

    themeAccent = accent;
    themeAccent2 = accent2;
    themePanelBg = panelBg;
    repaint();
}

void SalekHightechAudioProcessorEditor::cycleHero()
{
    // Toggle only the 2 official backgrounds
    juce::Image imgs[2] = { lianImg, cyanImg };
    heroIndex = (heroIndex + 1) % 2;
    if (imgs[heroIndex].isValid())
        heroImg = imgs[heroIndex];
    else if (imgs[1 - heroIndex].isValid())
        heroImg = imgs[1 - heroIndex];
    repaint();
}

void SalekHightechAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.x >= 8 && e.x <= 222 && e.y >= 46 && e.y <= 210)
        cycleHero();
}

void SalekHightechAudioProcessorEditor::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    // Mouse-wheel over any ComboBox changes selection (filter mode, shapes, themes…)
    auto tryWheel = [&] (juce::ComboBox& box) -> bool
    {
        if (! box.isVisible() || ! box.getBounds().contains (e.getEventRelativeTo (&box).getPosition()))
            return false;
        const int n = box.getNumItems();
        if (n <= 0) return false;
        int idx = box.getSelectedItemIndex();
        if (idx < 0) idx = 0;
        idx += (wheel.deltaY > 0.0f ? -1 : 1);
        idx = juce::jlimit (0, n - 1, idx);
        box.setSelectedItemIndex (idx, juce::sendNotificationSync);
        return true;
    };
    if (tryWheel (filterMode)) return;
    if (tryWheel (filterRouteBox)) return;
    if (tryWheel (voiceModeBox)) return;
    if (tryWheel (osc1ShapeBox)) return;
    if (tryWheel (osc2ShapeBox)) return;
    if (tryWheel (osc3ShapeBox)) return;
    if (tryWheel (themeBox)) return;
    if (tryWheel (lfo1WaveBox)) return;
    if (tryWheel (lfo2WaveBox)) return;
    if (tryWheel (lfo3WaveBox)) return;
    if (tryWheel (distModeBox)) return;
    if (tryWheel (reverbModeBox)) return;
    juce::Component::mouseWheelMove (e, wheel);
}

void SalekHightechAudioProcessorEditor::applyUiLanguage()
{
    if (uiLangFa)
    {
        tabs.setTabName (0, juce::CharPointer_UTF8 ("\xd8\xa7\xd8\xb5\xd9\x84\xdb\x8c"));
        tabs.setTabName (1, juce::CharPointer_UTF8 ("\xd9\x85\xd8\xaf\xd9\x88\xd9\x84"));
        tabs.setTabName (2, juce::CharPointer_UTF8 ("\xd8\xa7\xd9\x84\xd8\xa7\xd9\x81\xd8\xa7\xd9\x88"));
        tabs.setTabName (3, juce::CharPointer_UTF8 ("\xd8\xa7\xd9\x81\xda\xa9\xd8\xaa"));
        tabs.setTabName (4, juce::CharPointer_UTF8 ("\xd9\x85\xd8\xac\xdb\x8c\xda\xa9"));
        tabs.setTabName (5, juce::CharPointer_UTF8 ("\xd8\xb3\xda\xa9\xd9\x88\xd8\xa7\xd9\x86\xd8\xb3"));
        magicLoopBtn.setButtonText (juce::CharPointer_UTF8 ("\xd9\x84\xd9\x88\xd9\xbe"));
        magicGlitchBtn.setButtonText (juce::CharPointer_UTF8 ("\xda\xaf\xd9\x84\xdb\x8c\xda\x86"));
        magicFlangeBtn.setButtonText (juce::CharPointer_UTF8 ("\xd9\x81\xd9\x84\xd9\x86\xd8\xac"));
        magicPsychBtn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xb1\xd9\x88\xd8\xa7\xd9\x86\xdb\x8c"));
        magicHold.setButtonText (juce::CharPointer_UTF8 ("\xd9\x86\xda\xaf\xd9\x87\xd8\xaf\xd8\xa7\xd8\xb1"));
        arpOn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xa2\xd8\xb1\xd9\xbe"));
        seqOn.setButtonText (juce::CharPointer_UTF8 ("\xd8\xb3\xda\xa9\xd9\x88\xd8\xa7\xd9\x86\xd8\xb3"));
        for (int i = 0; i < 8; ++i)
            fxBypass[i].setButtonText (juce::CharPointer_UTF8 ("\xd8\xa8\xd8\xa7\xdb\x8c\xd9\xbe\xd8\xb3"));
    }
    else
    {
        tabs.setTabName (0, "MAIN");
        tabs.setTabName (1, "MOD");
        tabs.setTabName (2, "LFO");
        tabs.setTabName (3, "FX");
        tabs.setTabName (4, "MAGIC");
        tabs.setTabName (5, "SEQ");
        magicLoopBtn.setButtonText ("LOOP");
        magicGlitchBtn.setButtonText ("GLITCH");
        magicFlangeBtn.setButtonText ("FLANGE");
        magicPsychBtn.setButtonText ("PSY");
        magicHold.setButtonText ("HOLD");
        arpOn.setButtonText ("ARP");
        seqOn.setButtonText ("SEQ");
        for (int i = 0; i < 8; ++i)
            fxBypass[i].setButtonText ("BYP");
    }
    repaint();
}

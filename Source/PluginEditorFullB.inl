int SalekHightechAudioProcessorEditor::getNumRows() { return presetRows.size(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);
    if (pr.isHeader)
    {
        g.fillAll (juce::Colour (0xff12081c));
        g.setColour (themeAccent);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("  " + pr.label, 6, 0, width - 10, height, juce::Justification::centredLeft);
        g.setColour (themeAccent2.withAlpha (0.5f));
        g.drawLine (4.0f, (float) height - 1.0f, (float) width - 4.0f, (float) height - 1.0f, 1.0f);
        return;
    }
    if (selected) g.fillAll (juce::Colour (0xff3a0066));
    else g.fillAll (themePanelBg);
    juce::Colour tc = selected ? themeAccent : juce::Colour (0xffd0c0e0);
    auto fullName = processor.getProgramName (pr.programIndex);
    if (fullName.startsWith ("Acid")) tc = selected ? tc : juce::Colour (0xffffcc44);
    if (fullName.startsWith ("Bass")) tc = selected ? tc : juce::Colour (0xff66ff99);
    if (fullName.startsWith ("Lead")) tc = selected ? tc : juce::Colour (0xff00f0ff);
    if (fullName.startsWith ("Kick")) tc = selected ? tc : juce::Colour (0xffffab40);
    if (fullName.startsWith ("FM"))   tc = selected ? tc : juce::Colour (0xffff66cc);
    if (fullName.startsWith ("Retro"))tc = selected ? tc : juce::Colour (0xffa0a0ff);
    if (fullName.startsWith ("SALEK")) tc = selected ? tc : juce::Colour (0xffffd700);
    if (fullName.startsWith ("USER")) tc = selected ? tc : juce::Colour (0xffffaa00);
    g.setColour (tc);
    g.setFont (juce::FontOptions (12.5f));
    g.drawText ("   " + pr.label, 8, 0, width - 12, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);
    if (pr.isHeader || pr.programIndex < 0) return;
    processor.setCurrentProgram (pr.programIndex);
    presetLabel.setText (processor.getProgramName (pr.programIndex), juce::dontSendNotification);
}

#include "PluginEditorPaint.inl"

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 8) resized();
    animPhase += 0.04f;
    repaint();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto full = getLocalBounds();

    {
        auto kb = full.removeFromBottom (68).reduced (2, 1);
        keyboard.setKeyWidth ((float) juce::jmax (10, kb.getWidth() / 52));
        keyboard.setBounds (kb);
    }
    full.removeFromBottom (4);

    // Collapsible left hero strip (default open = 222 px, collapsed = 36 px)
    const int leftW = sideCollapsed ? 36 : 222;
    full.removeFromLeft (leftW);
    // Toggle sits under the hero image (or at left edge when collapsed)
    sideToggle.setBounds (sideCollapsed ? 4 : 8, sideCollapsed ? 48 : 300, sideCollapsed ? 28 : 56, 22);
    sideToggle.toFront (false);

    auto header = full.removeFromTop (36);
    themeBox.setBounds (header.removeFromRight (110).reduced (2));
    spectrum.setBounds (header.removeFromRight (90).reduced (2));
    scope.setBounds (header.removeFromRight (110).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (160).reduced (2));

    full.removeFromTop (2);
    tabs.setBounds (full);
    tabs.toFront (false);
    keyboard.toFront (false);

    auto place = [] (juce::Rectangle<int> area,
                     std::vector<std::unique_ptr<Knob>>& all,
                     int start, int count, int cols)
    {
        if (count <= 0 || area.getWidth() < 20 || area.getHeight() < 20 || cols < 1) return;
        const int rows = (count + cols - 1) / cols;
        const int cw = area.getWidth() / cols;
        const int ch = area.getHeight() / juce::jmax (1, rows);
        for (int i = 0; i < count; ++i)
        {
            const int idx = start + i;
            if (idx < 0 || idx >= (int) all.size()) break;
            auto* k = all[(size_t) idx].get();
            const int c = i % cols;
            const int r = i / cols;
            auto cell = juce::Rectangle<int> (area.getX() + c * cw, area.getY() + r * ch, cw, ch).reduced (6);
            if (cell.getHeight() < 28) continue;
            k->name.setBounds (cell.removeFromBottom (13));
            k->s.setBounds (cell);
        }
    };

    {
        auto b = mainTab.getLocalBounds().reduced (4);
        presetTab.setBounds (b.removeFromLeft (200));
        envTab.setBounds (b.removeFromBottom (128));
        filterTab.setBounds (b.removeFromRight (248));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (3);
            auto top = pb.removeFromTop (24);
            prevPreset.setBounds (top.removeFromLeft (22).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (22).reduced (1));
            initBtn.setBounds (top.removeFromLeft (32).reduced (1));
            savePresetBtn.setBounds (top.removeFromLeft (36).reduced (1));
            loadPresetBtn.setBounds (top.removeFromLeft (36).reduced (1));
            bankBtn.setBounds (top.removeFromLeft (40).reduced (1));
            auto nameRow = pb.removeFromTop (20);
            presetLabel.setBounds (nameRow.reduced (2, 0));
            presetLabel.setJustificationType (juce::Justification::centredLeft);
            presetLabel.setColour (juce::Label::textColourId, themeAccent);
            presetList.setBounds (pb);
        }

        {
            auto oa = oscTab.getLocalBounds().reduced (4);
            // Top row: 3 shape monitors
            auto monRow = oa.removeFromTop (78);
            const int mw = monRow.getWidth() / 3;
            if (oscMon1 != nullptr) oscMon1->setBounds (monRow.removeFromLeft (mw).reduced (3));
            if (oscMon2 != nullptr) oscMon2->setBounds (monRow.removeFromLeft (mw).reduced (3));
            if (oscMon3 != nullptr) oscMon3->setBounds (monRow.reduced (3));
            // Shape selector combos under monitors
            auto shRow = oa.removeFromTop (26);
            const int sw = shRow.getWidth() / 3;
            osc1ShapeBox.setBounds (shRow.removeFromLeft (sw).reduced (2));
            osc2ShapeBox.setBounds (shRow.removeFromLeft (sw).reduced (2));
            osc3ShapeBox.setBounds (shRow.reduced (2));
            place (oa, knobs, 0, 21, 6);
        }

        {
            auto fr = filterTab.getLocalBounds().reduced (4);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (72).reduced (2));
            fr.removeFromTop (4);
            place (fr, knobs, 21, 4, 2);
        }

        {
            auto er = envTab.getLocalBounds().reduced (4);
            if (adsrDisplay != nullptr)
                adsrDisplay->setBounds (er.removeFromTop (52).reduced (2));
            place (er, knobs, 25, 4, 4);
        }
    }

    {
        auto r = modTab.getLocalBounds().reduced (6);
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (juce::jmax (320, r.getWidth() * 55 / 100)).reduced (2));
        auto right = r.reduced (4);
        auto row1 = right.removeFromTop (right.getHeight() / 2);
        place (row1, knobs, 29, 6, 3);
        place (right, knobs, 35, 4, 4);
    }

    {
        auto r = lfoTab.getLocalBounds().reduced (6);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (70).reduced (2));

        auto presetRow = r.removeFromTop (26);
        const int bw = presetRow.getWidth() / 12;
        juce::TextButton* presets[] = {
            &lfoPresetSine, &lfoPresetTri, &lfoPresetSaw, &lfoPresetSqr,
            &lfoPresetPulse, &lfoPresetExp, &lfoPresetLog, &lfoPresetBell,
            &lfoPresetWob, &lfoPresetChaos, &lfoPresetGate, &lfoPresetCustom
        };
        for (auto* b : presets)
            b->setBounds (presetRow.removeFromLeft (bw).reduced (1));

        auto waveRow = r.removeFromTop (26);
        lfo1WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 3).reduced (2));
        lfo2WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 2).reduced (2));
        lfo3WaveBox.setBounds (waveRow.reduced (2));

        // Copy shape to LFO1/2/3 + SAVE/LOAD slots A/B/C
        auto copyRow = r.removeFromTop (24);
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

        auto knobArea = r.removeFromBottom (100);
        place (knobArea, knobs, 39, 6, 6);
        lfoShapeEditor.setBounds (r.reduced (3));
    }

    #include "PluginEditorFullB_FxLayout.inl"

    {
        auto r = magicTab.getLocalBounds().reduced (10);
        auto top = r.removeFromTop (32);
        const int bw = top.getWidth() / 5;
        magicLoopBtn.setBounds (top.removeFromLeft (bw).reduced (2));
        magicGlitchBtn.setBounds (top.removeFromLeft (bw).reduced (2));
        magicFlangeBtn.setBounds (top.removeFromLeft (bw).reduced (2));
        magicPsychBtn.setBounds (top.removeFromLeft (bw).reduced (2));
        magicHold.setBounds (top.reduced (2));
        magicHint.setBounds (r.removeFromBottom (22));
        if (magicPad != nullptr)
            magicPad->setBounds (r.reduced (6));
    }

    {
        // FX now 24 knobs (45..68), SEQ starts at 69
        auto bounds = seqTab.getLocalBounds().reduced (8);
        auto top = bounds.removeFromTop (52);
        arpOn.setBounds (top.removeFromLeft (100).reduced (4));
        seqOn.setBounds (top.removeFromLeft (100).reduced (4));
        place (top.removeFromRight (360).reduced (4), knobs, 69, 3, 3);
        bounds.removeFromTop (8);
        if (stepGrid != nullptr)
            stepGrid->setBounds (bounds);
    }
}

void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    if (! lianImg.isValid()) lianImg = SalekAssets::loadLian();
    if (! faceImg.isValid()) faceImg = SalekAssets::loadFace();
    if (! cyanImg.isValid()) cyanImg = SalekAssets::loadCyanGirl();

    juce::Colour accent, accent2, panelBg, labelBg, labelTx;
    if (id == 2)
    {
        heroImg = faceImg.isValid() ? faceImg : (lianImg.isValid() ? lianImg : logoImg);
        heroIndex = 1;
        accent  = juce::Colour (0xffc0ff00);
        accent2 = juce::Colour (0xffff6b00);
        panelBg = juce::Colour (0xff0a1204);
        labelBg = juce::Colour (0xff1a2a08);
        labelTx = juce::Colour (0xffc0ff00);
    }
    else if (id == 3)
    {
        heroImg = cyanImg.isValid() ? cyanImg : (faceImg.isValid() ? faceImg : lianImg);
        heroIndex = 2;
        accent  = juce::Colour (0xffff2d9b);
        accent2 = juce::Colour (0xff7c4dff);
        panelBg = juce::Colour (0xff12061a);
        labelBg = juce::Colour (0xff2a0a30);
        labelTx = juce::Colour (0xffff66cc);
    }
    else
    {
        heroImg = lianImg.isValid() ? lianImg : (faceImg.isValid() ? faceImg : logoImg);
        heroIndex = 0;
        accent  = juce::Colour (0xff00e8ff);
        accent2 = juce::Colour (0xffffd700);
        panelBg = juce::Colour (0xff0a0614);
        labelBg = juce::Colour (0xff1a0a30);
        labelTx = juce::Colour (0xff00e8ff);
    }

    tabs.setColour (juce::TabbedComponent::backgroundColourId, panelBg);
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

    presetList.setColour (juce::ListBox::backgroundColourId, panelBg);
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
    juce::Image imgs[4] = { lianImg, faceImg, cyanImg, logoImg };
    for (int n = 0; n < 4; ++n)
    {
        heroIndex = (heroIndex + 1) % 4;
        if (imgs[heroIndex].isValid())
        {
            heroImg = imgs[heroIndex];
            repaint();
            return;
        }
    }
}

void SalekHightechAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (! sideCollapsed && e.x >= 8 && e.x <= 222 && e.y >= 46 && e.y <= 210)
        cycleHero();
}

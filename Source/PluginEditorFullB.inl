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
        const bool folded = collapsedCats.contains (pr.category);
        juce::String mark = folded ? "[+] " : "[-] ";
        g.drawText (mark + pr.label, 6, 0, width - 10, height, juce::Justification::centredLeft);
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

    // Hero art always visible — only presets collapse (see presetToggle)
    full.removeFromLeft (222);

    auto header = full.removeFromTop (40);
    langToggle.setBounds (header.removeFromRight (36).reduced (2));
    themeBox.setBounds (header.removeFromRight (90).reduced (2));
    // Master gain + drive at top of VST (removed from FX tab)
    if (knobs.size() > 55)
    {
        auto* gainK = knobs[55].get(); // master_gain
        auto* drvK  = knobs[54].get(); // master_drive
        auto gArea = header.removeFromRight (70).reduced (2);
        gainK->name.setBounds (gArea.removeFromBottom (12));
        gainK->s.setBounds (gArea);
        gainK->s.setVisible (true); gainK->name.setVisible (true);
        auto dArea = header.removeFromRight (70).reduced (2);
        drvK->name.setBounds (dArea.removeFromBottom (12));
        drvK->s.setBounds (dArea);
        drvK->s.setVisible (true); drvK->name.setVisible (true);
        gainK->name.setText ("GAIN", juce::dontSendNotification);
        drvK->name.setText ("DRIVE", juce::dontSendNotification);
    }
    spectrum.setBounds (header.removeFromRight (80).reduced (2));
    scope.setBounds (header.removeFromRight (100).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (140).reduced (2));

    full.removeFromTop (2);
    tabs.setBounds (full);
    tabs.toFront (false);
    keyboard.toFront (false);

    auto place = [] (juce::Rectangle<int> area,
                     std::vector<std::unique_ptr<Knob>>& all,
                     int start, int count, int cols,
                     int maxCellH = 0)
    {
        if (count <= 0 || cols < 1) return;
        const int rows = juce::jmax (1, (count + cols - 1) / cols);
        const int cw = juce::jmax (1, area.getWidth() / cols);
        int ch = juce::jmax (1, area.getHeight() / rows);
        if (maxCellH > 0) ch = juce::jmin (ch, maxCellH);
        // vertical centre the grid when maxCellH shrinks rows
        const int usedH = ch * rows;
        const int y0 = area.getY() + juce::jmax (0, (area.getHeight() - usedH) / 2);
        for (int i = 0; i < count; ++i)
        {
            const int idx = start + i;
            if (idx < 0 || idx >= (int) all.size()) break;
            auto* k = all[(size_t) idx].get();
            const int c = i % cols;
            const int r = i / cols;
            const int pad = (ch < 40 || cw < 50) ? 2 : 4;
            auto cell = juce::Rectangle<int> (area.getX() + c * cw, y0 + r * ch, cw, ch).reduced (pad);
            const int nameH = (ch < 40) ? 11 : 14;
            k->name.setBounds (cell.removeFromBottom (nameH));
            k->name.setJustificationType (juce::Justification::centred);
            k->name.setVisible (true);
            k->s.setBounds (cell);
            k->s.setVisible (true);
            if (ch < 55)
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (28, cw - 8), 12);
            else
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
        }
    };

    {
        auto b = mainTab.getLocalBounds().reduced (2);
        // Collapsible PRESET column (not hero)
        const int presetW = presetCollapsed ? 28 : 180;
        presetTab.setBounds (b.removeFromLeft (presetW));
        envTab.setBounds (b.removeFromBottom (120));
        filterTab.setBounds (b.removeFromRight (220));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (2);
            auto top = pb.removeFromTop (24);
            presetToggle.setBounds (top.removeFromLeft (24).reduced (1));
            if (! presetCollapsed)
            {
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

        {
            auto oa = oscTab.getLocalBounds().reduced (3);
            // Monitors — compact so knobs always have room
            auto monRow = oa.removeFromTop (juce::jlimit (48, 70, oa.getHeight() / 5));
            const int mw = monRow.getWidth() / 3;
            if (oscMon1 != nullptr) oscMon1->setBounds (monRow.removeFromLeft (mw).reduced (2));
            if (oscMon2 != nullptr) oscMon2->setBounds (monRow.removeFromLeft (mw).reduced (2));
            if (oscMon3 != nullptr) oscMon3->setBounds (monRow.reduced (2));
            auto shRow = oa.removeFromTop (22);
            const int sw = shRow.getWidth() / 3;
            osc1ShapeBox.setBounds (shRow.removeFromLeft (sw).reduced (1));
            osc2ShapeBox.setBounds (shRow.removeFromLeft (sw).reduced (1));
            osc3ShapeBox.setBounds (shRow.reduced (1));
            // 21 knobs: 3 rows of 6 (OSC1/2/3) + 1 row of 3 unison — force 6 cols
            place (oa, knobs, 0, 21, 6);
        }

        {
            auto fr = filterTab.getLocalBounds().reduced (3);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (juce::jlimit (50, 80, fr.getHeight() / 3)).reduced (2));
            filterMode.setBounds (fr.removeFromTop (24).reduced (1));
            place (fr, knobs, 21, 4, 2);
        }

        {
            auto er = envTab.getLocalBounds().reduced (2);
            // Compact ADSR curve on the left; large knobs on the right (cleaner)
            if (adsrDisplay != nullptr)
            {
                auto curve = er.removeFromLeft (juce::jmin (200, er.getWidth() / 3));
                adsrDisplay->setBounds (curve.reduced (2));
            }
            place (er, knobs, 25, 4, 4);
        }
    }

    {
        auto r = modTab.getLocalBounds().reduced (4);
        // Right rail: FM block (top) + MACRO block (bottom) — fixed cell heights
        auto right = r.removeFromRight (juce::jmin (300, r.getWidth() * 34 / 100));
        auto macroBand = right.removeFromBottom (110); // fixed height: knob+label glued
        auto fmArea = right.reduced (2, 4);

        // FM/PM/RM/AM: 2x3 grid, max cell 92px
        place (fmArea, knobs, 29, 6, 3, 92);

        // MACROS: explicit layout so name is always glued under rotary (no floating labels)
        {
            const int start = 35;
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
        place (knobArea, knobs, 39, 6, 6);
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
    else if (id == 4)
    {
        // PATINA — soft mint + hot pink (reference aesthetic)
        heroImg = lianImg.isValid() ? lianImg : (faceImg.isValid() ? faceImg : logoImg);
        heroIndex = 0;
        accent  = juce::Colour (0xffff2d9b);   // hot pink
        accent2 = juce::Colour (0xff39ff14);  // lime
        panelBg = juce::Colour (0xff1a0f24);  // deep mauve (readable UI)
        labelBg = juce::Colour (0xff2a1838);
        labelTx = juce::Colour (0xffff66aa);
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
    if (e.x >= 8 && e.x <= 222 && e.y >= 46 && e.y <= 210)
        cycleHero();
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

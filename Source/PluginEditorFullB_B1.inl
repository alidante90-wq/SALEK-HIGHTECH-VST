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

    {
        const int presetW = presetCollapsed ? 28 : 210;
        auto leftStrip = full.removeFromLeft (presetW);
        const int logoPad = 88;
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
            prevPreset.setVisible (true); nextPreset.setVisible (true);
            initBtn.setVisible (true); savePresetBtn.setVisible (true);
            loadPresetBtn.setVisible (true); bankBtn.setVisible (true);
            presetLabel.setVisible (true);
        }
        else
        {
            presetList.setVisible (false); prevPreset.setVisible (false);
            nextPreset.setVisible (false); initBtn.setVisible (false);
            savePresetBtn.setVisible (false); loadPresetBtn.setVisible (false);
            bankBtn.setVisible (false); presetLabel.setVisible (false);
        }
    }

    auto header = full.removeFromTop (40);
    langToggle.setBounds (header.removeFromRight (36).reduced (2));
    themeBox.setBounds (header.removeFromRight (90).reduced (2));
    bgSwapBtn.setBounds (header.removeFromRight (40).reduced (2));
    spectrum.setBounds (header.removeFromRight (80).reduced (2));
    scope.setBounds (header.removeFromRight (100).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (140).reduced (2));

    tabs.setBounds (full);
    tabs.toFront (false);

    // MAIN — OSC top half + separators, filter top-right, env bottom with real glide knobs
    {
        auto b = mainTab.getLocalBounds().reduced (2);
        envTab.setBounds (b.removeFromBottom (110));
        filterTab.setBounds (b.removeFromRight (200));
        // OSC uses only UPPER half — lower half left empty (user workspace)
        auto oscFull = b;
        oscTab.setBounds (oscFull.removeFromTop (oscFull.getHeight() / 2));
        // (oscFull remainder is empty by design)

        {
            auto oa = oscTab.getLocalBounds().reduced (2);
            const int gap = 10; // green separators between oscillators
            const int colW = (oa.getWidth() - gap * 2) / 3;
            juce::Component* mons[3] = { oscMon1.get(), oscMon2.get(), oscMon3.get() };
            juce::ComboBox* shapes[3] = { &osc1ShapeBox, &osc2ShapeBox, &osc3ShapeBox };
            for (int c = 0; c < 3; ++c)
            {
                auto col = juce::Rectangle<int> (oa.getX() + c * (colW + gap), oa.getY(), colW, oa.getHeight()).reduced (2, 2);
                // vertical separator between oscillators
                if (c > 0)
                {
                    // drawn in paint; keep a small gap via reduced above
                }
                auto monH = juce::jlimit (36, 56, col.getHeight() / 5);
                if (mons[c] != nullptr) mons[c]->setBounds (col.removeFromTop (monH).reduced (1));
                shapes[c]->setBounds (col.removeFromTop (18).reduced (1));
                auto paramArea = col.removeFromTop (juce::jmax (70, col.getHeight() * 2 / 3));
                place (paramArea, knobs, c * 6, 6, 2);
                place (col, knobs, 18 + c * 3, 3, 3);
            }
        }
        {
            // filter: knobs + modes at TOP, leave lower part of filter strip free
            auto fr = filterTab.getLocalBounds().reduced (3);
            auto top = fr.removeFromTop (juce::jmax (140, fr.getHeight() * 55 / 100));
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (top.removeFromTop (juce::jlimit (44, 70, top.getHeight() / 3)).reduced (2));
            auto modeRow = top.removeFromTop (22);
            filterMode.setBounds (modeRow.removeFromLeft (modeRow.getWidth() / 2).reduced (1));
            filterRouteBox.setBounds (modeRow.reduced (1));
            place (top, knobs, 27, 4, 2);
            // fr remainder empty under filter
        }
        {
            auto er = envTab.getLocalBounds().reduced (2);
            if (adsrDisplay != nullptr)
            {
                auto curve = er.removeFromLeft (juce::jmin (150, er.getWidth() / 4));
                adsrDisplay->setBounds (curve.reduced (2));
            }
            auto adsrKnobs = er.removeFromLeft (juce::jmin (240, er.getWidth() * 2 / 5));
            place (adsrKnobs, knobs, 31, 4, 4); // attack decay sustain release
            voiceModeBox.setBounds (er.removeFromTop (20).reduced (1));
            // glide voices noise sub — by param id if available
            place (er, knobs, 38, 4, 4);
        }
    }

    // MOD
    {
        auto r = modTab.getLocalBounds().reduced (4);
        auto right = r.removeFromRight (juce::jmin (300, r.getWidth() * 34 / 100));
        auto macroBand = right.removeFromBottom (110);
        auto fmArea = right.reduced (2, 4);
        place (fmArea, knobs, 35, 6, 3, 92);
        {
            const int start = 41, n = 4;
            const int cw = juce::jmax (1, macroBand.getWidth() / n);
            for (int i = 0; i < n; ++i)
            {
                if (start + i >= (int) knobs.size()) break;
                auto* k = knobs[(size_t) (start + i)].get();
                auto cell = juce::Rectangle<int> (macroBand.getX() + i * cw, macroBand.getY(), cw, macroBand.getHeight()).reduced (3, 2);
                k->name.setBounds (cell.removeFromBottom (16));
                k->name.setJustificationType (juce::Justification::centred);
                k->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
                k->name.setVisible (true);
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, cw - 10), 12);
                k->s.setBounds (cell);
                k->s.setVisible (true);
            }
        }
        if (matrixPanel != nullptr) matrixPanel->setBounds (r.reduced (2));
    }

    // LFO
    {
        auto r = lfoTab.getLocalBounds().reduced (6);
        if (lfoDisplay != nullptr) lfoDisplay->setBounds (r.removeFromTop (56).reduced (1));
        auto presetRow = r.removeFromTop (24);
        const int bw = presetRow.getWidth() / 12;
        juce::TextButton* presets[] = {
            &lfoPresetSine, &lfoPresetTri, &lfoPresetSaw, &lfoPresetSqr,
            &lfoPresetPulse, &lfoPresetExp, &lfoPresetLog, &lfoPresetBell,
            &lfoPresetWob, &lfoPresetChaos, &lfoPresetGate, &lfoPresetCustom
        };
        for (auto* b : presets) b->setBounds (presetRow.removeFromLeft (bw).reduced (1));
        auto waveRow = r.removeFromTop (24);
        lfo1WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 3).reduced (2));
        lfo2WaveBox.setBounds (waveRow.removeFromLeft (waveRow.getWidth() / 2).reduced (2));
        lfo3WaveBox.setBounds (waveRow.reduced (2));
        auto bankRow = r.removeFromTop (24);
        lfoShapeBank.setBounds (bankRow.reduced (2));
        auto copyRow = r.removeFromTop (22);
        const int cw = copyRow.getWidth() / 9;
        lfoCopyTo1.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoCopyTo2.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoCopyTo3.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveA.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadA.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveB.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadB.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoSaveC.setBounds (copyRow.removeFromLeft (cw).reduced (1));
        lfoLoadC.setBounds (copyRow.reduced (1));
        auto knobArea = r.removeFromBottom (88);
        place (knobArea, knobs, 45, 6, 6);
        lfoShapeEditor.setBounds (r.reduced (2));
    }

    #include "PluginEditorFullB_FxLayout.inl"

    // MAGIC: mode + per-effect HOLD
    {
        auto r = magicTab.getLocalBounds().reduced (6);
        auto rail = r.removeFromRight (160);
        magicHint.setBounds (rail.removeFromBottom (28).reduced (2));
        const int rowH = juce::jmax (36, rail.getHeight() / 5);
        juce::TextButton* modes[4] = { &magicLoopBtn, &magicGlitchBtn, &magicFlangeBtn, &magicPsychBtn };
        const char* holdNames[4] = { "H-LOOP", "H-GLITCH", "H-FLANGE", "H-PSY" };
        for (int i = 0; i < 4; ++i)
        {
            auto row = rail.removeFromTop (rowH).reduced (2, 2);
            auto holdArea = row.removeFromRight (70);
            modes[i]->setBounds (row);
            for (int c = 0; c < magicTab.getNumChildComponents(); ++c)
            {
                if (auto* b = dynamic_cast<juce::TextButton*> (magicTab.getChildComponent (c)))
                {
                    if (b->getButtonText() == holdNames[i])
                    {
                        b->setBounds (holdArea.reduced (1));
                        b->setVisible (true);
                        break;
                    }
                }
            }
        }
        magicHold.setBounds (rail.reduced (3));
        if (magicPad != nullptr)
        {
            magicPad->setBounds (r.reduced (4));
            magicPad->setAudioPeak (processor.getOutputPeak());
        }
    }

    // SEQ
    {
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

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
            k->s.setNumDecimalPlacesToDisplay (2);
            k->s.textFromValueFunction = [] (double v)
            {
                if (std::abs (v - std::round (v)) < 1e-3)
                    return juce::String ((int) std::round (v));
                return juce::String (v, 2);
            };
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
        const int logoPad = 64;
        // Preset ~32%, big character model gets remaining left column
        const int presetH = juce::jmax (110, (int) ((leftStrip.getHeight() - logoPad) * 0.32f));
        presetTab.setBounds (leftStrip.getX(), leftStrip.getY() + logoPad, leftStrip.getWidth(), presetH);
        presetTab.toFront (false);
        // Character PNG slot under presets (always visible when not collapsed)
        {
            if (presetCollapsed)
            {
                charSlotBounds = juce::Rectangle<int> (
                    leftStrip.getX() + 2, leftStrip.getY() + logoPad,
                    leftStrip.getWidth() - 4, juce::jmax (100, leftStrip.getHeight() - logoPad - 8));
            }
            else
            {
                const int cy = leftStrip.getY() + logoPad + presetH + 4;
                const int ch = juce::jmax (120, leftStrip.getBottom() - cy - 4);
                charSlotBounds = juce::Rectangle<int> (leftStrip.getX() + 2, cy, leftStrip.getWidth() - 4, ch);
            }
        }
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
    charCycleBtn.setBounds (header.removeFromRight (48).reduced (2));
    bgSwapBtn.setBounds (header.removeFromRight (40).reduced (2));
    masterGainSlider.setBounds (header.removeFromRight (40).reduced (2));
    // Compact LFO sources (horizontal strip, always visible)
    modSrcLfo3.setBounds (header.removeFromRight (38).reduced (1, 4));
    modSrcLfo2.setBounds (header.removeFromRight (38).reduced (1, 4));
    modSrcLfo1.setBounds (header.removeFromRight (38).reduced (1, 4));
    spectrum.setBounds (header.removeFromRight (72).reduced (2));
    scope.setBounds (header.removeFromRight (90).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (140).reduced (2));

    tabs.setBounds (full);
    tabs.toFront (false);

    // MAIN — place ALL knobs by PARAMETER ID (never by fragile array index)
    {
        auto byId = [this] (const char* id) -> Knob*
        {
            for (auto& k : knobs)
                if (k != nullptr && k->paramId == id)
                    return k.get();
            return nullptr;
        };
        auto placeId = [&] (juce::Rectangle<int> box, const char* id)
        {
            if (auto* k = byId (id))
            {
                k->name.setBounds (box.removeFromBottom (14));
                k->name.setJustificationType (juce::Justification::centred);
                k->name.setVisible (true);
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, box.getWidth() - 8), 12);
                k->s.setNumDecimalPlacesToDisplay (2);
                k->s.textFromValueFunction = [] (double v)
                {
                    if (std::abs (v - std::round (v)) < 1e-3)
                        return juce::String ((int) std::round (v));
                    return juce::String (v, 2);
                };
                k->s.setBounds (box);
                k->s.setVisible (true);
            }
        };

        auto b = mainTab.getLocalBounds().reduced (2);
        // FL Studio sometimes reports 0-size mid-session — skip only MAIN placement
        if (b.getWidth() >= 200 && b.getHeight() >= 120)
        {
        // ADSR strip taller so ATTACK/DECAY/SUSTAIN/RELEASE are visible
        envTab.setBounds (b.removeFromBottom (130));
        filterTab.setBounds (b.removeFromRight (180));
        auto oscFull = b;
        oscTab.setBounds (oscFull);
        oscTab.setVisible (true);
        filterTab.setVisible (true);
        envTab.setVisible (true);
        mainTab.setVisible (true);

        {
            auto oa = oscTab.getLocalBounds().reduced (2);
            const int gap = 12;
            const int colW = (oa.getWidth() - gap * 2) / 3;
            juce::Component* mons[3] = { oscMon1.get(), oscMon2.get(), oscMon3.get() };
            juce::ComboBox* shapes[3] = { &osc1ShapeBox, &osc2ShapeBox, &osc3ShapeBox };
            const char* idsLvl[3]   = { "osc1_level", "osc2_level", "osc3_level" };
            const char* idsTbl[3]   = { "osc1_table", "osc2_table", "osc3_table" };
            const char* idsWarp[3]  = { "osc1_warp", "osc2_warp", "osc3_warp" };
            const char* idsFold[3]  = { "osc1_fold", "osc2_fold", "osc3_fold" };
            const char* idsDrive[3] = { "osc1_drive", "osc2_drive", "osc3_drive" };
            const char* idsOct[3]   = { "osc1_octave", "osc2_octave", "osc3_octave" };
            const char* idsUni[3]   = { "osc1_unison", "osc2_unison", "osc3_unison" };
            const char* idsDet[3]   = { "osc1_udet", "osc2_udet", "osc3_udet" };
            const char* idsSpr[3]   = { "osc1_uspread", "osc2_uspread", "osc3_uspread" };
            const char* idsPh[3]    = { "osc1_phase", "osc2_phase", "osc3_phase" };
            const char* idsRnd[3]   = { "osc1_rand", "osc2_rand", "osc3_rand" };
            const char* idsPan[3]   = { "osc1_pan", "osc2_pan", "osc3_pan" };

            for (int c = 0; c < 3; ++c)
            {
                auto col = juce::Rectangle<int> (oa.getX() + c * (colW + gap), oa.getY(), colW, oa.getHeight()).reduced (2, 2);
                auto monH = juce::jlimit (36, 56, col.getHeight() / 5);
                if (mons[c] != nullptr) mons[c]->setBounds (col.removeFromTop (monH).reduced (1));
                shapes[c]->setBounds (col.removeFromTop (18).reduced (1));

                // Equal rows so knobs stay readable
                const int rowH = juce::jmax (48, col.getHeight() / 5);
                auto row1 = col.removeFromTop (rowH);
                placeId (row1.removeFromLeft (row1.getWidth() / 2).reduced (3), idsLvl[c]);
                placeId (row1.reduced (3), idsTbl[c]);
                auto row2 = col.removeFromTop (rowH);
                placeId (row2.removeFromLeft (row2.getWidth() / 2).reduced (3), idsWarp[c]);
                placeId (row2.reduced (3), idsFold[c]);
                auto row3 = col.removeFromTop (rowH);
                placeId (row3.removeFromLeft (row3.getWidth() / 2).reduced (3), idsDrive[c]);
                placeId (row3.reduced (3), idsOct[c]);
                auto uniRow = col.removeFromTop (rowH);
                const int uw = uniRow.getWidth() / 3;
                placeId (uniRow.removeFromLeft (uw).reduced (2), idsUni[c]);
                placeId (uniRow.removeFromLeft (uw).reduced (2), idsDet[c]);
                placeId (uniRow.reduced (2), idsSpr[c]);
                auto pr = col;
                auto pr3 = pr.getWidth() / 3;
                placeId (pr.removeFromLeft (pr3).reduced (2), idsPh[c]);
                placeId (pr.removeFromLeft (pr3).reduced (2), idsRnd[c]);
                placeId (pr.reduced (2), idsPan[c]);
            }
        }
        {
            auto fr = filterTab.getLocalBounds().reduced (3);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (56).reduced (2));
            auto modeRow = fr.removeFromTop (22);
            filterMode.setBounds (modeRow.removeFromLeft (modeRow.getWidth() / 2).reduced (1));
            filterRouteBox.setBounds (modeRow.reduced (1));
            // 2x2 grid: CUTOFF RESO / F DRIVE F ENV
            auto r1 = fr.removeFromTop (fr.getHeight() / 2);
            placeId (r1.removeFromLeft (r1.getWidth() / 2).reduced (4), "filter_cutoff");
            placeId (r1.reduced (4), "filter_reso");
            placeId (fr.removeFromLeft (fr.getWidth() / 2).reduced (4), "filter_drive");
            placeId (fr.reduced (4), "filter_env");
        }
        {
            auto er = envTab.getLocalBounds().reduced (4);
            // ADSR curve
            if (adsrDisplay != nullptr)
            {
                auto curve = er.removeFromLeft (120);
                adsrDisplay->setBounds (curve.reduced (2));
            }
            // ATTACK DECAY SUSTAIN RELEASE — fixed wide cells so they never disappear
            auto adsrKnobs = er.removeFromLeft (320);
            {
                const int aw = adsrKnobs.getWidth() / 4;
                placeId (juce::Rectangle<int> (adsrKnobs.getX(), adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_attack");
                placeId (juce::Rectangle<int> (adsrKnobs.getX() + aw, adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_decay");
                placeId (juce::Rectangle<int> (adsrKnobs.getX() + aw * 2, adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_sustain");
                placeId (juce::Rectangle<int> (adsrKnobs.getX() + aw * 3, adsrKnobs.getY(), aw, adsrKnobs.getHeight()).reduced (4, 2), "amp_release");
            }
            // Mono + glide / voices / noise / sub on the right
            voiceModeBox.setBounds (er.removeFromTop (22).removeFromLeft (120).reduced (1));
            const int ew = juce::jmax (1, er.getWidth() / 4);
            placeId (er.removeFromLeft (ew).reduced (3, 2), "glide");
            placeId (er.removeFromLeft (ew).reduced (3, 2), "poly_voices");
            placeId (er.removeFromLeft (ew).reduced (3, 2), "noise_level");
            placeId (er.reduced (3, 2), "sub_level");
        }
    }


        } // end valid MAIN size
    // FL recovery: never leave MAIN knobs invisible after tab switch / host resize
    {
        auto forceVis = [] (juce::Component& c)
        {
            c.setVisible (true);
            for (int i = 0; i < c.getNumChildComponents(); ++i)
                if (auto* ch = c.getChildComponent (i))
                    ch->setVisible (true);
        };
        forceVis (oscTab);
        forceVis (filterTab);
        forceVis (envTab);
        forceVis (mainTab);
    }

    // MOD — matrix left; clean FM grid + macros right (hide stray knobs)
    {
        auto r = modTab.getLocalBounds().reduced (4);
        auto right = r.removeFromRight (juce::jmin (300, r.getWidth() * 34 / 100));

        // Only hide MOD-owned params (never touch osc/filter/env parents)
        static const char* modOnly[] = {
            "fm_2to1","fm_3to1","fm_3to2","pm_2to1","rm_2to1","am_2to1",
            "macro1","macro2","macro3","macro4","lfo_rate","lfo_amount"
        };
        for (auto& kk : knobs)
        {
            if (kk == nullptr) continue;
            bool isMod = false;
            for (auto* id : modOnly)
                if (kk->paramId == id) { isMod = true; break; }
            if (isMod)
            {
                kk->s.setVisible (false);
                kk->name.setVisible (false);
            }
        }

        auto byId = [this] (const char* id) -> Knob*
        {
            for (auto& k : knobs)
                if (k != nullptr && k->paramId == id) return k.get();
            return nullptr;
        };
        auto placeOne = [&] (juce::Rectangle<int> cell, const char* id)
        {
            auto* k = byId (id);
            if (k == nullptr) return;
            k->name.setBounds (cell.removeFromBottom (14));
            k->name.setJustificationType (juce::Justification::centred);
            k->name.setFont (juce::FontOptions (10.f, juce::Font::bold));
            k->name.setVisible (true);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (40, cell.getWidth() - 8), 13);
            k->s.setNumDecimalPlacesToDisplay (2);
            k->s.setBounds (cell);
            k->s.setVisible (true);
        };

        // FM block top of right rail (2 rows x 3 cols)
        auto fmArea = right.removeFromTop (juce::jmax (160, right.getHeight() * 55 / 100)).reduced (4, 4);
        const char* fmIds[] = { "fm_2to1","fm_3to1","fm_3to2","pm_2to1","rm_2to1","am_2to1" };
        const int fmCols = 3, fmRows = 2;
        const int cellW = fmArea.getWidth() / fmCols;
        const int cellH = fmArea.getHeight() / fmRows;
        for (int i = 0; i < 6; ++i)
        {
            auto cell = juce::Rectangle<int> (
                fmArea.getX() + (i % fmCols) * cellW,
                fmArea.getY() + (i / fmCols) * cellH, cellW, cellH).reduced (5, 3);
            placeOne (cell, fmIds[i]);
        }

        // Macros bottom of right rail — 4 equal columns
        auto macroBand = right.reduced (4, 6);
        const char* macIds[] = { "macro1","macro2","macro3","macro4" };
        const int cw = juce::jmax (1, macroBand.getWidth() / 4);
        for (int i = 0; i < 4; ++i)
        {
            auto cell = juce::Rectangle<int> (macroBand.getX() + i * cw, macroBand.getY(),
                                              cw, macroBand.getHeight()).reduced (4, 2);
            placeOne (cell, macIds[i]);
        }

        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.reduced (2));
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
        lfoPointsBox.setBounds (bankRow.removeFromRight (90).reduced (2));
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
        auto knobArea = r.removeFromBottom (96);
        {
            const char* lids[] = { "lfo_rate","lfo_amount","lfo2_rate","lfo2_amount","lfo3_rate","lfo3_amount" };
            const int lfoCw = juce::jmax (1, knobArea.getWidth() / 6);
            for (int i = 0; i < 6; ++i)
            {
                Knob* k = nullptr;
                for (auto& kk : knobs)
                    if (kk != nullptr && kk->paramId == lids[i]) { k = kk.get(); break; }
                if (k == nullptr) continue;
                auto cell = juce::Rectangle<int> (knobArea.getX() + i * lfoCw, knobArea.getY(), lfoCw, knobArea.getHeight()).reduced (3, 2);
                k->name.setBounds (cell.removeFromBottom (14));
                k->name.setJustificationType (juce::Justification::centred);
                k->name.setVisible (true);
                k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (36, lfoCw - 10), 12);
                k->s.setBounds (cell);
                k->s.setVisible (true);
            }
        }
        lfoShapeEditor.setBounds (r.reduced (2));
    }

    #include "PluginEditorFullB_FxLayout.inl"

    // MAGIC: pad + combo presets only (8 mode/hold buttons removed per UX)
    {
        auto r = magicTab.getLocalBounds().reduced (6);
        // Hide legacy mode / hold buttons
        magicLoopBtn.setVisible (false);
        magicGlitchBtn.setVisible (false);
        magicFlangeBtn.setVisible (false);
        magicPsychBtn.setVisible (false);
        magicHold.setVisible (false);
        for (int c = 0; c < magicTab.getNumChildComponents(); ++c)
            if (auto* b = dynamic_cast<juce::TextButton*> (magicTab.getChildComponent (c)))
            {
                auto t = b->getButtonText();
                if (t.startsWith ("H-") || t == "LOOP" || t == "GLITCH" || t == "FLANGE" || t == "PSY" || t == "HOLD")
                    b->setVisible (false);
            }
        magicHint.setVisible (false);

        // Combo presets strip bottom
        {
            auto row = r.removeFromBottom (52).reduced (2, 1);
            const int n = 14;
            const int cw = juce::jmax (36, row.getWidth() / n);
            for (int i = 0; i < n; ++i)
                if (auto* c = magicTab.findChildWithID ("magicCombo" + juce::String (i)))
                {
                    c->setVisible (true);
                    c->setBounds (row.getX() + i * cw, row.getY(), cw - 2, row.getHeight());
                }
        }
        if (magicPad != nullptr)
        {
            magicPad->setBounds (r.reduced (4));
            magicPad->setAudioPeak (processor.getOutputPeak());
            magicPad->setVisible (true);
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

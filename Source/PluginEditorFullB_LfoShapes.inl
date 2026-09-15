        // 12 shape preset buttons in two rows of 6
        auto presetRow1 = r.removeFromTop (26);
        auto presetRow2 = r.removeFromTop (26);
        const int bw = presetRow1.getWidth() / 6;
        juce::TextButton* row1[] = {
            &lfoPresetSine, &lfoPresetTri, &lfoPresetSaw,
            &lfoPresetSqr, &lfoPresetPulse, &lfoPresetExp
        };
        juce::TextButton* row2[] = {
            &lfoPresetLog, &lfoPresetBell, &lfoPresetWob,
            &lfoPresetChaos, &lfoPresetGate, &lfoPresetCustom
        };
        for (int i = 0; i < 6; ++i)
        {
            row1[i]->setBounds (presetRow1.removeFromLeft (bw).reduced (2));
            row2[i]->setBounds (presetRow2.removeFromLeft (bw).reduced (2));
        }

    // FX 8 rows + mode combos for Reverb/Dist
    {
        auto area = fxTab.getLocalBounds().reduced (6);
        auto modeRow = area.removeFromTop (26);
        reverbModeBox.setBounds (modeRow.removeFromLeft (140).reduced (2));
        distModeBox.setBounds (modeRow.removeFromLeft (140).reduced (2));

        const int fxStart = 45;
        struct Sec { int off; int count; };
        const Sec secs2[8] = {
            {0,3}, {3,3}, {6,3}, {9,3}, {12,3}, {15,3}, {18,3}, {21,3}
        };
        const int numSec = 8;
        const int labelW = 78;
        const int monW = 72;
        const int rowH = juce::jmax (56, area.getHeight() / numSec);

        for (int s = 0; s < numSec; ++s)
        {
            auto row = area.removeFromTop (rowH).reduced (1, 1);
            if (s < fxSectionLabels.size())
            {
                auto head = row.removeFromLeft (labelW);
                fxSectionLabels[s]->setBounds (head.withSizeKeepingCentre (labelW - 4, 18));
                fxSectionLabels[s]->setVisible (true);
            }
            if (s < fxMonitors.size())
            {
                auto monArea = row.removeFromLeft (monW).reduced (1);
                fxMonitors[s]->setBounds (monArea);
                const int k0 = fxStart + secs2[s].off;
                if (k0 < (int) knobs.size())
                {
                    float nv = (float) knobs[(size_t) k0]->s.getValue();
                    float lv = juce::jlimit (0.f, 1.f, std::abs (nv) > 2.f ? std::abs (nv) / 20.f : std::abs (nv));
                    fxMonitors[s]->setLevel (lv);
                }
            }
            // REVERB row: leave space already used by mode combo at top
            const int off = secs2[s].off;
            const int cnt = secs2[s].count;
            const int cellW = juce::jmin (110, juce::jmax (64, row.getWidth() / juce::jmax (1, cnt)));
            const int cellH = juce::jmin (row.getHeight(), 68);
            for (int i = 0; i < cnt; ++i)
            {
                const int idx = fxStart + off + i;
                if (idx < 0 || idx >= (int) knobs.size()) break;
                auto* k = knobs[(size_t) idx].get();
                auto cell = juce::Rectangle<int> (row.getX() + i * cellW,
                                                  row.getCentreY() - cellH / 2,
                                                  cellW, cellH).reduced (3, 1);
                if (cell.getHeight() < 22) continue;
                k->name.setBounds (cell.removeFromBottom (11));
                k->s.setBounds (cell);
            }
        }
        for (int s = numSec; s < fxSectionLabels.size(); ++s)
            fxSectionLabels[s]->setVisible (false);
    }

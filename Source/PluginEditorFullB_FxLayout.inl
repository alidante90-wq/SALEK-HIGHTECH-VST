// Clean FX layout: 2 columns x 4 modules (Serum-style chain readability)
{
    auto area = fxTab.getLocalBounds().reduced (6);
    const int fxStart = 45;

    // Module defs: label index, knob offset from fxStart, knob count, optional mode box
    struct Mod {
        int labelIdx;
        int off;
        int count;
        int modeKind; // 0 none, 1 reverb, 2 dist
    };
    // Chorus Delay | Reverb Bassify
    // Comp EQ | Phaser Dist
    const Mod mods[8] = {
        { 0,  0, 3, 0 }, // CHORUS
        { 1,  3, 3, 0 }, // DELAY
        { 2,  6, 3, 1 }, // REVERB + mode
        { 3, 11, 1, 0 }, // BASSIFY (skip drive/gain 9,10)
        { 4, 12, 3, 0 }, // COMP
        { 5, 15, 3, 0 }, // EQ
        { 6, 18, 3, 0 }, // PHASER
        { 7, 21, 3, 2 }  // DIST + mode
    };

    const int cols = 2, rows = 4;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / rows;

    for (int m = 0; m < 8; ++m)
    {
        const int col = m % cols;
        const int row = m / cols;
        auto cell = juce::Rectangle<int> (
            area.getX() + col * cellW,
            area.getY() + row * cellH,
            cellW, cellH).reduced (4);

        // header strip: label + bypass + optional mode
        auto head = cell.removeFromTop (22);
        if (mods[m].labelIdx < fxSectionLabels.size())
        {
            auto* lab = fxSectionLabels[mods[m].labelIdx];
            if (mods[m].labelIdx == 3)
                lab->setText ("BASSIFY", juce::dontSendNotification);
            lab->setBounds (head.removeFromLeft (72));
            lab->setVisible (true);
        }
        fxBypass[mods[m].labelIdx].setBounds (head.removeFromLeft (36).reduced (2, 2));
        fxBypass[mods[m].labelIdx].setVisible (true);

        if (mods[m].modeKind == 1)
        {
            reverbModeBox.setBounds (head.removeFromLeft (100).reduced (2, 1));
            reverbModeBox.setVisible (true);
        }
        else if (mods[m].modeKind == 2)
        {
            distModeBox.setBounds (head.removeFromLeft (100).reduced (2, 1));
            distModeBox.setVisible (true);
        }

        // mini monitor
        if (mods[m].labelIdx < fxMonitors.size())
        {
            // COMP + EQ get wider advanced meters
            const int monW = (mods[m].labelIdx == 4 || mods[m].labelIdx == 5) ? 72 : 48;
            auto mon = cell.removeFromLeft (monW).reduced (2);
            fxMonitors[mods[m].labelIdx]->setBounds (mon);
            fxMonitors[mods[m].labelIdx]->setVisible (true);
            const int k0 = fxStart + mods[m].off;
            if (k0 < (int) knobs.size())
            {
                float nv = (float) knobs[(size_t) k0]->s.getValue();
                float lv = juce::jlimit (0.f, 1.f, std::abs (nv) > 2.f ? std::abs (nv) / 20.f : std::abs (nv));
                fxMonitors[mods[m].labelIdx]->setLevel (lv);
            }
        }

        // knobs fill rest
        const int cnt = mods[m].count;
        const int kw = juce::jmax (50, cell.getWidth() / juce::jmax (1, cnt));
        for (int i = 0; i < cnt; ++i)
        {
            const int idx = fxStart + mods[m].off + i;
            if (idx < 0 || idx >= (int) knobs.size()) break;
            auto* k = knobs[(size_t) idx].get();
            auto kc = juce::Rectangle<int> (cell.getX() + i * kw, cell.getY(), kw, cell.getHeight()).reduced (3);
            k->name.setBounds (kc.removeFromBottom (12));
            k->s.setBounds (kc);
            k->s.setVisible (true);
            k->name.setVisible (true);
        }
    }

    // hide master drive/gain from FX (live in header)
    if (fxStart + 9 < (int) knobs.size())
    {
        knobs[(size_t)(fxStart + 9)]->s.setVisible (false);
        knobs[(size_t)(fxStart + 9)]->name.setVisible (false);
    }
    if (fxStart + 10 < (int) knobs.size())
    {
        knobs[(size_t)(fxStart + 10)]->s.setVisible (false);
        knobs[(size_t)(fxStart + 10)]->name.setVisible (false);
    }
}

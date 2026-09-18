// FX layout: 2 cols x 4 rows — knobs created in FxSections starting at index 58
{
    auto area = fxTab.getLocalBounds().reduced (6);

    // 58-60 CHORUS | 61-63 DELAY | 64-66 REVERB | 67 BASSIFY
    // 68-71 COMP   | 72-74 EQ    | 75-77 PHASER | 78-80 DIST
    struct Mod {
        int labelIdx;
        int knobIdx[4];
        int knobCount;
        int modeKind; // 0 none, 1 reverb, 2 dist
    };
    const Mod mods[8] = {
        { 0, { 58, 59, 60, -1 }, 3, 0 }, // CHORUS
        { 1, { 61, 62, 63, -1 }, 3, 0 }, // DELAY
        { 2, { 64, 65, 66, -1 }, 3, 1 }, // REVERB
        { 3, { 67, -1, -1, -1 }, 1, 0 }, // BASSIFY
        { 4, { 68, 69, 70, 71 }, 4, 0 }, // COMP
        { 5, { 72, 73, 74, -1 }, 3, 0 }, // EQ
        { 6, { 75, 76, 77, -1 }, 3, 0 }, // PHASER
        { 7, { 78, 79, 80, -1 }, 3, 2 }  // DIST
    };

    const int cols = 2, rows = 4;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / rows;

    for (int i = 58; i <= 80 && i < (int) knobs.size(); ++i)
    {
        knobs[(size_t) i]->s.setVisible (false);
        knobs[(size_t) i]->name.setVisible (false);
    }

    for (int m = 0; m < 8; ++m)
    {
        const int col = m % cols;
        const int row = m / cols;
        auto cell = juce::Rectangle<int> (
            area.getX() + col * cellW,
            area.getY() + row * cellH,
            cellW, cellH).reduced (4);

        auto head = cell.removeFromTop (22);
        if (mods[m].labelIdx < fxSectionLabels.size())
        {
            auto* lab = fxSectionLabels[mods[m].labelIdx];
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

        if (mods[m].labelIdx < fxMonitors.size())
        {
            const int monW = (mods[m].labelIdx == 4 || mods[m].labelIdx == 5) ? 88 : 48;
            auto mon = cell.removeFromLeft (monW).reduced (2);
            fxMonitors[mods[m].labelIdx]->setBounds (mon);
            fxMonitors[mods[m].labelIdx]->setVisible (true);
        }

        // place knobs in remaining cell
        const int n = mods[m].knobCount;
        if (n <= 0) continue;
        const int kw = cell.getWidth() / n;
        const int kh = juce::jmin (cell.getHeight(), 78);
        for (int k = 0; k < n; ++k)
        {
            const int idx = mods[m].knobIdx[k];
            if (idx < 0 || idx >= (int) knobs.size()) continue;
            auto* knob = knobs[(size_t) idx].get();
            auto box = juce::Rectangle<int> (
                cell.getX() + k * kw, cell.getY() + (cell.getHeight() - kh) / 2, kw, kh).reduced (3);
            auto nameArea = box.removeFromBottom (14);
            knob->name.setBounds (nameArea);
            knob->s.setBounds (box);
            knob->s.setVisible (true);
            knob->name.setVisible (true);
        }
    }
}

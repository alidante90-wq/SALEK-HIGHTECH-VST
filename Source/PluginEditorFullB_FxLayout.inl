// Clean FX layout: 2 columns x 4 modules — CORRECT knob indices from FullA order
// FX knobs start at index 51 (after OSC 0-26, filter 27-30, ADSR 31-34, MOD 35-44, LFO 45-50)
{
    auto area = fxTab.getLocalBounds().reduced (6);

    // Absolute knob indices for each FX module (must match addKnob order in FxSections.inl)
    // Chorus 51-53 | Delay 54-56 | Reverb 57-59 | Bassify 62
    // Comp 63-66   | EQ 74-76    | Phaser 77-79  | Dist 80-82
    struct Mod {
        int labelIdx;
        int knobIdx[4];   // up to 4 knobs, -1 = unused
        int knobCount;
        int modeKind;     // 0 none, 1 reverb, 2 dist
    };
    const Mod mods[8] = {
        { 0, { 51, 52, 53, -1 }, 3, 0 }, // CHORUS
        { 1, { 54, 55, 56, -1 }, 3, 0 }, // DELAY
        { 2, { 57, 58, 59, -1 }, 3, 1 }, // REVERB + mode
        { 3, { 62, -1, -1, -1 }, 1, 0 }, // BASSIFY
        { 4, { 63, 64, 65, 66 }, 4, 0 }, // COMP (thr/ratio/mix/depth)
        { 5, { 74, 75, 76, -1 }, 3, 0 }, // EQ
        { 6, { 77, 78, 79, -1 }, 3, 0 }, // PHASER
        { 7, { 80, 81, 82, -1 }, 3, 2 }  // DIST + mode
    };

    const int cols = 2, rows = 4;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / rows;

    // Hide all FX knobs first so leftovers don't float
    for (int i = 51; i < (int) knobs.size() && i <= 82; ++i)
    {
        knobs[(size_t) i]->s.setVisible (false);
        knobs[(size_t) i]->name.setVisible (false);
    }
    // Also hide granular (67-73) and master drive/gain from this page if present
    for (int i = 60; i <= 61; ++i)
        if (i < (int) knobs.size()) { knobs[(size_t)i]->s.setVisible(false); knobs[(size_t)i]->name.setVisible(false); }
    for (int i = 67; i <= 73; ++i)
        if (i < (int) knobs.size()) { knobs[(size_t)i]->s.setVisible(false); knobs[(size_t)i]->name.setVisible(false); }

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

        // mini monitor — COMP & EQ get wider for multi-band bars
        if (mods[m].labelIdx < fxMonitors.size())
        {
            const int monW = (mods[m].labelIdx == 4 || mods[m].labelIdx == 5) ? 80 : 48;
            auto mon = cell.removeFromLeft (monW).reduced (2);
            fxMonitors[mods[m].labelIdx]->setBounds (mon);
            fxMonitors[mods[m].labelIdx]->setVisible (true);
        }

        // knobs fill rest
        const int cnt = mods[m].knobCount;
        const int kw = juce::jmax (50, cell.getWidth() / juce::jmax (1, cnt));
        for (int i = 0; i < cnt; ++i)
        {
            const int idx = mods[m].knobIdx[i];
            if (idx < 0 || idx >= (int) knobs.size()) break;
            auto* k = knobs[(size_t) idx].get();
            auto kc = juce::Rectangle<int> (cell.getX() + i * kw, cell.getY(), kw, cell.getHeight()).reduced (3);
            k->name.setBounds (kc.removeFromBottom (12));
            k->s.setBounds (kc);
            k->s.setVisible (true);
            k->name.setVisible (true);
        }
    }
}

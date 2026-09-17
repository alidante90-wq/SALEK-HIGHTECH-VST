// FX rows: mode combos sit ON their effect row (not top bar)
// MASTER gain/drive moved to header — Bassify stays as its own row
{
    auto area = fxTab.getLocalBounds().reduced (4);

    // Row order: Chorus, Delay, Reverb(+mode), Bassify, Comp, EQ, Phaser, Dist(+mode)
    struct Sec { int off; int count; const char* name; };
    // knobs start at fxStart — master drive/gain still at off 9,10 but we skip showing them here
    const Sec secs2[] = {
        {0,3},   // chorus
        {3,3},   // delay
        {6,3},   // reverb
        {11,1},  // bassify only (index 45+11 = bassify knob) — wait: 9 drive, 10 gain, 11 bassify
        {12,3},  // comp
        {15,3},  // eq
        {18,3},  // phaser
        {21,3}   // dist
    };
    const int numSec = 8;
    const int fxStart = 45;
    const int labelW = 64;
    const int monW   = 52;
    const int bypW   = 32;
    const int modeW  = 110;
    const int rowH   = juce::jmax (44, area.getHeight() / numSec);

    // Hide top mode boxes from floating top (we place them per-row)
    // reverbModeBox / distModeBox still used below

    for (int s = 0; s < numSec; ++s)
    {
        auto row = area.removeFromTop (rowH).reduced (1, 0);

        if (s < fxSectionLabels.size())
        {
            auto head = row.removeFromLeft (labelW);
            fxSectionLabels[s]->setBounds (head.withSizeKeepingCentre (labelW - 4, 16));
            fxSectionLabels[s]->setVisible (true);
            // Rename MASTER label slot to BASSIFY
            if (s == 3)
                fxSectionLabels[s]->setText ("BASSIFY", juce::dontSendNotification);
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

        fxBypass[s].setBounds (row.removeFromLeft (bypW).reduced (2, 6));

        // Mode combo beside REVERB (s==2) and DIST (s==7)
        if (s == 2)
        {
            reverbModeBox.setBounds (row.removeFromLeft (modeW).reduced (2, 4));
            reverbModeBox.setVisible (true);
        }
        else if (s == 7)
        {
            distModeBox.setBounds (row.removeFromLeft (modeW).reduced (2, 4));
            distModeBox.setVisible (true);
        }

        const int off = secs2[s].off;
        const int cnt = secs2[s].count;
        const int cellW = juce::jmax (56, row.getWidth() / juce::jmax (1, cnt));
        const int cellH = juce::jmin (row.getHeight() - 2, 68);
        for (int i = 0; i < cnt; ++i)
        {
            const int idx = fxStart + off + i;
            if (idx < 0 || idx >= (int) knobs.size()) break;
            auto* k = knobs[(size_t) idx].get();
            auto cell = juce::Rectangle<int> (row.getX() + i * cellW,
                                              row.getCentreY() - cellH / 2,
                                              cellW, cellH).reduced (3, 1);
            if (cell.getHeight() < 20) continue;
            k->name.setBounds (cell.removeFromBottom (12));
            k->s.setBounds (cell);
            k->s.setVisible (true);
            k->name.setVisible (true);
        }
    }
    for (int s = numSec; s < fxSectionLabels.size(); ++s)
        fxSectionLabels[s]->setVisible (false);

    // Hide master_drive + master_gain knobs from FX tab (shown in header)
    if (fxStart + 9 < (int) knobs.size())
    {
        knobs[(size_t) (fxStart + 9)]->s.setVisible (false);
        knobs[(size_t) (fxStart + 9)]->name.setVisible (false);
    }
    if (fxStart + 10 < (int) knobs.size())
    {
        knobs[(size_t) (fxStart + 10)]->s.setVisible (false);
        knobs[(size_t) (fxStart + 10)]->name.setVisible (false);
    }
}

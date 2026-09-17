// FX 8 rows + mode combos + per-row Bypass — compact
{
    auto area = fxTab.getLocalBounds().reduced (4);
    auto modeRow = area.removeFromTop (24);
    reverbModeBox.setBounds (modeRow.removeFromLeft (130).reduced (2));
    distModeBox.setBounds (modeRow.removeFromLeft (130).reduced (2));

    const int fxStart = 45;
    struct Sec { int off; int count; };
    const Sec secs2[8] = {
        {0,3}, {3,3}, {6,3}, {9,3}, {12,3}, {15,3}, {18,3}, {21,3}
    };
    const int numSec = 8;
    const int labelW = 68;
    const int monW   = 56;
    const int bypW   = 34;
    const int rowH   = juce::jmax (48, area.getHeight() / numSec);

    for (int s = 0; s < numSec; ++s)
    {
        auto row = area.removeFromTop (rowH).reduced (1, 0);

        if (s < fxSectionLabels.size())
        {
            auto head = row.removeFromLeft (labelW);
            fxSectionLabels[s]->setBounds (head.withSizeKeepingCentre (labelW - 4, 16));
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

        fxBypass[s].setBounds (row.removeFromLeft (bypW).reduced (2, 8));

        const int off = secs2[s].off;
        const int cnt = secs2[s].count;
        const int cellW = juce::jmin (100, juce::jmax (56, row.getWidth() / juce::jmax (1, cnt)));
        const int cellH = juce::jmin (row.getHeight() - 2, 56);
        for (int i = 0; i < cnt; ++i)
        {
            const int idx = fxStart + off + i;
            if (idx < 0 || idx >= (int) knobs.size()) break;
            auto* k = knobs[(size_t) idx].get();
            auto cell = juce::Rectangle<int> (row.getX() + i * cellW,
                                              row.getCentreY() - cellH / 2,
                                              cellW, cellH).reduced (2, 1);
            if (cell.getHeight() < 20) continue;
            k->name.setBounds (cell.removeFromBottom (11));
            k->s.setBounds (cell);
        }
    }
    for (int s = numSec; s < fxSectionLabels.size(); ++s)
        fxSectionLabels[s]->setVisible (false);
}

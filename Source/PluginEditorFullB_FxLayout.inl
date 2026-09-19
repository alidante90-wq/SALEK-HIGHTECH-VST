// FX layout by PARAMETER ID (never by fragile index)
{
    auto area = fxTab.getLocalBounds().reduced (6);

    auto byId = [this] (const char* id) -> Knob*
    {
        for (auto& k : knobs)
            if (k != nullptr && k->paramId == id)
                return k.get();
        return nullptr;
    };
    auto placeKnob = [] (Knob* k, juce::Rectangle<int> box)
    {
        if (k == nullptr) return;
        auto nameArea = box.removeFromBottom (14);
        k->name.setBounds (nameArea);
        k->s.setBounds (box);
        k->s.setVisible (true);
        k->name.setVisible (true);
    };

    // hide all known FX knobs first
    const char* allFx[] = {
        "chorus_mix","chorus_rate","chorus_depth",
        "delay_mix","delay_time","delay_fb",
        "reverb_mix","reverb_size","reverb_decay",
        "bassify",
        "comp_threshold","comp_ratio","comp_mix","comp_depth",
        "eq_low","eq_mid","eq_high",
        "phaser_mix","phaser_rate","phaser_depth",
        "dist_mix","dist_drive","dist_crush"
    };
    for (auto* id : allFx)
        if (auto* k = byId (id))
        {
            k->s.setVisible (false);
            k->name.setVisible (false);
        }

    struct Mod {
        int labelIdx;
        const char* ids[4];
        int count;
        int modeKind;
    };
    const Mod mods[8] = {
        { 0, { "chorus_mix", "chorus_rate", "chorus_depth", nullptr }, 3, 0 },
        { 1, { "delay_mix",  "delay_time",  "delay_fb",     nullptr }, 3, 0 },
        { 2, { "reverb_mix", "reverb_size", "reverb_decay", nullptr }, 3, 1 },
        { 3, { "bassify",    nullptr,       nullptr,        nullptr }, 1, 0 },
        { 4, { "comp_threshold","comp_ratio","comp_mix","comp_depth" }, 4, 0 },
        { 5, { "eq_low", "eq_mid", "eq_high", nullptr }, 3, 0 },
        { 6, { "phaser_mix", "phaser_rate", "phaser_depth", nullptr }, 3, 0 },
        { 7, { "dist_mix", "dist_drive", "dist_crush", nullptr }, 3, 2 }
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

        auto head = cell.removeFromTop (22);
        if (mods[m].labelIdx < fxSectionLabels.size())
        {
            fxSectionLabels[mods[m].labelIdx]->setBounds (head.removeFromLeft (72));
            fxSectionLabels[mods[m].labelIdx]->setVisible (true);
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
            fxMonitors[mods[m].labelIdx]->setBounds (cell.removeFromLeft (monW).reduced (2));
            fxMonitors[mods[m].labelIdx]->setVisible (true);
        }

        const int n = mods[m].count;
        if (n <= 0) continue;
        const int kw = cell.getWidth() / n;
        const int kh = juce::jmin (cell.getHeight(), 78);
        for (int k = 0; k < n; ++k)
        {
            auto box = juce::Rectangle<int> (
                cell.getX() + k * kw,
                cell.getY() + (cell.getHeight() - kh) / 2,
                kw, kh).reduced (3);
            placeKnob (byId (mods[m].ids[k]), box);
        }
    }
}

// FX layout — compact knobs, full coverage of all FX params
{
    auto fullFx = fxTab.getLocalBounds().reduced (4);
    auto area = fullFx;

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
        // compact: smaller name + value box
        auto nameArea = box.removeFromBottom (11);
        k->name.setBounds (nameArea);
        k->name.setFont (juce::Font (9.0f));
        k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, juce::jmax (28, box.getWidth() - 4), 10);
        k->s.setNumDecimalPlacesToDisplay (2);
        k->s.textFromValueFunction = [] (double v)
        {
            if (std::abs (v - std::round (v)) < 1e-3)
                return juce::String ((int) std::round (v));
            return juce::String (v, 2);
        };
        k->s.setBounds (box);
        k->s.setVisible (true);
        k->name.setVisible (true);
    };

    // hide all FX knobs first
    const char* allFx[] = {
        "chorus_mix","chorus_rate","chorus_depth",
        "delay_mix","delay_time","delay_fb","delay_time_l","delay_time_r",
        "reverb_mix","reverb_size","reverb_decay","reverb_damping",
        "bassify","spatial_azim","spatial_dist","spatial_size","spatial_elev",
        "comp_threshold","comp_ratio","comp_mix","comp_depth","comp_gain","comp_attack","comp_release",
        "comp_thr_lo","comp_thr_mid","comp_thr_hi",
        "eq_low","eq_mid","eq_high",
        "phaser_mix","phaser_rate","phaser_depth",
        "dist_mix","dist_drive","dist_crush","formant_morph","formant_amt",
        "res_mix","res_decay","res_bright","res_freq",
        "spectral_mix","spectral_amt","spectral_shift","spectral_gate",
        "granular_mix","granular_density","granular_size","granular_pos"
    };
    for (auto* id : allFx)
        if (auto* k = byId (id))
        {
            k->s.setVisible (false);
            k->name.setVisible (false);
        }

    // 12 modules in 3 cols x 4 rows — more slots for complete controls
    struct Mod {
        int labelIdx; // icon/bypass index 0..7 (reuse when >7)
        const char* ids[6];
        int count;
        int modeKind; // 0 none, 1 reverb, 2 dist, 3 delay
    };
    const Mod mods[12] = {
        // row0
        { 0, { "chorus_mix", "chorus_rate", "chorus_depth", nullptr, nullptr, nullptr }, 3, 0 },
        { 1, { "delay_mix", "delay_time", "delay_fb", "delay_time_l", "delay_time_r", nullptr }, 5, 3 },
        { 2, { "reverb_mix", "reverb_size", "reverb_decay", "reverb_damping", nullptr, nullptr }, 4, 1 },
        // row1
        { 3, { "bassify", "spatial_azim", "spatial_dist", "spatial_size", "spatial_elev", nullptr }, 5, 0 },
        { 4, { "comp_mix", "comp_threshold", "comp_ratio", "comp_gain", "comp_attack", "comp_release" }, 6, 0 },
        { 5, { "eq_low", "eq_mid", "eq_high", "comp_thr_lo", "comp_thr_mid", "comp_thr_hi" }, 6, 0 },
        // row2
        { 6, { "phaser_mix", "phaser_rate", "phaser_depth", "comp_depth", nullptr, nullptr }, 4, 0 },
        { 7, { "dist_mix", "dist_drive", "dist_crush", "formant_morph", "formant_amt", nullptr }, 5, 2 },
        { 0, { "res_mix", "res_freq", "res_decay", "res_bright", nullptr, nullptr }, 4, 0 },
        // row3
        { 1, { "spectral_mix", "spectral_amt", "spectral_shift", "spectral_gate", nullptr, nullptr }, 4, 0 },
        { 2, { "granular_mix", "granular_density", "granular_size", "granular_pos", nullptr, nullptr }, 4, 0 },
        { 3, { "chorus_depth", "phaser_depth", "dist_crush", nullptr, nullptr, nullptr }, 0, 0 } // spacer / unused
    };

    const int cols = 3, rows = 4;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / rows;

    for (int m = 0; m < 12; ++m)
    {
        if (mods[m].count <= 0) continue;
        const int col = m % cols;
        const int row = m / cols;
        auto cell = juce::Rectangle<int> (
            area.getX() + col * cellW,
            area.getY() + row * cellH,
            cellW, cellH).reduced (3);

        auto head = cell.removeFromTop (18);
        const int li = mods[m].labelIdx;
        if (li < fxSectionIcons.size())
        {
            fxSectionIcons[li]->setBounds (head.removeFromLeft (18).reduced (1));
            fxSectionIcons[li]->setVisible (true);
            fxSectionIcons[li]->toFront (false);
        }
        if (li < fxSectionLabels.size() && row < 3 && col < 2)
        {
            // only first occurrence keeps big label space
            fxSectionLabels[li]->setBounds (head.removeFromLeft (48));
            fxSectionLabels[li]->setVisible (true);
        }
        if (li < 8)
            fxBypass[li].setBounds (head.removeFromLeft (32).reduced (1, 1));
        if (li < 8)
            fxBypass[li].setVisible (true);

        if (mods[m].modeKind == 1)
        {
            reverbModeBox.setBounds (head.removeFromLeft (88).reduced (1, 0));
            reverbModeBox.setVisible (true);
        }
        else if (mods[m].modeKind == 2)
        {
            distModeBox.setBounds (head.removeFromLeft (88).reduced (1, 0));
            distModeBox.setVisible (true);
        }
        else if (mods[m].modeKind == 3)
        {
            delayModeBox.setBounds (head.removeFromLeft (88).reduced (1, 0));
            delayModeBox.setVisible (true);
        }

        // smaller monitors only for main 8
        if (m < 8 && li < fxMonitors.size())
        {
            const int monW = (li == 4 || li == 5) ? 56 : 36;
            if (cell.getWidth() > monW + 40)
            {
                fxMonitors[li]->setBounds (cell.removeFromLeft (monW).reduced (1));
                fxMonitors[li]->setVisible (true);
            }
        }

        const int n = mods[m].count;
        const int kw = juce::jmax (1, cell.getWidth() / n);
        const int kh = juce::jmin (cell.getHeight(), 62); // smaller knobs
        for (int k = 0; k < n; ++k)
        {
            if (mods[m].ids[k] == nullptr) continue;
            auto box = juce::Rectangle<int> (
                cell.getX() + k * kw,
                cell.getY() + (cell.getHeight() - kh) / 2,
                kw, kh).reduced (2);
            placeKnob (byId (mods[m].ids[k]), box);
        }
    }
}

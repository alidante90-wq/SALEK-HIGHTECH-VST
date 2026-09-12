    {
        auto kbArea = full.removeFromBottom (78).reduced (2, 2);
        const int whiteKeys = 52;
        int keyW = juce::jmax (12, kbArea.getWidth() / whiteKeys);
        keyboard.setKeyWidth ((float) keyW);
        keyboard.setBounds (kbArea);
    }
    full.removeFromBottom (4);

    auto a = full;
    a.removeFromLeft (182);
    title.setVisible (false);
    tagline.setVisible (false);

    auto header = a.removeFromTop (44);
    themeBox.setBounds (header.removeFromRight (150).reduced (2));
    scope.setBounds (header.removeFromRight (130).reduced (3));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (260).reduced (2));

    a.removeFromTop (4);
    tabs.setBounds (a);
    tabs.toFront (false);
    keyboard.toFront (false);

    {
        auto b = mainTab.getLocalBounds().reduced (8);
        presetTab.setBounds (b.removeFromLeft (220));
        envTab.setBounds (b.removeFromBottom (150));
        filterTab.setBounds (b.removeFromRight (280));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (6);
            auto top = pb.removeFromTop (36);
            prevPreset.setBounds (top.removeFromLeft (36).reduced (2));
            nextPreset.setBounds (top.removeFromLeft (36).reduced (2));
            initBtn.setBounds (top.removeFromLeft (56).reduced (2));
            presetLabel.setBounds (top.reduced (2));
            presetList.setBounds (pb);
        }

        auto layoutKnobs = [] (juce::Component& parent, std::vector<std::unique_ptr<Knob>>& all, int start, int count)
        {
            auto r = parent.getLocalBounds().reduced (6);
            if (count <= 0) return;
            int cols = juce::jmin (count, 5);
            int rows = (count + cols - 1) / cols;
            int cellW = juce::jmax (1, r.getWidth() / cols);
            int cellH = juce::jmax (1, r.getHeight() / juce::jmax (1, rows));
            for (int i = 0; i < count; ++i)
            {
                int idx = start + i;
                if (idx >= (int) all.size()) break;
                auto* k = all[(size_t) idx].get();
                int c = i % cols, row = i / cols;
                auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (4);
                k->name.setBounds (cell.removeFromBottom (16));
                k->s.setBounds (cell);
            }
        };

        layoutKnobs (oscTab, knobs, 0, 17);
        layoutKnobs (filterTab, knobs, 17, 4);
        layoutKnobs (envTab, knobs, 21, 4);
        if (filterDisplay != nullptr)
            filterDisplay->setBounds (filterTab.getLocalBounds().removeFromTop (90).reduced (4));
        if (adsrDisplay != nullptr)
            adsrDisplay->setBounds (envTab.getLocalBounds().removeFromTop (70).reduced (4));
    }

    {
        auto r = modTab.getLocalBounds().reduced (4);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (80).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (280).reduced (2));
        int x = r.getX(), y = r.getY(), w = 90, h = 100;
        for (int i = 25; i < juce::jmin (37, (int) knobs.size()); ++i)
        {
            auto* k = knobs[(size_t) i].get();
            k->s.setBounds (x, y, w, h - 18);
            k->name.setBounds (x, y + h - 18, w, 16);
            x += w;
            if (x + w > r.getRight()) { x = r.getX(); y += h; }
        }
    }

    {
        auto r = fxTab.getLocalBounds().reduced (8);
        int cols = 4, i0 = 37;
        int n = juce::jmin (22, (int) knobs.size() - i0);
        int cellW = juce::jmax (1, r.getWidth() / cols);
        int cellH = 110;
        for (int i = 0; i < n; ++i)
        {
            auto* k = knobs[(size_t) (i0 + i)].get();
            int c = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (4);
            k->name.setBounds (cell.removeFromBottom (16));
            k->s.setBounds (cell);
        }
    }

    {
        auto bounds = seqTab.getLocalBounds().reduced (4);
        auto top = bounds.removeFromTop (28);
        arpOn.setBounds (top.removeFromLeft (80).reduced (2));
        seqOn.setBounds (top.removeFromLeft (80).reduced (2));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (4));
    }

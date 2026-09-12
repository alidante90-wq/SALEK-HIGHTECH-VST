int SalekHightechAudioProcessorEditor::getNumRows() { return presetRows.size(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);
    if (pr.isHeader)
    {
        g.fillAll (juce::Colour (0xff12081c));
        g.setColour (juce::Colour (0xff00f0ff));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("  " + pr.label, 6, 0, width - 10, height, juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffff2d6a).withAlpha (0.5f));
        g.drawLine (4.0f, (float) height - 1.0f, (float) width - 4.0f, (float) height - 1.0f, 1.0f);
        return;
    }
    if (selected) g.fillAll (juce::Colour (0xff3a0066));
    else g.fillAll (juce::Colour (0xff0a0614));
    juce::Colour tc = selected ? juce::Colour (0xff00f0ff) : juce::Colour (0xffd0c0e0);
    auto fullName = processor.getProgramName (pr.programIndex);
    if (fullName.startsWith ("Acid")) tc = selected ? tc : juce::Colour (0xffffcc44);
    if (fullName.startsWith ("Bass")) tc = selected ? tc : juce::Colour (0xff66ff99);
    if (fullName.startsWith ("Lead")) tc = selected ? tc : juce::Colour (0xff00f0ff);
    if (fullName.startsWith ("Kick")) tc = selected ? tc : juce::Colour (0xffffab40);
    if (fullName.startsWith ("FM"))   tc = selected ? tc : juce::Colour (0xffff66cc);
    if (fullName.startsWith ("Retro"))tc = selected ? tc : juce::Colour (0xffa0a0ff);
    g.setColour (tc);
    g.setFont (juce::FontOptions (12.5f));
    g.drawText ("   " + pr.label, 8, 0, width - 12, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);
    if (pr.isHeader || pr.programIndex < 0) return;
    processor.setCurrentProgram (pr.programIndex);
    presetLabel.setText (processor.getProgramName (pr.programIndex), juce::dontSendNotification);
}

#include "PluginEditorPaint.inl"

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 5) resized(); // only initial layout pass
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto full = getLocalBounds();

    // Keyboard bottom
    {
        auto kbArea = full.removeFromBottom (72).reduced (2, 2);
        const int whiteKeys = 52;
        int keyW = juce::jmax (11, kbArea.getWidth() / whiteKeys);
        keyboard.setKeyWidth ((float) keyW);
        keyboard.setBounds (kbArea);
    }
    full.removeFromBottom (8); // room for ISATIS text

    // Left branding panel reserved (matches paint logo panel)
    full.removeFromLeft (230);

    // Top header
    auto header = full.removeFromTop (40);
    themeBox.setBounds (header.removeFromRight (140).reduced (3));
    scope.setBounds (header.removeFromRight (120).reduced (3));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (220).reduced (2));

    full.removeFromTop (4);

    // Tabs take remaining
    tabs.setBounds (full);
    tabs.toFront (false);
    keyboard.toFront (false);

    // MAIN tab — clean non-overlapping layout
    {
        auto b = mainTab.getLocalBounds().reduced (6);
        presetTab.setBounds (b.removeFromLeft (190));
        filterTab.setBounds (b.removeFromRight (240));
        envTab.setBounds (b.removeFromBottom (130));
        oscTab.setBounds (b);

        // Preset list
        {
            auto pb = presetTab.getLocalBounds().reduced (4);
            auto top = pb.removeFromTop (30);
            prevPreset.setBounds (top.removeFromLeft (30).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (30).reduced (1));
            initBtn.setBounds (top.removeFromLeft (48).reduced (1));
            presetLabel.setBounds (top.reduced (1));
            presetList.setBounds (pb);
        }

        auto layoutKnobsIn = [] (juce::Rectangle<int> r, std::vector<std::unique_ptr<Knob>>& all, int start, int count, int maxCols)
        {
            if (count <= 0 || r.isEmpty()) return;
            int cols = juce::jmin (count, maxCols);
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
                k->name.setBounds (cell.removeFromBottom (13));
                k->s.setBounds (cell);
            }
        };

        // OSC: full area, 5 columns
        layoutKnobsIn (oscTab.getLocalBounds().reduced (4), knobs, 0, 17, 5);

        // FILTER: curve on top, knobs below
        {
            auto fr = filterTab.getLocalBounds().reduced (4);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (70).reduced (2));
            layoutKnobsIn (fr, knobs, 17, 4, 2);
        }

        // ENV: curve on top, knobs below
        {
            auto er = envTab.getLocalBounds().reduced (4);
            if (adsrDisplay != nullptr)
                adsrDisplay->setBounds (er.removeFromTop (55).reduced (2));
            layoutKnobsIn (er, knobs, 21, 4, 4);
        }
    }

    // MOD tab
    {
        auto r = modTab.getLocalBounds().reduced (4);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (70).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (260).reduced (2));
        int x = r.getX(), y = r.getY(), w = 88, h = 95;
        for (int i = 25; i < juce::jmin (37, (int) knobs.size()); ++i)
        {
            auto* k = knobs[(size_t) i].get();
            k->s.setBounds (x, y, w, h - 16);
            k->name.setBounds (x, y + h - 16, w, 14);
            x += w;
            if (x + w > r.getRight()) { x = r.getX(); y += h; }
        }
    }

    // FX tab
    {
        auto r = fxTab.getLocalBounds().reduced (6);
        int cols = 4, i0 = 37;
        int n = juce::jmin (22, (int) knobs.size() - i0);
        int cellW = juce::jmax (1, r.getWidth() / cols);
        int cellH = 105;
        for (int i = 0; i < n; ++i)
        {
            auto* k = knobs[(size_t) (i0 + i)].get();
            int c = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (3);
            k->name.setBounds (cell.removeFromBottom (14));
            k->s.setBounds (cell);
        }
    }

    // SEQ tab
    {
        auto bounds = seqTab.getLocalBounds().reduced (4);
        auto top = bounds.removeFromTop (26);
        arpOn.setBounds (top.removeFromLeft (70).reduced (2));
        seqOn.setBounds (top.removeFromLeft (70).reduced (2));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (3));
    }

    if (glBackdrop != nullptr)
        glBackdrop->setBounds (0, 0, 1, 1);
}

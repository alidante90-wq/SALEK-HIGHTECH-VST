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
    if (++ticks < 40) resized();
}

void SalekHightechAudioProcessorEditor::resized()
{
    if (glBackdrop != nullptr)
        glBackdrop->setBounds (0, 0, 1, 1);

    auto full = getLocalBounds().reduced (6);
    if (sonicCore != nullptr)
    {
        auto core = full;
        core.removeFromTop (juce::jmax (0, core.getHeight() - 110));
        core = core.withSizeKeepingCentre (juce::jmin (420, core.getWidth()), 90);
        core.translate (0, -80);
        sonicCore->setBounds (core);
        sonicCore->toBack();
    }
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

    // MAIN: presets column (expandable library) + osc/filter/env
    {
        auto b = mainTab.getLocalBounds().reduced (6);
        presetTab.setBounds (b.removeFromLeft (240));
        envTab.setBounds (b.removeFromBottom (150));
        filterTab.setBounds (b.removeFromRight (280));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (4);
            auto top = pb.removeFromTop (32);
            prevPreset.setBounds (top.removeFromLeft (34).reduced (2));
            nextPreset.setBounds (top.removeFromLeft (34).reduced (2));
            initBtn.setBounds (top.removeFromLeft (52).reduced (2));
            presetLabel.setBounds (top.reduced (2));
            presetFilterBox.setBounds (pb.removeFromTop (26).reduced (2));
            presetList.setBounds (pb.reduced (2));
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
                auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (3);
                k->name.setBounds (cell.removeFromBottom (14));
                k->s.setBounds (cell);
            }
        };

        layoutKnobs (oscTab, knobs, 0, 17);
        layoutKnobs (filterTab, knobs, 17, 4);
        layoutKnobs (envTab, knobs, 21, 4);
        if (filterDisplay != nullptr)
            filterDisplay->setBounds (filterTab.getLocalBounds().removeFromTop (80).reduced (4));
        if (adsrDisplay != nullptr)
            adsrDisplay->setBounds (envTab.getLocalBounds().removeFromTop (64).reduced (4));
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
}

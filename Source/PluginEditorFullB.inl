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
    if (++ticks < 5) resized();
}

void SalekHightechAudioProcessorEditor::resized()
{
    auto full = getLocalBounds();

    {
        auto kb = full.removeFromBottom (70).reduced (2, 2);
        int keyW = juce::jmax (11, kb.getWidth() / 52);
        keyboard.setKeyWidth ((float) keyW);
        keyboard.setBounds (kb);
    }
    full.removeFromBottom (6);
    full.removeFromLeft (228);

    auto header = full.removeFromTop (38);
    themeBox.setBounds (header.removeFromRight (130).reduced (3));
    scope.setBounds (header.removeFromRight (110).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (200).reduced (2));

    full.removeFromTop (2);
    tabs.setBounds (full);
    tabs.toFront (false);
    keyboard.toFront (false);

    auto placeKnobs = [] (juce::Rectangle<int> area, std::vector<std::unique_ptr<Knob>>& all,
                          int start, int count, int cols)
    {
        if (count <= 0 || area.isEmpty() || cols < 1) return;
        int rows = (count + cols - 1) / cols;
        int cw = juce::jmax (1, area.getWidth() / cols);
        int ch = juce::jmax (1, area.getHeight() / juce::jmax (1, rows));
        for (int i = 0; i < count; ++i)
        {
            int idx = start + i;
            if (idx >= (int) all.size()) break;
            auto* k = all[(size_t) idx].get();
            int c = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (area.getX() + c * cw, area.getY() + row * ch, cw, ch).reduced (5);
            k->name.setBounds (cell.removeFromBottom (12));
            k->s.setBounds (cell);
        }
    };

    // MAIN
    {
        auto b = mainTab.getLocalBounds().reduced (5);
        presetTab.setBounds (b.removeFromLeft (185));
        filterTab.setBounds (b.removeFromRight (230));
        envTab.setBounds (b.removeFromBottom (125));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (3);
            auto top = pb.removeFromTop (28);
            prevPreset.setBounds (top.removeFromLeft (28).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (28).reduced (1));
            initBtn.setBounds (top.removeFromLeft (46).reduced (1));
            presetLabel.setBounds (top.reduced (1));
            presetList.setBounds (pb);
        }

        placeKnobs (oscTab.getLocalBounds().reduced (4), knobs, 0, 17, 5);

        {
            auto fr = filterTab.getLocalBounds().reduced (4);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (68).reduced (2));
            placeKnobs (fr, knobs, 17, 4, 2);
        }
        {
            auto er = envTab.getLocalBounds().reduced (4);
            if (adsrDisplay != nullptr)
                adsrDisplay->setBounds (er.removeFromTop (50).reduced (2));
            placeKnobs (er, knobs, 21, 4, 4);
        }
    }

    // MOD
    {
        auto r = modTab.getLocalBounds().reduced (6);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (72).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (250).reduced (2));
        placeKnobs (r.reduced (2), knobs, 25, 12, 4);
    }

    // FX — clean 4-col grid
    {
        auto r = fxTab.getLocalBounds().reduced (8);
        const int totalFx = juce::jmin (22, (int) knobs.size() - 37);
        placeKnobs (r, knobs, 37, totalFx, 4);
    }

    // SEQ — ARP/SEQ + rate knobs + large step grid
    {
        auto bounds = seqTab.getLocalBounds().reduced (8);
        auto top = bounds.removeFromTop (36);
        arpOn.setBounds (top.removeFromLeft (90).reduced (3));
        seqOn.setBounds (top.removeFromLeft (90).reduced (3));

        const int seqKnobStart = 37 + juce::jmin (22, (int) knobs.size() - 37);
        int nSeqKnobs = juce::jmax (0, (int) knobs.size() - seqKnobStart);
        if (nSeqKnobs > 0)
        {
            auto knobStrip = top.removeFromRight (juce::jmin (top.getWidth(), nSeqKnobs * 100));
            placeKnobs (knobStrip, knobs, seqKnobStart, nSeqKnobs, nSeqKnobs);
        }

        bounds.removeFromTop (6);
        if (stepGrid != nullptr)
            stepGrid->setBounds (bounds.reduced (2));
    }
}

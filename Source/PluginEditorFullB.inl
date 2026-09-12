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
    if (++ticks < 8) resized();
}

void SalekHightechAudioProcessorEditor::resized()
{
    // CORRECT knob index map (from FullA addKnob order):
    // OSC  0..18  (19) | FILTER 19..22 (4) | ENV 23..26 (4)
    // MOD 27..38 (12) | FX 39..60 (22) | SEQ 61..63 (3)

    auto full = getLocalBounds();

    {
        auto kb = full.removeFromBottom (68).reduced (2, 1);
        keyboard.setKeyWidth ((float) juce::jmax (10, kb.getWidth() / 52));
        keyboard.setBounds (kb);
    }
    full.removeFromBottom (4);
    full.removeFromLeft (222);

    auto header = full.removeFromTop (36);
    themeBox.setBounds (header.removeFromRight (120).reduced (2));
    scope.setBounds (header.removeFromRight (100).reduced (2));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (190).reduced (2));

    full.removeFromTop (2);
    tabs.setBounds (full);
    tabs.toFront (false);
    keyboard.toFront (false);

    auto place = [] (juce::Rectangle<int> area,
                     std::vector<std::unique_ptr<Knob>>& all,
                     int start, int count, int cols)
    {
        if (count <= 0 || area.getWidth() < 20 || area.getHeight() < 20 || cols < 1) return;
        const int rows = (count + cols - 1) / cols;
        const int cw = area.getWidth() / cols;
        const int ch = area.getHeight() / juce::jmax (1, rows);
        for (int i = 0; i < count; ++i)
        {
            const int idx = start + i;
            if (idx < 0 || idx >= (int) all.size()) break;
            auto* k = all[(size_t) idx].get();
            const int c = i % cols;
            const int r = i / cols;
            auto cell = juce::Rectangle<int> (area.getX() + c * cw, area.getY() + r * ch, cw, ch).reduced (6);
            if (cell.getHeight() < 28) continue;
            k->name.setBounds (cell.removeFromBottom (13));
            k->s.setBounds (cell);
        }
    };

    // ---- MAIN ----
    {
        auto b = mainTab.getLocalBounds().reduced (4);
        presetTab.setBounds (b.removeFromLeft (178));
        filterTab.setBounds (b.removeFromRight (220));
        envTab.setBounds (b.removeFromBottom (118));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (3);
            auto top = pb.removeFromTop (26);
            prevPreset.setBounds (top.removeFromLeft (26).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (26).reduced (1));
            initBtn.setBounds (top.removeFromLeft (44).reduced (1));
            presetLabel.setBounds (top.reduced (1));
            presetList.setBounds (pb);
        }

        place (oscTab.getLocalBounds().reduced (4), knobs, 0, 19, 5);

        {
            auto fr = filterTab.getLocalBounds().reduced (3);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (64).reduced (2));
            place (fr, knobs, 19, 4, 2);
        }

        {
            auto er = envTab.getLocalBounds().reduced (3);
            if (adsrDisplay != nullptr)
                adsrDisplay->setBounds (er.removeFromTop (48).reduced (2));
            place (er, knobs, 23, 4, 4);
        }
    }

    // ---- MOD ----
    {
        auto r = modTab.getLocalBounds().reduced (6);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (64).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (240).reduced (2));
        place (r.reduced (2), knobs, 27, 12, 4);
    }

    // ---- FX ----
    {
        auto r = fxTab.getLocalBounds().reduced (10);
        place (r, knobs, 39, 22, 4);
    }

    // ---- SEQ ----
    {
        auto bounds = seqTab.getLocalBounds().reduced (8);
        auto top = bounds.removeFromTop (40);
        arpOn.setBounds (top.removeFromLeft (88).reduced (4));
        seqOn.setBounds (top.removeFromLeft (88).reduced (4));
        place (top.removeFromRight (300).reduced (2), knobs, 61, 3, 3);
        bounds.removeFromTop (4);
        if (stepGrid != nullptr)
            stepGrid->setBounds (bounds);
    }
}

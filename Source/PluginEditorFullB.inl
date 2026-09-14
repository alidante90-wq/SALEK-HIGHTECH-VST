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
    if (fullName.startsWith ("SALEK")) tc = selected ? tc : juce::Colour (0xffffd700);
    if (fullName.startsWith ("USER")) tc = selected ? tc : juce::Colour (0xffffaa00);
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
    animPhase += 0.04f;
    repaint();
}

void SalekHightechAudioProcessorEditor::resized()
{
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

    {
        auto b = mainTab.getLocalBounds().reduced (4);
        presetTab.setBounds (b.removeFromLeft (200));
        envTab.setBounds (b.removeFromBottom (128));
        filterTab.setBounds (b.removeFromRight (248));
        oscTab.setBounds (b);

        {
            auto pb = presetTab.getLocalBounds().reduced (3);
            auto top = pb.removeFromTop (24);
            prevPreset.setBounds (top.removeFromLeft (24).reduced (1));
            nextPreset.setBounds (top.removeFromLeft (24).reduced (1));
            initBtn.setBounds (top.removeFromLeft (36).reduced (1));
            savePresetBtn.setBounds (top.removeFromLeft (42).reduced (1));
            loadPresetBtn.setBounds (top.removeFromLeft (42).reduced (1));
            auto nameRow = pb.removeFromTop (20);
            presetLabel.setBounds (nameRow.reduced (2, 0));
            presetLabel.setJustificationType (juce::Justification::centredLeft);
            presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff00e8ff));
            presetList.setBounds (pb);
        }

        place (oscTab.getLocalBounds().reduced (4), knobs, 0, 21, 6);

        {
            auto fr = filterTab.getLocalBounds().reduced (4);
            if (filterDisplay != nullptr)
                filterDisplay->setBounds (fr.removeFromTop (72).reduced (2));
            fr.removeFromTop (4);
            place (fr, knobs, 21, 4, 2);
        }

        {
            auto er = envTab.getLocalBounds().reduced (4);
            if (adsrDisplay != nullptr)
                adsrDisplay->setBounds (er.removeFromTop (52).reduced (2));
            place (er, knobs, 25, 4, 4);
        }
    }

    {
        auto r = modTab.getLocalBounds().reduced (6);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (64).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (240).reduced (2));
        place (r.reduced (2), knobs, 29, 12, 4);
    }

    {
        auto area = fxTab.getLocalBounds().reduced (8);
        const int fxStart = 41;
        struct Sec { int off; int count; };
        const Sec secs2[10] = {
            {0,3},{3,3},{6,3},{9,2},{11,3},{14,3},{17,4},{21,1},{22,3},{25,3}
        };
        const int numSec = 10;
        const int rowH = juce::jmax (68, area.getHeight() / numSec);
        for (int s = 0; s < numSec; ++s)
        {
            auto row = area.removeFromTop (rowH).reduced (2, 1);
            if (s < fxSectionLabels.size())
            {
                auto head = row.removeFromLeft (100);
                fxSectionLabels[s]->setBounds (head.withSizeKeepingCentre (96, 22));
                fxSectionLabels[s]->setVisible (true);
            }
            int off = secs2[s].off;
            int cnt = secs2[s].count;
            const int cw = row.getWidth() / juce::jmax (1, cnt);
            for (int i = 0; i < cnt; ++i)
            {
                const int idx = fxStart + off + i;
                if (idx < 0 || idx >= (int) knobs.size()) break;
                auto* k = knobs[(size_t) idx].get();
                auto cell = juce::Rectangle<int> (row.getX() + i * cw, row.getY(), cw, row.getHeight()).reduced (3);
                if (cell.getHeight() < 28) continue;
                k->name.setBounds (cell.removeFromBottom (12));
                k->s.setBounds (cell);
            }
        }
        for (int s = numSec; s < fxSectionLabels.size(); ++s)
            fxSectionLabels[s]->setVisible (false);
    }

    {
        auto bounds = seqTab.getLocalBounds().reduced (8);
        auto top = bounds.removeFromTop (52);
        arpOn.setBounds (top.removeFromLeft (100).reduced (4));
        seqOn.setBounds (top.removeFromLeft (100).reduced (4));
        place (top.removeFromRight (360).reduced (4), knobs, 69, 3, 3);
        bounds.removeFromTop (8);
        if (stepGrid != nullptr)
            stepGrid->setBounds (bounds);
    }
}

void SalekHightechAudioProcessorEditor::applyHeroFromTheme()
{
    const int id = themeBox.getSelectedId();
    if (! lianImg.isValid()) lianImg = SalekAssets::loadLian();
    if (! faceImg.isValid()) faceImg = SalekAssets::loadFace();
    if (! cyanImg.isValid()) cyanImg = SalekAssets::loadCyanGirl();

    if (id == 2) {
        heroImg = faceImg.isValid() ? faceImg : (lianImg.isValid() ? lianImg : logoImg);
        heroIndex = 1;
    } else if (id == 3) {
        heroImg = cyanImg.isValid() ? cyanImg : (faceImg.isValid() ? faceImg : lianImg);
        heroIndex = 2;
    } else {
        heroImg = lianImg.isValid() ? lianImg : (faceImg.isValid() ? faceImg : logoImg);
        heroIndex = 0;
    }
    repaint();
}

void SalekHightechAudioProcessorEditor::cycleHero()
{
    juce::Image imgs[4] = { lianImg, faceImg, cyanImg, logoImg };
    for (int n = 0; n < 4; ++n)
    {
        heroIndex = (heroIndex + 1) % 4;
        if (imgs[heroIndex].isValid())
        {
            heroImg = imgs[heroIndex];
            repaint();
            return;
        }
    }
}

void SalekHightechAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.x >= 8 && e.x <= 222 && e.y >= 46 && e.y <= 210)
        cycleHero();
}

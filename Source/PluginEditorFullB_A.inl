int SalekHightechAudioProcessorEditor::getNumRows() { return presetRows.size(); }

void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto& pr = presetRows.getReference (row);

    if (pr.isHeader)
    {
        auto cat = pr.category.toUpperCase();
        juce::Colour catCol (0xffff2ec8);
        juce::String icon = "*";
        if (cat.contains ("BASS"))      { catCol = juce::Colour (0xff66ff99); icon = "B"; }
        else if (cat.contains ("KICK")) { catCol = juce::Colour (0xffffab40); icon = "K"; }
        else if (cat.contains ("LEAD")) { catCol = juce::Colour (0xff00e8ff); icon = "L"; }
        else if (cat.contains ("PAD"))  { catCol = juce::Colour (0xffb388ff); icon = "P"; }
        else if (cat.contains ("ACID")) { catCol = juce::Colour (0xffffcc44); icon = "A"; }
        else if (cat.contains ("FM"))   { catCol = juce::Colour (0xffff66cc); icon = "F"; }
        else if (cat.contains ("FX"))   { catCol = juce::Colour (0xffff5252); icon = "X"; }
        else if (cat.contains ("ARP"))  { catCol = juce::Colour (0xff69f0ae); icon = "R"; }
        else if (cat.contains ("SALEK") || cat.contains ("GITI")) { catCol = juce::Colour (0xffffd700); icon = "S"; }
        else if (cat.contains ("USER")) { catCol = juce::Colour (0xffffaa00); icon = "U"; }
        else if (cat.contains ("RETRO")){ catCol = juce::Colour (0xffa0a0ff); icon = "R"; }
        else if (cat.contains ("OTHER")){ catCol = juce::Colour (0xff90a4ae); icon = "."; }

        g.setColour (juce::Colour (0xdd0a0618));
        g.fillRoundedRectangle (1.f, 1.f, (float) width - 2.f, (float) height - 2.f, 6.f);
        g.setColour (catCol.withAlpha (0.55f));
        g.drawRoundedRectangle (1.5f, 1.5f, (float) width - 3.f, (float) height - 3.f, 6.f, 1.2f);

        auto badge = juce::Rectangle<float> (6.f, (float) height * 0.15f, (float) height * 0.7f, (float) height * 0.7f);
        g.setColour (catCol.withAlpha (0.25f));
        g.fillEllipse (badge);
        g.setColour (catCol);
        g.drawEllipse (badge, 1.2f);
        g.setFont (juce::FontOptions (badge.getHeight() * 0.55f, juce::Font::bold));
        g.drawText (icon, badge.toNearestInt(), juce::Justification::centred);

        const bool folded = collapsedCats.contains (pr.category);
        g.setColour (catCol.brighter (0.2f));
        g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
        juce::String mark = folded ? "> " : "v ";
        g.drawText (mark + pr.label, (int) badge.getRight() + 6, 0, width - (int) badge.getRight() - 10, height,
                    juce::Justification::centredLeft);
        return;
    }

    if (selected)
    {
        g.setColour (juce::Colour (0xbb0a2848));
        g.fillRect (0, 0, width, height);
        g.setColour (juce::Colour (0xff00e8ff));
        g.fillRect (0, 0, 4, height);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
        g.drawRect (0, 0, width, height, 1);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.15f));
        g.fillRect (4, 1, width - 5, height - 2);
    }
    else
    {
        g.setColour (juce::Colour (0x880a0614));
        g.fillRect (0, 0, width, height);
        if ((row & 1) == 0)
        {
            g.setColour (juce::Colour (0x5512081c));
            g.fillRect (0, 0, width, height);
        }
    }

    juce::Colour tc = selected ? juce::Colour (0xff00f0ff) : juce::Colour (0xffc8c0e8);
    auto fullName = processor.getProgramName (pr.programIndex);
    if (fullName.startsWith ("Acid"))  tc = selected ? tc : juce::Colour (0xffffcc44);
    if (fullName.startsWith ("Bass"))  tc = selected ? tc : juce::Colour (0xff66ff99);
    if (fullName.startsWith ("Lead"))  tc = selected ? tc : juce::Colour (0xff00f0ff);
    if (fullName.startsWith ("Kick"))  tc = selected ? tc : juce::Colour (0xffffab40);
    if (fullName.startsWith ("FM"))    tc = selected ? tc : juce::Colour (0xffff66cc);
    if (fullName.startsWith ("Retro")) tc = selected ? tc : juce::Colour (0xffa0a0ff);
    if (fullName.startsWith ("SALEK")) tc = selected ? tc : juce::Colour (0xffffd700);
    if (fullName.startsWith ("USER"))  tc = selected ? tc : juce::Colour (0xffffaa00);

    g.setColour (tc);
    g.setFont (juce::FontOptions (selected ? 13.0f : 12.2f, selected ? juce::Font::bold : juce::Font::plain));
    g.drawText (pr.label, 14, 0, width - 20, height, juce::Justification::centredLeft);
}

void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (! juce::isPositiveAndBelow (row, presetRows.size())) return;
    const auto pr = presetRows.getReference (row);
    if (pr.isHeader)
    {
        if (collapsedCats.contains (pr.category))
            collapsedCats.removeString (pr.category);
        else
            collapsedCats.add (pr.category);
        rebuildPresetRows();
        return;
    }
    if (pr.programIndex < 0) return;
    processor.setCurrentProgram (pr.programIndex);
    presetLabel.setText (processor.getProgramName (pr.programIndex), juce::dontSendNotification);
}

#include "PluginEditorPaint.inl"

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 4) resized();
    float bpmScale = 1.f;
    if (auto* ph = processor.getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (pos->getBpm())
                bpmScale = (float) (*pos->getBpm() / 120.0);
    }
    animPhase += 0.022f * juce::jlimit (0.5f, 1.8f, bpmScale);

    auto g = [&](const char* id, float d=0.f) -> float {
        if (auto* p = processor.getAPVTS().getRawParameterValue (id)) return p->load();
        return d;
    };
    if (fxMonitors.size() >= 8)
    {
        fxMonitors[0]->setLevel (g ("chorus_mix"));
        fxMonitors[1]->setLevel (g ("delay_mix"));
        fxMonitors[2]->setLevel (g ("reverb_mix"));
        fxMonitors[3]->setLevel (g ("bassify"));
        fxMonitors[4]->setLevel (g ("comp_mix"));
        fxMonitors[4]->bindCompressor (&processor.getCompressor());
        {
            auto& c = processor.getCompressor();
            auto toN = [] (float db) { return juce::jmap (db, -40.f, 0.f, 0.f, 1.f); };
            fxMonitors[4]->setBandThresholdNorms (
                toN (c.getBandThresholdDb (0)),
                toN (c.getBandThresholdDb (1)),
                toN (c.getBandThresholdDb (2)));
        }
        fxMonitors[4]->setBandGR (
            processor.getCompressor().getBandGR (0),
            processor.getCompressor().getBandGR (1),
            processor.getCompressor().getBandGR (2));
        fxMonitors[5]->setLevel (juce::jmax (std::abs (g ("eq_low")), std::abs (g ("eq_mid")), std::abs (g ("eq_high"))) / 12.f);
        fxMonitors[5]->setEqBands (g ("eq_low") / 12.f, g ("eq_mid") / 12.f, g ("eq_high") / 12.f);
        fxMonitors[6]->setLevel (g ("phaser_mix"));
        fxMonitors[7]->setLevel (g ("dist_mix"));
    }
    repaint();
}

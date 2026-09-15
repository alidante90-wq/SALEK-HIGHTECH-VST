
        if (lfoDisplay != nullptr) lfoTab.addAndMakeVisible (*lfoDisplay);
        lfoTab.addAndMakeVisible (lfoShapeEditor);
        lfoShapeEditor.setLfo (&processor.getLfo1());

        for (auto* b : { &lfoPresetSine, &lfoPresetTri, &lfoPresetSaw,
                         &lfoPresetSqr, &lfoPresetPulse, &lfoPresetCustom })
        {
            b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a0a30));
            b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
            lfoTab.addAndMakeVisible (*b);
        }
        auto loadShape = [this] (int preset)
        {
            salek::LFO* targets[3] = { &processor.getLfo1(), &processor.getLfo2(), &processor.getLfo3() };
            auto* L = targets[juce::jlimit (0, 2, lfoShapeTarget)];
            L->loadPresetShape (preset);
            lfoShapeEditor.setLfo (L);
            lfoShapeEditor.syncFromLfo();
            const char* ids[] = { "lfo_wave", "lfo2_wave", "lfo3_wave" };
            if (auto* p = processor.getAPVTS().getParameter (ids[juce::jlimit(0,2,lfoShapeTarget)]))
            {
                int waveIdx = (preset <= 3) ? preset : 5;
                if (preset == 4) waveIdx = 3;
                p->setValueNotifyingHost (p->convertTo0to1 ((float) waveIdx));
            }
        };
        lfoPresetSine.onClick  = [loadShape] { loadShape (0); };
        lfoPresetTri.onClick   = [loadShape] { loadShape (1); };
        lfoPresetSaw.onClick   = [loadShape] { loadShape (2); };
        lfoPresetSqr.onClick   = [loadShape] { loadShape (3); };
        lfoPresetPulse.onClick = [loadShape] { loadShape (4); };
        lfoPresetCustom.onClick = [this] {
            if (auto* p = processor.getAPVTS().getParameter ("lfo_wave"))
                p->setValueNotifyingHost (p->convertTo0to1 (5.f));
            lfoShapeEditor.setLfo (&processor.getLfo1());
            lfoShapeEditor.syncFromLfo();
        };

        lfo1WaveBox.addItemList ({ "Sine","Triangle","Saw","Square","S&H","Custom" }, 1);
        lfo2WaveBox.addItemList ({ "Sine","Triangle","Saw","Square","S&H","Custom" }, 1);
        lfo3WaveBox.addItemList ({ "Sine","Triangle","Saw","Square","S&H","Custom" }, 1);
        lfoTab.addAndMakeVisible (lfo1WaveBox);
        lfoTab.addAndMakeVisible (lfo2WaveBox);
        lfoTab.addAndMakeVisible (lfo3WaveBox);
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo_wave", lfo1WaveBox));
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo2_wave", lfo2WaveBox));
        comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.getAPVTS(), "lfo3_wave", lfo3WaveBox));

        lfo1WaveBox.onChange = [this] {
            if (lfo1WaveBox.getSelectedItemIndex() == 5)
            { lfoShapeTarget = 0; lfoShapeEditor.setLfo (&processor.getLfo1()); }
        };
        lfo2WaveBox.onChange = [this] {
            if (lfo2WaveBox.getSelectedItemIndex() == 5)
            { lfoShapeTarget = 1; lfoShapeEditor.setLfo (&processor.getLfo2()); }
        };
        lfo3WaveBox.onChange = [this] {
            if (lfo3WaveBox.getSelectedItemIndex() == 5)
            { lfoShapeTarget = 2; lfoShapeEditor.setLfo (&processor.getLfo3()); }
        };

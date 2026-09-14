        savePresetBtn.onClick = [this] {
            auto aw = std::make_shared<juce::AlertWindow> ("Save / Export Preset",
                "Name this preset, then choose where to save the .salek.xml file (you can send that file to anyone):",
                juce::AlertWindow::NoIcon);
            aw->addTextEditor ("name", "My Sound", "Name");
            aw->addButton ("Save File...", 1, juce::KeyPress (juce::KeyPress::returnKey));
            aw->addButton ("Bank Only", 2);
            aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
            aw->enterModalState (true, juce::ModalCallbackFunction::create ([this, aw] (int r) {
                if (r == 0) return;
                auto name = aw->getTextEditorContents ("name");
                int idx = processor.saveCurrentAsUserPreset (name);
                rebuildPresetRows();
                processor.setCurrentProgram (idx);
                presetLabel.setText (processor.getProgramName (idx), juce::dontSendNotification);
                presetList.updateContent();
                presetList.repaint();
                if (r == 1)
                {
                    auto safe = name.trim().isEmpty() ? juce::String ("MySound") : name.trim();
                    safe = safe.replaceCharacter ('/', '-').replaceCharacter ('\\', '-');
                    auto chooser = std::make_shared<juce::FileChooser> (
                        "Export SALEK preset",
                        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                            .getChildFile (safe + ".salek.xml"),
                        "*.salek.xml;*.xml");
                    chooser->launchAsync (
                        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                            | juce::FileBrowserComponent::warnAboutOverwriting,
                        [this, chooser, safe] (const juce::FileChooser& fc)
                        {
                            auto f = fc.getResult();
                            if (f == juce::File()) return;
                            if (processor.exportCurrentPresetToFile (f, safe))
                                presetLabel.setText ("Exported: " + f.getFileName(), juce::dontSendNotification);
                            else
                                presetLabel.setText ("Export failed", juce::dontSendNotification);
                        });
                }
            }));
        };
        loadPresetBtn.onClick = [this] {
            auto chooser = std::make_shared<juce::FileChooser> (
                "Load SALEK preset file",
                juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                "*.salek.xml;*.xml");
            chooser->launchAsync (
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this, chooser] (const juce::FileChooser& fc)
                {
                    auto f = fc.getResult();
                    if (f == juce::File()) return;
                    int idx = processor.importPresetFromFile (f);
                    if (idx < 0)
                    {
                        presetLabel.setText ("Invalid preset file", juce::dontSendNotification);
                        return;
                    }
                    rebuildPresetRows();
                    processor.setCurrentProgram (idx);
                    presetLabel.setText (processor.getProgramName (idx), juce::dontSendNotification);
                    presetList.updateContent();
                    presetList.repaint();
                });
        };

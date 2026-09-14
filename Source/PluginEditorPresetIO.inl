        savePresetBtn.onClick = [this] {
            auto aw = std::make_shared<juce::AlertWindow> ("Save / Export Preset",
                "Name this preset. Save File = shareable .salek.xml for others.",
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
                "Load SALEK preset / bank",
                juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                "*.salek.xml;*.salekbank.xml;*.xml");
            chooser->launchAsync (
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this, chooser] (const juce::FileChooser& fc)
                {
                    auto f = fc.getResult();
                    if (f == juce::File()) return;

                    std::unique_ptr<juce::XmlElement> probe (juce::XmlDocument::parse (f));
                    if (probe != nullptr
                        && (probe->hasTagName ("SALEK_USER_PRESETS") || probe->hasTagName ("USER_PRESETS")))
                    {
                        int n = processor.importUserBankFromFile (f);
                        rebuildPresetRows();
                        presetLabel.setText ("Bank import: " + juce::String (n) + " presets", juce::dontSendNotification);
                        presetList.updateContent();
                        presetList.repaint();
                        return;
                    }

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
        bankBtn.onClick = [this] {
            juce::PopupMenu m;
            m.addItem (1, "Export Full USER Bank...");
            m.addItem (2, "Import Bank File...");
            m.addSeparator();
            const int cur = processor.getCurrentProgram();
            const bool isUser = processor.getProgramName (cur).startsWith ("USER/");
            m.addItem (3, "Delete Selected USER Preset", isUser);
            m.addSeparator();
            m.addItem (4, "USER count: " + juce::String (processor.getNumUserPresets()), false);

            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&bankBtn),
                [this] (int result)
                {
                    if (result == 1)
                    {
                        auto chooser = std::make_shared<juce::FileChooser> (
                            "Export SALEK USER bank",
                            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                .getChildFile ("SALEK_USER_Bank.salekbank.xml"),
                            "*.salekbank.xml;*.xml");
                        chooser->launchAsync (
                            juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                | juce::FileBrowserComponent::warnAboutOverwriting,
                            [this, chooser] (const juce::FileChooser& fc)
                            {
                                auto f = fc.getResult();
                                if (f == juce::File()) return;
                                if (processor.exportUserBankToFile (f))
                                    presetLabel.setText ("Bank exported: " + f.getFileName(), juce::dontSendNotification);
                                else
                                    presetLabel.setText ("Bank export failed (empty?)", juce::dontSendNotification);
                            });
                    }
                    else if (result == 2)
                    {
                        auto chooser = std::make_shared<juce::FileChooser> (
                            "Import SALEK bank",
                            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                            "*.salekbank.xml;*.salek.xml;*.xml");
                        chooser->launchAsync (
                            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                            [this, chooser] (const juce::FileChooser& fc)
                            {
                                auto f = fc.getResult();
                                if (f == juce::File()) return;
                                int n = processor.importUserBankFromFile (f);
                                rebuildPresetRows();
                                presetLabel.setText ("Imported " + juce::String (n) + " presets", juce::dontSendNotification);
                                presetList.updateContent();
                                presetList.repaint();
                            });
                    }
                    else if (result == 3)
                    {
                        int cur = processor.getCurrentProgram();
                        auto name = processor.getProgramName (cur);
                        if (! name.startsWith ("USER/")) return;
                        if (processor.deleteUserPreset (cur))
                        {
                            rebuildPresetRows();
                            presetLabel.setText ("Deleted " + name, juce::dontSendNotification);
                            presetList.updateContent();
                            presetList.repaint();
                        }
                    }
                });
        };

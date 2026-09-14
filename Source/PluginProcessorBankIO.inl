
bool SalekHightechAudioProcessor::exportUserBankToFile (const juce::File& file)
{
    juce::XmlElement root ("SALEK_USER_PRESETS");
    root.setAttribute ("version", 2);
    root.setAttribute ("plugin", "SALEK HIGHTECH");
    root.setAttribute ("exported", juce::Time::getCurrentTime().toISO8601 (true));

    int count = 0;
    for (auto& pr : factoryPresets)
    {
        if (! pr.name.startsWith ("USER/")) continue;
        auto* px = root.createNewChildElement ("PRESET");
        px->setAttribute ("name", pr.name);
        for (auto& kv : pr.values)
        {
            auto* p = px->createNewChildElement ("PARAM");
            p->setAttribute ("id", kv.first);
            p->setAttribute ("value", (double) kv.second);
        }
        ++count;
    }

    if (count == 0)
        return false;

    auto target = file;
    if (! target.hasFileExtension (".xml") && ! target.hasFileExtension (".salekbank.xml"))
        target = target.withFileExtension ("salekbank.xml");

    auto tmp = target.getSiblingFile (target.getFileNameWithoutExtension() + ".tmp.xml");
    if (! root.writeTo (tmp))
        return false;
    return tmp.moveFileTo (target);
}

int SalekHightechAudioProcessor::importUserBankFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return 0;

    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
    if (xml == nullptr)
        return 0;

    if (xml->hasTagName ("SALEK_PRESET"))
        return importPresetFromFile (file) >= 0 ? 1 : 0;

    if (! xml->hasTagName ("SALEK_USER_PRESETS") && ! xml->hasTagName ("USER_PRESETS"))
        return 0;

    int n = 0;
    for (auto* presetXml : xml->getChildIterator())
    {
        if (! presetXml->hasTagName ("PRESET")) continue;

        FactoryPreset pr;
        pr.name = presetXml->getStringAttribute ("name");
        if (pr.name.isEmpty()) continue;
        if (! pr.name.startsWith ("USER/"))
            pr.name = "USER/" + pr.name;

        for (auto* pXml : presetXml->getChildIterator())
        {
            if (pXml->hasTagName ("PARAM"))
            {
                auto id = pXml->getStringAttribute ("id");
                if (id.isNotEmpty())
                    pr.values[id] = (float) pXml->getDoubleAttribute ("value");
            }
            else if (pXml->hasTagName ("P"))
            {
                auto id = pXml->getStringAttribute ("id");
                if (id.isNotEmpty())
                    pr.values[id] = (float) pXml->getDoubleAttribute ("v");
            }
        }
        if (pr.values.empty()) continue;

        int found = -1;
        for (int i = 0; i < (int) factoryPresets.size(); ++i)
            if (factoryPresets[(size_t) i].name == pr.name) { found = i; break; }

        if (found >= 0)
            factoryPresets[(size_t) found].values = std::move (pr.values);
        else
            factoryPresets.push_back (std::move (pr));
        ++n;
    }

    if (n > 0)
        saveUserPresetsToDisk();
    return n;
}

bool SalekHightechAudioProcessor::deleteUserPreset (int programIndex)
{
    if (programIndex < 0 || programIndex >= (int) factoryPresets.size())
        return false;
    if (! factoryPresets[(size_t) programIndex].name.startsWith ("USER/"))
        return false;

    factoryPresets.erase (factoryPresets.begin() + programIndex);
    if (currentProgram >= (int) factoryPresets.size())
        currentProgram = juce::jmax (0, (int) factoryPresets.size() - 1);
    else if (currentProgram > programIndex)
        --currentProgram;

    saveUserPresetsToDisk();
    return true;
}

int SalekHightechAudioProcessor::getNumUserPresets() const
{
    int n = 0;
    for (auto& pr : factoryPresets)
        if (pr.name.startsWith ("USER/")) ++n;
    return n;
}

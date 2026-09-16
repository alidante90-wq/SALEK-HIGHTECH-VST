static juce::File salekUserPresetFile()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("SALEK_HIGHTECH");
    dir.createDirectory();
    return dir.getChildFile ("user_presets.xml");
}

void SalekHightechAudioProcessor::loadUserPresetsFromDisk()
{
    auto f = salekUserPresetFile();
    if (! f.existsAsFile()) return;

    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
    if (xml == nullptr)
        return;
    if (! xml->hasTagName ("SALEK_USER_PRESETS") && ! xml->hasTagName ("USER_PRESETS"))
        return;

    const int version = xml->getIntAttribute ("version", 1);
    juce::ignoreUnused (version);

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
                const auto id = pXml->getStringAttribute ("id");
                if (id.isNotEmpty())
                    pr.values[id] = (float) pXml->getDoubleAttribute ("value");
            }
            else if (pXml->hasTagName ("P"))
            {
                const auto id = pXml->getStringAttribute ("id");
                if (id.isNotEmpty())
                    pr.values[id] = (float) pXml->getDoubleAttribute ("v");
            }
        }

        int found = -1;
        for (int i = 0; i < (int) factoryPresets.size(); ++i)
            if (factoryPresets[(size_t) i].name == pr.name) { found = i; break; }

        if (found >= 0)
            factoryPresets[(size_t) found].values = std::move (pr.values);
        else
            factoryPresets.push_back (std::move (pr));
    }
}

void SalekHightechAudioProcessor::saveUserPresetsToDisk()
{
    juce::XmlElement root ("SALEK_USER_PRESETS");
    root.setAttribute ("version", 2);
    root.setAttribute ("plugin", "SALEK HIGHTECH");

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
    }

    auto target = salekUserPresetFile();
    auto tmp = target.getSiblingFile (target.getFileNameWithoutExtension() + ".tmp.xml");
    if (root.writeTo (tmp))
        tmp.moveFileTo (target);
}

int SalekHightechAudioProcessor::saveCurrentAsUserPreset (const juce::String& name)
{
    juce::String clean = name.trim();
    if (clean.isEmpty()) clean = "My Preset";
    clean = clean.replaceCharacter ('/', '-').replaceCharacter ('\\', '-');
    juce::String full = clean.startsWith ("USER/") ? clean : ("USER/" + clean);

    std::map<juce::String, float> vals;
    for (auto* param : getParameters())
    {
        if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (param))
                vals[withId->paramID] = rp->convertFrom0to1 (rp->getValue());
    }

    int found = -1;
    for (int i = 0; i < (int) factoryPresets.size(); ++i)
        if (factoryPresets[(size_t) i].name == full) { found = i; break; }

    if (found >= 0)
        factoryPresets[(size_t) found].values = std::move (vals);
    else
    {
        factoryPresets.push_back ({ full, std::move (vals) });
        found = (int) factoryPresets.size() - 1;
    }

    saveUserPresetsToDisk();
    currentProgram = found;
    return found;
}

bool SalekHightechAudioProcessor::exportCurrentPresetToFile (const juce::File& file, const juce::String& displayName)
{
    juce::String clean = displayName.trim();
    if (clean.isEmpty()) clean = file.getFileNameWithoutExtension();
    if (clean.isEmpty()) clean = "Shared Preset";
    clean = clean.replaceCharacter ('/', '-').replaceCharacter ('\\', '-');

    juce::XmlElement root ("SALEK_PRESET");
    root.setAttribute ("version", 2);
    root.setAttribute ("plugin", "SALEK HIGHTECH");
    root.setAttribute ("name", clean);

    for (auto* param : getParameters())
    {
        if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (param))
            {
                auto* p = root.createNewChildElement ("PARAM");
                p->setAttribute ("id", withId->paramID);
                p->setAttribute ("value", (double) rp->convertFrom0to1 (rp->getValue()));
            }
    }

    auto target = file;
    if (! target.hasFileExtension (".xml") && ! target.hasFileExtension (".salek.xml"))
        target = target.withFileExtension ("salek.xml");

    auto tmp = target.getSiblingFile (target.getFileNameWithoutExtension() + ".tmp.xml");
    if (! root.writeTo (tmp))
        return false;
    return tmp.moveFileTo (target);
}

int SalekHightechAudioProcessor::importPresetFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return -1;

    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
    if (xml == nullptr)
        return -1;

    juce::String pname;
    std::map<juce::String, float> vals;

    if (xml->hasTagName ("SALEK_PRESET"))
    {
        pname = xml->getStringAttribute ("name");
        for (auto* pXml : xml->getChildIterator())
        {
            if (pXml->hasTagName ("PARAM"))
            {
                auto id = pXml->getStringAttribute ("id");
                if (id.isNotEmpty())
                    vals[id] = (float) pXml->getDoubleAttribute ("value");
            }
            else if (pXml->hasTagName ("P"))
            {
                auto id = pXml->getStringAttribute ("id");
                if (id.isNotEmpty())
                    vals[id] = (float) pXml->getDoubleAttribute ("v");
            }
        }
    }
    else if (xml->hasTagName ("SALEK_USER_PRESETS") || xml->hasTagName ("USER_PRESETS"))
    {
        for (auto* presetXml : xml->getChildIterator())
        {
            if (! presetXml->hasTagName ("PRESET")) continue;
            pname = presetXml->getStringAttribute ("name");
            for (auto* pXml : presetXml->getChildIterator())
            {
                if (pXml->hasTagName ("PARAM"))
                {
                    auto id = pXml->getStringAttribute ("id");
                    if (id.isNotEmpty())
                        vals[id] = (float) pXml->getDoubleAttribute ("value");
                }
                else if (pXml->hasTagName ("P"))
                {
                    auto id = pXml->getStringAttribute ("id");
                    if (id.isNotEmpty())
                        vals[id] = (float) pXml->getDoubleAttribute ("v");
                }
            }
            break;
        }
    }
    else
        return -1;

    if (vals.empty())
        return -1;

    if (pname.isEmpty())
        pname = file.getFileNameWithoutExtension();
    pname = pname.trim().replaceCharacter ('/', '-');
    if (! pname.startsWith ("USER/"))
        pname = "USER/" + pname;

    for (auto& kv : vals)
        if (auto* p = apvts.getParameter (kv.first))
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
                rp->setValueNotifyingHost (rp->convertTo0to1 (kv.second));

    int found = -1;
    for (int i = 0; i < (int) factoryPresets.size(); ++i)
        if (factoryPresets[(size_t) i].name == pname) { found = i; break; }

    if (found >= 0)
        factoryPresets[(size_t) found].values = std::move (vals);
    else
    {
        factoryPresets.push_back ({ pname, std::move (vals) });
        found = (int) factoryPresets.size() - 1;
    }

    saveUserPresetsToDisk();
    currentProgram = found;
    return found;
}

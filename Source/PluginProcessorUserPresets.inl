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
    if (auto xml = juce::XmlDocument::parse (f))
    {
        for (auto* presetXml : xml->getChildIterator())
        {
            if (! presetXml->hasTagName ("PRESET")) continue;
            FactoryPreset pr;
            pr.name = presetXml->getStringAttribute ("name");
            if (! pr.name.startsWith ("USER/"))
                pr.name = "USER/" + pr.name;
            for (auto* pXml : presetXml->getChildIterator())
            {
                if (pXml->hasTagName ("P"))
                    pr.values[pXml->getStringAttribute ("id")] = (float) pXml->getDoubleAttribute ("v");
            }
            bool exists = false;
            for (auto& e : factoryPresets)
                if (e.name == pr.name) { exists = true; break; }
            if (! exists)
                factoryPresets.push_back (std::move (pr));
        }
    }
}

void SalekHightechAudioProcessor::saveUserPresetsToDisk()
{
    juce::XmlElement root ("USER_PRESETS");
    for (auto& pr : factoryPresets)
    {
        if (! pr.name.startsWith ("USER/")) continue;
        auto* px = root.createNewChildElement ("PRESET");
        px->setAttribute ("name", pr.name);
        for (auto& kv : pr.values)
        {
            auto* p = px->createNewChildElement ("P");
            p->setAttribute ("id", kv.first);
            p->setAttribute ("v", (double) kv.second);
        }
    }
    root.writeTo (salekUserPresetFile());
}

int SalekHightechAudioProcessor::saveCurrentAsUserPreset (const juce::String& name)
{
    juce::String clean = name.trim();
    if (clean.isEmpty()) clean = "My Preset";
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

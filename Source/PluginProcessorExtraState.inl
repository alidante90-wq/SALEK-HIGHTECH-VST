
// Extra non-APVTS state: sequencer steps, mod routes, LFO custom tables
void SalekHightechAudioProcessor::appendExtraState (juce::XmlElement& parent)
{
    // ---- Step sequencer pattern ----
    auto* seq = parent.createNewChildElement ("SEQ_PATTERN");
    seq->setAttribute ("length", stepSequencer.getPatternLength());
    for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
    {
        const auto& st = stepSequencer.getStep (i);
        auto* s = seq->createNewChildElement ("S");
        s->setAttribute ("i", i);
        s->setAttribute ("a", st.active ? 1 : 0);
        s->setAttribute ("n", st.noteOffset);
        s->setAttribute ("v", (double) st.velocity);
        s->setAttribute ("g", (double) st.gate);
        s->setAttribute ("p", (double) st.probability);
        s->setAttribute ("m", (double) st.modValue);
        s->setAttribute ("c", st.accent ? 1 : 0);
    }

    // ---- Mod matrix routes ----
    auto* mod = parent.createNewChildElement ("MOD_ROUTES");
    for (const auto& r : modMatrix.getRoutes())
    {
        if (! r.active) continue;
        auto* row = mod->createNewChildElement ("R");
        row->setAttribute ("src", (int) r.source);
        row->setAttribute ("dst", (int) r.dest);
        row->setAttribute ("amt", (double) r.amount);
    }

    // ---- LFO custom tables ----
    auto writeLfo = [&] (const char* tag, salek::LFO& lfo)
    {
        auto* el = parent.createNewChildElement (tag);
        const auto& tbl = lfo.getTable();
        juce::String csv;
        for (int i = 0; i < salek::LFO::TableSize; ++i)
        {
            if (i) csv << ",";
            csv << juce::String (tbl[(size_t) i], 5);
        }
        el->setAttribute ("pts", csv);
        el->setAttribute ("phase", (double) lfo.getPhase());
    };
    writeLfo ("LFO1_TABLE", lfo1);
    writeLfo ("LFO2_TABLE", lfo2);
    writeLfo ("LFO3_TABLE", lfo3);
}

void SalekHightechAudioProcessor::restoreExtraState (const juce::XmlElement& parent)
{
    if (auto* seq = parent.getChildByName ("SEQ_PATTERN"))
    {
        stepSequencer.setPatternLength (seq->getIntAttribute ("length", 16));
        for (auto* s : seq->getChildIterator())
        {
            if (! s->hasTagName ("S")) continue;
            const int i = s->getIntAttribute ("i", -1);
            if (i < 0 || i >= salek::StepSequencer::NumSteps) continue;
            auto& st = stepSequencer.getStep (i);
            st.active = s->getIntAttribute ("a", 0) != 0;
            st.noteOffset = s->getIntAttribute ("n", 0);
            st.velocity = (float) s->getDoubleAttribute ("v", 0.8);
            st.gate = (float) s->getDoubleAttribute ("g", 0.7);
            st.probability = (float) s->getDoubleAttribute ("p", 1.0);
            st.modValue = (float) s->getDoubleAttribute ("m", 0.0);
            st.accent = s->getIntAttribute ("c", 0) != 0;
        }
    }

    if (auto* mod = parent.getChildByName ("MOD_ROUTES"))
    {
        modMatrix.clear();
        for (auto* row : mod->getChildIterator())
        {
            if (! row->hasTagName ("R")) continue;
            const int src = row->getIntAttribute ("src", 0);
            const int dst = row->getIntAttribute ("dst", 0);
            const float amt = (float) row->getDoubleAttribute ("amt", 0.0);
            if (src >= 0 && src < (int) salek::ModMatrix::Source::NumSources
                && dst >= 0 && dst < (int) salek::ModMatrix::Dest::NumDests)
                modMatrix.addRoute ((salek::ModMatrix::Source) src,
                                    (salek::ModMatrix::Dest) dst, amt);
        }
    }

    auto readLfo = [&] (const char* tag, salek::LFO& lfo)
    {
        if (auto* el = parent.getChildByName (tag))
        {
            auto csv = el->getStringAttribute ("pts");
            if (csv.isNotEmpty())
            {
                juce::StringArray parts;
                parts.addTokens (csv, ",", "");
                for (int i = 0; i < salek::LFO::TableSize && i < parts.size(); ++i)
                    lfo.setCustomPoint (i, parts[i].getFloatValue());
                lfo.setWave (salek::LFO::Wave::Custom);
            }
            if (el->hasAttribute ("phase"))
                lfo.setPhase01 (el->getDoubleAttribute ("phase"));
        }
    };
    readLfo ("LFO1_TABLE", lfo1);
    readLfo ("LFO2_TABLE", lfo2);
    readLfo ("LFO3_TABLE", lfo3);
}

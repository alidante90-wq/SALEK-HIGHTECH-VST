#pragma once
#include <JuceHeader.h>
#include <vector>

struct ModRoute
{
    int sourceId = -1;
    int destId   = -1;
    float amount = 0.0f;
    bool bipolar = true;
    bool enabled = true;
    float curve  = 0.0f;
};

class ModulationMatrix
{
public:
    static constexpr int maxRoutes = 96;
    static constexpr int numSources = 24;
    static constexpr int numDests   = 48;

    void prepare (double) {}
    void clear() { routes.clear(); }

    bool addRoute (int source, int dest, float amount = 0.5f, bool bipolar = true)
    {
        if ((int) routes.size() >= maxRoutes) return false;
        for (auto& r : routes)
            if (r.sourceId == source && r.destId == dest)
            {
                r.amount = amount;
                r.bipolar = bipolar;
                return true;
            }
        ModRoute r;
        r.sourceId = source;
        r.destId = dest;
        r.amount = amount;
        r.bipolar = bipolar;
        routes.push_back (r);
        return true;
    }

    void removeRoute (int index)
    {
        if (juce::isPositiveAndBelow (index, (int) routes.size()))
            routes.erase (routes.begin() + index);
    }

    void process (const float* sources, float* destOffsets)
    {
        for (const auto& r : routes)
        {
            if (!r.enabled || r.sourceId < 0 || r.destId < 0) continue;
            if (r.sourceId >= numSources || r.destId >= numDests) continue;

            float src = sources[r.sourceId];
            if (!r.bipolar) src = src * 0.5f + 0.5f;

            if (r.curve > 0.01f)
                src = std::pow (std::abs (src), 1.0f + r.curve) * (src >= 0.0f ? 1.0f : -1.0f);
            else if (r.curve < -0.01f)
                src = std::pow (std::abs (src), 1.0f / (1.0f - r.curve)) * (src >= 0.0f ? 1.0f : -1.0f);

            destOffsets[r.destId] += src * r.amount;
        }
    }

    std::vector<ModRoute>& getRoutes() { return routes; }
    const std::vector<ModRoute>& getRoutes() const { return routes; }

    enum Source {
        LFO1 = 0, LFO2, LFO3, LFO4,
        Env1, Env2, Env3,
        Macro1, Macro2, Macro3, Macro4, Macro5, Macro6, Macro7, Macro8,
        Velocity, NoteNumber, Aftertouch, ModWheel,
        Random, Chaos, Seq1, Seq2, Arp
    };

    enum Dest {
        Osc1Level = 0, Osc1WTPos, Osc1Morph, Osc1Warp, Osc1FM, Osc1AM, Osc1RM,
        Osc2Level, Osc2WTPos, Osc2Morph, Osc2Warp, Osc2FM, Osc2AM, Osc2RM,
        Osc3Level, Osc3WTPos, Osc3Morph, Osc3Warp, Osc3FM, Osc3AM, Osc3RM,
        SubLevel, NoiseLevel,
        FilterCutoff, FilterRes, FilterDrive,
        AmpLevel, Pan,
        LFO1Rate, LFO2Rate, LFO3Rate, LFO4Rate,
        FXDist, FXDelay, FXReverb,
        MasterGain
    };

private:
    std::vector<ModRoute> routes;
};
#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

/** Realtime-safe modulation matrix.
 *  Optimized: setSourceValue rebuilds per-dest sums once per block;
 *  getModulation(dest) is O(1).
 */
class ModMatrix
{
public:
    static constexpr int MaxRoutes = 64;

    enum class Source : int
    {
        LFO1 = 0,
        LFO2,
        LFO3,
        Env1,
        Velocity,
        ModWheel,
        Macro1,
        Macro2,
        Macro3,
        Macro4,
        Random,
        MSEG,
        NumSources
    };

    enum class Dest : int
    {
        FilterCutoff = 0,
        FilterReso,
        Osc1Level,
        Osc2Level,
        Osc3Level,
        Osc1Table,
        Osc2Table,
        Osc3Table,
        Osc1Warp,
        Osc2Warp,
        Osc3Warp,
        Osc1Fold,
        Osc2Fold,
        Osc3Fold,
        Fm2to1,
        Fm3to1,
        Pitch,
        Amp,
        Osc1Pan,
        Osc2Pan,
        Osc3Pan,
        Osc1Drive,
        Osc2Drive,
        Osc3Drive,
        DelayMix,
        ReverbMix,
        DistDrive,
        ChorusMix,
        FilterEnv,
        PhaserMix,
        Bassify,
        MagicX,
        MagicY,
        NumDests
    };

    static const char* sourceName (Source s)
    {
        static const char* n[] = {
            "LFO1","LFO2","LFO3","ENV","VEL","MW",
            "MAC1","MAC2","MAC3","MAC4","RND","MSEG"
        };
        const int i = (int) s;
        return (i >= 0 && i < (int) Source::NumSources) ? n[i] : "?";
    }

    static const char* destName (Dest d)
    {
        static const char* n[] = {
            "CUT","RESO","O1LVL","O2LVL","O3LVL",
            "O1TBL","O2TBL","O3TBL","O1WRP","O2WRP","O3WRP",
            "O1FLD","O2FLD","O3FLD","FM21","FM31","PITCH","AMP",
            "O1PAN","O2PAN","O3PAN","O1DRV","O2DRV","O3DRV",
            "DLYMX","REVMX","DIST","CHOR","FENV","PHSR","BASS","MAGX","MAGY"
        };
        const int i = (int) d;
        return (i >= 0 && i < (int) Dest::NumDests) ? n[i] : "?";
    }

    struct Route
    {
        Source source = Source::LFO1;
        Dest   dest   = Dest::FilterCutoff;
        float  amount = 0.0f;
        bool   active = false;
    };

    void clear() noexcept
    {
        for (auto& r : routes) r.active = false;
        dirty = true;
    }

    int addRoute (Source src, Dest dst, float amount) noexcept
    {
        amount = juce::jlimit (-1.0f, 1.0f, amount);
        for (int i = 0; i < MaxRoutes; ++i)
        {
            auto& r = routes[static_cast<size_t>(i)];
            if (r.active && r.source == src && r.dest == dst)
            {
                r.amount = amount;
                dirty = true;
                return i;
            }
        }
        for (int i = 0; i < MaxRoutes; ++i)
        {
            auto& r = routes[static_cast<size_t>(i)];
            if (! r.active)
            {
                r = { src, dst, amount, true };
                dirty = true;
                return i;
            }
        }
        return -1;
    }

    void removeRoute (int slot) noexcept
    {
        if (slot >= 0 && slot < MaxRoutes)
        {
            routes[static_cast<size_t>(slot)].active = false;
            dirty = true;
        }
    }

    void removeRoute (Source src, Dest dst) noexcept
    {
        for (int i = 0; i < MaxRoutes; ++i)
        {
            auto& r = routes[static_cast<size_t>(i)];
            if (r.active && r.source == src && r.dest == dst)
            {
                r.active = false;
                dirty = true;
            }
        }
    }

    void setSourceValue (Source s, float v) noexcept
    {
        const size_t i = static_cast<size_t>(s);
        if (i >= static_cast<size_t>(Source::NumSources)) return;
        // Avoid denormals / NaN in matrix
        if (! std::isfinite (v)) v = 0.f;
        sourceValues[i] = v;
        dirty = true;
    }

    float getSourceValue (Source s) const noexcept
    {
        return sourceValues[static_cast<size_t>(s)];
    }

    /** Call once after all setSourceValue for the block (or rely on lazy getModulation). */
    void finalizeBlock() noexcept
    {
        if (! dirty) return;
        destSums.fill (0.f);
        for (const auto& r : routes)
        {
            if (! r.active) continue;
            const size_t si = static_cast<size_t>(r.source);
            const size_t di = static_cast<size_t>(r.dest);
            if (si >= static_cast<size_t>(Source::NumSources)) continue;
            if (di >= static_cast<size_t>(Dest::NumDests)) continue;
            destSums[di] += sourceValues[si] * r.amount;
        }
        // soft clamp per dest to avoid extreme stacks
        for (auto& s : destSums)
            s = juce::jlimit (-2.f, 2.f, s);
        dirty = false;
    }

    float getModulation (Dest d) const noexcept
    {
        // Lazy rebuild if someone forgot finalizeBlock
        if (dirty)
            const_cast<ModMatrix*>(this)->finalizeBlock();
        const size_t i = static_cast<size_t>(d);
        return i < destSums.size() ? destSums[i] : 0.f;
    }

    const std::array<Route, MaxRoutes>& getRoutes() const noexcept { return routes; }

    float getRouteAmount (Source src, Dest dst) const noexcept
    {
        for (const auto& r : routes)
            if (r.active && r.source == src && r.dest == dst)
                return r.amount;
        return 0.f;
    }

    bool hasRoute (Source src, Dest dst) const noexcept
    {
        return std::abs (getRouteAmount (src, dst)) > 1.0e-5f;
    }

    void setRouteAmount (Source src, Dest dst, float amount) noexcept
    {
        amount = juce::jlimit (-1.f, 1.f, amount);
        if (std::abs (amount) < 1.0e-5f)
        {
            removeRoute (src, dst);
            return;
        }
        addRoute (src, dst, amount);
    }

    /** Persist only active routes. Kept separate from APVTS so old presets remain valid. */
    void writeState (juce::XmlElement& parent) const
    {
        auto* node = parent.createNewChildElement ("MOD_MATRIX");
        for (const auto& r : routes)
        {
            if (! r.active) continue;
            auto* route = node->createNewChildElement ("ROUTE");
            route->setAttribute ("src", (int) r.source);
            route->setAttribute ("dst", (int) r.dest);
            route->setAttribute ("amount", r.amount);
        }
    }

    void readState (const juce::XmlElement& parent) noexcept
    {
        clear();
        if (auto* node = parent.getChildByName ("MOD_MATRIX"))
        {
            forEachXmlChildElementWithTagName (*node, route, "ROUTE")
            {
                const int src = route->getIntAttribute ("src", -1);
                const int dst = route->getIntAttribute ("dst", -1);
                const float amount = (float) route->getDoubleAttribute ("amount", 0.0);
                if (src >= 0 && src < (int) Source::NumSources
                    && dst >= 0 && dst < (int) Dest::NumDests)
                    addRoute ((Source) src, (Dest) dst, amount);
            }
        }
        finalizeBlock();
    }
    Route& getRoute (int i) noexcept { return routes[static_cast<size_t>(juce::jlimit(0, MaxRoutes-1, i))]; }

    int countActiveRoutes() const noexcept
    {
        int n = 0;
        for (const auto& r : routes) if (r.active) ++n;
        return n;
    }

private:
    std::array<Route, MaxRoutes> routes {};
    std::array<float, static_cast<size_t>(Source::NumSources)> sourceValues {};
    std::array<float, static_cast<size_t>(Dest::NumDests)> destSums {};
    bool dirty = true;
};

} // namespace salek

#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace salek
{

/** Lightweight real-time modulation matrix.
    Sources and destinations are fixed enums for speed and safety.
    Up to MaxRoutes simultaneous routes.
*/
class ModMatrix
{
public:
    static constexpr int MaxRoutes = 16;

    enum class Source : int
    {
        LFO1 = 0,
        Env1,
        Velocity,
        ModWheel,
        Macro1,
        Macro2,
        Macro3,
        Macro4,
        Random,
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
        Osc1Fold,
        Fm2to1,
        Fm3to1,
        Pitch,
        NumDests
    };

    struct Route
    {
        Source source = Source::LFO1;
        Dest   dest   = Dest::FilterCutoff;
        float  amount = 0.0f;   // -1 .. +1
        bool   active = false;
    };

    void clear() noexcept
    {
        for (auto& r : routes) r.active = false;
    }

    /** Add or update a route. Returns slot index or -1 if full. */
    int addRoute (Source src, Dest dst, float amount) noexcept
    {
        // Try to find existing same src→dst
        for (int i = 0; i < MaxRoutes; ++i)
        {
            if (routes[static_cast<size_t>(i)].active
                && routes[static_cast<size_t>(i)].source == src
                && routes[static_cast<size_t>(i)].dest == dst)
            {
                routes[static_cast<size_t>(i)].amount = juce::jlimit (-1.0f, 1.0f, amount);
                return i;
            }
        }
        // Find free slot
        for (int i = 0; i < MaxRoutes; ++i)
        {
            if (! routes[static_cast<size_t>(i)].active)
            {
                routes[static_cast<size_t>(i)] = { src, dst, juce::jlimit (-1.0f, 1.0f, amount), true };
                return i;
            }
        }
        return -1;
    }

    void removeRoute (int slot) noexcept
    {
        if (slot >= 0 && slot < MaxRoutes)
            routes[static_cast<size_t>(slot)].active = false;
    }

    void setSourceValue (Source s, float v) noexcept
    {
        sourceValues[static_cast<size_t>(s)] = v;
    }

    /** Accumulate modulation for a destination. Returns summed modulation (-something .. +something). */
    float getModulation (Dest d) const noexcept
    {
        float sum = 0.0f;
        for (const auto& r : routes)
        {
            if (r.active && r.dest == d)
                sum += sourceValues[static_cast<size_t>(r.source)] * r.amount;
        }
        return sum;
    }

    const std::array<Route, MaxRoutes>& getRoutes() const noexcept { return routes; }

private:
    std::array<Route, MaxRoutes> routes {};
    std::array<float, static_cast<size_t>(Source::NumSources)> sourceValues {};
};

} // namespace salek

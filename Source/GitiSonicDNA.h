#pragma once
#include <JuceHeader.h>
#include "GitiEdition.h"
#include <map>
#include <cmath>

namespace giti::dna
{
struct Profile
{
    float spectralFocus = 0.5f;
    float harmonicity = 0.5f;
    float modulation = 0.5f;
    float transient = 0.5f;
    float spatial = 0.5f;
    float aggression = 0.5f;
    float lowWeight = 0.5f;
    float highWeight = 0.5f;
    float instability = 0.5f;
};

inline float hash01 (int id, int salt)
{
    const auto x = std::sin (float (id * 97 + salt * 53)) * 43758.5453f;
    return x - std::floor (x);
}

inline Profile profileFor (const Edition& e)
{
    Profile p;
    p.spectralFocus = 0.22f + 0.70f * hash01(e.id, 11);
    p.harmonicity = 0.18f + 0.72f * hash01(e.id, 29);
    p.modulation = 0.18f + 0.78f * hash01(e.id, 47);
    p.transient = 0.20f + 0.72f * hash01(e.id, 71);
    p.spatial = 0.18f + 0.78f * hash01(e.id, 89);
    p.aggression = 0.15f + 0.82f * hash01(e.id, 107);
    p.lowWeight = 0.15f + 0.82f * hash01(e.id, 127);
    p.highWeight = 0.15f + 0.82f * hash01(e.id, 149);
    p.instability = 0.10f + 0.85f * hash01(e.id, 173);

    const auto archetype = juce::String(e.archetype);
    if (archetype == "BASS")    p.lowWeight = 0.78f;
    if (archetype == "FM")      p.modulation = 0.88f;
    if (archetype == "LEAD")    p.highWeight = 0.82f;
    if (archetype == "SCREECH") p.instability = 0.90f;
    if (archetype == "ACID")    p.aggression = 0.82f;
    if (archetype == "ATMOS")   p.spatial = 0.90f;
    if (archetype == "DRONE")   p.harmonicity = 0.30f;
    if (archetype == "RITUAL")  p.modulation = 0.70f;
    if (archetype == "ALIEN")   p.instability = 0.82f;
    if (archetype == "MACHINE") p.transient = 0.88f;
    return p;
}

inline void scale(std::map<juce::String,float>& v, const juce::String& id, float factor, float lo, float hi)
{
    if (auto it = v.find(id); it != v.end())
        it->second = juce::jlimit(lo, hi, it->second * factor);
}

inline void shift(std::map<juce::String,float>& v, const juce::String& id, float amount, float lo, float hi)
{
    if (auto it = v.find(id); it != v.end())
        it->second = juce::jlimit(lo, hi, it->second + amount);
}

// Bounded deterministic morphing. It never touches INIT, discrete mode IDs,
// or values outside the known parameter domains.
inline void applyToPreset(const Edition& e, const juce::String& presetName,
                           std::map<juce::String,float>& v)
{
    if (presetName.startsWithIgnoreCase("Init/"))
        return;

    const auto p = profileFor(e);
    const float idPhase = hash01(e.id, 211);
    const float spectral = 0.72f + 0.56f * p.spectralFocus;
    const float harmonic = 0.76f + 0.48f * p.harmonicity;
    const float mod = 0.70f + 0.60f * p.modulation;
    const float transient = 0.72f + 0.56f * p.transient;
    const float space = 0.70f + 0.62f * p.spatial;

    for (int o = 1; o <= 3; ++o)
    {
        const auto prefix = "osc" + juce::String(o);
        scale(v, prefix + "_table", harmonic, 0.0f, 1.0f);
        shift(v, prefix + "_warp", (idPhase - 0.5f) * 0.08f, 0.0f, 1.0f);
        shift(v, prefix + "_fold", (p.instability - 0.5f) * 0.12f, 0.0f, 1.0f);
        shift(v, prefix + "_drive", (p.aggression - 0.5f) * 0.10f, 0.0f, 1.0f);
    }

    scale(v, "filter_cutoff", spectral, 80.0f, 18000.0f);
    shift(v, "filter_reso", (p.instability - 0.5f) * 0.18f, 0.0f, 1.0f);
    shift(v, "filter_drive", (p.aggression - 0.5f) * 0.14f, 0.0f, 1.0f);
    shift(v, "filter_env", (p.modulation - 0.5f) * 0.16f, 0.0f, 1.0f);

    for (const char* id : { "fm_2to1","fm_3to1","fm_3to2","pm_2to1","rm_2to1","am_2to1" })
        scale(v, id, mod, 0.0f, 1.0f);

    scale(v, "amp_attack", 1.0f / transient, 0.0001f, 4.0f);
    scale(v, "amp_decay", 1.0f / transient, 0.005f, 5.0f);
    scale(v, "amp_release", 0.78f + 0.55f * p.spatial, 0.005f, 6.0f);

    for (const char* id : { "lfo_amount","lfo2_amount","lfo3_amount" })
        scale(v, id, mod, 0.0f, 1.0f);
    for (const char* id : { "lfo_rate","lfo2_rate","lfo3_rate" })
        scale(v, id, 0.65f + 1.8f * p.modulation, 0.01f, 30.0f);

    scale(v, "sub_level", 0.72f + 0.62f * p.lowWeight, 0.0f, 1.0f);
    scale(v, "noise_level", 0.60f + 0.85f * p.instability, 0.0f, 1.0f);
    scale(v, "delay_mix", space, 0.0f, 1.0f);
    scale(v, "reverb_mix", space, 0.0f, 1.0f);
    scale(v, "chorus_mix", 0.70f + 0.55f * p.spatial, 0.0f, 1.0f);
    scale(v, "phaser_mix", 0.70f + 0.60f * p.modulation, 0.0f, 1.0f);
    scale(v, "dist_mix", 0.72f + 0.62f * p.aggression, 0.0f, 1.0f);
    scale(v, "master_drive", 0.78f + 0.45f * p.aggression, 0.0f, 1.0f);
    scale(v, "comp_mix", 0.78f + 0.40f * p.transient, 0.0f, 1.0f);

    const auto archetype = juce::String(e.archetype);
    if (archetype == "BASS") {
        scale(v, "filter_cutoff", 0.78f + 0.24f * p.lowWeight, 80.0f, 12000.0f);
        scale(v, "sub_level", 1.15f, 0.0f, 1.0f);
    } else if (archetype == "FM") {
        scale(v, "fm_2to1", 1.12f, 0.0f, 1.0f);
        scale(v, "fm_3to1", 1.08f, 0.0f, 1.0f);
    } else if (archetype == "LEAD") {
        scale(v, "filter_cutoff", 1.15f, 80.0f, 18000.0f);
        scale(v, "delay_mix", 1.08f, 0.0f, 1.0f);
    } else if (archetype == "SCREECH") {
        scale(v, "filter_reso", 1.18f, 0.0f, 1.0f);
        scale(v, "osc1_fold", 1.16f, 0.0f, 1.0f);
        scale(v, "dist_mix", 1.12f, 0.0f, 1.0f);
    } else if (archetype == "ACID") {
        scale(v, "filter_reso", 1.12f, 0.0f, 1.0f);
        scale(v, "filter_drive", 1.15f, 0.0f, 1.0f);
    } else if (archetype == "ATMOS") {
        scale(v, "reverb_mix", 1.18f, 0.0f, 1.0f);
        scale(v, "reverb_size", 1.08f, 0.0f, 1.0f);
    } else if (archetype == "DRONE") {
        scale(v, "amp_release", 1.35f, 0.005f, 8.0f);
        scale(v, "reverb_mix", 1.12f, 0.0f, 1.0f);
    } else if (archetype == "RITUAL") {
        scale(v, "lfo_amount", 1.14f, 0.0f, 1.0f);
        scale(v, "reverb_mix", 1.08f, 0.0f, 1.0f);
    } else if (archetype == "ALIEN") {
        scale(v, "fm_2to1", 1.15f, 0.0f, 1.0f);
        scale(v, "phaser_mix", 1.15f, 0.0f, 1.0f);
        scale(v, "noise_level", 1.15f, 0.0f, 1.0f);
    } else if (archetype == "MACHINE") {
        scale(v, "comp_mix", 1.12f, 0.0f, 1.0f);
        scale(v, "master_drive", 1.10f, 0.0f, 1.0f);
    }
}
}

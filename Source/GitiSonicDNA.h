#pragma once
/**
 * GitiSonicDNA.h — Identity → SHAE parameter seed mapping
 * SALEK HIGHTECH / GITI
 *
 * Rules:
 * - Does NOT invent or rename identities (source of truth: GitiIdentities.h)
 * - Provides deterministic DSP starting points per GITI based on branch/voice/mood
 * - Used by factory presets and identity selection to bias oscillators, filter,
 *   noise, envelopes and character without erasing the identity layer
 */

#include "GitiIdentities.h"
#include <cmath>
#include <cstring>
#include <map>

namespace giti {

/** Branch-derived synthesis bias (design rules, not samples) */
enum class BranchBias : int
{
    Breath = 0,   // noise/air, soft attack, formant/band, pitch drift
    Strings,      // pluck/excite, harmonic richness, decay, subtle pitch
    Voice,        // formant, harmonic clusters, vowel-like movement
    Pulse,        // transient, sub, sync, rhythmic envelopes
    EarthWater    // organic noise, comb/resonator, slow mod, drone
};

inline BranchBias branchFromString (const char* b) noexcept
{
    if (b == nullptr) return BranchBias::Breath;
    if (std::strstr (b, "Breath"))        return BranchBias::Breath;
    if (std::strstr (b, "Strings"))       return BranchBias::Strings;
    if (std::strstr (b, "Voice"))         return BranchBias::Voice;
    if (std::strstr (b, "Pulse"))         return BranchBias::Pulse;
    if (std::strstr (b, "Earth"))         return BranchBias::EarthWater;
    return BranchBias::Breath;
}

/** Compact seed that can be applied to SHAE / factory preset maps */
struct SonicSeed
{
    float osc1Level   = 0.85f;
    float osc2Level   = 0.25f;
    float osc3Level   = 0.0f;
    float subLevel    = 0.15f;
    float noiseLevel  = 0.05f;

    float osc1Table   = 0.25f;   // 0..1 wavetable / wave position
    float osc1Warp    = 0.15f;
    float osc1Fold    = 0.08f;
    float osc1Drive   = 0.12f;
    float osc1Unison  = 1.f;
    float osc1Detune  = 8.f;
    float osc1Spread  = 0.4f;

    float filterCutoff = 4500.f;
    float filterReso   = 0.25f;
    float filterEnv    = 0.45f;
    float filterDrive  = 0.15f;
    int   filterMode   = 0;      // 0 LP, 1 HP, 2 BP, 3 Notch, ...

    float ampAttack   = 0.02f;
    float ampDecay    = 0.25f;
    float ampSustain  = 0.6f;
    float ampRelease  = 0.35f;

    float reverbMix   = 0.25f;
    float delayMix    = 0.1f;
    float chorusMix   = 0.12f;
    float masterDrive = 0.2f;
    float masterGain  = 0.75f;

    float formantAmt  = 0.0f;
    float formantMorph= 0.3f;
    float resonatorMix= 0.0f;
};

/** Build a deterministic seed from a GITI identity index (0..49) */
inline SonicSeed seedForIdentity (int index0based) noexcept
{
    SonicSeed s;
    if (index0based < 0 || index0based >= 50)
        return s;

    const auto& id = kIdentities[index0based];
    const BranchBias bias = branchFromString (id.branch);

    // Stable pseudo-variation from id number (no random at runtime)
    const float t = (float) (index0based + 1) / 50.f;
    const float wobble = 0.04f * std::sin (t * 12.566f);

    switch (bias)
    {
        case BranchBias::Breath:
            s.noiseLevel   = 0.12f + 0.08f * t;
            s.osc1Level    = 0.72f;
            s.osc2Level    = 0.28f;
            s.subLevel     = 0.08f;
            s.ampAttack    = 0.04f + 0.06f * t;
            s.ampRelease   = 0.35f + 0.25f * t;
            s.filterCutoff = 3800.f + 2200.f * t;
            s.filterReso   = 0.18f;
            s.filterMode   = 2; // BP-ish for air
            s.formantAmt   = 0.25f + 0.2f * t;
            s.osc1Warp     = 0.12f + wobble;
            s.reverbMix    = 0.28f + 0.15f * t;
            break;

        case BranchBias::Strings:
            s.noiseLevel   = 0.03f;
            s.osc1Level    = 0.82f;
            s.osc2Level    = 0.38f;
            s.osc1Unison   = 2.f + (float)(index0based % 3);
            s.osc1Detune   = 10.f + 12.f * t;
            s.osc1Spread   = 0.55f + 0.3f * t;
            s.ampAttack    = 0.008f + 0.02f * t;
            s.ampDecay     = 0.35f + 0.3f * t;
            s.ampSustain   = 0.35f;
            s.ampRelease   = 0.4f + 0.3f * t;
            s.filterCutoff = 3200.f + 2800.f * t;
            s.filterReso   = 0.28f + 0.1f * t;
            s.osc1Fold     = 0.1f + 0.15f * t;
            s.osc1Warp     = 0.2f + 0.15f * t;
            s.reverbMix    = 0.3f + 0.15f * t;
            s.chorusMix    = 0.18f;
            break;

        case BranchBias::Voice:
            s.noiseLevel   = 0.06f;
            s.osc1Level    = 0.8f;
            s.osc2Level    = 0.35f;
            s.formantAmt   = 0.45f + 0.25f * t;
            s.formantMorph = 0.2f + 0.6f * t;
            s.ampAttack    = 0.05f + 0.08f * t;
            s.ampSustain   = 0.7f;
            s.ampRelease   = 0.45f + 0.25f * t;
            s.filterCutoff = 2800.f + 2000.f * t;
            s.filterReso   = 0.32f;
            s.filterMode   = 0;
            s.osc1Warp     = 0.25f + 0.2f * t;
            s.reverbMix    = 0.35f + 0.15f * t;
            break;

        case BranchBias::Pulse:
            s.noiseLevel   = 0.04f;
            s.osc1Level    = 0.9f;
            s.subLevel     = 0.28f + 0.15f * t;
            s.osc2Level    = 0.15f;
            s.ampAttack    = 0.002f + 0.008f * t;
            s.ampDecay     = 0.15f + 0.2f * t;
            s.ampSustain   = 0.08f + 0.15f * t;
            s.ampRelease   = 0.12f + 0.15f * t;
            s.filterCutoff = 800.f + 1800.f * t;
            s.filterReso   = 0.4f + 0.2f * t;
            s.filterEnv    = 0.7f + 0.2f * t;
            s.osc1Drive    = 0.35f + 0.25f * t;
            s.osc1Fold     = 0.25f + 0.25f * t;
            s.masterDrive  = 0.35f + 0.2f * t;
            s.reverbMix    = 0.12f;
            break;

        case BranchBias::EarthWater:
            s.noiseLevel   = 0.1f + 0.08f * t;
            s.osc1Level    = 0.75f;
            s.osc2Level    = 0.4f;
            s.subLevel     = 0.2f;
            s.ampAttack    = 0.03f + 0.1f * t;
            s.ampDecay     = 0.5f + 0.4f * t;
            s.ampSustain   = 0.45f + 0.2f * t;
            s.ampRelease   = 0.7f + 0.5f * t;
            s.filterCutoff = 2500.f + 2500.f * t;
            s.filterReso   = 0.35f + 0.15f * t;
            s.resonatorMix = 0.15f + 0.25f * t;
            s.osc1Warp     = 0.15f;
            s.reverbMix    = 0.4f + 0.2f * t;
            s.chorusMix    = 0.15f;
            break;
    }

    // Special case: GITI 050 — collective / succession (heavier unison + wider)
    if (index0based == 49)
    {
        s.osc1Unison  = 4.f;
        s.osc1Detune  = 18.f;
        s.osc1Spread  = 0.9f;
        s.osc2Level   = 0.55f;
        s.reverbMix   = 0.4f;
        s.ampSustain  = 0.6f;
        s.filterCutoff= 4200.f;
    }

    // Tiny deterministic variation so neighbouring IDs are not identical
    s.osc1Table   = juce::jlimit (0.f, 1.f, s.osc1Table   + wobble * 0.5f);
    s.filterCutoff= juce::jlimit (80.f, 16000.f, s.filterCutoff * (1.f + wobble * 0.3f));
    s.masterGain  = juce::jlimit (0.5f, 0.9f, s.masterGain);

    return s;
}

/** Convert a SonicSeed into the string→float map used by factory preset loader */
inline std::map<juce::String, float> seedToPresetMap (const SonicSeed& s)
{
    return {
        { "osc1_level",   s.osc1Level },
        { "osc2_level",   s.osc2Level },
        { "osc3_level",   s.osc3Level },
        { "sub_level",    s.subLevel },
        { "noise_level",  s.noiseLevel },
        { "osc1_table",   s.osc1Table },
        { "osc1_warp",    s.osc1Warp },
        { "osc1_fold",    s.osc1Fold },
        { "osc1_drive",   s.osc1Drive },
        { "osc1_unison",  s.osc1Unison },
        { "osc1_udet",    s.osc1Detune },
        { "osc1_uspread", s.osc1Spread },
        { "filter_cutoff",s.filterCutoff },
        { "filter_reso",  s.filterReso },
        { "filter_env",   s.filterEnv },
        { "filter_drive", s.filterDrive },
        { "filter_mode",  (float) s.filterMode },
        { "amp_attack",   s.ampAttack },
        { "amp_decay",    s.ampDecay },
        { "amp_sustain",  s.ampSustain },
        { "amp_release",  s.ampRelease },
        { "reverb_mix",   s.reverbMix },
        { "delay_mix",    s.delayMix },
        { "chorus_mix",   s.chorusMix },
        { "master_drive", s.masterDrive },
        { "master_gain",  s.masterGain },
        { "formant_amt",  s.formantAmt },
        { "formant_morph",s.formantMorph },
        { "resonator_mix",s.resonatorMix }
    };
}

/** Convenience: full preset map for GITI id 1..50 */
inline std::map<juce::String, float> presetMapForGiti (int gitiId1to50)
{
    return seedToPresetMap (seedForIdentity (gitiId1to50 - 1));
}

struct GitiSonicDNA
{
    static constexpr int kCount = 50;

    static const Identity& identity (int id1to50) noexcept
    {
        const int i = juce::jlimit (1, 50, id1to50) - 1;
        return kIdentities[i];
    }

    static SonicSeed seed (int id1to50) noexcept
    {
        return seedForIdentity (juce::jlimit (1, 50, id1to50) - 1);
    }

    static BranchBias branch (int id1to50) noexcept
    {
        return branchFromString (identity (id1to50).branch);
    }
};

} // namespace giti

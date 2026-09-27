#pragma once
#include <JuceHeader.h>

namespace giti
{
struct Edition
{
    int id;
    const char* code;
    const char* name;
    const char* element;
    const char* archetype;
    int bpm;
    const char* palette;
};

inline constexpr Edition genesisEditions[] =
{
    { 1, "GITI-001", "ORIGIN", "PLASMA", "BASS", 174, "CYAN/MAGENTA" },
    { 2, "GITI-002", "NOVA", "VOID", "FM", 175, "VIOLET/CYAN" },
    { 3, "GITI-003", "CYAN", "CYBER", "LEAD", 176, "RED/CYAN" },
    { 4, "GITI-004", "VOID", "QUANTUM", "SCREECH", 177, "GREEN/VIOLET" },
    { 5, "GITI-005", "PULSE", "SONIC", "ACID", 178, "GOLD/PURPLE" },
    { 6, "GITI-006", "ACID", "ACID", "ATMOS", 179, "CYAN/MAGENTA" },
    { 7, "GITI-007", "PHOTON", "PHOTON", "DRONE", 180, "VIOLET/CYAN" },
    { 8, "GITI-008", "ECLIPSE", "ECLIPSE", "RITUAL", 181, "RED/CYAN" },
    { 9, "GITI-009", "NEON", "NEON", "ALIEN", 182, "GREEN/VIOLET" },
    { 10, "GITI-010", "QUANTUM", "GRAVITY", "MACHINE", 183, "GOLD/PURPLE" },
    { 11, "GITI-011", "RIFT", "PLASMA", "BASS", 184, "CYAN/MAGENTA" },
    { 12, "GITI-012", "SPECTRA", "VOID", "FM", 185, "VIOLET/CYAN" },
    { 13, "GITI-013", "AURORA", "CYBER", "LEAD", 186, "RED/CYAN" },
    { 14, "GITI-014", "MIRAGE", "QUANTUM", "SCREECH", 187, "GREEN/VIOLET" },
    { 15, "GITI-015", "TEMPEST", "SONIC", "ACID", 188, "GOLD/PURPLE" },
    { 16, "GITI-016", "SOLAR", "ACID", "ATMOS", 189, "CYAN/MAGENTA" },
    { 17, "GITI-017", "LUNAR", "PHOTON", "DRONE", 190, "VIOLET/CYAN" },
    { 18, "GITI-018", "COMET", "ECLIPSE", "RITUAL", 174, "RED/CYAN" },
    { 19, "GITI-019", "PLASMA", "NEON", "ALIEN", 175, "GREEN/VIOLET" },
    { 20, "GITI-020", "OBSIDIAN", "GRAVITY", "MACHINE", 176, "GOLD/PURPLE" },
    { 21, "GITI-021", "PRISM", "PLASMA", "BASS", 177, "CYAN/MAGENTA" },
    { 22, "GITI-022", "VECTOR", "VOID", "FM", 178, "VIOLET/CYAN" },
    { 23, "GITI-023", "ECHO", "CYBER", "LEAD", 179, "RED/CYAN" },
    { 24, "GITI-024", "FLUX", "QUANTUM", "SCREECH", 180, "GREEN/VIOLET" },
    { 25, "GITI-025", "ION", "SONIC", "ACID", 181, "GOLD/PURPLE" },
    { 26, "GITI-026", "CHROME", "ACID", "ATMOS", 182, "CYAN/MAGENTA" },
    { 27, "GITI-027", "MYSTIC", "PHOTON", "DRONE", 183, "VIOLET/CYAN" },
    { 28, "GITI-028", "PORTAL", "ECLIPSE", "RITUAL", 184, "RED/CYAN" },
    { 29, "GITI-029", "SHARD", "NEON", "ALIEN", 185, "GREEN/VIOLET" },
    { 30, "GITI-030", "STORM", "GRAVITY", "MACHINE", 186, "GOLD/PURPLE" },
    { 31, "GITI-031", "ORBIT", "PLASMA", "BASS", 187, "CYAN/MAGENTA" },
    { 32, "GITI-032", "SIGNAL", "VOID", "FM", 188, "VIOLET/CYAN" },
    { 33, "GITI-033", "HYPERION", "CYBER", "LEAD", 189, "RED/CYAN" },
    { 34, "GITI-034", "ZENITH", "QUANTUM", "SCREECH", 190, "GREEN/VIOLET" },
    { 35, "GITI-035", "KARBALA", "SONIC", "ACID", 174, "GOLD/PURPLE" },
    { 36, "GITI-036", "PERSIA", "ACID", "ATMOS", 175, "CYAN/MAGENTA" },
    { 37, "GITI-037", "TEHRAN", "PHOTON", "DRONE", 176, "VIOLET/CYAN" },
    { 38, "GITI-038", "ISATIS", "ECLIPSE", "RITUAL", 177, "RED/CYAN" },
    { 39, "GITI-039", "ALIEN", "NEON", "ALIEN", 178, "GREEN/VIOLET" },
    { 40, "GITI-040", "UFO", "GRAVITY", "MACHINE", 179, "GOLD/PURPLE" },
    { 41, "GITI-041", "DIMENSION", "PLASMA", "BASS", 180, "CYAN/MAGENTA" },
    { 42, "GITI-042", "GENESIS", "VOID", "FM", 181, "VIOLET/CYAN" },
    { 43, "GITI-043", "SYNTHESIS", "CYBER", "LEAD", 182, "RED/CYAN" },
    { 44, "GITI-044", "MATRIX", "QUANTUM", "SCREECH", 183, "GREEN/VIOLET" },
    { 45, "GITI-045", "PHOENIX", "SONIC", "ACID", 184, "GOLD/PURPLE" },
    { 46, "GITI-046", "DRAGON", "ACID", "ATMOS", 185, "CYAN/MAGENTA" },
    { 47, "GITI-047", "CELESTIAL", "PHOTON", "DRONE", 186, "VIOLET/CYAN" },
    { 48, "GITI-048", "INFINITY", "ECLIPSE", "RITUAL", 187, "RED/CYAN" },
    { 49, "GITI-049", "SABER", "NEON", "ALIEN", 188, "GREEN/VIOLET" },
    { 50, "GITI-050", "SINGULARITY", "GRAVITY", "MACHINE", 189, "GOLD/PURPLE" }
};

inline constexpr int kGenesisEditionCount = 50;

inline const Edition& currentEdition() noexcept
{
#if defined(GITI_EDITION_ID)
    constexpr int requested = GITI_EDITION_ID;
#else
    constexpr int requested = 1;
#endif
    constexpr int index = (requested < 1 || requested > kGenesisEditionCount) ? 0 : requested - 1;
    return genesisEditions[index];
}

inline juce::String editionId() { return "GENESIS #" + juce::String(currentEdition().id).paddedLeft('0', 3); }
inline juce::String editionCode() { return currentEdition().code; }
inline juce::String editionName() { return currentEdition().name; }
inline juce::String editionSignature()
{
    const auto& e = currentEdition();
    return juce::String(e.code) + " • " + e.name + " • " + e.element + " • " + e.archetype
         + " • " + juce::String(e.bpm) + " BPM";
}
} // namespace giti

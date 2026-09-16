#pragma once
#include <JuceHeader.h>
#include <cmath>

namespace salek {

/** SALEK HIGHTECH — Persian / Dastgah microtuning.
 *  Modes: Equal, Shur, Segah, Homayun, Mahur, FreeKoron.
 */
struct PersianScale
{
    enum Mode : int
    {
        Equal = 0,
        Shur,
        Segah,
        Homayun,
        Mahur,
        FreeKoron,
        NumModes
    };

    static const char* modeName (int m)
    {
        switch (m)
        {
            case Equal:     return "Equal 12-TET";
            case Shur:      return "Shur";
            case Segah:     return "Segah";
            case Homayun:   return "Homayun";
            case Mahur:     return "Mahur";
            case FreeKoron: return "Free Koron";
            default:        return "Equal";
        }
    }

    static const float* tableFor (int mode)
    {
        static const float shur[12] = {
            0.f, 149.f, 300.f, 400.f, 500.f, 600.f,
            702.f, 783.f, 885.f, 985.f, 1085.f, 1150.f
        };
        static const float segah[12] = {
            0.f, 100.f, 198.f, 352.f, 495.f, 600.f,
            707.f, 826.f, 900.f, 1013.f, 1100.f, 1150.f
        };
        static const float homayun[12] = {
            0.f, 100.f, 200.f, 398.f, 502.f, 600.f,
            715.f, 800.f, 900.f, 990.f, 1080.f, 1150.f
        };
        static const float mahur[12] = {
            0.f, 104.f, 208.f, 300.f, 397.f, 497.f,
            600.f, 702.f, 800.f, 891.f, 994.f, 1100.f
        };

        switch (mode)
        {
            case Shur:    return shur;
            case Segah:   return segah;
            case Homayun: return homayun;
            case Mahur:   return mahur;
            default:      return nullptr;
        }
    }

    static float noteToHz (int midiNote, int octaveOffset, int semiOffset, float fineCents,
                           int mode, float koronCents, int rootMidi = 60)
    {
        const int note = midiNote + octaveOffset * 12 + semiOffset;
        const float fine = fineCents;

        if (mode == Equal || mode < 0 || mode >= NumModes)
            return 440.0f * std::pow (2.0f, (float) (note - 69) / 12.0f + fine / 1200.0f);

        if (mode == FreeKoron)
        {
            int pc = ((note - rootMidi) % 12 + 12) % 12;
            float extra = 0.f;
            if (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10)
                extra = koronCents;
            return 440.0f * std::pow (2.0f, (float) (note - 69) / 12.0f + (fine + extra) / 1200.0f);
        }

        const float* tbl = tableFor (mode);
        if (tbl == nullptr)
            return 440.0f * std::pow (2.0f, (float) (note - 69) / 12.0f + fine / 1200.0f);

        const int rel = note - rootMidi;
        int octaves = rel / 12;
        int pc = rel % 12;
        if (pc < 0) { pc += 12; --octaves; }

        const float centsFromTonic = tbl[pc] + fine;
        const float rootHz = 440.0f * std::pow (2.0f, (float) (rootMidi - 69) / 12.0f);
        return rootHz * std::pow (2.0f, (float) octaves + centsFromTonic / 1200.0f);
    }
};

} // namespace salek

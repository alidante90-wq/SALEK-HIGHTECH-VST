#pragma once
#include <JuceHeader.h>
#include <atomic>

// SALEK UNIVERSE / GITI E01 identity seed. This is a public product fingerprint,
// not a secret key. License codes are still validated by their salted SHA-256 hash.

namespace giti
{
namespace license
{
inline juce::String normalise (juce::String code)
{
    return code.trim().toUpperCase().removeCharacters (" -_\t\r\n");
}
inline juce::String fingerprint (const juce::String& code)
{
    const auto n = normalise (code);
    const juce::String payload =
        "376692CA7C88CEA09A76B1CB9C404D906F6EB48362AB74510EE38D24C7D56BAE|" + n;
    return juce::SHA256 (payload.toRawUTF8(), (size_t) payload.getNumBytesAsUTF8())
        .toHexString().toUpperCase();
}

// Production fingerprints are intentionally empty until SALEK supplies the issued codes.
// Add SHA256(normalised-code) strings here; plaintext customer codes are never stored.
inline const juce::StringArray& trialCodeHashes()
{
    static const juce::StringArray hashes
    {
    "9A765E13707DCDF173CFD89920DA4AB41621695214D724D97D1FF0981A346F21",
    "3BF460CA7B355D00B7FF129DF7B596E39442BC4C657743F0770DBBDBE8505EC8",
    "7A29DD3C689A100B26482BE7A975D625F4D5F7D3E5E9B4C767643570798B43E6",
    "886261648C82CE5D8F211D47D351C2DE87FB914610C51C9209310EB90C4ADE5A",
    "BBAACDD251F309F43A3AA9C93420446B0358232598CAEE6CA90C1D9D9E8B1B94",
    "982242B38A568F4FA6793277BA6D7E9991E3361AF1DD2930647DF0FFDAF87A44",
    "084C540A25998C962B18E29E953368F46699F622425FA81341D25A1AB0AF424F",
    "8B5DC24BCF39DCC2F979A800F91EA144F9BBBBB2068D2FE3A9E9E340FA7B9AB0",
    "6D6B33B8663FAD3BD87EE03949DD104850B4AFDDEB539EF6E424DBAAFDBCA4C8",
    "61A5478C5A82B891A4F5F9BA54ABEB9D2CF0B66938550B06DA2201A5C5D70844"
    };
    return hashes;
}
inline const juce::StringArray& permanentCodeHashes()
{
    static const juce::StringArray hashes
    {
    "7F8CED0D25D4F87A5209905CE601CA40153AED1C6F98D82B4165CD5E67F0FFC2",
    "B545205E1862A9911C94E93C124D8221B4A8E3CDAC2D826F9944ED34135DF649",
    "558E565E34D8BEA7FB2BBA875394A618602E9CED6A51C870AD52F0AF8DFBCEE5",
    "9858926A33FB8BFEF55934A3CA56B3ED7A2384405F416AE38D187048E7CC51F5",
    "39939410DC4DD66F1B11D9CE425F9A6E3BD7B8C1AF15AFC6A6B90CE4CE0CE76D",
    "5853B40AC10374F0810513CA64D5039BD62632955D3D2B2B68218E94DB4785C5",
    "C8BE5E2D3DB2353779120F8FE4E283A2B25AB1538097B4C3B5706152DA24B97C",
    "4D636749BD5DA578177CE5B7F7FA34BDBCFC0C64D0F38D59BA4CA98264EF95EC",
    "1AFF6EB695AFC44DC38CBFF73FAD66F0A7761448BBD15CAE88900D8115D62A6E",
    "BF9D857373EE508C462E2A4FF643DA641F37070B69DE83E7327CC14D57C692C2"
    };
    return hashes;
}
}

class ThreeDayTrial
{
public:
    static constexpr int64 trialHours = 72;
    enum class ActivationResult { invalid, threeDay, permanent };

    ThreeDayTrial() { initialise(); }

    bool isActive() const noexcept { return active.load(); }
    bool isExpired() const noexcept { return ! active.load(); }
    bool isPermanent() const noexcept { return mode.load() == 2; }
    bool isNemoMode() const noexcept
    {
       #if defined (GITI_NEMO_MODE) && GITI_NEMO_MODE
        return true;
       #else
        return false;
       #endif
    }

    int hoursRemaining() const noexcept
    {
        if (! active.load() || isPermanent()) return 0;
        const auto left = juce::jmax<int64> (0, activationMs + trialHours * 60 * 60 * 1000LL - nowMs());
        return (int) juce::jmax<int64> (0, (left + 3599999) / 3600000);
    }
    int daysRemaining() const noexcept
    {
        const int h = hoursRemaining();
        return h <= 0 ? 0 : (h + 23) / 24;
    }
    juce::String statusText() const
    {
        if (isPermanent()) return "LICENSED • PERMANENT";
        if (! active.load())
            return isNemoMode() ? "NEMOGITI LOCKED • 3-DAY CODE REQUIRED"
                                : "TRIAL EXPIRED — LICENSE REQUIRED";
        if (mode.load() == 1)
            return "CODE ACTIVATED • " + juce::String (daysRemaining()) + " DAY(S) REMAINING";
        return "3-DAY TRIAL • " + juce::String (daysRemaining()) + " DAY(S) REMAINING";
    }

    ActivationResult activateCode (const juce::String& rawCode)
    {
        const auto fp = license::fingerprint (rawCode);
        if (license::permanentCodeHashes().contains (fp))
        {
            mode.store (2); active.store (true); activationMs = nowMs(); writeState (lastSeenMs);
            return ActivationResult::permanent;
        }
        if (license::trialCodeHashes().contains (fp))
        {
            if (isPermanent()) return ActivationResult::permanent;
            const auto now = nowMs();
            activationMs = now; lastSeenMs = now; mode.store (1); active.store (true); writeState (lastSeenMs);
            return ActivationResult::threeDay;
        }
        return ActivationResult::invalid;
    }

private:
    std::atomic<bool> active { false };
    std::atomic<int> mode { 0 }; // 0 automatic trial, 1 code 72h, 2 permanent
    int64 firstRunMs = 0, activationMs = 0, lastSeenMs = 0;
    juce::File stateFile;

    static juce::File getStateFile()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                     .getChildFile ("SALEK").getChildFile ("GITI BY SALEK");
        dir.createDirectory();
       #if defined (GITI_NEMO_MODE) && GITI_NEMO_MODE
        return dir.getChildFile ("nemogiti_license.dat");
       #else
        return dir.getChildFile ("trial.dat");
       #endif
    }
    static int64 nowMs() { return juce::Time::getCurrentTime().toMilliseconds(); }
    bool clockValid (int64 now) const noexcept { return lastSeenMs <= 0 || now + 5 * 60 * 1000LL >= lastSeenMs; }

    void initialise()
    {
        stateFile = getStateFile();
        const auto now = nowMs();
        if (stateFile.existsAsFile())
        {
            const auto lines = juce::StringArray::fromLines (stateFile.loadFileAsString().trim());
            if (lines.size() >= 4)
            {
                firstRunMs = lines[0].getLargeIntValue();
                lastSeenMs = lines[1].getLargeIntValue();
                mode.store (juce::jlimit (0, 2, lines[2].getIntValue()));
                activationMs = lines[3].getLargeIntValue();
            }
            else if (lines.size() >= 2)
            {
                firstRunMs = lines[0].getLargeIntValue();
                lastSeenMs = lines[1].getLargeIntValue();
                activationMs = firstRunMs;
            }
        }

       #if defined (GITI_NEMO_MODE) && GITI_NEMO_MODE
        if (mode.load() == 2) { active.store (true); return; }
        if (mode.load() == 1 && activationMs > 0 && clockValid (now)
            && now - activationMs < trialHours * 60 * 60 * 1000LL)
        {
            lastSeenMs = juce::jmax (lastSeenMs, now); writeState (lastSeenMs); active.store (true); return;
        }
        active.store (false);
       #else
        if (firstRunMs <= 0)
        {
            firstRunMs = now; activationMs = now; lastSeenMs = now; mode.store (0);
            writeState (lastSeenMs); active.store (true); return;
        }
        if (! clockValid (now)) { active.store (false); return; }
        if (mode.load() == 2) { active.store (true); return; }
        const auto start = mode.load() == 1 ? activationMs : firstRunMs;
        if (now - start >= trialHours * 60 * 60 * 1000LL)
        {
            active.store (false); writeState (juce::jmax (lastSeenMs, now)); return;
        }
        lastSeenMs = juce::jmax (lastSeenMs, now); writeState (lastSeenMs); active.store (true);
       #endif
    }

    void writeState (int64 lastSeen) const
    {
        if (! stateFile.getParentDirectory().isDirectory()) stateFile.getParentDirectory().createDirectory();
        stateFile.replaceWithText (juce::String (firstRunMs) + "\n"
                                 + juce::String (lastSeen) + "\n"
                                 + juce::String (mode.load()) + "\n"
                                 + juce::String (activationMs) + "\nGITI-LICENSE-V2\n",
                                 false, false, "\n");
    }
};
} // namespace giti

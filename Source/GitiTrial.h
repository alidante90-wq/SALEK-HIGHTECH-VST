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
        "F9AD206CBC3B50BA57159720D6237F146B0411BC9C04CA7F67B282A16503DB37",
        "E1F2D6FB79F2BCEF3B90E8AE02E06393A18B47AC7458702EF83F050DD26DE068",
        "A6993E6ACEDE82EB3B6D94C1BE94146BB41ACCC86D3E9C632D440E43D40872C5",
        "3597AF38038F621D6B29D6D2A0CB92674F3E81B9B918B4DEFAEB82F0B3CAE5F3",
        "CEE76443165F323ADC42277668AD1DCEB7B86F08308C354E4A87D7C88570A1A1"
    };
    return hashes;
}
inline const juce::StringArray& permanentCodeHashes()
{
    static const juce::StringArray hashes
    {
        "14BBBB8F340BF99E07163A6C0C736916581FBC21CA896035913DCAA9FD20709A",
        "E1B6FB80FC70E22EBAE98CCC2DA960F098B357E99CC2A10513CF604BE9258BD5",
        "0CC48BCFFFE0EDF9FFEB897B1616EB5F09440A4993F0F1B1C2167F027F6F286E",
        "6D0FAC5BC4C671B89A8AF2228DAE71820B18B8D7D99443F1BD84FA33456BB822",
        "900A17686721B4362F0AD70299D74FA2B2AE459B9B4D967994B17343C0298038"
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

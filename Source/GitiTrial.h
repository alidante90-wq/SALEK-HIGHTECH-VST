#pragma once
#include <JuceHeader.h>
#include <atomic>

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
    return juce::SHA256 (n.toRawUTF8(), (size_t) n.getNumBytesAsUTF8()).toHexString().toUpperCase();
}

// Production fingerprints are intentionally empty until SALEK supplies the issued codes.
// Add SHA256(normalised-code) strings here; plaintext customer codes are never stored.
inline const juce::StringArray& trialCodeHashes()
{
    static const juce::StringArray hashes;
    return hashes;
}
inline const juce::StringArray& permanentCodeHashes()
{
    static const juce::StringArray hashes;
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

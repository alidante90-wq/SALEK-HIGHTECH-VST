#pragma once
#include <JuceHeader.h>

namespace giti
{
class ThreeDayTrial
{
public:
    static constexpr int64 trialHours = 72;

    ThreeDayTrial()
    {
        initialise();
    }

    bool isActive() const noexcept { return active; }
    bool isExpired() const noexcept { return !active; }

    int hoursRemaining() const noexcept
    {
        if (!active) return 0;
        const auto now = juce::Time::getCurrentTime().toMilliseconds();
        const auto left = juce::jmax<int64> (0, (firstRunMs + trialHours * 60 * 60 * 1000LL) - now);
        return (int) juce::jmax<int64> (0, (left + 3599999) / 3600000);
    }

    int daysRemaining() const noexcept
    {
        const auto h = hoursRemaining();
        return h <= 0 ? 0 : (h + 23) / 24;
    }

    juce::String statusText() const
    {
        if (! active)
            return "TRIAL EXPIRED — LICENSE REQUIRED";
        return "3-DAY TRIAL • " + juce::String (daysRemaining()) + " DAY(S) REMAINING";
    }

private:
    bool active = true;
    int64 firstRunMs = 0;
    juce::File stateFile;

    static juce::File getStateFile()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                     .getChildFile ("SALEK")
                     .getChildFile ("GITI BY SALEK");
        dir.createDirectory();
        return dir.getChildFile ("trial.dat");
    }

    static int64 nowMs()
    {
        return juce::Time::getCurrentTime().toMilliseconds();
    }

    void initialise()
    {
        stateFile = getStateFile();
        const auto now = nowMs();

        int64 lastSeen = 0;
        if (stateFile.existsAsFile())
        {
            auto text = stateFile.loadFileAsString().trim();
            auto lines = juce::StringArray::fromLines (text);
            if (lines.size() >= 2)
            {
                firstRunMs = lines[0].getLargeIntValue();
                lastSeen = lines[1].getLargeIntValue();
            }
        }

        if (firstRunMs <= 0)
        {
            firstRunMs = now;
            lastSeen = now;
            writeState (lastSeen);
            active = true;
            return;
        }

        // Basic clock-rollback protection. This is an offline evaluation guard,
        // not a cryptographic licensing system; production licensing should use
        // a server-issued signed license if stronger protection is required.
        if (now + 5 * 60 * 1000LL < lastSeen)
        {
            active = false;
            return;
        }

        if (now - firstRunMs >= trialHours * 60 * 60 * 1000LL)
        {
            active = false;
            writeState (juce::jmax (lastSeen, now));
            return;
        }

        writeState (juce::jmax (lastSeen, now));
        active = true;
    }

    void writeState (int64 lastSeen) const
    {
        if (! stateFile.getParentDirectory().isDirectory())
            stateFile.getParentDirectory().createDirectory();

        stateFile.replaceWithText (juce::String (firstRunMs) + "\n" + juce::String (lastSeen) + "\n",
                                   false, false, "\n");
    }
};
} // namespace giti

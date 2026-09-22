#pragma once
#include <JuceHeader.h>

/** Early-access license locker for GITI / SALEK HIGHTECH.
 *  Offline HMAC codes, machine-bound after first unlock.
 *  Not unbreakable DRM — raises the bar against casual copy.
 */
namespace SalekLicense
{
    /** Human-readable machine fingerprint (show in unlock dialog). */
    juce::String getMachineId();

    /** True if a valid non-expired activation is stored for this machine. */
    bool isLicensed();

    /** Validate an access code. On success, bind to this machine and persist.
     *  Code format: SALEK-XXXX-XXXX-XXXX (case-insensitive).
     *  Returns empty string on success, or error message. */
    juce::String tryActivate (const juce::String& code);

    /** Clear local activation (owner / support). */
    void deactivate();

    /** Days left in license (-1 = unlimited, 0 = expired/none). */
    int daysRemaining();

    /** Optional soft demo: true only when never activated (for trial builds). */
    bool isDemoMode();

    /** Generate an access code for a machine (owner tool).
     *  daysValid: 0 = unlimited early-access for that machine. */
    juce::String generateCodeForMachine (const juce::String& machineId, int daysValid = 0);

    /** Owner secret version / product id tag. */
    juce::String productTag();
}

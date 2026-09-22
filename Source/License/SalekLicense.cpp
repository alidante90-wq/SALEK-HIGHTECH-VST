#include "SalekLicense.h"
#include <cstdint>

namespace SalekLicense
{
namespace
{
    // Obfuscated product pepper (not public API key). Rotate by changing these bytes + version.
    static const uint8_t kPepper[] = {
        0x53,0x41,0x4c,0x45,0x4b,0x2d,0x47,0x49,0x54,0x49,0x2d,0x45,0x41,
        0x32,0x30,0x32,0x36,0x2d,0x48,0x54,0x2d,0x9a,0x3c,0x71,0xe2,0x44
    };

    juce::File licenseFile()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("SALEK-HIGHTECH")
                       .getChildFile ("License");
        dir.createDirectory();
        return dir.getChildFile ("giti_ea.lic");
    }

    juce::String normalizeMachine (const juce::String& s)
    {
        return s.trim().toUpperCase().retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    }

    juce::String hashHex (const juce::String& input)
    {
        // Simple portable mix (not crypto-grade; enough for casual locker)
        juce::MemoryBlock mb (input.toRawUTF8(), (size_t) input.getNumBytesAsUTF8());
        uint64_t h = 1469598103934665603ull; // FNV-ish
        const auto* p = static_cast<const uint8_t*> (mb.getData());
        for (size_t i = 0; i < mb.getSize(); ++i)
        {
            h ^= p[i];
            h *= 1099511628211ull;
            for (auto b : kPepper)
            {
                h ^= b;
                h = (h << 7) | (h >> 57);
            }
        }
        // fold to 12 hex chars
        juce::String out;
        for (int i = 0; i < 3; ++i)
        {
            const uint32_t part = (uint32_t) (h >> (i * 16));
            out << juce::String::toHexString ((int) (part & 0xffff)).paddedLeft ('0', 4);
        }
        return out.toUpperCase();
    }

    juce::String formatCode (const juce::String& hex12)
    {
        auto h = hex12.paddedRight ('0', 12).substring (0, 12).toUpperCase();
        return "SALEK-" + h.substring (0, 4) + "-" + h.substring (4, 8) + "-" + h.substring (8, 12);
    }

    juce::String stripCode (const juce::String& code)
    {
        return normalizeMachine (code).replace ("SALEK", {});
    }

    struct LicData
    {
        juce::String machine;
        juce::String codeHash;
        int64_t expiryEpoch = 0; // 0 = none
        int version = 1;
    };

    bool loadLic (LicData& out)
    {
        auto f = licenseFile();
        if (! f.existsAsFile()) return false;
        auto xml = juce::XmlDocument::parse (f);
        if (xml == nullptr) return false;
        out.machine = xml->getStringAttribute ("m");
        out.codeHash = xml->getStringAttribute ("h");
        out.expiryEpoch = (int64_t) xml->getStringAttribute ("e").getLargeIntValue();
        out.version = xml->getIntAttribute ("v", 1);
        return out.machine.isNotEmpty() && out.codeHash.isNotEmpty();
    }

    void saveLic (const LicData& d)
    {
        juce::XmlElement xml ("SALEK_EA");
        xml.setAttribute ("m", d.machine);
        xml.setAttribute ("h", d.codeHash);
        xml.setAttribute ("e", juce::String (d.expiryEpoch));
        xml.setAttribute ("v", d.version);
        xml.setAttribute ("p", productTag());
        xml.writeTo (licenseFile());
    }

    juce::String expectedPayload (const juce::String& machineId, int daysValid)
    {
        // daysValid encoded into hash material (0 = unlimited)
        return normalizeMachine (machineId) + "|D" + juce::String (daysValid) + "|GITI-EA1";
    }
} // namespace

juce::String productTag() { return "GITI-EA-2026"; }

juce::String getMachineId()
{
    // Stable-ish fingerprint
    juce::String raw;
    raw << juce::SystemStats::getLogonName();
    raw << "|";
    raw << juce::SystemStats::getComputerName();
    raw << "|";
    raw << juce::SystemStats::getOperatingSystemName();
    raw << "|";
    raw << juce::SystemStats::getDeviceDescription();
    // 8-char display id
    auto h = hashHex (raw);
    return h.substring (0, 4) + "-" + h.substring (4, 8);
}

juce::String generateCodeForMachine (const juce::String& machineId, int daysValid)
{
    auto mid = normalizeMachine (machineId);
    if (mid.length() < 4)
        return {};
    auto hex = hashHex (expectedPayload (mid, daysValid));
    return formatCode (hex);
}

juce::String tryActivate (const juce::String& code)
{
    auto mid = normalizeMachine (getMachineId());
    auto body = stripCode (code);
    if (body.length() < 8)
        return "Invalid code format. Use SALEK-XXXX-XXXX-XXXX";

    // Accept unlimited (days=0) and common limited windows
    const int trials[] = { 0, 7, 14, 30, 90, 180, 365 };
    bool ok = false;
    int matchedDays = 0;
    for (int d : trials)
    {
        auto expect = stripCode (generateCodeForMachine (mid, d));
        if (expect == body)
        {
            ok = true;
            matchedDays = d;
            break;
        }
    }
    // Also allow codes generated with the display form (with dashes) normalized the same way
    if (! ok)
    {
        // try matching against raw machine string user might have typed without dashes
        for (int d : trials)
        {
            auto expect = stripCode (generateCodeForMachine (getMachineId(), d));
            if (expect == body)
            {
                ok = true;
                matchedDays = d;
                break;
            }
        }
    }

    if (! ok)
        return "Code not valid for this machine. Send Machine ID to SALEK.";

    LicData d;
    d.machine = mid;
    d.codeHash = hashHex (body + "|" + mid);
    d.expiryEpoch = 0;
    if (matchedDays > 0)
        d.expiryEpoch = (int64_t) (juce::Time::getCurrentTime().toMilliseconds() / 1000)
                        + (int64_t) matchedDays * 86400;
    d.version = 1;
    saveLic (d);
    return {};
}

bool isLicensed()
{
    LicData d;
    if (! loadLic (d)) return false;
    if (normalizeMachine (d.machine) != normalizeMachine (getMachineId()))
        return false;
    if (d.expiryEpoch > 0)
    {
        const int64_t now = (int64_t) (juce::Time::getCurrentTime().toMilliseconds() / 1000);
        if (now > d.expiryEpoch)
            return false;
    }
    return true;
}

void deactivate()
{
    auto f = licenseFile();
    if (f.existsAsFile())
        f.deleteFile();
}

int daysRemaining()
{
    LicData d;
    if (! loadLic (d)) return 0;
    if (normalizeMachine (d.machine) != normalizeMachine (getMachineId()))
        return 0;
    if (d.expiryEpoch <= 0) return -1;
    const int64_t now = (int64_t) (juce::Time::getCurrentTime().toMilliseconds() / 1000);
    if (now > d.expiryEpoch) return 0;
    return (int) ((d.expiryEpoch - now) / 86400);
}

bool isDemoMode() { return ! isLicensed(); }
} // namespace SalekLicense

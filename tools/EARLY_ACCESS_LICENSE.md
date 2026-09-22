# GITI Early Access — License Locker

## Artist flow
1. Install VST3 from your Early Access build.
2. Open plugin → dialog shows **Machine ID** (e.g. `A1B2-C3D4`).
3. Artist sends Machine ID to you (SALEK).
4. You generate a code and send it.
5. Artist enters code → audio unlocks on **that machine only**.

## You generate codes
```bash
python3 tools/generate_ea_code.py A1B2-C3D4        # unlimited
python3 tools/generate_ea_code.py A1B2-C3D4 30   # 30 days
```

## Security notes
- Offline HMAC-style code bound to machine fingerprint.
- Casual copy of the VST folder to another PC will **not** carry the license.
- This is **not** military DRM — determined reverse engineers can bypass any offline lock.
- For stronger control later: online activation server + short-lived session tokens.

## Files
- `Source/License/SalekLicense.h/.cpp`
- License store: `%AppData%/SALEK-HIGHTECH/License/giti_ea.lic` (Windows)

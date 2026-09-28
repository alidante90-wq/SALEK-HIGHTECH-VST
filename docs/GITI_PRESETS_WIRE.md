# GITI presets + identities (from metadata pack)

## Files
- `Source/PluginProcessorPresetsGITI.inl` — 50 factory presets (GITI/001 … GITI/050)
- `Source/GitiIdentities.h` — titles, branch, color, mood, voice, frequency power

## Wire presets
In `Source/PluginProcessorPresets.inl`, after:
```cpp
#include "PluginProcessorPresetsEngineered.inl"
```
add:
```cpp
#include "PluginProcessorPresetsGITI.inl"
```
before the closing `}` of `initFactoryPresets()`.

## Wire identities (optional)
```cpp
#include "GitiIdentities.h"
// giti::kIdentities[0] .. [49]
```

No UI layout changes. No wallet/mint in the VST binary unless you add it separately.

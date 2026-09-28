# Wire GITI 50 presets into SALEK HIGHTECH

In `Source/PluginProcessorPresets.inl`, after:

```cpp
#include "PluginProcessorPresetsEngineered.inl"
```

add:

```cpp
#include "PluginProcessorPresetsGITI.inl"
```

before the closing `}` of `initFactoryPresets()`.

Rebuild. Factory list gains GITI/001 … GITI/050.
No UI layout changes.

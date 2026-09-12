# SALEK HIGHTECH v1.2 UI update

## What changed in this commit
- Your real logo is embedded (LOGO BACK.png) — always shows even without Assets folder
- TORONOWLA / face / LIAN / cyan girl art supported when PNGs are present
- Stronger dynamic glow and full transparent visualizer bar above the keyboard
- Keyboard keys stretch to fill the full width (no empty right side)
- Preset list forced to rebuild on open

## Add the character images (for full TORONOWLA art)

Put these files in Source/Assets/:

- logo.png (optional — already embedded as fallback)
- toronowla.png
- face.png
- lian.png
- cyan_girl.png
- headphones.png

Easiest way on GitHub:
1. Open https://github.com/alidante90-wq/SALEK-HIGHTECH-VST
2. Go to Source/Assets
3. Add file → Upload files
4. Drop the PNGs and commit

## Rebuild
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

After rebuild you get logo + glow + full keyboard + presets.
With the PNGs uploaded you also get TORONOWLA and theme character art.

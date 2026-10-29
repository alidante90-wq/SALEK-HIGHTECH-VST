# Build all GITI artist editions

## One-command local build (Windows)

1. Install Visual Studio 2022 Build Tools with the **Desktop development with C++** workload, CMake 3.22+, Git, and PowerShell 7 (Windows PowerShell 5.1 is also suitable).
2. From the repository root, ensure JUCE 8.0.4 is available in `JUCE/` or allow CMake FetchContent to download it. Internet access is required if it is not present locally.
3. For both ZIP packages and Windows installer EXEs, install Inno Setup 6. Then run:

```powershell
.\\tools\\Build-All-Artists.ps1 -Configuration Release
```

The script builds AVANAVA, PSYELL, AUDIOLOGIST, ASHKANPASHA, BRAIN POISON, MIL.ODY, IKKI, OSOM, ANAHITA, and SALEK sequentially. Every edition has its own CMake build directory, product name, and four-character plugin code to reduce host/cache identity collisions.

## Output

- `dist/<ARTIST>/` contains the staged plugin and install notes.
- `dist/<ARTIST>-GITI-Artist-Build.zip` is the portable package for that artist.
- `dist/setups/<ARTIST>-GITI-Setup.exe` is generated when Inno Setup 6 is installed.
- `dist/build-report.csv` records the build results.

If Inno Setup is absent, ZIP files are still created and setup EXEs are skipped with a warning. To build ZIP packages only, run `.\tools\Build-All-Artists.ps1 -SkipInstaller`.

## GitHub Actions

The Windows workflow builds all ten editions and uploads `GITI-All-Artist-Builds` as a workflow artifact. Open **Actions → Build All GITI Artist Versions → Run workflow** to run it manually on the desired branch. The workflow must pass before treating the output as a verified release.

## Important

This is a shared source tree with artist-specific product/plugin identities, not ten separately engineered DSP engines. Installer builds and compilation must be checked in CI before sending the installers to artists. The first successful package is not a substitute for DAW compatibility testing on target systems.

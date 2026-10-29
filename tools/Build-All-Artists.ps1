[CmdletBinding()]
param([string]$Configuration = "Release", [string]$OutputRoot = "dist", [switch]$SkipInstaller)
$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
Push-Location $repo
try {
    $null = Get-Command cmake -ErrorAction Stop
    $hasNinja = [bool](Get-Command ninja -ErrorAction SilentlyContinue)
    $artists = @(
        @{ Name="AVANAVA"; Code="Av01" }, @{ Name="PSYELL"; Code="Psy2" },
        @{ Name="AUDIOLOGIST"; Code="Aud3" }, @{ Name="ASHKANPASHA"; Code="Ash4" },
        @{ Name="BRAIN POISON"; Code="Bra5" }, @{ Name="MIL.ODY"; Code="Mil6" },
        @{ Name="IKKI"; Code="Ikk7" }, @{ Name="OSOM"; Code="Oso8" },
        @{ Name="ANAHITA"; Code="Ana9" }, @{ Name="SALEK"; Code="Sal0" }
    )
    $out = Join-Path $repo $OutputRoot
    New-Item -ItemType Directory -Force -Path $out | Out-Null
    $results = @()
    foreach ($artist in $artists) {
        $slug = ($artist.Name -replace "[^A-Za-z0-9]+","-").Trim("-")
        $buildDir = Join-Path $repo ("build-artists\" + $slug)
        $stage = Join-Path $out $slug
        Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
        New-Item -ItemType Directory -Force -Path $stage | Out-Null
        Write-Host ("=== BUILDING " + $artist.Name + " ===") -ForegroundColor Cyan
        $args = @("-S", $repo, "-B", $buildDir, "-DCMAKE_BUILD_TYPE=$Configuration", "-DSALEK_ARTIST_NAME=$($artist.Name)", "-DSALEK_PLUGIN_CODE=$($artist.Code)")
        if ($hasNinja) { $args += @("-G", "Ninja") } else { $args += @("-G", "Visual Studio 17 2022", "-A", "x64") }
        & cmake @args
        if ($LASTEXITCODE -ne 0) { throw "CMake configure failed for $($artist.Name)" }
        & cmake --build $buildDir --config $Configuration --parallel
        if ($LASTEXITCODE -ne 0) { throw "Build failed for $($artist.Name)" }
        $vst3 = Get-ChildItem -Path $buildDir -Recurse -Directory -Filter "*.vst3" | Where-Object { Test-Path (Join-Path $_.FullName "Contents") } | Select-Object -First 1
        if (-not $vst3) { throw "VST3 bundle not found for $($artist.Name)" }
        Copy-Item $vst3.FullName (Join-Path $stage ($artist.Name + ".vst3")) -Recurse
        $standalone = Get-ChildItem -Path $buildDir -Recurse -File -Filter "*.exe" | Where-Object { $_.FullName -match "Standalone" -and $_.Name -notmatch "unins|cmake|ninja" } | Select-Object -First 1
        if ($standalone) { Copy-Item $standalone.FullName (Join-Path $stage ($artist.Name + ".exe")) }
        $readme = ($artist.Name + " - GITI Artist Build`r`nVST3 bundle: " + $artist.Name + ".vst3`r`nInstall by copying the VST3 folder to your DAW VST3 folder, then rescan plugins.`r`n")
        Set-Content -Path (Join-Path $stage "README-INSTALL.txt") -Value $readme -Encoding UTF8
        $zip = Join-Path $out ($slug + "-GITI-Artist-Build.zip")
        Remove-Item $zip -Force -ErrorAction SilentlyContinue
        Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $zip -CompressionLevel Optimal
        $installer = $null
        if (-not $SkipInstaller) {
            $iscc = Get-Command ISCC.exe -ErrorAction SilentlyContinue
            if (-not $iscc) {
                $candidate = (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe")
                if (Test-Path $candidate) { $iscc = Get-Item $candidate }
            }
            if ($iscc) {
                $setupDir = Join-Path $out "setups"
                New-Item -ItemType Directory -Force -Path $setupDir | Out-Null
                & $iscc.Source "/DArtistName=$($artist.Name)" "/DStageDir=$stage" "/DOutputDir=$setupDir" "/DOutputBaseName=$slug-GITI-Setup" (Join-Path $repo "tools\ArtistInstaller.iss")
                if ($LASTEXITCODE -ne 0) { throw "Installer build failed for $($artist.Name)" }
                $installer = Join-Path $setupDir ($slug + "-GITI-Setup.exe")
            } else { Write-Warning "Inno Setup not installed; ZIP is ready, setup EXE skipped." }
        }
        $results += [pscustomobject]@{ Artist=$artist.Name; Status="PASS"; Zip=$zip; Setup=$installer }
        Write-Host ("PACKAGED: " + $zip) -ForegroundColor Green
    }
    $results | Format-Table -AutoSize
    $results | Export-Csv (Join-Path $out "build-report.csv") -NoTypeInformation -Encoding UTF8
    Write-Host ("All artist builds completed. Output: " + $out) -ForegroundColor Green
} finally { Pop-Location }

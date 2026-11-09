$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$EnvFile = Join-Path $Root ".env"
if (-not (Test-Path $EnvFile)) {
    Copy-Item (Join-Path $Root ".env.example") $EnvFile
    Write-Host "Created .env. Free-credit budget is zero; generation is locked."
}
$Py = Get-Command py -ErrorAction SilentlyContinue
if ($Py) {
    & py -3 (Join-Path $Root "server.py")
} else {
    & python (Join-Path $Root "server.py")
}

#Requires -Version 7.0
param(
    [string]$Version = "1.0.0",
    [switch]$Install
)

$ErrorActionPreference = "Stop"
$root    = Split-Path -Parent $MyInvocation.MyCommand.Path
$dist    = Join-Path $root "dist"
$release = Join-Path $root "release"
$stage   = Join-Path $release ("stage-v" + $Version)
$zipPath = Join-Path $release ("DL1-Enemy-HP-And-Damage-Numbers-v" + $Version + ".zip")

# 1. Fresh x64 Release build (and optionally install to local game folder)
if ($Install) {
    & (Join-Path $root "build.ps1") -Install
} else {
    & (Join-Path $root "build.ps1")
}

# 2. Prepare clean staging folder (only end-user files for Nexus Mods)
if (Test-Path $stage) {
    Remove-Item -Recurse -Force $stage
}
New-Item -ItemType Directory -Force $stage | Out-Null

Copy-Item -Force (Join-Path $dist "winmm.dll")         $stage
Copy-Item -Force (Join-Path $dist "dl1_hp_damage.ini") $stage
Copy-Item -Force (Join-Path $dist "README.txt")        $stage
Copy-Item -Force (Join-Path $root "LICENSE")           $stage

# 3. Create Nexus Mods zip archive
if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}
Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $zipPath -CompressionLevel Optimal
Remove-Item -Recurse -Force $stage

$zipInfo = Get-Item $zipPath
$dllHash = (Get-FileHash -Algorithm SHA256 (Join-Path $dist "winmm.dll")).Hash
$zipHash = (Get-FileHash -Algorithm SHA256 $zipPath).Hash

Write-Host ""
Write-Host ("Nexus release archive ready: {0} ({1:N0} bytes)" -f $zipInfo.FullName, $zipInfo.Length) -ForegroundColor Green
Write-Host ("  winmm.dll SHA256 : {0}" -f $dllHash) -ForegroundColor Gray
Write-Host ("  zip       SHA256 : {0}" -f $zipHash) -ForegroundColor Gray

#Requires -Version 7.0
param(
    [switch]$Install,
    [string]$GameDir = "G:\SteamLibrary\steamapps\common\Dying Light"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src  = Join-Path $root "src"
$obj  = Join-Path $root "build_obj"
$dist = Join-Path $root "dist"

New-Item -ItemType Directory -Force $obj, $dist | Out-Null

# Locate Visual Studio 2022 vcvars64.bat
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw "Visual Studio 2022 C++ x64 build tools not found."
}
$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at $vcvars"
}

$outDll  = Join-Path $dist "winmm.dll"
$defFile = Join-Path $src "winmm_proxy.def"
$asmFile = Join-Path $src "winmm_stubs.asm"
$asmObj  = Join-Path $obj "winmm_stubs.obj"
$sources = @(
    (Join-Path $src "dllmain.cpp"),
    (Join-Path $src "winmm_proxy.cpp"),
    (Join-Path $src "config.cpp"),
    (Join-Path $src "hook.cpp"),
    (Join-Path $src "game_hooks.cpp"),
    (Join-Path $src "d3d11_renderer.cpp")
) -join '" "'

$cmd = "`"$vcvars`" && ml64.exe /nologo /c /Fo`"$asmObj`" `"$asmFile`" && cl.exe /nologo /O2 /MT /EHsc /std:c++17 /W3 /DWIN32_LEAN_AND_MEAN /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /Fo`"$obj\\`" `"$sources`" `"$asmObj`" /LD /Fe`"$outDll`" /link /DEF:`"$defFile`" user32.lib gdi32.lib d3d11.lib dxgi.lib d3dcompiler.lib"

Write-Host "Building winmm.dll (x64 Release)..." -ForegroundColor Cyan
cmd.exe /c $cmd
if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

Remove-Item -Force -ErrorAction SilentlyContinue (Join-Path $dist "winmm.lib"), (Join-Path $dist "winmm.exp")
$dllInfo = Get-Item $outDll
Write-Host ("Build succeeded: {0} ({1:N0} bytes)" -f $dllInfo.FullName, $dllInfo.Length) -ForegroundColor Green

if ($Install) {
    if (-not (Test-Path $GameDir)) {
        throw "Game directory not found: $GameDir"
    }
    Copy-Item -Force $outDll (Join-Path $GameDir "winmm.dll")
    $dstIni = Join-Path $GameDir "dl1_hp_damage.ini"
    if (-not (Test-Path $dstIni)) {
        Copy-Item -Force (Join-Path $dist "dl1_hp_damage.ini") $dstIni
        Write-Host "Installed winmm.dll + dl1_hp_damage.ini to $GameDir" -ForegroundColor Green
    } else {
        Write-Host "Installed winmm.dll to $GameDir (kept existing dl1_hp_damage.ini)" -ForegroundColor Green
    }
}

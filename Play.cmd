@echo off
rem MyMetalSlug2 launcher: incremental Release build (+ asset bake if missing), then run. Extra arguments are passed to the game (e.g. Play.cmd --life 10).
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts\Build.ps1 -Config Release
if not exist "Assets\Baked\manifest.json" (
    powershell -NoProfile -ExecutionPolicy Bypass -File Scripts\Bake.ps1
)
start "" "Output\Release\MyMetalSlug2.exe" %*

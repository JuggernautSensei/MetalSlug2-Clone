@echo off
rem MetalSlug2_Clone launcher: incremental Release build, then run. Extra arguments are passed to the game (e.g. Play.cmd --life 10).
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts\Build.ps1 -Config Release
start "" "Output\Release\MetalSlug2_Clone.exe" %*

@echo off
rem MyMetalSlug2 launcher: builds Release first if the exe is missing.
cd /d "%~dp0"
if not exist "Build\vs\bin\Release\MyMetalSlug2.exe" (
    powershell -NoProfile -ExecutionPolicy Bypass -File Scripts\Build.ps1 -Config Release
)
start "" "Build\vs\bin\Release\MyMetalSlug2.exe"

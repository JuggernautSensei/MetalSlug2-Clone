@echo off
rem MyMetalSlug2 launcher: builds Release first if the exe is missing.
cd /d "%~dp0"
if not exist "Output\Release\MyMetalSlug2.exe" (
    powershell -NoProfile -ExecutionPolicy Bypass -File Scripts\Build.ps1 -Config Release
)
start "" "Output\Release\MyMetalSlug2.exe"

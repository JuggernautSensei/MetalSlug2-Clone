@echo off
rem Opens the generated Visual Studio solution (creates it first if missing).
cd /d "%~dp0"
if not exist "Build\vs\MyMetalSlug2.slnx" (
    set VCPKG_ROOT=C:\vcpkg
    "C:\Program Files\CMake\bin\cmake.exe" --preset vs
)
start "" "Build\vs\MyMetalSlug2.slnx"

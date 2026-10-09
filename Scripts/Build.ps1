# 동시에 여러 에이전트/프로세스가 같은 Build 폴더를 쓰지 않도록 전역 뮤텍스로 직렬화한다.
# 사용: powershell -NoProfile -File Scripts/Build.ps1 [-Config Debug|Release] [-Target SlugTest] [-Reconfigure]
param(
    [ValidateSet('Debug', 'Release')] [string] $Config = 'Debug',
    [string] $Target = '',
    [switch] $Reconfigure
)

$ErrorActionPreference = 'Continue'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
$env:VCPKG_ROOT = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { 'C:\vcpkg' }
$cmake = 'C:\Program Files\CMake\bin\cmake.exe'

$mutex = New-Object System.Threading.Mutex($false, 'Global\MyMetalSlug2Build')
$bLocked = $false
try {
    $bLocked = $mutex.WaitOne([TimeSpan]::FromMinutes(30))
    if (-not $bLocked) { Write-Output 'build lock timeout'; exit 3 }

    if ($Reconfigure -or -not (Test-Path 'Build/vs/CMakeCache.txt')) {
        & $cmake --preset vs 2>&1 | Select-Object -Last 15
    }

    $preset = if ($Config -eq 'Release') { 'vs-release' } else { 'vs-debug' }
    $cmakeArgs = @('--build', '--preset', $preset)
    if ($Target) { $cmakeArgs += @('--target', $Target) }
    $out = & $cmake @cmakeArgs 2>&1
    $code = $LASTEXITCODE

    # 에러 요약만 출력 (전체 로그는 Build/last_build.log)
    $out | Out-File -Encoding utf8 'Build/last_build.log'
    $out | Select-String -Pattern 'error [A-Z]+[0-9]+|error:|fatal error|LNK[0-9]+|FAILED' | Select-Object -First 40
    Write-Output "exit=$code"
    exit $code
}
finally {
    if ($bLocked) { $mutex.ReleaseMutex() }
}

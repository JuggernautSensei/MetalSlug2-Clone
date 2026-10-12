# 동작 불변 검증: 리팩터링 전후 결과가 같은지 한 번에 확인한다.
#   1) 빌드 (게임 + SlugTest)
#   2) Regress.ps1  : 헤드리스 리플레이 해시 (Docs/Wayfinder/Baseline.txt)
#   3) 캡처 해시    : 상태별 스크린샷 SHA256 (Docs/Wayfinder/BaselineCapture.txt, 오프스크린 + 소프트웨어 렌더)
#   4) 튜닝 덤프 해시 (--dump-tuning)
#   5) SlugTest     : 실패 목록이 알려진 기존 실패와 같은지
# 사용: powershell -NoProfile -File Scripts/Verify.ps1 [-Update] [-NoBuild] [-Config Debug|Release]
#   -Update  캡처/튜닝 기준선을 현재 값으로 기록 (의도한 동작 변경일 때만)
param(
    [switch] $Update,
    [switch] $NoBuild,
    [ValidateSet('Debug', 'Release')] [string] $Config = 'Debug'
)
$ErrorActionPreference = 'Continue'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$knownFailures = @('AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor')
$captureStates = @('stage0', 'bosslab', 'helilab', 'buslab', 'title', 'uidemo', 'sandbox')
$baselinePath  = Join-Path $root 'Docs/Wayfinder/BaselineCapture.txt'
$bFail = $false

if (-not $NoBuild)
{
    foreach ($target in @('MetalSlug2_Clone', 'SlugTest'))
    {
        & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $root 'Scripts/Build.ps1') -Config $Config -Target $target
        if ($LASTEXITCODE -ne 0) { Write-Output "FAIL: build $target"; exit 1 }
    }
}

$exe  = Join-Path $root "Output/$Config/MetalSlug2_Clone.exe"
$test = Join-Path $root "Output/$Config/SlugTest.exe"

# 1) 회귀 리플레이 해시
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $root 'Scripts/Regress.ps1') -NoBuild -Config $Config | Where-Object { $_ -notmatch '^\[info\]' }
if ($LASTEXITCODE -ne 0) { $bFail = $true }

# 2) 캡처 + 튜닝 해시
$current = @()
$tuning = (& $exe --headless --dump-tuning) -join "`n"
$sha = [System.Security.Cryptography.SHA256]::Create()
$current += 'tuning ' + (($sha.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($tuning)) | ForEach-Object { $_.ToString('x2') }) -join '')

$capDir = Join-Path $env:TEMP 'slug_verify_cap'
New-Item -ItemType Directory -Force -Path $capDir | Out-Null
$env:SDL_VIDEO_DRIVER  = 'offscreen'
$env:SDL_RENDER_DRIVER = 'software'
foreach ($state in $captureStates)
{
    $bmp = Join-Path $capDir "$state.bmp"
    $expected = $null
    if (Test-Path $baselinePath) { foreach ($line in Get-Content $baselinePath) { $p = $line -split ' '; if ($p[0] -eq "capture_$state") { $expected = $p[1] } } }
    # 일부 상태(uidemo)는 벽시계 의존으로 간헐적으로 다른 프레임이 잡힌다: 기준선과 다르면 최대 3회까지 다시 찍는다.
    $h = 'MISSING'
    for ($try = 0; $try -lt 3; ++$try)
    {
        Remove-Item $bmp -ErrorAction SilentlyContinue
        & $exe --no-audio --state $state --seed 1 --capture $bmp --capture-frame 120 *> $null
        if (Test-Path $bmp) { $h = (Get-FileHash $bmp -Algorithm SHA256).Hash.ToLower() } else { $h = 'MISSING' }
        if ($Update -or $h -eq $expected) { break }
    }
    $current += "capture_$state $h"
}
Remove-Item Env:SDL_VIDEO_DRIVER, Env:SDL_RENDER_DRIVER -ErrorAction SilentlyContinue

if ($Update)
{
    Set-Content -Path $baselinePath -Value $current -Encoding ascii
    Write-Output 'capture/tuning baseline UPDATED'
}
else
{
    $baseline = @{}
    if (Test-Path $baselinePath) { foreach ($line in Get-Content $baselinePath) { $p = $line -split ' '; if ($p.Count -ge 2) { $baseline[$p[0]] = $p[1] } } }
    foreach ($line in $current)
    {
        $p = $line -split ' '
        if ($baseline.ContainsKey($p[0]) -and $baseline[$p[0]] -eq $p[1] -and $p[1] -ne 'MISSING') { Write-Output "$($p[0]) PASS" }
        else { Write-Output "$($p[0]) FAIL"; $bFail = $true }
    }
}

# 3) 단위 테스트
$out = & $test 2>&1
$failed = @($out | Where-Object { $_ -match '^\[  FAILED  \] ([A-Za-z0-9_]+\.[A-Za-z0-9_]+)\s*$' } | ForEach-Object { ($_ -replace '^\[  FAILED  \] ', '').Trim() } | Sort-Object -Unique)
$summary = $out | Where-Object { $_ -match '^\[  PASSED  \]|^\[==========\] [0-9]+ tests' }
$summary | ForEach-Object { Write-Output $_ }
$unexpected = @($failed | Where-Object { $knownFailures -notcontains $_ })
if ($unexpected.Count -gt 0) { Write-Output ("FAIL: unexpected test failures: " + ($unexpected -join ', ')); $bFail = $true }
else { Write-Output 'unit tests PASS (known failures only)' }

if ($bFail) { Write-Output 'VERIFY: FAIL'; exit 1 }
Write-Output 'VERIFY: PASS'
exit 0

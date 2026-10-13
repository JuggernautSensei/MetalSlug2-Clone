# 회귀 기준선: MetalSlug2_Clone 를 빌드하고 --replay --hash (오프스크린, 실시간 대기 없음) 로 Test/Replays/*.replay 를 돌려 상태 해시를 Docs/Wayfinder/Baseline.txt 와 비교한다.
# 사용: powershell -NoProfile -File Scripts/Regress.ps1 [-Update] [-NoBuild] [-Config Debug|Release]
#   -Update  현재 해시를 Baseline.txt 로 기록 (의도한 동작 변경일 때만)
param(
    [switch] $Update,
    [switch] $NoBuild,
    [ValidateSet('Debug', 'Release')] [string] $Config = 'Debug'
)
$ErrorActionPreference = 'Continue'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

if (-not $NoBuild)
{
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $root 'Scripts/Build.ps1') -Config $Config -Target MetalSlug2_Clone
    if ($LASTEXITCODE -ne 0) { Write-Output 'FAIL: build'; exit 1 }
}
$exe = Get-ChildItem -Path (Join-Path $root 'Output') -Recurse -Filter 'MetalSlug2_Clone.exe' |
    Where-Object { $_.FullName -match $Config } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $exe) { Write-Output 'FAIL: MetalSlug2_Clone.exe not found'; exit 1 }

$baselinePath = Join-Path $root 'Docs/Wayfinder/Baseline.txt'
$baseline = @{}
if (Test-Path $baselinePath)
{
    foreach ($line in Get-Content $baselinePath)
    {
        if ($line -match '^(\S+)\s+hash=([0-9a-f]+)') { $baseline[$Matches[1]] = $Matches[2] }
    }
}

$env:SDL_VIDEO_DRIVER  = 'offscreen'
$env:SDL_RENDER_DRIVER = 'software'
$results = @()
$bFail = $false
foreach ($file in Get-ChildItem (Join-Path $root 'Test/Replays') -Filter '*.replay' | Sort-Object Name)
{
    $name = $file.BaseName
    $out = (& $exe.FullName --replay $file.FullName --hash) -join ' '
    if ($out -match 'hash=([0-9a-f]+)') { $hash = $Matches[1] } else { $hash = 'ERROR'; $bFail = $true }
    $results += "$name hash=$hash"
    if ($Update) { $verdict = 'UPDATED' }
    elseif ($baseline.ContainsKey($name) -and $baseline[$name] -eq $hash) { $verdict = 'PASS' }
    else { $verdict = 'FAIL'; $bFail = $true }
    Write-Output "$name hash=$hash $verdict"
}
Remove-Item Env:SDL_VIDEO_DRIVER, Env:SDL_RENDER_DRIVER -ErrorAction SilentlyContinue
if ($Update -and -not $bFail) { Set-Content -Path $baselinePath -Value $results -Encoding ascii }
if ($bFail) { Write-Output 'RESULT: FAIL'; exit 1 }
Write-Output 'RESULT: PASS'
exit 0

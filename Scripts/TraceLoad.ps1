# 텍스처 로드 진단: --trace-load 의 "[tex]" 줄을 상태/모드(async, sync) 별로 모아 기준선과 비교한다.
#   - async 표와 --no-async-load(sync) 표가 같아야 한다.
#   - 각 표가 Scripts/Baseline/BaselineTextures.txt 와 같아야 한다.
# 사용: powershell -NoProfile -File Scripts/TraceLoad.ps1 [-Update] [-Config Debug|Release]
#   -Update  기준선을 현재 값으로 기록 (의도한 로드 변경일 때만)
param(
    [switch] $Update,
    [ValidateSet('Debug', 'Release')] [string] $Config = 'Debug'
)
$ErrorActionPreference = 'Continue'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$states       = @('title', 'stage0', 'bosslab', 'sandbox')
$modes        = @('async', 'sync')
$exe          = Join-Path $root "Output/$Config/MetalSlug2_Clone.exe"
$baselinePath = Join-Path $root 'Scripts/Baseline/BaselineTextures.txt'
$tmpDir       = Join-Path $env:TEMP 'slug_traceload'
New-Item -ItemType Directory -Force -Path $tmpDir | Out-Null

if (-not (Test-Path $exe)) { Write-Output "FAIL: exe not found: $exe"; exit 1 }

$env:SDL_VIDEO_DRIVER  = 'offscreen'
$env:SDL_RENDER_DRIVER = 'software'

function Get-TexLines([string] $state, [string] $mode)
{
    $bmp = Join-Path $tmpDir "$state.bmp"
    $err = Join-Path $tmpDir "$state.$mode.txt"
    Remove-Item $bmp, $err -ErrorAction SilentlyContinue
    $extra = ''
    if ($mode -eq 'sync') { $extra = ' --no-async-load' }
    $cmd = "`"$exe`" --no-audio --state $state --seed 1 --capture `"$bmp`" --capture-frame 5 --trace-load$extra 2> `"$err`" 1> nul"
    & cmd.exe /c $cmd
    if (-not (Test-Path $err)) { return @() }
    return @(Get-Content $err | Where-Object { $_ -match '^\[tex\] ' })
}

$current = [ordered]@{}
foreach ($state in $states)
{
    foreach ($mode in $modes)
    {
        $current["$state $mode"] = Get-TexLines $state $mode
    }
}
Remove-Item Env:SDL_VIDEO_DRIVER, Env:SDL_RENDER_DRIVER -ErrorAction SilentlyContinue

$bFail = $false

if ($Update)
{
    $out = @()
    foreach ($key in $current.Keys)
    {
        $out += "## $key"
        $out += $current[$key]
    }
    Set-Content -Path $baselinePath -Value $out -Encoding ascii
    Write-Output "texture baseline UPDATED ($($current.Keys.Count) tables)"
}

$baseline = @{}
if (Test-Path $baselinePath)
{
    $key = $null
    foreach ($line in Get-Content $baselinePath)
    {
        if ($line -match '^## (.+)$') { $key = $Matches[1]; $baseline[$key] = @() }
        elseif ($key -and $line -ne '') { $baseline[$key] += $line }
    }
}

foreach ($state in $states)
{
    $a = $current["$state async"]
    $s = $current["$state sync"]
    if ($a.Count -eq 0) { Write-Output "$state async-vs-sync FAIL (no [tex] lines)"; $bFail = $true }
    elseif (($a -join "`n") -ceq ($s -join "`n")) { Write-Output "$state async-vs-sync PASS ($($a.Count) lines)" }
    else { Write-Output "$state async-vs-sync FAIL"; $bFail = $true }

    foreach ($mode in $modes)
    {
        $key = "$state $mode"
        if ($baseline.ContainsKey($key) -and (($baseline[$key] -join "`n") -ceq ($current[$key] -join "`n"))) { Write-Output "$key baseline PASS" }
        else { Write-Output "$key baseline FAIL"; $bFail = $true }
    }
}

if ($bFail) { Write-Output 'TRACELOAD: FAIL'; exit 1 }
Write-Output 'TRACELOAD: PASS'
exit 0

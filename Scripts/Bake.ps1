# 레거시 리소스를 Assets/Baked 로 변환한다 (MyMetalSlug2.sln 의 SlugBaker 프로젝트를 빌드한 뒤 실행).
# 사용:
#   powershell -NoProfile -File Scripts/Bake.ps1
#   powershell -NoProfile -File Scripts/Bake.ps1 -Only player,npc          # 부분 굽기 (빠른 반복용; 출력이 부분적이므로 -Out 을 따로 쓰는 것을 권장)
#   powershell -NoProfile -File Scripts/Bake.ps1 -Threshold 192,96,128     # 지형 임계값 B,RG,A
#   powershell -NoProfile -File Scripts/Bake.ps1 -Legacy <Resources 경로> -Out Assets/Baked -NoBuild
param(
    [string]   $Legacy    = '',
    [string]   $Out       = 'Assets/Baked',
    [string[]] $Only      = @(),
    [string]   $Threshold = '',
    [string[]] $Extra     = @(),
    [ValidateSet('Debug', 'Release')] [string] $Config = 'Debug',
    [switch]   $NoBuild
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

if (-not $Legacy) {
    $Legacy = Join-Path (Split-Path -Parent $root) 'Client\Output\Binaries\Resources'
}
if (-not (Test-Path (Join-Path $Legacy 'ResourceData'))) {
    Write-Error "legacy Resources 경로를 찾을 수 없습니다: $Legacy (-Legacy 로 지정하세요)"
    exit 2
}

if (-not $NoBuild) {
    & powershell -NoProfile -File (Join-Path $PSScriptRoot 'Build.ps1') -Config $Config -Target SlugBaker
    if ($LASTEXITCODE -ne 0) { Write-Error 'SlugBaker 빌드 실패'; exit $LASTEXITCODE }
}

$exe = Join-Path $root "Output\$Config\SlugBaker.exe"
if (-not (Test-Path $exe)) { Write-Error "SlugBaker.exe 가 없습니다: $exe"; exit 3 }

$bakerArgs = @('--legacy', $Legacy, '--out', $Out)
if ($Only.Count -gt 0)  { $bakerArgs += @('--only', ($Only -join ',')) }
if ($Threshold)         { $bakerArgs += @('--threshold', $Threshold) }
$bakerArgs += $Extra

& $exe @bakerArgs
exit $LASTEXITCODE

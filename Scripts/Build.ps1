# MetalSlug2_Clone.sln 을 MSBuild 로 빌드하는 얇은 래퍼. CMake 는 쓰지 않는다.
# 사용: powershell -NoProfile -File Scripts/Build.ps1 [-Config Debug|Release] [-Target SlugTest] [-Reconfigure (무시됨)]
# 동시에 여러 에이전트/프로세스가 같은 Output/Intermediate 를 쓰지 않도록 전역 뮤텍스로 직렬화한다.
# 출력은 에러 요약과 exit=<code> 만 보여주며, 전체 로그는 Intermediate\last_build.log 에 남는다.
param(
    [ValidateSet('Debug', 'Release')] [string] $Config = 'Debug',
    [string] $Target = '',
    [switch] $Reconfigure
)

$ErrorActionPreference = 'Continue'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
$sln = Join-Path $root 'MetalSlug2_Clone.sln'
$logDir = Join-Path $root 'Intermediate'
$logPath = Join-Path $logDir 'last_build.log'

function Find-MSBuild
{
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere)
    {
        $found = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\amd64\MSBuild.exe' | Select-Object -First 1
        if ($found) { return $found }
    }
    return 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
}

$mutex = New-Object System.Threading.Mutex($false, 'Global\MetalSlug2_CloneBuild')
$bLocked = $false
try
{
    $bLocked = $mutex.WaitOne([TimeSpan]::FromMinutes(30))
    if (-not $bLocked) { Write-Output 'build lock timeout'; exit 3 }

    $msbuild = Find-MSBuild
    if (-not (Test-Path $msbuild)) { Write-Output "MSBuild not found: $msbuild"; exit 4 }

    $msbuildArgs = @($sln, "/p:Configuration=$Config", '/p:Platform=x64', '/m', '/v:minimal', '/nologo')
    if ($Target) { $msbuildArgs += "/t:$Target" }

    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $out = & $msbuild @msbuildArgs 2>&1
    $code = $LASTEXITCODE

    $out | Out-File -Encoding utf8 $logPath
    $out | Select-String -Pattern ': error |error [A-Z]+[0-9]+|LNK[0-9]+|MSB[0-9]+' | Select-Object -First 40 | ForEach-Object { $_.Line }
    Write-Output "exit=$code"
    exit $code
}
finally
{
    if ($bLocked) { $mutex.ReleaseMutex() }
}

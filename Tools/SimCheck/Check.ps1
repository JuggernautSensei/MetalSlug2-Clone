param([switch] $Record)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Push-Location $root
$video = $env:SDL_VIDEO_DRIVER
$render = $env:SDL_RENDER_DRIVER
try
{
    & powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1 -Target SimCheck
    if ($LASTEXITCODE -ne 0) { throw 'SimCheck build failed' }
    $env:SDL_VIDEO_DRIVER = 'offscreen'
    $env:SDL_RENDER_DRIVER = 'software'
    $ErrorActionPreference = 'Continue'
    $output = @(& "$root/Output/Debug/SimCheck.exe" 2> "$root/Intermediate/simcheck.log")
    $ErrorActionPreference = 'Stop'
    if ($LASTEXITCODE -ne 0) { throw 'SimCheck execution failed' }
    $lines = @($output | Where-Object { $_ -cmatch '^SIM ' })
    if ($lines.Count -ne 780) { throw "Incomplete output: $($lines.Count)/780" }
    $baseline = Join-Path $PSScriptRoot 'baseline.txt'
    if ($Record)
    {
        if (Test-Path -LiteralPath $baseline) { throw 'Baseline already exists; do not overwrite' }
        $lines | Set-Content -LiteralPath $baseline -Encoding ascii
        Write-Output "SIMCHECK: recorded $($lines.Count) checkpoints"
    }
    else
    {
        $difference = @(Compare-Object (Get-Content -LiteralPath $baseline) $lines -SyncWindow 0)
        if ($difference.Count -gt 0)
        {
            $difference | Select-Object -First 12 | Format-Table
            throw "SimCheck mismatch: $($difference.Count) lines"
        }
        Write-Output "SIMCHECK: PASS ($($lines.Count) checkpoints)"
    }
}
finally
{
    $env:SDL_VIDEO_DRIVER = $video
    $env:SDL_RENDER_DRIVER = $render
    Pop-Location
}

# Source/Game 의 하드코딩 좌표/오프셋/크기/속도/시간 상수를 찾아 Docs/Wayfinder/C6_audit.csv 로 내보낸다.
# 사용: powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/AuditCoords.ps1
# 종료코드: 0 = 미허용 항목 0건, 1 = 미허용 항목 있음 (회귀 규칙으로 사용)
param([string[]] $Path = @())
$Path = @($Path | ForEach-Object { $_ -split ',' })
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$gameDir = Join-Path $root 'Source/Game'
$allowPath = Join-Path $root 'Scripts/AuditCoords.allow.txt'
$outPath = Join-Path $root 'Docs/Wayfinder/C6_audit.csv'
$inv = [Globalization.CultureInfo]::InvariantCulture

$numRx = '[-+]?(?:\d[\d\x27]*(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?[fF]?'
$pureRx = '^\s*(' + $numRx + ')\s*$'
$tokRx = '(?<![\w.])' + $numRx + '(?!\w)'
$pairGroupRx = '\{([^{}]*)\}'
$vecCallRx = '\bVec2\s*\(([^()]*)\)'
$kRx = '^\s*(?:(?:inline|static)\s+)*(?:constexpr|const)\s+(?:const\s+)?(?:float|int|int8_t|int16_t|int32_t|int64_t|uint8_t|uint16_t|uint32_t|uint64_t|size_t|double)\s+(k[A-Z]\w*)\s*=\s*([^;]+);'
$arrRx = '^\s*(?:(?:inline|static|constexpr|const)\s+)*ARRAY\s*<\s*(?:float|int|int32_t|uint32_t|int64_t|uint64_t|double|size_t|uint8_t|uint16_t|int16_t)\s*,\s*[^>]*>\s+(\w+)\s*='
$cArrRx = '^\s*(?:(?:inline|static|constexpr|const)\s+)*(?:float|int|int32_t|uint32_t|int64_t|uint64_t|double|size_t|uint8_t|uint16_t|int16_t)\s+(\w+)\s*\[[^\]]*\]\s*='
$structRx = '^\s*(?:struct|class)\s+(\w+)\b'
$asgRx = '(\w+)\s*(?<![=!<>])=(?!=)'
$fnRx = '(\w+)\s*\([^()]*$'

function ConvertTo-Num([string]$s) {
    $t = $s -replace "[\x27\s]", ''
    $t = $t -replace '[fF]$', ''
    return [double]::Parse($t, $inv)
}

function Get-PureValue([string]$s) {
    $m = [regex]::Match($s, $pureRx)
    if ($m.Success) { return (ConvertTo-Num $m.Groups[1].Value) }
    return $null
}

function Test-PairFlag([string[]]$parts) {
    $v0 = Get-PureValue $parts[0]
    $v1 = Get-PureValue $parts[1]
    if (($null -ne $v0) -and ($null -ne $v1) -and ($v0 -eq 0) -and ($v1 -eq 0)) { return $false }
    if (($null -ne $v0) -and ($v0 -ne 0)) { return $true }
    if (($null -ne $v1) -and ($v1 -ne 0)) { return $true }
    foreach ($p in $parts) {
        if ($null -eq (Get-PureValue $p)) {
            $noIdx = $p -replace '\[[^\]]*\]', ''
            foreach ($t in [regex]::Matches($noIdx, $tokRx)) {
                if ((ConvertTo-Num $t.Value) -ne 0) { return $true }
            }
        }
    }
    return $false
}

function Test-VecArgsFlag([string[]]$args2) {
    foreach ($a in $args2) {
        $v = Get-PureValue $a
        if (($null -ne $v) -and ($v -ne 0)) { return $true }
    }
    return $false
}

function Get-Symbol([string]$code, [int]$idx) {
    $pre = $code.Substring(0, $idx)
    $fn = [regex]::Match($pre, $fnRx)
    if ($fn.Success) { return $fn.Groups[1].Value }
    $asg = [regex]::Matches($pre, $asgRx)
    if ($asg.Count -gt 0) { return $asg[$asg.Count - 1].Groups[1].Value }
    return '(익명)'
}

function Get-Kind([string]$s) {
    if ($s -match 'Color|Rgb|Tint|Alpha') { return 'color' }
    if ($s -match 'Offset|Shift|Dy$|Dx$') { return 'offset' }
    if ($s -match 'Size|Box|Extent|Blast|Radius|Half|Width|Height') { return 'size' }
    if ($s -match 'Speed|Vel|Accel|Gravity|Fall|Sweep|Return|Friction|Damping|PerTick|Dps|Rate|^v0$|Vx|Vy') { return 'speed' }
    if ($s -match 'Tick|Sec|Time|Interval|Delay|Duration|Timer|Wait') { return 'time' }
    if ($s -match 'Count|Max$|Rows|Cols|Slots|Frames$|Steps$|Levels|Variant|Kinds|Stage') { return 'count' }
    if ($s -match 'Anchor|Spawn|Pos|Position|Origin|Feet|Muzzle|Base|Top|Center|Bomb|Puff|Pod|Rotor|Para|Cannon|Land|Target|Respawn|Chute|Stairs|Soldier') { return 'position' }
    return 'other'
}

function Get-OwnerKey([string]$owner) {
    $o = $owner -replace '(Cfg|Defs?|Types|Params)$', ''
    if (-not $o) { return 'unknown' }
    return $o.Substring(0, 1).ToLower() + $o.Substring(1)
}

function Get-SymKey([string]$sym) {
    if ($sym.StartsWith('(')) { return '?' }
    $t = $sym -replace '^_+', '' -replace '^m_', ''
    if ($t -cmatch '^k[A-Z]') { $t = $t.Substring(1) }
    if (-not $t) { return '?' }
    return $t.Substring(0, 1).ToLower() + $t.Substring(1)
}

function Get-SuggestedPath([string]$area, [string]$owner, [string]$sym) {
    $key = Get-OwnerKey $owner
    $sk = Get-SymKey $sym
    if ($area -eq 'Enemies' -or $area -eq 'Props') { return "defs.$key.$sk" }
    return "tuning.$($area.ToLower()).$key.$sk"
}

# 허용 목록: <상대경로 또는 *>:<심볼명 | 줄 내용 정규식>
$allow = New-Object System.Collections.ArrayList
if (Test-Path -LiteralPath $allowPath) {
    foreach ($ln in [IO.File]::ReadAllLines($allowPath, [Text.Encoding]::UTF8)) {
        $t = $ln.Trim()
        if ($t -eq '' -or $t.StartsWith('#')) { continue }
        $idx = $t.IndexOf(':')
        if ($idx -lt 1) { continue }
        $p = $t.Substring(0, $idx).Trim()
        $rest = $t.Substring($idx + 1).Trim()
        if ($rest -match '^[A-Za-z_]\w*$') {
            [void]$allow.Add([pscustomobject]@{ Path = $p; Sym = $rest; Rx = $null })
        } else {
            [void]$allow.Add([pscustomobject]@{ Path = $p; Sym = $null; Rx = (New-Object Text.RegularExpressions.Regex($rest)) })
        }
    }
}

function Test-Allowed([string]$rel, [string]$sym, [string]$code) {
    foreach ($e in $allow) {
        if (($e.Path -ne '*') -and ($e.Path -ne $rel)) { continue }
        if ($null -ne $e.Rx) {
            if ($e.Rx.IsMatch($code)) { return $true }
        } elseif ($sym -eq $e.Sym) {
            return $true
        }
    }
    return $false
}

$files = @(Get-ChildItem -LiteralPath $gameDir -Recurse -File | Where-Object {
    (($_.Extension -eq '.cpp') -or ($_.Extension -eq '.h')) -and ($_.FullName -notmatch '[\\/]Ui[\\/]')
})
$relList = New-Object System.Collections.ArrayList
foreach ($f in $files) { [void]$relList.Add($f.FullName.Substring($root.Length + 1).Replace('\', '/')) }
$relArr = [string[]]$relList.ToArray()
[Array]::Sort($relArr, [StringComparer]::Ordinal)

$all = New-Object System.Collections.ArrayList

foreach ($rel in $relArr) {
    $lines = [IO.File]::ReadAllLines((Join-Path $root $rel), [Text.Encoding]::UTF8)
    $n = $lines.Count
    $codes = New-Object string[] $n
    $cmts = New-Object string[] $n
    for ($i = 0; $i -lt $n; $i++) {
        $ln = $lines[$i]
        $ci = $ln.IndexOf('//')
        if ($ci -ge 0) {
            $codes[$i] = $ln.Substring(0, $ci)
            $cmts[$i] = $ln.Substring($ci + 2).Trim()
        } else {
            $codes[$i] = $ln
            $cmts[$i] = ''
        }
    }

    $parts = $rel -split '/'
    $area = 'game'
    if ($parts.Count -gt 3) { $area = $parts[2] }
    $baseOwner = [IO.Path]::GetFileNameWithoutExtension($rel) -replace '(Cfg|Defs?|Types)$', ''

    $stack = New-Object System.Collections.ArrayList
    $depth = 0
    $pending = $null
    $covered = @{}

    for ($i = 0; $i -lt $n; $i++) {
        $code = $codes[$i]
        $owner = $baseOwner
        if ($stack.Count -gt 0) { $owner = $stack[$stack.Count - 1].Name }

        $arrMatch = [regex]::Match($code, $arrRx)
        if (-not $arrMatch.Success) { $arrMatch = [regex]::Match($code, $cArrRx) }
        if ($arrMatch.Success) {
            $text = ''
            $d = 0
            $seenOpen = $false
            $j = $i
            $done = $false
            while ((-not $done) -and ($j -lt $n) -and (($j - $i) -lt 60)) {
                $c = $codes[$j]
                $text += $c + "`n"
                foreach ($ch in $c.ToCharArray()) {
                    if ($ch -eq '{') { $d++; $seenOpen = $true }
                    elseif ($ch -eq '}') { $d-- }
                }
                if ($seenOpen -and ($d -le 0)) { $done = $true } else { $j++ }
            }
            if ($j -ge $n) { $j = $n - 1 }
            for ($k = $i; $k -le $j; $k++) { $covered[$k] = $true }
            $eq = $text.IndexOf('=')
            if ($eq -ge 0) {
                $afterEq = $text.Substring($eq + 1)
                $opens = ($afterEq.ToCharArray() | Where-Object { $_ -eq '{' }).Count
                $toks = @([regex]::Matches($afterEq, $tokRx) | ForEach-Object { $_.Value })
                if (($opens -eq 1) -and ($toks.Count -gt 0)) {
                    $val = '{' + ($toks -join ', ') + '}'
                    if ($val.Length -gt 100) { $val = $val.Substring(0, 100) + '...' }
                    $sym = $arrMatch.Groups[1].Value
                    [void]$all.Add([pscustomobject]@{ File = $rel; Line = $i + 1; Symbol = $sym; Value = $val; Pattern = 'C'; Owner = $owner; Area = $area; Code = $code; Comment = $cmts[$i]; Note = ("배열 원소 " + $toks.Count + "개"); Kind = (Get-Kind $sym); Allowed = (Test-Allowed $rel $sym $code) })
                }
            }
        }

        $kMatch = [regex]::Match($code, $kRx)
        if ($kMatch.Success -and ($kMatch.Groups[2].Value -match '\d')) {
            $sym = $kMatch.Groups[1].Value
            $val = $kMatch.Groups[2].Value.Trim()
            $note = 'k 상수'
            if ($val -match '/') { $note = 'k 상수 (환산식)' }
            [void]$all.Add([pscustomobject]@{ File = $rel; Line = $i + 1; Symbol = $sym; Value = $val; Pattern = 'B'; Owner = $owner; Area = $area; Code = $code; Comment = $cmts[$i]; Note = $note; Kind = (Get-Kind $sym); Allowed = (Test-Allowed $rel $sym $code) })
        }

        if (-not $covered.ContainsKey($i)) {
            foreach ($gm in [regex]::Matches($code, $pairGroupRx)) {
                $inner = $gm.Groups[1].Value
                if ($inner.Contains('"') -or ($inner -match '\b(nullptr|true|false|NULL)\b')) { continue }
                $pp = @($inner.Split(',') | ForEach-Object { $_.Trim() })
                if ($pp.Count -ne 2) { continue }
                if (-not (Test-PairFlag $pp)) { continue }
                $sym = Get-Symbol $code $gm.Index
                $val = $gm.Value
                $note = '벡터 리터럴'
                if ((Get-PureValue $pp[0]) -eq $null -or (Get-PureValue $pp[1]) -eq $null) { $note = '벡터 리터럴 (식 포함)' }
                [void]$all.Add([pscustomobject]@{ File = $rel; Line = $i + 1; Symbol = $sym; Value = $val; Pattern = 'A'; Owner = $owner; Area = $area; Code = $code; Comment = $cmts[$i]; Note = $note; Kind = (Get-Kind $sym); Allowed = (Test-Allowed $rel $sym $code) })
            }
        }

        if (-not $covered.ContainsKey($i)) {
            foreach ($vm in [regex]::Matches($code, $vecCallRx)) {
                $args2 = @($vm.Groups[1].Value.Split(',') | ForEach-Object { $_.Trim() })
                if (-not (Test-VecArgsFlag $args2)) { continue }
                $sym = Get-Symbol $code $vm.Index
                [void]$all.Add([pscustomobject]@{ File = $rel; Line = $i + 1; Symbol = $sym; Value = $vm.Value; Pattern = 'D'; Owner = $owner; Area = $area; Code = $code; Comment = $cmts[$i]; Note = 'Vec2 생성자 인자'; Kind = (Get-Kind $sym); Allowed = (Test-Allowed $rel $sym $code) })
            }
        }

        $sd = [regex]::Match($code, $structRx)
        if ($sd.Success -and ($code -notmatch ';\s*$')) { $pending = $sd.Groups[1].Value }
        foreach ($ch in $code.ToCharArray()) {
            if ($ch -eq '{') {
                $depth++
                if ($null -ne $pending) {
                    [void]$stack.Add([pscustomobject]@{ Name = $pending; Depth = $depth })
                    $pending = $null
                }
            } elseif ($ch -eq '}') {
                if (($stack.Count -gt 0) -and ($stack[$stack.Count - 1].Depth -eq $depth)) { $stack.RemoveAt($stack.Count - 1) }
                $depth--
            }
        }
    }
}

$rows = New-Object System.Collections.ArrayList
$allowedCount = 0
foreach ($m in $all) {
    if ($Path.Count -gt 0) {
        $bIn = $false
        foreach ($pp in $Path) { if ($m.File.StartsWith($pp.Replace([string][char]92, '/'))) { $bIn = $true } }
        if (-not $bIn) { continue }
    }
    if ($m.Allowed) { $allowedCount++; continue }
    $note = $m.Note
    if ($m.Comment) {
        $c = $m.Comment
        if ($c.Length -gt 60) { $c = $c.Substring(0, 60) + '...' }
        $note = $note + ' / 주석: ' + $c
    }
    $relArea = $m.Area
    [void]$rows.Add([pscustomobject][ordered]@{
        file = $m.File
        line = $m.Line
        symbol = $m.Symbol
        value = $m.Value
        kind = $m.Kind
        owner = $m.Owner
        suggestedJsonPath = (Get-SuggestedPath $relArea $m.Owner $m.Symbol)
        note = $note
    })
}

if (-not (Test-Path -LiteralPath (Split-Path -Parent $outPath))) { New-Item -ItemType Directory -Path (Split-Path -Parent $outPath) | Out-Null }
if ($Path.Count -gt 0) {
    # 경로 필터 실행은 CSV 를 덮어쓰지 않는다
} elseif ($rows.Count -gt 0) {
    $rows | Export-Csv -LiteralPath $outPath -NoTypeInformation -Encoding UTF8
} else {
    Set-Content -LiteralPath $outPath -Value 'file,line,symbol,value,kind,owner,suggestedJsonPath,note' -Encoding UTF8
}

Write-Output ("총 매치: " + $all.Count + "건 / allowlist 처리: " + $allowedCount + "건 / 미허용: " + $rows.Count + "건")
$kindCounts = $rows | Group-Object kind | Sort-Object Name | ForEach-Object { $_.Name + "=" + $_.Count }
Write-Output ("미허용 종류별: " + ($kindCounts -join ', '))
Write-Output ("CSV: " + $outPath)
foreach ($r in $rows) {
    Write-Output ($r.file + ':' + $r.line + ' [' + $r.kind + '] ' + $r.symbol + ' = ' + $r.value)
}
if ($rows.Count -gt 0) { exit 1 }
exit 0

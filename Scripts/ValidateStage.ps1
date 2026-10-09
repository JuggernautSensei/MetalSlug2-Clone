# Stage JSON 검증: 파싱, ID 유일/참조, 에셋 이름 존재, 섹션별 개수 출력.
# 사용: powershell -File Scripts/ValidateStage.ps1 [-Stage Assets/Stage/Stage0.json]
param(
    [string]$Stage = (Join-Path $PSScriptRoot '..\Assets\Stage\Stage0.json'),
    [string]$Baked = (Join-Path $PSScriptRoot '..\Assets\Baked')
)

$ErrorActionPreference = 'Stop'
$errors = New-Object System.Collections.Generic.List[string]
function Err([string]$m) { $script:errors.Add($m) }

$s = Get-Content $Stage -Raw -Encoding UTF8 | ConvertFrom-Json
$spr = (Get-Content (Join-Path $Baked 'sprites.json') -Raw -Encoding UTF8 | ConvertFrom-Json).sprites.PSObject.Properties.Name
$ani = (Get-Content (Join-Path $Baked 'anim.json') -Raw -Encoding UTF8 | ConvertFrom-Json).animations.PSObject.Properties.Name
$snd = (Get-Content (Join-Path $Baked 'sounds.json') -Raw -Encoding UTF8 | ConvertFrom-Json).sounds.PSObject.Properties.Name
$man = Get-Content (Join-Path $Baked 'manifest.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$gnd = $man.ground.PSObject.Properties.Name

function Has($o, [string]$k) { if ($null -eq $o) { return $false }; return $null -ne $o.PSObject.Properties[$k] }
function ChkSpr([string]$n, [string]$w) { if ($n -and ($spr -notcontains $n)) { Err "missing sprite '$n' ($w)" } }
function ChkAni([string]$n, [string]$w) { if ($n -and ($ani -notcontains $n)) { Err "missing anim '$n' ($w)" } }

# 최상위
ChkSpr $s.world.background 'world.background'
if ($snd -notcontains $s.bgm.name) { Err "missing sound '$($s.bgm.name)'" }
foreach ($m in $s.ground.maps) { if ($gnd -notcontains $m) { Err "missing ground '$m'" } }
if ($s.ground.maps -notcontains $s.ground.initial) { Err 'ground.initial not in ground.maps' }
foreach ($p in $s.backdrop.modes.PSObject.Properties) { ChkSpr $p.Value.sprite "backdrop.$($p.Name)" }
if (-not (Has $s.backdrop.modes $s.backdrop.initial)) { Err 'backdrop.initial unknown' }
$rects = if (Has $s 'rects') { $s.rects.PSObject.Properties.Name } else { @() }

# 섹션
$ids = @{}; $spawnIds = @{}; $gateIds = @{}; $prevX = -1
$secNames = @($s.sections | ForEach-Object { $_.id })
foreach ($sec in $s.sections) {
    if ($ids.ContainsKey($sec.id)) { Err "duplicate section id '$($sec.id)'" }
    $ids[$sec.id] = $true
    if ($sec.triggerX -lt $prevX) { Err "section '$($sec.id)' triggerX not ascending" }
    $prevX = $sec.triggerX
}
foreach ($c in $s.camera.boundsCurves) {
    if ($secNames -notcontains $c.from -or $secNames -notcontains $c.until) { Err "boundsCurve '$($c.id)' from/until unknown" }
}

function Walk($actions, [string]$where, $cnt) {
    foreach ($a in $actions) {
        switch ($a.type) {
            'spawn' {
                $n = 1; if (Has $a 'repeat') { $n = $a.repeat.count }
                $cnt.spawn += $n; $cnt[$a.kind] += $n
                if ($a.def -eq 'pow') { $cnt.pow += $n }
                if (Has $a 'id') { if ($spawnIds.ContainsKey($a.id)) { Err "duplicate spawn id '$($a.id)'" }; $spawnIds[$a.id] = $true }
                if (Has $a 'sprite') { ChkSpr $a.sprite $where }
                if (Has $a 'anim') { ChkAni $a.anim $where }
                if (Has $a.params 'sprites') { foreach ($x in $a.params.sprites) { ChkSpr $x $where } }
                if ($a.kind -eq 'collider' -and -not (Has $a 'size')) { Err "collider without size ($where)" }
                if ($a.kind -ne 'collider' -and (Has $a 'collisionLayer')) { Err "collisionLayer on non-collider ($where)" }
                if ((Has $a 'groundSnap') -and (-not (Has $a 'x'))) { Err "groundSnap without x ($where)" }
                if ((Has $a 'camera') -and ((Has $a 'x') -or (Has $a 'y'))) { Err "camera + x/y ($where)" }
                if (-not (Has $a 'camera') -and -not (Has $a 'x')) { Err "spawn without position ($where)" }
                if ((Has $a 'origin') -and $a.origin -eq 'legacyCenter' -and ($a.kind -in 'enemy', 'npc') -and -not (Has $a 'halfHeight')) { Err "legacyCenter creature without halfHeight ($where)" }
                if ((Has $a 'params') -and (Has $a.params 'arena') -and ($rects -notcontains $a.params.arena)) { Err "unknown rect '$($a.params.arena)' ($where)" }
            }
            'spawnWave' {
                foreach ($e in $a.entries) {
                    $n = 1; if (Has $e 'repeat') { $n = $e.repeat.count }
                    $cnt.wave += $n
                    if (-not (Has $e 'delay')) { Err "wave entry without delay ($where)" }
                    if (-not (Has $e 'camera') -and -not (Has $e 'x')) { Err "wave entry without position ($where)" }
                }
            }
            'lockCamera' {
                if ($a.rect -is [string] -and $a.rect -ne 'current' -and ($rects -notcontains $a.rect)) { Err "unknown rect '$($a.rect)' ($where)" }
            }
            'setBackdrop' { if (-not (Has $s.backdrop.modes $a.mode)) { Err "unknown backdrop mode '$($a.mode)' ($where)" } }
            'setGround' { if ($s.ground.maps -notcontains $a.name) { Err "unknown ground '$($a.name)' ($where)" } }
            'gate' {
                $cnt.gate++
                $gateIds[$a.id] = $a
                Walk $a.actions "$where/gate:$($a.id)" $cnt
            }
            'command' { $script:cmdTargets += $a.target }
            'playBgm' { if ($snd -notcontains $a.name) { Err "missing sound '$($a.name)' ($where)" } }
            { $_ -in 'setBounds', 'unlockCamera', 'cameraMove', 'setCameraOffset', 'clear', 'banner', 'fade', 'playerInvincible', 'endStage' } { }
            default { Err "unknown action type '$($a.type)' ($where)" }
        }
    }
}

$script:cmdTargets = @()
$total = [ordered]@{ spawn = 0; enemy = 0; npc = 0; prop = 0; decor = 0; item = 0; collider = 0; pow = 0; wave = 0; gate = 0 }
'{0,-16} {1,6} {2,6} {3,4} {4,5} {5,5} {6,5} {7,6} {8,5} {9,5}' -f 'section', 'spawn', 'enemy', 'pow', 'prop', 'decor', 'item', 'collid', 'wave', 'gate'
foreach ($sec in $s.sections) {
    $c = @{ spawn = 0; enemy = 0; npc = 0; prop = 0; decor = 0; item = 0; collider = 0; pow = 0; wave = 0; gate = 0 }
    Walk $sec.actions $sec.id $c
    '{0,-16} {1,6} {2,6} {3,4} {4,5} {5,5} {6,5} {7,6} {8,5} {9,5}' -f $sec.id, $c.spawn, $c.enemy, $c.pow, $c.prop, $c.decor, $c.item, $c.collider, $c.wave, $c.gate
    foreach ($k in @($total.Keys)) { $total[$k] += $c[$k] }
}
'{0,-16} {1,6} {2,6} {3,4} {4,5} {5,5} {6,5} {7,6} {8,5} {9,5}' -f 'TOTAL', $total.spawn, $total.enemy, $total.pow, $total.prop, $total.decor, $total.item, $total.collider, $total.wave, $total.gate

# 참조 검사
foreach ($g in $gateIds.Values) { foreach ($w in $g.waitFor) { if (-not $spawnIds.ContainsKey($w)) { Err "gate '$($g.id)' waitFor unknown spawn id '$w'" } } }
foreach ($t in $script:cmdTargets) { if (-not $spawnIds.ContainsKey($t)) { Err "command target unknown '$t'" } }

if ($errors.Count -gt 0) {
    Write-Host "`nFAILED: $($errors.Count) error(s)" -ForegroundColor Red
    $errors | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
    exit 1
}
Write-Host "`nOK: $($s.sections.Count) sections, $($total.spawn) spawns, $($total.wave) wave entries, $($total.gate) gates" -ForegroundColor Green

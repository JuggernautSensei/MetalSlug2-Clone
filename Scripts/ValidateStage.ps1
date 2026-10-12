# Stage JSON 검증: 파싱, ID 유일/참조, 에셋 이름 존재, 섹션별 개수 출력.
# 사용: powershell -File Scripts/ValidateStage.ps1 [-Stage Assets/Stage/Stage0.json]
param(
    [string]$Stage = (Join-Path $PSScriptRoot '..\Assets\Stage\Stage0.json'),
    [string]$AssetDir = (Join-Path $PSScriptRoot '..\Assets')
)

$ErrorActionPreference = 'Stop'
$errors = New-Object System.Collections.Generic.List[string]
$warnings = New-Object System.Collections.Generic.List[string]
function Err([string]$m) { $script:errors.Add($m) }

$s = Get-Content $Stage -Raw -Encoding UTF8 | ConvertFrom-Json
$spr = (Get-Content (Join-Path $AssetDir 'sprites.json') -Raw -Encoding UTF8 | ConvertFrom-Json).sprites.PSObject.Properties.Name
$ani = (Get-Content (Join-Path $AssetDir 'anim.json') -Raw -Encoding UTF8 | ConvertFrom-Json).animations.PSObject.Properties.Name
$snd = (Get-Content (Join-Path $AssetDir 'sounds.json') -Raw -Encoding UTF8 | ConvertFrom-Json).sounds.PSObject.Properties.Name
$man = Get-Content (Join-Path $AssetDir 'manifest.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$gnd = $man.ground.PSObject.Properties.Name

function Has($o, [string]$k) { if ($null -eq $o) { return $false }; return $null -ne $o.PSObject.Properties[$k] }
function ChkSpr([string]$n, [string]$w) { if ($n -and ($spr -notcontains $n)) { Err "missing sprite '$n' ($w)" } }
function ChkAni([string]$n, [string]$w) { if ($n -and ($ani -notcontains $n)) { Err "missing anim '$n' ($w)" } }

ChkSpr $s.world.background 'world.background'
if ($snd -notcontains $s.bgm.name) { Err "missing sound '$($s.bgm.name)'" }
foreach ($m in $s.ground.maps) { if ($gnd -notcontains $m) { Err "missing ground '$m'" } }
if ($s.ground.maps -notcontains $s.ground.initial) { Err 'ground.initial not in ground.maps' }
foreach ($p in $s.backdrop.modes.PSObject.Properties) { ChkSpr $p.Value.sprite "backdrop.$($p.Name)" }
if (-not (Has $s.backdrop.modes $s.backdrop.initial)) { Err 'backdrop.initial unknown' }
$rects = if (Has $s 'rects') { $s.rects.PSObject.Properties.Name } else { @() }

$ids = @{}; $spawnIds = @{}; $gateIds = @{}; $prevX = -1

# 스폰 위치 규칙 (원작처럼 적/포로가 화면 안에서 '뿅' 생기지 않게 한다, Docs/StageFormat.md §5 와 함께 볼 것)
# 뷰 반폭 320 (640x448 뷰), 스프라이트 반폭 근사 40. 카메라 기준 스폰은 뷰 밖(오른쪽 `dx >= 40`, 왼쪽 `dx <= -40`)이어야 한다.
$kViewHalfW = 320.0
$kSpriteHalfW = 40.0
function ChkCameraOutside($_cam, [string]$_where) {
    $dx = @(0.0, 0.0)
    if (Has $_cam 'dx') { if ($_cam.dx -is [array]) { $dx = @([double]$_cam.dx[0], [double]$_cam.dx[1]) } else { $dx = @([double]$_cam.dx, [double]$_cam.dx) } }
    switch ($_cam.ax) {
        'right' { if ($dx[0] -lt $kSpriteHalfW) { Err "camera spawn inside view: dx min $($dx[0]) < $kSpriteHalfW ($_where)" } }
        'left' { if ($dx[1] -gt -$kSpriteHalfW) { Err "camera spawn inside view: dx max $($dx[1]) > -$kSpriteHalfW ($_where)" } }
        default { Err "camera spawn centered on view ($_where)" }
    }
}
# 고정 좌표 적/포로: 섹션 발동 시점(플레이어 x = triggerX, 카메라 중심 = 플레이어 x 근사)의 뷰 오른쪽 밖이어야 한다.
# cameraMove 가 있으면 그 목표 중심을, lockCamera 가 있는 섹션은 건너뛴다(잠금 자리는 별도 확인).
function ChkFixedOutsideView($_sec) {
    if (@($_sec.actions | Where-Object { $_.type -eq 'lockCamera' }).Count -gt 0) { return }
    $camRight = [double]$_sec.triggerX + $kViewHalfW
    foreach ($cm in @($_sec.actions | Where-Object { $_.type -eq 'cameraMove' })) { $camRight = [math]::Max($camRight, [double]$cm.to[0] + $kViewHalfW) }
    foreach ($a in $_sec.actions) {
        if ($a.type -ne 'spawn' -or (Has $a 'camera') -or -not (Has $a 'x')) { continue }
        if ($a.kind -notin 'enemy', 'npc') { continue }
        $n = 1; if (Has $a 'repeat') { $n = $a.repeat.count }
        for ($i = 0; $i -lt $n; $i++) {
            $x = [double]$a.x; if (Has $a 'repeat') { $x += $i * [double]$a.repeat.stepX }
            if ($x - $kSpriteHalfW -lt $camRight) { $script:warnings.Add("fixed $($a.kind) '$($a.def)' x=$x inside view at trigger (cam right ~$camRight) ($($_sec.id))") }
        }
    }
}
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
                if (Has $a 'camera') { ChkCameraOutside $a.camera "$where/$($a.def)" }
                if (-not (Has $a 'camera') -and -not (Has $a 'x')) { Err "spawn without position ($where)" }
                if ((Has $a 'origin') -and $a.origin -eq 'legacyCenter' -and ($a.kind -in 'enemy', 'npc') -and -not (Has $a 'halfHeight')) { Err "legacyCenter creature without halfHeight ($where)" }
                if ((Has $a 'params') -and (Has $a.params 'arena') -and ($rects -notcontains $a.params.arena)) { Err "unknown rect '$($a.params.arena)' ($where)" }
            }
            'spawnWave' {
                foreach ($e in $a.entries) {
                    $n = 1; if (Has $e 'repeat') { $n = $e.repeat.count }
                    if (Has $e 'times') { $n = $n * $e.times }
                    $cnt.wave += $n
                    if (-not (Has $e 'delay')) { Err "wave entry without delay ($where)" }
                    if (-not (Has $e 'camera') -and -not (Has $e 'x')) { Err "wave entry without position ($where)" }
                    if (Has $e 'camera') { ChkCameraOutside $e.camera "$where/wave:$($e.def)" }
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
    ChkFixedOutsideView $sec
    '{0,-16} {1,6} {2,6} {3,4} {4,5} {5,5} {6,5} {7,6} {8,5} {9,5}' -f $sec.id, $c.spawn, $c.enemy, $c.pow, $c.prop, $c.decor, $c.item, $c.collider, $c.wave, $c.gate
    foreach ($k in @($total.Keys)) { $total[$k] += $c[$k] }
}
'{0,-16} {1,6} {2,6} {3,4} {4,5} {5,5} {6,5} {7,6} {8,5} {9,5}' -f 'TOTAL', $total.spawn, $total.enemy, $total.pow, $total.prop, $total.decor, $total.item, $total.collider, $total.wave, $total.gate

foreach ($g in $gateIds.Values) { foreach ($w in $g.waitFor) { if (-not $spawnIds.ContainsKey($w)) { Err "gate '$($g.id)' waitFor unknown spawn id '$w'" } } }
foreach ($t in $script:cmdTargets) { if (-not $spawnIds.ContainsKey($t)) { Err "command target unknown '$t'" } }

if ($warnings.Count -gt 0) {
    Write-Host "`nWARN: $($warnings.Count) fixed-position spawn(s) inside view at section trigger (pop-in risk, not a failure)" -ForegroundColor Yellow
    $warnings | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
}
if ($errors.Count -gt 0) {
    Write-Host "`nFAILED: $($errors.Count) error(s)" -ForegroundColor Red
    $errors | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
    exit 1
}
Write-Host "`nOK: $($s.sections.Count) sections, $($total.spawn) spawns, $($total.wave) wave entries, $($total.gate) gates" -ForegroundColor Green

# EnemyDefs JSON 스키마 (제안)

`Assets/Tuning/EnemyDefs.json` — 모든 적/소품/NPC/투사체/아이템을 같은 형식의 `def` 로 기술한다. 행동 수치는 데이터, 행동 *종류*(이동 방식, 사격 방식)만 코드 프리미티브. 사양 근거는 `EnemySpecs.md`. 핫리로드 대상.

## 1. 최상위
```json
{
  "version": 1,
  "globals": { "dpsRef": 10, "activationMarginPx": 150, "despawnBehindCameraPx": 200 },
  "defs": { "<id>": { ...Def }, ... }
}
```
- `id`: snake_case (`soldier_bomb`, `jeep`, `jeep_rocket`). 스테이지 JSON 의 `{def:"jeep", x, ground:true}` 가 참조.
- 좌표: **발 중앙 원점**, x 오른쪽 +, y 아래 +(박스/오프셋은 발 기준, 위쪽은 음수). 좌향일 때 x 오프셋은 자동 미러(`mirrorX` 기본 true).
- 시간 s, 길이 px, 속도 px/s, 가속도 px/s², 각도 도(°, 0=오른쪽, +=아래).

## 2. Def 필드
| 필드 | 형 | 설명 |
|---|---|---|
| `kind` | enum | `walker`(지상 보행), `vehicle`(지상 차량), `turret`(고정), `flyer`(비행, 원점=중심 `originMode:"center"`), `boss`, `prop`(파괴물), `npc`, `item`, `projectile`, `effect`, `debris` |
| `team` | enum | `enemy` `neutral` `item` |
| `originMode` | enum | `feet`(기본) / `center`(비행체) |
| `human` | bool | `true` 면 사람 대상: 칼(Melee)을 받는다. 생략/`false` 면 차량·보스·건물처럼 칼이 닿지 않고 총/폭탄만 받는다 (`human_base` 가 `true`를 상속) |
| `hp` | int | 새 HP. `ttk`(초) 병기 시 `hp = ttk × globals.dpsRef` 로 빌드 시 검증/경고 |
| `ttk` | float? | 권총 지속 dps 기준 목표 처치 시간(문서용·검증용) |
| `score` | int | 처치 점수 |
| `body` | Box | 이동/지형 충돌 박스 |
| `hurtboxes` | `[Hurtbox]` | 피격 박스(생략 시 `body` 1개). 다중(보스 3박스) 가능 |
| `solid` | Box? | 살아 있는 동안만 플레이어/적을 막는 벽 `{w,h,cx,cy}` (발 기준 좌표, Box 와 같은 형식). 사망(Dying 이후) 시 제거. 버스 차체 등 통과 불가 구조물용. 생략 시 벽 없음 |
| `activation` | obj | `{marginPx, latch:true, mode:"screen"\|"playerXAhead"\|"distance", range?}` (BTank = `playerXAhead:500`, MissleTank = `distance:600 latch:false`) |
| `physics` | obj | `{gravity, maxFall, speed, jumpVy, stepDownPx:8, ground:"pixel"\|"none"}` |
| `anim` | map | 논리 이름→클립. 접미 규칙 `{side}`=`Left`/`Right`: `"idle":"SD_Idle_{side}"`. 접미 없는 클립은 그대로 |
| `sideRule` | enum | `face`(접미=바라보는 방향, 기본) / `invertFire`(지프: 접미=주행 방향, 발사는 반대) |
| `ai` | FSM | 아래 §3 |
| `weapons` | map | 이름→Weapon (§4) |
| `death` | Death | §5 |
| `drops` | `[Drop]` | 드롭 테이블 |
| `gate` | obj? | `{lockCamera:true}` 처치 시까지 카메라 락 |
| `rig` | obj? | 다파트 보스: `parts:[{id, anim, offset:[x,y]}]` |

### Box / Hurtbox
```json
"body": { "w": 40, "h": 75, "cx": 0, "cy": -37.5 }
"hurtboxes": [ { "id":"bow", "w":170, "h":190, "cx":175, "cy":-92,
                 "accepts":["bullet","melee","blast"],
                 "shield": { "arc":"front", "blocks":["bullet"] } } ]
```
- `accepts`: HitResolver 종류 `bullet|melee|blast`. `shield`(방패병): 정면(`front`/`up`/`down`) 에서 온 `blocks` 종류를 막음; `activeWhen` 으로 사격 중 해제(상태 플래그 `blockOn`).
- 한 공격(`hitId`)이 같은 def 의 여러 hurtbox 에 중복 피해를 주지 않는다(보스 3박스/관통 샷건).

## 3. FSM (`ai`)
```json
"ai": {
  "initial": "FallIn",
  "params": { "cooldown": 1.0, "detect": 350, "closeStop": 200 },
  "states": {
    "<State>": {
      "anim": "move",                      // anim 논리 이름
      "facing": "toPlayer"|"keep",         // 진입 시 방향 갱신
      "move": { "vx": 100 },               // 전방 +, 상태 지속 중 적용
      "onEnter": [ {"do":"setVy","value":-450} ],
      "events":  [ {"frame":10,"do":"fire","weapon":"bomb"} ],   // 클립 프레임 이벤트(진입 후 f×frameDuration)
      "transitions": [
        { "when": {...}, "to": "State" | "choose": [ {"w":1,"to":"A"}, {"w":2,"to":"B"} ] }
      ]
    }
  }
}
```
**조건 `when`** (모두 AND, 위에서부터 첫 일치):
`animEnd`, `t:">=1.0"`(상태 경과), `absDx:">=350"|"<=200"`, `dy:">50"|"<-100"`(플레이어−자신, +=플레이어가 아래), `dist:"<=400"`(유클리드), `grounded`, `onOneWay`, `wallAhead`, `edgeAhead`, `bullets:"==0"`, `hpFrac:"<=0.5"`, `playerOnFireSide`(지프), `eventFlag:"x"`.
**액션 `do`**: `setVy`, `setVx`, `fire(weapon)`, `spawn(def,offset)`, `setCounter(name,value)`, `dec(name)`, `playFx(anim,offset)`, `dropThrough`, `flipFacing`, `swapGround(patch)`, `shake`.
- `choose.w` 가중치(레거시 `rand()%k` 균등 = 가중치 1).
- 상태 전이는 **같은 틱** 반영(이중버퍼 없음). `animEnd` 는 비루프 클립에만.
- 반복 규칙(예: Jeep 2발 후 2 s 휴지)은 `counter` + `when:{bullets:"==2"}` 로 표현.

## 4. Weapon / Projectile
```json
"weapons": {
  "bomb": {
    "projectile": "soldier_bomb_proj",
    "muzzle": [0, -37],                    // 발 기준, 좌향 시 x 미러
    "dir": "facing" | {"deg": 22.5} | "toPlayer" | "toPlayerClamped",
    "angleSet": [0, 22.5, 45, 67.5, 90],   // 선택: toPlayer 를 구간 양자화
    "tiltTowardPlayer": {"threshold": 50, "dy": 0.1},
    "muzzleFx": "Bazuka_Shot_Effect",
    "count": 1
  }
}
```
Projectile def (`kind:"projectile"`):
| 필드 | 설명 |
|---|---|
| `box` | 판정 박스 (`w,h,cx,cy` 투사체 중심 기준) |
| `speed` | 일정 속도(방향 벡터에 곱) |
| `speedRamp` | `{from:0, accel:400, max:400}` (SoldierMissle) |
| `startDelay` | 점화 대기(제자리) s, 동안 `anim.begin` (Jeep rocket 0.4) |
| `v0` | `[vx, vy]` 초기 속도 (포물선: `[±200,-500]`) |
| `gravity`, `maxFall` | 중력(없으면 0) |
| `lifetime`(기본 5), `killOffscreenPx`(100) | |
| `onSolid` | `explode`(기본)/`roll`/`stick` ; `ignoreTopOfOneWay:true` |
| `roll` | `{vx:100, life:5}` (Sarubia 수류탄: 착지→`collision` 클립→굴러감) |
| `arcAim` | `{initVy:-200, g:500, gMax:500, speedClamp:[40,450]}` 포탑/버스 포탄이 플레이어 위치로 속도 계산 |
| `anim` | `{begin, fly, hit}`; 방향별 인덱스 `angleClips:["Alian_Bullet_1",…]` |
| `trail` | `{fx:"Rocket_Smoke", every:0.1, offset:[15,-15]}` |
| `explosion` | `{fx:"Explosion_Mid", offset:[0,-35]}` |
| `lethal` | true: 플레이어 즉사. 무적/사망 시 통과 |

## 5. Death / Drop
```json
"death": {
  "byKind": { "blast": "Die_Bomb", "melee": "Die_Slash" },          // 선택
  "ground": { "chain": ["Die_Stand_1|Die_Stand_2", "Die_Fall", "Die_Land"],
              "fallAt": {"Die_Stand_1":10, "Die_Stand_2":8} },
  "air":    { "chain": ["Die_Jump", "Die_Land"] },
  "corpse": { "blinkAt": 2.0, "blinkFor": 1.0, "removeAt": 3.0 },
  "fx": [ {"anim":"Explosion_Huge","offset":[0,-20],"delay":0}, {"anim":"Explosion_Huge","offset":[0,-20],"delay":1.7} ],
  "debris": [ {"set":"metal","count":5,"delay":1.7} ],
  "hold": "Jeep_Destroy",
  "swapGround": "Stage0_Pixel_1",
  "disableHurtboxes": true
}
"drops": [ { "item":"item_random_score", "count":3, "bounce":true, "scatterX":25, "weight":1 } ]
```
디브리 세트: `metal(12)`, `wood(6)`, `rock(3)`, 단일 `Bazuka_Debics` — 물리 `g=1000, vy0∈[−800,−301], vx∈[−250,249], life 5`(def `debris_*`).

## 6. 스테이지 참조 예
`{ "def":"jeep", "x":6000, "ground":true, "activate":"screen", "gate":false }` · `{ "def":"soldier_bomb", "x":900, "ground":true }` · `{ "def":"para_bzk", "spawn":"cameraTopRight", "offset":[-80,30], "randX":120, "afterSec":4 }`.

---

## 7. 예제 A — Soldier (BOMB_THROWER)
```json
"soldier_bomb": {
  "kind": "walker", "team": "enemy", "hp": 1, "score": 100,
  "body": { "w": 40, "h": 75, "cx": 0, "cy": -37.5 },
  "hurtboxes": [ { "id": "body", "w": 40, "h": 75, "cx": 0, "cy": -37.5, "accepts": ["bullet","melee","blast"] } ],
  "probes": { "wall": { "w": 40, "h": 50, "cx": 25, "cy": -37.5 } },
  "activation": { "mode": "screen", "marginPx": 150, "latch": true },
  "physics": { "gravity": 1000, "maxFall": 3000, "speed": 100, "jumpVy": -450, "dropThroughSpeed": 100, "ground": "pixel" },
  "anim": {
    "idle": "SD_Idle_{side}", "move": "SD_Move_{side}", "moveDown": "SD_MoveDown_{side}",
    "jump": "SD_Jump_{side}", "moveJump": "SD_MoveJump_{side}", "land": "SD_Land_{side}",
    "throw": "SD_Throw_Bomb_{side}"
  },
  "weapons": { "bomb": { "projectile": "soldier_bomb_proj", "muzzle": [0, -37], "dir": "facing" } },
  "ai": {
    "initial": "Fall",
    "params": { "cooldown": 1.0, "detect": 350, "closeStop": 200 },
    "states": {
      "Fall":  { "anim": "jump", "transitions": [ { "when": {"grounded":true}, "to": "Land" } ] },
      "Land":  { "anim": "land", "transitions": [ { "when": {"animEnd":true}, "to": "Idle" } ] },
      "Idle":  { "anim": "idle", "facing": "toPlayer",
        "transitions": [
          { "when": {"t":">=1.0","absDx":">=350"}, "choose": [ {"w":1,"to":"Move"}, {"w":1,"to":"@Reposition(Move)"} ] },
          { "when": {"t":">=1.0"}, "choose": [ {"w":1,"to":"Throw"}, {"w":1,"to":"@Reposition(Throw)"}, {"w":1,"to":"Move"} ] } ] },
      "Move":  { "anim": "move", "facing": "toPlayer", "move": {"vx": 100},
        "transitions": [
          { "when": {"t":">=1.0"}, "choose": [ {"w":1,"to":"Idle"}, {"w":1,"to":"@Reposition(Throw)"}, {"w":1,"to":"Throw"} ] },
          { "when": {"absDx":"<=200"}, "choose": [ {"w":1,"to":"Idle"}, {"w":1,"to":"Throw"} ] },
          { "when": {"wallAhead":true}, "choose": [ {"w":1,"to":"HJump"}, {"w":1,"to":"Throw"} ] } ] },
      "Throw": { "anim": "throw", "events": [ {"frame": 10, "do": "fire", "weapon": "bomb"} ],
        "transitions": [ { "when": {"animEnd":true}, "to": "Idle" } ] },
      "MoveDown": { "anim": "moveDown", "onEnter": [ {"do":"dropThrough"} ], "transitions": [ {"when":{"animEnd":true},"to":"Fall"} ] },
      "HJump": { "anim": "moveJump", "onEnter": [ {"do":"setVy","value":-450} ], "move": {"vx": 100},
        "transitions": [ { "when": {"grounded":true}, "to": "Land" } ] }
    },
    "macros": {
      "Reposition(x)": [ { "when": {"onOneWay":true,"dy":">50"}, "to": "MoveDown" },
                         { "when": {"dy":"<-100"},               "to": "HJump" },
                         { "otherwise": true,                    "to": "$x" } ]
    }
  },
  "death": {
    "byKind": { "blast": "SD_Die_Bomb_{side}", "melee": "SD_Die_Slash_{side}" },
    "ground": { "chain": ["SD_Die_Stand_1_{side}|SD_Die_Stand_2_{side}", "SD_Die_Fall_{side}", "SD_Die_Land_{side}"],
                "fallAt": {"SD_Die_Stand_1":10, "SD_Die_Stand_2":8} },
    "air":    { "chain": ["SD_Die_Jump_{side}", "SD_Die_Land_{side}"] },
    "corpse": { "blinkAt": 2.0, "blinkFor": 1.0, "removeAt": 3.0 },
    "disableHurtboxes": true
  }
},
"soldier_bomb_proj": {
  "kind": "projectile", "lethal": true,
  "box": { "w": 20, "h": 20, "cx": 0, "cy": 0 },
  "v0": [200, -500], "gravity": 750, "maxFall": 3000,
  "anim": { "fly": "Soldier_Bomb" },
  "onSolid": "explode", "ignoreTopOfOneWay": true,
  "explosion": { "fx": "Enemy_Bomb_Exp", "offset": [0, -8] }
}
```
- `Throw` 의 `frame:10` = 0.80 s(클립 14f×0.08). `v0.x` 는 좌향이면 자동 −200.
- 같은 몸 + 다른 `ai` 로 `soldier_sword`(SWORD_MAN: `detect 350, jumpAttack 150`, 상태 `Slash`·`JumpSlashJump/Ready/Slash/Land`, 히트박스 무기 `slash`) / `soldier_missle`(`detect 400`, `Throw_Missle` f10 + `soldier_missle_proj` `speedRamp{0,400,400}`) 을 구성. 공통 부분은 `"extends":"soldier_base"` 로 상속.
- 히트박스 무기(칼) 정의 예: `"slash": {"hitbox": {"w":60,"h":50,"cx":35,"cy":-37,"life":0.4,"followMove":false}}`.

## 8. 예제 B — Jeep
```json
"jeep": {
  "kind": "vehicle", "team": "enemy", "hp": 80, "ttk": 8, "score": 500,
  "body": { "w": 75, "h": 60, "cx": 0, "cy": -30 },
  "activation": { "mode": "screen", "marginPx": 150, "latch": true },
  "physics": { "gravity": 1400, "maxFall": 3000, "speed": 200, "ground": "pixel", "stepDownPx": 8 },
  "sideRule": "invertFire",
  "anim": {
    "idle": "Jeep_Idle_{side}", "move": "Jeep_Move_{side}", "turn": "Jeep_Turn_{side}",
    "aim": "Jeep_Aimming_{side}", "reaim": "Jeep_ReAimming_{side}", "ready": "Jeep_Ready_{side}",
    "shotReady": "Jeep_Shot_Ready_{side}", "shot": "Jeep_Shot_{side}", "destroy": "Jeep_Destroy_{side}"
  },
  "weapons": { "rocket": { "projectile": "jeep_rocket", "muzzle": [51, -88], "muzzleStepX": 20, "stepByCounterBit": "bullets&1",
                           "dir": "fireSide", "tiltTowardPlayer": {"threshold": 50, "dy": 0.1} } },
  "ai": {
    "initial": "Wait",
    "params": { "edgeMarginX": 75, "arriveTol": 10 },
    "counters": { "bullets": 0 },
    "states": {
      "Wait":   { "anim": "idle", "transitions": [ { "when": {"activated":true}, "to": "Enter" } ] },
      "Enter":  { "onEnter": [ {"do":"setMoveTarget","target":"screenLeft+75"} ],
                  "transitions": [ { "when": {"needFlip":true}, "to": "TurnToMove" }, { "otherwise": true, "to": "Move" } ] },
      "TurnToMove": { "anim": "turn", "transitions": [ { "when": {"animEnd":true}, "do": ["flipFacing"], "to": "Move" } ] },
      "Move":   { "anim": "move", "move": {"vx": 200, "along": "drive"},
                  "transitions": [ { "when": {"arrived":true}, "to": "Turn" } ] },
      "Turn":   { "anim": "turn", "transitions": [ { "when": {"animEnd":true}, "do": ["flipFacing"], "to": "Idle" } ] },
      "Idle":   { "anim": "idle", "transitions": [
                  { "when": {"t":">=1.0","playerOnFireSide":true}, "to": "Aim" },
                  { "when": {"t":">=1.0"}, "to": "Turn" } ] },
      "Aim":    { "anim": "aim", "transitions": [ { "when": {"animEnd":true}, "do": [ {"do":"setCounter","name":"bullets","value":4} ], "to": "ShotHold" } ] },
      "ShotHold": { "anim": "ready", "transitions": [
                  { "when": {"bullets":"==0","t":">=2.0"}, "to": "ReAim" },
                  { "when": {"bullets":"==2","t":">=2.0"}, "to": "ShotReady" },
                  { "when": {"bullets":"!=0,!=2"},          "to": "ShotReady" } ] },
      "ShotReady": { "anim": "shotReady", "transitions": [ { "when": {"animEnd":true}, "to": "Shot" } ] },
      "Shot":   { "anim": "shot", "events": [ {"frame": 1, "do": "fire", "weapon": "rocket"} ],
                  "transitions": [ { "when": {"animEnd":true}, "do": [ {"do":"dec","name":"bullets"} ], "to": "ShotHold" } ] },
      "ReAim":  { "anim": "reaim", "transitions": [ { "when": {"animEnd":true}, "to": "TurnAfterShot" } ] },
      "TurnAfterShot": { "anim": "turn", "transitions": [ { "when": {"animEnd":true}, "do": ["flipFacing"], "to": "IdleMove" } ] },
      "IdleMove": { "anim": "idle", "transitions": [ { "when": {"t":">=1.0"}, "do": [ {"do":"setMoveTarget","target":"awayFromPlayerEdge+75"} ], "to": "Enter" } ] }
    }
  },
  "death": {
    "hold": "destroy",
    "fx": [ {"anim":"Explosion_Huge","offset":[0,-20],"delay":0,"layer":"back"},
            {"anim":"Explosion_Huge","offset":[0,-20],"delay":1.7} ],
    "debris": [ {"set":"metal","count":5,"delay":1.7} ],
    "removeAt": 1.7, "disableHurtboxes": true
  }
},
"jeep_rocket": {
  "kind": "projectile", "lethal": true,
  "box": { "w": 50, "h": 20, "cx": 0, "cy": 0 },
  "speed": 300, "startDelay": 0.4,
  "anim": { "begin": "Jeep_Proj_Begin_{side}", "fly": "Jeep_Proj_{side}" },
  "trail": { "fx": "Rocket_Smoke", "every": 0.1, "offset": [15, -15] },
  "onSolid": "explode",
  "explosion": { "fx": "Explosion_Mid", "offset": [0, -35] }
}
```
- `sideRule:"invertFire"`: 클립 접미는 주행 방향 기준, 발사 방향 = 반대. `playerOnFireSide` 조건이 이 규칙을 사용.
- 사격 패턴 `bullets: 4→3(즉시) → 2(2 s 휴지) → 1(즉시) → 0(2 s 휴지) → ReAim` 이 `ShotHold` 의 3 전이로 그대로 재현된다(`bullets` 카운터 `dec` 는 `Shot` 종료 시).
- FixedJeep 은 `"extends":"jeep"` + `ai` 교체(`Idle absDx<=450`, `Detect 2 s`, 이동 상태 제거). 헬기/전차 등은 `kind:"flyer"`/`"turret"` 로 동일 스키마.

## 9. 검증 규칙(빌드 시 `Baker`/로더가 확인)
1. 모든 `anim` 값은 `anim.json`/`sprites.json` 에 존재(접미 `{side}` 는 `_Left`/`_Right` 둘 다 확인, `sideRule` 에 따라).
2. `events.frame` < 클립 프레임 수. 루프 클립에 `animEnd` 전이 금지.
3. `choose` 가중치 합 > 0, 모든 `to` 가 존재하는 상태/매크로.
4. `hp`/`ttk`: `|hp − ttk×dpsRef| > 5%` 면 경고.
5. 발 기준 박스: 스폰 시 `body` 하단이 지면에 닿아야 한다(`originMode:"feet"`).

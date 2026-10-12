# 최종보스 전함 (BigShiee) 개선 계획 B1~B5

원작: V1 = `videoplayback (1).mp4` (496x360, 30 fps), V2 = `(2).mp4` (480x360, 29.5 fps). 둘 다 아케이드 원작. `Release 2026.10.10 ...mp4` 는 리메이크 녹화.
단위: 영상 px (vpx) -> 게임 px = x 1.225 (가로) / x 1.244 (세로). 틱 = 1/60 s. 증거 이미지: `Docs/Wayfinder/ref/` (아래 파일명).
선행: `Docs/Wayfinder/Boss_sync.md` (다른 에이전트, 뚜껑/주포 동기 + 고폭탄 muzzle 이중 적용 수정 완료) 위에 쌓는다. 채점: `Docs/Wayfinder/Scoring.md`.
코드 줄 번호는 2026-10-12 02:23 작업 트리 기준.

## 기준 점수 (BASELINE)
| 장면 | 점수 | 목표 |
|---|---|---|
| boss_entrance (B4) | 0.749 | >= 0.75 |
| camera_slide (B4) | 0.600 | >= 0.75 |
| cannon_deploy (B1) | 0.457 | >= 0.75 |
| cannon_fire_down (B1) | 0.506 | >= 0.75 |
| cannon_casing (B3) | 0.285 | >= 0.75 |
| shack_destroy (B2) | 0.000 (채점 불가) | >= 0.75 |
| boss_explosion (B1) | 0.000 (채점 불가) | >= 0.75 |

---

## B1. 주포 사격 / 폭발 모션과 위치

### 원작 (V1, `B1_cannon_shot_down_V1_330-333.jpg`, `B1_cannon_phase_V1_315-321.jpg`, `B1_death_V1_367-375.jpg`)
- 주포 사격 1 회 (V1 331.3, 머리 정면 = yaw 4, 플레이어가 바로 아래):
  - 331.3: 포구 바로 아래 어두운 회색 연기 원 + 주황 탄두. 331.4: 포구에 흰 섬광 원 (짧게 1~2 프레임).
  - 331.4~332.4: 고폭탄 (은색 몸통 + 위쪽 불꽃) 이 포구에서 수직 하강. 처음엔 느리고 점점 빨라진다: 포구 y 116 vpx -> 지면 y 328 vpx 를 약 1.0~1.1 s (평균 약 260 게임 px/s, 끝 속도 약 330 게임 px/s).
  - 꼬리: 포구부터 탄까지 끊김 없는 회색 연기 기둥 (퍼프가 촘촘히 쌓임). 착탄 뒤 0.3 s 동안 위에서부터 사라진다.
  - 332.5: 착탄 = 큰 불꽃 폭발 (지면에서 솟는 화염 + 주황 폭발, 약 0.4 s).
- 사격 간격: Boss_sync 실측 3.04 s (V2 10 회 평균), V1 다음 발도 +3.04 s 와 맞음 (채점 timing 0.948).
- 검은 원 (Boss_Disc_Black) 은 사망 때만이 아니라 전투 중에도 여러 번 보인다 (V1 296.0, 309.6, 316.2, 319.8, 367.5). 위치가 매번 달라 포탑 파괴/착탄 계열로 보이나 확정 못 함 (T0 로그 + 프레임 대조 필요).
- 사망 (V1): 367.5 검은 원 -> 368.0 주포 꼭대기 화염 기둥 -> 369.0~369.5 흰 섬광 원 4~5 개 -> 370~372 선체 전체 연쇄 폭발 -> 372.0 선체가 앞으로 기울며 붕괴 -> 373.0 잔해 (붉게 달아오른 구멍) -> 373.5 이후 회색 연기 퍼프. **사망 -> 잔해 = 약 5.0 s** (리메이크 `dyingWreckSec` 6.3). 결산 박스 6:15 = 잔해 뒤 약 2 s.
- 주포 위치: cannon_deploy 추적에서 원작 주포 피벗 (304, 246) vs 리메이크 (360, 252) 게임 px (V2 300.6 정렬). 리메이크 주포가 화면에서 56 px 오른쪽 = 선체가 화면에서 더 오른쪽에 있다 (entranceKeys 끝 392 / driveTargetX 385 문제).

### 리메이크 현재
- 고폭탄 `bigshiee_missile` (Assets/Defs/BigShiee.json:21-33): speed 200 일정, trailSec 0.15 (퍼프 간격 넓음), 착탄 `Explosion_Large`. 포구 섬광/연기 원 없음 (주포 Shot 클립 2 번째 프레임 섬광만).
- 사망 연출 BigShiee.cpp:811-867 (OnDeath_/TickDying_): 6.3 s 뒤 잔해, 8.4 s 종료.
- 주포 사격 행은 플레이어 각도로 고른다 (TargetYaw_ 507, 정상). 원작 V1 331 은 정면 행 (플레이어가 아래), 리메이크 bosslab_bot 은 플레이어가 왼쪽이라 Shot1: 행 자체는 맞음.

### 제안
1. (HAIKU, 데이터) BigShiee.json `bigshiee_missile`: `speedRamp: { from: 60, accel: 420, max: 330 }` (EnemyDefLoader 가 이미 지원), `trailSec 0.05`, `explosion.fx "PlayerBombExplosion"` (불꽃) + 기존 blast 유지. `dyingWreckSec 5.0`, `dyingDoneSec 7.0`.
2. (SONNET) 사격 순간 포구 FX: Shot 시작 틱에 `Boss_Smoke_Small` 큰 크기 1 개 + 흰 섬광 (기존 FX 중 흰 원: `Boss_Blast_ABCD1` 0~1 프레임 또는 새 `Boss_Flash`) 을 포구에 RequestFxAt. 위치 = FireMissile_ 의 muzzle (BigShiee.cpp:554).
3. (SONNET) 화면상 선체 위치: entranceKeys 마지막 값/`driveTargetX` 를 원작 선수 화면 x 에 맞춘다 (B4 와 같이 처리, 아래 B4-3).
- 위험: speedRamp 가 rotate (방향 회전) 와 같이 동작하는지 확인 (가속은 속도 방향). 착탄 FX 를 바꾸면 BigShieeSync 테스트의 생성 위치 검사에는 영향 없음.
- 수락: cannon_fire_down >= 0.75 (T0 후 missile_hit 이벤트 추가), boss_explosion >= 0.75 (T0 log 앵커), 유닛: `BigShieeMissile.AcceleratesFromMuzzleToGround` (첫 10 틱 평균 속도 < 끝 10 틱, 착탄까지 60~70 틱 @ 낙하 260 px).

## B2. 판자집 (나무 망루) 파괴

### 원작 (`B2_tower_hit_by_player_cannon_V1_362.2-363.4.jpg`, `B2_tower_V2_264.6-266.2.jpg`, `B2_tower1_V1_308.4-312.jpg`)
- 부서지는 것: 보스 아레나 바닥의 **나무 비계 망루** (Props `tower`, 스테이지 Stage0 x 14904 와 같은 물체). 보스전 중 2 개가 화면에 들어온다 (V1 292~310 첫째, 362~364 둘째).
- 원인: **플레이어 SV-001 캐논 포탄** (전함 주포 아님). V1 362.67~363.0: 주황 캐논 탄이 왼쪽에서 수평으로 날아와 망루에 맞는 순간 파괴. V2 265.55~266.0 도 같은 패턴 (작은 주황 탄 -> 붕괴). V1 첫째 망루 (309.2~309.4) 는 플레이어 사격 (캐논/발칸) 으로 위쪽에 폭발 2 회 뒤 붕괴.
  전함 주포/포탑 탄이 망루를 부수는 장면은 두 영상 모두 없다. 전함 주포는 망루 파괴 (V1 309, V2 266) 뒤에야 전개된다 (V1 313, V2 300).
  V2 에서는 그 순간 선수가 망루를 지나 앞으로 나가고 있어 "선체에 깔린다" 처럼 보이지만, 탄이 맞는 프레임이 있고 V1 에서는 선체가 망루를 13 s 동안 겹쳐도 멀쩡했다 -> 선체 접촉은 원인이 아님.
- 모습: 맞는 순간 흰 섬광 + 검은 원 (0.1 s), 망루 밑동에서 큰 화염 기둥 (PlayerBombExplosion 류, 위로 약 200 게임 px, 0.6 s), 밑동 폭발 무더기, 판자 4~6 개가 회전하며 위/양옆으로 튐, 위로 검은 연기 버섯 (약 1 s). 아랫부분 밑동은 남는다 (`Stage0_Tower_Destroy` 와 같은 모양).
- 한 방: V1 둘째 망루는 캐논 1 발로 파괴.

### 리메이크 현재
- `Assets/Defs/Props/Props.json:389-399` tower: hp 24, `onDestroy` 는 나무 파편 5 개뿐 (폭발/화염 없음).
- 보스 공격은 소품을 못 친다 (`CanTeamHit(Enemy, Neutral) == false`, Source/World/Combat.cpp:154-162, C15.md 3 항목).
- bosslab (`Assets/Stage/BossLab.json`, GenBossLab.py 생성) 에는 보스 아레나 망루가 없다 -> shack_destroy 채점 불가. `--state stage0 --warp-boss` 에서는 약 40 s 뒤 망루가 화면에 들어온다 (`B4_stage0_warpboss_remake.jpg` 마지막 칸).

### 제안
1. (HAIKU, 데이터) Props.json tower `onDestroy.explosions`: `PlayerBombExplosion` 1 (밑동, offset [0, 100]), `Explosion_Large` 2 (spread 60), `Explosion_Mid` 2, debris wood 6. hp 를 슬러그 캐논 1 발에 죽게 (Tuning `slug.cannon` 피해/폭발 피해 확인 후 hp <= 그 값).
2. (HAIKU, 데이터 + 사용자 승인) BossLab.json 재생성 시 보스 아레나 망루 (x 14904) 포함 (Stage 파일은 사용자 영역: 승인 필요). bosslab_bot 에 "앞으로 캐논/총 쏘기" 구간 추가는 SONNET (봇 입력, StageState).
3. (선택, 사용자 결정) 사용자 요청 문장 ("대포를 쏘면 판자집이 부서진다") 이 전함 주포를 뜻했다면: 고폭탄 blast 가 Neutral 소품도 치도록 `blast.hitProps: true` 같은 플래그 (SONNET, CombatSystem 경로 1 곳). 원작 근거는 없음을 보고서에 명시.
- 위험: 소품 hp 를 낮추면 일반 구간 (개활지) 의 같은 tower 3 개도 쉽게 부서진다 (원작도 그렇다고 보이나 미확인).
- 수락: shack_destroy >= 0.75 (망루가 있는 런 + T0 `prop.destroyed`), 유닛: `PropTower.OneSlugCannonHitDestroysWithFireColumn` (FX 요청 목록에 PlayerBombExplosion 포함).

## B3. 주포 탄피 ("탄환 찌꺼기")

### 원작 (`B3_casing_V1_316.4-318.jpg`)
- 주포가 쏠 때 머리 오른쪽 (약실 쪽) 에서 은회색 속 빈 원통 탄피가 튀어나온다. V1 316.5 첫 프레임: 포구 기준 오른쪽 약 +60 게임 px, 위 약 -20.
- 궤적 (V1 316.5 -> 317.4, vpx): (308,46) (325,85) (314,108) (325,131) (325,148) (331,171) (336,205) (331,228) (342,262) (342,296). = 수평 +30~40 게임 px/s 로 거의 곧게 떨어지며 선체 앞면을 타고 내려옴, 낙하 약 310 게임 px / 0.9 s (평균 340 px/s, 처음부터 빠름: 초속 약 250 아래 + 중력).
- 회전: 원통이 굴러가며 옆면/단면이 번갈아 보임 (약 10~12 fps).
- 수명: 지면 (선체 바닥 높이) 에 닿으면 0.1~0.3 s 뒤 사라짐. 가끔 지면에 0.3 s 이상 누워 있음 (V1 330.4~330.7).
- 매 발마다 나오는지는 미확인: V1 331.3 발은 머리 오른쪽에 탄피가 안 보인다 (정면 자세에서 머리에 가렸을 수 있음).

### 리메이크 현재
- 탄피 없음 (anim.json 에 탄피 클립 없음). 원본 시트 `Trash/AssetsSource/40204.png` 에 **"CARTRIDGE" 6 프레임** (회전하는 원통, 황동/은색) 이 있다: 시트 px 대략 x 2425~2630, y 3065~3090 (라벨 "CARTRIDGE" 아래 한 줄). 같은 시트에 "SHELL" 3x3 (탄두 각도), "DEPLOYMENT SPARK" 5 프레임 (주포 전개 불꽃) 도 잘리지 않은 채 남아 있다.

### 제안
1. (HAIKU, 아트 자르기 + 사용자 승인) 시트에서 CARTRIDGE 6 프레임을 `Boss_Casing` 클립 (12 fps, loop) 으로 자른다. anim.json/아틀라스 변경이라 사용자 승인/에디터 작업 필요. 같은 김에 `Boss_DeploySpark` (5 프레임) 도.
2. (SONNET) BigShiee: 고폭탄 발사 틱에 탄피 1 개 생성 (포탄 아님, 판정 없음). Bus 파편 (`Bus.cpp` SubmitDebris_ 253-277) 처럼 해석적으로 그린다: 시작 = 주포 머리 + `casingOffset` [60,-20], 속도 `casingVel` [36, 240], 중력 900, 지면 = 선체 원점 y + `casingGroundDy` 0, 지면 뒤 `casingLingerSec` 0.2 뒤 사라짐. 슬롯 1 개 (`kBigShieeExtCasingTick`) 면 충분 (발사 간격 3 s > 수명 1.1 s). 새 파라미터 5 개 (json).
- 위험: ext 슬롯 여유 확인 (BigShiee.h). 다른 에이전트의 Shot 타이밍 (missileFireTick 4) 과 같은 틱에 묶을 것.
- 수락: cannon_casing >= 0.75 (탄피 템플릿 `Boss_Casing:0` 이 생기면 바로 채점됨), 유닛: `BigShieeCasing.SpawnsOnEachShotAndVanishesWithin70Ticks`.

## B4. 카메라 오른쪽 자동 스크롤 + 아레나 끝 정지 + 왕복 (와리가리)

### 원작 측정 (`B4_scroll_V1_vs_remake.png`, 수치: Captures/VideoScore/baseline/camera_slide.json)
- 속도: 망루 (월드 고정) 추적 V1 296->303 s 에 103 vpx / 7 s = **14.7 vpx/s = 18 게임 px/s**. 지면 밴드 실측 이동평균 14~20 게임 px/s. (주의: 지붕 무늬 주기가 약 13 vpx 라 1 s 간격 비교는 앨리어싱, 위상상관은 절반으로 과소추정 -> 0.5 s 간격 + 탐색 창 제한으로 측정.)
- V1 멈춤 (선체 등장 289.2 기준 초): 약 3, 4.5, 15, 18~19, **25~34 (주포 전개, 약 9.5 s)**, 55, 78 (사망). 총 이동 약 1073 게임 px / 74 s.
- V2: 260~271 스크롤, **272~281 정지**, 282~286 스크롤, **287~331 정지 (44 s)**, 332~342 스크롤, 이후 사망. 총 이동 약 490 게임 px. 정지 구간에 선체도 화면에서 움직이지 않음.
  -> "아레나 끝 고정 위치" 증거는 없다 (V1 은 1073 px 를 가고도 계속 감, V2 는 490 px 에서 44 s 멈춤). V2 정지 중 플레이어 슬러그가 화면 왼쪽 끝 (x < 60 게임 px) 에 붙어 있었다 -> "플레이어가 왼쪽 끝에 몰리면 스크롤 정지" 가설 (미검증).
- 선체 화면 운동 (V1, 선체 밴드 추적): 평소 화면에서 왼쪽으로 -13~-16 vpx/s (= 월드 정지, 카메라만 감). 주기적으로 앞으로 돌진: +30~+50 vpx/s (= 화면 +37~+61, 월드 약 +55~+80 게임 px/s) 를 2~4 s: 289~296 (입장), 320~322, 336~339, 345~347, 353~357, 363~364 -> **주기 약 8.5~9 s**. 주포 전개 뒤 한 번 **뒤로 후진** 327~328 (화면 -54 게임 px/s, 월드 약 -36 px/s) = 실제 왕복.

### 리메이크 현재
- 자동 스크롤 BigShiee.cpp:640-643 (`RequestCameraScroll(scrollSpeed * kStepSec)`, 20 px/s) -> StageRunner::AddCameraScroll (Source/Game/Stage/StageRunner.cpp:242-257) 가 락 사각형과 카메라를 같이 민다. 멈춤 = `scrollPauses`, `deployPause*`, `latePause*` (IsScrollPaused_ 180).
- **실측: 동작한다.** bosslab_bot 평균 15.3 px/s (원작 14.6), 총 1123 px (원작 1073), `--state stage0 --warp-boss` 에서도 20 px/s 로 계속 간다 (`B4_stage0_warpboss_remake.jpg`). "약하다/안 된다" 는 지금 빌드에서는 재현되지 않음 (사용자가 본 빌드가 이전이었을 가능성, 또는 선체가 카메라와 같이 움직여 스크롤이 안 느껴짐).
- 끝이 없다: 스크롤 예산/정지 없음. 선체 주행 (BigShiee.cpp:663-690) 은 앞으로만 (`driveTriggerX` 200 -> `driveTargetX` 385, 65 px/s). 후진 없음.
- 스크롤 멈춤 시각이 원작과 다름: 리메이크 멈춤 14~22 s (자동 피해로 주포 전개가 빠름) vs 원작 25~34 s. 앵커가 단계 사건이면 맞아진다.

### 제안 (SONNET, M)
1. `scrollBudgetPx` (기본 0 = 무한, json 1100 = V1 총 이동): 보스 생성 뒤 누적 스크롤이 예산에 닿으면 이후 스크롤 0 (락 사각형 고정 = 아레나 끝). 누적값 슬롯 1 개.
2. 예산 소진 뒤 왕복: `shuttle: { periodSec: 9, forwardPx: 140, speed: 60, accel: 160 }` - 화면 x `driveTargetX` 와 `driveTargetX - forwardPx` 사이를 주기적으로 왕복 (Drive 상태를 앞/뒤 둘 다로 일반화, 기존 가감속 식 재사용). 주포 전개 중 (deployPause) 에는 안 함, 공격 (포탑/주포) 은 계속.
3. 선수 화면 위치 보정: entranceKeys 끝/`driveTargetX` 를 원작 선수 위치에 맞춤 (cannon_deploy 추적 오차 56 px, bus 아님). boss_entrance 의 선수 오차 중앙 16.6 px 는 이미 작음 -> 입장 곡선은 유지, 정착점만 -40~-56 px.
4. (선택, 사용자 결정) `playerEdgePausePx` 60: 플레이어가 화면 왼쪽 끝에서 이 거리 안이면 스크롤 일시 정지 (V2 가설).
- 위험: 락 사각형 이동을 멈추면 기존 StageIntegration 의 "락 사각형은 오른쪽으로만" 기대와 충돌 없음 (정지일 뿐). 엔드리스 지면 (BossLab world endless 14094~) 은 예산 이후 필요 없어짐. 낙하산 인질 x 보정 (TickChute_ 436) 은 스크롤 0 이면 자동으로 맞음.
- 수락: camera_slide >= 0.75, boss_entrance >= 0.75, 유닛 `BigShieeScroll.StopsAfterBudgetThenShuttles` (예산 도달 뒤 300 틱 동안 카메라 x 변화 0, 선체 화면 x 가 두 끝 사이를 9 +-1 s 주기로 오감).

## B5. 포탑 7 문 조준 (플레이어 쪽으로 머리를 돌려 쏨)

### 클립 의미 (`B5_turretA_clips.png`, `python Scripts/VideoScore/sprites.py Boss_Turret_A_Idle1 ...`)
- `Idle1` = 포신 왼쪽 위 (약 30~40 도) 자세 (22 프레임 중 0~9 낮게, 10~14 올라감 = 미세 앙각 흔들림), `Idle2` = 수직, `Idle3` = 오른쪽 위.
- `Turn` 9 프레임: 0 = 왼쪽 (Idle1), 4 = 수직 (Idle2), 8 = 오른쪽 (Idle3).
- `Shot1/2/3` = 왼쪽/수직/오른쪽 사격: 0~4 섬광·반동, 5~13 은 포신이 V 자로 벌어진 자세로 끝난다 -> Shot 끝 프레임이 어느 Idle/Turn 프레임과 이어지는지 미확인 (Boss_sync 의 주포 짝 검사와 같은 방법으로 측정 필요). E/F/G (단발) 도 같은 3 방향 구성.

### 원작 (`B5_turrets_V1_297-301.6.jpg`)
- V1 299.4~299.8: 선수 쪽 (오른쪽) 포탑이 포신을 왼쪽 위로 두고 사격 (섬광 299.6~299.8). 300.4 같은 포탑 피격 (금색) 중에도 왼쪽 위.
- V1 301.0~301.6: 선체 가운데 포탑 (E/F 계열) 이 포신 **수직** 으로 사격, 쌍발 점 (청록 -> 노랑 -> 빨강 깜박임) 이 바로 위로 오른다. 같은 시각 플레이어는 그 포탑의 거의 아래 (왼쪽 약간).
- 즉 포탑은 고정 방향이 아니라 자기 위치와 플레이어의 가로 거리에 따라 왼쪽 위/수직 (/오른쪽 위) 중 하나로 돌려 쏜다. 플레이어가 전함 오른쪽에 가는 장면은 영상에 없어 Shot3 사용은 직접 확인 못 함 (오른쪽 앵커 A/D 가 Idle3 로 쉬고 있다가 왼쪽으로 도는 것만 확인).
- 박자: Spec_BigShiee 실측 그대로 (한 번에 한 문, 쌍발 간격 0.17 s = 10 틱, 다음 포격 1.0~2.8 s). 회전 (Turn 9 프레임) 은 프레임당 약 3 틱.

### 리메이크 현재 (BigShiee.cpp)
- 이름 표 MakeNames_ 44-47: Idle1 / Idle3 / Shot1 만 쓴다 (Idle2, Shot2, Shot3 미사용).
- TickTurret_ 317-381: Prep 뒤 `pos > 0 ? TurnOut : Shot` (339) - 무조건 pos 0 (왼쪽) 으로 돌고 Shot1. 플레이어 위치를 보지 않는다. Idle 은 pos 0 또는 8 (SubmitTurret_).
- 포탄 x 속도는 weapon `shell.arc` (BigShiee.json:48) 가 플레이어 쪽으로 준다 (탄도는 조준됨, 머리 그림만 고정).

### 제안 (SONNET, M)
1. 조준 인덱스 = 플레이어 발 x - 포탑 포구 x (dx): `dx < -turretAimBandPx` -> 1 (왼쪽, Turn pos 0), `|dx| <= band` -> 2 (수직, pos 4), `dx > band` -> 3 (오른쪽, pos 8). `turretAimBandPx` 기본 72 (json).
2. Prep 끝에 목표 pos 로 Turn (현재 pos 에서 양방향 한 칸씩, turnFrameTicks), 도착하면 `Shot<aim>`. 쉬는 동안은 현재 pos 에 맞는 Idle (0 -> Idle1, 4 -> Idle2, 8 -> Idle3, 그 사이는 Turn 프레임 정지).
3. 쉬는 동안 추적 (선택): `turretTrackTicks` 30 마다 목표 pos 로 한 칸 (원작에서 쉬는 포탑이 천천히 따라오는지 미확인 -> 기본 0 = 끔).
4. 수직 사격 (aim 2) 의 포탄은 arc minVx 0 으로 거의 수직 (현재 arc 그대로 dx 비례).
5. Shot 끝 프레임과 Idle 짝 (HAIKU 측정) 결과대로 Shot 뒤 pos 를 정한다.
- 위험: 포탑 sort/depth (params.parts) 는 그대로. 기존 테스트 중 "오른쪽 포탑은 쏘기 전에 왼쪽으로 돈다" 류 기대는 플레이어가 왼쪽일 때만 참이 되도록 고친다.
- 수락: 유닛 `BigShieeTurretAim.FacesPlayerSide` (플레이어를 포탑 왼쪽/아래/오른쪽에 두고 각각 Shot1/Shot2/Shot3 와 pos 0/4/8), T0 로그 기반 장면 `turret_aim` (turret_fire 이벤트의 aim 이 원작 표: V1 299.6 오른쪽 포탑=1, 301.0 가운데 포탑=2 와 일치) >= 0.75.

---

## 작업 목록 (의존 순서)
| # | 모델 | 크기 | 작업 | 수락 |
|---|---|---|---|---|
| T0 | SONNET | M | 게임 `--event-log <file>` (Scoring.md T0 사건 목록) + scenes.json 런에 `eventLog: true` | `log` 앵커로 boss_explosion 채점 가능 (점수 무관), 유닛: 로그 줄 형식 |
| T1 | HAIKU | S | 포탑 Shot1/2/3 끝 프레임 vs Idle1/2/3·Turn 0..8 짝 표 (Boss_sync 2 절 방법) | 표 + 결론 문서 |
| T2 | SONNET | M | B4: scrollBudgetPx + 왕복 + 정착 위치 보정 (+선택 playerEdgePause) | camera_slide, boss_entrance >= 0.75 + BigShieeScroll 테스트 |
| T3 | SONNET | M | B5: 포탑 조준 3 방향 (T1 결과 사용) | BigShieeTurretAim 테스트 + turret_aim >= 0.75 (T0 후) |
| T4 | HAIKU | S | B1 데이터: missile speedRamp/trailSec/착탄 FX, dyingWreckSec 5.0, dyingDoneSec 7.0 | cannon_fire_down, boss_explosion >= 0.75 (T0 후) + BigShieeMissile 테스트 |
| T5 | SONNET | S | B1 포구 섬광/연기 원 FX | cannon_fire_down frame 향상 (>= 0.75 합산) |
| T6 | HAIKU | S | B3 아트: 시트 40204 CARTRIDGE -> Boss_Casing (+ DEPLOYMENT SPARK -> Boss_DeploySpark). **anim.json/아틀라스 변경 = 사용자 승인 필요** | 클립 존재, sprites.py 띠 이미지 확인 |
| T7 | SONNET | S | B3 탄피 생성/해석적 낙하/사라짐 (T6 후) | cannon_casing >= 0.75 + BigShieeCasing 테스트 |
| T8 | HAIKU | S | B2 데이터: tower onDestroy FX + hp (캐논 1 발) | PropTower 테스트 |
| T9 | HAIKU+사용자 | S | BossLab.json 에 아레나 망루 포함 (Stage 파일 = 사용자 승인) + bosslab_bot 전방 사격 구간 (SONNET S) | shack_destroy 채점 가능 -> >= 0.75 |

## 새 아트 필요 목록
- 없음 (새로 그릴 것 없음). 단 **자르기 필요**: `Boss_Casing` (시트 40204 "CARTRIDGE" 6 프레임), `Boss_DeploySpark` (같은 시트 "DEPLOYMENT SPARK" 5 프레임, 원작 주포 전개 때 불꽃 여부는 미확인).
- 원작 탄피는 은회색으로 보이고 시트는 황동색: 영상 압축/팔레트 차로 보이나 확인 필요.
- 망루 파괴 연기 버섯은 기존 `Boss_Smoke_Big` 로 대체 가능.

## 상위 위험
1. 작은 스프라이트 (탄피/포탄/고폭탄) 는 영상 템플릿 추적이 불가 -> T0 로그 없이는 위치 점수를 못 낸다.
2. bosslab_bot 자동 피해로 단계 시간이 원작보다 짧다 (카메라 멈춤 위치 비교에 섞임).
3. 다른 에이전트가 BigShiee.cpp/json 을 계속 바꾸는 중: exe 와 json 이 어긋나면 정의 로드 실패 (이번에 실제 발생, 미러로 우회). 채점은 빌드 직후에.
4. 사용자 요청 B2 의 "대포" 가 전함 주포라면 원작과 다르다 (원작은 플레이어 캐논) -> 사용자 확인 필요.
5. 사루비아 구간 막힘 (StageIntegration 봇이 x 12972.7 에서 멈춤): 사용자 수정 Stage0.json 이 openland_4 에 **새 lockCamera [12440,13080] + gate sarubia_dead** 를 넣었다 (이전엔 락 없음). 12972.7 = 락 오른쪽 13080 안쪽 (플레이어 가두기 여유). 테스트 봇 `ReachBossArena_` (Test/StageIntegrationTest.cpp:1142) 은 사격하지 않고 (`BotStep(true,false)`), 섹션 워프 뒤 KillAll 을 한 번만 해서 그 뒤 스폰/지연된 사루비아가 살아 있으면 락이 안 풀린다 -> **테스트 봇 쪽 문제일 가능성이 높다 (데이터 변경 + 비사격 봇)**. 실제 플레이는 사루비아를 잡으면 풀린다. 단 사루비아 스폰이 groundSnap 실패로 건너뛰어지면 (노트: legacy 는 이때 락도 건너뜀) 락/게이트 처리 여부를 확인할 것 (미검증, `--warp-x 12200` + 무기 사격으로 확인 필요).

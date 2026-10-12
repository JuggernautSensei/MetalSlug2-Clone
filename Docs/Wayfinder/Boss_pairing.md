# BigShiee 주포: 조준(Aim) <-> 사격(Shot) 짝, 발사 각도, 탄피

이전 분석 (`Boss_sync.md` 2 절) 은 아틀라스 픽셀 비교만 했다. 이번에는 실제 게임의 틱 순서 (`bosslab_bot`, 오프스크린, `--event-log` + `--capture-ticks`) 와 렌더 경로 (원점 - 피벗 = 좌상단) 로 다시 봤다.
짝: 사격 행 r (0 기준) = Aim 프레임 2r. Shot 칸 6 은 Aim 2r 과 아틀라스 사각형까지 같은 그림 (픽셀 차 0), 칸 0 은 포구 섬광 자세, 칸 7 은 같은 자세의 2 px 이동본.

## 1. 순서 버그 (논리, 수정함)

원인: `TickCannon_` Ready 에서 `TurnCannon_` 을 먼저 돌리고 같은 틱에 `yaw == target` 이면 바로 Shot 으로 들어갔다.
머리가 홀수 프레임 (예: Aim 7) 이었다면 그 틱에 Aim 6 으로 한 칸 돌면서 곧바로 Shot4 칸 0 을 그려서, 사격 행의 Aim 프레임은 화면에 한 번도 나오지 않았다 (Aim 7 -> Shot4 칸 0).
테스트가 옛 코드에서 잡은 값: 쏘기 직전 틱 yaw 7, 사격 행 yaw 6.

수정 (`Source/Game/Enemies/BigShiee.cpp`):
- 쏘는 조건: 이번 틱 `TurnCannon_` 이전에 이미 `yaw == target` 일 때만 (그 Aim 프레임이 최소 한 틱 보인 뒤). 이미 맞춰져 있으면 타이밍 변화 없음, 돌아야 하면 한 틱 늦다. 발사 간격 (3.04 s) 은 사격 시작 시각 기준이라 그대로.
- 사격 중 머리는 돌지 않는다 (옛 코드도 그랬음, 테스트로 고정).
- 사격 클립은 마지막 칸 (칸 7, Aim 자세의 2 px 이동본) 을 보여 주지 않고 칸 6 (= Aim 과 픽셀 동일) 에서 Ready 로 넘어간다 (`ShotTicks_`: 클립 틱 x (칸 수 - 1) / 칸 수 = 34 -> 29 틱). 돌아올 때 튀지 않는다. 고폭탄은 틱 4 에 나가므로 영향 없다.
- 점검한 다른 후보: (a) 홀수 yaw 반올림 / 2r vs 2(r-1) 오프바이원: 없음 (행 = target / 2, target 은 짝수). (b) 쏘는 순간 다른 목표로 행 재계산: 같은 틱 `target` 하나만 쓴다. (c) 사격 뒤 다른 yaw 로 복귀: 아니다 (같은 yaw). 단 Ready 로 돌아온 뒤에는 플레이어를 다시 따라간다 (홀수 yaw 포함, 이것은 추적). (e) 칸 0 이 섬광 자세: 원작 영상 (V2 303 s) 에서도 쏘는 순간 머리가 하얗게 번쩍인다 -> 의도.

테스트 (`Test/BigShieeTest.cpp`): `BigShiee.EachShotStartsAndEndsOnItsPairedAimFrame` (플레이어를 -60..60 도로 훑으며 14 발: 쏘기 직전 틱 yaw == 2r, 사격 중 yaw 불변, 끝난 틱 yaw == 2r, 사격 길이 == 클립 x 7/8), 옛 코드에서는 실패한다.

## 2. 남은 순수 피벗 차이 (사용자 조정 영역, 건드리지 않음)

`dx, dy` = 그 Shot 칸을 화면에서 이만큼 옮겨야 Aim 프레임에 겹친다 (+dx 오른쪽, +dy 아래). 피벗으로는 `pivotX - dx`, `pivotY - dy` (그리기 = 원점 - 피벗). 픽셀 차 = 겹친 알파 영역 평균 색차 (0..255).

| 행 | Aim 프레임 | Shot 칸 0 dx,dy | 칸 0 차 (그대로 -> 맞춘 뒤) | Shot 마지막 칸 (7) dx,dy | 마지막 차 (그대로 -> 맞춘 뒤) | 칸 6 |
|---|---|---|---|---|---|---|
| Shot1 | 0 | -2, +2 | 52.4 -> 35.3 | -2, +2 | 38.8 -> 13.6 | 0 |
| Shot2 | 2 | 0, +2 | 50.3 -> 36.3 | 0, +2 | 34.0 -> 10.8 | 0 |
| Shot3 | 4 | 0, +2 | 52.4 -> 37.1 | 0, +2 | 35.9 -> 11.2 | 0 |
| Shot4 | 6 | 0, +2 | 51.3 -> 35.8 | 0, +2 | 35.9 -> 9.8 | 0 |
| Shot5 | 8 | +2, +2 | 53.4 -> 40.9 | +2, +2 | 40.5 -> 17.3 | 0 |

- 칸 0 이 맞춘 뒤에도 35~41 인 것은 섬광 (흰 그림) 때문이다 (정상).
- 마지막 칸 7 을 맞춘 뒤 10~17 은 같은 자세의 작은 그림 차이. 현재는 칸 7 을 안 보여 주므로 화면에 영향 없다.
- Shot 행 마지막 칸 7 의 피벗을 (pivotX - dx, pivotY - dy) 로 두면 이동본이 Aim 에 맞는다 (예: Shot2..4 pivotY 256 -> 254). 칸 0 도 같다 (pivotY 256 -> 254).

## 3. 포구 위치 차이 (사용자 조정 영역, 건드리지 않음)

피벗이 좌상단 기준 (pivotX 2) 이라 주포 머리 중심은 원점에서 오른쪽 약 +100 px 인데, `cannonMuzzleX = [-42,-24,0,24,42]`, `missileMuzzle.y = -290` 은 머리 중심이 원점이던 시절의 값이다.
실제 게임에서 고폭탄/섬광/탄피가 포구 (그림의 구멍) 보다 약 100 px 왼쪽 (선체 위) 에서 나온다 (`Boss_pairing_after.png` 의 3054, 3058).
Aim 짝수 프레임의 포구 구멍 중심 (어두운 구멍 중심, 오차 +-4 px) 을 원점 (= `cannonAnchor`) 기준으로 재면:

| 행 | Aim | 구멍 중심 x (원점 기준) | 현재 `cannonMuzzleX` | 차 (더해야 할 값) |
|---|---|---|---|---|
| 0 | 0 | +40 | -42 | +82 |
| 1 | 2 | +73 | -24 | +97 |
| 2 | 4 | +102 | 0 | +102 |
| 3 | 6 | +130 | +24 | +106 |
| 4 | 8 | +163 | +42 | +121 |

구멍 중심 y = 약 -138 (원점 기준) -> `cannonAnchor.y (-170)` 합쳐 -308. 현재 `missileMuzzle.y` -290: 약 -18 px 차.
탄피는 포구 + `casingOffset` [60, -20] 이라 포구가 맞으면 원작처럼 머리 오른쪽 (약실 쪽) 에 나온다.

## 4. 조준 각도 (A) / 발사 각도 (B)

측정 (`Aim` 짝수 프레임의 구멍 중심 x, 머리 중심 기준): -62, -29, 0, +28, +61 px (좌우 대칭, 좌우 안 뒤집힘: Aim 0 = 왼쪽, 8 = 오른쪽).
원작 영상 V2 303.0 .. 303.7 s (카메라 정지: 탑 위치 3 px 이동뿐) 의 왼쪽 행 고폭탄 궤적을 주황 색 추적으로 재면 수직 기준 약 -47.3 도 (14 점 직선 맞춤).
선형 비례로 중간 행 +-22 도. 그래서 `cannonAimDeg` [-50,-25,0,25,50] -> [-47,-22,0,22,47] (`BigShiee.json`, `BigShieeCfg.h` 기본값). 홀수 yaw 는 이웃 두 행의 가운데.
발사 방향 == 각도 (`dx = sin, dy = cos`), 스프라이트 회전 = 속도 방향 (`rotate: true`, 기존 테스트 `MissileFliesAlongTheFiringRowAngleWithMatchingSprite`).
새 테스트:
- `BigShiee.MissileTravelDirectionMatchesTheBarrelArtWithinFiveDegrees`: 행마다 포구에서 10 틱 날아간 변위의 방향 오차 <= 5 도 (실측 오차 약 0 도: 속도 방향이 곧 각도), `cannonAimDeg` 도 그림에서 잰 표와 5 도 안.
- `BigShiee.ReadyHeadFollowsThePlayerAngleWithoutMirroring`: 플레이어 -50..50 도 (12.5 도 간격) 에서 보이는 머리 각도가 14 도 안, 좌우 부호 일치.

## 5. 탄피 (C)

- 발마다 정확히 한 번: 이벤트 로그 `bigshiee.casing` 1 개 / `missile_fire` 1 개 (tick 3054, 3236, 3418, 3600, 3782, 3964 모두), `BigShieeCasing.SpawnsOnEachShotAndVanishesWithin70Ticks`.
- 실제 카메라에서 보인다: 오프스크린 캡처 `Boss_pairing_after.png` 3054 ~ 3082 (포구 옆에서 튀어나와 회전하며 떨어진다, 깊이 54 = 포구 연기와 같음, 선체/주포 위에 그려짐, 오른쪽으로 튀므로 방향 정상).
- 이번에 추가: 지면에 닿은 뒤 `casingLingerSec` (0.3 s) 동안 서서히 투명해진다 (이전에는 갑자기 사라졌다).
- 남은 어긋남은 3 절의 포구 위치 (약 -100 px) 로 탄피도 머리 오른쪽이 아니라 왼쪽 선체 위에서 나온다.

## 6. 검증

- 캡처: `Boss_pairing_before.png` / `Boss_pairing_after.png` (tick 3048 .. 3108, bosslab_bot 첫 사격 = Shot1, 주황 칸은 봇이 때려 생긴 피격 플래시). `Boss_pairing_strip.png` = Aim 0..8 과 Shot1..5 전 칸.
- 하니스 (Release, `bosslab_bot` 같은 런):

| 장면 | 전 | 후 |
|---|---|---|
| cannon_fire_down | 0.327 (timing 0.474, position 0.004, frame 0.532) | 0.441 (timing 0.808, position 0.004, frame 0.524) |
| cannon_casing | 0.759 | 0.759 |

  position 0.004 는 3 절의 위치 차 (사용자 영역). timing 은 쏘는 시점 정렬이 들어가면서 올랐다.
- Debug 빌드 exit=0, 경고 0. SlugTest 908 중 903 통과. 실패 5 = `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` (알려진 것), `StageIntegration.Boss*` 2개 (알려진 것), `VehicleBTank.*` 2 개 (다른 작업의 탱크 변경, BigShiee 와 무관).
- Regress: enemylab_fight 7b12d9c08892fd47 PASS (불변), stage0_walk d9c16e2e80483af2 FAIL (Stage0/Tuning/Defs 사용자 편집과 다른 작업으로 이미 달라져 있다, BigShiee 는 이 리플레이에 나오지 않는다).

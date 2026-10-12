# 보스 1단계 작업 (T0 / T2 / T3 / T5) 결과

## 변경 요약
- T0 `--event-log <파일>`: 한 줄 JSON `{"tick":N,"event":"이름",...}` (하니스 `events.jsonl` 형식과 같다). 꺼져 있으면 아무 일도 안 한다 (`Base/Log.cpp` 의 `EventLogFormat`). scenes.json `boss` 런에 `"eventLog": true`.
  사건: `bigshiee.cannon` (mode Hidden/Deploy/Aim/Ready/Shot + yaw), `bigshiee.yaw`, `bigshiee.lid` (step 0..4), `bigshiee.turret_fire` (turret, barrel, aim, x, y), `bigshiee.turret_dead`, `bigshiee.missile_fire` (row, x, y, dx, dy), `bigshiee.muzzle_fx`, `bigshiee.cam` (30 틱마다 cam/boss x), `bigshiee.entrance_done`, `bigshiee.death`, `bigshiee.wreck`, `bigshiee.done`, `enemy.shot` (def, weapon, x, y, vx, vy; 모든 적 발사), `prop.destroyed` (def, x, y).
  탄피 사건은 없다 (탄피 클립/스폰 자체가 아직 없음, T6/T7).
- T2 카메라: 스크롤은 원래 속도 그대로, `scrollBudgetPx` (json 1160) 에 닿으면 카메라 정지 (아레나 끝). 정지 뒤 선체는 `shuttleDriftSec` (6 s) 동안 scrollSpeed 로 왼쪽으로 흐르다 기존 `driveTargetX` 로 돌진 (주기 약 8.5~9 s). 주포 전개 뒤 `reverseAtSec` 12.8 s 에 `reverseSec` 1.0 s 동안 `reverseSpeed` 36 px/s 로 한 번 후진. 위치 숫자 (driveTriggerX/driveTargetX/entranceKeys 등) 는 건드리지 않았다.
  1160 = V1 창 안 이동 1073 px + 창이 끝난 뒤 사망 (78.3 s) 까지 20 px/s 로 계속 간 만큼. (플랜의 1100 은 창 안 값. 1100 이면 하니스 창 끝 4 s 전에 멈춰 속도 곡선이 벌어진다.)
  타이밍 데이터 보정: `deployPauseDelaySec` 1.0 -> 0.5, `latePauseFirstSec` 12.5 -> 11.6, `latePausePeriodSec` 19 -> 18.6 (Boss_sync 가 주포 전개 시작을 뚜껑 + 1.2 s 로 늦춘 뒤 멈춤이 1 s 늦어져 있었다).
  bosslab_bot (StageState `bAutoDamage`): 포탑 단계 자동 피해만 12 틱 간격 (주포는 그대로 6 틱). 포탑 단계가 13 s -> 25 s 로 원작 (약 24 s) 에 맞는다. 하니스 전용 봇이라 게임/테스트/리플레이 영향 없음.
- T3 포탑 조준: 조준 위치 = 플레이어 발 x - 포구 x 가 `turretAimBandPx` (72) 안이면 Turn 프레임 4 (위, Idle2/Shot2), 밖이면 0 (왼쪽, Idle1/Shot1) / 8 (오른쪽, Idle3/Shot3). Prep 뒤 Turn 클립을 한 프레임씩 (`turnFrameTicks`) 돌려 목표에 닿으면 Shot<aim>. 쉬는 동안 `turretTrackTicks` (30) 마다 한 칸씩 따라간다 (쿨다운). 포구 x 는 왼쪽 자세 값 (muzzle.x), 위 = 0, 오른쪽 = -muzzle.x (새 숫자 없이 대칭).
  Shot 이 끝나면 오른쪽으로 복귀하지 않고 그 자세로 쉰다 (옛 TurnBack 제거).
- T5 포구 FX: 고폭탄 발사 틱에 기존 `Bazuka_Shot_Effect` (흰 섬광 -> 어두운 연기) + `Boss_Smoke_Small` 을 기존 포구 위치 (FireMissile_ 의 muzzle) 에 낸다. (`Boss_Blast_ABCD1` 은 전체 폭발이라 섬광으로 안 맞아 안 썼다.) 캡처로 확인 (tick 2072..2096).

## 점수 (하니스, Release, 같은 bosslab_bot 런)
| 장면 | 전 (BASELINE) | 후 | 메모 |
|---|---|---|---|
| boss_entrance | 0.749 | **0.831** | position 0.794, frame 0.874 |
| camera_slide | 0.600 | **0.764** | position 0.840 (속도 곡선), frame 0.675 |
| cannon_deploy | 0.457 | 0.539 | 미달: 위치 0.374 = 주포 x 가 원작보다 약 56 px 오른쪽 (driveTargetX / entranceKeys 끝값, 사용자 튜닝 영역). timing 0.572 |
| cannon_fire_down | 0.506 | 0.485 | 미달: 위치 0.003 (주포 187 px 차, 위치 숫자) |
| cannon_casing | 0.285 | 0.291 | 탄피 클립 없음 (T6/T7) |
| shack_destroy | 0.000 | 0.000 | 채점 불가: bosslab 에 망루 없음 (T8/T9) |
| boss_explosion | 0.000 (채점 불가) | 0.631 (실행마다 0.63~0.74) | 채점 가능해졌다 (log 앵커 `bigshiee.wreck`). frame 만 점수: wreck 추적은 연기/결산에 가려 3 프레임 미만. dyingWreckSec 6.3 -> 5.0 (T4) 이 남은 일 |
| turret_aim | - | - | 장면 없음 (하니스는 로그 필드 비교 미지원). 대신 유닛 테스트 BigShieeTurretAim 4 개 |

## 검증
- Debug 빌드 exit=0, 경고 0. SlugTest 891 중 887 통과, 1 건너뜀, 실패 3 = 알려진 것 (`AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor`, `StageIntegration.BossSpawns...`, `StageIntegration.BossDeath...`).
- 새 테스트: `BigShieeScroll.StopsAfterBudgetThenShuttles`, `BigShieeScroll.ReversesOnceAfterTheCannonDeploys`, `BigShieeTurretAim.FacesPlayerSideLeft/Right/Above`, `BigShieeTurretAim.IdleTurretsTrackThePlayerOneTurnFrameAtATime`.
- Regress: enemylab_fight 7b12d9c08892fd47 PASS (불변), stage0_walk acfc69d63f33c641 (기준 0dae6ffe6e917f7e 와 다름, 이전 Boss_sync 보고와 같은 해시 = 이번 변경 전부터 사용자 데이터 편집으로 달라져 있음).

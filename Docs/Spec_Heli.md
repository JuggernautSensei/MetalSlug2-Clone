# 헬기 (Aparch) 실측 · 사양 (Spec_Heli)

대상: `Assets/Defs/Vehicles.json` 의 `aparch`, `aparch_missile`, `aparch_drop`, `sarubia_grenade`, 구현 `Source/Game/Enemies/VehicleAparch.cpp`. 상세 원전 수치는 `Docs/EnemySpecs.md` §2.6.

## 0. 원본 영상 조사 결과 (중요)
- 대상 영상: `Assets/Source/videoplayback (2).mp4` (480x360, 29.5 fps, 378 s), `videoplayback (1).mp4` (388 s).
- 조사 범위: (2) 0–250 s 를 2–3 s 간격 콘택트 시트 (`Captures/heli/vid2_every3s_0_144.png`, `video_sheet_130_250.png`), 146–192 s 를 0.5–1 s 간격 (`vid_146_166.png`, `vid_176_192.png`, `vid_top_167_175.png`). (1) 은 6 s 간격 전체 (`vid1_every6s.png`).
- **결과: 두 영상 어디에도 헬기는 확인되지 않았다.** 150–165 s 는 지상 전투 (다리/망루/시가). 166–178 s 에 보이는 것은 **고정익 제트기 1대** (프로펠러/로터 없음, 저공으로 좌향 통과, `vid_plane_zoom.png`). 폭탄 투하/미사일 패턴 없음.
- 따라서 **원본 실측 수치(진입 경로, 호버 높이, 투하 간격, 착탄 위치)는 얻지 못했다.** 아래 값은 레거시 코드 (EnemySpecs §2.6, 코드 확인 = 신뢰도 높음) 와 게임 내 캡처 (신뢰도 중간) 로 정했다. 원본 영상 대조는 헬기가 나오는 다른 플레이 영상이 있어야 가능하다.

## 1. 게임 내 캡처로 확인한 문제 (수정 전 → 후)
| 항목 | 수정 전 | 수정 후 | 증거 |
|---|---|---|---|
| 스프라이트 위치 | `EnemyWorld::Submit` 이 `feet.y - GetFootBaseline()` 에 pivot 을 둔다. 발 기준 차량용 로직이라 원점이 기체 중심인 헬기는 스프라이트가 약 66 px 위로 밀렸다 (기체 상단이 화면 위로 잘리고 HUD 와 겹침) | `aparch.centerAnchor = true` → 원점에 pivot. 기체가 호버 점 (445,125) 중심으로 전신 표시 | `Captures/heli/cropB1.png` (수정 전), `cropD.png` (수정 후) |
| 호버 높이 | y 125 (유지). 스프라이트 상단 y≈59–68, HUD 줄 밑 | 동일 (`approach` y 125) | 테스트 `HoverKeepsTheWholeSpriteBelowTheHudRow` |
| 폭탄 착탄 | 접촉 판정이 스프라이트 중심. 폭탄 하단이 지면에 14–25 px 묻힌 뒤 터졌고 폭발 fx 는 중심 기준 -35 px (공중) | `groundProbe 25` (스프라이트 바닥) 로 접촉. 폭발 fx offset -10 (접촉 중심 위 35 px) | `cropF2.png` (폭발이 지면에 놓임) |
| 수류탄 착지 | 중심이 지면에 닿아 반쯤 묻힘 | `groundProbe 14`, 구르는 중 높이 378-14=364 | 테스트 `GrenadeRollsWithItsSpriteBottomOnTheGround` |
| 지붕/발판 | 셸은 지형 픽셀만 판정 → 지붕을 통과 | `ProjectileSystem::Step` 에 발판 span 추가. 낙하 중 윗면을 가로지르면 그 위에 착탄 | 테스트 `BombLandsOnTopOfARoof...`, `BombPassingBeside...` |

스프라이트 방향 (원본 시트 확대 `src_zoom_missile_bomb.png` 에서 확인): 헬기 기수 좌향 (돔 조종석 좌측), 미사일 머리 좌측·플룸 우측 (좌향 비행과 일치), 폭탄 머리 아래 (낙하 방향과 일치). 뒤집힘 문제 없음.

스프라이트 프레임 정렬: `Heli_Flying` 7프레임은 타이트 크롭이라 프레임마다 bbox 가 다르지만, 기체 무게중심의 pivot 대비 편차는 ±1.5 px (시트 분석). 점프는 아님. `Heli_Missile` 은 트림 후 폭 37–43 px 변동으로 머리 위치가 ±3 px 흔들린다 (허용 범위, 미수정).

## 2. 헬기 사양 (구현 값, `Vehicles.json` aparch)
| 항목 | 값 | 출처/신뢰도 |
|---|---|---|
| 진입 | 목표 (445,125) 로 300 px/s, 거리 100 px 부터 감속 (-600/s², 최소 50), 도착 반경 20 | 코드 (EnemySpecs 2.6), 높음 |
| 활성 | 화면 x ≤ W+400 (래치). 게이트 `aparch_dead` (river_mid) | Stage0.json, 높음 |
| 호버 | 중심 y 125 고정. x 는 플레이어 발 x 를 따라감 (vx = 1.5·dx) | 코드, 높음 |
| 패턴 | 대기 2 s → 폭탄/미사일 교대 (시작 패리티 랜덤 → 결정적 시드) | 코드, 높음 |
| 폭탄 패턴 | 0.5 s 단위 20 스텝, `(counter%7)&1` 일 때 투하 → 3발×3묶음, 묶음 간 1 s, 총 약 6 s | 코드, 높음 |
| 투하 위치 | 중심 + (10, 60) (향에 따라 x 반전) = 기체 하부 | 코드, 중간 (캡처에서 기체 바닥 위치와 일치) |
| 폭탄 물리 | 정지 출발, 중력 300 px/s², 직하 (수평 속도 0). 접촉: 스프라이트 바닥 (+25) | 코드 + 캡처 (`cropF2.png` 직하 확인), 높음 |
| 폭탄 착탄 | 지형/발판 윗면에서 `Explosion_Mid` (접촉 중심 위 35 px) | 캡처, 중간 |
| 미사일 패턴 | 포 전개 1 s → (540,248) 로 이동 1 s → 3발 1 s 간격, 머즐 (-30,+50) 좌향 300 px/s, 명중/지형에서 `Explosion_Mid` (-35) | 코드, 높음 (캡처 미확인, 저해상도 한계) |
| 사망 | `Aparch_Destroy_Left` 표류 (50,-70) px/s, 1.5 s 동안 0.3 s 간격 연쇄 폭발 6회, `Explosion_Large` 후 제거 | 코드, 높음 |
| 체력/점수 | TTK 12 s, hp 120, score 2000 (게이트) | 설계 값, 중간 |

## 3. 수치 불확실성
1. 원본 영상에 헬기 없음 → 레거시 코드 외에 실측 대조 불가 (위 0절).
2. 폭탄 착탄 x 는 "호버 x 가 플레이어를 따라감" 에 의존. 플레이어가 빠르게 움직이면 1.5 gain 의 지연으로 폭탄이 뒤에 떨어진다 (의도된 추적 지연).
3. 미사일 패턴은 캡처에서 헬기 미사일 궤적을 분리하지 못했다 (지상 제트/로켓과 겹침). 유닛 테스트 `MissilePatternFiresThreeLeftwardMissilesOneSecondApart` 로만 확인.

## 4. 검증 증거 (Captures/heli/)
`cropB1.png` 수정 전 (스프라이트가 상단으로 밀림), `cropD.png` 수정 후 (전신, 호버 중심), `cropE.png` 폭탄 투하 시퀀스, `cropF2.png` 폭탄 착탄 (지면), `cropM.png` 미사일 패턴 (구분 어려움), `cropS.png` openland_3 (warp 19: 개활지에서도 헬기 등장 확인), `cropS2.png` openland_4 (warp 20, 사루비아 구간), `cropG.png` 사루비아 수류탄 (망루 muzzle 에서 낙하).

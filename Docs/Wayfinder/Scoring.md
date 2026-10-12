# VideoScore: 원작 영상 대비 리메이크 장면 점수

원작 (`Trash/AssetsSource/videoplayback (1).mp4` = V1 496x360 30fps, `(2).mp4` = V2 480x360 29.5fps, 둘 다 아케이드 원작) 와
리메이크 오프스크린 캡처를 장면 단위로 맞춰 0..1 점수를 낸다. `Release 2026.10.10 ...mp4` (1920x1080) 는 리메이크 데스크톱 녹화라 기준으로 쓰지 않는다.

## 실행 (한 줄)

```
python Scripts/VideoScore/videoscore.py                       # 전 장면. 게임을 오프스크린으로 돌려 캡처 -> 채점 (약 3 분)
python Scripts/VideoScore/videoscore.py --scene cannon_deploy --reuse Captures/VideoScore/baseline   # 캡처 재사용, 채점만 (수 초~1 분)
python Scripts/VideoScore/videoscore.py --list
```
- 옵션: `--exe` (기본 `Output/Release/MetalSlug2_Clone.exe`), `--out` (기본 `Captures/VideoScore/<시각>/`), `--cwd` (Assets 를 찾을 루트), `--scenes`.
- 환경: python + opencv (cv2) + numpy. 게임은 `SDL_VIDEO_DRIVER=offscreen SDL_RENDER_DRIVER=software` 로 창 없이 돈다 (스크립트가 설정).
- 종료 코드: 모든 장면 >= `passThreshold` 이면 0, 아니면 1, 설정 오류 2. CI/에이전트 수락 조건으로 그대로 쓸 수 있다.
- 출력: `<out>/<scene>.json` (구성 점수, 이벤트 오차 틱, 추적 위치 쌍, 프레임별 유사도), `<out>/<scene>_sheet.png` (윗줄 원작 / 아랫줄 리메이크, 6 시점),
  `<out>/summary.json`, `<out>/runs/<run>/f_<tick>.png` (리메이크 캡처), `Captures/VideoScore/last.txt`.
- 주의: 캡처는 실시간 속도 (6000 틱 = 100 s). 다른 에이전트가 같은 `Assets/Defs` 를 고치는 중이면 exe 와 json 이 어긋나 정의 로드가 실패할 수 있다 (실제로 겪음): 빌드 직후에 돌릴 것.

## 파일
| 파일 | 내용 |
|---|---|
| `Scripts/VideoScore/videoscore.py` | 캡처 실행, 원작 프레임 추출, 검출기, 점수, 시트 |
| `Scripts/VideoScore/scenes.json` | 장면/런/가중치/문턱 정의 (데이터만 고쳐 장면 추가) |
| `Scripts/VideoScore/sprites.py` | `Assets/anim.json` + 아틀라스에서 템플릿/클립 띠 이미지 (읽기 전용). `python Scripts/VideoScore/sprites.py Boss_Turret_A_Shot1 --out a.png` |

## 좌표 정규화
- 원작 게임 화면 304x224 (아케이드 px). 리메이크 게임 px = 아케이드 px x 2 (아틀라스도 2x).
- 원작 영상 -> 크롭 (V2 는 위아래 4 px 띠 제거) -> 608x448 로 리사이즈 (= 아케이드 2x). 탑 템플릿 실측 배율 1.22/1.24 로 확인 (계산값 1.225/1.244).
- 리메이크 640x448 은 가운데 608 (x 16..624) 만 비교 (리메이크가 아케이드보다 16 px 넓게 보인다).
- HUD 는 위 44 / 아래 20 px 를 지우고 비교.

## 점수 = 가중 평균 (없는 구성은 가중치에서 빠진다)
| 구성 | 가중치 | 계산 |
|---|---|---|
| timing | 0.35 | 이벤트마다 `exp(-|dt| / 30 틱)`. dt = (리메이크 이벤트 - 리메이크 앵커) - (원작 이벤트 - 원작 앵커). 리메이크에서 검출 못 하면 0 |
| position | 0.35 | 추적 템플릿의 화면 위치 (게임 px) 중앙값 오차 e 로 `exp(-e / 40)`. 양쪽 다 잡힌 시점이 3 개 미만이면 생략. 카메라 장면은 대신 스크롤 속도 곡선: `1 - mean|v_ref - v_rem| / mean|v_ref|` (2 s 이동평균) |
| frame | 0.30 | 0.5 x SSIM (152x112 흐림) + 0.5 x 엣지 F1 (Canny, 2 px 허용) 의 평균을 `frameCeiling` 0.60 으로 나눈 값 (상한 1). 0.60 = 원작 V1 vs V2 (같은 사건, 다른 플레이) 실측 0.597 |

- 합격 문턱 `passThreshold` = **0.75**. 원작끼리 (V1 vs V2 보스 입장) 비교하면 frame 1.0, 위치 오차 수십 px 수준이라 0.75 는 "같은 연출, 다른 플레이" 수준이다.
- 검출기 (`scenes.json` 의 detector): 템플릿은 아틀라스 스프라이트 (`Clip:frame` 또는 `sprite:Name`), 그레이디언트 크기 영상끼리 `TM_CCOEFF_NORMED` (원작 압축/색 차에 강함), 평평한 영역 거짓 일치 제거, 화면 밖으로 걸친 스프라이트도 잡힘.
  - `appear` (minOn 프레임 연속 conf >= thr 인 첫 프레임), `vanish` (minOn 연속 있다가 minOn 연속 없어진 첫 프레임), `templates` (여러 템플릿 중 최댓값), `rot` (회전 템플릿), `sub` (템플릿 부분 [x0,y0,x1,y1] 비율), `occurrence` (n 번째), `startTick`.
  - `log`: 게임 이벤트 로그 `<out>/runs/<run>/events.jsonl` (한 줄 `{"tick":N,"event":"bigshiee.wreck"}`) 의 n 번째 이벤트. 게임 `--event-log <파일>` 이 쓴다 (T0 구현 완료: Source/Base/Log.cpp, 사건 이름은 아래 목록 참고. 실제 이름은 `bigshiee.cannon/yaw/lid/turret_fire/turret_dead/missile_fire/muzzle_fx/cam/entrance_done/death/wreck/done`, `enemy.shot`, `prop.destroyed`).
  - 원작 쪽 앵커는 기본이 사람이 잰 시각 (`ref.anchor`). `ref.detect` 를 주면 같은 검출기를 원작에도 돌려 앵커를 잡는다 (보스 입장/카메라).
- 작은 스프라이트 (Boss_Missile3 50x124, 포탄 점, 탄피) 는 그레이디언트 템플릿이 배경 잡음에 묻혀 신뢰할 수 없었다 (실험: 원작/리메이크 모두 HUD/지붕에 0.45~0.65 거짓 일치). 주포 사격은 대신 "Aim 자세가 안 맞게 되는 순간" (= Shot 클립 재생) 으로 잡는다.

## 장면 추가
1. 원작 시각을 고른다 (프레임 확인: `python Scripts/VideoScore/sprites.py` 와 함께 Docs/Wayfinder/ref 의 시트 참고). `ref.anchor` = 기준 사건 시각, `window` = 앵커 기준 [시작, 끝] 초.
2. 리메이크 쪽 같은 사건을 잡는 `remake.anchor` 검출기를 고른다 (큰 스프라이트가 나타남/사라짐이 가장 안정적). 필요하면 `runs` 에 새 런 (`--state`, 틱 범위, 간격) 을 더한다.
3. `events` (앵커 기준 원작 초 + 리메이크 검출기), `tracks` (위치를 비교할 템플릿) 를 더한다.
4. `--scene <이름> --reuse <이전 out>` 으로 돌려 `<scene>_sheet.png` 에서 정렬이 맞는지 눈으로 확인한 뒤 커밋.

## 기준 점수 (BASELINE, 2026-10-12, Release 빌드 = Boss_sync 수정 포함, 작업 트리 그대로)
캡처: `Captures/VideoScore/baseline/` (git 무시), 사본: `Docs/Wayfinder/ref/baseline/` (시트 + summary.json).

| 장면 | 항목 | 원작 앵커 | 점수 | timing | position | frame(raw) | 메모 |
|---|---|---|---|---|---|---|---|
| boss_entrance | B4 | V1 289.2 (검출) | **0.749** | - | 0.661 (선수 위치 오차 중앙 16.6 px) | 0.851 (0.511) | 입장 곡선 거의 맞음 |
| camera_slide | B4 | V1 289.2 +0..74 s | **0.600** | - | 0.540 (속도 곡선) | 0.670 | 평균 속도 원작 14.6 / 리메이크 15.3 px/s, 총 1073 / 1123 px. 멈춤 위치가 다름 (리메이크 14~22 s, 원작 24~34 s) |
| cannon_deploy | B1 | V2 300.60 | **0.457** | 0.501 (주포 상승 +35 틱, 첫 발 +11 틱) | 0.245 (주포 x 56 px 오른쪽) | 0.654 | 보스 구동/선체 위치 차가 주포 위치 오차로 |
| cannon_fire_down | B1 | V1 331.30 | **0.506** | 0.948 (다음 발 간격 +1.6 틱) | 0.009 (주포 위치 187 px 차) | 0.568 | 원작은 정면 (yaw 4) 수직 하강, 리메이크는 왼쪽 행 |
| cannon_casing | B3 | V1 316.60 | **0.285** | 0.000 (탄피 없음) | - | 0.617 | 탄피 스프라이트 없음 |
| shack_destroy | B2 | V1 363.00 | **0.000** | 채점 불가 | | | bosslab 에 망루 없음 + 아무것도 망루를 못 부숨 |
| boss_explosion | B1 | V1 373.00 | **0.000** | 채점 불가 | | | 리메이크 쪽 안정 앵커 없음 (잔해가 연기/MISSION COMPLETE 에 가림) -> T0 로그 필요 |
| bus_climb | U1/U2 | V1 69.60 | **0.255** | 0.012 (버스 파괴 -132 틱, 봇 화력 차) | 0.190 (버스 x 66 px 오른쪽) | 0.615 | 계단 오르기는 리메이크 뷰 밖이라 직접 비교 불가 |

자동 채점이 안 되는/약한 것과 이유:
- **boss_explosion**: 잔해 (Boss_Wreck) 템플릿이 연기/결산 글자에 가려 conf 0.19~0.37 로 흔들린다. 게임 이벤트 로그 (T0) 가 있으면 `log` 앵커로 바로 채점된다.
- **shack_destroy**: 리메이크 bosslab 런에 망루가 화면에 없다 (BossLab.json 은 Stage0 의 보스 구간 망루 14904 를 안 담음, `--state stage0 --warp-boss` 에서는 약 40 s 뒤 화면에 들어오지만 그 런은 봇 사격이 없다). 기능 (B2) 이 생기고 망루가 랩에 들어가야 채점된다.
- **포탄/고폭탄/탄피의 궤적 (위치)**: 작은 스프라이트라 템플릿 추적 불가 -> T0 로그 (발사/착탄 좌표) 로 비교해야 한다. 지금은 사격 시각만 채점.
- **포탑 조준 방향 (B5)**: 포탑 7 문이 겹쳐 있고 작아 자세 (Idle1/2/3) 판별이 영상에서 불안정. 로그 (포탑별 자세 인덱스) + 유닛 테스트로 검증해야 한다.
- **버스 계단 병사 (U1/U2)**: 원작은 계단이 버스 오른쪽 뒤 (화면 안), 리메이크는 계단 아래쪽이 카메라 락 오른쪽 밖 (stairsBase x = 앵커 + 186 = 3171 > 락 오른쪽 3122) 이라 보이지 않는다. 병사 템플릿은 종류/자세가 많아 추적 불가.
- 봇 의존: bosslab_bot 은 자동 피해 (6 틱마다 2) 로 단계가 원작 플레이보다 빠르다 (포탑 단계 약 13 s vs 원작 25 s). 장면 앵커가 단계 사건이라 앵커 이후 상대 시각만 비교하므로 대부분 상쇄되지만 카메라 멈춤 위치 비교 (camera_slide) 에는 섞인다.

## T0 (권장 선행 작업): 게임 이벤트 로그
`--event-log <file>` CLI: 틱마다 사건을 JSON 줄로 쓴다. 최소: `{"tick","event","x","y"}`.
사건 이름 제안: `bigshiee.entrance_done`, `bigshiee.turret_fire` (+turret, aim), `bigshiee.turret_dead` (+turret), `bigshiee.lid_start`, `bigshiee.cannon_ready`, `bigshiee.missile_fire` (+row,x,y), `bigshiee.missile_hit` (+x,y), `bigshiee.casing` (+x,y), `bigshiee.death`, `bigshiee.wreck`, `bigshiee.done`, `camera.x` (매 30 틱), `prop.destroyed` (+def,x), `bus.open`, `bus.soldier_spawn/climb_done/land` (+id,x,y,state), `bus.wreck`.
videoscore.py 는 `runs/<run>/events.jsonl` 이 있으면 `log` 앵커를 쓴다. T0 이 끝나면 scenes.json 의 런에 `"eventLog": true` 를 주면 캡처 때 `--event-log` 가 붙는다.

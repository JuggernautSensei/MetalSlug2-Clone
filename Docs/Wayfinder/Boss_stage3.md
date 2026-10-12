# 보스 3단계 (T4 / T6 / T7 / T8 / T9) 결과

## 변경
- T4 데이터 (Assets/Defs/BigShiee.json): `bigshiee_missile` speedRamp {60, 420, 330}, trailSec 0.05, 착탄 `PlayerBombExplosion` (blast 유지). `dyingWreckSec` 5.0, `dyingDoneSec` 7.0.
- T6 아트: `Trash/AssetsSource/40204.png` 의 CARTRIDGE 6 칸 (sheets.json 의 프레임 사각형) 을 `Assets/Textures/Boss_Casing.png` (192x24, 칸 32x24, 색키 153,217,234 / 0,255,0 투명) 로 잘라 `Assets/sprites.json` 끝에 `Boss_CasingStrip` 한 항목만 덧붙였다 (바이트 비교: 끝 부분 외 변경 없음). anim.json/아틀라스 불변.
- T7 코드: 고폭탄 발사 틱 (`missileFireTick`) 에 탄피 생성. 포구 + `casingOffset` [60,-20] 에서 `casingVel` [36,240], `casingGravity` 900, 지면 = 선체 원점 y + `casingGroundDy` 0, 지면 뒤 `casingLingerSec` 0.3 에 사라진다 (플랜의 0.1~0.3 안), `casingFps` 12 로 6 칸을 시간으로 돌린다. ext 슬롯 44..46. 이벤트 `bigshiee.casing` / `bigshiee.casing_gone`.
  단독 텍스처를 월드 스프라이트 큐에 올릴 길이 없어 `SpriteBatch` 에 `SetAssetRoot` / `FindStandalone` (상대 경로 지연 로드, 캐시) / `SubmitStandalone` (SpriteCommand.standalone) 을 더했다. 아틀라스 경로는 그대로.
- T8 데이터 (Props.json tower): `onDestroy` 에 PlayerBombExplosion (밑동), Explosion_Large x2, Explosion_Mid x2, Boss_Smoke_Big, 판자 6. hp 24 는 슬러그 캐논 피해 24 한 방.
- T9: BossLab.json `openland_props` 끝에 tower (14904, 1186) 한 객체를 텍스트로 삽입. 하니스 전용 `bAutoDamage` (bosslab_bot) 가 화면 안 망루를 피해 24 로 한 번 친다 (StageState.cpp). 이벤트 `prop.destroyed` (def tower) 는 기존 것.
- 하니스: `videoscore.py` 의 `log` 검출기에 `match` (필드 일치) 추가. scenes.json: cannon_casing 은 `bigshiee.casing` / `casing_gone` 로그 이벤트, shack_destroy 앵커는 `prop.destroyed` (def tower) 로그.
- 테스트: `BigShieeMissile.AcceleratesFromMuzzleToGround`, `BigShieeCasing.SpawnsOnEachShotAndVanishesWithin70Ticks`.

## 점수 (Release, bosslab_bot seed 1)
| 장면 | 전 (stage1) | 후 | 구성 | 남은 격차 |
|---|---|---|---|---|
| cannon_fire_down | 0.485 | 0.487 | timing 0.948, position 0.003, frame 0.512 | 주포 위치 187 px 차 = 사용자 위치 숫자 (driveTargetX/entranceKeys) 담당 |
| boss_explosion | 0.631 | 0.631 | frame 0.631 (wreck 추적 3 프레임 미만) | 연기/결산 가림. 시간 5.0 s 로 맞춤. 원작 영상과의 프레임 유사도 상한 |
| cannon_casing | 0.291 | **0.757** | timing 0.864, frame 0.633 | 위치 추적 없음 (작은 스프라이트) |
| shack_destroy | 0.000 | 0.594 | frame 0.594 (이벤트/추적 없음) | 앵커는 맞음. 화면 구성 (카메라/선체 위치) 차 = 사용자 위치 담당. 원작은 플레이어 캐논 탄이 날아와 맞는 장면, 리메이크는 봇 직접 피해 |

## 검증
- Debug 빌드 exit=0, 경고 0. SlugTest 898 중 892 통과, 3 건너뜀, 실패 3 = 알려진 것.
- Regress PASS: enemylab_fight 7b12d9c08892fd47, stage0_walk 4489aa958d2ebdda (불변).

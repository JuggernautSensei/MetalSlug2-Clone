# Assets / 리소스 미사용 후보 조사 (읽기 전용)

조사일 2026-10-13. 에셋/소스 수정 없음, 빌드 없음. 이 문서와 `Prune_Assets_Items.csv` 만 생성. 분석 대상은 작업 시점의 작업 트리 (`Assets/anim.json` 은 이미 `M`).

작업 시점 해시 (삭제 에이전트가 시작할 때 달라졌으면 이 문서를 다시 돌려야 한다): anim.json sha256 `a8beeffbc50bd640...`, sprites.json `6703c65cb0f7a309...`, sounds.json `46bba27efb6a76b7...`

## 1. 요약

| 구분 | 전체 | 사용 | 미사용 | 불확실 |
|---|---|---|---|---|
| Assets 파일 | 64 개 / 61.7 MB (64717732 B) | 58 | 1 개 / 10,030 B | 5 개 / 469,206 B |
| anim.json 클립 | 698 | 689 (테스트 필수 3 포함) | 2 (프레임 3) | 7 (프레임 38) |
| sprites.json 항목 | 174 | 174 (테스트 필수 1 포함) | 0 | 0 |
| sounds.json 항목 | 25 | 20 | 1 | 4 |
| 아틀라스 페이지 | 16 | 16 | 0 | 0 |

정리 후보의 실질 절감 (파일 단위, 확정 미사용만): **10,030 B (Audio/Sfx_Bingo.mp3 1개)**. 불확실까지 전부 지우면 +469,206 B. Assets 는 이미 이전 정리(Cleanup.md)를 거쳐 거의 비어 있다. 큰 용량은 Assets 밖에 있다:

| 위치 | 파일 수 | 크기 | 추적 | 복구 | 성격 |
|---|---|---|---|---|---|
| `Trash/` 추적분 | 1,586 | 925,668,994 B (883 MB) | tracked | git 이력으로 복구 가능 | 옛 파이프라인/원본 리소스/옛 문서 |
| `Trash/AssetsSource/Release 2026.10.10 - 20.46.29.11.mp4` | 1 | 270,542,995 B (258 MB) | untracked (ignore 아님) | **복구 불가** | 녹화 영상 |
| `Trash/AssetsSource/Metal-Slug-Remake/` (gitlink 160000, 내부 별도 git) | 192 + 내부 .git | 522,704,457 B (499 MB) | 본 저장소에서는 gitlink 1개 | 원격(github ahmadraza2730/Metal-Slug-Remake) 있음. **로컬 수정 Knife.h, Menu.h 와 untracked `.vs/`(494 MB) `Metal Slug/`(1.1 MB) 는 복구 불가** | 외부 저장소 사본 |
| `Trash/**` ignore 된 잔여 | 22 | 25,952 B | ignored | 복구 불가 (생성물 txt/bmp) | Alian/BattleShip output |
| `Captures/` | 4,718 | 908,869,177 B (867 MB) | gitignore | 복구 불가, 도구로 재생성 가능 | 캡처 산출물 |
| `Backup/` | 131 | 7,682,674 B | gitignore | 복구 불가 | Assets 손편집 스냅샷 8 개 |
| `Docs/**` 이미지/CSV 중 파일명 미참조 | 20 | 3,515,596 B | tracked | git | 문서 스크린샷 |
| `Raw/` | 4 | 2,680,187 B | tracked | git | 원본 시트 (3 개는 Trash/AssetsSource 와 바이트 동일) |
| (참고) `Output/` `Intermediate/` `vcpkg_installed/` | 12 / 1,100 / 289 | 564,457,194 / 2,202,945,239 / 178,994,304 B | gitignore | 빌드로 재생성 | 이번 범위 아님 |

### 보호 목록 (삭제 금지)
- `Test/Replays/*` 16 개 / 57,503 B (`*.replay` 15 개 + `sandbox_regression.rep`). `Scripts/Regress.ps1` 이 `*.replay` 를 글롭, `.rep` 은 `Test/PlayerReplayTest.cpp:89/109` 가 씀/읽음.
- `Docs/Wayfinder/Baseline.txt`, `BaselineCapture.txt`, `BaselineTextures.txt` (Regress.ps1 / Verify.ps1 / TraceLoad.ps1 기준선).
- 사용자 손편집 데이터: `Assets/anim.json` `sprites.json` `sounds.json` `Stage/*.json` `Ground/*.png` `Defs/**` `Tuning/Tuning.json`. 읽기만 했다. anim.json 은 에디터로 수정 중이므로 삭제 작업 중 에디터를 닫을 것.
- `Trash/Unused/` (anim_removed.json, sprites_sounds_removed.json, 이전 정리의 복구본 14 파일 2,639,759 B). `Trash/` 를 지울 때도 마지막까지 남길 것.
- 테스트가 존재를 요구하는 항목: FlameShot_Orig_Head/Loop/Fade (PlayerWeaponTest), PressStartUI (UiTest), AudioSink.h `kAll` 25 개 (AudioTest.cpp:136), 위 표의 `사용(테스트 필수)`.

### 삭제 후 검증 방법 제안
1. **삭제 전**에 현재 상태로 `powershell -NoProfile -File Scripts/Verify.ps1` 을 한 번 돌려 기준선이 맞는지 확인 (빌드 + Regress 리플레이 해시 15 개 vs Baseline.txt + 캡처 해시 7 상태 + 튜닝 덤프 + SlugTest 실패 목록 = 알려진 실패 1 개). 같은 명령을 삭제 후에 다시 돌려 전부 동일해야 한다. `-Update` 금지.
2. `Scripts/TraceLoad.ps1` : 로드되는 텍스처/아틀라스 목록이 `BaselineTextures.txt` 와 같은지 (async/sync 둘 다). Textures/Atlas 파일 누락 검증.
3. `Scripts/ValidateStage.ps1` (Stage0.json / BossLab.json 각각): 스테이지가 쓰는 에셋 이름이 존재하는지.
4. 이름 누락 검증은 현재 기존 스크립트에 없다. 별도 스크립트(읽기 전용, 예: `Scripts/AuditAssets.py` 신규) 로 이 문서와 같은 방식을 재현하는 것을 권장: (a) Source/Test 문자열 리터럴 + Defs/Stage/Tuning JSON 문자열 + `{side}` `{W}` `{S}` 확장 + 접두를 모은 참조 집합, (b) anim/sprites/sounds 키 집합, (c) **참조 집합 중 키가 아닌 것 중 클립/스프라이트/사운드 이름처럼 보이는 것이 삭제 전보다 늘지 않았는지**, 삭제 목록이 참조 집합과 교집합 0 인지 확인. 이 문서의 조사 스크립트는 scratchpad 에 있고 저장소에는 넣지 않았다.
5. 최종: `Output/*/SlugTest.exe` 전체 + `--headless --replay` 전 리플레이. 런타임이 없는 이름을 조용히 폴백하는 경우(PlayerRig 는 필수 클립 누락 시 WARN 후 폴백)가 있으므로 로그에 `필수 클립` 경고가 0 인지도 확인.

### 판정 기준과 한계
- 참조 = `Source/**` `Test/**` 의 `.cpp .h` 문자열 리터럴 (주석 제거 후) + `Assets/Defs/**` `Assets/Stage/*` `Assets/Tuning` `manifest.json` 의 JSON 문자열(키 포함).
- Cleanup.md 기준 유지: `{side}`(Left/Right) `{W}`(MG/SG/DG, PlayerRig.cpp:73-77) `{S}`(Left/Right) `{}` `{:02}` 확장, 접미 `_Left`/`_Right` 제거 후 일치 허용, 코드가 번호를 붙여 조합하는 접두 보존 (Boss_Turret_, Boss_Cannon_Shot, Heli_Rotor_, Slug_Vulcan_Shot_, Slug_Shell_, Slug_CannonFx_, Bus_BackD/BodyD, Fx_Blood_Hit_/Splat_, ShotGun_Bullet_, HM_Bullet_, UI_Mission_, Wood/Rock/Metal N).
- 판정은 문자열이 코드 어딘가에 있으면 `사용` 이다 (그 코드가 실제 실행되는지는 보지 않음). 따라서 미사용 후보는 보수적이다. 에디터 전용 경로에서만 쓰이는 이름도 `사용`.
- 접두 일치 중 `Parker` 는 Stage0.json 의 `"Parker"` 때문에 `Parker_Run` `Parker_Stopping` 이 걸렸으나 이는 우연이라 수동으로 `불확실` 로 내렸다.
- 테스트에서만 나오는 이름은 별도 분류 (`테스트만`): 존재 필수면 `사용(테스트 필수)`, 허용 목록 문자열이면 `불확실`.

## 2. Assets 파일 수준 (64 개)

| 파일 | 크기 | 참조 근거 | 판정 |
|---|---|---|---|
| `Assets/Atlas/atlas_000.png` | 500453 B | manifest.json atlasPages[0]; anim.json 프레임 37개 + sprites.json 항목 2개가 atlas=0 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_001.png` | 559631 B | manifest.json atlasPages[1]; anim.json 프레임 58개 + sprites.json 항목 2개가 atlas=1 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_002.png` | 550134 B | manifest.json atlasPages[2]; anim.json 프레임 41개 + sprites.json 항목 0개가 atlas=2 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_003.png` | 621057 B | manifest.json atlasPages[3]; anim.json 프레임 26개 + sprites.json 항목 2개가 atlas=3 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_004.png` | 623289 B | manifest.json atlasPages[4]; anim.json 프레임 53개 + sprites.json 항목 6개가 atlas=4 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_005.png` | 598648 B | manifest.json atlasPages[5]; anim.json 프레임 28개 + sprites.json 항목 6개가 atlas=5 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_006.png` | 730859 B | manifest.json atlasPages[6]; anim.json 프레임 124개 + sprites.json 항목 4개가 atlas=6 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_007.png` | 827230 B | manifest.json atlasPages[7]; anim.json 프레임 156개 + sprites.json 항목 34개가 atlas=7 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_008.png` | 838748 B | manifest.json atlasPages[8]; anim.json 프레임 150개 + sprites.json 항목 8개가 atlas=8 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_009.png` | 806122 B | manifest.json atlasPages[9]; anim.json 프레임 262개 + sprites.json 항목 4개가 atlas=9 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_010.png` | 881247 B | manifest.json atlasPages[10]; anim.json 프레임 257개 + sprites.json 항목 7개가 atlas=10 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_011.png` | 1002185 B | manifest.json atlasPages[11]; anim.json 프레임 476개 + sprites.json 항목 3개가 atlas=11 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_012.png` | 1028308 B | manifest.json atlasPages[12]; anim.json 프레임 732개 + sprites.json 항목 0개가 atlas=12 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_013.png` | 941116 B | manifest.json atlasPages[13]; anim.json 프레임 826개 + sprites.json 항목 51개가 atlas=13 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_014.png` | 884998 B | manifest.json atlasPages[14]; anim.json 프레임 1021개 + sprites.json 항목 13개가 atlas=14 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Atlas/atlas_015.png` | 678267 B | manifest.json atlasPages[15]; anim.json 프레임 1099개 + sprites.json 항목 26개가 atlas=15 로 참조; StageEditor.cpp:346/AnimEditor.cpp:503 이 atlas_{:03}.png 조합 | 사용 |
| `Assets/Audio/Boss_BGM.ogg` | 2562452 B | sounds.json "Boss_BGM" -> 이름 정확 일치: Assets/Stage/Stage0.json | 사용 |
| `Assets/Audio/Coin.wav` | 136028 B | sounds.json "Coin" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/Field_BGM.ogg` | 3445507 B | sounds.json "Field_BGM" -> 이름 정확 일치: Assets/Stage/Stage0.json | 사용 |
| `Assets/Audio/OK.wav` | 220516 B | sounds.json "OK" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerBomb.wav` | 133720 B | sounds.json "PlayerBomb" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGunDead.wav` | 24360 B | sounds.json "PlayerGunDead" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGunShot.wav` | 65836 B | sounds.json "PlayerGunShot" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGunShot_FS.wav` | 191124 B | sounds.json "PlayerGunShot_FS" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGun_EC.wav` | 346572 B | sounds.json "PlayerGun_EC" -> Source/Audio/AudioSink.h 의 상수 kVoiceEnemyCh 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 | 불확실 |
| `Assets/Audio/PlayerGun_FS.wav` | 367316 B | sounds.json "PlayerGun_FS" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGun_HM.wav` | 402788 B | sounds.json "PlayerGun_HM" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGun_RL.wav` | 335032 B | sounds.json "PlayerGun_RL" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerGun_SG.wav` | 309660 B | sounds.json "PlayerGun_SG" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerReload.wav` | 99308 B | sounds.json "PlayerReload" -> Source/Audio/AudioSink.h 의 상수 kReload 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 | 불확실 |
| `Assets/Audio/PlayerSlash1.wav` | 65532 B | sounds.json "PlayerSlash1" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/PlayerSlash2.wav` | 76268 B | sounds.json "PlayerSlash2" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/Sfx_Bingo.mp3` | 10030 B | sounds.json "Sfx_Bingo" -> Source/Test/Stage/Defs 어디에도 문자열/상수 없음 (AudioSink.h 에도 없음) | 미사용 |
| `Assets/Audio/Sfx_GameOver.mp3` | 20688 B | sounds.json "Sfx_GameOver" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/Sfx_HereWeGo.mp3` | 11701 B | sounds.json "Sfx_HereWeGo" -> Source/Audio/AudioSink.h 의 상수 kHereWeGo 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 | 불확실 |
| `Assets/Audio/Sfx_LetsGo.mp3` | 10448 B | sounds.json "Sfx_LetsGo" -> Source/Audio/AudioSink.h 의 상수 kLetsGo 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 | 불확실 |
| `Assets/Audio/Sfx_Mission4Start.ogg` | 73523 B | sounds.json "Sfx_Mission4Start" -> 이름 정확 일치: Assets/Stage/Stage0.json | 사용 |
| `Assets/Audio/Sfx_MissionComplete.ogg` | 63255 B | sounds.json "Sfx_MissionComplete" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/Sfx_PowerUp.mp3` | 12119 B | sounds.json "Sfx_PowerUp" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/Stage1_BGM.wav` | 41193588 B | sounds.json "Stage1_BGM" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Audio/Title_MS2.ogg` | 136340 B | sounds.json "Title_MS2" -> 이름 정확 일치: Source/Audio/AudioSink.h | 사용 |
| `Assets/Defs/BigShiee.json` | 7841 B | EnemyDefLoader.cpp:1534 LoadDirectory(Defs) 가 *.json 전부 로드 (EnemyDefLoader.cpp:1593); DepthEditor/HitboxEditor 파일 목록 | 사용 |
| `Assets/Defs/Bus.json` | 5072 B | EnemyDefLoader.cpp:1534 LoadDirectory(Defs) 가 *.json 전부 로드 (EnemyDefLoader.cpp:1593); DepthEditor/HitboxEditor 파일 목록 | 사용 |
| `Assets/Defs/Enemies.json` | 20011 B | EnemyDefLoader.cpp:1534 LoadDirectory(Defs) 가 *.json 전부 로드 (EnemyDefLoader.cpp:1593); DepthEditor/HitboxEditor 파일 목록 | 사용 |
| `Assets/Defs/Helicopter.json` | 5044 B | EnemyDefLoader.cpp:1534 LoadDirectory(Defs) 가 *.json 전부 로드 (EnemyDefLoader.cpp:1593); DepthEditor/HitboxEditor 파일 목록 | 사용 |
| `Assets/Defs/Props/Props.json` | 20104 B | SandboxState.cpp:166, StageState.cpp:285 PropDefLibrary::Load | 사용 |
| `Assets/Defs/Vehicles.json` | 13076 B | EnemyDefLoader.cpp:1534 LoadDirectory(Defs) 가 *.json 전부 로드 (EnemyDefLoader.cpp:1593); DepthEditor/HitboxEditor 파일 목록 | 사용 |
| `Assets/Ground/Stage0_0.png` | 185637 B | manifest.json ground; Stage0.json:6 ground.maps; SandboxState.cpp:135; GroundEditorTest/StageDataTest | 사용 |
| `Assets/Ground/Stage0_1.png` | 23926 B | manifest.json ground; Stage0.json:6 / BossLab.json:26 ground; SandboxState.cpp:318 | 사용 |
| `Assets/Stage/BossArena.json` | 1177 B | 문자열 "BossArena.json" 참조 없음 (StateFactory 는 BossLab.json 만). StageEditor.cpp:1108 이 Stage/ 디렉터리를 열거해 편집기 목록에만 나타남. boss_arena 섹션은 Stage0.json / BossLab.json 에 인라인으로 이미 들어 있음 (각 1회). 사용자 손편집 데이터 | 불확실 |
| `Assets/Stage/BossLab.json` | 4588 B | StateFactory.cpp:56 config.stageFile = "BossLab.json" | 사용 |
| `Assets/Stage/Stage0.json` | 32328 B | StageState 기본 stageFile (StageState.h:34); 다수 테스트 | 사용 |
| `Assets/Textures/Boss_Casing.png` | 1665 B | sprites.json "Boss_CasingStrip" file; BigShiee.cpp:1218 Find("Boss_CasingStrip") | 사용 |
| `Assets/Textures/Stage0_Background_0.png` | 54325 B | sprites.json "Stage0_Background_0" file; Stage0.json:7 / BossLab.json:36,45 backdrop sprite | 사용 |
| `Assets/Textures/Stage0_Background_1.png` | 98974 B | sprites.json "Stage0_Background_1" file; Stage0.json:7 / BossLab.json:62 backdrop sprite | 사용 |
| `Assets/Textures/Stage0_House_4_PixelMap.png` | 38553 B | sprites.json file; Props.json:360 groundPatch; StageState.cpp:814 Textures/<sprite>.png; StageIntegrationTest.cpp:267 | 사용 |
| `Assets/Textures/Stage0_House_5_PixelMap.png` | 2984 B | Props.json:386 groundPatch.sprite -> StageState.cpp:814 이 Textures/<sprite>.png 를 로드 (sprites.json 항목은 atlas 9 라 file 참조 없음); StageIntegrationTest.cpp:293 | 사용 |
| `Assets/Textures/Stage0_Main.png` | 1149165 B | BossLab.json:14 background -> StageState.cpp:197; SandboxState.cpp:208; Stage0.json background | 사용 |
| `Assets/Textures/TitleBackground.png` | 24117 B | sprites.json "TitleBackground" file; TitleState.cpp:119 DrawStandalone | 사용 |
| `Assets/Tuning/Tuning.json` | 5640 B | Tuning.cpp:450 경로 조합; AssetPath.cpp:42; HitboxEditor/테스트 | 사용 |
| `Assets/anim.json` | 616207 B | manifest.json animations; Animation.cpp:220 | 사용 |
| `Assets/manifest.json` | 578 B | Animation.cpp:209, App.cpp:302, AssetPath.cpp:42 | 사용 |
| `Assets/sounds.json` | 1794 B | manifest.json sounds; SoundBank.cpp:69 | 사용 |
| `Assets/sprites.json` | 18923 B | manifest.json sprites; SpriteLibrary.cpp:88 | 사용 |

소계: Atlas 16 / 12,072,292 B, Audio 25 / 50,313,711 B (Stage1_BGM.wav 41,193,588 B 가 전체의 64%), Textures 7 / 1,369,783 B, Ground 2 / 209,563 B, Stage 3 / 38,093 B, Defs 6 / 71,148 B, Tuning 1 / 5,640 B, 루트 JSON 4 / 합계 64,717,732 B.

## 3. Assets 항목 수준

| 종류 | 전체 | 사용 | 사용(테스트 필수) | 불확실 | 미사용 |
|---|---|---|---|---|---|
| anim.json 클립 | 698 | 686 | 3 | 7 | 2 |
| sprites.json 항목 | 174 | 173 | 1 | 0 | 0 |
| sounds.json 항목 | 25 | 20 | 0 | 4 | 1 |

사용 항목의 참조 방식 분포 (전체 목록은 CSV): 
- anim / Source/Game/Props/PropDef.cpp : 9
- anim / 이름 정확 일치 : 148
- anim / 접두 조합 : 81
- anim / 포맷/{side} 패턴 : 448
- sound / 이름 정확 일치 : 20
- sprite / 이름 정확 일치 : 74
- sprite / 접두 조합 : 99

### 미사용 / 불확실 항목 전체 (CSV 와 동일)

| 종류 | 이름 | 판정 | 근거 |
|---|---|---|---|
| anim | `Plane` | 미사용 | Source/Test/Defs/Stage/Tuning 문자열 어디에도 없음. 2프레임 |
| anim | `Parker_Run` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:191 배우별 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Stage0.json 은 Parker, Parker_Run_2 만 사용. Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `Parker_Stopping` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:191 배우별 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Stage0.json 은 Parker, Parker_Run_2 만 사용. Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `Order_Boy_Run_Left` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:192 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `Order_Boy_Run_Right` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:192 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `Order_Stopping` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:192 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `Order_Recoil` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:192 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `Order_Turn` | 불확실 | 코드/Defs/Stage 미참조. Test/AnimIntegrityTest.cpp:192 허용 목록 문자열에만 등장 (존재를 요구하지 않음). Cleanup.md 는 테스트 보호로 유지했음 |
| anim | `FlameShot_Orig_Head` | 사용(테스트 필수) | Test/PlayerWeaponTest.cpp:495 ASSERT_NE(FindClip) / :824; 런타임 코드는 미참조 |
| anim | `FlameShot_Orig_Loop` | 사용(테스트 필수) | Test/PlayerWeaponTest.cpp:495 ASSERT_NE(FindClip) / :824; 런타임 코드는 미참조 |
| anim | `FlameShot_Orig_Fade` | 사용(테스트 필수) | Test/PlayerWeaponTest.cpp:495 ASSERT_NE(FindClip) / :824; 런타임 코드는 미참조 |
| anim | `Boss_Disc_Black` | 미사용 | 어디에도 문자열 없음. BigShiee 의 deathDiscOffset/partDisc 설정만 있고 클립을 그리는 코드 없음. 1프레임 |
| sprite | `PressStartUI` | 사용(테스트 필수) | Test/UiTest.cpp:348 ASSERT_NE(library->Find) + :62 크기 표; 런타임 코드는 미참조 (Cleanup.md 가 복구해서 유지) |
| sound | `PlayerGun_EC` | 불확실 | Source/Audio/AudioSink.h 의 상수 kVoiceEnemyCh 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 |
| sound | `PlayerReload` | 불확실 | Source/Audio/AudioSink.h 의 상수 kReload 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 |
| sound | `Sfx_Bingo` | 미사용 | Source/Test/Stage/Defs 어디에도 문자열/상수 없음 (AudioSink.h 에도 없음) |
| sound | `Sfx_LetsGo` | 불확실 | Source/Audio/AudioSink.h 의 상수 kLetsGo 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 |
| sound | `Sfx_HereWeGo` | 불확실 | Source/Audio/AudioSink.h 의 상수 kHereWeGo 와 kAll 배열에만 등장, Play 호출 0건. Test/AudioTest.cpp:136 이 kAll 전체가 해석되는지 검사하므로 삭제하면 AudioSink.h(kAll 25->N) 수정 필요 |

삭제 시 같이 처리할 곳: 미사용 anim 클립 2 개 (`Plane` 2프레임, `Boss_Disc_Black` 1프레임, anim.json 본문 약 744 B) 은 anim.json 에서만 제거. `Sfx_Bingo` 는 sounds.json 항목 + `Audio/Sfx_Bingo.mp3` 제거 (다른 곳에 없음). 불확실 anim 7 클립은 anim.json 본문 약 6,776 B (프레임 38) 이고 아틀라스 용량은 재패킹 전엔 줄지 않는다.

## 4. 아틀라스 페이지 내부

- 16 페이지 전부 anim/sprites 가 참조한다. 페이지 전체 미참조 = 0. 페이지당 anim 프레임/sprites 항목 수는 2 절 표에 있다.
- 프레임/스프라이트 사각형이 덮는 면적은 전체 67,108,864 px 중 46,900,655 px (69.9%). 미덮개 30.1% 는 투명 여백/패킹 간격을 포함해 "사용 안 하는 스프라이트" 와 구분되지 않는다. 페이지별 덮개율: 0=81.0 1=91.9 2=59.6 3=39.5 4=60.0 5=28.0 6=72.9 7=84.5 8=67.0 9=89.7 10=72.2 11=75.8 12=84.2 13=78.6 14=82.2 15=51.1 (%). 낮은 페이지 3, 5, 15 가 재패킹 이득이 가장 크다.
- 미사용/불확실 클립 9 개 (프레임 41) 를 모두 지워도 덮개 면적은 118,588 px 만 줄어든다 (0.18%). 미사용 스프라이트 영역 제거는 **재패킹 필요, 이번 범위 아님**.

## 5. Assets 밖

| 대상 | 파일 / 크기 | 참조 | 판정 |
|---|---|---|---|
| `Raw/` (tracked) | 4 / 2,680,187 B (108017 163,593; 143506 357,803; 20642 59,512; 40204 2,099,279) | Source/Test/Scripts 에서 경로 참조 없음. `BigShiee.cpp:12` 주석에 시트 번호만. Docs/Wayfinder/Map.md, Vehicles_plan.md 등 문서가 `Raw/108017.png` `Raw/143506.png` 를 서술. README.md 는 Raw 를 언급하지 않음. 143506/20642/40204 는 `Trash/AssetsSource/` 에 바이트 동일본 (md5 일치) 이 있음, 108017 은 유일본 | 미사용 (런타임/테스트/스크립트 참조 0). 108017 은 유일본이므로 삭제 시 Trash/AssetsSource 로 이동 권장 |
| `Backup/` (gitignore) | 8 스냅샷 / 131 / 7,682,674 B (20261012_001840 .. _150645) | 코드/스크립트 참조 없음. 각 스냅샷은 Assets 의 사용자 손편집 JSON/Ground 사본 (구형 Tuning.json 위치 포함). 일부는 MANIFEST.md5 | 불확실 (사용자 데이터 스냅샷, 복구 불가, memory 규칙: 옛 사본으로 복원 금지이지만 삭제도 사용자 판단) |
| `Captures/Pairing` | 1,604 / 333,247,713 B (vs_after BMP) | 참조 없음 (Source/Test/Scripts 에 Pairing 경로 없음, Docs/Wayfinder/Boss_pairing.md 는 이미지 사본을 별도로 가짐) | 미사용 (재생성 가능 산출물) |
| `Captures/VideoScore` | 3,042 / 546,021,686 B (after1, baseline, stage3, stage3b, last.txt, stage3b.txt) | `Scripts/VideoScore/videoscore.py:517,546` 이 출력 디렉터리/`last.txt` 로 사용, Scoring.md 가 `--reuse Captures/VideoScore/baseline` 안내 (사본은 Docs/Wayfinder/ref/baseline) | 불확실 (baseline/last.txt 만 보존, 나머지는 재생성 가능) |
| `Captures/Visual` | 12 / 5,252 B (*.rep) | `Test/PlayerReplayTest.cpp:91` 이 이 폴더에 씀 (테스트가 재생성) | 미사용 (재생성) |
| `Captures/tmp` | 58 / 29,350,697 B | 참조 없음. 안에 `Bus.json.mine` (5,300 B, Bus.json 손편집 사본으로 보임), `patch.py`, `sum.py`, `s4/` | 불확실 (Bus.json.mine 은 사용자 데이터일 수 있어 확인 전 삭제 금지) |
| `Captures/bus_trace.csv`, `bus_trace_stage.csv` | 84 KB / 160 KB | 참조 없음 | 미사용 |
| `Test/Replays/*` | 16 / 57,503 B | Regress.ps1 글롭, PlayerReplayTest | **보호** |
| `Docs/**` md | 31 / 294,563 B | 문서 | 대상 아님 |
| `Docs/**` 이미지/CSV/json | 67 / 15,093,709 B 중 파일명 참조 없음 20 개 / 3,515,596 B | 아래 목록 | 미사용 후보 (git 복구 가능). `Docs/Wayfinder/Baseline*.txt` 는 보호 |

`Docs/Wayfinder/ref/baseline/*.jpg` 8 개 중 7 개가 파일명으로는 미참조이나 Scoring.md:54 가 폴더 단위 사본으로 기술하고 summary.json 은 videoscore.py 가 참조 -> 폴더 통째로 `불확실` 이다 (아래 20 개에 포함).

| Docs 파일 | 크기 |
|---|---|
| `Docs/Wayfinder/Boss_missile_casing.png` | 74084 B |
| `Docs/Wayfinder/C10_cannon.png` | 651942 B |
| `Docs/Wayfinder/C11_heli_idle.png` | 333713 B |
| `Docs/Wayfinder/C13_title.png` | 22648 B |
| `Docs/Wayfinder/C16_hitbox.png` | 77844 B |
| `Docs/Wayfinder/C18_map.png` | 68241 B |
| `Docs/Wayfinder/C19_motion.png` | 78720 B |
| `Docs/Wayfinder/C20_hitbox.png` | 79345 B |
| `Docs/Wayfinder/C7_bus.png` | 32368 B |
| `Docs/Wayfinder/C_editor.png` | 95440 B |
| `Docs/Wayfinder/H_fall.png` | 14591 B |
| `Docs/Wayfinder/Z_after.png` | 51408 B |
| `Docs/Wayfinder/Z_before.png` | 50982 B |
| `Docs/Wayfinder/ref/baseline/boss_entrance_sheet.jpg` | 258865 B |
| `Docs/Wayfinder/ref/baseline/boss_explosion_sheet.jpg` | 157179 B |
| `Docs/Wayfinder/ref/baseline/camera_slide_sheet.jpg` | 309482 B |
| `Docs/Wayfinder/ref/baseline/cannon_casing_sheet.jpg` | 344160 B |
| `Docs/Wayfinder/ref/baseline/cannon_deploy_sheet.jpg` | 329143 B |
| `Docs/Wayfinder/ref/baseline/cannon_fire_down_sheet.jpg` | 329747 B |
| `Docs/Wayfinder/ref/baseline/shack_destroy_sheet.jpg` | 155694 B |

### Trash/ 분리

| 하위 | tracked 파일 / 크기 | 비고 |
|---|---|---|
| `Trash/Resources` | 1,254 / 827,045,453 B | 옛 캐릭터/적 비트맵. 이 중 ignore(생성물 txt/bmp) 22 개 25,952 B 는 `Resources/Enemy/Alian/output`, `BattleShip/output`, `BattleShip/tes/Output`, `Resources/output` 에 있고 복구 불가 |
| `Trash/AssetsSource` | 192 / 67,057,259 B (gitlink Metal-Slug-Remake 1 포함) | 원본 시트 PNG, sheets.json, GroundEdits.json, SoundMap.json, OriginalCamera.csv, `videoplayback (1)/(2).mp4` (25.4/25.4 MB). **`Scripts/VideoScore/scenes.json:5,14` 가 videoplayback (1)/(2).mp4 를 경로로 참조** -> 이 두 파일은 사용. Docs 가 `Trash/AssetsSource/40204.png` 등을 서술 |
| `Trash/Assets` | 35 / 24,168,853 B | 옛 Assets (Stage0_0.bin 등) |
| `Trash/Unused` | 14 / 2,639,759 B | 이전 정리 복구본. **보호 권장** |
| `Trash/Docs` | 18 / 4,289,843 B | 옛 문서 |
| `Trash/Scripts` | 42 / 127,520 B | 옛 스크립트 |
| `Trash/Source` | 20 / 247,084 B | 옛 Baker 등 |
| `Trash/Projects` `Test` `CMakeLists.txt` `MyMetalSlug2.sln.stale` `gitmodules_empty` | 6+2+1+1+1 / 56,287+33,836+1,532+1,568+0 B | 옛 프로젝트 파일 |
| (untracked) `Trash/AssetsSource/Release 2026.10.10 - 20.46.29.11.mp4` | 1 / 270,542,995 B | 어떤 스크립트/문서도 참조 안 함 (grep 결과 0). 삭제하면 **복구 불가** -> 사용자 확인 필수 |
| (gitlink) `Trash/AssetsSource/Metal-Slug-Remake` | 디스크 192 파일 522,704,457 B | `.gitmodules` 에는 Vendor/JugX 만 있어 이 gitlink 는 등록되지 않은 깨진 서브모듈. 내부 `.git` 존재, 원격 있음. 작업 트리 수정 2 (Knife.h, Menu.h) + untracked `.vs/` 494 MB, `Metal Slug/` 1.1 MB. git status 의 ` m` 이 이것 |

`Trash/` 전체 디스크 합계 = 925,668,994 (tracked) + 270,542,995 (mp4) + 522,704,457 (gitlink 작업 트리, tracked 합계의 gitlink 항목과 별개) + 25,952 (ignored). 어떤 Source/Test/Scripts 도 `Trash/` 아래를 열지 않는다 (VideoScore 의 mp4 2 개 제외).

## 6. 잡파일

- `Trash/MyMetalSlug2.sln.stale` (1,568 B, tracked): 옛 솔루션. 루트 sln 은 `MetalSlug2_Clone.sln` 하나뿐이라 중복 sln 은 없다. 미사용.
- `Trash/gitmodules_empty` (0 B, tracked): 빈 파일. 미사용.
- `Trash/Assets/Ground/Stage0_0.bin`, `Stage0_1.bin`: 옛 Ground 포맷. Trash 안.
- `Projects/MetalSlug2_Clone/MetalSlug2_Clone.vcxproj.user`: ignore (`*.user`), 로컬 디버거 인자 이력.
- `Captures/tmp/Bus.json.mine`: 위 표 참조 (불확실).
- 루트의 tracked 파일은 `.clang-format .gitattributes .gitignore .gitmodules MetalSlug2_Clone.sln OpenInVS.cmd Play.cmd README.md vcpkg.json` 과 Projects/Scripts/Raw 뿐이며 `.bak .orig .tmp .stale`(Trash 밖) 은 없다.
- 작업 트리 변경 중: `Assets/anim.json` (M), `Source/Render/Camera.h` (M), `Vendor/JugX` (m). 이번 조사와 무관.

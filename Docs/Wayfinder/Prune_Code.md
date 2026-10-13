# 코드 미사용 후보 조사 (Prune_Code, 읽기 전용)

조사일 2026-10-13, 기준 HEAD `3790d01` (Simplify W1: remove single-implementation interfaces). 조사 도중 W1 이 들어와 IAudioSink/NullAudio/AudioSink.* 가 사라졌으므로 모든 수치는 그 뒤 작업 트리 기준으로 다시 뽑았다. Source/Assets 수정 없음, 빌드 없음 (`Output/Debug/SlugTest.exe --gtest_list_tests` 로 목록만 확인). 이 파일만 생성했다. 삭제 에이전트가 시작할 때 HEAD 가 달라졌으면 아래 스크립트 방식으로 다시 확인할 것.

## 0. 요약

| 구분 | 항목 수 | 예상 삭제 줄 수 |
|---|---|---|
| 1-A 확정 삭제: 호출/참조 0 인 함수/멤버 (Source·Test 모두 0) | 39 | Source 약 258 |
| 1-B 체인: 요청자가 없어 통째로 죽은 기능 묶음 | 6 (그중 확정 3, 불확실 3) | Source 약 650 (확정 3개만 약 600), Test 약 42 |
| 1-C 필드/상수/enum 값 | 17 | 약 70 |
| 1-D 테스트전용 API (게임 exe 에서 도달 불가, Test 만 참조) | 112 (관측 getter 73, 기능 32, 체인 헬퍼 6, 시임 1) | Source 약 1,025 (삭제하려면 Test 약 600줄 동반 수정/삭제) |
| 2 죽은 테스트/테스트 코드 | 아래 표 | Test 약 200 (확정), 최대 약 600 (1-D 연동) |
| 3 CLI/디버그 기능 | 확정 1 (`--editor-float-test`), 후보 1 (`--expect-finished`) | 약 30 |
| 4 죽은 파일 | Source 0 (모든 .cpp 가 vcxproj 에, 모든 .h 가 include 됨), SlugDef.cpp/.h 사실상 죽음, 문서/스크립트 후보 아래 | 문서 약 1,000, 스크립트 약 500 |
| 5 중복/과잉 | 복붙 함수 10종 22곳 (Test 2종 4곳 별도), FNV 믹스 14파일, 얇은 묶음 구조체 몇 개 | 약 120 |

**확정(1-A + 1-B 확정 3 + 1-C 삭제분) Source 예상 약 928 줄. 1-D 까지 전부면 약 1,900줄 + Test 약 800줄.** 줄 수는 함수 본문 줄 + 선언 1줄 단위의 추정(±20%)이다.

### 보호 (삭제 제외)
- 결정성/해시: `*Hash*`, `*Determin*`, `Replay.*`, `SimRunner.*`, `PlayerDeterminism.*`, `PlayerReplay.RegressionReplayPlaysTheSameWayTwiceAndHitsTheMilestones`. 이들이 읽는 `Test/Replays/*` 16개 (`*.replay` 15, `sandbox_regression.rep` 1), `Docs/Wayfinder/Baseline*.txt`.
- 에셋 정합성: `AnimIntegrity.*`, `*DefTest.cpp` (`EnemyDefs`, `VehicleDefs`, `PropDef`, `BusData`), `StageData*`, `DataMigrationA` (Def 값 고정, 2번 표에서 불확실로 표기).
- CLI: `--gen-replay` (사용자 지시 보호. 단 현재 Scripts/Test/Docs 어디에도 호출이 없다, 3번 참고), `--dump-tuning` (Verify.ps1), `--trace-load`/`--no-async-load` (TraceLoad.ps1), `--event-log`/`--capture-ticks` (VideoScore).
- 스크립트/문서: `Scripts/Verify|Regress|TraceLoad|Build.ps1`, `Docs/Architecture.md`, `Docs/CodingStyle.md`, `Stage2_Plan.md`, `StyleRubric.md`, `Docs/Wayfinder/Baseline*.txt`.
- 에셋 연동: AudioSystem.h 의 `kAll` 25개 상수 (AudioTest + Prune_Assets.md 가 보호).
- 사용자 손편집 데이터 (`Assets/**`, `Tuning`, `Defs`)는 이 문서의 삭제 대상이 아니다. 이 조사는 코드 위주.

### 불확실 (사용자/삭제 에이전트 판단 필요)
1. SlugDef 로더 (Slug.json 이 한 번도 없었음, 계획 여부).
2. SendCommand/onCommand 와 `command` 액션 (스테이지 JSON 포맷 문서에 있는 기능).
3. eHitKind::Hazard (테스트가 생산자).
4. eSlugClip Elev/LowAccel*, eRigClip::JumpLookDownDiag2, eFontColor::YellowWhite (anim.json/스프라이트 에셋과 연동).
5. SpriteBatch::LoadAtlasPage(s) 순차 로더 (오류 경로 테스트 치환).
6. 1-D 전체: 관측 getter 를 지우려면 테스트가 어떻게 상태를 보는지 다시 설계해야 한다.
7. `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` (이미 실패 중, 원인이 에셋, 2번 참고).
8. Docs 의 NodeSpec/PlayerSpec, AuditCoords 일체, VideoScore 일체 (4번).
9. 이번 조사 도구가 이름 단위(오버로드/동명 클래스 병합) 호출 그래프라서, 같은 이름의 다른 클래스 메서드는 한꺼번에 살았다고 본다 (예: `GetUi` 5개 중 Test 가 부르는 건 TitleState::GetUi 하나, 나머지 4개는 호출 0). 이는 "죽었다" 판정을 보수적으로 만들 뿐 틀리게 만들지는 않는다.

---

## 1. Source 도달 불가 코드

### 방법
- (a) 헤더에 나오는 모든 식별자의 Source 토큰 수를 센다 (주석 제거 후). 2 이하는 선언+정의뿐인 것 (`s<=2` 목록 375개 후보).
- (b) `.cpp` 함수 본문 단위 호출 그래프를 만들고 `main`, 가상 함수/override, 파일 스코프 표(`BehaviorOps`, `EnumEntry`), 헤더 인라인 본문을 루트로 삼아 도달 불가를 뽑았다 (169건 -> 사람이 걸러 아래 표).
- 문자열 간접 사용(JSON `behavior` -> enum, StateFactory 이름, `{side}`)은 죽은 코드로 보지 않았다. 에디터 UI 에서 호출되는 함수는 살아 있다. 가상 override 는 루트로 처리.
- 표 컬럼: Source 참조는 "정의/선언 제외 토큰 수", Test 참조는 `Test/**` 토큰 수.

### 1-A. 확정 삭제: Source 호출 0, Test 참조 0

| 심볼 | 정의 위치 | 헤더 선언 | 줄 | Source 참조 | Test 참조 | 판단 / 메모 |
|---|---|---|---|---|---|---|
| `InputHistory::GetAgo_` | Core/Input.cpp:97 | Input.h:71 | 6 | 0 | 0 | 삭제 / private 인데 호출 0 |
| `SlugVehicle::ForceHp` | Game/Player/SlugVehicle.cpp:922 | SlugVehicle.h:87 | 6 | 0 | 0 | 삭제 |
| `SlugVehicle::GetCannonCooldown` | Game/Player/SlugVehicle.cpp:1004 | SlugVehicle.h:104 | 5 | 0 | 0 | 삭제 |
| `SlugVehicle::GetCrouchLevel` | Game/Player/SlugVehicle.cpp:1042 | SlugVehicle.h:110 | 5 | 0 | 0 | 삭제 |
| `SlugVehicle::GetMaxHp` | Game/Player/SlugVehicle.cpp:989 | SlugVehicle.h:101 | 5 | 0 | 0 | 삭제 |
| `SlugVehicle::GetOwnerId` | Game/Player/SlugVehicle.cpp:943 | SlugVehicle.h:92 | 5 | 0 | 0 | 삭제 |
| `SlugVehicle::GetVulcanTargetAngle` | Game/Player/SlugVehicle.cpp:1014 | SlugVehicle.h:106 | 5 | 0 | 0 | 삭제 |
| `SlugVehicle::IsDriven` | Game/Player/SlugVehicle.cpp:933 | SlugVehicle.h:90 | 5 | 0 | 0 | 삭제 |
| `SlugSystem::GetRiddenIndex` | Game/Player/SlugSystem.cpp:125 | SlugSystem.h:33 | 5 | 0 | 0 | 삭제 |
| `SandboxState::GetSlugs (const/비const 2개)` | Game/SandboxState.cpp:1150,1155 | SandboxState.h:78-79 | 10 | 0 | 0 | 삭제 / Test 0 |
| `PropWorld::GetDecor` | Game/Props/PropWorld.cpp:966 | PropWorld.h:190 | 5 | 0 | 0 | 삭제 |
| `EnemyDefLibrary::GetDefaultPath` | Game/Enemies/EnemyDefLoader.cpp:1584 | EnemyDef.h:834 | 6 | 0 | 0 | 삭제 |
| `GetEntityKindName` | Game/Stage/StageData.cpp:432 | StageData.h:429 | 15 | 0 | 0 | 삭제 / switch 14줄 |
| `EnemyWorld::GetHealth` | Game/Enemies/EnemyWorld.cpp:1011 | EnemyWorld.h:68 | 7 | 0 | 0 | 삭제 |
| `ParamValue::GetKeys` | Game/Stage/StageData.cpp:158 | StageData.h:54 | 5 | 0 | 0 | 삭제 |
| `ParamValue::GetType` | Game/Stage/StageData.cpp:84 | StageData.h:33 | 5 | 0 | 0 | 삭제 |
| `VoiceAllocator::GetMaxPerSound` | Audio/VoiceAllocator.cpp:62 | VoiceAllocator.h:35 | 5 | 0 | 0 | 삭제 |
| `StageState::GetRescuedPows` | Game/Stage/StageState.cpp:124 | StageState.h:79 | 5 | 0 | 0 | 삭제 |
| `EnemyLab::IsPaused` | Editor/EnemyLab.cpp:265 | EnemyLab.h:45 | 5 | 0 | 0 | 삭제 / SetPaused 는 EnemyLabTest 3줄 사용 |
| `EnemyWorld::RequestCameraShake` | Game/Enemies/EnemyWorld.cpp:993 | EnemyWorld.h:89 | 8 | 0 | 0 | 삭제 / 1-B 카메라 흔들림 체인의 시작점 |
| `EnemyWorld::RequestPlayerMinX` | Game/Enemies/EnemyWorld.cpp:1038 | EnemyWorld.h:70 | 7 | 0 | 0 | 삭제 / 1-B 플레이어 X 제한 체인의 시작점 |
| `EnemyWorld::RequestPlayerMaxX` | Game/Enemies/EnemyWorld.cpp:1045 | EnemyWorld.h:73 | 7 | 0 | 0 | 삭제 / 1-B 동일 |
| `StageWorldHost::FindRouteMut_` | Game/Stage/StageWorldHost.cpp:337 | StageWorldHost.h:126 | 13 | 0 | 0 | 삭제 |
| `ParseSlugTuning` | Game/Player/SlugDef.cpp:344 | SlugDef.h:7 | 11 | 0 | 0 | 삭제 / 1-B SlugDef 체인 |
| `DumpSlugTuningJson` | Game/Player/SlugDef.cpp:368 | SlugDef.h:11 | 9 | 0 | 0 | 삭제 / 1-B 동일 |
| `VulcanAngleFromIndex` | Game/Player/SlugClips.cpp:58 | SlugClips.h:53 | 6 | 0 | 0 | 삭제 |
| `DefReader::WeaponOpt` | Game/Enemies/EnemyDef.cpp:408 | EnemyDefParse.h:43 | 6 | 0 | 0 | 삭제 |
| `CombatSystem::GetEvents` | Game/Combat/CombatSystem.cpp:75 | CombatSystem.h:26 | 5 | 0 | 0 | 삭제 / W1(3790d01) 에서 TestAudio 가 바뀌며 Test 참조 11 -> 0. m_events 멤버(CombatSystem.h:33, .cpp:9,14-15,77)도 반환용 복사본이라 같이 정리 가능 |
| `VehicleCommon 의 ScreenToWorld(EnemyEnv&, Vec2)` | Game/Enemies/VehicleCommon.cpp:54 | VehicleCommon.h:22 | 8 | 0 | 0 | 삭제 / 같은 이름 Camera::ScreenToWorld 는 별개(1-C) |
| `EnemyWorld::ForEachAlive (템플릿)` | (헤더 인라인) | EnemyWorld.h:93 | 13 | 0 | 0 | 삭제 / src 토큰 1, 호출 0 |
| `PropWorld::StartHit_` | (정의 없음) | PropWorld.h:214 | 1 | 0 | 0 | 삭제 / 선언만 있고 정의/호출 모두 없다 |
| `IStageHost::PlayerAlive (가상)` | Game/Stage/StageWorldHost.cpp:632 | StageHost.h:80, StageWorldHost.h:106 | 8 | 0 | 0 | 삭제 / 호출 0. Test 의 FakeStageHost::PlayerAlive(StageTestUtil.cpp:78) 와 bPlayerAlive(.h:45) 도 같이 정리 |
| `SlugVehicle::GetHp` | Game/Player/SlugVehicle.cpp:984 | SlugVehicle.h:100 | 5 | 0 | 0 | 삭제 / 같은 이름 PropWorld::GetHp 는 PropWorldTest 가 사용 (1-D) |
| `FadeController::GetPhase` | Game/Ui/FadeController.cpp:88 | FadeController.h:24 | 5 | 0 | 0 | 삭제 / 테스트의 flow.GetPhase() 는 ScreenFlow 것 |
| `SandboxState::GetUi` | Game/SandboxState.cpp:1125 | SandboxState.h:73 | 5 | 0 | 0 | 삭제 / TitleState::GetUi 만 UiTest 가 사용 (1-D) |
| `StageState::GetUi (const/비const 2개)` | Game/Stage/StageState.cpp:894,899 | StageState.h:73-74 | 10 | 0 | 0 | 삭제 |
| `UiDemoState::GetUi` | Game/Ui/UiDemoState.cpp:77 | UiDemoState.h:16 | 5 | 0 | 0 | 삭제 |
| `Renderer::IsInitialized` | Render/Renderer.cpp:141 | Renderer.h:33 | 6 | 0 | 0 | 삭제 / AudioSystem::IsInitialized 만 AudioTest 가 사용 (1-D) |
| `Editor::IsInitialized` | Editor/Editor.cpp:124 | Editor.h:40 | 5 | 0 | 0 | 삭제 |

합계 39개 항목 약 258 줄. 위 표의 `Request*`, `ParseSlugTuning`, `DumpSlugTuningJson` 은 아래 1-B 체인과 함께 지운다.

### 1-B. 체인 (요청자 없음 -> 소비자 전체가 죽음)

| 묶음 | 구성과 위치 | 근거 (grep) | 예상 줄 | 판단 |
|---|---|---|---|---|
| 플레이어 X 제한 | EnemyWorld::RequestPlayerMinX/MaxX (호출 0) -> HasPlayerMinX/GetPlayerMinX/HasPlayerMaxX/GetPlayerMaxX (EnemyWorld.cpp:1304-1322) -> PlayerController::PushFeetX/PushFeetXMax (.cpp:1098-1121), SlugVehicle::PushFeetX/PushFeetXMax (.cpp:896-920) -> 호출부 SandboxState.cpp:611-625, StageState.cpp:427-435. 멤버 m_playerMinX/MaxX, m_bPlayerMinX/MaxX (EnemyWorld.h:184-187, .cpp:113-114, 397) | Request* 가 어디서도 불리지 않아 Has* 는 항상 false, 호출부 if 블록은 한 번도 실행되지 않는다. Test 참조 0 (PushFeetX 계열 포함) | 약 105 | 삭제 |
| SlugDef 로더 | SlugDef.cpp 전체(381줄) + SlugDef.h(14줄): ParseSlugTuning/LoadSlugTuning/DumpSlugTuningJson/GetDefaultSlugTuningPath, SlugDefReadVisitor, JsonWriter. 호출부 SandboxState.cpp:113-129, SandboxConfig::bLoadSlugFile/slugTuning (SandboxState.h:48-49) | 로더가 읽는 `Assets/Defs/Slug/Slug.json` 은 디스크에도 없고 `git log --all -- Assets/Defs/Slug/Slug.json` 도 0건 (한 번도 만든 적 없음). SandboxState 는 `std::filesystem::exists` 가 항상 false 라 기본값만 쓴다. LoadSlugTuning 도 Test 0. bLoadSlugFile/slugTuning 은 StateFactory/Test 어디서도 설정 안 함 (Test 0) | 약 420 (cpp+h 395, 호출부/설정 25) + vcxproj 2행 | 삭제. 단 사용자가 Slug.json 을 만들 계획이면 보류 (불확실) |
| 카메라 흔들림 | EnemyWorld::RequestCameraShake (호출 0) -> TakeCameraShake (EnemyWorld.cpp:1001, StageState.cpp:462-469 블록) -> Camera::Shake/StopShake (Camera.cpp:591-610), Update 안 흔들림 분기 (Camera.cpp:567-580), NextShakeUnit_ (583-589), 멤버 m_shake* 5개 (Camera.h:122-126), GetPosition 의 m_shakeOffset 분기 (Camera.cpp:618-626) | 흔들림 요청자가 없다 (BigShiee.cpp 에서 호출 제거됨). GetPosition == GetPositionNoShake 가 항상 성립. 카메라 HashState 는 shake 필드를 섞지 않아 해시 불변. Test: Camera.ShakeIsDeterministicDecaysAndStaysInBounds(CameraTest.cpp:319, 18줄) + RunShake_(CameraTest.cpp:41-56) + Camera.StopShakeRestoresImmediately(:337, 11줄) | Source 약 75, Test 약 42 | 삭제 (GetPosition/GetPositionNoShake 통합 포함). Camera.h 는 현재 `M` (사용자 수정 중) 이므로 삭제 에이전트가 충돌 주의 |
| SpriteBatch 순차 로더 | SpriteBatch::LoadAtlasPages (SpriteBatch.cpp:696) -> LoadAtlasPage (:501) | S3 이후 App 은 LoadAtlasPagesParallel 만 쓴다. LoadAtlasPages 호출은 Test 1(PlayerWeaponTest), LoadAtlasPage 직접 호출은 Test 1(SpriteBatchTest.cpp:434, 없는 파일 오류 확인) | 약 48 (LoadAtlasPage 는 Parallel 이 DecodeImages+Add 로 쓰는지 확인 필요) | 삭제 후 테스트는 Parallel(1) 로 치환 (불확실: LoadAtlasPage 의 오류 경로가 Parallel 에도 같은지) |
| SendCommand / onCommand | BehaviorOps::onCommand 훅 (EnemyBehaviors.h:66) 은 14개 Ops 어디서도 지정 안 됨 -> EnemyWorld::SendCommand (EnemyWorld.cpp:946-963) 는 항상 무동작 -> StageWorldHost::SendCommand (:578), IStageHost::SendCommand (StageHost.h:74), StageRunner.cpp:541, 액션 eActionType::Command (StageData.cpp:462,565,2032,2284) | Assets/Stage/*.json 에 `"command"` 0건. Test 는 FakeStageHost::SendCommand 로 호출 여부만 확인 (StageRunnerTest). 스테이지 포맷(Docs/StageFormat.md) 의 일부라서 포맷 변경이 됨 | 약 45 | 불확실 (데이터 포맷 제거 결정 필요) |
| eHitKind::Hazard | 생산자 0: Source 어디서도 Hazard 히트를 만들지 않는다. SlugVehicle.cpp:718 case 와 SlugTuning::hazardDamage (SlugTypes.h:109, SlugDef.cpp:83) 만 남음. Test 5곳이 직접 생성 (CombatTest 2, PlayerCombatTest 2, PropWorldTest 1) | enum 값을 지우면 Combat 테스트 수정이 필요하고 HitKindMask::All 의 비트 수가 바뀐다 | 약 5 + 테스트 | 불확실 |

추가 체인 (작음):
- `CombatSystem::m_events`(CombatSystem.h:33): Resolve 결과 복사본이 GetEvents(1-A) 전용.
- `FindSpawnIds`/`WasSpawnSkipped` 등 StageRunner 관측 함수는 1-D.

### 1-C. 필드 / 상수 / enum 값

| 심볼 | 위치 | 근거 | 판단 |
|---|---|---|---|
| `PlayerInput::bJumpHeld` | Game/Player/PlayerInput.h:11 | 쓰기만 (PlayerInput.cpp:50), 읽기 0 | 삭제 |
| `PlayerRig::m_lowerSlot, m_upperSlot` | Game/Player/PlayerRig.h:128-129 | 쓰기만 (PlayerRig.cpp:375,391), 읽기 0 | 삭제 |
| `PlayerController::m_spawnFeet` | Game/Player/PlayerController.h:136 | 쓰기만 (PlayerController.cpp:84) | 삭제 |
| `TrainingTargets 더미 lastDamage` | Game/Sandbox/TrainingTargets.h:34 | 쓰기만 (TrainingTargets.cpp:183) | 삭제 |
| `EnemyEvent::defName` | Game/Enemies/EnemyTypes.h:69 | 쓰기만 (EnemyWorld.cpp:1281), 읽기 0 | 삭제 |
| `BannerSpec::maxScale` | Game/Ui/Banner.h:15 | 선언 외 참조 0 (UiLayout::bannerMaxScale 와 별개) | 삭제 |
| `SlugVehicle stats boards, emptyClicks` | Game/Player/SlugVehicle.h:62,65 | 쓰기만 (SlugVehicle.cpp:163, 542) | 삭제 |
| `UiLayout glyphSize, resultsFadeTicks, titleFadeTicks, pressStartCenterX, pressStartCenterY` | Game/Ui/UiLayout.h:9,58,60,61,84 | Source 참조 = 선언 1, Test 0 | 삭제 |
| `UiLayout bannerCenterY/MaxScale/Margin/LineGap, pressStartBlinkTicks, timeGlyphW/H, titleNameY` | Game/Ui/UiLayout.h:10,11,62,75-78,80 | Source 읽기 0 (선언만), Test 각 1 (UiTest 가 기본값 비교) | 테스트전용 |
| `RGBA::kWhite/kBlue/kYellow/kCyan/kMagenta` | Base/RGBA.h:23,26-29 (+정의 39-45) | 선언+정의 외 참조 0, Test 0. (kBlack/kRed/kGreen 은 SpriteBatchTest 만 사용) | 삭제 |
| `Vec2::kOne/kRight/kUp/kUnitX/kUnitY` | Base/Math.h:113-117 (+정의 124-128) | 선언+정의 외 참조 0, Test 0 | 삭제 |
| `kEpsilon` | Base/Math.h:13 | 참조 0 | 삭제 |
| `eError::SdlInit` | Base/SlugError.h:6 (+SlugError.cpp:14 case) | 생산자 0 (어디서도 반환 안 함) | 삭제 (Test 참조 0) |
| `eSlugClip::Elev, LowAccelFwd, LowAccelBack` | Game/Player/SlugClips.h:14,20,21 (+SlugClips.cpp:15,21,22 표 행) | enum 값 참조 0. 표 행은 enum 순서와 1:1 이라 enum+행을 같이 지워야 한다. anim.json 에 Slug_Elev/Slug_LowAccel* 클립 3개가 남는다 | 불확실 (anim.json 은 사용자 손편집, SlugClips 전수 순회 테스트 확인 필요) |
| `eRigClip::JumpLookDownDiag2` | Game/Player/PlayerRig.h:36 (+PlayerRig.cpp:44 표 행) | 선택 코드 0 (Diag1 만 쓰임) | 불확실 (PlayerRig 는 필수 슬롯 누락 시 WARN, anim 클립 Player_Jump_Lookingdown_2_* 는 에셋 판정 대상) |
| `eFontColor::YellowWhite` | Game/Ui/BitmapFont.h (case: BitmapFont.cpp:39,53) | 생산자 0 | 불확실 (스프라이트 Font_Yellow_White 는 UiTest FontStrips 가 요구할 수 있음) |
| `AudioSink 상수 kReload, kVoiceEnemyCh, kLetsGo, kHereWeGo, kMenuSelect` | Audio/AudioSystem.h (W1 로 AudioSink.h 에서 이동) | Source 참조 0. kAll 배열(AudioTest 가 sounds.json 에 이름이 있는지 순회)에만 등장 | 보호 (Prune_Assets.md 가 kAll 25개를 보호 목록에 둠) |

### 1-D. 테스트전용 API (게임 exe 에서 도달 불가, `Test/**` 만 참조)

정의/선언만 Source 에 있고 호출은 테스트뿐이다. "관측 getter" 는 지우면 테스트가 그 상태를 못 봐서 테스트를 같이 고쳐야 한다. "기능" 은 기능 자체가 게임에서 안 쓰이므로 기능+테스트를 같이 지운다. 이 표의 삭제 여부는 "결과 불변" 요건과 별개로 **사용자의 의도** (테스트 시임을 남길지)가 필요하다. 표 컬럼: Source 줄(정의+선언), Source 참조 = 정의/선언 외 토큰(체인 포함 3+면 호출자 열 참고), Test 참조 = 테스트 토큰 수.

| 심볼 | 위치 | 줄 | Source 토큰 | Test 토큰 | 종류 | 호출자(체인) |
|---|---|---|---|---|---|---|
| `AudioSystem::IsInitialized` | Audio/AudioSystem.cpp:485 | 5 | 6 | 7 | 관측 getter |  |
| `AudioSystem::IsBgmFadingOut` | Audio/AudioSystem.cpp:490 | 5 | 2 | 7 | 관측 getter |  |
| `AudioSystem::GetCurrentBgm` | Audio/AudioSystem.cpp:495 | 5 | 2 | 5 | 관측 getter |  |
| `VoiceAllocator::GetSoundId` | Audio/VoiceAllocator.cpp:73 | 6 | 2 | 1 | 관측 getter |  |
| `VoiceAllocator::GetActiveCount` | Audio/VoiceAllocator.cpp:79 | 10 | 2 | 6 | 관측 getter |  |
| `HasErrorCode` | Base/SlugError.cpp:30 | 11 | 2 | 3 | 관측 getter |  |
| `TuningWatcher::GetReloadCount` | Core/Tuning.cpp:532 | 5 | 2 | 3 | 관측 getter |  |
| `AnimEditor::GetDocument` | Editor/AnimEditor.cpp:366 | 5 | 2 | 7 | 관측 getter |  |
| `AnimEditor::GetStatus` | Editor/AnimEditor.cpp:371 | 5 | 2 | 2 | 관측 getter |  |
| `EnemyLab::GetSetCount` | Editor/EnemyLab.cpp:254 | 5 | 2 | 3 | 관측 getter |  |
| `ProjectileSystem::CountShots` | Game/Combat/Projectiles.cpp:569 | 14 | 2 | 7 | 관측 getter |  |
| `ProjectileSystem::GetBlasts` | Game/Combat/Projectiles.cpp:607 | 5 | 2 | 2 | 관측 getter |  |
| `EnemyWorld::IsTagDefeated` | Game/Enemies/EnemyWorld.cpp:1052 | 13 | 2 | 2 | 관측 getter |  |
| `EnemyWorld::GetEffects`, `EffectList::GetEffects` | Game/Enemies/EnemyWorld.cpp:1335 | 5 | 4 | 13 | 관측 getter |  |
| `PlayerController::IsKnifeInProgress` | Game/Player/PlayerController.cpp:1227 | 5 | 2 | 1 | 관측 getter |  |
| `PlayerController::GetChutePhase` | Game/Player/PlayerController.cpp:163 | 5 | 2 | 9 | 관측 getter |  |
| `GetRigClipName` | Game/Player/PlayerRig.cpp:128 | 7 | 2 | 2 | 관측 getter |  |
| `PlayerRig::IsSlotRequired` | Game/Player/PlayerRig.cpp:137 | 6 | 2 | 1 | 관측 getter |  |
| `PlayerRig::GetSlotClip` | Game/Player/PlayerRig.cpp:251 | 8 | 2 | 3 | 관측 getter |  |
| `PlayerRig::GetUpperClip` | Game/Player/PlayerRig.cpp:444 | 5 | 2 | 23 | 관측 getter |  |
| `PlayerRig::GetLowerClip` | Game/Player/PlayerRig.cpp:449 | 5 | 2 | 7 | 관측 getter |  |
| `PlayerRig::GetUpperFrame` | Game/Player/PlayerRig.cpp:454 | 5 | 2 | 4 | 관측 getter |  |
| `PlayerRig::GetLowerFrame` | Game/Player/PlayerRig.cpp:459 | 5 | 2 | 3 | 관측 getter |  |
| `PlayerRig::GetOriginLift` | Game/Player/PlayerRig.cpp:464 | 5 | 2 | 1 | 관측 getter |  |
| `PropWorld::GetHp` | Game/Props/PropWorld.cpp:151 | 5 | 4 | 6 | 관측 getter |  |
| `ItemWorld::GetPopups` | Game/Props/ItemWorld.cpp:479 | 5 | 2 | 5 | 관측 getter |  |
| `PropWorld::GetDebris` | Game/Props/PropWorld.cpp:939 | 5 | 2 | 4 | 관측 getter |  |
| `TrainingTargets::GetKillCount` | Game/Sandbox/TrainingTargets.cpp:251 | 5 | 2 | 2 | 관측 getter |  |
| `SandboxState::GetTargets` | Game/SandboxState.cpp:1085 | 5 | 2 | 5 | 관측 getter |  |
| `SandboxState::GetRig` | Game/SandboxState.cpp:1110 | 5 | 4 | 38 | 관측 getter |  |
| `TitleState::GetUi` | Game/TitleState.cpp:157 | 5 | 10 | 3 | 관측 getter |  |
| `ParamValue::GetInt` | Game/Stage/StageData.cpp:194 | 8 | 2 | 2 | 관측 getter |  |
| `ParamValue::GetVec2` | Game/Stage/StageData.cpp:210 | 13 | 2 | 4 | 관측 getter |  |
| `StageData::CountSpawns` | Game/Stage/StageData.cpp:400 | 10 | 2 | 2 | 관측 getter |  |
| `StageData::CountWaveEntries` | Game/Stage/StageData.cpp:412 | 10 | 2 | 1 | 관측 getter |  |
| `StageData::CountGates` | Game/Stage/StageData.cpp:422 | 10 | 2 | 2 | 관측 getter |  |
| `StageRunner::GetGroundName` | Game/Stage/StageRunner.cpp:122 | 5 | 2 | 3 | 관측 getter |  |
| `StageRunner::GetCameraOffset` | Game/Stage/StageRunner.cpp:132 | 5 | 2 | 6 | 관측 getter |  |
| `StageRunner::GetBaseBounds` | Game/Stage/StageRunner.cpp:137 | 5 | 2 | 17 | 관측 getter |  |
| `StageRunner::IsCurveActive` | Game/Stage/StageRunner.cpp:164 | 6 | 2 | 3 | 관측 getter |  |
| `StageRunner::WasSpawnSkipped` | Game/Stage/StageRunner.cpp:667 | 7 | 2 | 2 | 관측 getter |  |
| `StageRunner::GetPendingActionCount` | Game/Stage/StageRunner.cpp:900 | 10 | 2 | 2 | 관측 getter |  |
| `StageRunner::GetPendingWaveCount` | Game/Stage/StageRunner.cpp:910 | 10 | 2 | 6 | 관측 getter |  |
| `StageState::IsLoaded` | Game/Stage/StageState.cpp:1289 | 5 | 2 | 72 | 관측 getter |  |
| `StageState::GetData` | Game/Stage/StageState.cpp:1294 | 5 | 2 | 12 | 관측 getter |  |
| `StageState::GetRunner` | Game/Stage/StageState.cpp:1299 | 5 | 2 | 23 | 관측 getter |  |
| `StageState::GetHost` | Game/Stage/StageState.cpp:1304 | 5 | 2 | 6 | 관측 getter |  |
| `TitleState::IsStarting` | Game/TitleState.cpp:142 | 5 | 2 | 6 | 관측 getter |  |
| `TitleState::GetSelectedStateName` | Game/TitleState.cpp:152 | 5 | 2 | 2 | 관측 getter |  |
| `BannerSystem::GetQueuedCount` | Game/Ui/Banner.cpp:227 | 5 | 2 | 2 | 관측 getter |  |
| `BitmapFont::HasSprites`, `UiSystem::HasSprites` | Game/Ui/BitmapFont.cpp:303 | 5 | 4 | 3 | 관측 getter |  |
| `HasGlyph` | Game/Ui/BitmapFont.cpp:327 | 6 | 2 | 1 | 관측 getter |  |
| `FadeController::IsClear` | Game/Ui/FadeController.cpp:103 | 5 | 2 | 5 | 관측 getter |  |
| `ScreenFlow::GetPhase` | Game/Ui/ScreenFlow.cpp:259 | 5 | 4 | 10 | 관측 getter |  |
| `FadeController::IsBlack` | Game/Ui/FadeController.cpp:98 | 5 | 2 | 7 | 관측 getter |  |
| `DisplayTimer::GetRemainingTicks` | Game/Ui/Hud.cpp:213 | 5 | 2 | 1 | 관측 getter |  |
| `AnimPlayer::GetNormalizedTime` | Render/Animation.cpp:425 | 11 | 2 | 3 | 관측 getter |  |
| `AnimPlayer::GetFrame` | Render/Animation.cpp:505 | 5 | 4 | 23 | 관측 getter | PlayerRig::GetLowerFrame,PlayerRig::GetUpperFrame |
| `AnimPlayer::GetTimeUnits` | Render/Animation.cpp:510 | 5 | 2 | 2 | 관측 getter |  |
| `Camera::GetViewRect` | Render/Camera.cpp:647 | 5 | 3 | 2 | 관측 getter | Camera::IsVisible |
| `Camera::IsVisible` | Render/Camera.cpp:666 | 7 | 2 | 4 | 관측 getter |  |
| `Camera::IsLocked` | Render/Camera.cpp:678 | 5 | 2 | 16 | 관측 getter |  |
| `Camera::IsForwardOnly` | Render/Camera.cpp:689 | 5 | 2 | 1 | 관측 getter |  |
| `Camera::IsForwardOnlyY` | Render/Camera.cpp:700 | 5 | 2 | 2 | 관측 getter |  |
| `Camera::GetLockBounds` | Render/Camera.cpp:720 | 5 | 2 | 6 | 관측 getter |  |
| `Camera::HasScrollFloor` | Render/Camera.cpp:736 | 5 | 2 | 2 | 관측 getter |  |
| `Camera::GetScrollFloorX` | Render/Camera.cpp:741 | 5 | 2 | 2 | 관측 getter |  |
| `SpriteBatch::GetAtlasPageCount` | Render/SpriteBatch.cpp:908 | 5 | 2 | 5 | 관측 getter |  |
| `SpriteBatch::GetPendingCommands` | Render/SpriteBatch.cpp:918 | 5 | 2 | 21 | 관측 getter |  |
| `HitResolver::GetTrackedHitCount` | World/Combat.cpp:118 | 5 | 2 | 3 | 관측 getter |  |
| `Health::GetRatio` | World/Combat.cpp:190 | 5 | 2 | 1 | 관측 getter |  |
| `Aabb::GetBottomCenter` | World/Geometry.cpp:73 | 5 | 2 | 1 | 관측 getter |  |
| `GroundMap::HasPeriodicTail` | World/GroundMap.cpp:351 | 5 | 2 | 1 | 관측 getter |  |
| `VoiceAllocator::ReleaseAll` | Audio/VoiceAllocator.cpp:49 | 8 | 2 | 11 | 기능 |  |
| `ParseTuning` | Core/Tuning.cpp:415 | 11 | 2 | 10 | 기능 |  |
| `EnemyLab::SetPaused` | Editor/EnemyLab.cpp:259 | 6 | 2 | 3 | 기능 |  |
| `EnemyDefLibrary::FindProjectile` | Game/Enemies/EnemyDefLoader.cpp:1617 | 7 | 2 | 10 | 기능 |  |
| `EnemyWorld::CancelPending` | Game/Enemies/EnemyWorld.cpp:1189 | 19 | 2 | 1 | 기능 |  |
| `EnemyWorld::ReseedRng` | Game/Enemies/EnemyWorld.cpp:1292 | 6 | 2 | 3 | 기능 |  |
| `PlayerController::SetVictory` | Game/Player/PlayerController.cpp:221 | 22 | 2 | 1 | 기능 |  |
| `PropDefLibrary::FindItem` | Game/Props/PropDef.cpp:1030 | 7 | 2 | 13 | 기능 |  |
| `PropDefLibrary::ParseSpawnParams` | Game/Props/PropDef.cpp:1051 | 23 | 2 | 4 | 기능 |  |
| `PropDefLibrary::FindMissingAsset` | Game/Props/PropDef.cpp:1074 | 56 | 2 | 1 | 기능 |  |
| `PropWorld::ClearByLayer` | Game/Props/PropWorld.cpp:86 | 7 | 2 | 2 | 기능 |  |
| `SandboxState::AddTarget` | Game/SandboxState.cpp:342 | 8 | 2 | 36 | 기능 |  |
| `StageRunner::FindSpawnIds` | Game/Stage/StageRunner.cpp:660 | 7 | 2 | 14 | 기능 |  |
| `BannerSystem::Enqueue` | Game/Ui/Banner.cpp:159 | 11 | 2 | 2 | 기능 |  |
| `FitTextScale` | Game/Ui/BitmapFont.cpp:151 | 23 | 2 | 6 | 기능 |  |
| `MeasureTextWidth` | Game/Ui/BitmapFont.cpp:59 | 13 | 2 | 1 | 기능 |  |
| `MeasureTextHeight` | Game/Ui/BitmapFont.cpp:74 | 9 | 2 | 3 | 기능 |  |
| `Camera::WorldToScreen` | Render/Camera.cpp:652 | 7 | 2 | 1 | 기능 |  |
| `SpriteBatch::LoadAtlasPage` | Render/SpriteBatch.cpp:501 | 17 | 3 | 1 | 기능 | SpriteBatch::LoadAtlasPages |
| `SpriteBatch::LoadAtlasPages` | Render/SpriteBatch.cpp:696 | 16 | 2 | 1 | 기능 |  |
| `Health::Heal` | World/Combat.cpp:207 | 13 | 2 | 3 | 기능 |  |
| `Aabb::Intersection` | World/Geometry.cpp:1 | 15 | 2 | 2 | 기능 |  |
| `Aabb::Translated` | World/Geometry.cpp:101 | 9 | 2 | 1 | 기능 |  |
| `Aabb::Union` | World/Geometry.cpp:110 | 9 | 2 | 1 | 기능 |  |
| `Aabb::MirroredX` | World/Geometry.cpp:119 | 9 | 2 | 1 | 기능 |  |
| `Aabb::Expanded` | World/Geometry.cpp:83 | 9 | 5 | 1 | 기능 | Camera::IsVisible |
| `GroundMap::MakeFromBits` | World/GroundMap.cpp:16 | 16 | 2 | 1 | 기능 |  |
| `GroundMap::FindCeilingY` | World/GroundMap.cpp:252 | 21 | 2 | 2 | 기능 |  |
| `GroundMap::FindSurfaceUp` | World/GroundMap.cpp:273 | 26 | 2 | 5 | 기능 |  |
| `GroundMap::SetBoundsPolicy` | World/GroundMap.cpp:345 | 6 | 2 | 2 | 기능 |  |
| `GroundMap::MakeFromRows` | World/GroundMap.cpp:48 | 24 | 2 | 2 | 기능 |  |
| `MissingAsset_` | Game/Props/PropDef.cpp:60 | 17 | 6 | 0 | 체인 헬퍼 | MissingBurstAsset_,PropDefLibrary::FindMissingAsset |
| `MissingBurstAsset_` | Game/Props/PropDef.cpp:77 | 25 | 4 | 0 | 체인 헬퍼 | PropDefLibrary::FindMissingAsset |
| `CountSpawnsIn_` | Game/Stage/StageData.cpp:347 | 18 | 3 | 0 | 체인 헬퍼 | StageData::CountSpawns |
| `CountWaveEntriesIn_` | Game/Stage/StageData.cpp:365 | 21 | 3 | 0 | 체인 헬퍼 | StageData::CountWaveEntries |
| `CountGatesIn_` | Game/Stage/StageData.cpp:386 | 15 | 3 | 0 | 체인 헬퍼 | StageData::CountGates |
| `SplitLineLengths_` | Game/Ui/BitmapFont.cpp:11 | 21 | 4 | 0 | 체인 헬퍼 | FitTextScale,MeasureTextHeight,MeasureTextWidth |
| `SoundBank/AnimationLibrary/SpriteLibrary::LoadFromJson` | Audio/SoundBank.cpp:52 | 12 | 6 | 25 | 테스트 시임 |  |
| `Camera::ScreenToWorld` | Render/Camera.cpp:659 | 8 | 4 | 1 | 기능 | |

주의:
- `LoadFromJson` 은 SoundBank/AnimationLibrary/SpriteLibrary 세 클래스 (문자열 JSON 입력 시임, 디렉터리 로더가 내부 파서를 공유). 테스트 25회 사용.
- `ParseTuning`(Tuning.cpp:415)은 Tuning 테스트 10곳이 쓰는 문자열 시임이다. 실제 파일 로더는 같은 방문자를 쓰므로 삭제하려면 임시 파일을 만드는 방식으로 테스트를 바꿔야 한다.
- 새로 생긴 W1 시임: `AudioSystem::bRecord`/`events`/`AudioEvent`/`eAudioEvent` (AudioSystem.h:77-93,121-122, AudioSystem.cpp:107,261,321,374,384,406,436) 는 게임에서 항상 `bRecord=false`, Test 3파일만 켬. 기록 분기 6곳이 Source 에 상주.
- 아래 항목은 도구가 죽었다고 했으나 **살아 있어서 제외**: `ParamValue::EmptyObject` (StageHost.h:54 기본 멤버 초기화).

---

## 2. 죽은 테스트 / 테스트 코드 (HEAD 3790d01: 정의 921개, 미실행/실패는 아래)

기준선 (사용자 값 918 = 통과)에 대해 현재 921개 정의 중 SKIP 3, FAIL 1, 통과 917 이다 (W1 에서 1개가 줄어 922 -> 921. 사용자의 918 은 W1 이전 값).

| 스위트.테스트 | 파일:줄 (줄수) | 사유 | 삭제 안전도 |
|---|---|---|---|
| `Bus.StairSoldierTraceWritesCsvWhenRequested` | BusTest.cpp:389 (32) | (e) `BUS_TRACE` 환경변수 없으면 항상 SKIP. 진단용 CSV 덤프. | 안전. 단 위쪽 `#include <fstream>/<cstdlib>` 도 정리 |
| `StageIntegration.BusStairSoldierTraceWritesCsvWhenRequested` | StageIntegrationTest.cpp:869 (43) | (e) `BUS_TRACE_STAGE` 없으면 항상 SKIP | 안전 |
| `StageIntegration.BossEntryDumpWritesCsvWhenRequested` | StageIntegrationTest.cpp:1310 (17) | (e) `C15_DUMP` 없으면 SKIP. 이것이 쓰던 `BossEntryRecorder::WriteCsv` (StageIntegrationTest.cpp:1236-1255, 약 20줄) 는 호출 0 이 된다. 산출물 `Docs/Wayfinder/C15_dump_before/after.csv` 는 문서 | 안전 (WriteCsv 도 같이) |
| `PlayerReplay.WriteRegressionAndVisualScripts` | PlayerReplayTest.cpp:87 (20) | (a)(c) 검증이 아니라 파일 생성기: `Test/Replays/sandbox_regression.rep` 를 매 실행 덮어쓰고(추적 파일), `Captures/Visual/*.rep` 12개를 쓴다 (gitignore 폴더). 다음 테스트가 이 파일을 읽어서 **실행 순서 의존**. Visual 12줄(:93-104)은 읽는 곳 0 | 부분 안전: visual 12줄 + `Captures/Visual` 만 삭제, 회귀 .rep 은 보호(스크립트 상수화 필요) |
| `Camera.ShakeIsDeterministicDecaysAndStaysInBounds`, `Camera.StopShakeRestoresImmediately` | CameraTest.cpp:319 (18), :337 (11), 헬퍼 RunShake_ :41-56 | (a) 도달 불가 Camera::Shake 만 검증 (1-B 흔들림 체인) | 체인과 같이 삭제 |
| `UiBanner.QueueRunsSpecsBackToBackAndShowReplaces` | UiTest.cpp:416 (32) | (a) `BannerSystem::Enqueue`/`GetQueuedCount` (Source 호출 0) 만 검증. "Queue" 는 StyleRubric 5 의 제거 대상 | 안전 (Enqueue 와 같이) |
| `GroundMap.FindCeilingYScansUp` | GroundMapTest.cpp:131 (9) | (a) FindCeilingY 테스트전용 | 기능+테스트 동반 삭제 |
| `GroundMap.FindSurfaceUpClimbsOutOfShallowEmbedding` | GroundMapTest.cpp:140 (12) | (a) FindSurfaceUp 테스트전용 | 동일 |
| `GroundMap.MakeFromBitsUsesLsbAsLeftmostPixel`, `GroundMap.MakeFromRowsBuildsAsciiMap` | GroundMapTest.cpp:63 (16), :79 (16) | (a) MakeFromBits 는 이 테스트만. MakeFromRows 는 PropWorldTest:490 도 사용 (테스트 시임) | MakeFromBits 만 삭제, MakeFromRows 보류 |
| `Geometry.AabbIntersectionExpandAndTransform`, `Geometry.AabbMirrorFlipsAroundAxis`, `Geometry.AabbFactoriesAndAccessors` | GroundMapTest.cpp:270 (17), :287 (7), :232 (18) | (a) Aabb::Intersection/Union/Translated/MirroredX/GetBottomCenter 가 Source 호출 0 (Expanded 는 Camera::IsVisible(테스트전용) 경유) | 일부 (Expanded/Overlaps 검증 줄은 유지) |
| `Health.NonPositiveDamageAndHealing`, `Health.DeadEntitiesCannotBeHealed` | CombatTest.cpp:331 (15), :346 (8) | (a) Health::Heal/GetRatio 호출 0 | 안전 (Heal 과 같이. Damage 부분 줄은 보존) |
| `UiFont.FitTextScalePicksLargestScaleThatFits`, `UiFont.MeasureTextSizeUsesWidestLineAndLineGap`, `UiFont.UnsupportedCharactersHaveNoGlyph` | UiTest.cpp:191 (9), :200 (10), :153 (9) | (a) FitTextScale/MeasureTextWidth/Height/HasGlyph 호출 0 (UiBanner.StandardBannersFitTheScreenWidth :462 도 FitTextScale 사용) | 함께 삭제 |
| `StageData.GateParsesNestedActionsAndCounts` (CountGates/CountSpawns 줄), `StageDataReal.Stage0LoadsWithExpectedCounts` (CountSpawns/CountWaveEntries 줄) | StageDataTest.cpp:187 (17), :600 (36) | (a) Count* 3개 (+CountXxxIn_ 3개 51줄) 호출 0. 테스트 본체는 파싱 검증이라 삭제 말고 Count 줄만 | 줄만 삭제. **StageDataReal.Stage0LoadsWithExpectedCounts 는 StageData* 보호 + 기존 실패 이력(Cleanup.md)** |
| `EnemyWorld.CancelPendingRemovesOnlyDelayedSpawns` | EnemyWorldTest.cpp:221 (13) | (a) CancelPending 호출 0 | 안전 (CancelPending 과 같이) |
| `PropWorld.ClearByLayerAndResetRestoreIds` | PropWorldTest.cpp:577 (15) | (a) ClearByLayer 호출 0, Reset 부분은 유지 | 일부 |
| `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` | AnimIntegrityTest.cpp:99 (35) | (f) 이미 실패 중. 아래 상세 | **삭제 금지 (에셋 정합성 보호). 테스트가 맞고 에셋이 틀림** |
| `DataMigrationA.LoadedDefsKeepTheOldHardCodedValues` | DataMigrationATest.cpp:15 (42) | (b) C6 이관(코드 상수 -> Defs JSON) 직후 한 번 확인용: 옛 하드코딩 값 17개를 Defs 와 비교. 이관은 끝났고 값은 Defs 가 원본 | 불확실. 사용자가 Defs 를 손으로 고치면 깨지므로 삭제 쪽이 안전하지만 *DefTest 계열이라 보호 우선 |
| `SimRunner.StateFactoryKnowsDemoOnly` | ReplayTest.cpp:90 (6) | (b) 이름이 현실과 다름 (팩토리는 demo 외 17개 이름을 안다). 동작은 "demo 가 있고 임의 이름은 nullptr" | 이름만 바꾸거나 삭제. 위험 낮음 |
| `PlayerReplay.HeadlessReplayThroughTheStateFactoryUsesTheSamePath` | PlayerReplayTest.cpp:146 (16) | (c) 600틱 돌리고 tick 수만 확인. ReplayTest 의 SimRunner.*/Regress.ps1 이 더 강하게 검증 | 안전하나 가치 낮은 정도 (삭제 권장 약함) |
| `EnemySandbox.SandboxHashIsDeterministicWithEnemiesPresent` / `VehicleSandbox.SandboxHashIsDeterministicWithAllVehiclesPresent` | EnemySandboxTest.cpp:238 (10), VehicleSandboxTest.cpp:87 (9) | (c) 본문이 글자 단위로 같다 (`RunHashes_` 가 각자 다른 헬퍼라 입력은 다름) | 중복 아님 (정말 다른 입력). **보호 (Hash)** |
| `VehicleProjectile.RollingShotCanRollRightWhenItsDirectionSaysSo` | VehicleProjectileTest.cpp:86 (10) | (c) 바로 위 RollingShotKeepsRolling... (:75) 와 토큰 유사도 0.91, 방향만 다름 | 합치기 가능 (파라미터화) |
| `EnemyDefs.EveryDefinedClipExistsInTheAnimationLibrary` | EnemyDefTest.cpp:307 (38) | (c) `AnimIntegrity.EveryDefClipExistsWithMatchingFrameCountAndSingleFamily` (AnimIntegrityTest.cpp:134) 가 같은 검사(+프레임 수, 계열)의 상위 집합 | 사용자 판단 (*DefTest 보호). VehicleDefs.EveryVehicleClipNameExists... (VehicleDefTest.cpp:226, 36) 도 같은 종류 |
| `PropWorld.UnknownDefSpawnsNothing` / `EnemyLabTest.UnknownDefSpawnsNothing` | PropWorldTest.cpp:109 (8), EnemyLabTest.cpp:29 (9) | (c) 같은 이름, 다른 대상 (서로 다른 월드). 중복 아님 | 유지 |

### (d) 호출 0 헬퍼 (Test/*Util.*, TestAudio.*)

| 헬퍼 | 위치 | 근거 | 안전도 |
|---|---|---|---|
| `Sim::Runner()` | PlayerTestUtil.h:41, .cpp:62 | 외부 호출 0 (Test 전체 grep) | 안전 (약 5줄) |
| `Sim::StepUntil<Pred>` 템플릿 | PlayerTestUtil.h:58-72 | 외부 호출 0 (StepUntilGrounded/Airborne/Dead/Weapon 은 사용됨) | 안전 (약 15줄) |
| `VehicleRig::CountAllShots()` | VehicleTestUtil.h:76, .cpp:190 | 외부 호출 0 | 안전 (약 8줄) |
| `FakeStageHost::PlayerAlive()`, `bPlayerAlive` | StageTestUtil.h:45,92, .cpp:78-81 | IStageHost::PlayerAlive 호출 0 (1-A) | Source 와 같이 |
| `TestAudio.*` | TestAudio.h/.cpp | W1 이후 `CountPlays/CountKind/LastBgm` 3개 자유 함수, 모두 호출됨 | 유지 |

### (f) `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` 실패 원인

- 판정: **에셋 문제, 테스트는 정당**. 테스트 본문(AnimIntegrityTest.cpp:99-132)은 `_Left`/`_Right` 쌍의 (a) 발 기준 y 차이 64px 이하 (b) `loop` 플래그 일치를 요구한다.
- 같은 규칙을 `Assets/anim.json` (작업 트리 = HEAD 와 동일 결과) 에 파이썬으로 재현: 검사 209쌍 중 (a) 위반 0, (b) 위반 4쌍. `loop` 가 `_Left=true`, `_Right=false` 로 어긋남: `MissleTank_Up_Destroy`, `Jeep_ReAimming`, `Jeep_Shot_Ready`, `Jeep_Turn`.
- Cleanup.md 의 "알려진 기존 실패" 와 동일 (`Verify.ps1` knownFailures). anim.json 은 사용자 손편집 데이터이므로 이 에이전트가 고치지 않는다. 사용자에게: 위 4개 `_Left` 의 `loop` 를 `false` 로 (또는 의도라면 테스트 예외 목록에 추가) 할지 결정 필요.

### 테스트 파일 단위 메모
- 테스트 헤더 주석이 존재하지 않는 문서를 가리킴 (주석만, 동작 무관): `AnimIntegrityTest.cpp:1` (`Docs/AnimAudit.md`), `BigShieeTest.cpp:1` (`Docs/Spec_BigShiee.md`), `StateFactory.cpp:77` (`Docs/Spec_Slug.md`, `RemakePlan`).
- `Captures/Visual`, `Captures/` 는 gitignore 폴더라 `WriteRegressionAndVisualScripts` 가 매번 디스크를 채운다.

---

## 3. 죽은 CLI 옵션 / 디버그 기능

(`README.md` 옵션표 기준. 파싱 위치는 `Source/Core/App.cpp:126-270`, `Source/Core/Headless.cpp:64-105`.)

| 옵션 | 위치 | 참조 (Scripts / Test / Docs / README) | 판단 |
|---|---|---|---|
| `--editor-float-test` | App.cpp:154-157, App.h:62, App.cpp:337-340; Editor.h:31, Editor.cpp:92-95,175-178,219-223 (`m_bFloatTest`) | 0 / 0 / 0 / 0 | **삭제 (약 25줄)**. ImGui 뷰포트 분리 창 확인용 임시 옵션 |
| `--expect-finished` | Headless.cpp:101-104 (+ usage 문자열 :109, Headless.h:4 주석, 반환 코드 2 분기) | 0 / 0 / 0 / 0 | 삭제 후보 (Regress/Verify 가 쓰지 않음, 약 8줄) |
| `--gen-replay` | Headless.cpp:81-100, `MakeScriptedFrame_` :32-42 (약 35줄) | **0 / 0 / 0 / 0** (사용자 지시로 보호하지만 호출처가 현재 없다) | 보호. `Test/Replays/synth_*.replay` 15개의 생성기로만 남음 |
| `--dump-tuning` | Headless.cpp:76-80 | Verify.ps1 | 보호 |
| `--trace-load` | App.cpp:186-189 | TraceLoad.ps1 | 보호 |
| `--no-async-load` | App.cpp:182-185, App.cpp:307 (`numWorkers = 1`) | TraceLoad.ps1 (sync 모드), Architecture.md:73 | **살아 있음**: 비동기 로더는 지워졌지만 `LoadAtlasPagesParallel`/`DecodeImages` 의 워커 수를 1 로 만드는 효과가 있다. 이름만 낡음. TraceLoad.ps1 의 async/sync 두 모드는 이제 "워커 N개 vs 1개" 비교라 의미는 유지. BaselineTextures.txt 에 `## <state> async/sync` 8개 표가 있어 모드 통합은 기준선 수정 필요 (보호) |
| `--event-log` | App.cpp:194-200, Base/Log.cpp:80 | Scripts/VideoScore/videoscore.py, 여러 Wayfinder 문서 | 유지 (README 옵션표에는 없음) |
| `--replay-hold`, `--editor-god`, `--editor-lab-spawn`, `--editor-anim/-stage/-tab`, `--warp*`, `--life`, `--weapon`, `--hold`, `--capture*`, `--seed`, `--record` | App.cpp | README 에 기재, 코드 효과 확인됨 | 유지 |
| 헤드리스 `--ticks`, `--state` | Headless.cpp:68-75 | README | 유지 |

효과 없는 옵션은 찾지 못했다 (파싱된 값이 모두 읽힌다). `[load] workers N` 정보 로그 (App.cpp:308) 는 TraceLoad 가 `[tex]` 줄만 보므로 영향 없음.

---

## 4. 죽은 파일

| 대상 | 근거 | 판단 |
|---|---|---|
| `Source/**/*.cpp` 전부 | 3개 vcxproj(MetalSlug2_Clone 190, SlugTest 251, JugX 84 항목) 를 파싱해 모든 `Source/**/*.cpp` 가 게임 프로젝트에 있음. vcxproj 가 가리키는 없는 파일도 0 | 죽은 .cpp 없음 |
| `Source/**/*.h`, `Test/**/*.h` | 전부 어딘가에서 `#include` 됨 | 죽은 .h 없음 |
| `Source/Game/Player/SlugDef.cpp` + `SlugDef.h` | 1-B: 읽을 `Slug.json` 이 한 번도 없었음. 단 파일은 컴파일에 포함 | 삭제 후보 (1-B 와 동일, 불확실 1) |
| `Scripts/AuditCoords.ps1` (303줄) + `Scripts/AuditCoords.allow.txt` (203줄) + `Docs/Wayfinder/C6_audit.csv/.md` + `C6_migrate_A.csv` | 헤더 주석은 "회귀 규칙으로 사용" 이나 Verify/Regress/Build 어디서도 호출 안 함. 참조는 C6_audit.md 와 DataMigrationATest 주석뿐 | 후보 (불확실). 코드가 매직 넘버를 줄이는 중이라면 도구로 남길 가치 |
| `Scripts/VideoScore/{videoscore.py,sprites.py,scenes.json}` (약 650줄) | 호출은 수동 (`Docs/Wayfinder/Scoring.md`). 입력으로 `Trash/AssetsSource/videoplayback (1).mp4`, `(2).mp4` (추적됨), Python+cv2 필요. `--event-log`/`--capture-ticks` 옵션의 유일한 소비자 | 불확실 (사용자의 영상 비교 도구). 지우면 `--event-log`/`--capture-ticks` 도 사실상 죽음 |
| `Scripts/ValidateStage.ps1` | README, AssetFormat.md:30,54, StageFormat.md:159 가 참조. Prune_Assets 가 검증 수단으로 제안 | 유지 |
| `Docs/NodeSpec.md` (92줄) | 제목 "노드 기반 액터 데이터 설계 (제안)". README/코드/타 문서 참조 0. 제안 문서이며 노드 해석기 구현은 없음 | 삭제 후보 |
| `Docs/PlayerSpec.md` (92줄) | "플레이어 사양 (M2)". README 목록/코드/타 문서 참조 0. 현재 Tuning.json/코드가 원본 | 삭제 후보 (불확실: 레거시 문제 서술이 설계 근거) |
| `Docs/DepthSpec.md` | SpriteBatch.h:42 주석이 참조 | 유지 |
| `Docs/AssetFormat.md`, `EnemyDefsSchema.md`, `PropSpecs.md`, `StageFormat.md` | README 가 참조 | 유지 |
| `Docs/Wayfinder` 의 .md 중 다른 파일/README/코드에서 파일명 참조 0 | `A2_Summary.md`, `Boss_missile_offset.md`, `Boss_plan.md`, `Bus_plan.md`, `Bus_stage2.md`, `Bus_stage4.md`, `C6_audit.md`, `Vehicles_stage.md` (각 단독 작업 기록) | 후보. 이미지/CSV 는 Prune_Assets.md 가 다룸 |
| `Docs/Wayfinder/Prune_Assets.md`, `Prune_Assets_Items.csv` | 다른 에이전트 산출물 (미추적) | 건드리지 않음 |
| `Trash/` (추적 1,586개 883MB, 미추적 mp4 1개 258MB), `Raw/` (4개), `Backup/` (gitignore 8 스냅샷), `Captures/` | 코드 참조 0. 자산 쪽 상세는 Prune_Assets.md 에 있다. **미추적 `Trash/AssetsSource/Release 2026.10.10 - 20.46.29.11.mp4` 는 복구 불가이므로 지우지 말고 사용자에게 알린다** (StyleRubric 7) | 자산 조사로 위임 |
| `OpenInVS.cmd`, `Play.cmd`, `vcpkg.json`, `.sln` | 사용 중 | 유지 |
| `Test/Replays/enemylab_fight.replay` | Baseline.txt 의 `enemylab_fight` 와 `synth_enemylab` 해시가 같다 (7b12d9c08892fd47). 즉 같은 시나리오의 복제본일 가능성이 높다 (파일 크기 3632 B 동일) | 불확실. **보호 (리플레이 해시 15개 중 하나)** 이므로 지우면 해시 수가 14개로 바뀐다. 사용자 판단 |

---

## 5. 중복 / 과잉

### 5-1. 복붙 함수 (본문 동일, 정규화 후 비교)

| 이름 | 위치 | 합칠 곳 / 메모 |
|---|---|---|
| `Lerp_(Vec2,Vec2,float)` ×4 | SandboxState.cpp:34, EnemyWorld.cpp:17, SlugRender.cpp:15, StageState.cpp:40 | `Base/Math.h` 의 `Lerp<T>` (Math.h:40, `_x + (_y-_x)*_t`) 로 치환 가능 (Vec2 에 `-`,`*(float)`,`+` 있음). 부동소수 연산 순서가 같음을 빌드 후 해시로 확인 |
| `ToScreen_` ×2, `DrawBox_` ×2 | SandboxState.cpp:42/49, StageState.cpp:48/55 | 한쪽으로 합치기 (렌더 디버그 박스, 약 18줄) |
| `MakeCameraTuning_` ×2 | SandboxState.cpp:24, StageState.cpp:30 | 본문 동일 (`= {}` 초기화만 차이) |
| FNV-1a 믹스 헬퍼 (`MixValue_`/`HashBytes_`/`FnvMix_`) | 14 파일: Headless, DemoState, EnemyWorld, PlayerController, SlugSystem, SlugVehicle, ItemWorld, PropWorld, TrainingTargets, SandboxState, StageRunner, StageState, StageWorldHost, SpriteBatch (`1099511628211` 상수 14파일) | `HashBytes_` 는 PlayerController.cpp:58 / SlugVehicle.cpp:49 가 글자 단위로 같다. 나머지는 값 단위 변형이라 **모두 합치면 해시가 바뀔 수 있음** — 바이트 단위가 같은 것만 통합 |
| `MakePhysicsTuning_` ×2 | PlayerController.cpp:16, SlugVehicle.cpp:24 | 같은 본문 |
| `RandRange_` ×2 | ItemWorld.cpp:17, PropWorld.cpp:15 | 같은 본문 (템플릿) |
| `ReadInt_` ×2 | Animation.cpp:23, SpriteLibrary.cpp:12 | 같은 본문 |
| `ReadNumber_` ×2 | Tuning.cpp:313, SlugDef.cpp:202 | SlugDef 가 죽으면 자동 해소 |
| `Done_` ×2 | VehicleJeep.cpp:52, VehicleSarubia.cpp:45 | 같은 본문 (VehicleCommon 로 이동) |
| Test: `OnGround_` ×2, `ReadAll_` ×2 | EnemySandboxTest.cpp:13/EnemyWorldTest.cpp:13, AnimEditorTest.cpp:14/StageEditorTest.cpp:11 | TestUtil 로 |

### 5-2. 인터페이스 / 가짜 구현

| 대상 | 현황 | 판단 |
|---|---|---|
| `IAudioSink` + `NullAudio` + `TestAudio` | **W1(3790d01) 에서 해체 완료** (AudioSink.cpp/.h 삭제, AudioSystem 하나로, `bRecord` 시임, TestAudio 는 자유 함수) | 완료. 남은 것은 1-D 의 `bRecord`/`events` 상주 코드 |
| `IStageHost` | 구현 2개 (`StageWorldHost`, 테스트의 `FakeStageHost`). 가상 20개 중 `PlayerAlive` 미호출 (1-A). W1 커밋 메시지에서 "유지: FakeStageHost 가 StageRunner 테스트의 스크립트 월드" 로 결정 | 사용자 결정됨. 가상 함수 1개만 정리 |
| `IGameState` | 구현 5개 (Demo, Sandbox, Stage, Title, UiDemo) | 유지 (실제 다형) |
| `jug::IErrorCategory` / `SlugErrorCategory` | 구현 1개 (Vendor JugX 의 인터페이스) | 범위 밖 (JugX) |
| `BehaviorOps::onCommand` 훅 | 14개 Ops 어디서도 지정 안 됨 | 1-B SendCommand |

### 5-3. 묶음 구조체 (`*Context`/`*Diagnostics`/`*Config`) 실제 사용처

| 타입 | 정의 | 참조 (Source 파일 수 / 토큰, Test 토큰) | 판단 |
|---|---|---|---|
| `StageLoadDiagnostics` | StageData.h:412 (`message` 1필드) | 3 / 7, Test 12 | **필드 1개짜리 래퍼**. `String*` 로 대체 가능. Test 12곳 수정 필요. 에러 문구는 보존 |
| `PropTickContext` | PropWorld.h:117 (6필드) | 4 / 5, Test 5 | 호출부 2곳 (StageState.cpp:391, SandboxState.cpp:569) + Tick 1개. 값 운반 전용 (Rubric 5) |
| `EnemyPlayerInfo` | EnemyTypes.h:76 | 2 / 3 | EnemyContext 안의 필드 묶음 (사용처 1) |
| `CameraConfig` | StageData.h:346 | 2 / 3 | 데이터 정의 (Rubric 5-0-a 에 따라 구조체 자체는 OK) |
| `ExecContext` | StageRunner.h:111 (3필드, private) | 2 / 12 | 한 클래스 안 임시 묶음 |
| `TitleConfig` | TitleState.h:6 | 2 / 4, Test 5 | StateFactory 는 기본값만 사용. 모든 설정은 UiTest 가 주입 -> **테스트 전용 시임** (`inputDelayTicks`, `startFadeTicks`, `fadeInTicks`, `nextState`; `bPlayVoice` 만 Source 가 읽음) |
| `SandboxConfig` | SandboxState.h:27-50 (22필드) | 3 / 10, Test 16 | `bLoadSlugFile`, `slugTuning` 은 Test 0 (1-B), `bBackground`/`bLoadTuningFile`/`enemyDefPath` 는 PlayerTestUtil/EnemyTestUtil 만 설정 (Source 읽기 1) |
| `StageStateConfig` | StageState.h:30 | 3 / 5 | StateFactory 의 bosslab 계열 설정. 정당 |
| `EnemyContext`, `PlayerContext`, `GameContext`, `SlugContext` | 각 헤더 | 6/7, 4/31, 20/107, 5/33 | 실제로 여러 곳에서 쓰임. 유지 |
| `FxRequest`/`PropFxRequest`/`ItemFxRequest`/`EnemyFxRequest`/`SlugEjectRequest` | Projectiles.h / PropWorld.h / ItemWorld.h / EnemyBehaviors.h / SlugVehicle.h | 4~6 파일 / 4~15 토큰 | "요청 큐" 형태 (Rubric 5 의 Notice/Queue 계열). 5종 중 `PropFxRequest`(2파일/4토큰)가 가장 얇다 |

### 5-4. 한 곳에서만 쓰이는 얇은 래퍼 (Source 호출 1, 별도 정의)
- `EnemyWorld::GetHealth`, `PropWorld::GetDecor`, `StageState::GetRescuedPows` 는 1-A (호출 0).
- 단순 위임 getter 가 `PlayerRig::GetUpperFrame/GetLowerFrame -> AnimPlayer::GetFrame` (PlayerRig.cpp:454-462, 호출은 PlayerRigTest 만).
- `StageState::GetData/GetHost/GetRunner/IsLoaded` 는 Source 호출 0, Test 12/6/23/72회 (관측 getter, 1-D).

---

## 부록: 검증 제안 (삭제 에이전트용)
1. 삭제 전: `Scripts/Verify.ps1` (빌드 + Regress 15 해시 + 캡처 7 + 튜닝 + SlugTest, 알려진 실패 1개) 로 기준선 확인.
2. 1-A -> 1-C 의 "삭제" 분은 Test 참조 0 이라 테스트 수정 없이 지울 수 있다. 순서: 1-A 함수 -> 1-B 플레이어 X 제한 -> SlugDef -> 카메라 흔들림(Test 동반).
3. 해시 영향: 1-B 의 세 체인은 `HashState`/`DebugHash` 에 섞이는 필드가 없음을 확인했다 (Camera 해시는 shake 미포함, EnemyWorld 해시는 m_playerMin/Max 미포함 — 삭제 시 `EnemyWorld.cpp` HashState 본문에서 재확인 권장).
4. 스크립트 측 영향: 3번 표의 `--editor-float-test`/`--expect-finished` 삭제는 Scripts 를 건드리지 않는다.

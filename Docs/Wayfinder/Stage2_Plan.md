# [개정] 사용자 방향: 구조를 늘리지 않고 줄인다

아래 원안(Opus)은 새 클래스를 많이 만드는 설계였다(ImageLoader, GameApp/Layer/SimLayer/EditorLayer, LaunchDesc, InputSource, CaptureController, DebugKeys, SimSession). 사용자 지시("복잡한 구조/아키텍처 전부 제거, 웹스러운 코드 금지, C++ 스러운 코드")와 맞지 않아 아래처럼 바꾼다.

| 원안 | 개정 |
|---|---|
| S1 ImageLoader 클래스(워커/슬롯/Take) | **병렬 디코드 자유 함수 1개** (`std::for_each(std::execution::par)`, chunk 단위로 메모리 상한 유지). 메인이 결과를 순서대로 GPU 텍스처로 업로드 |
| S3~S6 호출부 전환, AsyncLoader/MainThreadQueue/Job/Notice 삭제 | 유지 (삭제가 핵심) |
| S7~S10 App 분해 (클래스 10개 신설) | **폐기.** App 은 하나로 두고, 죽은 옵션/중복/주석만 걷어낸다. Layer 스택/SimSession 등 신설 없음 |
| S11 Core/App → App/ 이동 | 순환(Core↔Game)이 실제로 문제가 되는 파일만 이동 (이름만 바꾸는 이동은 하지 않음) |
| S12 중복 헬퍼 추출 | 유지 (중복 제거) |
| S13 단독 텍스처 캐시 통합 | 유지 |
| S14 거대 파일 분할 | 이번 범위 아님 |
| (신규) 죽은 코드/리소스/파일 삭제 | `StyleRubric.md` 7절 |
| (신규) 웹스러운 구조 제거 | 단일 구현 인터페이스(IAudioSink, IStageHost) → 구체 클래스, *Context/*Diagnostics 묶음 구조체 해체 |

기준 문서: `Docs/Wayfinder/StyleRubric.md`.

---

# 2단계 구조 재설계 계획 (Opus 작성, 동작 불변)

기준: 백업 커밋 1a94a68 의 `Source/` (201파일, 약 5.2만 줄). 목표는 JugX 작성자처럼 단순/미니멀한 구조. **현재와 동작이 바뀌면 안 된다.**

## 한눈에 보는 결론
1. 비동기 로드는 약 600줄짜리 3겹 구조(워커 풀 → 메인 큐 통지 → 순서 재조립)다. `ImageLoader` 하나(약 150줄)로 줄인다. 워커는 "파일 읽기 + PNG 디코드"만, 메인 스레드가 요청 순서대로 결과를 받아 GPU 텍스처를 만든다. 메인 큐/통지/작업 인터페이스는 사라진다.
2. 실제로 안 쓰이는 코드가 있다: `App::SubmitToMainThread`(호출 0곳), `StageState` 의 동기 텍스처 분기(StageState.cpp:321-349, 도달 불가), 오디오 비동기 등록(이득 거의 0, C14_load.md:27).
3. `App.cpp`(657줄)는 인자 파싱/리플레이/캡처/디버그키/에디터 연결/로딩을 한 클래스가 한다. Application → GameApp → Layer 로 나눈다.
4. 회귀 안전망이 얕았다 → 1단계에서 이미 확장함 (`Scripts/Verify.ps1`, 합성 리플레이 13개, 캡처 해시).

## A. 현재 구조

| 모듈 | 파일 | 줄 | 내용 |
|---|---|---|---|
| Base | 20 | 1,124 | Json, Result, Error, Log, Random, Handle, Types (1단계에서 JugX 로 교체) |
| Core | 18 | 2,722 | App, Headless, Input, Replay, FixedStep, AssetPath, Tuning, AsyncLoader, MainThreadQueue |
| Render | 10 | 2,882 | Renderer, SpriteBatch, Animation, SpriteLibrary, Camera |
| Audio | 11 | 1,154 | AudioSystem(miniaudio), SoundBank, VoiceAllocator |
| World | 9 | 1,654 | GroundMap, Physics, Combat, Geometry |
| Game | 112 | 33,350 | 상태, 플레이어, 적, 소품, 스테이지, UI |
| Editor | 20 | 8,829 | ImGui 에디터 탭 |

의존 방향: Core ↔ Game 순환(App/Headless 가 Core 에 있고, Tuning/Input 이 Core 에 있음). Render/Audio 가 Core 를 아는 이유는 오직 AsyncLoader.

### 실행/프레임 흐름 (App.cpp)
- Init: ParseArgs_(옵션 ~25종) → 리플레이 로드(seed/상태 확정) → loader.Start → anim 파싱 → (렌더러 생성 전) 아틀라스 16장 제출 → Renderer/Editor/Sprites Init → Audio Init(작업 제출 후 WaitAll) → CreateState → SimRunner(OnEnter = 상태별 로드)
- Update_(dt): mainQueue.DrainAll → audio.Update → 디버그값 15개 ctx 복사 → 스텝 수 결정(FixedStepClock 1/60, 최대 0.25s, 프레임당 최대 5스텝) → Tick_×N(입력→녹화→Step) → Render_(state.Render → 에디터 오버레이 → ImGui → 캡처 → Present)
- 이벤트: 에디터가 먼저 받고 KEY_DOWN 만 차단. KEY_UP/FOCUS_LOST 는 항상 게임에 전달.
- 종료: 녹화 저장 → loader → runner(OnExit) → audio → sprites → editor → renderer.

### 에셋 로딩 경로
| 에셋 | 로더 | 스레드 |
|---|---|---|
| manifest.json → anim.json | AnimationLibrary::LoadFromDirectory | 메인 |
| Atlas/atlas_000..015.png | AtlasPageLoadJob: 디코드는 워커, SDL_CreateTexture 는 메인 | 워커+메인 |
| sprites.json | SpriteLibrary (Stage/UiSystem/Sandbox 가 각자 파싱) | 메인 |
| 단독 텍스처 | Stage: 비동기 작업 / UiSystem: static 캐시 / BigShiee: 지연 | 혼합 |
| Stage/*.json, Ground/*.png, Defs/**/*.json, Tuning.json | 각 로더 (Defs 는 경로 정렬 후 병합) | 메인 |
| sounds.json + ogg | SoundBank, 효과음은 SoundRegisterJob | 워커+메인 |

## B. 목표 흐름
```
main
 ├─ --headless → RunHeadless → SimSession::Create (App 과 같은 조립 함수) → Step×N → 해시 출력(형식 동일)
 └─ GameApp app{desc}; app.Run()
      Init: ParseLaunchArgs(순수 함수) → images.Start → anim 파싱 → 아틀라스 Request×16
            → Renderer/Editor/Sprites Init → Audio(동기 등록) → Take×16 → AddAtlasPage
            → SimLayer/EditorLayer attach
      루프: PollEvents → Layer.OnEvent(위→아래) ; Update(dt): audio → debug → 스텝 결정(FixedStepClock) → SimLayer.OnFixedUpdate×N → OnRender → EditorLayer.OnRenderGui → 캡처 → Present
      Shutdown: 녹화 저장 → images → 레이어 → audio → sprites → editor → renderer (순서 동일)
```
모듈 방향(목표): App → Editor → Game → {Render, Audio, World} → Core → JugX. 새 폴더는 `App/` 하나.

## C. 복잡도 진단
| # | 우선 | 문제 | 조치 |
|---|---|---|---|
| C1 | P1 | AsyncLoader/MainThreadQueue/SpriteBatch/Audio 작업 클래스 (~600줄, 뮤텍스 2, CV 2) | ImageLoader 로 대체 |
| C2 | P1 | 동기/비동기 분기 4곳 중복 | 동기 모드 = Take 에서 디코드 |
| C3 | P1 | SubmitToMainThread 호출 0곳 | 삭제 |
| C4 | P1 | StageState 동기 분기 도달 불가 | 삭제 |
| C5 | P1 | 아틀라스 작업이 SpriteBatch::Init 전 제출되는 숨은 전제 | Init 뒤에만 Take |
| C6 | P1 | 오디오 WaitAll 이 아틀라스 Complete 까지 실행 (숨은 결합) | 오디오 동기 등록 |
| C7 | P2 | App 이 책임 8가지 | LaunchDesc/InputSource/Capture/DebugKeys/Layer 분리 |
| C8 | P2 | Core↔Game 순환 | App/ 신설, Tuning → Game |
| C9 | P2 | App 과 Headless 의 시뮬 조립 중복 | SimSession::Create |
| C10 | P2 | GameContext 에 서비스+디버그 명령 혼재 | (선택) 분리 |
| C11 | P2 | 단독 텍스처 캐시 2개 | 1개로 통합 |
| C12 | P2 | Sandbox/Stage 에 static 헬퍼 4개 중복 | GameDraw 추출 |
| C13 | P3 | StageData.cpp 2384줄 등 거대 파일 | 파일 분할만 |
| C14 | 하지 말 것 | JSON 필드 리더 3벌 | 손대지 않음 |

남길 추상화: IGameState, IStageHost(FakeStageHost), IAudioSink(Null/TestAudio), SimRunner, FixedStepClock, StateFactory.

## D. 목표 구조
```
Source/
  App/    Main, GameApp, Layer, SimLayer, EditorLayer, LaunchDesc, SimSession, InputSource, CaptureController, DebugKeys, Headless
  Core/   Input, Replay, FixedStep, AssetPath
  Render/ Renderer, SpriteBatch, ImageLoader(신규), Animation, SpriteLibrary, Camera
  Audio/  AudioSystem(동기 등록), SoundBank, VoiceAllocator, AudioSink, AudioMath
  World/  (변경 없음)
  Game/   Tuning(이동), GameState, SimRunner, StateFactory, GameDraw(신규), Stage/ Enemies/ Player/ Props/ Ui/
  Editor/ (StageEditor 패널 분할)
```

## E. 단계별 실행 (스텝 1개 = 커밋 1개)
공통 관문: 빌드 + `Scripts/Verify.ps1` PASS.

| 스텝 | 내용 | 위험 | 담당 |
|---|---|---|---|
| S0 | 기준선 확보 (1단계에서 완료: Verify.ps1, 합성 리플레이, 캡처 해시) + `--trace-load` | 낮음 | sonnet |
| S1 | ImageLoader + 테스트 추가 (호출부 없음) | 낮음 | opus 설계 → sonnet |
| S2 | SpriteBatch::AddAtlasPage/AddStandaloneTexture(DecodedImage) 추가 | 낮음 | sonnet |
| S3 | App 아틀라스를 ImageLoader 로 전환 | 중간 | sonnet |
| S4 | StageState 단독 텍스처 Request/Take, 죽은 분기 삭제 | 중간 | sonnet |
| S5 | AudioSystem 동기 등록, SoundRegisterJob 삭제 | 낮음 | haiku |
| S6 | AsyncLoader/MainThreadQueue/관련 테스트 삭제 | 낮음 | haiku |
| S7 | ParseArgs_ → ParseLaunchArgs 순수 함수 + 테스트 | 중간 | sonnet |
| S8 | InputSource/CaptureController/DebugKeys 분리 | 중간 | opus 경계 → sonnet |
| S9 | SimSession 로 App/Headless 조립 공통화 | 중간 | sonnet |
| S10 | GameApp/Layer/SimLayer/EditorLayer 재조립 | **높음** | opus |
| S11 | Core/App → App/, Tuning → Game/ 파일 이동 | 낮음 | haiku |
| S12 | GameDraw 중복 헬퍼 추출 | 낮음 | haiku |
| S13 | UiSystem static 캐시 → SpriteBatch 캐시 통합 | 중간 | sonnet |
| S14 | StageData/StageEditor/EnemyDefLoader 파일 분할 (본문 무변경) | 낮음 | haiku |
| S15,16 | (선택) GameContext 분리, sprites.json 1회 파싱 | 중간 | opus 검토 → sonnet |

순서: S1~S6(비동기 단순화) → S7~S10(App 분해) → S11~S14(정리).

## F. 비동기 로드 단순화
현재: Submit → 워커 Execute(디코드) → ExecutedNotice → MainThreadQueue.DrainAll → 연속 구간 Complete(SDL_CreateTexture) → WaitForTask(20ms) 반복.

목표:
```
메인: t = images.Request(path, bFlash)        워커: 다음 미처리 && (ticket < takeHead+window)
      ...(다른 초기화)                              파일 읽기 → PNG 디코드 → (플래시 램프) → bDone
      img = images.Take(t)   (Request 순서대로만)
      sprites.AddAtlasPage(img)  ← GPU 생성은 메인 전용
```
- 워커가 만지는 것: 파일 읽기, stb 디코드, 플래시 램프, 로그. 메인만: SDL/SpriteBatch/게임 객체.
- 메모리 상한: window = 워커 수 유지 (페이지당 32MB, 상한 없으면 최대 512MB).
- 동기 모드(`--no-async-load`): 워커 0 → Take 에서 직접 디코드. 분기 소멸.
- 결정성: 시뮬레이션은 텍스처 핸들을 읽지 않음. 헤드리스는 로더 자체가 없음. Take 순서 = Request 순서(assert). 디코드 함수 하나뿐.
- 줄 수: 제거 약 620줄, 추가 약 190줄.
- **사용자 선택 필요**: "파일 IO 만 비동기"를 글자 그대로 하면 PNG 디코드가 메인으로 돌아와 로딩이 느려진다(Release stage0 png-decode 0.65s). 권장은 워커 = 읽기+디코드, 메인 = GPU 생성.

## G. 검증 불변식
시드(리플레이 seed, Stage 내부 0x57A6E0), 고정 스텝(1/60, 0.25s, 5스텝, 리셋), 틱당 InputFrame 1개, 로드 순서(anim 아틀라스 순서 = 페이지 번호; Stage: 원경 → 배경 → 지면 → Tuning → Defs 경로 정렬 → Props), 헤드리스 경계(pRenderer == nullptr), 프레임 순서, 이벤트 규칙, 종료 순서(녹화 저장 먼저), 종료 코드(0/1/2).

## H. 리스크 / 하지 말 것
- JugX Application 상속(S10): Init 중 Quit 무시, SDL_INIT_GAMEPAD 기본 true, SDL_Init 실패 시 terminate → bInitGamepad=false, 실패 플래그 처리, 또는 형태만 본뜬 자체 GameApp.
- Juggernaut GameApp 의 고정 스텝(float, 제한 없음)은 우리 FixedStepClock 과 다름 → 반드시 FixedStepClock 사용.
- Debug 디코더 TU 최적화 설정 유지 (stb 구현을 다른 파일로 옮기지 않음).
- 하지 말 것: JSON 파서 3벌 통합, 헤드리스/창 모드 차이 통일, 로드 병렬화/정렬 제거, RNG 시드 위치 변경, FixedStepClock 교체, 서비스 싱글톤화, 에디터 처리를 틱 단계로 이동, IStageHost/IAudioSink/IGameState 제거, Assets/Baseline/Trash 건드리기.
- 되돌리기: 스텝 1개 = 커밋 1개, 실패 시 revert. `--no-async-load` 유지. 구조 작업 중 Baseline 갱신 금지.

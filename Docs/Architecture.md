# MetalSlug2 Clone — 프로젝트 개요와 아키텍처

C++20 + SDL3 로 만든 메탈슬러그 2 스타일 2D 액션 게임(Mission 4 한 스테이지). 에셋은 `Assets/` 의 JSON/PNG 를 손으로 고치면 바로 반영되고(변환 단계 없음), 같은 시뮬레이션 코어를 창 없이 돌려 회귀 해시로 검증한다.

---

## 1. 한눈에 보기

| 항목 | 값 |
|---|---|
| 언어 / 빌드 | C++20, MSVC(v145), MSBuild (`MetalSlug2_Clone.sln`), vcpkg manifest |
| 의존성 | SDL3, yyjson, stb, miniaudio, imgui(docking), gtest, **JugX**(서브모듈) |
| 소스 | `Source/` 약 184 파일 / 4.4만 줄, `Test/` 64 파일 / 2.0만 줄 |
| 테스트 | 단위 테스트 903개 (알려진 기존 실패 1건) + 리플레이 해시 15개 + 화면 캡처 해시 7개 |
| 핵심 약속 | 시뮬레이션은 **(틱 번호, 그 틱의 입력)** 만으로 결정된다 → 같은 리플레이는 항상 같은 해시 |

```text
빌드      powershell -NoProfile -File Scripts/Build.ps1 [-Config Debug|Release] [-Target SlugTest]
실행      Play.cmd            (Release 증분 빌드 후 실행, 인자는 게임으로 전달)
검증      powershell -NoProfile -File Scripts/Verify.ps1     ← 리팩터링 후 반드시
```

---

## 2. 눈여겨볼 부분 (리뷰 우선순위)

1. **`Source/Render/SpriteBatch.cpp` 의 병렬 디코드 함수** — 비동기 로딩이 약 600줄짜리 3겹 구조(`AsyncLoader` + `MainThreadQueue` + Job/Notice)에서 **함수 하나**로 줄었다. 워커는 "파일 읽기 + PNG 디코드 + 피격 플래시 이미지"만 하고, 메인 스레드가 결과를 요청 순서대로 받아 GPU 텍스처를 만든다. 대가는 Release `stage0` 첫 프레임 약 +100ms(+8%).
2. **`Source/Base/` 와 `Base/Pch.h`** — 자체 유틸이 JugX 로 교체되었다(`Types`, `Handle`, `HashMap`, `Result/Error`, `Random`, `Assert`, 로그 코어). `using namespace jug` 는 쓰지 않고 `using jug::X;` 로 교체한 이름만 가져온다(`RGBA`, `Min/Max` 같은 MS2 고유 이름과 충돌하기 때문).
3. **의도적으로 JugX 로 바꾸지 않은 것** — `Vec2`(JugX 는 나눗셈이 역수 곱 + 기본 생성자 미초기화), `RGBA`(8바이트 union, 필드 순서 다름), `Min/Max/Abs/Lerp`(혼합 타입 절삭, 부호비트 차이), `Json`(에러 계약·에디터 의존), `EventLog`. 바꾸면 해시·캡처가 달라질 수 있다.
4. **JugX 서브모듈의 알려진 버그 2건(우회 중)** — `JugX/Random.h:77`(실수 범위 난수가 컴파일 불가), `JugX/Scoped.h`(변환 생성자의 `requires` 조건이 거꾸로). 수정은 로컬에서 끝났고 푸시 대기 중이다. 푸시 후 우회 코드(`Random<float>(rng)*(max-min)+min` 4곳, `Scoped<Base>{ new Derived }` 약 11곳)를 되돌릴 수 있다.
5. **`Scripts/Verify.ps1`** — 동작 불변을 지키는 유일한 안전망. 기준선은 `Docs/Wayfinder/Baseline*.txt` 에 있고, 의도한 동작 변경이 아니면 `-Update` 를 쓰지 않는다.
6. **`Assets/anim.json` 의 `loop` 불일치 4건** — `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` 실패의 원인(`MissleTank_Up_Destroy`, `Jeep_ReAimming`, `Jeep_Shot_Ready`, `Jeep_Turn` 은 `_Left=true`, `_Right=false`). 코드가 아니라 데이터 문제이고, 고치면 재생이 달라지므로 사용자 결정 사항이다.
7. **`Game/GameState.h` 의 `GameContext`** — 서비스 포인터(`pRenderer`, `pSprites`, `pAnims`, `pRng`, `pAudio`)와 디버그 스위치가 섞인 묶음. 헤드리스 판별(`pRenderer == nullptr`)도 겸한다. JugX 의 `GetSingleton()` 스타일과 가장 거리가 있는 곳이다.

---

## 3. 실행 흐름

```text
main (Source/Main.cpp)
 ├─ --headless  → RunHeadless(): 리플레이 로드 → 시뮬 N틱 → "ticks= finished= hash=" 출력 (창/렌더/오디오 없음)
 └─ 일반 실행   → App::Run()
        ① SDL 초기화
        ② Init_  : 명령줄 파싱 → 리플레이/시드 확정 → anim.json 파싱
                   → Renderer / Editor / SpriteBatch 초기화 → 아틀라스 16장 병렬 디코드 후 순서대로 GPU 업로드
                   → Audio 초기화(효과음 동기 등록) → 첫 GameState 생성 (SimRunner)
        ③ 루프   : PollEvents_ → Update_(dt)
        ④ Shutdown_ : 녹화 저장 → 상태 종료 → 오디오 → 스프라이트 → 에디터 → 렌더러
```

### 매 프레임 (`Update_`)
```text
1. 오디오 갱신
2. 디버그 키 값(F1~F10 등)을 GameContext 로 복사
3. 이번 프레임에 몇 틱을 돌릴지 결정 (FixedStepClock: 1/60초 고정, 프레임당 최대 5틱)
4. 틱마다:  InputFrame(키보드 or 리플레이) → (녹화) → SimRunner.Step → GameState.Tick
5. 그리기:  GameState.Render → 에디터 오버레이(ImGui) → (스크린샷 캡처) → 화면 출력
```

### 입력 / 리플레이 / 헤드리스
- 한 틱 = `InputFrame` 1개. 키보드 입력도 같은 형식으로 샘플링되어 `--record` 로 파일에 남고 `--replay` 로 재생된다.
- 헤드리스는 같은 `SimRunner` 를 렌더러/오디오 없이 돌린다. 해시 = FNV-1a(틱 수, 종료 여부, 상태의 `DebugHash`).
- 시드: 리플레이 seed → `RngMt19937`. 스테이지 내부 RNG 는 고정(`0x57A6E0`), 오디오 피치 RNG 는 별도라 해시와 무관.

---

## 4. 모듈 지도

```text
Source/
  Base/    JugX 별칭/래퍼(Types, Handle, SlugError, Log) + 게임 고유 유틸(Json, Math/Vec2, RGBA, PhaseTimer)
  Core/    App(루프), Headless, Input, Replay, FixedStep, AssetPath, Tuning
  Render/  Renderer(SDL), SpriteBatch(그리기 + 병렬 디코드), Animation, SpriteLibrary, Camera
  Audio/   AudioSystem(miniaudio), SoundBank, VoiceAllocator, AudioMath
  World/   GroundMap(지면 마스크), Physics, Combat, Geometry
  Game/    GameState/SimRunner/StateFactory + Player, Enemies, Props, Stage, Sandbox, Ui, Combat
  Editor/  ImGui 에디터 (Animation / Map / Collision / AI 탭)
Vendor/JugX    서브모듈. Projects/JugX 정적 라이브러리로 빌드되어 게임/테스트가 링크
```

| 모듈 | 파일 | 줄(비어있지 않은 줄) |
|---|---|---|
| Base | 14 | 794 |
| Core | 14 | 2,057 |
| Render | 10 | 2,442 |
| Audio | 9 | 941 |
| World | 9 | 1,268 |
| Game | 110 | 28,746 (Enemies 10.7k · Stage 5.9k · Player 4.1k · Props 3.2k · Ui 1.8k · Combat 1.1k) |
| Editor | 20 | 8,286 |

의존 방향은 대체로 `Editor → Game → Render/Audio/World → Core → Base → JugX` 이다. 현재 `Core/App` 이 Game·Editor 를 알고 Game 이 `Core/Tuning` 을 아는 순환이 하나 남아 있다.

---

## 5. 게임 상태(GameState)

`IGameState` 한 개가 "화면 하나"다. 구현은 5개(Title, Stage, Sandbox, Demo, UiDemo)이고 런타임에 전환되므로 다형성이 실제로 쓰인다.

```text
OnEnter(로드) → 매 틱 Tick → 매 프레임 Render → TakeNextState 로 다음 상태 지정 → OnExit
```
- `StateFactory` 가 이름 문자열(`stage0`, `bosslab`, `helilab_bot` …)로 상태를 만든다. 개발용 Lab(`enemylab`, `buslab`, `bosslab`, `helilab`, `sluglab`, `proplab`, `vehiclelab`)은 `SandboxState` 의 설정 분기다.
- `SimRunner` 가 틱 수, 상태 전환, 입력 기록(history)을 관리한다.

---

## 6. 데이터(에셋) 흐름

```text
Assets/
  manifest.json → anim.json → AnimationLibrary   (에디터가 같은 객체를 직접 수정, 1초마다 파일 변경 감지)
  Atlas/atlas_000..015.png → SpriteBatch 텍스처 페이지 (병렬 디코드 → 순서대로 GPU 업로드)
  sprites.json → SpriteLibrary
  sounds.json + Audio/*.ogg → SoundBank → AudioSystem
  Stage/<이름>.json → StageData(파서) → StageState(배치/스폰)
  Ground/*.png → GroundMap  (충돌 마스크: 파랑 = 땅)
  Defs/**/*.json → EnemyDefLibrary / PropDefLibrary / 차량·보스 정의 (경로 정렬 후 병합)
  Tuning/Tuning.json → Tuning (플레이어/카메라 수치)
```
- 에디터 저장은 JSON **원문 텍스트를 제자리에서 고치는** 방식이라 서식과 키 순서가 보존된다(`Editor/JsonScan`).
- 로드 순서(Stage): 원경 → 배경 레이어 → 지면 → Tuning → 적 정의(경로 정렬) → 소품. 이 순서가 텍스처 핸들 번호와 결과를 결정하므로 바꾸지 않는다.

---

## 7. 비동기 로딩 (이번에 가장 크게 바뀐 부분)

```text
이전: Submit(Job) → 워커 Execute → ExecutedNotice → MainThreadQueue.DrainAll → 순서 재조립 → Complete(GPU)
      (AsyncLoader 222줄 + MainThreadQueue 54줄 + Job 3종 + Notice + 인터페이스 2개 ≈ 600줄)

지금: DecodeImages(경로 목록, 워커 수)          ← std::for_each(std::execution::par), 결과는 요청 순서 그대로
      for 결과: AddAtlasPage / AddStandaloneTexture   ← 메인 스레드, GPU 생성
      (메모리 상한: 한 번에 워커 수만큼씩 chunk 로 처리)
```
- 스레드 경계: 워커는 파일/디코드/플래시 이미지만, SDL 과 게임 객체는 메인만 만진다. 콜백/큐/통지 없음.
- `--no-async-load` 는 워커 1(순차)로 같은 결과를 낸다. 시뮬레이션은 텍스처 핸들을 읽지 않으므로 헤드리스 해시는 구조적으로 영향받지 않는다.

---

## 8. 에디터 (`--editor`)

| 탭 | 대상 | 비고 |
|---|---|---|
| Animation | `Assets/anim.json` | 피벗/루프/프레임 길이 편집, 실행 중 스프라이트에 즉시 반영 |
| Map | `Assets/Stage/*.json` | 스테이지 배치 |
| Collision | `Assets/Ground/*.png` | 충돌 마스크 칠하기 |
| AI | `Assets/Defs/*.json` | 정의 핫 리로드, Hitbox 창 |

에디터 처리(정의 리로드 등 시뮬 변경)는 렌더 단계에서 일어난다. 이 순서를 틱 단계로 옮기지 않는다.

---

## 9. 검증 장치 (동작 불변)

`Scripts/Verify.ps1` 한 번으로:

| 단계 | 내용 | 기준선 |
|---|---|---|
| 빌드 | Debug (게임 + 테스트) | — |
| 리플레이 해시 | 헤드리스 15개 (상태별 합성 리플레이 13 + 기존 2) | `Docs/Wayfinder/Baseline.txt` |
| 캡처 해시 | 상태 7개 오프스크린 스크린샷 SHA256 | `Docs/Wayfinder/BaselineCapture.txt` |
| 튜닝 덤프 | `--dump-tuning` SHA256 | 〃 |
| 단위 테스트 | 903개, 알려진 실패 1건만 허용 | — |
| 텍스처 로드 표 | `Scripts/TraceLoad.ps1` (async/sync × 4상태) | `Docs/Wayfinder/BaselineTextures.txt` |

---

## 10. 코드 스타일 (JugX/JugGfx/JugNet 기준)

- 이름: 타입/함수 PascalCase, 멤버 `m_`, 매개변수 `_`, 상수 `k`, `enum class eX`, 포인터 `p`, bool `b`, 단위 접미사 `Sec`, private 함수/익명 NS 헬퍼 `Foo_`.
- 서식: `.clang-format` (Allman, 4칸). `.h` 선언은 한 줄, `.cpp` 정의는 `(` 뒤 줄바꿈 + 인자 1개/줄 + 값 전달 매개변수 `const`.
- 규칙: `auto` 금지, 변수에 담는 람다 금지, C 배열 금지(`ARRAY`/`Vector`), 소유 포인터는 `Scoped`, 사전조건은 `JUG_ASSERT`, 에러는 `Result<T>`(예외 없음).
- 주석: "왜" 한 줄만. 이름/타입을 반복하는 주석과 후행 주석은 쓰지 않는다.
- 상세: `Docs/CodingStyle.md`, 정리 기준: `Docs/Wayfinder/StyleRubric.md`.

---

## 11. 이번 작업 요약과 자가평가

### 변경 규모 (1단계 완료 시점 `stage1-complete` 이후, Source/Test)
- **+2,275 / −5,519줄**, 201 파일. 단위 테스트 920 → 903개(죽은 테스트 제거).
- 제거: `AsyncLoader`/`MainThreadQueue`/Job/Notice, `IAudioSink`/`NullAudio`/`TestAudio`, `SlugDef` 로더(읽을 JSON 이 없던 약 420줄), 플레이어 X 제한 체인, 카메라 흔들림 체인, 항상 SKIP 이던 테스트 3종, 호출 0 헬퍼, 죽은 CLI 옵션(`--editor-float-test`, `--expect-finished`), 중복 헬퍼(`Lerp_`, `DrawBox_`, `ToScreen_`, `MakeCameraTuning_` …).

### 지표 (JugX 코퍼스 vs MS2, 같은 스크립트로 측정)
| 지표 | JugX | MS2 | 판정 |
|---|---|---|---|
| clang-format 일치 | 87.6% | 100% | 달성 |
| 매개변수 `_` 접두 | 98.0% | 97.8% | 달성 |
| 주석 밀도 | 4.2% | 1.4% | 달성 |
| 헤더 줄바꿈 선언(/kloc) | 51 | 1.8 | 달성 |
| `std::` 직접 사용 / `auto` | 0.3 / 2.3 | 0.02 / 0 | 달성 |
| Job·Queue·Manager 류 | 3 | 0 | 달성 |
| 인터페이스 `I*` | 2 | 2 | 달성 |
| `JUG_ASSERT` (/kloc) | 16.2 | 0.29 | **미달** — MS2 는 방어 분기/`Result` 로 처리 |
| `Result<>` 반환 (/kloc) | 2.8 | 6.9 | **미달** — 파일/JSON 경계라 대부분 유지 |
| `*Context`/`*Diagnostics` 구조체 | 1 | 9 | **미달** — `GameContext` 등 |
| 1000줄 초과 파일 | 5 | 9 | **미달** — `StageData.cpp` 2.2k 등 |

### 종합
| 항목 | 평가 |
|---|---|
| 동작·결과 불변 | **달성** — 해시 15/15, 캡처 7/7, 튜닝, TraceLoad, 단위 테스트 모두 변경 전과 동일 |
| JugX 유사성 | **표면은 높음**(서식·이름·주석·헤더), **구조는 중간**(사전조건 assert 습관, Context 묶음, 큰 파일) |
| 단순성 | **중간 이상** — 가장 복잡하던 로더/인터페이스/중복 제거. `GameContext`, 거대 파일, JSON 필드 리더는 남음 |
| 리소스 compact | **미완** — `Assets/` 는 이미 거의 비어 있음(미사용 1개). `Trash/` 925MB(tracked 1,586파일)는 삭제 권한 대기 |

### 남은 일 (사용자 결정/권한 필요)
1. `Trash/`, `Scripts/VideoScore`, `Raw/`, 미참조 문서 이미지, `Assets/Audio/Sfx_Bingo.mp3` 삭제 (`git rm`, 권한 필요)
2. JugX 서브모듈 수정 푸시 → 우회 코드 되돌리기
3. `GameContext` → `GetSingleton()` 전환 (모든 상태 시그니처와 테스트에 영향, 별도 웨이브)
4. `anim.json` `loop` 불일치 4건 수정 여부 (재생이 달라짐)
5. 디스크에만 있는 복구 불가 파일(`Trash/**/*.mp4` 270MB, 중첩 저장소 522MB, `Captures/` 879MB, `Backup/`) 삭제 여부
6. JSON 필드 리더 3벌(`StageData` / `EnemyDefLoader` / `PropDef`, 합쳐 약 700줄): 뼈대는 같지만 에러 문구·정수 변환 규칙·허용 키 규칙이 서로 달라 지금은 합칠 수 없다(합치면 인자로 문구를 넘기는 구조가 되어 더 복잡). **에러 문구 통일과 테스트 단언 갱신을 허용하시면** `Json.h` 의 `GetNumber/GetString/GetVec2` 로 통합 가능

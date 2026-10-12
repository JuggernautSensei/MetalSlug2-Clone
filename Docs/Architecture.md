# MetalSlug2 프로젝트 전체 흐름 (현재 구조)

이 문서는 "코드를 처음 보는 사람이 전체 그림을 잡는" 용도다. 1단계(JugX 포팅 + 스타일 변환) 이후 기준이며, 2단계에서 바뀔 부분은 맨 아래에 따로 적었다. 자세한 2단계 계획은 `Docs/Wayfinder/Stage2_Plan.md`.

## 1. 한 문장 요약
**"입력(키보드/리플레이) → 고정 1/60초 시뮬레이션(GameState) → 그리기(SpriteBatch) → 화면"** 을 반복하는 게임이고, 같은 시뮬레이션 코어를 창 없이(`--headless`) 돌려 회귀 해시로 검증한다.

## 2. 실행 흐름

```
main
 ├─ --headless  → RunHeadless(): 리플레이 로드 → 시뮬 N틱 → "ticks= finished= hash=" 출력 (창/렌더/오디오 없음)
 └─ 일반 실행   → App::Run()
        ① SDL 초기화
        ② Init_ : 명령줄 파싱 → 리플레이/시드 확정 → 애니메이션(anim.json)·아틀라스 로드
                  → Renderer / Editor / SpriteBatch / Audio 초기화 → 첫 GameState 생성
        ③ 루프  : PollEvents_ (입력) → Update_(dt)
        ④ Shutdown_ : 녹화 저장 → 상태 종료 → 오디오 → 스프라이트 → 에디터 → 렌더러
```

### 매 프레임(Update_)
```
1. 오디오 갱신
2. 디버그 키 값(F1~F10 등)을 GameContext 로 복사
3. 이번 프레임에 몇 틱을 돌릴지 결정 (FixedStepClock: 1/60초 고정, 최대 5틱)
4. 틱마다:  InputFrame(키보드 or 리플레이) → (녹화) → SimRunner.Step → GameState.Tick
5. 그리기:  GameState.Render → 에디터 오버레이(ImGui) → (스크린샷 캡처) → 화면 출력
```
핵심 약속: **시뮬레이션은 "틱 번호 + 그 틱의 입력"만으로 결정**된다. 그래서 같은 리플레이는 항상 같은 해시가 나온다.

## 3. 모듈 지도
```
Source/
  Base/    공용 유틸. JugX 로 대체된 것(Types, Handle, Result/Error, Random, Assert, Log 코어)과
           게임 고유 유틸(Json, Math(Vec2), RGBA, PhaseTimer, EventLog)
  Core/    App(루프), Headless, Input, Replay, FixedStep, AssetPath, Tuning
  Render/  Renderer(SDL), SpriteBatch(그리기), Animation, SpriteLibrary, Camera
  Audio/   AudioSystem(miniaudio), SoundBank, VoiceAllocator
  World/   GroundMap(지면 마스크), Physics, Combat, Geometry
  Game/    GameState/SimRunner/StateFactory, Player, Enemies, Props, Stage, Sandbox, Ui
  Editor/  ImGui 에디터 (Animation/Map/Collision/AI 탭)
Vendor/JugX   서브모듈. 정적 라이브러리(Projects/JugX)로 빌드되어 게임/테스트가 링크
```
의존 방향(대체로 위 → 아래): `Editor → Game → Render/Audio/World → Core → Base → JugX`.
단, 현재는 `Core/App` 이 Game·Editor 를 알고 Game 이 `Core/Tuning` 을 아는 **순환**이 있다 (2단계 정리 대상).

## 4. 게임 상태(GameState)
`IGameState` 한 개가 "화면 하나"다 (Title, Stage, Sandbox, Demo, UiDemo 와 각종 Lab).
- `OnEnter`(로드) → 매 틱 `Tick` → 매 프레임 `Render` → 끝나면 `TakeNextState` 로 다음 상태 지정 → `OnExit`.
- `StateFactory` 가 이름 문자열(`stage0`, `bosslab`, …)로 상태를 만든다.
- `SimRunner` 가 틱 수/상태 전환/입력 기록(history)을 관리한다.

## 5. 데이터(에셋) 흐름
```
Assets/
  manifest.json → anim.json → AnimationLibrary (에디터가 같은 객체를 직접 수정, 1초마다 파일 변경 감지)
  Atlas/atlas_000..015.png → (병렬 디코드) → SpriteBatch 텍스처 페이지
  sprites.json → SpriteLibrary
  sounds.json + Audio/*.ogg → SoundBank → AudioSystem
  Stage/<이름>.json → StageData(파서) → StageState(배치/스폰)
  Ground/*.png → GroundMap (충돌 마스크: 파랑 = 땅)
  Defs/**/*.json → EnemyDefLibrary / PropDefLibrary / 차량·보스 정의 (경로 정렬 후 병합)
  Tuning/Tuning.json → Tuning (플레이어/카메라 수치)
```
에디터 저장은 JSON 원문 텍스트를 제자리에서 고치는 방식(서식·키 순서 보존)이라 `JsonScan` 같은 별도 코드가 있다.

## 6. 이미지 로딩
```
DecodeImages(경로 목록): 여러 스레드가 PNG 읽기+디코드(+피격 플래시 이미지) → 경로 순서 그대로 반환
메인: 결과를 순서대로 SDL_CreateTexture (GPU 업로드는 메인 스레드에서만 가능)
```
- 대상은 아틀라스 16장(`LoadAtlasPagesParallel`)과 Stage 단독 텍스처(`LoadStandaloneTexturesParallel`). 오디오 효과음은 순서대로 동기 등록한다.
- 한 번에 `워커 수`장씩 디코드해 메모리 상한을 둔다. `--no-async-load` 는 워커 1개(순차).
- 결과는 경로 순서대로 확정되므로 텍스처 번호가 일정하다(결정성).

## 7. 검증 장치 (동작 불변을 지키는 방법)
`powershell -NoProfile -File Scripts/Verify.ps1` 한 번으로:
1. Debug 빌드 (게임 + 테스트)
2. 헤드리스 리플레이 해시 15개 (`Docs/Wayfinder/Baseline.txt`)
3. 화면 캡처 해시 7개 + 튜닝 덤프 해시 (`Docs/Wayfinder/BaselineCapture.txt`)
4. 단위 테스트 920개 (기존 실패 1건만 허용: `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor`)

의도한 동작 변경이 아니면 `-Update` 를 쓰지 않는다.

## 8. 1단계에서 달라진 것
- JugX(`Vendor/JugX`)를 서브모듈 + 정적 라이브러리로 도입.
- 교체: `Types`, `Handle`, `HashMap`(phmap), `Result/Error`, `Random`, `Assert`, `Log 코어` → JugX. (MS2 쪽에는 `eError` 카테고리, 로그 래퍼가 남음)
- 의도적으로 유지: `Vec2`, `RGBA`, `Math` 함수(연산 순서·메모리 레이아웃이 달라 결과가 바뀔 수 있음), `Json`(에러 계약·에디터 의존), `PhaseTimer`, `EventLog`.
- 스타일: JugX `.clang-format` 일괄 적용, file-scope `static` → 익명 네임스페이스(`Foo_`), `unique_ptr` → `Scoped`, 테스트의 `std::` → 별칭, 정의에 `const`.
- 알려진 JugX 이슈 2건(서브모듈은 수정하지 않고 MS2 쪽에서 우회):
  1. `Random<T>(rng, min, max)` 실수 경로가 `Random(_rng)` 로 T 를 추론하지 못해 컴파일 불가 → `Random<float>(rng) * (max-min) + min` 으로 직접 풀어 씀 (4곳).
  2. `Scoped<Derived>` → `Scoped<Base>` 변환 생성자의 `requires` 조건이 거꾸로 → `Scoped<Base> { new Derived(...) }` 형태로 우회 (약 11곳).

## 9. 2단계에서 바뀔 큰 그림 (계획)
```
Core/App.cpp(657줄, 책임 8가지)  →  App/ { GameApp, Layer, SimLayer, EditorLayer, LaunchDesc, InputSource, CaptureController, DebugKeys, SimSession }
옛 비동기 로더(작업+통지 큐)    →  (완료) 병렬 디코드 함수 + 메인에서 순서대로 GPU 텍스처 생성
Core ↔ Game 순환                 →  App(최상위) → Editor → Game → Render/Audio/World → Core → JugX
```

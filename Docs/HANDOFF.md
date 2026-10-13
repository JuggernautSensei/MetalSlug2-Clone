# HANDOFF — 다음 AI 에이전트가 가장 먼저 읽을 문서

작성 시점: 2026-10-13. 브랜치 `claude/affectionate-brahmagupta-tgqzo1` (작업 중), `main` 은 원본 코드 그대로(병합/푸시는 사용자가 함).
이 문서만 읽고 바로 작업을 시작할 수 있게 정리했다. 세부 근거는 `Docs/Wayfinder/*` 에 있고, 1번 작업이 끝나면 그 폴더는 사라진다.

## 0. 현재 상태 (사실)
- 게임: C++20 + SDL3, 소스 약 4.4만 줄, 단위 테스트 895개(전부 통과), 캡처 해시 7개 + 튜닝 해시.
- 이미 끝난 것: JugX 서브모듈 도입 및 자체 유틸 교체(Types/Handle/Result/Random/Assert/Log/Json/Math/Timer/파일 IO/Vec2),
  비동기 로더 삭제(→ `SpriteBatch` 의 병렬 디코드 함수), headless/`GameContext`/리플레이 삭제, 죽은 코드·테스트·리소스·문서 삭제,
  주석 토막화, 헤더 선언 한 줄, 헤더 값 인자 const 제거(`.cpp` 정의는 const 유지).
- **아직 안 한 것(이 문서의 작업 목록)**: 다형성 전면 도입, Slug 제거, 스타일 일괄(클래스 레이아웃 등), md 정리/통합, SimCheck 도구.
- 최신 백업 태그: `stage1-complete`, `backup-before-replay-removal`(리플레이/해시 경로가 남은 마지막 소스).
- `Trash/` 는 gitignore 된 사용자 휴지통(삭제 금지, 미참조 파일은 여기로 이동만).

## 1. 사용자 요구 (불변)
1. **결과는 논리적으로 동치**. 같은 입력이면 같은 상태 전이, 난수 소비 순서/횟수, 부동소수 연산 순서, 출력.
   "테스트 통과"는 근거가 아니다. 변경마다 "왜 동치인지" 한 줄로 설명할 수 있어야 한다.
2. JugX/JugGfx/JugNet 작성자 스타일. 기준은 **사람이 읽고 "내가 짠 코드 같은가"**. "돌아가기만 하면"이 아니다.
3. 구조를 단순하고 평평하게, C++ 관용구로. 웹/서버스러운 구조(Job/Queue/Notice/Context 묶음/콜백 체인) 금지.
4. **매우 공격적으로**: 파일/클래스/함수를 통째로 통합·삭제·새로 작성해도 된다(동치만 지키면).
5. 사용자는 허락을 기다리지 않는 자체 진행을 원한다. 단 아래 "사용자 확인 필요" 항목은 보고하고 넘어간다.
6. 문서는 적게. 작업 기록 폴더(`Docs/Wayfinder`) 같은 걸 새로 만들지 않는다.

## 2. 코딩 규칙 (사용자 확정 + JugX 코퍼스 측정치)
| 규칙 | 근거 |
|---|---|
| 클래스: **함수는 위, 변수는 아래.** 멤버 변수 전부 private. 데이터만이면 struct(public) | 코퍼스: 함수·데이터 있는 class 85개 모두 데이터가 뒤 |
| 클래스 본문의 선언은 **붙여 쓰고**, 빈 줄은 기능 묶음 경계만 | 선언 간 밀착 74% |
| `.cpp` 함수 사이는 빈 줄 1줄 | 821/822 |
| `JUG_CLASS(X, NO_COPY, NO_MOVE)` 등으로 5룰/3룰 명시 | 25곳 |
| `final` 쓰지 않음 | 6곳뿐 |
| **다형성(virtual+override) 적극 사용. C 스타일 함수 포인터 테이블 금지.** 함수 객체가 필요하면 `jug::Callable` | virtual 26 / override 76 / Callable 16 / std::function 0 |
| 문자열보다 `enum class` 먼저, 개수는 `EnumRefl`, 문자열 키는 로드 경계에서만 | 사용자 지시 |
| 헤더(.h): 선언 한 줄, 값 전달 매개변수에 `const` 안 붙임(`const&`/`const*`만) | 헤더 const 없음 460 : 123 |
| `.cpp` 정의: 값 인자에 `const` 붙임, `(` 뒤 줄바꿈 + 인자 1개/줄 | 542 : 8 |
| 주석: 짧은 토막(`// 5 fps`). 문장형/괄호 부연/배너/이력 언급/이름 반복 금지 | 사용자 지적 |
| 사전조건은 `JUG_ASSERT("문장.\n")`, 에러는 `Result`/`Error` | 16/kloc |
| **`try/catch` 절대 금지**(외부 라이브러리 경계 제외). MS2 현재 0건 | 사용자 지시 |
| 싱글턴 `GetSingleton()`, 전역 `g_` 허용 | 25 / 67 |
| pch 는 `<JugX/JugX.h>` + `using namespace jug;` 로 간다(현재 `using jug::X;` 나열은 이관 발판) | 사용자 지시. 코퍼스에는 `using namespace jug` 가 0건이므로 습관이 아니라 지시임 |
| `[[nodiscard]]`/`noexcept`/`constexpr` 적극 사용 | 45 / 5 / 34 per kloc |
| **JugX 에 이미 있는 타입을 먼저 쓴다**(Handle, Result, Scoped, Callable, Flags/IndexedFlags, EnumRefl, Timer, FileReader/Writer, JsonDeserializer/Serializer, RGBA, Random …) | — |

주의: 예전 `Docs/CodingStyle.md`(AI 가 쓴 "하드 룰": auto/람다/네임스페이스 금지, 함수 포인터 선호)는 사용자 코드와 **모순**이므로 따르지 않는다.
`final`, `auto`(반복자/구조화 바인딩 정도), 람다(알고리즘 인자) 는 사용자 코드에도 있다.

## 3. 작업 순서 (효과 순)
### T1. md 정리 및 통합 (코드 수정 없음, 먼저)
- 삭제: `Docs/Wayfinder/*`(작업 기록/분석/계획 — 단 `Plan_Restructure.md`, `NextSteps_UserDirectives.md` 는 내용을 `Docs/ROADMAP.md` 로
  흡수 후 삭제), `Docs/Portfolio_Architecture.md`, `Docs/Architecture.md`(필요한 20~40줄만 RULES 로 흡수), `Docs/CodingStyle.md`(삭제만).
- 유지/검증: 데이터 포맷 사양 5개 `AssetFormat`/`StageFormat`/`EnemyDefsSchema`/`PropSpecs`/`DepthSpec`(현재 코드와 맞는지 확인).
- 신규: `Docs/RULES.md` 하나(이 문서 2절 + 아키텍처 요약 + 데이터 포맷 요약 + 금지 + 로드맵 링크 + "AI 전용 도구" 절). 이 HANDOFF 는
  T1 이 끝나면 RULES/ROADMAP 으로 흡수하고 삭제해도 된다. README 는 빌드/실행/폴더만 남기고 코드 규칙은 RULES 로.
- 삭제 전 `grep -rn <파일명>` 으로 참조 확인. `.claude/` 의 hooks/settings 가 삭제한 md 를 참조하면 보고만.
- 커밋: `Docs: consolidate rules into Docs/RULES.md, remove legacy md`.

### T2. 동치 검증 도구 `Tools/SimCheck` (게임 코드에 검증 장치를 넣지 않는다)
- 별도 콘솔 exe 프로젝트를 `MetalSlug2_Clone.sln` 에 추가. 게임과 같은 `Source/` 를 컴파일하되 `Source/Main.cpp` 와
  `Source/Core/App.cpp` 는 제외하고 자기 `main` 을 가진다.
- 상태별(stage0, sandbox, bosslab, buslab, helilab, enemylab, proplab, title 등) 시드 + 합성 입력 스크립트로 `SimRunner` 를 N틱 돌려
  틱 수/종료 여부/상태 `DebugHash`(+선택 캡처 SHA256)를 한 줄씩 출력. 합성 입력은 오른쪽 이동 + 주기 점프/사격/폭탄,
  좌우 전환 주기 변형(예전 `MakeScriptedFrame`: 태그 `backup-before-replay-removal` 의 `Source/Main.cpp`/`Core/Replay.*` 참조).
- **기준선은 변경 전(`before-restructure` 태그) 상태에서 이 도구로 한 번 생성**해 `Tools/SimCheck/baseline.txt` 로 저장.
  이후 변경마다 출력을 diff. 한 줄이라도 다르면 되돌린다("허용 오차" 없음).
- 이 도구는 AI 전용이다. 사용자 스타일 규칙 적용 대상이 아니고 게임/README 에 설명하지 않는다(RULES 의 "AI 전용 도구" 절 한 곳만).
- 게임 클래스 안의 `DebugHash`/`HashState` 는 SimCheck 가 쓰는 동안 유지하고, 개편이 끝나면 SimCheck 쪽으로 이동/삭제 후보로 보고.
- 저장소의 기존 검증은 그대로: `Scripts/Verify.ps1`(캡처 7, 튜닝, 단위 테스트) + `Scripts/TraceLoad.ps1`.

### T3. 삭제
1. **Slug(SV-001) 전체**(사용자 확정): `Game/Player/Slug*.{h,cpp}`(SlugVehicle 1,083줄, SlugRender 281, SlugSystem 165, SlugTypes …),
   `sluglab` 상태, `StateFactory` 항목, 튜닝/정의/테스트/문서, `PlayerController` 의 탑승 분기.
2. 쓰이지 않는 Lab/옵션 재점검(`vehiclelab`/`demo`/`uidemo`), `SendCommand/onCommand` 체인.
3. `Base/Json.h/.cpp` → JugX `JsonDeserializer/Serializer` 직접 사용.
4. `Base/Math.h` 의 `using jug::X;` 나열, 자체 `RGBA`(→ `jug::RGBA`, 레이아웃/기본값 차이는 호출부에서 명시해 동치 유지), `RandRange`(→ `jug::Random`).
5. 미참조 에셋은 `Trash/` 로 이동(삭제 금지).

### T4. 다형성 전면 도입 (핵심, 위험 높음)
- **적/보스/차량**(`Game/Enemies`, 14종): 추상 `Enemy` + 파생 14개(Soldier, Bzk, Hang, FlyingBzk, Shielder, Pow, Jeep, BTank, MissleTank,
  Sarubia, Bus, Heli, HeliBomb, BigShiee). virtual: `Init/Tick/OnHit/OnDeath/TickDying/RegisterHurtboxes/SubmitParts/DeathDone/BodyFrame`.
  `EnemyWorld` 는 `Vector<Scoped<Enemy>>`(또는 슬롯 풀) + 팩토리 한 곳. `BehaviorOps`/`GetBehaviorOps` 삭제.
  **동치 핵심**: 틱 순서, 슬롯 재사용, 핸들 인덱스/토큰 규칙, `HashState` 믹스 값과 순서를 유지.
  단계: Soldier 하나를 끝까지 통과 → Pow/Shielder/Bzk/Hang/FlyingBzk → 차량 4종 → Bus/Heli/HeliBomb → BigShiee(마지막).
- **플레이어/무기**: 추상 `Weapon` + Pistol/Hmg/Shotgun/Rocket/Flame. `PlayerController`(1.2k줄) 를 이동·무기 보유·피격·리그로 분해.
  샷 RNG(`m_shotRng`, `m_flameRng`)는 별도 스트림이므로 **합치지 않는다**.
- **투사체/소품**: 종류별 분기 수를 먼저 측정해 이득 큰 곳만.
- `IGameState`(5종) 유지. `IStageHost` + `FakeStageHost`(테스트 시임)는 마지막에 재검토.
- 단계마다 커밋 1개 + SimCheck diff 0 + `Verify.ps1` PASS.

### T5. 스타일 일괄 (기계적, T4 뒤)
클래스 함수 위/변수 아래 재배치, 선언 사이 빈 줄 제거, 멤버 private 화, `JUG_CLASS` 부여, 문자열→enum, 개수 상수→`EnumRefl`,
`final` 제거, 남은 어색한 주석 삭제, pch 를 `JugX.h` + `using namespace jug` 로.

### T6. 큰 파일 정리
`BigShiee.cpp`(1.4k)는 T4 에서 분해, `StageData.cpp`(2.2k)/`StageEditor.cpp`(2.3k)/`MotionEditor.cpp`(1.2k)는 구조 개편 후 판단
(에디터는 사용 빈도에 따라 삭제 검토).

### T7. 마무리
JugX 대조 지표 재측정(clang-format 일치, 이름 규칙, 주석 밀도, 클래스 레이아웃, 헤더 const), "사람이 열어볼 파일 3개" 확인,
문서 갱신, 사용자에게 `main` 병합 안내.

## 4. 사용자 확인이 필요한 변경 (동치가 깨지므로 임의로 하지 않는다)
- 난수 생성기를 전역 하나로 합치기(스트림이 달라져 결과가 바뀜).
- 해시/캡처/튜닝이 달라지는 기능 삭제(단 Slug 삭제는 확정, 그 외 일반 플레이 경로 결과는 동치여야 함).
- `anim.json` 의 사용자 편집 데이터(`Assets/**`)는 국소 수정만.

## 5. 함정 (실제로 겪은 것)
- **값 전달 `MemoryView`/`MutableMemoryView` 에서 `const` 를 빼면** 템플릿 생성자가 복사 생성자보다 먼저 선택돼 컴파일이 깨진다
  (`AssetPath.cpp`). 원인은 JugX `MemoryView` 의 생성자 제약. 고치거나 const 를 남긴다.
- `<JugX/JugX.h>` 는 `Windows.h` 를 끌어와 `GetObject` 매크로가 `JsonReader::GetObject`(JugX 것)와 충돌할 수 있다 → `#undef GetObject`.
- `capture_uidemo` 는 벽시계 의존이라 간헐적으로 다르다(`Verify.ps1` 이 3회 재시도). `MSBuild` 병렬 빌드가 가끔 메시지 없이 exit=1 → 재실행.
- 권한 분류기가 대량 삭제와 공개 저장소 푸시를 거부한 적이 있다. 거부되면 우회하지 말고 보고. `main` 병합/푸시는 사용자 몫.
- `Assets/anim.json` 은 사용자가 에디터로 편집 중일 수 있다. 복원/덮어쓰기 금지.
- JugX 서브모듈(`Vendor/JugX`)은 수정하지 않는다. 필요 시 사용자에게 JugX 쪽 수정을 요청(이미 `Random`/`Scoped` 2건은 반영/푸시됨).
- `Trash/` 안의 `*.mp4`(270MB), 중첩 저장소, `Captures`, `Backup`(`Bus.json.mine` 포함)은 복구 불가 파일이므로 삭제하지 않는다.

## 6. 명령
```text
빌드      powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1 [-Config Debug|Release] [-Target MetalSlug2_Clone|SlugTest|JugX]
검증      powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Verify.ps1          (캡처 7 + 튜닝 + 단위 테스트)
로드      powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/TraceLoad.ps1
포맷      clang-format --style=file:.clang-format -i <파일>      (.clang-format 은 JugX 와 동일)
```
커밋은 단계마다 1개, 경로를 한정해 `git add`, 마지막 줄에 `Co-Authored-By`. `main` 병합/푸시 금지.

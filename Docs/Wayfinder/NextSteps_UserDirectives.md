# 다음 작업 지시 (사용자가 직접 정함, 2026-10-13 기록)

사용자 메시지 그대로 정리. 오늘은 실행하지 않고 기록만 한다. 검증 기준은 변하지 않는다: 빌드 + `Scripts/Verify.ps1` + 사람이 읽었을 때 "내 코드 같은가".

## 스타일 규칙
1. `struct`/`class` 안: **함수는 위, 변수는 아래.**
2. 함수 사이에 빈 줄을 띄우지 않고 **따닥따닥 붙인다.** 빈 줄은 기능별로 묶는 경계에만 둔다.
3. 문자열보다 `enum class` 를 먼저 고려한다.
4. `final` 은 쓰지 않는다.
5. class 멤버는 전부 `private`.
6. 5룰/3룰을 어기지 않는다. `JUG_CLASS(...)` 를 적재적소에 쓴다.
7. `Json.h`/`Json.cpp` 는 필요 없을 수 있다. `JsonSerializer`/`JsonDeserializer` 를 적절히 쓰면 된다.

## 헤더/pch
8. `Pch.h` 에 `using jug::X;` 를 하나씩 나열하지 말 것. **`#include <JugX/JugX.h>` 한 줄 + `using namespace jug;`** 로 간다.
   - 현재 `Base/Math.h`, `Base/Types.h`, `Base/Pch.h` 의 `using jug::...` 나열은 이관 중에 이름 충돌을 피하려고 남은 것. MS2 쪽 중복 정의가 없어졌으므로 정리 가능.
   - `Windows.h` 의 `GetObject` 매크로가 `JugX.h` 에서 들어오는 문제가 있었음(MS2 `JsonReader::GetObject`). JsonReader 를 JugX 것으로 쓰면 이름 충돌이 사라진다. 필요하면 `GetObject` 를 undef.
9. **`RGBA` 는 MS2 자체 구조체를 지우고 `jug::RGBA` 를 쓴다.**
   - 레이아웃/필드 순서/기본값(a=0 vs 255)이 다르므로 사용처(약 80곳)와 캡처 해시를 확인.
10. **헤더의 `const` 는 `const&`, `const*` 인 경우만.** 값 전달 매개변수에 `const` 를 붙이지 않는다 (예: `WriteFileBytes(const FilePath& _path, const MemoryView _bytes)` → `MemoryView _bytes`).
    - 주의: `MemoryView`/`MutableMemoryView` 값 인자는 템플릿 생성자가 복사 생성자보다 먼저 선택돼 non-const 이면 컴파일이 깨진 적이 있음(AssetPath.cpp). JugX 의 `MemoryView` 쪽 생성자 제약을 고치거나 호출부를 정리한 뒤 제거.
11. `inline constexpr size_t kActionCount = static_cast<size_t>(eAction::Count);` 같은 개수 상수는 **`EnumRefl`** 로 대체.

## 구조
12. **난수 생성기는 전역에 하나 두고 재사용.** 시스템별 `RngMt19937` 멤버/인자 전달을 걷어낸다.
    - 시뮬레이션 결정성(시드 순서)을 바꾸지 않도록 주의: 스테이지 내부 RNG 시드 `0x57A6E0`, 적/소품 `Reset(1)`, 플레이어 샷 RNG 등 현재 분리된 스트림이 있음. 하나로 합치면 결과가 바뀌므로, "결과 불변"을 포기할지 사용자와 확인 후 진행.
    - `Base/Math.h` 의 `RandRange` 도 JugX `Random<float>(rng, min, max)` 로 대체.
13. **`BehaviorOps`(함수 포인터 테이블)는 `jug::Callable` 사용** 또는 클래스화 검토.
    - 위치: `Game/Enemies/EnemyBehaviors.h`. 원래 코드(이전 AI 세션)에 있던 구조이며 이번 작업에서 만든 것이 아님.
    - 현재 구조: 평문 `Enemy`(데이터) + `EnemyDef`(정의) + 종류별 함수 테이블(`init/tick/onHit/onDeath/...`) = C 스타일 vtable 에 가까움. ECS 는 아님.
14. **`Slug`(플레이어가 타는 전차, SV-001) 전체 제거.** 안 쓰는 기능.
    - 대상: `Game/Player/Slug*.{h,cpp}`(`SlugVehicle`, `SlugTypes`, `SlugRender` 등), `sluglab` 상태, `StateFactory` 항목, 관련 튜닝/정의, 테스트(`VehicleSandboxTest` 중 Slug 관련 등), `Docs`.
    - 제거 후 영향: 해시(`synth_sluglab`)/기준선 관련 서술, 튜닝 덤프 해시가 바뀔 수 있음 → 기준 갱신 필요.

## 사용자 코멘트 (맥락)
- "많은 부분이 개선된 건 부정하지 않는다."
- "클래스화를 안 하고 싹 다 함수 포인터로 뺀 이유가 있냐, 순수 C 스타일이라 어지럽다." (13번)
- 정리 우선순위는 위 번호 순서와 무관. 한 번에 하나씩, 매번 빌드 + `Verify.ps1` 확인.

## 코퍼스에서 직접 뽑은 작성자 습관 (JugX/JugGfx/JugNet/Juggernaut, 230파일 2.8만 줄, 측정치)
| 항목 | 측정 | 결론 |
|---|---|---|
| `class` 레이아웃 | 함수와 데이터가 모두 있는 class 85개 중 **데이터가 함수 뒤 85, 앞 0** | 함수 위 / 변수 아래 (규칙 1 확인) |
| 헤더 클래스 본문 | 선언 다음 줄이 바로 다음 선언 2,404 : 빈 줄 828 (약 74% 밀착) | 붙이고 기능 묶음 경계에만 빈 줄 (규칙 2 확인) |
| `.cpp` 함수 사이 | 함수 끝 `}` 다음 빈 줄 821/822 | `.cpp` 는 항상 1줄 띄움 |
| 헤더 값 인자 `const` | const 없음 460 : const 123 | 헤더는 const 안 붙임 (규칙 10 확인) |
| **`.cpp` 값 인자 `const`** | **const 542 : 없음 8** | **`.cpp` 정의는 const 를 붙인다. (지운 것은 잘못이라 복원함)** |
| `final` | 6 (0.2/kloc) | 사실상 안 씀 (규칙 4) |
| `auto` / 람다 / `Callable` / `std::function` | 66 / 18 / 16 / **0** | 함수 객체는 `Callable`, `std::function` 안 씀. auto 는 반복자·구조화 바인딩 정도 |
| `virtual` / `override` | 26 / 76 | 다형성을 실제로 씀 → `BehaviorOps` 함수 포인터 테이블은 **다형성(virtual)으로** (사용자 지시) |
| `JUG_CLASS(`, `= delete`, `= default` | 25, 4, 55 | 5룰/3룰은 매크로로 선언 (규칙 6) |
| `JUG_ASSERT` | 458 (16/kloc) | 사전조건은 assert 로 선언 |
| `GetSingleton` / `g_` 전역 | 25 / 67 | 싱글턴·전역 허용, 구현 안은 `g_pSingleton` |
| `namespace jug {` | 152 | 라이브러리는 네임스페이스 안 |
| `using namespace jug` | **0 (코퍼스에 없음)** | 이 규칙(pch 에 `JugX.h` + `using namespace jug`)은 **사용자 지시**이지 코퍼스에서 확인된 습관이 아님 |
| EnumRefl 관련 호출 | 2 (정규식 한계, 재확인 필요) | 사용자 지시(규칙 11)를 따르되 JugX `EnumRefl.h` API 로 확인 |
| `[[nodiscard]]` / `noexcept` / `constexpr` | 45 / 5.2 / 34 per kloc | 적극 사용 |
| `try/catch` | 25 | 외부 라이브러리 경계에서만 |
| 헤더 당 `"..."` include | 평균 2.1 | 최소 include |

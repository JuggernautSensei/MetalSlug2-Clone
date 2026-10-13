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

# ROADMAP (구조 개편 계획 + 사용자 지시)


사용자 지시 (2026-10-13): "다음 삭제할 거. 구조를 바꿀 것. 전부 정리. 몬스터/플레이어/무기 같은 건 **C 스타일 함수 포인터 말고 다형성을 적극 사용**. try/catch 는 절대 쓰지 않기(외부 라이브러리 경계 제외). 매우 공격적으로."
기준 문서: `NextSteps_UserDirectives.md`(규칙/코퍼스 측정), `StyleRubric.md`. 검증 수단은 아래 0절.

## 0. 전제와 안전망
- 리플레이/해시 기준선은 삭제되었다(AI 용). 지금 남은 검증: 단위 테스트 895개 + 캡처 해시 7개(`Verify.ps1`) + 튜닝 덤프 + `TraceLoad.ps1`.
- 시작 전 `git tag before-restructure`. 단계마다 커밋 1개, 빌드 + `Verify.ps1` PASS 후 다음 단계. 실패하면 그 단계만 revert.
- 결과(화면/동작)는 불변. 단 **삭제 대상(Slug 등)과 난수 통합은 결과가 바뀔 수 있어 별도 단계에서 사용자 확인**.
- 현황 사실: MS2 에 `try/catch` 는 **0건**(JugX 라이브러리 내부의 utf8 경계에만 있음) → 유지만 하면 됨. 새 코드에서도 금지.

## 1. 삭제 (먼저, 가장 안전하고 효과 큼)
| # | 대상 | 규모 | 비고 |
|---|---|---|---|
| D1 | **Slug(SV-001) 전체**: `Game/Player/Slug*.{h,cpp}`(SlugVehicle 1,083, SlugRender 281, SlugSystem 165, SlugTypes …), `sluglab` 상태, `StateFactory` 항목, Slug 튜닝/에셋/테스트/문서 | 약 2k줄+ | `PlayerController` 의 차량 탑승 분기도 같이 삭제 |
| D2 | 쓰이지 않는 모드/옵션 재점검: `vehiclelab`/`demo`/`uidemo` 등 개발용 Lab 중 안 쓰는 것, `SendCommand/onCommand` 체인, `eHitKind::Hazard` | 사용자 확인 | 에디터/Lab 중 실제 쓰는 것만 유지 |
| D3 | `Base/Json.h/.cpp`: JugX `JsonDeserializer/Serializer` 직접 사용으로 대체, 공용 필드 리더는 호출부 쪽 정리 | 중 | 에러 문구는 이미 통일됨 |
| D4 | `Base/Math.h` 의 `using jug::X;` 나열, `RGBA` 자체 구조체, `RandRange` 삭제 → `JugX.h` + `using namespace jug`, `jug::RGBA`, `jug::Random` | 소 | `RGBA` 사용처 약 80곳, 캡처 해시로 확인 |
| D5 | 미참조 에셋 → `Trash/` 로 이동(삭제 아님) | 소 | `Prune_Assets*.md` 참조 |

## 2. 다형성 전면 도입 (핵심)
공통 규칙: 추상 베이스 + virtual, `override`, `final` 안 씀, 멤버 전부 private, `JUG_CLASS(X, NO_COPY, NO_MOVE)`, 소유는 `Scoped`, 종류 선택은 `enum class` + `switch` 한 곳(팩토리). **함수 포인터 테이블(`BehaviorOps`) 폐기.** 상태는 각 파생 클래스 멤버로.

### 2-A. 적/보스/차량 (`Game/Enemies`, 현재 `BehaviorOps` 10개 슬롯 × 14종)
- 종류: Soldier, Bzk, Hang, FlyingBzk, Shielder, Pow, Jeep, BTank, MissleTank, Sarubia, Bus, Heli, HeliBomb, BigShiee.
- 설계: `class Enemy`(추상) — `Init/Tick/OnHit/OnDeath/TickDying/RegisterHurtboxes/SubmitParts/DeathDone/BodyFrame` virtual. 14개 파생 클래스가 각자 `.h/.cpp`(이미 종류별 .cpp 가 있으므로 구조가 대응됨).
- `EnemyWorld` 는 `Vector<Scoped<Enemy>>`(또는 슬롯 풀) 보유, 생성은 `MakeEnemy(eEnemyBehavior)` 팩토리.
- 주의(결과 불변의 핵심): **틱 순서·슬롯 재사용·핸들 인덱스/토큰 규칙**이 해시/동작에 영향 → 슬롯 풀 의미를 그대로 유지하고 순회 순서를 바꾸지 않는다. `EnemyWorld::HashState` 가 읽는 필드를 파생 클래스 내부로 옮길 때 접근자를 통해 같은 순서로 믹스.
- 단계: (1) `Enemy` 베이스 + Soldier 하나로 end-to-end 통과 → (2) 단순한 종류부터(Pow, Shielder, Bzk, Hang, FlyingBzk) → (3) 차량(Jeep, BTank, MissleTank, Sarubia) → (4) Bus, Heli, HeliBomb → (5) BigShiee(1.4k줄, 마지막) → (6) `BehaviorOps`/`GetBehaviorOps` 삭제.
- 같이: `EnemyDefLoader.cpp`(1.5k줄)는 정의 파서라 구조 변경 대상은 아님. 문자열 `behavior` 는 로드 경계에서 enum 으로 한 번만 변환(이미 `FindBehavior`).

### 2-B. 플레이어/무기 (`Game/Player`, `Core/Tuning.h` 의 `eWeapon` 5종)
- `class Weapon`(추상) — `Fire/Update/Reload/IsEmpty/...` virtual. 파생: Pistol, Hmg, Shotgun, Rocket, Flame. 현재 `PlayerWeapons.cpp`(177줄)의 `switch`/발사 로직을 클래스로.
- `PlayerController`(1.2k줄) 분해: 이동/점프/조준, 무기 보유, 피격/사망, 리그(`PlayerRig`) 로 책임 분리. 상태 머신(`ePlayerState`)이 있으면 상태 객체(`PlayerState` 다형성)로 전환 검토.
- 난수: 샷 RNG(`m_shotRng`, `m_flameRng`)는 별도 스트림. **전역 단일 RNG 로 합치면 결과가 바뀐다** → 사용자 확인 후 진행(시드 순서 재정의 필요).

### 2-C. 투사체/소품/폭발 (`Game/Combat`, `Game/Props`)
- 투사체 종류(총알/로켓/화염/적탄/폭탄 …)가 enum + switch 라면 `Projectile` 다형성으로. 소품(`PropWorld`)의 종류별 처리도 동일하게 점검.
- 먼저 규모를 측정(`ProjectileSystem`, `PropWorld` 의 종류별 분기 수)하고 이득이 큰 곳만.

### 2-D. 이미 다형성인 곳
- `IGameState`(5종) 유지. `IStageHost`(+`FakeStageHost`)는 테스트 시임: 2-A 가 끝나면 `StageRunner` 테스트를 실제 `StageWorldHost` 로 돌리는 방향 재검토(테스트 의도 유지 가능할 때만).

## 3. 스타일 일괄 적용 (기계적, 구조 변경 뒤에)
| 규칙 | 방법 |
|---|---|
| 클래스: 함수 위 / 변수 아래 | 파생/신규 클래스는 작성 시 준수, 기존 클래스는 스크립트로 멤버 선언 재배치(함수 블록 → 데이터 블록) |
| 함수 사이 빈 줄 없음(기능 묶음 경계만) | 헤더 클래스 본문 정리 스크립트 |
| 멤버 전부 private | `class` 의 public 데이터 → private + 필요한 접근자(최소) 또는 `struct` 로 의도 구분 |
| 5룰/3룰 + `JUG_CLASS` | 복사/이동 선언 누락 클래스 전수 조사 후 `JUG_CLASS(X, NO_COPY, NO_MOVE)` 부여 |
| 문자열보다 enum | 문자열 키 조회가 로드 경계 밖에 남은 곳(클립 이름, 사운드 이름 등) 목록화 후 enum/EnumRefl 로 |
| 개수 상수 → `EnumRefl` | `kWeaponCount`, `kActionCount` 류 |
| `final` 없음 | grep 으로 확인 |
| 헤더 `const` | 값 인자 const 없음(완료), `.cpp` 정의는 const 유지(코퍼스 542:8) |
| try/catch 금지 | 현재 0건, 리뷰 규칙으로 유지 |

## 4. 큰 파일 정리 (2, 3 의 부산물로 일어남)
- `BigShiee.cpp` 1.4k → 클래스 분해(터렛/본체/탄막).
- `StageData.cpp` 2.2k: 파서/질의 분리는 구조 개편 후 판단(내용은 로드 경계라 다형성 이득 적음).
- `StageEditor.cpp` 2.3k, `MotionEditor.cpp` 1.2k: 탭 단위 클래스 분리 가능(에디터는 사용자 사용 빈도에 따라 후순위/삭제 검토).

## 5. 진행 순서와 담당
| 단계 | 내용 | 위험 | 담당 |
|---|---|---|---|
| P0 | 태그, 기준선 확인 | 낮음 | haiku |
| P1 | D1~D5 삭제 | 낮음~중 | sonnet(D1,D3,D4), haiku(D5) |
| P2 | 2-A 적 다형성 (Soldier 하나로 관통 → 나머지) | **높음** | **opus 설계 → sonnet 구현**, 종류별 커밋 |
| P3 | 2-B 플레이어/무기 | **높음** | opus 설계 → sonnet |
| P4 | 2-C 투사체/소품 | 중 | sonnet |
| P5 | 3 스타일 일괄(스크립트 + 리뷰) | 중 | sonnet(스크립트), haiku |
| P6 | 4 큰 파일 정리, 에디터 범위 결정 | 중 | sonnet |
| P7 | 최종 평가(JugX 대조 지표 + 사람이 열어볼 파일 3개), 문서 갱신, `main` 병합 | — | opus 검토 |

## 6. 합격 기준 (사용자 기준 그대로)
1. 사람이 열어서 "내 코드 같다": 클래스 레이아웃/주석/헤더 const/빈 줄 밀도가 코퍼스 수치와 같은 범위.
2. 함수 포인터 테이블 0개, `virtual` 기반 다형성, `try/catch` 0(경계 제외).
3. 단위 테스트 전부 통과 + 캡처 7개 동일 + 튜닝 동일.
4. 죽은 코드/리소스 0, 미참조 에셋은 `Trash/` 로.

## 7. 사용자 결정이 필요한 지점 (미리 모음)
- 난수를 전역 하나로 합치면 결과가 바뀐다 → 합칠지, 스트림 분리를 유지할지.
- Lab 계열/에디터 탭 중 삭제할 것.
- `IStageHost`+`FakeStageHost` 폐기 여부(테스트 방식 변경).
- 값 전달 `MemoryView` 의 `const` 문제(JugX 쪽 생성자 제약 수정 여부).

---

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

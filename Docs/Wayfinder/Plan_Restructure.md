# 구조 개편 계획 (매우 공격적) — 다형성 전면 도입 + 삭제 + 스타일 일괄 적용

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

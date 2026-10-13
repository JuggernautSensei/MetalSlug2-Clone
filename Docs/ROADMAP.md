# ROADMAP — 구조 재설계

2026-10-13. 규칙은 `RULES.md`, 실행 가능한 검증 명령은 이 문서와 해당 스크립트를 기준으로 한다.

## 목표

JugX의 실제 코드를 기준으로 경량 C++ 구조로 바꾼다. 작은 클래스, 명확한 소유권, 필요한 곳의 virtual/override, 함수 위·private 데이터 아래 구성을 따른다. 중간 래퍼, 반복 분기, 의미 없는 범용 상태, 개발 중 남은 코드와 문서를 줄인다.

최우선 조건은 변경 전후 동작 동치다. 상태 전이, 입력 소비, 난수 호출과 스트림, 부동소수 연산, 이벤트·스폰·렌더 제출 순서를 유지한다. 테스트와 해시 일치는 회귀를 탐지하는 근거이며, 전체 동치를 증명하지 않는다. 각 변경은 코드 수준의 동치 근거도 남긴다.

## 확인한 출발점

- 브랜치: `claude/affectionate-brahmagupta-tgqzo1`. 시작 커밋 `be19fcd`; 비교 태그 `before-restructure`.
- C++20, SDL3, miniaudio, yyjson, stb, ImGui, GoogleTest. MSBuild 프로젝트의 도구 집합은 v145.
- 개편 전 Source 183파일 / 47,900줄, Test 62파일 / 21,864줄. 공백·주석 포함 물리적 줄 수.
- 변경 전 Debug 빌드, 테스트 **895/895**, 캡처 **7/7**, 튜닝, 텍스처 로드 비교 통과.
- 리플레이 실행·생성 기능은 이미 삭제됨. `SimRunner`, `InputFrame`, `DebugHash`는 남아 있음.
- 기존 적 구조: `Vector<Enemy>` + 14종 `BehaviorOps`. 핸들은 증가하는 ID이며, 삭제 시 생존 순서를 유지하는 압축이다. 슬롯 인덱스/세대 토큰 기반 풀이 아니다.
- `PlayerController.cpp` 1,197줄, `PlayerWeapons.cpp` 177줄. 무기 판단과 발사·연사·화염 상태는 컨트롤러에 흩어져 있음.
- `BigShiee.cpp` 1,429줄, `EnemyWorld.cpp` 1,203줄, `StageData.cpp` 2,267줄, `StageEditor.cpp` 2,289줄.
- `Base/Json`(현재 `Base/JsonFields`)은 이미 JugX 디시리얼라이저를 사용함. 남은 코드는 숫자 반올림·필수 필드·키 검증 등 게임 데이터 규칙이므로 무작정 삭제하면 의미가 달라진다.
- `SendCommand`는 스테이지 스키마와 테스트에 존재하지만 적 14종의 콜백 구현은 없음. 실제 기능과 파서 호환성을 구별한다.
- `--seed`는 기존에 App에서 파싱만 하고 시뮬레이션에 전달하지 않았음. 지금은 호환을 위해 인자를 소비하되 불필요한 저장은 제거함. 기존 시드 동작을 고치는 일은 기능 변경으로 분리한다.
- `Docs/Portfolio_Architecture.md`는 시작 시점부터 있던 미추적 문서다. 이전 리플레이·테스트 수 설명이 낡았으므로 현재 아키텍처의 근거로 쓰지 않고 파일은 보존한다.

## 실행 순서

| 단계 | 작업 | 완료 조건 |
|---|---|---|
| P0 | 변경 전 검증, 외부 SimCheck 기준선 작성 | 기존 검증 통과 + 840개 체크포인트 기준선 보존 |
| P1 | 적 14종의 함수 포인터 테이블을 소유되는 Enemy 파생 클래스로 교체 | 테이블 제거, 기존 행동 본문·호출 순서 유지, 전체 검증 동일 |
| P2 | 적 상태와 공통 동작 정리 | aux/ext 의미를 종류별 이름으로 드러내고, 필요한 상태만 소유; 가독성 개선과 메모리 비용 확인 |
| P3 | 플레이어의 이동·공격·피격 책임 분리, 무기 다형성 | 무기 변경·탄 소진·연사 취소·화염 수명·입력 소비·RNG 순서 동일 |
| P4 | JugX 타입 직접 사용, 남은 래퍼·중복 제거 | JSON 반올림/오류 규칙과 RGBA 채널/기본값 유지; 전체 검증 동일 |
| P5 | 개발 흔적·미참조 코드/에셋 정리, 클래스 스타일 정리 | 사용 경로와 데이터 참조를 확인해 제거; 사용자 파일은 보존 |
| P6 | 큰 파서/에디터 파일 책임 분리, 문서 갱신 | 장황한 추상화를 추가하지 않고 사람이 진입점에서 구조를 따라갈 수 있음 |

Enemy는 공통 생명주기·물리를 소유하고, 종류별 행동 진행은 각 파생 클래스의 private 필드가 소유한다. 등반 구간과 사망 시작 높이/넘어지는 프레임은 공통 처리에 이름을 붙였다. counter/aux/ext 저장소는 제거했다.

종류별 선택은 생성 팩토리 한 곳에 둔다. 생성 시만 할당하고 틱마다 할당하지 않는다. `EnemyWorld`의 ID 증가, 지연 스폰 적용 시점, 생존 순서 압축, 후속 핸들 연결, 이벤트 순서를 유지한다. RTTI, 공유 소유권, 이벤트 버스, 작업 큐는 추가하지 않는다.

무기는 현존하는 발사 차이를 기준으로 인터페이스를 정한다. 현재 없는 Reload 시스템이나 상태 객체 프레임워크를 새로 만들지 않는다. 소품과 투사체의 작은 데이터 분기까지 무조건 클래스로 늘리지 않는다.

## 검증 방법

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Verify.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/TraceLoad.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/SimCheck/Check.ps1
```

- SimCheck는 별도 콘솔 프로젝트. 게임에 도구용 CLI나 리플레이 기록을 다시 넣지 않는다.
- 현재 13개 상태 × 좌우 전환 2종 × 60틱 간격 30개 = 780개 체크포인트. Slug 삭제 요청으로 sluglab의 60개만 제외했고, 나머지는 최초 840개 기준선의 값을 그대로 유지한다. 이동·점프·사격·폭탄·조준·시작 입력을 합성한다. 각 시나리오는 1,800틱이다.
- 게임에 이미 있는 기본 시드와 RNG 스트림을 그대로 사용한다. `--seed`를 테스트 다양성으로 계산하지 않는다.
- 최초 기준선만 `Check.ps1 -Record`로 생성. 파일이 있으면 덮어쓰기 거부. 변경 후 실패를 없애려고 기준선을 갱신하지 않는다.
- 일부 기존 HashState는 좌표를 양자화하고 모든 내부 필드/RNG를 포함하지 않는다. 체크포인트 일치는 보조 근거다. 수치 연산과 상태 갱신의 순서를 코드로 대조하고, 중요한 전환은 기존 테스트와 화면 비교로 확인한다.
- Verify는 알려진 실패 허용 목록을 제거했다. 프로세스 실패·테스트 미실행·잘못된 출력도 성공 처리하지 않는다.
- uidemo의 벽시계 캡처 재시도는 완전 동치 증명이 아니다. 동일한 렌더 방식끼리 비교한다.

## 삭제의 경계

실제로 실행되지 않는 개발 흔적은 바로 정리한다. 현재 게임·에디터·데이터에서 사용되는 경로는 이름이 Lab/Debug라는 이유만으로 삭제하지 않는다. 검증 전용 코드는 Tools/Test로 모은다.

사용자가 Slug 차량 전체 삭제를 명시했다. 차량 구현·sluglab·탑승 상태·전용 HUD·애니메이션을 제거한다. 나머지 실행 상태의 동작은 유지하고 기존 체크포인트와 비교한다.

난수 스트림 통합, 실제 사용 중인 Lab/에디터 제거, CLI 시드 의미 수정, 테스트 시임 제거도 동작 변경과 구조 변경을 구별한다. `IStageHost`는 테스트에서 실제로 여러 동작을 검증하므로 그대로 유지한다.

## 스타일 기준

실제 참고 파일: `Vendor/JugX/JugX/Application.h`, `Event.h`, `Scoped.h`, `FileReader.h`, `Timer.h`.

- 함수 위 / private 데이터 아래. 데이터 운반용 struct는 public 필드 허용.
- 헤더 선언은 한 줄, 기능별 묶음만 빈 줄. cpp 정의는 인자를 줄별로 쓰고 함수 사이 빈 줄 하나.
- 소유권은 Scoped, 필요한 상속은 virtual/override. final과 C 방식 함수 포인터 테이블을 쓰지 않는다.
- JUG_CLASS는 실제 복사·이동 요구에 맞게 사용한다. 이동이 필요한 값 타입에 NO_MOVE를 일괄 적용하지 않는다.
- enum class와 EnumRefl을 우선 고려한다. 로드 경계의 문자열 데이터는 유지한다.
- 주석은 의도나 단위가 필요한 곳에 짧게 쓴다. 불변 조건·버그 원인을 설명하는 유용한 주석까지 없애지 않는다.
- 부동소수 계산 순서를 바꾸는 유틸 대체, 색상 기본값 변경, RNG 재배치는 금지한다.

main 병합과 원격 푸시는 수행하지 않는다.

## 반영한 구조

- P0: 변경 전 검증과 SimCheck 기준선 생성 완료.
- P1: Enemy + 14종 파생 클래스, Scoped 소유권, 팩토리 생성으로 전환. 행동 본문 57개는 데이터 타입 이름과 상태 참조 바인딩을 제외하면 기존과 동일하다. 초기 전환 뒤 895개 테스트·캡처·튜닝·840개 체크포인트가 통과했다.
- P2: 공통 ext[48]을 제거했다. BigShieeData, HeliFlight, 버스 단계별 시작 틱, FlyingBzk 낙하 틱을 각 파생 클래스가 private으로 소유한다. 포탑/주포 모드는 enum이다. 보스 슬롯 접근 함수와 단순 상태 반환 중계를 제거하고, 적 14종의 행동 보조 함수를 각 클래스의 private 메서드로 옮겼다. 단순 상태 반환 중계는 제거했다. 이후 counter/aux도 제거해 발사·선회·조준·파손 진행을 각 파생 클래스의 private 필드로 옮겼다. 공통 등반/사망의 값은 역할별 필드로 분리했다.
- P3 일부: WeaponState의 조작을 멤버 함수로 모으고, HMG/화염의 발사 차이를 Weapon 파생 구현으로 이동했다. 일반 탄은 공통 Weapon 구현을 공유한다. 무기 객체는 읽기 전용 고정 인스턴스로 발사 시 할당하지 않는다.
- PlayerController.cpp는 1,197줄에서 591줄로 줄었다. PlayerMovement.cpp 271줄, PlayerAttack.cpp 336줄이다. PlayerController는 생명주기·틱 조율을 담당하고 PlayerMovement.cpp/PlayerAttack.cpp에 해당 구현을 모았다. 새 상태나 실행 순서를 도입하지 않고 기존 메서드를 이동했다.
- P4 일부: 소품 RandRange 제거, 기존 double→float 변환 유지. 공용 JSON 규칙은 JsonFields로 이름을 명확히 했고, 단순 LoadJsonFile 중계를 제거해 JugX에 직접 로드한다. 기존 BOM/주석 허용 옵션을 호출부에 명시했다.
- P5 일부: final/__forceinline 제거, 미구현 적 명령 중계 제거, Math/Handle 호환 헤더와 개발용 시작 시간 계측 제거, 헤더·구현 서식 정리. 시간 계측 로그만 제거하고 게임 상태와 텍스처 진단은 유지했다. 스테이지 명령 스키마와 테스트 계약은 유지했다.

플레이어 공격 상태의 추가 재설계는 별도 개선 후보다. 사용자 에셋과 실제 실행되는 기능을 지워 검증 기준선을 바꾸지 않는다.

현재 Release 프로젝트의 WholeProgramOptimization은 기존대로 false다. 이번 인라이닝 정리는 강제 인라이닝 지정자를 제거한 것이며, 컴파일러 옵션 변경과 Release LTO 검증은 별도 단계다.

## 구조 전환까지의 검증

이번 추가 정리 전까지 Debug 빌드, 895/895 테스트, 7/7 캡처, 튜닝, 840/840 SimCheck, 모든 TraceLoad 비교를 통과했다. ValidateStage는 22개 섹션·195개 스폰·37개 웨이브 항목·4개 게이트를 통과했다. Assets, JugX와 기존 기준선은 변경하지 않았다.

적 행동 본문 135개를 타입 이름·메서드 수신 객체·명명된 필드 매핑을 정규화해 대조했다. 모두 이전 코드와 일치했다. 버스 단계 저장은 시작 단계 0을 포함해 kStageCount + 1칸이다.

이번 추가 정리의 검증 결과는 아래 체크리스트를 기준으로 판단한다. enum 기반 타입 등 레이아웃을 바꾸면 전체 재빌드 후 비교한다. 실패한 병렬 빌드 뒤의 증분 빌드는 오래된 오브젝트가 섞일 수 있으므로 통과 근거로 쓰지 않는다.

## 요청별 완료 체크리스트

완료 표시는 구현과 검증 근거가 있을 때만 붙인다. “가장 단순하다”, “AI 흔적이 전부 없어졌다”, “전체 동치를 증명했다”는 확인할 수 없는 주장으로 완료 처리하지 않는다.

- [x] 이전 문서와 실제 코드 분석, 실행 순서 재계획: 이 문서의 출발점·P0~P6.
- [x] 계획 뒤 승인 요청 없이 구현 진행: Enemy/Weapon 구조 전환과 아래 정리 반영.
- [x] JugX 스타일과 소유권 적용: 클래스의 함수 위/private 데이터 아래, Scoped, virtual/override. 값 타입의 필요한 복사·이동은 보존.
- [x] PCH 하나에서 JugX 전체 사용: JugX/JugX.h와 using namespace jug. 개별 JugX 헤더/using 목록 없음.
- [x] 이름 충돌은 JugX 대체 먼저 평가: RGBA 대체, 게임 전용 FindEnemy/DrawString으로 변경. Vendor는 변경하지 않음.
- [x] __forceinline 제거: Source/Test/Tools 검색 결과 없음. 일반 inline/constexpr는 ODR와 헤더 정의에 필요한 경우 유지.
- [x] Cfg → Config: 적 설정 타입·파서·변수·BigShieeConfig.h와 프로젝트 등록까지 변경. c/_c도 config/_config로 펼쳐 설정임을 드러냄.
- [x] MissileTank C++ 철자 정리: 타입·enum·필드·파일 이름 변경. missle_tank 데이터 문자열과 에셋 파일명은 기존 형식 유지.
- [x] enum의 불필요한 기반 타입 제거: eAction 외 모두 기본 int. eAction은 JugX IndexedFlags가 기반 타입을 저장 타입으로 쓰므로 입력 프레임의 16비트 비트셋을 유지. 기존 상태 해시의 1바이트 enum 기록은 해시 경계에서 명시 변환.
- [x] 참조 없는 코드 삭제: DefReader::HasError, ParamValue::IsNull/IsBool, Slug shotFlashTicks/arrowGapPx, Bus kOpenTicks, 미사용 사운드 별칭 5개(kReload/kVoiceEnemyCh/kLetsGo/kHereWeGo/kMenuSelect). 동명 JugX 메서드와 실제 호출되는 게임 메서드를 구분해서 확인.
- [x] 게임 코드에 있던 테스트용 목록 이동: 사운드 이름 목록은 Test/AudioTest.cpp로 이동, 기존 테스트 항목 유지.
- [x] 불필요한 중계와 구문 정리: TrainingTargets의 정수 해시 중계 제거, StateFactory는 MakeScoped 직접 사용, float 비트 복사는 bit_cast, 중복/미사용 include 삭제. ParamValue::GetVec2의 중복 반환 경로도 정리.
- [x] 이번 추가 변경 후 전체 Debug 재컴파일/링크 성공. Verify: 895/895 테스트, 7/7 캡처, 튜닝 동일. 증분 빌드의 오래된 오브젝트로 생긴 실패는 전체 재빌드 뒤 해소됐으며 기준선은 변경하지 않음.
- [x] 이번 추가 변경 후 SimCheck 840/840 동일. TraceLoad 4개 상태의 async/sync와 기존 기준선 모두 동일.
- [x] 변경 본문 대조: 68파일은 이름·enum 선언·include·서식만 변경. 나머지 14파일은 명시한 삭제/팩토리/해시 경계/비트 변환/반환 경로 정리. 입력·RNG 호출과 게임 수치 연산·이벤트 순서는 유지. 에셋/스키마 문자열은 동일.
- [x] 전체 적의 상태 정리 검증 완료: counter/aux/ext 제거 구현 완료. 남은 탄 수·조준 자세·드리프트·선회 대기·버스 파손 단계·보스 나이를 각 종류의 private 필드로 옮김. 등반/사망은 공통 필드로 분리. 최종 전체 회귀 검증 통과.
- [x] 파서/에디터 책임 재검토 검증 완료: StageParams와 4종 Document 모델 분리, 공유 JSON 숫자 포맷 이동, 정의 파서 오류 데이터 private화 구현 완료. 모델의 Renderer/ImGui 호출·헤더 의존은 없음. 최종 전체 회귀 검증 통과.
- [x] 전체 사용 경로 감사 검증 완료: Source/Test/Tools 참조, private 필드·상수, 내부 함수, 프로젝트 등록, 진단/검증 호출 경로 검토. SoundBank/GroundMap 미호출 IsEmpty와 비어 있는 POW namespace 제거. 중복 include·단일 참조 private 필드/상수 후보 없음. 최종 전체 회귀 검증 통과.

## 읽고 설명할 수 있는 설계의 기준

1. 실행 흐름: SimRunner가 상태를 Tick하고, Stage/Sandbox가 플레이어·적·소품을 조율한다. EnemyWorld는 적 소유·생성·삭제·이벤트 순서를 책임지고 실제 종류별 행동은 해당 Enemy 파생 클래스가 맡는다.
2. 상태 위치: 공통 생명주기/물리는 EnemyState, 보스 포탑·주포는 BigShieeData, 헬기 비행은 HeliFlight, 버스 단계 시간은 BusEnemy다. 등반/사망의 공통 진행은 이름을 가진 공통 필드이고, 발사/이동/파손 진행은 해당 적의 private 필드다.
3. 상속 이유: 적은 이미 14종의 행동 차이가 있다. 함수 포인터 테이블을 클래스의 override로 바꾸면 상태와 행동을 함께 읽을 수 있다. 무기는 HMG 조준/난수와 화염 입자 발사만 다르게 구현하고 일반 탄은 공통 구현을 공유한다.
4. 비용: 적 생성 때만 객체를 할당한다. Weapon은 정적 객체이고 발사 때 할당하지 않는다. 틱마다 RTTI·공유 소유권·이벤트 버스는 추가하지 않았다. 모든 enum을 좁히기보다 실제 입력 저장 경계만 폭을 고정한다.
5. 남긴 코드의 이유: JsonFields는 게임 데이터의 반올림·키 검증을 수행하고 Aabb는 경계 포함 규칙을 갖는다. 이름이 비슷한 JugX API로 바꾸기 전에 의미가 같은지 확인해야 한다. Lab/에디터는 실제 실행·검증 경로가 있어 미사용으로 취급하지 않는다.

이번 남은 세 항목은 구현·감사·검증을 완료했다. 최종 Debug 빌드 성공, Verify 895/895 테스트와 캡처 7/7·튜닝 동일, SimCheck 840/840, TraceLoad 전부 동일, ValidateStage 통과. 사용 중인 Assets/JugX 및 기존 기준선은 변경하지 않았다. 커밋 상태는 git log와 git status를 기준으로 확인한다.

## 현재 코드의 책임과 유지 이유

| 영역 | 진입점·책임 | 유지/분리 이유 |
|---|---|---|
| 적 생명주기 | EnemyWorld::Tick → Enemy::Tick/TickDying | 월드는 ID·소유·스폰·이벤트 순서, 파생 클래스는 자신의 행동과 진행 값 |
| 적 진행 | m_shotsRemaining / m_aimPosture / m_driftTicks / m_turnWaitTicks / m_damageStage / m_ageTicks | 숫자 슬롯을 해석할 필요 없이 사용처와 필드 이름에서 의미를 읽음 |
| 공통 등반/사망 | climbSegment, deathLaunchY, deathFallFrame, deathStart | 공통 동작이 직접 필요한 값. 사망 전 자세/등반 기록과 별개로 이름을 붙임 |
| 상태 해시 | Enemy::HashActionState | 기록 경계에서 기존 다섯 64비트 값의 순서를 유지. 게임 상태에는 범용 슬롯을 다시 두지 않으며 힙 할당 없음 |
| 스테이지 데이터 | StageParser + StageParams | 문법/오류/ID 참조 검증과 소유되는 매개변수 값의 책임 분리. 스테이지 전체 참조 검증은 한 파서에 유지 |
| 정의 데이터 | EnemyDefParser / PropDefParser / DefReader / TuningReadVisitor | 상속 해석·키 검사·게임 수치 규칙은 로드 경계의 책임. 오류 문자열을 공개 가변 멤버로 노출하지 않음 |
| 에디터 모델 | AnimDocument / HitboxDocument / MotionDocument / StageDocument | 로드·검증·텍스트 패치·Undo·저장. UI와 Renderer를 호출하거나 소유하지 않음 |
| 에디터 UI | AnimEditor / HitboxEditor / MotionEditor / StageEditor | 선택·드래그·미리보기·텍스처 생명주기. 문서를 변경하는 일은 모델 API로 전달 |
| JSON 텍스트 편집 | JsonScanner + FormatJsonNumber | JSON의 문자 범위와 사용자 서식을 보존하기 위해 유지. JugX 디시리얼라이저는 의미를 읽고 검사하는 경계에서 사용 |
| 검증·진단 | Test / Tools/SimCheck / capture / trace-load | 실제 호출되는 회귀 경로이므로 미사용 코드로 분류하지 않음. Vendor·사용자 Assets·Trash는 삭제하지 않음 |

문서 모델/파서 메서드 128개의 본문은 포맷 함수·타입 이름을 정규화하면 변경 전과 동일하다. StageDocument::Reparse_의 변경은 Reader 필드 직접 쓰기를 SetRectOutput 호출로 바꾼 부분이며, setter는 동일한 포인터 대입만 수행한다. Reader의 문자열/문자 리터럴도 원본과 일치한다.

적 필드 변환은 게임 연산·상태 전이·RNG 호출의 위치를 바꾸지 않는다. Bzk는 조준 자세가 정해졌는지 표시해 등반 진행과 구별한다. 공통 사망 시작 방식은 폭발 시작 높이와 서 있는 사망의 넘어짐 프레임을 구별하고, 기록 경계에서는 기존 값의 지속 기간도 유지한다. EnemyLab에서 의미 없는 aux 슬롯 표시는 등반 구간/사망 단계 표시로 바꿨다. 이 진단 문구 변경을 게임 동작 동치와 혼동하지 않는다.

적 메서드 139개의 본문은 필드 이름·사용하지 않게 된 순수 상태 참조를 정규화하면 변경 전과 같다. Bzk::Go_는 등반 구간을 조준 자세와 분리하는 대입만 다르며, 조준 자세의 유효 여부는 기존 기록 형식을 유지할 때 사용한다. 공통 등반/사망 코드는 동일한 분기·값·읽기 시점을 유지한다.

사용 경로 감사에서는 미호출 IsEmpty API 2개와 필드 이동 뒤 남은 미사용 상태 참조 8개를 추가로 삭제했다. 프로젝트 등록에는 모든 실제 Source 파일이 포함되며, Pch.h는 ForcedIncludeFiles로 사용된다. 헤더의 단일 참조 private 필드·상수 및 중복 include 후보는 남아 있지 않다. 이는 감사 범위의 완료이며, 정적 검색만으로 모든 미래 입력의 동치나 수학적 최적성을 증명했다는 뜻은 아니다.

## 미사용 재료 정리

- [x] Assets 63개 파일의 코드·JSON 참조와 폴더 열거 로드 경로를 확인했다. 실제 사용하는 에셋은 보존했다.
- [x] Assets/Stage/BossArena.json은 참조되지 않는 옛 구간 조각이다. 스테이지 필수 필드가 없어 독립 스테이지로 읽을 수 없으며, 현재 보스 구간은 Stage0.json에 있다. Trash/UnusedAssets_20261013로 이동해 활성 에셋과 에디터 목록에서 제거했다.
- [x] 이번 작업의 비교 사본 폴더 2개, 일회성 변환 스크립트 2개, 중간 로그 12개, 커밋 메시지 임시 파일 1개를 Trash/TemporaryRefactor_20261013로 옮겨 Intermediate에서 제거했다. 기존 백업·영상·사용자 문서와 실제 빌드/검증 도구는 보존했다. 최신 빌드/SimCheck 로그는 현재 진단 자료로 유지했다.
- [ ] 임시 재료의 영구 삭제: 자동 승인 검토가 Remove-Item 실행을 blocked by policy로 거절하여 수행하지 못했다. 활성 작업 폴더에서 정리한 파일은 Trash에 남아 있다.

## Slug 차량 제거

- [x] 차량 코드 9파일, 상태 팩토리, Sandbox 연결, 플레이어 탑승 API/상태, 차량 HUD/게이지/깊이 항목, 프로젝트 등록 제거.
- [x] Slug 전용 애니메이션 71개와 PlayerStateUI_Cannon 스프라이트 제거. 다른 JSON 항목은 값과 서식을 보존. 차량 튜닝은 삭제한 SlugTypes에만 있었으며 공용 Tuning.json에는 없었음.
- [x] 모든 atlas PNG에는 다른 애니메이션/스프라이트가 함께 있어 공유 파일을 유지. PlayerHPUI는 보스 체력 표시에도 사용되어 유지.
- [x] SimCheck의 sluglab 60개만 삭제. 나머지 780개 기준선은 값/순서 그대로 유지. 기존 팩토리 테스트에 sluglab 생성 실패 확인 추가.
- [x] Debug 빌드 성공. Verify 895/895 테스트·캡처 7/7·튜닝 동일, SimCheck 780/780 기존 기록과 동일, TraceLoad 4상태 async/sync·기준선 동일, ValidateStage 통과. 첫 병렬 검증에서 uidemo 캡처가 달랐지만 단독 Verify 재실행은 전체 통과했다. 캡처 기준선은 변경하지 않았다.

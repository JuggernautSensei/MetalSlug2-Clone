# RULES — 다음 작업자가 먼저 읽을 규칙

2026-10-13. 구조와 작업 순서는 `ROADMAP.md`에 모은다. 별도 작업 기록 폴더를 만들지 않는다.

## 최우선 요구

1. JugX는 사용자의 스타일 기준이다. 실제 파일을 읽고 구조·이름·소유권·클래스 배치를 따른다.
2. 경량 C++로 작성한다. 필요한 곳에 virtual/override를 사용하고 C 방식 함수 포인터 테이블을 없앤다.
3. 사람이 호출 흐름과 책임을 따라갈 수 있게 작성한다. 범용 프레임워크, 작업 큐, 알림 계층, 서비스를 포인터 묶음으로 운반하는 구조를 늘리지 않는다.
4. 논리적으로 동치이면 파일·클래스·함수를 적극적으로 삭제/통합/재작성해도 된다.
5. 수정 전후의 상태 전이, 입력 소비, 난수 소비, 수치 계산, 이벤트와 렌더 순서를 유지한다. 테스트 통과만으로 동치를 주장하지 않는다.
6. 사용자가 계획 후 즉시 코딩을 요청했다. 통상적인 구현 판단은 자체 진행한다.

## 코드 스타일

| 대상 | 규칙 |
|---|---|
| class | 함수 위, 데이터 아래; 멤버 데이터는 private |
| struct | 데이터 운반용 public 필드 허용; 함수가 있으면 위에 배치 |
| 헤더 | 선언 한 줄; 값 전달 인자는 const 없음; const&/const* 유지 |
| cpp | 값 전달 인자는 const; 정의의 인자는 줄별로, 함수 사이 빈 줄 하나 |
| 선언 묶음 | 헤더의 연속 선언은 붙이고 기능 경계에만 빈 줄 |
| 생명주기 | JUG_CLASS를 실제 복사/이동 필요에 맞게 사용 |
| 다형성 | virtual/override 사용; final을 쓰지 않음 |
| 소유권 | Scoped 우선; 공유 소유권과 RTTI를 불필요하게 도입하지 않음 |
| 종류 | enum class 우선; 문자열은 데이터 로드 경계에 유지 |
| 주석 | 단위·불변 조건·의도를 짧게; 이력·배너·코드 반복 설명 제거 |
| 실패 | 사전조건은 JUG_ASSERT, 오류 반환은 Result/Error |
| 예외 | try/catch 금지; 외부 라이브러리 경계만 예외 |
| 라이브러리 | JugX에 있는 타입과 함수를 우선 확인 |
| 인라이닝 | 프로젝트 __forceinline 제거; 헤더의 ODR용 inline과 constexpr만 유지 |

참고: `Vendor/JugX/JugX/Application.h`, `Event.h`, `Scoped.h`, `FileReader.h`, `Timer.h`. auto와 람다는 사용자 코드에도 있으므로 전면 금지하지 않는다. 서로 다른 난수 스트림을 전역 하나로 통합하지 않는다.

## 보호할 파일과 상태

- `Assets/anim.json` 등 사용자 에셋은 복원/덮어쓰기 금지. 수정이 필요하면 국소 변경만.
- `Vendor/JugX` 서브모듈은 수정하지 않는다. 라이브러리 결함과 애플리케이션 개편을 구분한다.
- `Trash/`, 기존 Captures/Backup, 영상, 중첩 저장소는 삭제하지 않는다. 미참조 자산은 Trash로 이동한다.
- 시작 당시 미추적 `Docs/Portfolio_Architecture.md`는 사용자 소유 파일로 보존한다.
- main 병합과 원격 푸시는 사용자가 한다.
- 변경 전 비교점: `before-restructure` → `be19fcd`. 이전 복구 태그: `stage1-complete`, `backup-before-replay-removal`.

## 검증

게임 변경 후 빌드와 `Scripts/Verify.ps1`, `Scripts/TraceLoad.ps1`, `Tools/SimCheck/Check.ps1`을 실행한다. 최초 비교점에서 테스트 895개·캡처 7개·튜닝·텍스처 비교가 통과했다. 동치 변경의 실패를 덮기 위해 기준선을 갱신하지 않는다.

AI 전용 SimCheck는 별도 프로젝트에서 게임 Source를 컴파일하되 Main/App를 제외한다. 840개의 고정 입력 체크포인트를 비교한다. 게임에 검증용 리플레이/CLI를 다시 넣거나 README에 이 도구를 설명하지 않는다. 해시의 누락·양자화 한계는 ROADMAP에 명시한다.

각 변경의 동치 근거는 코드와 diff에서 확인 가능해야 한다. 함수 본문 이동은 토큰 대조, 기본 처리 이동은 동일한 분기·쓰기 순서 대조, 타입 교체는 기본값/레이아웃/반올림/난수 호출을 확인한다.

## 주의점

- MemoryView/MutableMemoryView 값 인자에서 const 제거는 JugX 템플릿 생성자 선택을 바꿀 수 있다. 선언과 정의를 무작정 치환하지 않는다.
- JugX.h를 넣으면 Windows.h의 GetObject 매크로와 JSON 메서드가 충돌할 수 있다. 라이브러리 API의 GetObject에 한해서 Windows 매크로를 undef한다. 게임의 FindEnemy/DrawString, 테스트의 GetLoadMessage는 용도가 다른 기능이므로 이름을 변경했다.
- PCH는 JugX/JugX.h + using namespace jug로 통합했다. 자체 RGBA는 제거했다. 사용처는 r/g/b/a를 명시적으로 초기화하고 채널별로 사용한다. JugX RGBA의 packed 저장 영역이나 equality를 게임에서 사용하지 않는다. 이름 충돌은 JugX 대체 가능성을 먼저 검토한 뒤 게임 이름을 변경한다.
- Random<float>는 이전 RandRange의 uint64→double→float 순서와 다르다. 소품 난수는 Random<double> 뒤 float 변환을 유지한다.
- uidemo 캡처는 벽시계 영향이 있다. 재시도 성공을 완전 동치의 증명으로 쓰지 않는다.
- SimCheck/게임/테스트를 빌드하는 중 소스 파일을 수정하지 않는다. 실행 중인 검증이 끝난 뒤 다음 코드 변경을 한다.

## 실행 명령

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Verify.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/TraceLoad.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/SimCheck/Check.ps1
```

소스 목록 변경 시 게임, SlugTest, SimCheck의 vcxproj와 기존 filters를 함께 갱신한다. clang-format은 저장소의 `.clang-format`을 사용한다. 커밋한다면 변경 경로를 한정하고 마지막 줄에 Co-Authored-By를 쓴다.

## 가독성 판단

작성자가 읽고 구조를 파악하며 설계 이유를 설명할 수 있어야 한다. 줄 수나 파일 수만 줄이지 않는다. 추가 클래스/함수는 실제 상태 소유권이나 동작 차이를 설명해야 한다. 불필요한 중계·미사용 API를 남기지 않는다.

설정 이름은 Config로 쓴다. enum class는 기반 타입을 생략하는 것을 기본으로 하고, 저장 형식·비트셋의 폭이 필요한 경계만 명시한다. eAction은 IndexedFlags의 16비트 입력 저장 폭을 유지한다. 해시 형식의 폭은 HashState 경계에서 변환한다.

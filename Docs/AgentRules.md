# 에이전트 공통 규칙 (MyMetalSlug2)

## 프로젝트
- 저장소: `C:\Users\Ahnjiwoo\Desktop\Projects\windowAPIProject(24.02.05)\MyMetalSlug2` (CMake + vcpkg + VS 2026, C++20, 네임스페이스 `slug`, JugX 라이브러리 `ThirdParty/JugX` = 네임스페이스 `jug`, `<JugX/Xxx.h>`, JugX `pch.h` 자동 force-include).
- 전체 계획: `C:\Users\Ahnjiwoo\.claude\plans\zippy-dreaming-beaver.md`. 문서: `Docs/*.md` (AssetFormat, PlayerSpec, EnemySpecs, EnemyDefsSchema, StageFormat).
- 시뮬레이션은 고정 60Hz(`SimRunner`), 라이브와 헤드리스(`SlugHeadless`, 테스트)가 같은 `IGameState::Tick` 경로. 모든 판정은 `CombatSystem`/`HitResolver` 단일 경로. 좌표는 `jug::VECTOR2` float 월드 픽셀, +y 아래, **엔티티 원점 = 발 중앙**.

## 코드 스타일 (JugX 규약, 엄수)
`#pragma once`; Allman 중괄호, 4칸 들여쓰기, 열 제한 없음; 타입/함수 PascalCase; `enum class eName`(값 PascalCase); 상수 `kName`; 멤버 `m_camelCase`(bool `m_bX`, 포인터 `m_pX`); 함수 매개변수 `_name`; private 메서드는 이름 끝 `_`; POD 구조체 필드는 접두어 없이; `[[nodiscard]]`, `constexpr`, concepts, `std::span`(`jug::Span`) 적극 사용; 컨테이너 접근자 `GetSize()/IsEmpty()`; 에러는 `jug::Result<T>`/`jug::Result<void>` (`Result<void>` 는 `jug::Error` 만 받음: `return jug::Error { eError::X };`, 성공 `return jug::Error { jug::kOK };`; 새 에러 코드는 `Source/Core/Error.h/.cpp` 에 추가 가능); 예외/RTTI 금지(`dynamic_cast` 불가), shared_ptr 그물 금지(값 타입/인덱스/핸들); 전제조건은 `JUG_ASSERT`; 주석은 한글, `// ====` 배너 구분. 포맷은 clang-format 이 있으면(`C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe`) 저장소 `.clang-format` 으로 실행, 없으면 손으로 맞춤. 새 소스는 CRLF/LF 어느 쪽이든 무방(.gitattributes 가 정규화).

## 빌드/테스트 (PowerShell 도구 사용)
- 빌드: `powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1 [-Target <t>] [-Reconfigure]` (전역 뮤텍스로 직렬화됨. 다른 에이전트가 동시에 빌드한다. Bash 도구에서는 스크립트 실행이 막혀 있으므로 PowerShell 도구로 실행). 새 .cpp/테스트 파일을 추가한 직후엔 두 번 빌드해야 glob 이 반영될 수 있음(`-Reconfigure` 로 해결).
- 다른 에이전트의 작업 중인 파일 때문에 빌드가 깨지면 잠시 후 재시도하고, **남의 파일은 수정하지 말 것** (소유 목록 참고). 정말 필요한 공유 파일 수정은 최소·추가형(additive)으로만 하고 보고서에 적는다.
- 테스트: `Build\vs\bin\Debug\SlugTest.exe` (저장소 루트에서 실행). 기존 테스트는 모두 통과 상태를 유지한다.
- 라이브 창 실행은 짧게(`MyMetalSlug2.exe --state <name> --capture <prefix> --capture-ticks a,b,c` 로 결정적 캡처 → BMP→PNG 변환 후 Read 도구로 확인). 프로세스를 남겨두지 말 것.
- **git add / git commit 금지** (리드가 커밋한다). `ThirdParty/**` 수정 금지.

## 공통 보고 형식
마지막에 간결히: 만든/수정한 파일, 공개 API(헤더 경로 + 핵심 시그니처), 테스트 개수/결과, 시각 검증에서 본 것, 알려진 문제.

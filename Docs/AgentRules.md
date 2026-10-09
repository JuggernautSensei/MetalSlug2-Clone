# 에이전트 공통 규칙 (MyMetalSlug2)

## 프로젝트
- 저장소: `C:\Users\Ahnjiwoo\Desktop\Projects\windowAPIProject(24.02.05)\MyMetalSlug2` (Visual Studio 2026 솔루션 `MyMetalSlug2.sln` + vcpkg 매니페스트 모드, C++20, 네임스페이스 `slug`). 외부 의존은 vcpkg 의 SDL3, yyjson, stb, miniaudio 뿐이며 자체 기반 레이어는 `Source/Base/` (`Types.h`, `Math.h`(`Vec2`), `RGBA.h`, `Error.h`/`Result.h`, `Json.h`, `Handle.h`/`HandleStorage.h`/`ResourcePool.h`, `Random.h`, `EnumFlags.h`, `Log.h`, `Assert.h`). 헤더는 `"Base/Xxx.h"` 로 포함하고 PCH 강제 포함에 기대지 않는다.
- 전체 계획: `C:\Users\Ahnjiwoo\.claude\plans\zippy-dreaming-beaver.md`. 문서: `Docs/*.md` (AssetFormat, PlayerSpec, EnemySpecs, EnemyDefsSchema, StageFormat).
- 시뮬레이션은 고정 60Hz(`SimRunner`), 라이브와 헤드리스(`SlugHeadless`, 테스트)가 같은 `IGameState::Tick` 경로. 모든 판정은 `CombatSystem`/`HitResolver` 단일 경로. 좌표는 `slug::Vec2` float 월드 픽셀, +y 아래, **엔티티 원점 = 발 중앙**.

## 코드 스타일 (JugX 를 참고한 규약, 엄수)
`#pragma once`; Allman 중괄호, 4칸 들여쓰기, 열 제한 없음; 타입/함수 PascalCase; `enum class eName`(값 PascalCase); 상수 `kName`; 멤버 `m_camelCase`(bool `m_bX`, 포인터 `m_pX`); 함수 매개변수 `_name`; private 메서드는 이름 끝 `_`; POD 구조체 필드는 접두어 없이; `[[nodiscard]]`, `constexpr`, concepts, `std::span`(`slug::Span`) 적극 사용; 컨테이너 접근자 `GetSize()/IsEmpty()`; 에러는 `slug::Result<T>`/`slug::Result<void>` (`Result<void>` 는 `slug::Error` 만 받음: `return slug::Error { eError::X };`, 성공 `return slug::Error { slug::kOK };`; 새 에러 코드는 `Source/Base/Error.h/.cpp` 에 추가); 예외/RTTI 금지(`dynamic_cast` 불가), shared_ptr 그물 금지(값 타입/인덱스/핸들, 단일 소유는 `std::unique_ptr`); 전제조건은 `SLUG_ASSERT`; 로그는 `SLUG_LOG_*`; 주석은 한글, 장식용 배너 주석 금지(필요한 경우 짧게 한 줄). 포맷은 clang-format 이 있으면(`C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe`) 저장소 `.clang-format` 으로 실행, 없으면 손으로 맞춤. 새 소스는 CRLF/LF 어느 쪽이든 무방(.gitattributes 가 정규화).

## 빌드/테스트 (PowerShell 도구 사용)
- 빌드: `powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1 [-Config Debug|Release] [-Target <프로젝트명>]` (MSBuild 로 `MyMetalSlug2.sln` 을 빌드, 전역 뮤텍스로 직렬화됨. 다른 에이전트가 동시에 빌드한다. Bash 도구에서는 스크립트 실행이 막혀 있으므로 PowerShell 도구로 실행). 출력은 에러 요약과 `exit=<code>`, 전체 로그는 `Intermediate\last_build.log`. CMake 는 쓰지 않는다. 소스 파일을 추가/삭제/이동하면 `powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/GenerateProjects.ps1` 로 `.sln`/`.vcxproj`/`.filters` 를 다시 생성한다(소스 목록은 명시적으로 기록됨, glob 없음).
- 다른 에이전트의 작업 중인 파일 때문에 빌드가 깨지면 잠시 후 재시도하고, **남의 파일은 수정하지 말 것** (소유 목록 참고). 정말 필요한 공유 파일 수정은 최소·추가형(additive)으로만 하고 보고서에 적는다.
- 실행 파일: `Output\Debug\MyMetalSlug2.exe`, `Output\Debug\SlugTest.exe`, `Output\Debug\SlugBaker.exe`, `Output\Debug\SlugHeadless.exe` (Release 는 `Output\Release\`). Assets 는 실행 파일 위치에서 상위로 올라가며 자동 탐색(`slug::FindAssetRoot()`)하므로 어느 작업 폴더에서 실행해도 된다. 테스트는 `Output\Debug\SlugTest.exe` (`Assets\Baked` 가 없으면 먼저 `Scripts/Bake.ps1`). 기존 테스트는 모두 통과 상태를 유지한다.
- 라이브 창 실행은 짧게(`Output\Debug\MyMetalSlug2.exe --state <name> --capture <prefix> --capture-ticks a,b,c` 로 결정적 캡처 → BMP→PNG 변환 후 Read 도구로 확인). 프로세스를 남겨두지 말 것.
- **git add / git commit 금지** (리드가 커밋한다). `ThirdParty/**` 수정 금지.

## 공통 보고 형식
마지막에 간결히: 만든/수정한 파일, 공개 API(헤더 경로 + 핵심 시그니처), 테스트 개수/결과, 시각 검증에서 본 것, 알려진 문제.

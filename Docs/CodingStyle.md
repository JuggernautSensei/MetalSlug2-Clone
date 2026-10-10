# JuggernautSensei 코딩 스타일 가이드 (MetalSlug2 클론용)

분석 대상: `jugx`(2026-09, 최신) > `juggfx`(2026-10, `jug` 네임스페이스 그래픽스 레이어, jugx 위에 얹힘) > `jamengine`(2026-06, 구버전).
세 저장소가 충돌하면 **jugx / juggfx 를 따른다**. jamengine 은 "이전 세대"이며 차이점은 각 절 끝의 `[차이]` 로 표기한다.
(juggfx 와 jugx 는 같은 `jug` 네임스페이스·같은 `.clang-format` 을 쓰므로 사실상 한 세대다.)

이 문서의 경로는 `/home/user/juggernautsensei/` 기준 상대 경로다 (`jugx/JugX/Timer.h` 등).

## 0. 최우선 규칙 (저장소와 충돌하면 이 규칙이 이긴다)

사용자의 하드 룰. 저장소 코드가 어겨도 **새 코드는 반드시 따른다.**

1. **C 스타일 배열 금지.** `int a[8]` 대신 `ARRAY<int, 8>`(= `std::array`). 저장소는 jamengine 에 C 배열이 100개 이상, jugx 에도 일부(`Float16.h` 의 union 등) 있지만 따라 하지 않는다.
2. **`auto` 금지.** 저장소는 `const auto it = m_entities.find(_uid);` 처럼 이터레이터/복잡 타입에 `auto` 를 쓰지만(jamengine 349곳, jugx 25곳) 따라 하지 않는다. 타입을 명시한다: `const HashMap<UID, Entity>::const_iterator it = ...` 가 길면 `using` 별칭을 만들어 쓴다 (저장소도 `using AssetMapIter = AssetMap::iterator;` 같은 별칭을 흔히 쓴다 → `jamengine/JamEngine/AssetManager.h`). 예외 없이 `auto` 키워드 자체를 쓰지 않는다 (`auto` 반환형, `auto&` 구조적 바인딩, 제네릭 람다 포함).
3. **람다 금지**, 단 `<algorithm>` 류 호출(`std::sort`, `std::find_if`, `std::remove_if` 등)의 **인자로 넘기는 경우만** 허용. 변수에 담은 람다, 콜백 등록용 람다, 지연 실행용 람다 금지. 저장소 jamengine/jugx 에는 람다가 일부 있으나 따르지 않는다. 대신 `private` 멤버 함수 `Foo_()` 또는 익명 네임스페이스의 `Foo_()` 자유 함수를 쓴다.

참고로 저장소 자체는 이 규칙들과 반대로 `Callable<void()>`(`jugx/JugX/Callable.h`)·`Delegate<>` 같은 자체 함수 래퍼를 쓰지만, 클론에서는 위 규칙 때문에 "함수 포인터 / 멤버 함수 포인터 / 인터페이스 + virtual" 로 대체한다.

이 프로젝트에는 이미 `Docs/AgentRules.md` 의 "코드 스타일" 절이 있고, 그것은 이 문서와 같은 계열이다. 두 문서가 다르면 `AgentRules.md` 가 프로젝트 전용 결정이다 (특히 "장식 배너 주석 금지" — 7절 참고).

---

## 1. 파일 레이아웃

### 1.1 헤더/소스 분리, 가드

- 헤더는 `#pragma once` 단독. `#ifndef` 가드 금지. 헤더 첫 줄이 곧 `#pragma once`.
- 클래스 선언은 `.h`, 구현은 `.cpp`. **작은 접근자/한 줄 함수도 비템플릿 클래스라면 `.cpp` 로 뺀다** (`Timer.h` 는 선언만, `Timer.cpp` 에 `GetElapsedCount()` 구현). 인라인 본문은 템플릿, `constexpr`, 한 줄짜리 virtual override(`JUG_EVENT_BODY`)에 한해 헤더에 둔다.
- 템플릿 대형 구현도 헤더 안에 클래스 본문 내 정의 (`jugx/JugX/RingBuffer.h`, `ResourcePool.h`). `.inl` 은 수학처럼 free 함수 템플릿이 길 때만 (`jugx/JugX/Math.inl`, `Matrix.inl`).
- 템플릿 전용 헤더도 짝 `.cpp` 를 둔다. 내용은 `#include "pch.h"` 와 자기 헤더 두 줄뿐 (헤더 단독 컴파일 검증용):

```cpp
// jugx/JugX/RingBuffer.cpp
#include "pch.h"
#include "RingBuffer.h"
```

### 1.2 include 순서

`.cpp` 의 순서는 고정이다.

1. `#include "pch.h"` (PCH 가 있을 때 가장 먼저)
2. 자기 헤더
3. (빈 줄) 표준 라이브러리 `<...>` 알파벳순
4. (빈 줄) 프로젝트 헤더 `"..."` 알파벳순 (자기 모듈의 다른 헤더)

```cpp
// jugx/JugX/Logger.cpp
#include "pch.h"
#include "Logger.h"

#include <format>

#include "EnumArray.h"
#include "EnumFlags.h"
#include "TimeStamp.h"
#include "Typedef.h"
```

- 헤더 안의 include 는 **최소**. 헤더끼리는 자기가 직접 쓰는 "바로 아래 계층" 헤더만 `"..."` 로 넣고, 표준/타입 별칭은 PCH(`pch.h` → `Typedef.h`)에 기댄다 (`jugx/JugX/Timer.h` 는 include 가 하나도 없다).
  - **[이 프로젝트에서는 다름]** `Docs/AgentRules.md` 가 "PCH 강제 포함에 기대지 않는다" 고 명시했다. 즉 클론의 헤더는 `#include "Base/Types.h"` 를 직접 쓴다. 저장소의 "헤더 비자기완결" 습관(jugx `CMakeLists.txt` 주석이 인정함)은 따라 하지 않는다.
- 전방 선언 허용, 파일 상단 `namespace` 밖에 3rd-party 타입을 선언하기도 한다 (`juggfx/JugGfx/ImGuiRenderer.h` 의 `struct ImDrawData;`).
- 헤더 include 에는 정렬 설정 없음 (`SortIncludes: false`). 사람이 직접 알파벳순 정렬.

### 1.3 PCH

- 이름은 `pch.h` / `pch.cpp`. `pch.h` 는 얇다. 자체 기반 헤더만 모아 둔다:

```cpp
// jugx/JugX/pch.h
#pragma once
#include "Assert.h"
#include "Config.h"
#include "Fatal.h"
#include "Macro.h"
#include "Platform.h"
#include "Typedef.h"
```

- 모든 `.cpp` 의 첫 줄이 `#include "pch.h"`. 헤더에는 `pch.h` 를 넣지 않는다.
- juggfx 는 `pch.h` 에서 `<JugX/JugX.h>` 를 끌어온다 (라이브러리 위 계층이 하위 라이브러리를 포함). `jamengine/JamEngine/pch.h` 는 windows/std/d3d/entt/fmt/imgui 를 주석 소제목(`// std`, `// directx`)으로 묶는 무거운 형태 `[차이]`.

### 1.4 파일 이름과 폴더

- 파일명은 **PascalCase**, 확장자 `.h` / `.cpp` / `.inl`. (`.hpp` 안 씀.) 내용물의 주요 타입명과 같다: `Timer.h` ↔ `class Timer`.
- 폴더 구조는 평평하다. jugx 는 `JugX/` 한 폴더에 160여 개 파일이 몰려 있고, 서브폴더 대신 **접두/접미 이름**으로 묶는다: `Vector2/3/4.h`, `JsonSerializer.h`, `JsonDeserializer.h`, `JsonAdlSerializer.h`, `FileReader.h`, `FileWriter.h`, `ConsoleLogger.h`, `FileLogger.h`, `MemoryLogger.h`, `GroupLogger.h`. 프로젝트 루트 하위는 `JugX/ Test/ Bench/ Scripts/ cmake/`.
- 구현 상세는 `XxxDetail.h/.cpp` (`juggfx/JugGfx/ModelDetail.h`, `ImageDetail.h`) 또는 `Xxx_backend.cpp` (`ModelImporter_cgltf.cpp`, `ModelImporter_ufbx.cpp`).
- 3rd-party 헤더를 감싸는 얇은 파일은 `XxxInclude.h/.cpp` (`GraphicsInclude.h`, `ImGuiInclude.h`, `OsInclude.h`). 전부 `Vendor/` 에 두거나 vcpkg.
- **한 클래스 한 파일이 기본**이지만 엄격하지 않다: 밀접한 타입은 같이 둔다 (`Error.h` = `IErrorCategory` + `Error`, `Logger.h` = `eLogLevel`, `eLogPattern`, `Logger`, `Graphics.h` 내부 `*D3D11` 구조체 다수). 관련 enum 은 그 enum 을 쓰는 클래스 헤더 상단에 같이 둔다.
- 모듈 대표 헤더(`JugX.h`, `JugGfx.h`): include 만 모은 "umbrella" 헤더.
- 파일은 **UTF-8 BOM** 이 붙어 있는 경우가 많다 (jugx/juggfx 거의 전부, jamengine 은 ASCII). 새 파일은 BOM 유무 무관하나 .editorconfig/.gitattributes 가 정규화한다는 `AgentRules.md` 에 따른다.

---

## 2. 네이밍

| 대상 | 규칙 | 예 (출처) |
|---|---|---|
| 네임스페이스 | 소문자 약어 `jug` (jamengine 은 `Jam`) | `namespace jug` |
| 내부 네임스페이스 | `xxx_detail` (snake_case, 소문자) | `ring_buffer_detail`, `handle_detail`, `enum_relf_detail` (`jugx/JugX/RingBuffer.h`, `Handle.h`) |
| 클래스/구조체 | PascalCase | `RingBuffer`, `HandleAllocator` |
| 인터페이스 | `I` + PascalCase | `IErrorCategory` (`Error.h`) |
| enum 타입 | **`enum class` 필수 + `e` 접두** | `enum class eLogLevel`, `eTimeUnit`, `ePass` |
| enum 값 | PascalCase, 접두 없음 (`None`/`Count` 같은 규칙값 포함) | `Trace, Info, Warn, Error, Fatal` |
| 비트플래그 enum | 값은 `1 << n`, 첫 값 `None = 0` | `eLogPattern` (`Logger.h`) |
| 함수(공개/protected) | PascalCase | `GetElapsedCount()`, `ReportVAssert()` |
| 함수(private/내부) | PascalCase + **후행 `_`** | `LogPrefix_()`, `InitSystems_()`, `MakeMask_()` |
| 익명 네임스페이스 자유 함수 | 후행 `_` | `MakeKeyMods_()` (`Application.cpp`), `ToVertexSemantic_()` (`Graphics.cpp`) |
| 멤버변수 | `m_` + camelCase | `m_alpha`, `m_ticksLeft` |
| 멤버 bool | `m_b` + PascalCase | `m_bRunning`, `m_bHandled` |
| 멤버 포인터 | `m_p` + PascalCase | `m_pObj`, `m_pContainer`, `m_pCategory` |
| 함수 매개변수 | **`_` + camelCase** | `_deltaTimeSec`, `_pCaller` |
| 매개변수 bool/포인터 | `_b…`, `_p…`, `_xxxOrNull` | `_bEnable`, `_pGraph`, `_executorOrNull` |
| 지역변수 | camelCase (접두 없음) | `elapsedCount`, `hashIt` |
| 지역 bool | `b` + PascalCase | `bAnyLogged` (`Logger.cpp`) |
| 지역 포인터 | `p` + PascalCase | `pField`, `pElement`, `pDst` (`JsonSerializer.cpp`, `Callable.h`) |
| 파일 상수/`constexpr` | **`k` + PascalCase** | `kDefaultCapacity`, `kMaxToken` |
| 상수 bool | `kb` + PascalCase | `kbBitmaskItself`, `kbConst` (템플릿 non-type 인자) |
| 정적 멤버 | `s_` + camelCase (jamengine) | `s_pInstance` |
| 파일 전역(싱글턴 포인터) | 익명 NS 안 `g_p` + PascalCase | `g_pSingleton` (`Application.cpp`, `Graphics.cpp`) |
| 템플릿 타입 인자 | 단일 대문자 / 짧은 PascalCase: `T`, `U`, `K`, `V`, `H`, `E`, `Alloc`, `Fn`, `Args` | `template<typename K, typename V, typename H = Hasher<K>, ...>` |
| 템플릿 non-type 인자 | `k` + PascalCase | `template<typename T, size_t kSize>` |
| 템플릿 내부 별칭 | `xxxT` 접미 | `ValueT`, `ListT`, `HashMapT`, `ContainerT`, `UnderlyingT` |
| concept | `…T` 접미 | `EnumT`, `HandleT`, `ErrorCodeT`, `EventT`, `VectorScalarT` |
| 매크로 | `JUG_` + UPPER_SNAKE | `JUG_ASSERT`, `JUG_CLASS`, `JUG_FATAL` |
| 매크로 매개변수 | `_` + 소문자 | `JUG_CONCAT(_a, _b)`, `JUG_CLASS_NO_COPY(_class)` |
| 태그 타입 | `XxxType` + 상수 `kXxx` | `NoInitType kNoInit`, `OkType kOK`, `NullHandleType kNullHandle` (`Tag.h`, `Error.h`) |
| 핸들 상수 | `kInvalidHandle`, `kNullHandle`, `kNullopt` | `Graphics.h` |

예시 모음:

```cpp
// jugx/JugX/Application.cpp
namespace
{
    Application* g_pSingleton = nullptr;
    constexpr float kGamepadAxisScale = 1.f / 32767.f;
    [[nodiscard]] Flags<eKeyMod> MakeKeyMods_(const SDL_Keymod _mod) { ... }
}
```

```cpp
// jugx/JugX/RingBuffer.h — 내부 상수 네임스페이스
namespace ring_buffer_detail
{
    constexpr size_t kDefaultCapacity = 8;
}
```

세부 규칙:

- 멤버 `bool` 은 **항상 `b` 가 이름에 들어간다** (필드명에도: `bEnabled`, `bOwnership`, `bMapped`, `bU32`, `bSideEffect`). POD 구조체 필드도 bool 이면 `b…` 를 붙인다 (`Graphics.h` 의 `bMapped`).
- **포인터는 항상 `p`** : `pBuffer`, `pSrvOrNull`, `pDsvOrNull`. null 가능한 포인터/핸들/값은 `OrNull` / `OrEmpty` / `OrInvalid` 접미 (`_executorOrNull`, `_initDataOrEmpty`, `_byteWidthOrInvalid`, `fbhOrInvalid`, `_nameOrEmpty`).
- POD(데이터만 있는 struct) 필드는 접두 없이 camelCase: `vertexData`, `descIndex`, `dirtyBegin`. 단 bool 은 `b`, 포인터는 `p`.
- 복수 개수 변수는 `num` 접두 (`m_numVertices`, `_numInstances`, `NumDirties()`), 핸들 변수는 타입 약자 (`fbh`, `texh`, `cbh`, `sbh`, `ph`, `vbh`, `ibh`).
- 타입 별칭은 `using` 전용 (`typedef` 금지). 컨테이너 별칭은 PascalCase, **수학/배열 타입만 전부 대문자**: `ARRAY`, `ENUM_ARRAY`, `VECTOR3`, `VEC3U`, `AABB`, `OBB`, `INTERVAL`, `LINK`(내부 struct) — `jugx/JugX/Typedef.h`, `Collision.h`, `MemoryArena.h`. (`Vector`, `HashMap`, `String` 은 일반 PascalCase.)
- 이벤트 클래스 `XxxEvent`, 핸들러 멤버 `OnXxxEvent_()`, 가상 훅 `OnUpdate`/`OnEvent` (jamengine) 혹은 `Init/Shutdown/OnEvent/Update` (jugx).
- 약어는 첫 글자만 대문자: `Srgb`, `Msaa`, `Dxgi` 로 함수/타입 안에서 (`IsSrgbFormat`, `ToDxgiVertexFormat_`), 단 `UID`, `RGBA`, `SIMD`, `API` 는 전부 대문자.

`[차이]` jamengine: 지역 변수 PascalCase 라고 CLAUDE.md 는 말하지만 실제 코드는 camelCase (`const UID uid = ...`). 정수 별칭 `Int32/UInt32/UInt64/SizeT`, 후행 `_` 공통. jugx 는 `int`, `uint32_t`, `uint64_t`, `size_t` 같은 표준 이름을 **그대로** 쓴다 (별칭 없음).

---

## 3. 포맷 (clang-format)

`.clang-format` 은 `jugx/.clang-format` 과 `juggfx/.clang-format` 이 동일, 핵심만 복사한다:

```yaml
IndentWidth: 4
ColumnLimit: 0                      # 줄 길이 제한 없음 (자동 줄바꿈 안 함)
AccessModifierOffset: "-4"          # public:/private: 는 class 와 같은 들여쓰기
BreakBeforeBraces: Custom           # Allman: 거의 모든 { 는 새 줄
BraceWrapping: { AfterClass/AfterStruct/AfterEnum/AfterFunction/AfterNamespace: true,
                 AfterControlStatement: Always, AfterCaseLabel: true, BeforeElse: true, BeforeCatch: true }
NamespaceIndentation: Inner         # 가장 바깥 namespace 만 들여쓰기 없음, 안쪽 namespace 는 들여쓴다
FixNamespaceComments: true          # 닫는 괄호 뒤 `// namespace jug`
SpacesBeforeTrailingComments: 3     # => `}   // namespace jug` , `x;   // 설명` (공백 3칸)
AlignConsecutiveAssignments/Declarations/Macros: Enabled   # 연속된 줄의 `=` `이름` 정렬
AlignArrayOfStructures: Right
PointerAlignment: Left              # Type* p, Type& r
BinPackParameters: false            # 매개변수는 한 줄에 몰아 쓰지 않는다
BinPackArguments: false
BreakConstructorInitializers: BeforeComma   # 초기화 리스트 콤마를 줄 앞에
BreakInheritanceList: AfterColon
BreakBeforeBinaryOperators: NonAssignment   # 줄바꿈 시 연산자가 줄 앞에
AlignOperands: AlignAfterOperator
Cpp11BracedListStyle: false         # `{ a, b }` 중괄호 안쪽에 공백, `Type { }`
SpaceBeforeCpp11BracedList: true    # `Foo { 1, 2 }` / `= {}`
IndentCaseLabels: true              # switch 안 case 들여쓰기
IndentPPDirectives: AfterHash       # `#    define` 처럼 # 뒤에 들여쓰기
AllowShortFunctionsOnASingleLine / IfStatements / Blocks / CaseLabels / Lambdas: true
SpaceBeforeParens: ControlStatements   # `if (`, `for (`, 함수 호출은 붙임
SortIncludes: false
AlwaysBreakTemplateDeclarations: Yes   # template<> 는 별도 줄
CompactNamespaces: true
```

### 3.1 눈에 보이는 결과 (이 모양을 만들어라)

**Allman + 들여쓰기 4칸. `if` 한 줄이어도 중괄호 쓴다.**

```cpp
// jugx/JugX/Timer.cpp
void Timer::Lap()
{
    JUG_ASSERT(m_bRunning, "Timer is not running.\n");

    const TimePoint now          = SteadyClock::now();
    const uint64_t  elapsedCount = std::chrono::duration_cast<Ns>(now - m_startCount).count();
    m_elapsedCount               = elapsedCount;
    m_totalCount += elapsedCount;
    m_startCount = now;
}
```

`if` 에 단일 문장이라도 중괄호를 생략하지 않는다 (`jugx/JugX/Application.cpp` 의 `MakeKeyMods_`). 극히 일부 `if (depth > 0) --depth;` 가 `TypeRefl.h` 에 있으나 예외로 본다.

**연속 선언/대입 정렬**: 타입 열과 이름 열, `=` 열을 맞춘다. 지역 선언 블록이 곧 표처럼 보여야 한다.

```cpp
// jugx/JugX/Logger.cpp
constexpr Flags<eLogPattern>                kLogPatternFlags = { eLogPattern::YearMonthDay, eLogPattern::HourMinSec };
constexpr ENUM_ARRAY<eLogLevel, StringView> kLogLevelNames   = { "TRACE", "INFO", "WARN", "ERROR", "FATAL" };
```

```cpp
// juggfx/JugGfx/Graphics.h — 멤버 데이터 정렬
ARRAY<ID3D11RenderTargetView*, kNumMaxRenderTargetSlots> rtvs             = {};
ID3D11DepthStencilView*                                  pDsvOrNull       = nullptr;
ISwapChain*                                              pSwapChainOrNull = nullptr;
```

**함수 매개변수는 정의에서 항상 한 줄에 하나**, 첫 매개변수는 함수명 다음 줄. 모두 `const` 값(또는 `const T&`), 이름은 정렬한다. 인자가 하나뿐이어도 정의(.cpp)에서는 줄바꿈한다:

```cpp
// jugx/JugX/Logger.cpp
void Logger::VLog(
    const eLogLevel        _level,
    const StringView       _msg,
    const std::format_args _args)
{
```

```cpp
// jugx/JugX/Logger.cpp — 매개변수 1개도 줄바꿈
void Logger::SetName(
    const StringView _name)
{
    m_name = _name;
}
```

**선언(.h)** 은 짧으면 한 줄, 길거나 기본값이 섞이면 줄바꿈 정렬:

```cpp
// juggfx/JugGfx/Graphics.h
[[nodiscard]] TextureHandle CreateTexture2D(
    uint32_t              _width,
    uint32_t              _height,
    eTextureFormat        _format,
    uint32_t              _numLayers       = 1,
    eMSAA                 _msaa            = eMSAA::None,
    Flags<eTextureOption> _flags           = eTextureOption::None,
    MemoryView            _initDataOrEmpty = {});

// 같은 헤더 — 인자 2~3개의 단순 선언은 한 줄
void UpdateBuffer(BufferRef _buffer, MemoryView _data, uint32_t _offset = 0);   // (실제 파일에서는 줄바꿈 형태도 혼재)
```

(규칙: 선언에서 매개변수가 3개 이하이고 짧으면 한 줄도 허용. 4개 이상이거나 기본값이 있으면 위처럼 세로 정렬.)

**선언부의 반환형 열 정렬**: 연속된 `[[nodiscard]]` 선언은 반환형 폭을 맞춘다.

```cpp
// jugx/JugX/Timer.h
[[nodiscard]] uint64_t GetElapsedCount() const;
[[nodiscard]] uint64_t GetTotalCount() const;
[[nodiscard]] bool     IsRunning() const;
```

**생성자 초기화 리스트**: 콜론 줄 앞, 쉼표를 줄 앞에, 한 줄에 하나.

```cpp
// jugx/JugX/Error.h
template<ErrorCodeT T>
/* implicit */ Error(T _errorCode)
    : m_errorCode(ErrorCodeTraits<T>::GetErrorCode(_errorCode))
    , m_pCategory(ErrorCodeTraits<T>::GetCategory())
{
}
```

빈 본문은 `{` `}` 를 두 줄로 쓴다 (`{\n}`). 단 `virtual void OnEnter() {}` 처럼 한 줄 빈 본문은 인터페이스 기본 구현에 허용 (jamengine `Scene.h`).

**중괄호 초기화는 안쪽 공백, 이름과 `{` 사이 공백**:

```cpp
return JsonWriter { m_pDoc, pField };      // jugx/JugX/JsonSerializer.cpp
constexpr UID kRootEntityUID { 0 };        // jamengine Scene.cpp
FileReader reader = {};                    // jugx/JugX/FileReader.cpp
```

**삼항·긴 식 줄바꿈**은 연산자를 줄 앞에: 

```cpp
hf = (static_cast<uint16_t>(sign) << 15)
   | 0x7C00
   | (mantissa >> 13);                      // jugx/JugX/Float16.h
```

**switch**: `case` 들여쓰기, `case X:` 다음 줄에 본문, 중괄호가 필요하면 `case X:` 아래 줄에 `{`. 한 줄짜리 case 는 `case X: return ...;` 허용(`Graphics.cpp ToVertexSemantic_`). `default:` 에서 `JUG_ASSERT(false, ...)`.

```cpp
// jugx/JugX/Timer.cpp
switch (_unit)
{
    case eTimeUnit::Nano:
        return static_cast<float>(_count);
    ...
    default:
        JUG_ASSERT(false, "Invalid time unit.\n");
        return 0.f;
}
```

**익명 네임스페이스 안은 한 단계 들여쓴다** (`NamespaceIndentation: Inner`) :

```cpp
namespace jug
{

namespace
{
    constexpr uint32_t kSlotShift = 4;
    ...
}   // namespace

void ...
```

`namespace jug {` 직후와 `}` 직전에 **빈 줄 하나**, 닫는 줄은 `}   // namespace jug` (공백 3칸).

**부동소수 리터럴**은 `0.f`, `1.f`, `1e-3f`, `0.` (뒤에 0 생략 소문자 f) — `1.0f` 가 아님.

**정수 리터럴**: 16진수 대문자 `0xFFFFFFFF`, 큰 수 구분자 `1'000'000` (`AgentRules` 가 따르는 클론 쪽 관례).

**연산자 간 공백**: `a * b`, `x << 3`, 단항에 공백 없음. 캐스트 뒤 공백 없음 (`static_cast<float>(x)`). `Type* p`, `Type& r` (별은 타입에 붙임), `char** _argv`.

**한 줄 가드**: `if (!cond) { return; }` 같은 한 줄 중괄호는 쓰지 않는다 (Allman 으로 세 줄).

---

## 4. 언어 사용 정책

### 4.1 `auto`

0절 참조: **새 코드에서는 금지.** 저장소의 허용 패턴(이터레이터, 구조체 바인딩)은 타입 별칭 또는 명시적 타입으로 바꾼다. (예: `const Iterator listIt = m_list.begin();` — `jugx/JugX/LruCache.h` 는 이미 이렇게 쓴다.) `decltype(auto)` / 반환형 `auto` 도 쓰지 않는다 (`operator<=>` 의 반환형 `auto` 는 `= default` 일 때만 허용되는 불가피한 예외라 클론에서는 `std::strong_ordering` 을 명시).

### 4.2 람다

0절 참조: `<algorithm>` 인자 위치에서만. 그 외에는 `Foo_()` 멤버/자유 함수 + 함수 포인터. 예외로 `std::find_if(v.begin(), v.end(), [&](const Enemy& _e) { return _e.id == id; })` 같은 인자 람다는 허용. 인자명도 `_e` 처럼 `_` 접두.

### 4.3 배열/컨테이너

- C 배열 금지 → `ARRAY<T, N>` (`std::array`), 동적은 `Vector<T>`, 뷰는 `Span<T>` / `Span<const T>`, 문자열은 `String` / `StringView` (전 파일 `StringView` 를 값으로 받는다: `const StringView _name`).
- enum 으로 인덱싱되는 배열은 `ENUM_ARRAY<eE, V>` / `EnumArray<eE, V>` (`jugx/JugX/EnumArray.h`). `m_assetMaps[_type]` 처럼 enum 으로 바로 인덱스.
- 해시맵은 `HashMap`(= phmap parallel_flat), `HashSet`, 정렬맵은 `RbMap`/`RbSet` — std 이름을 직접 쓰지 않고 `Typedef.h` 의 별칭을 쓴다. 단, `std::unique_ptr`, `std::move`, `std::format_args` 등 별칭이 없는 것은 그대로.
- `Optional<T>` + `kNullopt` 별칭을 쓴다 (`Optional<eModelFileFormat> FormatFromExtension_()` — `juggfx/JugGfx/ModelImporter.cpp`).
- 별칭 정의(`Typedef.h`)는 `// ====` 배너로 섹션을 나눈 뒤 `using` 줄들의 `=` 를 정렬한다:

```cpp
// jugx/JugX/Typedef.h
using String      = BasicString<char>;
using StringView  = BasicStringView<char>;
using WString     = BasicString<wchar_t>;
using WStringView = BasicStringView<wchar_t>;
```

### 4.4 포인터/참조/스마트 포인터

- **기본은 참조.** 포인터는 (1) null 이 될 수 있을 때, (2) 배열/버퍼일 때, (3) 출력 매개변수일 때만 (`jamengine/CLAUDE.md`). null 가능성은 이름으로 표기: `pCameraOrNull`.
- 소유권: 값 타입 / 핸들 / `Scoped<T>`(unique_ptr 대응), `Ref<T>`(**intrusive** refcount, shared_ptr 아님, `jugx/JugX/Ref.h`). 비소유 관찰 포인터는 `m_pOwner`/`m_pGraph` 형태의 raw 포인터로 둔다.
- `new`/`delete` 직접 호출은 메모리 계층(`MemoryArena`, `FixedBlockMemoryPool`) 안에만. 일반 코드는 풀(`ResourcePool<H, V>`)에서 핸들을 발급받는다 (5절).
- 포인터 안전성은 `JUG_ASSERT(_pCaller, "…is nullptr.\n")` 로 검증.

### 4.5 const / constexpr / [[nodiscard]] / noexcept

- 지역 변수·매개변수는 **가능하면 모두 `const`**: `const float _deltaTimeSec`, `const size_t _index`, `const eLogLevel _level`. 값 매개변수에 `const` 를 쓰는 것이 시그니처(.cpp 정의)에서의 일상이다. 선언(.h)에서는 값 매개변수의 `const` 를 **생략**한다 (정의와 불일치해도 OK):

```cpp
// jugx/JugX/Timer.h  → Timer.cpp
double TimeCast(uint64_t _count, eTimeUnit _unit);                    // .h
double TimeCast(const uint64_t _count, const eTimeUnit _unit)  {...}  // .cpp (실제로는 줄바꿈)
```

- **모든 getter / 순수 질의 함수에 `[[nodiscard]]`.** `JAM_NODISCARD` 매크로는 구버전. 반환값을 버려도 되는 setter/mutator 에는 안 붙인다. 파일 내부 `static`/익명 NS 함수에도 값을 반환하면 붙인다 (`[[nodiscard]] Flags<eKeyMod> MakeKeyMods_(...)`).
- **`[[nodiscard]]` 는 선언 맨 앞**, `static`/`constexpr` 보다 앞: `[[nodiscard]] static Application& GetSingleton();`, `[[nodiscard]] constexpr bool IsNull() const`. 연산자도 똑같이.
- 상수는 `constexpr` (매크로 `#define` 상수 금지). `constexpr` 함수·생성자를 적극 사용 (jugx 헤더에 728회). 정수·수학 함수는 `constexpr` + `[[nodiscard]]`.
- `noexcept`: 이동 생성/대입, `swap`, 소멸 관련, `BaseFlags` 의 모든 멤버처럼 **보장 가능한 곳에만** (jugx 에 107회). 일반 함수에는 붙이지 않는다.
- 비멤버 `const` 위치는 **west const** : `const Foo&`, `const T`, `const StringView`.

### 4.6 캐스트

- C 스타일 캐스트 금지. 항상 `static_cast<T>(x)` (jugx .cpp 에서만 56회), 포인터·바이트 해석에만 `reinterpret_cast`. `const_cast` 는 const/비const 오버로드 중복 제거에만 (`EnumArray.h`). `dynamic_cast`/RTTI 사용 금지 (RTTI 대신 `GetTypeHash()` 같은 자체 해시).
- 부동↔정수 변환은 반드시 명시 `static_cast<float>(...)` 로 한다 (암시 변환 경고 허용 안 함).

### 4.7 초기화

- **멤버는 기본 멤버 초기화자(NSDMI) 필수, `= {}` 또는 `= 값`**: `float m_alpha = 0.f;`, `ARRAY<int, N> m_x = {};`, `Pointer* m_p = nullptr;`, 심지어 `String name = {};`, `Vector<X> list = {};`, `uint32_t dirtyEnd = kSize;`. 멤버를 초기화하지 않고 두지 않는다. 생성자 초기화 리스트는 매개변수로 값을 받을 때만.
- 지역 객체: `Type name = {};` 또는 `const Type name = expr;`. 값 초기화는 `=` 우선. 중괄호 생성은 집합체/명시적 `Type { a, b }` 에서만.
- 클래스 생성자에서 `explicit` 을 기본으로, 의도된 암시 변환에는 주석 `/* implicit */` 을 붙인다 (jugx 에 `/* implicit */` 이 수십 곳):

```cpp
// jugx/JugX/Handle.h
/* implicit */ constexpr Handle(
    const NullHandleType)
    : value(handle_detail::kNullValue)
{
}

constexpr explicit Handle(
    const uint32_t _value)
    : value(_value)
{
}
```

- 태그 디스패치 (`kNoInit`, `kZeroFlag`, `kAllFlag`, `kOK`, `kNullHandle`) 가 정수 매직 값을 대체한다 (`JUG_DEFINE_TAG`, `Tag.h`).

### 4.8 가상 함수

- 순수 인터페이스: 소멸자 `virtual ~IFoo() = default;`, 생성자 `Foo() = default;` 를 **정렬해서** 나란히:

```cpp
// jugx/JugX/Event.h
Event()          = default;
virtual ~Event() = default;
```

- 파생에서는 반드시 `override` (jugx 헤더에 65회). `final` 은 쓰지 않는다 (사용 사례 없음).
- 순수 가상은 `= 0;` 를 열 정렬: `[[nodiscard]] virtual String ToString() const = 0;`.
- 가상 함수 호출 경로(`virtual void Update`)는 최소화하고, 공개 비가상 함수 → private 가상 `LogImpl` 패턴(NVI)을 선호 (`jugx/JugX/Logger.h`).

### 4.9 정수 타입

- jugx/juggfx: 표준 `int`, `uint32_t`, `uint64_t`, `uint8_t`, `size_t` 를 그대로 사용. 인덱스·크기는 `size_t`, GPU/비트 필드 용도는 `uint32_t`, 일반 카운트는 `int`.
- jamengine: `Int32`, `UInt32`, `UInt64`, `SizeT` 별칭. 기본은 `Int32`, 비트마스크에만 `UInt32` `[차이]`.
- **클론은 `slug` 네임스페이스·`Base/Types.h` 의 정의를 따른다** (표준 이름을 그대로 쓰는 jugx 방식).

### 4.10 Assert / Log / 에러

- 전제조건은 `JUG_ASSERT(cond, "메시지 {}", args)`: 디버그 빌드에서만 동작(`JUG_ENABLE_ASSERT`), 메시지는 **영어**, 보통 끝에 `\n`, 형식은 `std::format`. 릴리스에서 assert 에 의존하는 코드 금지. 클론은 `SLUG_ASSERT`.

```cpp
JUG_ASSERT(m_bRunning, "Timer is not running.\n");
JUG_ASSERT(IsObject(), "BeginObjectField requires an object value - call SetObject() first");
```

- 도달 불가: `JUG_ASSERT(false, "Invalid time unit.\n"); return 0.f;` (switch default).
- 복구 불가: `JUG_FATAL("…{}", …)` → `std::terminate()` (`Fatal.h`).
- 로그: `JUG_CORE_LOG_INFO/WARN/ERROR/TRACE/FATAL(fmt, args…)` (`CoreLogger.h`), jamengine `JAM_LOG_ERROR`. 포맷은 `std::format` 문자열.
- 에러 처리: **예외 금지.** `Result<T>`(`jugx/JugX/Result.h`) 와 `Result<void>`, `Error` (`code + category` 값 타입, `kOK`):

```cpp
// jugx/JugX/FileReader.h
[[nodiscard]] static Result<FileReader> Open(const FilePath& _path);
Result<size_t> Read(MutableMemoryView _outBuffer) const;
```

  에러 enum 은 `eXxxError` + `XxxCategory : IErrorCategory` + `JUG_DEFINE_ERROR_ENUM(eXxxError, XxxCategory);` 로 등록 (`SerializeError.h`, `juggfx/JugGfx/Error.h`). 각 에러 값에는 한글 한 줄 주석(`// 입력이 올바른 포맷이 아님`).
  - jamengine 은 `Expect<T>` / `Error::Type` `[차이]`. 같은 개념의 구 이름이다.
  - 생성자는 실패할 수 없게 하고, 실패 가능한 초기화는 `static Result<T> Open()` + private `Open_()` 로 (`FileReader.h`, "Init 패턴").
  - 단순 질의는 `bool` 반환 + `OrNull` 포인터 (`GetOrNull()`), 존재 확인은 `IsValid()` / `Has…()`.
- 프로그래머 오류(전제조건 위반)는 assert, 외부 입력 오류(파일/파싱)는 `Result`.

### 4.11 템플릿 / concept

- C++20 concept 를 적극 사용: `requires` 절은 `template<…>` 아래 줄에 들여쓴다.

```cpp
// jugx/JugX/EventDispatcher.h
template<EventT T, typename Fn>
    requires EventDispatchFnT<T, Fn>
void Dispatch(
    Fn&& _fn)
{
```

- `enable_if` / SFINAE 금지, `if constexpr` 사용, 가변 인자는 `Args&&... _args` + `std::forward<Args>(_args)...`.
- 템플릿 내부 보조는 `xxx_detail` 네임스페이스(소문자 snake)에, 상수는 `kXxx`.
- 포맷 가능 타입은 `std::formatter<…>` 특수화를 같은 헤더 끝에 둔다 (`Handle.h`).

### 4.12 기타

- `enum class` 로 만든 비트 플래그는 `Flags<eX>` (jugx `EnumFlags.h`) 래퍼로 쓰고, 정수 비트 연산을 직접 하지 않는다: `flags.Has(eX::A)`, `Enable`, `Disable`, `HasAny`, `HasAll`.
- 매직 넘버는 `constexpr` 상수. 단위는 이름으로 (`_deltaTimeSec`, `m_ticksLeft`, `kShaderBytecodeHeaderSize`).
- 매크로는 최후의 수단: 디버그/진입점/클래스 보일러플레이트(`JUG_CLASS`), 컴파일러 분기(`JUG_FORCE_INLINE`), assert/log/event body. 정렬해서 `\` 를 맞춘다.
- `#pragma warning` 대신 `JUG_DISABLE_ANON_WARNING_BEGIN/END` 래퍼.
- anonymous union/struct 는 `JUG_DISABLE_ANON_WARNING_*` 로 감싸고 최소한만 (`Handle.h`).
- `static_assert(…, "메시지.")` 를 널리 사용 (타입 요구사항 문서화).

---

## 5. 클래스 설계

### 5.1 섹션 순서

한 클래스 안의 순서:

1. `JUG_CLASS(Foo, NO_COPY, NO_MOVE)` 매크로 줄 (class 본문 첫 줄, 접근 지정자 앞)
2. 내부 타입(private `struct` / `using` / `friend`) — 헤더 맨 위에 두는 경우가 많다 (`Graphics.h` 는 private 구조체와 `using` 풀 별칭이 `public:` 보다 먼저)
3. `public:` 생성자/소멸자 → 정적 팩토리/싱글턴 → 공개 메서드(`// ====` 섹션으로 그룹화) → 공개 데이터(POD 일 때)
4. `protected:` virtual 훅
5. `private:` 비공개 함수(후행 `_`) → **마지막에 멤버 변수** (함수보다 뒤)

```cpp
// jugx/JugX/Application.h
class Application
{
    JUG_CLASS(Application, NO_COPY, NO_MOVE)

public:
    virtual ~Application();
    [[nodiscard]] static Application& GetSingleton();

    int  Run(int _argc, char** _argv);
    void Quit();
    ...
protected:
    explicit Application(const ApplicationDesc& _desc);

    virtual void Init();
    virtual void Shutdown();
    virtual void OnEvent(Event& _event);
    virtual void Update(float _deltaTimeSec);

private:
    void InitSystems_() const;
    void ShutdownSystems_() const;
    void PollEvents_();

    void OnQuitEvent_(const QuitEvent& _event);

    ApplicationDesc m_desc = {};
    ...
};
```

- `public:` 가 `class` 와 같은 열(`AccessModifierOffset: -4`).
- 접근 지정자 앞뒤에 빈 줄.
- **`struct` 는 데이터 묶음(불변식 없음)**, **`class` 는 불변식이 있을 때** (`jamengine/CLAUDE.md`). `struct` 안에 멤버 함수가 있어도 간단한 헬퍼까지만 (`Graphics::ResourceBind`).
- 멤버 변수는 **상관 있는 것끼리 묶고 빈 줄**로 구분, 묶음 안에서 타입·이름·`=` 열 정렬. 필요하면 묶음 앞에 `// ====` 섹션 배너.

### 5.2 special member / 복사 이동

- `JUG_CLASS(Foo, NO_COPY, NO_MOVE)` 매크로 하나로 `= delete` 4개를 생성 (`DEFAULT_COPY`, `DEFAULT_MOVE` 도 있음). 클래스 본문 맨 앞, 접근 지정자 앞에 둔다:

```cpp
// jugx/JugX/Macro.h
#define JUG_CLASS_NO_COPY(_class)              \
    _class(const _class&)            = delete; \
    _class& operator=(const _class&) = delete;
```

- 이동만 허용하는 타입은 `JUG_CLASS(MemoryArena, NO_COPY)` + 명시적 이동 생성/대입 선언 (`MemoryArena.h`).
- Rule of five: 소멸자·복사·이동 중 하나를 건드리면 나머지를 매크로 또는 명시로 모두 선언한다 (`Result`, `Ref`, `Scoped`).
- `[차이]` jamengine 은 매크로 없이 네 줄을 손으로 쓰고 `=` 를 정렬한다 (`AssetManager.h`).

### 5.3 getter / setter / 네이밍

- getter `GetXxx()`, bool 은 `IsXxx()` / `HasXxx()` / `CanXxx()` (`IsRunning`, `IsEmpty`, `IsHandled`, `HasPass`, `Has`).
- setter `SetXxx(value)`; 플래그 토글 `Enable/Disable`.
- 컨테이너류: `GetSize()`, `IsEmpty()`, `Clear()`, `Insert/Emplace/Erase`, `Get()`/`GetOrNull()`.
- 팩토리: `static MakeXxx()` / `static Open()` / `Create…`, 변환은 `ToXxx()` / `MakeFromXxx()`.
- 수정 가능한 참조 반환: `ModifyXxx()` (jamengine `ModifyLocal()`) `[차이]`, jugx 는 비const `Get()` 오버로드.
- const / 비const 오버로드는 쌍으로 나란히 둔다 (`ResourcePool::Get`).
- 접근자 본문은 거의 항상 .cpp (비템플릿).
- 빌더는 `Xxx& SetYyy(...)` 체이닝을 반환 (`RenderPassBuilder`, `juggfx/JugGfx/RenderGraph.h`).

### 5.4 private 헬퍼

- **후행 `_`** 규칙: 클래스 private 함수, 익명 네임스페이스 자유 함수, 중첩 접근자 모두. `Foo_Recursive_()` 처럼 내부 접미가 있으면 `_` 로 둘 다 끝낸다 (`CloneEntity_Recursive_`).
- 공개 함수 = 외부 계약 + 검증(assert), `_` 함수 = 구현.

### 5.5 싱글턴 / 매니저

- 시스템은 **명시적 수명의 싱글턴**이다. `jugx`/`juggfx`: 익명 네임스페이스에 `Foo* g_pSingleton = nullptr;`, 생성자에서 `JUG_ASSERT(!g_pSingleton)` 후 대입, `[[nodiscard]] static Foo& GetSingleton();` 으로 접근 (`Application.cpp`, `Graphics.cpp`).
- 소유는 사용자 코드가 한다 (`Application` 파생 클래스를 스택/`main` 에서 만들고 `Run`). 정적 초기화 순서 문제를 피하려고 `static Foo instance;` 형태는 쓰지 않는다.
- `[차이]` jamengine: `inline static Foo* s_pInstance`, `GetInstance()`, `CreateInstance()/DestroyInstance()` private + `friend class Application`, 헬퍼 `GetAssetManager()` 자유 함수.
- 매니저는 서브시스템 플래그(`eAppInitFlags`)로 켜고 끈다.

### 5.6 소유권 패턴

- GPU/자원: 모두 **핸들**(`Handle<T>`, 24bit index + 8bit token) + `ResourcePool<Handle, Value>` 로 관리. 강타입 핸들 별칭: `using TextureHandle = Handle<…>;`. 객체 포인터를 외부로 노출하지 않는다. 핸들 유효성은 `IsValid()` / `IsNull()` / `kInvalidHandle`.
- `ResourcePool` = `HandleAllocator` + `HandleStorage`. Erase 시 토큰이 증가해 stale 핸들을 감지.
- 이름으로 자원을 찾는 계층(렌더 그래프)은 `HashMap<String, Resource<H>>` 에 `{ handle, bOwnership }` 를 저장한다 (`RenderGraph.h`).
- 파일/에셋: `Ref<IAsset>` (intrusive) `[차이: jamengine]`.

---

## 6. 아키텍처/설계 스타일

### 6.1 계층과 의존 방향

```
jugx  (플랫폼 중립 유틸: 수학, 컨테이너, Handle/ResourcePool, Result/Error, Logger, Json, Event, Application(SDL3))
  ↑
juggfx (D3D11 래퍼 + RenderGraph + 이미지/모델 임포트 + ImGui 렌더러)  — jugx 만 안다
  ↑
앱 / 게임  (jamengine 은 한 단계 더: Scene/ECS(EnTT)/Editor/AssetManager)
```

- 의존은 **한 방향**: 위 계층이 아래를 알고 반대는 없다. 하위 계층은 상위를 알지 못하므로 전방 선언·콜백·인터페이스(`IErrorCategory`, `Event`)로 확장점을 연다.
- 외부 3rd-party 는 `*Include.h` 1파일에 가두고, 공개 헤더에는 불투명 포인터·핸들만 노출한다.
- vcpkg 매니페스트 + CMake(`jugx`) 또는 MSBuild(`jamengine`, 클론) — 3rd-party 는 `Vendor/` 또는 vcpkg.

### 6.2 모듈 분해

- **작고 직교하는 헤더** 단위 (`Timer`, `RingBuffer`, `LruCache`, `Handle`, `Result`, `EnumFlags`, `EnumRefl` …). 큰 시스템은 "데이터(Desc/Handle) + 시스템(manager) + 빌더/그래프" 로 쪼갠다.
- 선언형 `Desc` 구조체 → 실행 시스템이 해석 (`ApplicationDesc`, `VertexBufferDesc`, `TextureDesc`, `PassDesc`).
- "Description → Resolved → Compiled" 3단계 변환 (`RenderGraph.h` 의 `PassDesc` → `ResolvedPass` → `CompiledPass`): 선언은 사용자 데이터, 해석은 이름→핸들 해소, 컴파일은 실행 직전 최적화(unbind mask).

### 6.3 데이터 기반 설정

- 설정/자산은 JSON (`yyjson` 래퍼: `JsonWriter`/`JsonReader`, ADL 직렬화 `JsonAdlSerializer`) 또는 바이너리. 컴포넌트는 `Serialize/Deserialize(ArchiveWriter&, …)` 쌍을 가진다 (jamengine `CameraComponent.h`).
- 리플렉션은 컴파일타임 `NameOf<T>()`/`HashOf<T>()` (`TypeRefl.h`), `EnumRefl` — 매크로 레지스트리 대신 constexpr + `__PRETTY_FUNCTION__` 파싱.
- 설정 구조체는 필드마다 기본값을 가진 평문 struct (`ApplicationDesc`): `StringView appName = "Application";`.
- 에러 문자열·로그는 영어, 사용자 설명 주석은 한국어.

### 6.4 업데이트/렌더 루프, 이벤트

```cpp
// jamengine/JamEngine/Application.cpp  (Run 루프)
while (m_bRunning)
{
    if (!win.PollEvents())
    {
        m_commandQueue.Execute();   // execute submitted commands

        m_timer.Tick();
        const float dtSec = m_timer.GetDeltaFloat(eSIPrefix::Sec);
        Update(dtSec);
        Render();
        ...
        gfx.Frame();      // present
        input.Update();   // update input
    }
}
```

- `Application::Run()` 비가상 루프 + 파생 클래스는 가상 훅 `Init / Update(float) / OnEvent / Shutdown` 만 오버라이드한다 (`jugx/JugX/Application.h`: "not recommended to override Run" 의도).
- 시간은 `float _deltaTimeSec` 로 전달, 단위를 이름에 쓴다.
- 이벤트: `class XxxEvent : public Event` + `JUG_EVENT_BODY(XxxEvent)` (타입 해시·이름 상수 정의), 수신은 `EventDispatcher dispatcher { _event }; dispatcher.Dispatch<XxxEvent>(this, &Class::OnXxxEvent_);` — 핸들러가 `SetHandled(true)` 하면 전파 중단. 이벤트는 **동기**로 `Application::DispatchEvent` 가 서브시스템 순서대로 전달한다 (`OnEvent` → 윈도우 → 그래픽스 → 에셋 → 에디터).
- 스레드 경계는 `CommandQueue` (메인 스레드에 커맨드 제출) 로 처리.

### 6.5 엔티티/컴포넌트

(jamengine 한정, 클론은 ECS 를 도입하지 않고 값 타입/핸들 풀을 쓴다)

- EnTT 래퍼 `Entity { entt::registry*, entt::entity, UID }`, 컴포넌트는 순수 데이터 `struct XxxComponent` + `JAM_COMPONENT_BODY(...)`, 표시이름/속성 플래그 매크로 + `Serialize/Deserialize/DrawEditPanel`. 씬은 엔티티 생성/삭제를 deferred 로 처리.
- 컴포넌트 접근은 `GetComponents<T…>()`, `HaveComponents<T…>()`, `CreateComponent<T>()` 로, 제약은 `requires` 로.

### 6.6 자원 관리 / 디버그

- 모든 GPU 자원은 `Create…` → 핸들 → `Destroy(handle)`; 생성/파괴를 `ReportLiveObjects()` 로 감시; 에디터/디버그 전용은 플래그/매크로로 끈다.
- 상태 변경 비용은 dirty flag 로 지연한다 (`ePipelineDirty` 비트마스크, `ResourceBind::MarkDirty / ClearDirty`).
- 그래픽 호출은 `JUG_DX_CHECK(hr)` 로 감싸고 `JUG_DX_RELEASE(p)` 로 해제.

---

## 7. 주석

- **언어: 한국어 위주, 영어 혼용.** 코드 의도·이유 설명은 한국어(`// 상위 옵션이 하위 옵션을 포함한다.`), 인라인 짧은 설명은 영어도 많음(`// compare ignoring case`, `// time stamp`). assert/log 문자열은 **영어**. 문서성 블록(클래스 헤더 위 설명)은 한국어.
- **밀도: 낮음.** 이름으로 설명하고, 이유/단위/불변식이 불분명할 때만 주석. 함수마다 doc comment 를 달지 않는다.
- 행 끝 주석은 공백 3칸 + `//` 로 정렬: `bool bIssued = false;   // Begin_ 호출됨. 결과를 모두 읽기 전까지 재사용 금지.`
- 헤더의 에러 enum/값에도 행 끝 한글 주석.
- **섹션 배너(헤더/구현 모두)**: 길이 43의 `// ===…` 선 + 제목 + 선, 앞뒤 빈 줄. 클래스 안 기능 그룹, 파일 안 구현 그룹을 나눈다.

```cpp
// ===========================================
//  Vertex Stream
// ===========================================

[[nodiscard]] VertexBufferHandle CreateVertexBuffer(...)
```

  더 큰 개념 설명은 길이 56/60 의 `// ====…` 블록에 제목과 한글 설명 여러 줄:

```cpp
// =========================================================
//  MemoryArena
//   메모리 해제없이 빠른 메모리 할당을 위해 사용되는 메모리 풀
//   영속적으로 사용하는 개념 아님
// =========================================================
```

  `[차이]` jamengine 은 `///////` 슬래시 줄 / `// scene graph` 소문자 한 줄 소제목.
  **[이 프로젝트에서는]** `AgentRules.md` 가 "장식용 배너 주석 금지(필요하면 짧게 한 줄)" 라고 못박았다. 그래서 클론에서는 배너를 최소화하고, 쓸 땐 `// ===` 3줄 블록 대신 `// 섹션명` 한 줄만 쓴다. 저장소와 가장 크게 다른 점이므로 주의.
- **Doxygen**: 거의 안 씀. jamengine 일부 `/// @brief 설명` 만 (`ILayer.h`). 클론에서는 쓰지 않는다.
- **TODO**: 형식은 `// TODO 한글 설명` (콜론 없음, 작성자 없음) — `jamengine/JamEngine/DebugDraw.cpp:12  // TODO 배치 렌더링 최적화 건덕지 존재`. FIXME/NOTE 는 쓰지 않음.
- `/* implicit */` 는 생성자 의도 표기용 주석, `// NOLINT` 는 린트 예외, `// ReSharper disable …` 는 파일 첫 줄.
- 주석 처리된 죽은 코드가 일부 남아 있다 (`CameraComponent.cpp`). 새 코드에서는 남기지 않는다.
- `static_assert(true)   // for semicolon after the macro` 처럼 매크로 뒤 세미콜론 용 트릭에는 주석을 단다.

---

## 8. 테스트 / 툴링 / 빌드

- **Google Test** (`gtest`), 전용 폴더 `Test/`(jugx) / `JamTest/`(jamengine), `Main.cpp` 가 `::testing::InitGoogleTest(); return RUN_ALL_TESTS();`. jamengine 은 `Application` 을 상속해 `Update()` 에서 `RUN_ALL_TESTS()`.
- 파일명: `<대상>Test.cpp` (`RingBufferTest.cpp`, `LruCacheTest.cpp`, `JsonSerializerTest.cpp`). 테스트 이름: `TEST(<대상>, <PascalCase 문장>)` — `TEST(RingBuffer, DefaultCapacityIsEight)`, `TEST(RingBuffer, PushIncreasesSizeAndPreservesOrder)`. jamengine 은 `TEST(BoundingVolumeTest, Region_AllCombinations)` 처럼 `<Suite>Test` + 언더스코어 구분 `[차이]`.
- 테스트 파일 첫 줄은 **한글 설명 주석**(동작 요약, 특이 규칙), 섹션은 `// ====` 배너로 구분 (`// 기본 상태`, `// Push / Emplace / Pop`).
- 테스트는 `using namespace jug;` 사용(테스트 파일에 한해), 헬퍼는 익명 네임스페이스에, 검증은 `EXPECT_EQ`/`EXPECT_TRUE`.
- 벤치마크 `Bench/`, 도구 스크립트 `Scripts/` (`generate_header.py`).
- 빌드: jugx = **CMake 3.25 + vcpkg + CMakePresets (Ninja Multi-Config / VS)**, C++20, `CMAKE_CXX_EXTENSIONS OFF`, `file(GLOB … CONFIGURE_DEPENDS)`, `target_precompile_headers(... PUBLIC pch.h)`, 옵션 `JUGX_BUILD_TESTS`/`BENCHMARKS`/`INSTALL`. jamengine/juggfx = Visual Studio 솔루션(`.slnx`/`.vcxproj`), 구성 `Debug|Release x64`. 클론은 MSBuild + `GenerateProjects.ps1` 로 목록 명시.
- 디버그/릴리스 매크로: `JUG_DEBUG` (`Config.h`), `JUG_ENABLE_ASSERT`.
- 도구: clang-format (필수 파일), ReSharper 주석/NOLINT, 커밋 메시지는 짧은 한국어/영어 명사구 ("백업", "Sync vcxproj/filters with current JugX sources").

---

## 9. 시그니처 습관 TOP 15

이 열다섯 개가 한꺼번에 보이면 "JuggernautSensei 코드"다.

1. **모든 매개변수 `_camelCase`**, 정의에서는 **전부 `const`** 이고 **한 줄에 하나씩** 세로로 쌓는다 (인자 1개도 줄바꿈).
2. **멤버 `m_`**, bool `m_b`, 포인터 `m_p`; 로컬 bool `b`, 로컬 포인터 `p`; null 가능은 `…OrNull`/`…OrEmpty`/`…OrInvalid`.
3. **private 함수·익명 NS 함수는 후행 `_`** (`LogPrefix_()`, `MakeSlot_()`).
4. **`enum class eXxx`** (소문자 `e` + PascalCase 타입, PascalCase 값), 상수는 `kXxx`, 파일 전역 싱글턴 포인터는 `g_pSingleton`.
5. **Allman 중괄호 + 4칸**, `if` 한 줄도 중괄호, `}   // namespace jug` (3칸 공백 주석), 네임스페이스 첫/끝 빈 줄.
6. **열 정렬**: 연속 선언의 타입·이름·`=`, 행 끝 주석, 반환형, 순수가상 `= 0`, `= default` 모두 세로로 맞춘다.
7. **`[[nodiscard]]` 를 getter/순수 질의 모두에**, 선언 맨 앞.
8. **`JUG_CLASS(Foo, NO_COPY, NO_MOVE)`** 첫 줄 매크로로 복사/이동 삭제; 소멸자 `virtual ~X() = default;` 와 `X() = default;` 정렬.
9. **`// ====` 3줄 배너**로 헤더/구현 섹션 구분, 한글 설명 블록.
10. **자체 타입 별칭 계층** `Vector/String/StringView/Span/ARRAY/HashMap/Optional/Flags/ENUM_ARRAY` — `std::` 직접 사용을 거의 안 한다.
11. **핸들 + 풀** (`Handle<T>`, `ResourcePool`, `kInvalidHandle`) 로 자원 소유, 포인터를 밖에 안 준다.
12. **`Result<T>`/`Error` + `kOK` 태그 + 에러 카테고리**; 예외 없음, 전제조건은 `JUG_ASSERT(cond, "영어 메시지.\n")`.
13. **`#pragma once` + 얇은 PCH + `.cpp` 첫 두 줄은 `#include "pch.h"`, 자기 헤더**, 그 뒤 빈 줄로 구분된 `<std>` 와 `"proj"` 알파벳 순.
14. **태그 디스패치**와 `/* implicit */` 주석 (`kNoInit`, `kZeroFlag`, `kAllFlag`, `kNullHandle`, `JUG_DEFINE_TAG`).
15. **concept + `requires`** (이름 `…T`), `xxx_detail` 소문자 네임스페이스, 멤버 데이터는 NSDMI 로 `= {}`/`= 0`/`= nullptr` 전부 초기화.

(보너스: 0.f/1.f 리터럴, `Type* p`(별은 타입 쪽), 값 `StringView` 전달, `Span<const T>` 반환, `OrNull`/`Or…` 접미 남용.)

---

## 10. 클론 코드에 적용한 Before → After

대상: `Source/Game/Enemies/Soldier.cpp` (현재 파일 일부). 이미 네임스페이스(`slug`)·`m_`/`_`/`k` 규칙은 맞지만 **함수 서명이 한 줄에 몰려 있고, `enum eSoldierState : int` 가 unscoped + `k` 값**이라 저장소 스타일과 다르다.

### Before (현재)

```cpp
namespace slug
{
namespace
{
    enum eSoldierState : int
    {
        kWait,
        kFall,
        kLand,
        kIdle,
        kMove,
        kMoveDown,
        kHJump,
        kThrow,
        kSlash,
        kJumpSlashReady,
        kJumpSlash,
        kJumpSlashLand
    };

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _state);

    [[nodiscard]] float AbsDx_(const Enemy& _e, const EnemyEnv& _env)
    {
        return std::abs(DxToPlayer(_e, _env));
    }

    void Reposition_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const int _fallback)
    {
        const SoldierCfg& cfg  = _def.soldier;
        const float       dy   = DyToPlayer(_e, _env);
        if (_e.bGrounded && _e.bOnOneWay && dy > cfg.dropDy)
        {
            Go_(_e, _def, _env, kMoveDown);
        }
        else if (_e.bGrounded && dy < -cfg.jumpDy)
        {
            Go_(_e, _def, _env, kHJump);
        }
        else
        {
            Go_(_e, _def, _env, _fallback);
        }
    }
```

### After (JuggernautSensei 스타일)

```cpp
namespace slug
{

namespace
{
    enum class eSoldierState
    {
        Wait,
        Fall,
        Land,
        Idle,
        Move,
        MoveDown,
        HJump,
        Throw,
        Slash,
        JumpSlashReady,
        JumpSlash,
        JumpSlashLand
    };

    void Go_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        eSoldierState   _state);

    [[nodiscard]] float AbsDx_(
        const Enemy&    _e,
        const EnemyEnv& _env)
    {
        return std::abs(DxToPlayer(_e, _env));
    }

    void Reposition_(
        Enemy&              _e,
        const EnemyDef&     _def,
        EnemyEnv&           _env,
        const eSoldierState _fallback)
    {
        const SoldierCfg& cfg = _def.soldier;
        const float       dy  = DyToPlayer(_e, _env);

        if (_e.bGrounded && _e.bOnOneWay && dy > cfg.dropDy)
        {
            Go_(_e, _def, _env, eSoldierState::MoveDown);
        }
        else if (_e.bGrounded && dy < -cfg.jumpDy)
        {
            Go_(_e, _def, _env, eSoldierState::HJump);
        }
        else
        {
            Go_(_e, _def, _env, _fallback);
        }
    }

}   // namespace
```

변경 포인트 (모두 위 절의 규칙에서 나온 것):
- 3.1: 매개변수 세로 정렬, `const` 정렬, 지역 선언 열 정렬(`cfg`/`dy` 의 `=` 를 맞춤 — 여분 공백 제거), 네임스페이스 직후·직전 빈 줄, `}   // namespace`.
- 2: `enum` → `enum class eSoldierState`, 값은 PascalCase 로 `k` 접두 제거 (`kWait` → `Wait`). 상태를 `int` 로 넘기던 매개변수도 `eSoldierState` 로 강타입화. (연동 코드의 `int` 상태 필드는 `static_cast<int>(…)` 로 변환하거나 필드 타입을 바꾼다 — 이 enum 이 파일 내부 전용이므로 영향이 작다.)
- 4.5: 값 매개변수 `_state` 도 `const` 로 (정의 쪽). 선언은 생략 가능.
- 0절 규칙: `auto`/람다/C 배열은 처음부터 사용하지 않았다.

### 보조 예 — 헤더 접근자와 클래스 형태 (`Source/Game/Ui/FadeController.h`)

저장소 스타일이라면 한 줄 접근자를 헤더에 인라인으로 두지 않고 .cpp 로 옮긴다 (비템플릿). 또 닫는 괄호는 `}   // namespace slug`. 접근자 반환형 열 정렬은 `uint8_t` 폭에 맞춘다:

```cpp
// Before
[[nodiscard]] float  GetAlpha() const { return m_alpha; }
[[nodiscard]] bool   IsBusy() const { return m_phase == ePhase::FadingToBlack || m_phase == ePhase::FadingToClear; }
...
}                                  // namespace slug 주석 없음

// After
[[nodiscard]] float   GetAlpha() const;
[[nodiscard]] ePhase  GetPhase() const;
[[nodiscard]] uint8_t GetAlpha255() const;
[[nodiscard]] bool    IsBusy() const;
[[nodiscard]] bool    IsBlack() const;
[[nodiscard]] bool    IsClear() const;
...
}   // namespace slug
```

```cpp
// FadeController.cpp (After)
float FadeController::GetAlpha() const
{
    return m_alpha;
}

bool FadeController::IsBusy() const
{
    return m_phase == ePhase::FadingToBlack || m_phase == ePhase::FadingToClear;
}
```

(주의: 클론은 hot path 접근자를 헤더 인라인으로 두는 편이므로 성능 민감 지점은 `constexpr`/인라인 유지가 허용된다. 저장소 규칙은 "기본은 .cpp, 템플릿/constexpr 만 헤더".)

---

## 11. 한 장 요약 체크리스트 (코드 작성 직후 자기 점검용)

- [ ] `auto` / 람다(알고리즘 인자 제외) / C 배열이 없다.
- [ ] `.cpp` 시작: `pch.h`(있다면) → 자기 헤더 → 빈 줄 → `<std>` → 빈 줄 → `"proj"`.
- [ ] 타입·함수 PascalCase, 멤버 `m_`, 매개변수 `_`, private 함수 후행 `_`, 상수 `k`, enum 은 `enum class eX`.
- [ ] bool 은 `b`, 포인터는 `p`, null 가능은 `OrNull`.
- [ ] 정의의 매개변수는 세로 정렬 + `const`; 지역 선언 열 정렬; Allman.
- [ ] 모든 getter 에 `[[nodiscard]]`; 질의는 `Get/Is/Has`.
- [ ] 멤버는 `= {}` 로 기본 초기화; 클래스 본문 마지막이 데이터.
- [ ] 복사/이동 규칙은 `JUG_CLASS(...)` 한 줄 (클론에는 `SLUG_CLASS` 가 아직 없으므로 `= delete` 4줄을 `=` 정렬해 직접 쓴다).
- [ ] 에러는 `Result<T>`, 전제조건은 assert, 예외 없음.
- [ ] `}   // namespace slug` (공백 3칸).

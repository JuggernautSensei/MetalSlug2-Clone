cmake_minimum_required(VERSION 3.25)

project(MyMetalSlug2
    VERSION 0.1.0
    DESCRIPTION "Metal Slug style 2D action game"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 실행 파일은 Build/<preset>/bin/<Config>/ 로 모은다.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# ---------------------------------------------------------------------------
# 외부 라이브러리 (vcpkg)
# ---------------------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)
find_package(yyjson CONFIG REQUIRED)
find_path(STB_INCLUDE_DIR "stb_image.h" REQUIRED)
find_path(MINIAUDIO_INCLUDE_DIR "miniaudio.h" REQUIRED)

# ---------------------------------------------------------------------------
# SlugCore : 게임 로직 정적 라이브러리 (exe / 테스트 / 도구가 공유)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE SLUG_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")
file(GLOB_RECURSE SLUG_CORE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")
list(FILTER SLUG_CORE_SOURCES EXCLUDE REGEX "/Source/(Main|Tools/.*)\\.cpp$")

add_library(SlugCore STATIC ${SLUG_CORE_SOURCES} ${SLUG_CORE_HEADERS})
target_include_directories(SlugCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Source")
target_include_directories(SlugCore SYSTEM PUBLIC "${STB_INCLUDE_DIR}" "${MINIAUDIO_INCLUDE_DIR}")
target_link_libraries(SlugCore PUBLIC SDL3::SDL3 yyjson::yyjson)
target_compile_features(SlugCore PUBLIC cxx_std_20)
target_compile_definitions(SlugCore PUBLIC
    NOMINMAX
    SLUG_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Assets"
)
if(WIN32)
    target_compile_definitions(SlugCore PUBLIC UNICODE _UNICODE)
endif()
target_compile_options(SlugCore PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/utf-8;/permissive-;/Zc:preprocessor>)
target_compile_options(SlugCore PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/MP>)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" FILES ${SLUG_CORE_SOURCES} ${SLUG_CORE_HEADERS})

# ---------------------------------------------------------------------------
# Slug : 게임 실행 파일
# ---------------------------------------------------------------------------
add_executable(Slug "${CMAKE_CURRENT_SOURCE_DIR}/Source/Main.cpp")
target_link_libraries(Slug PRIVATE SlugCore)
set_target_properties(Slug PROPERTIES OUTPUT_NAME "MyMetalSlug2")   # 실행 파일: MyMetalSlug2.exe
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Slug)   # Visual Studio 시작 프로젝트
if(MSVC)
    # 콘솔 창 없이 실행하되 main() 진입점을 유지한다.
    set_target_properties(Slug PROPERTIES
        WIN32_EXECUTABLE OFF
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

# ---------------------------------------------------------------------------
# 에셋 변환 도구 (Baker)
# ---------------------------------------------------------------------------
file(GLOB SLUG_BAKER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Tools/Baker/*.cpp")
if(SLUG_BAKER_SOURCES)
    add_executable(SlugBaker ${SLUG_BAKER_SOURCES})
    target_link_libraries(SlugBaker PRIVATE SlugCore)
    set_target_properties(SlugBaker PROPERTIES FOLDER "Tools")
endif()

# ---------------------------------------------------------------------------
# 헤드리스 리플레이 검증 도구
# ---------------------------------------------------------------------------
file(GLOB SLUG_HEADLESS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Tools/Headless/*.cpp")
if(SLUG_HEADLESS_SOURCES)
    add_executable(SlugHeadless ${SLUG_HEADLESS_SOURCES})
    target_link_libraries(SlugHeadless PRIVATE SlugCore)
    set_target_properties(SlugHeadless PROPERTIES FOLDER "Tools")
endif()

# ---------------------------------------------------------------------------
# 테스트 (GoogleTest)
# ---------------------------------------------------------------------------
find_package(GTest CONFIG QUIET)
if(GTest_FOUND)
    enable_testing()
    file(GLOB SLUG_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")
    add_executable(SlugTest ${SLUG_TEST_SOURCES})
    target_link_libraries(SlugTest PRIVATE SlugCore GTest::gtest)
    set_target_properties(SlugTest PROPERTIES FOLDER "Tests")
    include(GoogleTest)
    gtest_discover_tests(SlugTest WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

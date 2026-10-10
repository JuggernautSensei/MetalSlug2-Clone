cmake_minimum_required(VERSION 3.20)
project(MyMetalSlug2 CXX)

# Linux/CMake build (Windows uses MyMetalSlug2.sln + vcpkg).
# Configure with -DCMAKE_PREFIX_PATH=<prefix containing SDL3 and yyjson>.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(SDL3 REQUIRED CONFIG)
find_package(GTest REQUIRED)
find_path(YYJSON_INCLUDE_DIR yyjson.h REQUIRED)
find_library(YYJSON_LIBRARY yyjson REQUIRED)
find_path(STB_INCLUDE_DIR stb_image.h PATH_SUFFIXES stb REQUIRED)
find_path(STB_VORBIS_DIR stb_vorbis.c REQUIRED)
find_path(MINIAUDIO_INCLUDE_DIR miniaudio.h REQUIRED)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    Source/Audio/*.cpp Source/Base/*.cpp Source/Core/*.cpp
    Source/Game/*.cpp Source/Render/*.cpp Source/World/*.cpp)

add_library(SlugCore STATIC ${CORE_SOURCES})
target_include_directories(SlugCore PUBLIC Source ${STB_VORBIS_DIR} ${YYJSON_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${MINIAUDIO_INCLUDE_DIR})
target_compile_definitions(SlugCore PUBLIC SLUG_ASSET_DIR="${CMAKE_SOURCE_DIR}/Assets")
target_link_libraries(SlugCore PUBLIC SDL3::SDL3 ${YYJSON_LIBRARY})

add_executable(MyMetalSlug2 Source/Main.cpp)
target_link_libraries(MyMetalSlug2 PRIVATE SlugCore)

add_executable(SlugHeadless Source/Tools/Headless/Main.cpp)
target_link_libraries(SlugHeadless PRIVATE SlugCore)

file(GLOB BAKER_SOURCES CONFIGURE_DEPENDS Source/Tools/Baker/*.cpp)
add_executable(SlugBaker ${BAKER_SOURCES})
target_link_libraries(SlugBaker PRIVATE SlugCore)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS Test/*.cpp)
add_executable(SlugTest ${TEST_SOURCES})
target_link_libraries(SlugTest PRIVATE SlugCore GTest::gtest)

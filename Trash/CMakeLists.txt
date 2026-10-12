cmake_minimum_required(VERSION 3.20)
project(MetalSlug2_Clone CXX)

# Linux/CMake build (Windows uses MetalSlug2_Clone.sln + vcpkg).
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

# MetalSlug2_Clone: 게임 + --headless 모드. SlugTest: 같은 소스에 Test/*.cpp 를 더한다.
add_executable(MetalSlug2_Clone Source/Main.cpp ${CORE_SOURCES})
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS Test/*.cpp)
add_executable(SlugTest ${TEST_SOURCES} ${CORE_SOURCES})
foreach(T MetalSlug2_Clone SlugTest)
    target_include_directories(${T} PRIVATE Source ${STB_VORBIS_DIR} ${YYJSON_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${MINIAUDIO_INCLUDE_DIR})
    target_compile_definitions(${T} PRIVATE SLUG_ASSET_DIR="${CMAKE_SOURCE_DIR}/Assets")
    target_link_libraries(${T} PRIVATE SDL3::SDL3 ${YYJSON_LIBRARY})
endforeach()
target_link_libraries(SlugTest PRIVATE GTest::gtest)

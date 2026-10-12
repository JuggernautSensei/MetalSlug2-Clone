#!/bin/bash
# Cloud session setup: installs build deps (SDL3, yyjson, stb_vorbis, gtest...) and builds with CMake.
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

PREFIX=/opt/deps
SDL_TAG=release-3.2.8
YYJSON_TAG=0.12.0
ROOT="${CLAUDE_PROJECT_DIR:-$(cd "$(dirname "$0")/../.." && pwd)}"

if ! dpkg -s libgtest-dev libstb-dev libminiaudio-dev ninja-build xvfb libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libxfixes-dev libxss-dev libgl-dev libasound2-dev libdrm-dev libgbm-dev libegl-dev >/dev/null 2>&1; then
  apt-get update -qq
  DEBIAN_FRONTEND=noninteractive apt-get install -y -qq libgtest-dev libstb-dev libminiaudio-dev ninja-build xvfb \
    libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libxfixes-dev libxss-dev libgl-dev \
    libasound2-dev libdrm-dev libgbm-dev libegl-dev
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

if [ ! -f "$PREFIX/lib/libSDL3.a" ]; then
  git clone -q --depth 1 --branch "$SDL_TAG" https://github.com/libsdl-org/SDL.git "$WORK/sdl"
  cmake -S "$WORK/sdl" -B "$WORK/sdl/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC=ON -DSDL_SHARED=OFF \
    -DSDL_TEST_LIBRARY=OFF -DCMAKE_INSTALL_PREFIX="$PREFIX" >/dev/null
  cmake --build "$WORK/sdl/build" >/dev/null
  cmake --install "$WORK/sdl/build" >/dev/null
fi

if [ ! -f "$PREFIX/lib/libyyjson.a" ]; then
  git clone -q --depth 1 --branch "$YYJSON_TAG" https://github.com/ibireme/yyjson.git "$WORK/yyjson"
  cmake -S "$WORK/yyjson" -B "$WORK/yyjson/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" >/dev/null
  cmake --build "$WORK/yyjson/build" >/dev/null
  cmake --install "$WORK/yyjson/build" >/dev/null
fi

if [ ! -f "$PREFIX/include/stb_vorbis.c" ]; then
  mkdir -p "$PREFIX/include"
  curl -sSfL -o "$PREFIX/include/stb_vorbis.c" https://raw.githubusercontent.com/nothings/stb/master/stb_vorbis.c
fi

cd "$ROOT"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PREFIX" >/dev/null
cmake --build build


if [ -n "${CLAUDE_ENV_FILE:-}" ]; then
  echo "export CMAKE_PREFIX_PATH=$PREFIX" >> "$CLAUDE_ENV_FILE"
fi

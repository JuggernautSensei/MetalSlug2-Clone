#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// JugX: 우산 헤더(JugX.h)는 Windows.h 를 끌어와 GetObject 매크로가 JsonReader 를 깨므로 필요한 것만 개별 include
#include <JugX/Config.h>
#include <JugX/Macro.h>
#include <JugX/Platform.h>
#include <JugX/Typedef.h>
#include <JugX/Assert.h>
#include <JugX/Handle.h>

#include "Base/Types.h"
#include "Base/Handle.h"

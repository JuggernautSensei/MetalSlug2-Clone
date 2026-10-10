#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "Base/Result.h"
#include "Tools/Baker/AnimMath.h"
#include "Tools/Baker/BakerData.h"
#include "Tools/Baker/Image.h"

// 원본 스프라이트 시트(Assets/Source/*.png, spriters-resource 류) 를 sheets.json 규칙으로 잘라
// 레거시와 같은 아틀라스/anim.json/sprites.json 경로로 굽는다. 순수 알고리즘은 이 헤더에 inline 으로 둬서
// SlugTest 에서 바로 검증한다. JSON 로딩과 Baker 연결은 SheetSource.cpp (SlugBaker 전용).
namespace slug::baker
{
enum class eSheetKeyMode
{
    Color,     // colors 목록(채널별 ±tol)과 같은 픽셀 = 배경
    Alpha,     // 알파 0 = 배경
    Magenta    // (255,0,255) ±tol = 배경
};

enum class eSheetPivot
{
    Foot,      // 트림된 프레임의 아래 중앙 (x = w/2, y = h)
    Center     // 트림된 프레임의 중앙 (x = w/2, y = h/2)
};

struct SheetKey
{
    eSheetKeyMode                       mode = eSheetKeyMode::Magenta;
    std::vector<std::array<uint8_t, 3>> colors;
    int                                 tol  = 0;
};

struct SheetAuto
{
    bool bRows     = true;   // true: 행(밴드) 단위로 묶은 뒤 x 정렬. false: 한 줄로 보고 x 정렬
    int  mergeGap  = 2;      // 이 간격 이내의 픽셀은 같은 스프라이트로 본다 (팔다리/이펙트 조각 병합)
    int  minPixels = 30;     // 내용 픽셀이 이보다 적은 덩어리는 버린다 (노이즈, 점)
};

struct SheetClipSpec
{
    std::string        name;
    bool               bHasRegion = false;
    IRect              region;                 // auto 슬라이스 범위 (비어 있으면 시트 전체)
    bool               bAuto      = false;
    SheetAuto          autoOpts;
    std::vector<IRect> frames;                 // 명시 프레임 사각형 (있으면 auto 보다 우선)
    eSheetPivot        pivot     = eSheetPivot::Foot;
    bool               bLoop     = true;
    double             fps       = 12.0;
    bool               bMirror   = false;      // _Right <-> _Left 좌우 반전 클립을 추가로 굽는다
    bool               bAsSprite = false;      // 프레임 1개짜리를 sprites.json 에 넣는다
    bool               bReplace  = false;      // 같은 이름의 레거시/기존 클립을 대체 허용
    int                scale     = 0;          // 0 = 시트 배율 상속, 1 이상 = 이 클립만 덮어씀
};

struct SheetSpec
{
    std::string                file;           // sheets.json 기준 상대 경로 (보통 같은 폴더의 PNG)
    SheetKey                   key;
    std::vector<IRect>         exclude;        // 시트 좌표계에서 무시할 영역 (라벨 텍스트 등)
    int                        scale = 1;      // 기본 최근접 확대 배율 (정수 1..kSheetMaxScale)
    std::vector<SheetClipSpec> clips;
};

struct SheetConfig
{
    std::filesystem::path  path;               // 로드한 sheets.json 경로
    std::vector<SheetSpec> sheets;
};

enum class eSheetNameAction
{
    Add,       // 이름이 비어 있다
    Replace,   // 이름이 있고 replace:true
    Skip       // 이름이 있고 replace 가 없다 -> 경고 후 건너뜀
};

// 같은 이름의 기존 클립 처리 규칙. 레거시 이름은 replace:true 가 아니면 절대 덮어쓰지 않는다.
[[nodiscard]] inline eSheetNameAction ResolveSheetName(
    const bool _bExists,
    const bool _bReplace)
{
    if (!_bExists)
    {
        return eSheetNameAction::Add;
    }
    return _bReplace ? eSheetNameAction::Replace : eSheetNameAction::Skip;
}

// "X_Right" <-> "X_Left" (토큰 단위, 마지막 등장 기준). 대응 이름이 없으면 false.
[[nodiscard]] inline bool MirrorClipName(
    const std::string& _name,
    std::string&       _outName)
{
    struct Pair { const char* from; const char* to; };
    constexpr Pair kPairs[] = { { "_Right", "_Left" }, { "_Left", "_Right" } };
    for (const Pair& p: kPairs)
    {
        const std::string from = p.from;
        for (size_t pos = _name.rfind(from); pos != std::string::npos; pos = (pos == 0) ? std::string::npos : _name.rfind(from, pos - 1))
        {
            const size_t end = pos + from.size();
            if (end == _name.size() || _name[end] == '_')
            {
                _outName = _name.substr(0, pos) + p.to + _name.substr(end);
                return true;
            }
        }
    }
    return false;
}

[[nodiscard]] inline bool IsKeyBackground(
    const uint8_t*   _pPixel,
    const SheetKey&  _key)
{
    if (_pPixel[3] == 0)
    {
        return true;
    }
    switch (_key.mode)
    {
        case eSheetKeyMode::Alpha:
            return false;
        case eSheetKeyMode::Magenta:
            return std::abs(static_cast<int>(_pPixel[0]) - 255) <= _key.tol
                && std::abs(static_cast<int>(_pPixel[1])) <= _key.tol
                && std::abs(static_cast<int>(_pPixel[2]) - 255) <= _key.tol;
        case eSheetKeyMode::Color:
            for (const std::array<uint8_t, 3>& c: _key.colors)
            {
                if (std::abs(static_cast<int>(_pPixel[0]) - c[0]) <= _key.tol
                    && std::abs(static_cast<int>(_pPixel[1]) - c[1]) <= _key.tol
                    && std::abs(static_cast<int>(_pPixel[2]) - c[2]) <= _key.tol)
                {
                    return true;
                }
            }
            return false;
    }
    return false;
}

// 배경 픽셀을 완전 투명(RGBA 0)으로 바꾼다. 전경 픽셀은 원래 알파를 유지한다. 반환: 배경 픽셀 수.
inline uint64_t ApplySheetKey(
    Image&          _image,
    const SheetKey& _key)
{
    uint64_t bgCount = 0;
    uint8_t* p       = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (IsKeyBackground(p + i, _key))
        {
            p[i] = p[i + 1] = p[i + 2] = p[i + 3] = 0;
            ++bgCount;
        }
    }
    return bgCount;
}

inline void FlipHorizontal(Image& _image)
{
    for (int y = 0; y < _image.height; ++y)
    {
        for (int x = 0, xe = _image.width - 1; x < xe; ++x, --xe)
        {
            uint8_t* a = _image.At(x, y);
            uint8_t* b = _image.At(xe, y);
            for (int c = 0; c < 4; ++c)
            {
                std::swap(a[c], b[c]);
            }
        }
    }
}

[[nodiscard]] inline Pivot ComputeSheetPivot(
    const int        _w,
    const int        _h,
    const eSheetPivot _policy)
{
    if (_policy == eSheetPivot::Center)
    {
        return { _w / 2, _h / 2 };
    }
    return { _w / 2, _h };
}

inline constexpr int kSheetMaxScale = 8;

// 클립 배율: 클립 값이 1 이상이면 그것, 아니면 시트 기본 배율.
[[nodiscard]] inline int ResolveSheetScale(
    const int _sheetScale,
    const int _clipScale)
{
    return _clipScale > 0 ? _clipScale : std::max(1, _sheetScale);
}

// 최근접 이웃 확대. 배경(알파 0)과 색은 블록 단위로 그대로 복제된다. _scale <= 1 이면 복사만 한다.
[[nodiscard]] inline Image ScaleNearest(
    const Image& _src,
    const int    _scale)
{
    if (_scale <= 1)
    {
        return _src;
    }
    Image dst = Image::Make(_src.width * _scale, _src.height * _scale);
    for (int y = 0; y < dst.height; ++y)
    {
        for (int x = 0; x < dst.width; ++x)
        {
            std::memcpy(dst.At(x, y), _src.At(x / _scale, y / _scale), 4);
        }
    }
    return dst;
}

struct BuiltFrames
{
    std::vector<FrameOut> frames;
    std::vector<Image>    trimmed;
    int                   cellW = 0;
    int                   cellH = 0;
};

// 사각형 -> 최근접 확대(_scale) -> 알파 기준 트림 -> 피벗. 확대를 트림/피벗보다 먼저 하므로
// 트림 좌표와 피벗은 확대된 프레임 기준이다. _bFlip 이면 좌우 반전 프레임(피벗 x 도 뒤집음).
[[nodiscard]] inline BuiltFrames BuildSheetFrames(
    const Image&              _image,
    const std::vector<IRect>& _rects,
    const eSheetPivot         _pivot,
    const bool                _bFlip,
    const int                 _scale = 1)
{
    BuiltFrames out;
    for (const IRect& rect: _rects)
    {
        Image crop = ScaleNearest(CropImage(_image, rect), _scale);
        out.cellW  = std::max(out.cellW, crop.width);
        out.cellH  = std::max(out.cellH, crop.height);

        FrameOut    f;
        const IRect bounds = FindOpaqueBounds(crop, { 0, 0, crop.width, crop.height }, 0);
        if (bounds.IsEmpty())
        {
            f.bEmpty = true;
            out.frames.push_back(f);
            out.trimmed.emplace_back();
            continue;
        }

        Image trimmed = CropImage(crop, bounds);
        f.trimX       = bounds.x;
        f.trimY       = bounds.y;
        f.w           = bounds.w;
        f.h           = bounds.h;
        Pivot pivot   = ComputeSheetPivot(f.w, f.h, _pivot);
        if (_bFlip)
        {
            FlipHorizontal(trimmed);
            pivot.x = f.w - pivot.x;
        }
        f.pivotX = pivot.x;
        f.pivotY = pivot.y;
        out.frames.push_back(f);
        out.trimmed.push_back(std::move(trimmed));
    }
    return out;
}

// 키 적용이 끝난 시트에서 내용(알파 > 0) 덩어리를 찾아 사각형 목록을 돌려준다.
// 1) region 안에서 exclude 를 제외한 전경 마스크  2) mergeGap 만큼 팽창(가로/세로 분리형 윈도 합)
// 3) 4-연결 BFS 로 덩어리 분리  4) 덩어리의 원본 전경 bbox 를 사각형으로, minPixels 미만은 버림
// 5) bRows 면 세로 중심 기준 행 밴드로 묶고 밴드 안에서 x 순, 아니면 x 순. 결과 좌표는 시트 좌표계.
[[nodiscard]] inline std::vector<IRect> SliceAuto(
    const Image&               _image,
    const IRect&               _region,
    const SheetAuto&           _opts,
    const std::vector<IRect>&  _exclude)
{
    std::vector<IRect> out;
    const int x0 = std::max(_region.x, 0);
    const int y0 = std::max(_region.y, 0);
    const int x1 = std::min(_region.x + _region.w, _image.width);
    const int y1 = std::min(_region.y + _region.h, _image.height);
    if (x0 >= x1 || y0 >= y1)
    {
        return out;
    }

    const int    w = x1 - x0;
    const int    h = y1 - y0;
    const size_t n = static_cast<size_t>(w) * static_cast<size_t>(h);
    std::vector<uint8_t> fg(n, 0);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            fg[static_cast<size_t>(y) * w + x] = _image.At(x0 + x, y0 + y)[3] > 0 ? 1 : 0;
        }
    }
    for (const IRect& ex: _exclude)
    {
        const int ex0 = std::max(ex.x, x0) - x0;
        const int ey0 = std::max(ex.y, y0) - y0;
        const int ex1 = std::min(ex.x + ex.w, x1) - x0;
        const int ey1 = std::min(ex.y + ex.h, y1) - y0;
        for (int y = ey0; y < ey1; ++y)
        {
            for (int x = ex0; x < ex1; ++x)
            {
                fg[static_cast<size_t>(y) * w + x] = 0;
            }
        }
    }

    // 팽창: 가로 윈도 OR -> 세로 윈도 OR (각 행/열 누적합으로 O(n))
    const int g = std::max(0, _opts.mergeGap);
    std::vector<uint8_t> hd(n, 0);
    {
        std::vector<int> pref(static_cast<size_t>(w) + 1, 0);
        for (int y = 0; y < h; ++y)
        {
            const size_t row = static_cast<size_t>(y) * w;
            for (int x = 0; x < w; ++x)
            {
                pref[x + 1] = pref[x] + fg[row + x];
            }
            for (int x = 0; x < w; ++x)
            {
                const int lo = std::max(0, x - g);
                const int hi = std::min(w, x + g + 1);
                hd[row + x]  = pref[hi] - pref[lo] > 0 ? 1 : 0;
            }
        }
    }
    std::vector<uint8_t> dl(n, 0);
    {
        std::vector<int> pref(static_cast<size_t>(h) + 1, 0);
        for (int x = 0; x < w; ++x)
        {
            for (int y = 0; y < h; ++y)
            {
                pref[y + 1] = pref[y] + hd[static_cast<size_t>(y) * w + x];
            }
            for (int y = 0; y < h; ++y)
            {
                const int lo = std::max(0, y - g);
                const int hi = std::min(h, y + g + 1);
                dl[static_cast<size_t>(y) * w + x] = pref[hi] - pref[lo] > 0 ? 1 : 0;
            }
        }
    }

    // 4-연결 BFS. 팽창 마스크 위에서 덩어리를 나누고 bbox/픽셀 수는 원본 전경으로 잰다.
    std::vector<uint8_t> visited(n, 0);
    std::vector<size_t>  stack;
    std::vector<IRect>   comps;
    for (size_t seed = 0; seed < n; ++seed)
    {
        if (!dl[seed] || visited[seed])
        {
            continue;
        }
        visited[seed] = 1;
        stack.assign(1, seed);
        int      minX  = w;
        int      minY  = h;
        int      maxX  = -1;
        int      maxY  = -1;
        int64_t  count = 0;
        while (!stack.empty())
        {
            const size_t p = stack.back();
            stack.pop_back();
            const int x = static_cast<int>(p % static_cast<size_t>(w));
            const int y = static_cast<int>(p / static_cast<size_t>(w));
            if (fg[p])
            {
                ++count;
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
            const int dx[4] = { 1, -1, 0, 0 };
            const int dy[4] = { 0, 0, 1, -1 };
            for (int k = 0; k < 4; ++k)
            {
                const int nx = x + dx[k];
                const int ny = y + dy[k];
                if (nx < 0 || ny < 0 || nx >= w || ny >= h)
                {
                    continue;
                }
                const size_t q = static_cast<size_t>(ny) * w + nx;
                if (dl[q] && !visited[q])
                {
                    visited[q] = 1;
                    stack.push_back(q);
                }
            }
        }
        if (count > 0 && count >= _opts.minPixels)
        {
            comps.push_back({ x0 + minX, y0 + minY, maxX - minX + 1, maxY - minY + 1 });
        }
    }

    // 정렬: 세로 중심 순으로 밴드를 나눈다. 후보의 중심이 밴드 첫 사각형 안에 있으면 같은 행.
    std::stable_sort(comps.begin(), comps.end(), [](const IRect& _a, const IRect& _b)
    {
        return (2 * _a.y + _a.h) < (2 * _b.y + _b.h);
    });
    if (!_opts.bRows)
    {
        std::stable_sort(comps.begin(), comps.end(), [](const IRect& _a, const IRect& _b) { return _a.x < _b.x; });
        return comps;
    }
    size_t i = 0;
    while (i < comps.size())
    {
        const IRect ref = comps[i];
        size_t      end = i + 1;
        while (end < comps.size() && (2 * comps[end].y + comps[end].h) < (2 * ref.y + 2 * ref.h))
        {
            ++end;
        }
        std::stable_sort(comps.begin() + static_cast<std::ptrdiff_t>(i), comps.begin() + static_cast<std::ptrdiff_t>(end), [](const IRect& _a, const IRect& _b) { return _a.x < _b.x; });
        for (size_t k = i; k < end; ++k)
        {
            out.push_back(comps[k]);
        }
        i = end;
    }
    return out;
}
}

namespace slug::baker
{
// sheets.json 로딩 + 시트 패스. SheetSource.cpp (SlugBaker 전용).
// 지형 편집 (Assets/Source/GroundEdits.json): 굽기 전에 지면 원본 비트맵의 사각형을 공기(op "air") 또는 고체(op "solid") 로 바꾼다.
// 예: 보스 아레나의 잔존 언덕 제거. 재현 가능: 같은 JSON 과 원본 BMP 면 항상 같은 Ground/*.bin 이 나온다.
struct GroundEdit
{
    std::string map;
    std::string op;
    int         x = 0;
    int         y = 0;
    int         w = 0;
    int         h = 0;
};

[[nodiscard]] slug::Result<std::vector<GroundEdit>> LoadGroundEdits(const std::filesystem::path& _path);

inline int ApplyGroundEdits(Image& _img, const std::vector<GroundEdit>& _edits, const std::string& _map)
{
    int applied = 0;
    for (const GroundEdit& edit: _edits)
    {
        if (edit.map != _map)
        {
            continue;
        }
        const uint8_t rgba[4] = { 255, 255, 255, 255 };   // 공기 (흰색은 IsAirPixel)
        const uint8_t solid[4] = { 0, 0, 255, 255 };      // 고체 (파랑은 IsSolidPixel)
        const uint8_t* pColor = edit.op == "solid" ? solid : rgba;
        for (int y = std::max(0, edit.y); y < std::min(_img.height, edit.y + edit.h); ++y)
        {
            for (int x = std::max(0, edit.x); x < std::min(_img.width, edit.x + edit.w); ++x)
            {
                uint8_t* p = _img.At(x, y);
                std::memcpy(p, pColor, 4);
                ++applied;
            }
        }
    }
    return applied;
}

[[nodiscard]] slug::Result<void> LoadSheetConfig(
    const std::filesystem::path& _path,
    SheetConfig&                 _out);

// 시트 패스: sheets.json 의 클립을 자른 뒤 state.anims / state.sprites 에 추가한다 (MergeUniques_ 이전에 호출).
[[nodiscard]] slug::Result<void> AppendSheetClips(
    const BakerOptions& _options,
    BakeState&          _state);

// --sheet-preview: 자동 검출/명시 사각형에 번호를 붙여 Captures/SheetPreview/<name>.png 를 쓴다.
[[nodiscard]] slug::Result<void> RunSheetPreview(const BakerOptions& _options);
}

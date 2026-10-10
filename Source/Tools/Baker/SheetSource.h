#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Tools/Baker/AnimMath.h"
#include "Tools/Baker/BakerData.h"
#include "Tools/Baker/Image.h"

// 원본 스프라이트 시트(Assets/Source/*.png, spriters-resource 류) 를 sheets.json 규칙으로 잘라
// 레거시와 같은 아틀라스/anim.json/sprites.json 경로로 굽는다. 순수 알고리즘은 이 헤더에 inline 으로 둬서
// SlugTest 에서 바로 검증한다. JSON 로딩과 Baker 연결은 SheetSource.cpp (SlugBaker 전용).
//
// 피벗 규칙 (프레임 간 떨림 방지):
//  - 기준은 "트림 전 원본 사각형"(확대 후 크기)이다. 중심 피벗 = (w/2 - trimX, h/2 - trimY), 발 피벗 = (w/2 - trimX, h - trimY).
//    같은 크기 셀끼리는 내용이 같은 위치에 있으면 자동으로 정렬된다.
//  - register (기본 true, 2프레임 이상, asSprite 아님, effect 아님, noRegisterPrefixes 밖): 프레임끼리 겹침 최대가 되는
//    정수 이동량을 찾아 내용을 맞춘 뒤, 합집합 bbox 의 피벗 모드 점을 공통 원점으로 쓴다.
namespace slug::baker
{

enum class eSheetKeyMode
{
    Color,    // colors 목록(채널별 ±tol)과 같은 픽셀 = 배경
    Alpha,    // 알파 0 = 배경
    Magenta   // (255,0,255) ±tol = 배경
};

enum class eSheetPivot
{
    Foot,    // 트림된 프레임의 아래 중앙 (x = w/2, y = h)
    Center   // 트림된 프레임의 중앙 (x = w/2, y = h/2)
};

enum class eSheetRegisterRef
{
    First,   // 첫 프레임에 맞춘다. 점수가 모자라면 직전 정렬 프레임에 한 번 더 맞춰 본다 (chain)
    Prev     // 직전 정렬 프레임에만 맞춘다 (누적)
};

struct SheetKey
{
    eSheetKeyMode             mode   = eSheetKeyMode::Magenta;
    Vector<ARRAY<uint8_t, 3>> colors = {};
    int                       tol    = 0;
};

struct SheetAuto
{
    bool bRows     = true;   // true: 행(밴드) 단위로 묶은 뒤 x 정렬. false: 한 줄로 보고 x 정렬
    int  mergeGap  = 2;      // 이 간격 이내의 픽셀은 같은 스프라이트로 본다 (팔다리/이펙트 조각 병합)
    int  minPixels = 30;     // 내용 픽셀이 이보다 적은 덩어리는 버린다 (노이즈, 점)
};

struct SheetClipSpec
{
    String            name         = {};
    bool              bHasRegion   = false;
    IRect             region       = {};   // auto 슬라이스 범위 (비어 있으면 시트 전체)
    bool              bAuto        = false;
    SheetAuto         autoOpts     = {};
    Vector<IRect>     frames       = {};   // 명시 프레임 사각형 (있으면 auto 보다 우선)
    eSheetPivot       pivot        = eSheetPivot::Foot;
    bool              bLoop        = true;
    double            fps          = 12.0;
    bool              bMirror      = false;   // _Right <-> _Left 좌우 반전 클립을 추가로 굽는다
    bool              bAsSprite    = false;   // 프레임 1개짜리를 sprites.json 에 넣는다
    bool              bReplace     = false;   // 같은 이름의 레거시/기존 클립을 대체 허용
    int               scale        = 0;       // 0 = 시트 배율 상속, 1 이상 = 이 클립만 덮어씀
    bool              bEffect      = false;   // 효과/투사체 클립: register 를 생략하면 정렬하지 않는다
    bool              bRegisterSet = false;   // sheets.json 에 register 키가 있었다
    bool              bRegister    = false;   // register 값 (bRegisterSet 일 때만 의미)
    eSheetRegisterRef registerRef  = eSheetRegisterRef::First;
    int               searchRange  = 40;      // 정렬 탐색 반경 (시트 px, 확대 배율을 곱해 쓴다)
    bool              bHasAnchor   = false;   // anchor: 첫 프레임 트림 좌표계의 엔티티 원점 (x,y)
    int               anchorX      = 0;
    int               anchorY      = 0;
};

struct SheetSpec
{
    String                file               = {};   // sheets.json 기준 상대 경로 (보통 같은 폴더의 PNG)
    SheetKey              key                = {};
    Vector<IRect>         exclude            = {};   // 시트 좌표계에서 무시할 영역 (라벨 텍스트 등)
    int                   scale              = 1;    // 기본 최근접 확대 배율 (정수 1..kSheetMaxScale)
    Vector<SheetClipSpec> clips              = {};
    Vector<String>        noRegisterPrefixes = {};   // 이 접두어로 시작하는 클립은 기본 정렬을 끈다 (projectile/effect 류)
};

struct SheetConfig
{
    FilePath          path   = {};   // 로드한 sheets.json 경로
    Vector<SheetSpec> sheets = {};
};

// 클립의 정렬 여부. register 키가 있으면 그 값, 없으면 기본 규칙:
// 2프레임 이상 && asSprite 아님 && effect 아님 && 접두어 목록에 없음.
[[nodiscard]] inline bool ResolveSheetRegister(
    const SheetSpec&     _sheet,
    const SheetClipSpec& _clip,
    const size_t         _frameCount)
{
    if (_clip.bRegisterSet)
    {
        return _clip.bRegister;
    }
    if (_clip.bAsSprite || _clip.bEffect || _frameCount < 2)
    {
        return false;
    }
    for (const String& prefix: _sheet.noRegisterPrefixes)
    {
        if (!prefix.empty() && _clip.name.starts_with(prefix))
        {
            return false;
        }
    }
    return true;
}

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
    const String& _name,
    String&       _outName)
{
    struct Pair
    {
        const char* from;
        const char* to;
    };
    constexpr ARRAY<Pair, 2> kPairs = {
        Pair { "_Right",  "_Left" },
         Pair {  "_Left", "_Right" }
    };
    for (const Pair& p: kPairs)
    {
        const String from = p.from;
        for (size_t pos = _name.rfind(from); pos != String::npos; pos = (pos == 0) ? String::npos : _name.rfind(from, pos - 1))
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
    const uint8_t*  _pPixel,
    const SheetKey& _key)
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
            for (const ARRAY<uint8_t, 3>& c: _key.colors)
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

// 트림된 프레임 기준 피벗 (기존 규칙). 지금은 "떨림 측정의 이전 상태" 용으로만 쓴다.
[[nodiscard]] inline Pivot ComputeSheetPivot(
    const int         _w,
    const int         _h,
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

// ---- 프레임 정렬 (register) ----

struct SheetRegisterOpts
{
    bool              bEnabled    = false;   // 프레임끼리 정렬한다
    eSheetRegisterRef ref         = eSheetRegisterRef::First;
    int               searchRange = 40;      // 탐색 반경 (시트 px; 확대 배율을 곱해 쓴다)
    double            minScore    = 0.5;     // 이 점수 미만이면 정렬하지 않는다 (fallback)
    bool              bHasAnchor  = false;   // 공통 원점을 첫 프레임 트림 좌표 (anchorX, anchorY) 로 고정
    int               anchorX     = 0;
    int               anchorY     = 0;
};

struct SheetRegStats
{
    bool           bRegistered    = false;   // 정렬을 시도했다 (2프레임 이상, register 켜짐)
    int            alignedFrames  = 0;       // 정렬에 성공한 프레임 수 (첫 프레임 제외)
    Vector<size_t> fallbackFrames = {};      // 점수가 모자라 정렬하지 않은 프레임 인덱스
    double         shiftBefore    = 0.0;     // 프레임 간 최대 내용 어긋남(px), 기존 트림 중심 피벗 기준
    double         shiftAfter     = 0.0;     // 같은 측정, 최종 피벗 기준
};

struct BuiltFrames
{
    Vector<FrameOut> frames  = {};
    Vector<Image>    trimmed = {};
    int              cellW   = 0;
    int              cellH   = 0;
    SheetRegStats    reg     = {};
};

inline constexpr int    kSheetCoarseFactor  = 4;      // 탐색 1차: 이 배율로 줄인 영상에서 찾는다
inline constexpr int    kSheetMeasureRange  = 40;     // 떨림 측정 반경 (시트 px)
inline constexpr double kSheetZeroTolerance = 0.02;   // 0 이동 점수가 최고점 - 이 값 이내면 0 이동을 택한다
inline constexpr double kSheetMinMass       = 8.0;    // 내용 면적(px) 이 이보다 작으면 정렬하지 않는다

// 불투명 면적 (알파 합 / 255). 정렬 정규화에 쓴다.
[[nodiscard]] inline double ContentMass(const Image& _img)
{
    double sum = 0.0;
    for (size_t i = 3, n = _img.pixels.size(); i < n; i += 4)
    {
        sum += _img.pixels[i];
    }
    return sum / 255.0;
}

// _b 를 _a 기준 (_dx,_dy) 만큼 옮겼을 때의 겹침 점수(Dice 형). 겹치는 픽셀마다 min(알파a, 알파b)/255 * (1 - 평균 RGB 차/255) 를 더하고
// _norm (= 두 면적의 평균) 으로 나눈다. min 정규화는 작은 프레임이 큰 프레임 안에 들어가면 1.0 이 되어 오정렬을 만든다.
[[nodiscard]] inline double AlignScore(
    const Image& _a,
    const Image& _b,
    const int    _dx,
    const int    _dy,
    const double _norm)
{
    if (_norm <= 0.0)
    {
        return 0.0;
    }
    const int x0  = std::max(0, _dx);
    const int y0  = std::max(0, _dy);
    const int x1  = std::min(_a.width, _dx + _b.width);
    const int y1  = std::min(_a.height, _dy + _b.height);
    double    sum = 0.0;
    for (int y = y0; y < y1; ++y)
    {
        for (int x = x0; x < x1; ++x)
        {
            const uint8_t* pa = _a.At(x, y);
            const uint8_t* pb = _b.At(x - _dx, y - _dy);
            const int      aa = pa[3];
            const int      ab = pb[3];
            if (aa == 0 || ab == 0)
            {
                continue;
            }
            const int diff = (std::abs(static_cast<int>(pa[0]) - pb[0]) + std::abs(static_cast<int>(pa[1]) - pb[1]) + std::abs(static_cast<int>(pa[2]) - pb[2])) / 3;
            sum += (std::min(aa, ab) / 255.0) * (1.0 - diff / 255.0);
        }
    }
    return sum / _norm;
}

// 블록(_f x _f) 에서 알파가 가장 큰 픽셀을 골라 줄인다. 얇은 실루엣이 사라지지 않게 최대 알파를 쓴다.
[[nodiscard]] inline Image DownsampleMaxAlpha(
    const Image& _src,
    const int    _f)
{
    if (_f <= 1)
    {
        return _src;
    }
    Image dst = Image::Make((_src.width + _f - 1) / _f, (_src.height + _f - 1) / _f);
    for (int y = 0; y < dst.height; ++y)
    {
        for (int x = 0; x < dst.width; ++x)
        {
            const uint8_t* best = nullptr;
            for (int by = y * _f; by < std::min(_src.height, (y + 1) * _f); ++by)
            {
                for (int bx = x * _f; bx < std::min(_src.width, (x + 1) * _f); ++bx)
                {
                    const uint8_t* p = _src.At(bx, by);
                    if (best == nullptr || p[3] > best[3])
                    {
                        best = p;
                    }
                }
            }
            if (best != nullptr)
            {
                std::memcpy(dst.At(x, y), best, 4);
            }
        }
    }
    return dst;
}

struct AlignResult
{
    int    dx    = 0;   // _b 의 _a 기준 상대 위치 (x)
    int    dy    = 0;   // (y)
    double score = 0.0;
};

// _b 의 상대 위치를 _baseX/_baseY 근처 ±_range 안에서 찾는다.
// 1차: kSheetCoarseFactor 배 축소 영상에서 전 범위 탐색, 2차: 최적 주변 ±factor 를 원본 해상도로 정밀 탐색.
// _bPreferBase 이면 기준 위치(0 이동)의 점수가 최고점 - kSheetZeroTolerance 이상일 때 기준 위치를 택한다.
[[nodiscard]] inline AlignResult BestAlign(
    const Image& _a,
    const Image& _b,
    const int    _baseX,
    const int    _baseY,
    const int    _range,
    const bool   _bPreferBase)
{
    const double massA    = ContentMass(_a);
    const double massB    = ContentMass(_b);
    const double normFull = 0.5 * (massA + massB);
    if (std::min(massA, massB) < kSheetMinMass)
    {
        return { _baseX, _baseY, 0.0 };
    }
    const AlignResult zero { _baseX, _baseY, AlignScore(_a, _b, _baseX, _baseY, normFull) };
    if (_range <= 0)
    {
        return zero;
    }

    AlignResult best { _baseX, _baseY, -1.0 };
    const int   f = _range >= 16 ? kSheetCoarseFactor : 1;
    if (f > 1)
    {
        const Image  ca    = DownsampleMaxAlpha(_a, f);
        const Image  cb    = DownsampleMaxAlpha(_b, f);
        const double normC = 0.5 * (ContentMass(ca) + ContentMass(cb));
        const int    cbx   = static_cast<int>(std::floor(static_cast<double>(_baseX) / f));
        const int    cby   = static_cast<int>(std::floor(static_cast<double>(_baseY) / f));
        const int    rc    = _range / f;
        AlignResult  coarse { cbx, cby, -1.0 };
        for (int dy = -rc; dy <= rc; ++dy)
        {
            for (int dx = -rc; dx <= rc; ++dx)
            {
                const double s = AlignScore(ca, cb, cbx + dx, cby + dy, normC);
                if (s > coarse.score)
                {
                    coarse = { cbx + dx, cby + dy, s };
                }
            }
        }
        const int fx = coarse.dx * f;
        const int fy = coarse.dy * f;
        for (int dy = -f; dy <= f; ++dy)
        {
            for (int dx = -f; dx <= f; ++dx)
            {
                const double s = AlignScore(_a, _b, fx + dx, fy + dy, normFull);
                if (s > best.score)
                {
                    best = { fx + dx, fy + dy, s };
                }
            }
        }
    }
    else
    {
        for (int dy = -_range; dy <= _range; ++dy)
        {
            for (int dx = -_range; dx <= _range; ++dx)
            {
                const double s = AlignScore(_a, _b, _baseX + dx, _baseY + dy, normFull);
                if (s > best.score)
                {
                    best = { _baseX + dx, _baseY + dy, s };
                }
            }
        }
    }

    if (_bPreferBase && zero.score >= best.score - kSheetZeroTolerance)
    {
        return zero;
    }
    return best;
}

// 프레임 위치(_dx,_dy: 공통 원점 기준 트림 좌상단)를 직전/첫 프레임에 맞춘다. _dx/_dy 는 입력이 기본(T) 위치이고 출력이 정렬된 위치.
// 점수가 _reg.minScore 미만인 프레임은 기본 위치를 그대로 두고 fallbackFrames 에 넣는다.
inline void RegisterSheetFrames(
    const Vector<Image>&     _trimmed,
    Vector<int>&             _dx,
    Vector<int>&             _dy,
    const SheetRegisterOpts& _reg,
    const int                _range,
    SheetRegStats&           _stats)
{
    const Vector<int> tx = _dx;
    const Vector<int> ty = _dy;
    Vector<size_t>    valid;
    for (size_t i = 0; i < _trimmed.size(); ++i)
    {
        if (!_trimmed[i].IsEmpty())
        {
            valid.push_back(i);
        }
    }
    if (valid.size() < 2)
    {
        return;
    }

    const size_t ref0 = valid[0];
    size_t       prev = ref0;
    for (size_t k = 1; k < valid.size(); ++k)
    {
        const size_t   i     = valid[k];
        Vector<size_t> cands = { ref0 };
        if (_reg.ref == eSheetRegisterRef::Prev)
        {
            cands = { prev };
        }
        else if (prev != ref0)
        {
            cands.push_back(prev);
        }

        bool bAligned = false;
        for (const size_t c: cands)
        {
            const AlignResult r = BestAlign(_trimmed[c], _trimmed[i], tx[i] - _dx[c], ty[i] - _dy[c], _range, true);
            if (r.score >= _reg.minScore)
            {
                _dx[i]   = _dx[c] + r.dx;
                _dy[i]   = _dy[c] + r.dy;
                bAligned = true;
                break;
            }
        }
        if (bAligned)
        {
            ++_stats.alignedFrames;
        }
        else
        {
            _dx[i] = tx[i];
            _dy[i] = ty[i];
            _stats.fallbackFrames.push_back(i);
        }
        prev = i;
    }
}

// 연속 프레임의 최대 내용 어긋남(px). _qx/_qy 는 그리기 좌상단 (원점 기준, = -피벗).
// 두 프레임을 피벗 기준 위치에 놓았을 때와 겹침이 가장 큰 위치의 차이를 잰다.
[[nodiscard]] inline double MaxContentShift(
    const Vector<Image>& _trimmed,
    const Vector<int>&   _qx,
    const Vector<int>&   _qy,
    const int            _range)
{
    double maxShift = 0.0;
    size_t prev     = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < _trimmed.size(); ++i)
    {
        if (_trimmed[i].IsEmpty())
        {
            continue;
        }
        if (prev != std::numeric_limits<size_t>::max())
        {
            const int         baseX = _qx[i] - _qx[prev];
            const int         baseY = _qy[i] - _qy[prev];
            const AlignResult r     = BestAlign(_trimmed[prev], _trimmed[i], baseX, baseY, _range, false);
            maxShift                = std::max(maxShift, static_cast<double>(std::max(std::abs(r.dx - baseX), std::abs(r.dy - baseY))));
        }
        prev = i;
    }
    return maxShift;
}

// 키 적용이 끝난 시트 이미지에서 사각형 목록을 자르고, 피벗을 계산한다.
// 사각형 -> 최근접 확대(_scale) -> 알파 기준 트림 -> 피벗. 확대를 트림/피벗보다 먼저 하므로
// 트림 좌표와 피벗은 확대된 프레임 기준이다. _bFlip 이면 좌우 반전 프레임(피벗 x 도 뒤집음).
//
// 피벗 = 공통 원점 W - 프레임 좌상단 D (그리기: topLeft = origin - pivot).
//  - 정렬하지 않음: W = (0,0) (rect 기준), D = 트림 좌상단 - rect 기준점 -> 원본 사각형의 중심/발 피벗.
//  - 정렬함: D 는 정렬된 위치, W = 합집합 bbox 의 피벗 모드 점 (발 = 아래 중앙, 중심 = 중앙). anchor 가 있으면 첫 프레임 기준.
[[nodiscard]] inline BuiltFrames BuildSheetFrames(
    const Image&             _image,
    const Vector<IRect>&     _rects,
    const eSheetPivot        _pivot,
    const bool               _bFlip,
    const int                _scale = 1,
    const SheetRegisterOpts& _reg   = {})
{
    BuiltFrames out;
    struct Cell
    {
        Image crop   = {};
        IRect bounds = {};
    };
    Vector<Cell> cells;
    cells.reserve(_rects.size());
    size_t nonEmpty = 0;
    for (const IRect& rect: _rects)
    {
        Cell cell;
        cell.crop   = ScaleNearest(CropImage(_image, rect), _scale);
        cell.bounds = FindOpaqueBounds(cell.crop, { 0, 0, cell.crop.width, cell.crop.height }, 0);
        out.cellW   = std::max(out.cellW, cell.crop.width);
        out.cellH   = std::max(out.cellH, cell.crop.height);
        nonEmpty += cell.bounds.IsEmpty() ? 0 : 1;
        cells.push_back(std::move(cell));
    }

    const bool   bRegistering = _reg.bEnabled && nonEmpty >= 2;
    const int    scale        = std::max(1, _scale);
    const int    range        = std::max(0, _reg.searchRange) * scale;
    const size_t n            = cells.size();

    // 1) 기본 위치 T: 트림 좌상단 - 원본 사각형 기준점. 정렬할 때는 기준이 중심, 아니면 피벗 모드.
    Vector<Image> trimmed(n);
    Vector<int>   tx(n, 0);
    Vector<int>   ty(n, 0);
    for (size_t i = 0; i < n; ++i)
    {
        const Cell& c = cells[i];
        if (c.bounds.IsEmpty())
        {
            continue;
        }
        trimmed[i]   = CropImage(c.crop, c.bounds);
        const int ax = c.crop.width / 2;
        const int ay = (bRegistering || _pivot == eSheetPivot::Center) ? c.crop.height / 2 : c.crop.height;
        tx[i]        = c.bounds.x - ax;
        ty[i]        = c.bounds.y - ay;
    }

    // 2) 프레임 정렬 -> 최종 위치 D
    Vector<int> dx      = tx;
    Vector<int> dy      = ty;
    out.reg.bRegistered = bRegistering;
    if (bRegistering)
    {
        RegisterSheetFrames(trimmed, dx, dy, _reg, range, out.reg);
    }

    // 3) 공통 원점 W
    int wx = 0;
    int wy = 0;
    if (bRegistering)
    {
        int x0 = std::numeric_limits<int>::max();
        int y0 = std::numeric_limits<int>::max();
        int x1 = std::numeric_limits<int>::min();
        int y1 = std::numeric_limits<int>::min();
        for (size_t i = 0; i < n; ++i)
        {
            if (trimmed[i].IsEmpty())
            {
                continue;
            }
            x0 = std::min(x0, dx[i]);
            y0 = std::min(y0, dy[i]);
            x1 = std::max(x1, dx[i] + trimmed[i].width);
            y1 = std::max(y1, dy[i] + trimmed[i].height);
        }
        wx = x0 + (x1 - x0) / 2;
        wy = (_pivot == eSheetPivot::Center) ? y0 + (y1 - y0) / 2 : y1;
    }
    if (_reg.bHasAnchor)
    {
        for (size_t i = 0; i < n; ++i)
        {
            if (!trimmed[i].IsEmpty())
            {
                wx = dx[i] + _reg.anchorX;
                wy = dy[i] + _reg.anchorY;
                break;
            }
        }
    }

    // 4) 프레임 출력 (피벗 = W - D, 좌우 반전이면 x 를 뒤집는다)
    Vector<int> beforeQx(n, 0);
    Vector<int> beforeQy(n, 0);
    Vector<int> afterQx(n, 0);
    Vector<int> afterQy(n, 0);
    for (size_t i = 0; i < n; ++i)
    {
        const Cell& c = cells[i];
        FrameOut    f;
        if (c.bounds.IsEmpty())
        {
            f.bEmpty = true;
            out.frames.push_back(f);
            out.trimmed.emplace_back();
            continue;
        }
        Image tr = std::move(trimmed[i]);
        f.trimX  = c.bounds.x;
        f.trimY  = c.bounds.y;
        f.w      = c.bounds.w;
        f.h      = c.bounds.h;

        Pivot       p { wx - dx[i], wy - dy[i] };
        const Pivot before = ComputeSheetPivot(f.w, f.h, _pivot);
        Pivot       b      = before;
        if (_bFlip)
        {
            FlipHorizontal(tr);
            p.x = f.w - p.x;
            b.x = f.w - b.x;
        }
        f.pivotX = p.x;
        f.pivotY = p.y;
        out.frames.push_back(f);
        out.trimmed.push_back(std::move(tr));

        beforeQx[i] = -b.x;
        beforeQy[i] = -b.y;
        afterQx[i]  = -p.x;
        afterQy[i]  = -p.y;
    }

    // 5) 떨림 측정: 연속 프레임의 내용 어긋남 (정렬 전 = 기존 트림 피벗, 정렬 후 = 최종 피벗)
    if (nonEmpty >= 2)
    {
        const int measureRange = kSheetMeasureRange * scale;
        out.reg.shiftBefore    = MaxContentShift(out.trimmed, beforeQx, beforeQy, measureRange);
        out.reg.shiftAfter     = MaxContentShift(out.trimmed, afterQx, afterQy, measureRange);
    }
    return out;
}

// 키 적용이 끝난 시트에서 내용(알파 > 0) 덩어리를 찾아 사각형 목록을 돌려준다.
// 1) region 안에서 exclude 를 제외한 전경 마스크  2) mergeGap 만큼 팽창(가로/세로 분리형 윈도 합)
// 3) 4-연결 BFS 로 덩어리 분리  4) 덩어리의 원본 전경 bbox 를 사각형으로, minPixels 미만은 버림
// 5) bRows 면 세로 중심 기준 행 밴드로 묶고 밴드 안에서 x 순, 아니면 x 순. 결과 좌표는 시트 좌표계.
[[nodiscard]] inline Vector<IRect> SliceAuto(
    const Image&         _image,
    const IRect&         _region,
    const SheetAuto&     _opts,
    const Vector<IRect>& _exclude)
{
    Vector<IRect> out;
    const int     x0 = std::max(_region.x, 0);
    const int     y0 = std::max(_region.y, 0);
    const int     x1 = std::min(_region.x + _region.w, _image.width);
    const int     y1 = std::min(_region.y + _region.h, _image.height);
    if (x0 >= x1 || y0 >= y1)
    {
        return out;
    }

    const int       w = x1 - x0;
    const int       h = y1 - y0;
    const size_t    n = static_cast<size_t>(w) * static_cast<size_t>(h);
    Vector<uint8_t> fg(n, 0);
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
    const int       g = std::max(0, _opts.mergeGap);
    Vector<uint8_t> hd(n, 0);
    {
        Vector<int> pref(static_cast<size_t>(w) + 1, 0);
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
    Vector<uint8_t> dl(n, 0);
    {
        Vector<int> pref(static_cast<size_t>(h) + 1, 0);
        for (int x = 0; x < w; ++x)
        {
            for (int y = 0; y < h; ++y)
            {
                pref[y + 1] = pref[y] + hd[static_cast<size_t>(y) * w + x];
            }
            for (int y = 0; y < h; ++y)
            {
                const int lo                       = std::max(0, y - g);
                const int hi                       = std::min(h, y + g + 1);
                dl[static_cast<size_t>(y) * w + x] = pref[hi] - pref[lo] > 0 ? 1 : 0;
            }
        }
    }

    // 4-연결 BFS. 팽창 마스크 위에서 덩어리를 나누고 bbox/픽셀 수는 원본 전경으로 잰다.
    constexpr ARRAY<int, 4> kDx = { 1, -1, 0, 0 };
    constexpr ARRAY<int, 4> kDy = { 0, 0, 1, -1 };
    Vector<uint8_t>         visited(n, 0);
    Vector<size_t>          stack;
    Vector<IRect>           comps;
    for (size_t seed = 0; seed < n; ++seed)
    {
        if (!dl[seed] || visited[seed])
        {
            continue;
        }
        visited[seed] = 1;
        stack.assign(1, seed);
        int     minX  = w;
        int     minY  = h;
        int     maxX  = -1;
        int     maxY  = -1;
        int64_t count = 0;
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
            for (size_t k = 0; k < kDx.size(); ++k)
            {
                const int nx = x + kDx[k];
                const int ny = y + kDy[k];
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
    std::stable_sort(comps.begin(), comps.end(), [](const IRect& _a, const IRect& _b) {
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

}   // namespace slug::baker

namespace slug::baker
{

// sheets.json 로딩 + 시트 패스. SheetSource.cpp (SlugBaker 전용).
// 지형 편집 (Assets/Source/GroundEdits.json): 굽기 전에 지면 원본 비트맵의 사각형을 공기(op "air") 또는 고체(op "solid") 로 바꾼다.
// 예: 보스 아레나의 잔존 언덕 제거. 재현 가능: 같은 JSON 과 원본 BMP 면 항상 같은 Ground/*.bin 이 나온다.
struct GroundEdit
{
    String map = {};
    String op  = {};
    int    x   = 0;
    int    y   = 0;
    int    w   = 0;
    int    h   = 0;
};

[[nodiscard]] slug::Result<Vector<GroundEdit>> LoadGroundEdits(const FilePath& _path);

inline int ApplyGroundEdits(Image& _img, const Vector<GroundEdit>& _edits, const String& _map)
{
    int applied = 0;
    for (const GroundEdit& edit: _edits)
    {
        if (edit.map != _map)
        {
            continue;
        }
        const ARRAY<uint8_t, 4> air    = { 255, 255, 255, 255 };   // 공기 (흰색은 IsAirPixel)
        const ARRAY<uint8_t, 4> solid  = { 0, 0, 255, 255 };       // 고체 (파랑은 IsSolidPixel)
        const uint8_t*          pColor = edit.op == "solid" ? solid.data() : air.data();
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
    const FilePath& _path,
    SheetConfig&    _out);

// 시트 패스: sheets.json 의 클립을 자른 뒤 state.anims / state.sprites 에 추가한다 (MergeUniques_ 이전에 호출).
[[nodiscard]] slug::Result<void> AppendSheetClips(
    const BakerOptions& _options,
    BakeState&          _state);

// --sheet-preview: 자동 검출/명시 사각형에 번호를 붙여 Captures/SheetPreview/<name>.png 를 쓴다.
[[nodiscard]] slug::Result<void> RunSheetPreview(const BakerOptions& _options);

}   // namespace slug::baker

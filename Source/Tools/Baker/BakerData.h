#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Tools/Baker/Ground.h"
#include "Tools/Baker/Image.h"
#include "Tools/Baker/LegacyParser.h"
#include "Tools/Baker/Packer.h"

namespace slug::baker
{

struct BakerOptions
{
    FilePath        legacyRoot         = {};
    FilePath        outRoot            = slug::FindAssetRoot() / "Baked";
    RbSet<String>   only               = {};
    int             pageSize           = 2048;
    int             padding            = 1;      // 아틀라스 사각형 사이/가장자리 투명 간격
    int             standaloneSize     = 1024;   // 셀 한 변이 이보다 크면 아틀라스 대신 독립 PNG
    int             alphaThreshold     = 0;      // 트림 시 알파 > 이 값인 픽셀만 내용으로 취급
    int             threads            = 0;      // 0 = 자동
    bool            bKeyMagentaInAlpha = true;
    GroundThreshold ground             = {};
    FilePath        sheetsPath         = {};      // 비어 있으면 <Assets>/Source/sheets.json (있을 때만 사용)
    bool            bNoSheets          = false;   // --no-sheets: 시트 패스 생략
    FilePath        groundEditsPath    = {};      // 비어 있으면 <out>/../Source/GroundEdits.json (있을 때만 사용)
    String          sheetPreview       = {};      // --sheet-preview <file>: 디버그 미리보기만 생성
    int             sheetPreviewScale  = 1;       // 미리보기 배율

    [[nodiscard]] bool Wants(const String& _key) const
    {
        return only.empty() || only.contains(_key);
    }
};

enum class eAnimCategory
{
    PlayerUpper,
    PlayerLower,
    Player,
    Enemy,
    Effect,
    Npc,
    Sheet,   // sheets.json 으로 잘라낸 원본 시트 클립
    Other
};

inline constexpr ARRAY<eAnimCategory, 8> kAllCategories = {
    eAnimCategory::PlayerUpper,
    eAnimCategory::PlayerLower,
    eAnimCategory::Player,
    eAnimCategory::Enemy,
    eAnimCategory::Effect,
    eAnimCategory::Npc,
    eAnimCategory::Sheet,
    eAnimCategory::Other
};

[[nodiscard]] inline const char* CategoryKey(const eAnimCategory _c)
{
    switch (_c)
    {
        case eAnimCategory::PlayerUpper: return "player_upper";
        case eAnimCategory::PlayerLower: return "player_lower";
        case eAnimCategory::Player: return "player";
        case eAnimCategory::Enemy: return "enemy";
        case eAnimCategory::Effect: return "effect";
        case eAnimCategory::Npc: return "npc";
        case eAnimCategory::Sheet: return "sheet";
        case eAnimCategory::Other: return "other";
    }
    return "other";
}

[[nodiscard]] inline const char* CategoryFilterKey(const eAnimCategory _c)
{
    switch (_c)
    {
        case eAnimCategory::PlayerUpper:
        case eAnimCategory::PlayerLower:
        case eAnimCategory::Player: return "player";
        case eAnimCategory::Enemy: return "enemy";
        case eAnimCategory::Effect: return "effect";
        case eAnimCategory::Npc: return "npc";
        case eAnimCategory::Sheet: return "sheet";
        case eAnimCategory::Other: return "other";
    }
    return "other";
}

struct Anomaly
{
    String type      = {};
    String animation = {};
    String detail    = {};
    double value     = 0.0;   // 정렬용 크기 (px 등)
};

struct FrameOut
{
    int  unique       = -1;   // BakeState::uniques 인덱스, 빈 프레임은 -1
    int  w            = 0;
    int  h            = 0;
    int  pivotX       = 0;
    int  pivotY       = 0;
    int  trimX        = 0;
    int  trimY        = 0;
    bool bEmpty       = false;
    bool bOutOfBounds = false;
};

struct AnimOut
{
    AnimRecord       rec        = {};
    eAnimCategory    category   = eAnimCategory::Other;
    bool             bValid     = false;
    String           skipReason = {};
    int              cellW      = 0;
    int              cellH      = 0;
    Vector<FrameOut> frames     = {};
    Vector<Image>    trimmed    = {};
};

struct SpriteOut
{
    SpriteRecord rec          = {};
    bool         bValid       = false;
    bool         bImplicit    = false;
    String       skipReason   = {};
    int          cellW        = 0;
    int          cellH        = 0;
    int          col          = 0;
    int          line         = 0;
    int          texW         = 0;
    int          texH         = 0;
    bool         bFile        = false;
    String       file         = {};
    int          unique       = -1;
    Image        image        = {};
    bool         bCustomPivot = false;   // 시트 스프라이트: 트림된 크기 + 지정 피벗 사용
    int          pivotX       = 0;
    int          pivotY       = 0;
};

struct GroundOut
{
    String      key    = {};
    String      source = {};
    String      file   = {};
    int         w      = 0;
    int         h      = 0;
    bool        bOk    = false;
    String      error  = {};
    GroundStats stats  = {};
};

struct SoundOut
{
    String name  = {};
    String file  = {};
    bool   bBgm  = false;
    bool   bOk   = false;
    String error = {};
};

struct TextureStat
{
    String   name            = {};
    bool     bOk             = false;
    String   error           = {};
    int      w               = 0;
    int      h               = 0;
    bool     bSourceHasAlpha = false;
    bool     bStandalone     = false;
    uint64_t partialAlpha    = 0;   // 0 < a < 255
    uint64_t nonPremult      = 0;   // 부분 알파 중 max(rgb) > a (프리멀티플라이 가정 위반)
    uint64_t hiddenColor     = 0;   // a == 0 인데 rgb != 0
    uint64_t opaqueMagenta   = 0;   // a == 255 이고 (255, 0, 255)
};

struct BakeState
{
    Vector<AnimOut>       anims         = {};
    Vector<SpriteOut>     sprites       = {};
    Vector<Image>         uniques       = {};
    Vector<PackPlacement> placements    = {};
    int                   pageCount     = 0;
    Vector<String>        atlasPages    = {};
    uint64_t              uniquePixels  = 0;
    uint64_t              totalFrames   = 0;
    Vector<GroundOut>     grounds       = {};
    Vector<SoundOut>      sounds        = {};
    Vector<TextureStat>   textures      = {};
    Vector<Anomaly>       anomalies     = {};
    Vector<String>        parseWarnings = {};
    size_t                textureCount  = 0;
    double                elapsedSec    = 0.0;
};

}   // namespace slug::baker

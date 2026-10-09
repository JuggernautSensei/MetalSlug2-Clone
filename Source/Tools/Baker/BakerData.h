#pragma once
#include <cstdint>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "Tools/Baker/Ground.h"
#include "Tools/Baker/Image.h"
#include "Tools/Baker/LegacyParser.h"
#include "Tools/Baker/Packer.h"

namespace slug::baker
{
struct BakerOptions
{
    std::filesystem::path legacyRoot;
    std::filesystem::path outRoot = "Assets/Baked";
    std::set<std::string> only;
    int                   pageSize       = 2048;
    int                   padding        = 1;            // 아틀라스 사각형 사이/가장자리 투명 간격
    int                   standaloneSize = 1024;         // 셀 한 변이 이보다 크면 아틀라스 대신 독립 PNG
    int                   alphaThreshold = 0;            // 트림 시 알파 > 이 값인 픽셀만 내용으로 취급
    int                   threads        = 0;            // 0 = 자동
    bool                  bKeyMagentaInAlpha = true;
    GroundThreshold       ground;

    [[nodiscard]] bool Wants(const std::string& _key) const { return only.empty() || only.contains(_key); }
};

enum class eAnimCategory
{
    PlayerUpper,
    PlayerLower,
    Player,
    Enemy,
    Effect,
    Npc,
    Other
};

inline constexpr eAnimCategory kAllCategories[] = {
    eAnimCategory::PlayerUpper, eAnimCategory::PlayerLower, eAnimCategory::Player, eAnimCategory::Enemy,
    eAnimCategory::Effect, eAnimCategory::Npc, eAnimCategory::Other
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
        case eAnimCategory::Other: return "other";
    }
    return "other";
}

struct Anomaly
{
    std::string type;
    std::string animation;
    std::string detail;
    double      value = 0.0;   // 정렬용 크기 (px 등)
};

struct FrameOut
{
    int  unique          = -1;   // BakeState::uniques 인덱스, 빈 프레임은 -1
    int  w               = 0;
    int  h               = 0;
    int  pivotX          = 0;
    int  pivotY          = 0;
    int  trimX           = 0;
    int  trimY           = 0;
    bool bEmpty          = false;
    bool bOutOfBounds    = false;
};

struct AnimOut
{
    AnimRecord            rec;
    eAnimCategory         category = eAnimCategory::Other;
    bool                  bValid   = false;
    std::string           skipReason;
    int                   cellW = 0;
    int                   cellH = 0;
    std::vector<FrameOut> frames;
    std::vector<Image>    trimmed;
};

struct SpriteOut
{
    SpriteRecord rec;
    bool         bValid    = false;
    bool         bImplicit = false;
    std::string  skipReason;
    int          cellW = 0;
    int          cellH = 0;
    int          col   = 0;
    int          line  = 0;
    int          texW  = 0;
    int          texH  = 0;
    bool         bFile = false;
    std::string  file;
    int          unique = -1;
    Image        image;
};

struct GroundOut
{
    std::string key;
    std::string source;
    std::string file;
    int         w = 0;
    int         h = 0;
    bool        bOk = false;
    std::string error;
    GroundStats stats;
};

struct SoundOut
{
    std::string name;
    std::string file;
    bool        bBgm = false;
    bool        bOk  = false;
    std::string error;
};

struct TextureStat
{
    std::string name;
    bool        bOk             = false;
    std::string error;
    int         w               = 0;
    int         h               = 0;
    bool        bSourceHasAlpha = false;
    bool        bStandalone     = false;
    uint64_t    partialAlpha    = 0;   // 0 < a < 255
    uint64_t    nonPremult      = 0;   // 부분 알파 중 max(rgb) > a (프리멀티플라이 가정 위반)
    uint64_t    hiddenColor     = 0;   // a == 0 인데 rgb != 0
    uint64_t    opaqueMagenta   = 0;   // a == 255 이고 (255, 0, 255)
};

struct BakeState
{
    std::vector<AnimOut>      anims;
    std::vector<SpriteOut>    sprites;
    std::vector<Image>        uniques;
    std::vector<PackPlacement> placements;
    int                       pageCount = 0;
    std::vector<std::string>  atlasPages;
    uint64_t                  uniquePixels = 0;
    uint64_t                  totalFrames = 0;
    std::vector<GroundOut>    grounds;
    std::vector<SoundOut>     sounds;
    std::vector<TextureStat>  textures;
    std::vector<Anomaly>      anomalies;
    std::vector<std::string>  parseWarnings;
    size_t                    textureCount = 0;
    double                    elapsedSec = 0.0;
};
}

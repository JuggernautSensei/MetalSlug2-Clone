#pragma once
#include <cstdint>
#include <optional>

#include "Base/Json.h"
#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/Tuning.h"

inline constexpr int kPropDepthBack  = 12;
inline constexpr int kPropDepthFront = 38;

class AnimationLibrary;
class SpriteLibrary;

enum class ePropRole : uint8_t
{
    Hurt,
    Wall,
    Platform,
    EnemyPlatform
};

enum class ePropWhen : uint8_t
{
    Intact,
    Destroyed,
    Always
};

enum class eVisualRef : uint8_t
{
    None,
    Anim,
    Sprite
};

enum class eDebrisSet : uint8_t
{
    Wood,
    Metal,
    Rock
};

[[nodiscard]] int    GetDebrisClipCount(eDebrisSet _set);
[[nodiscard]] String GetDebrisClipName(eDebrisSet _set, int _index);

struct PropVisual
{
    eVisualRef ref  = eVisualRef::None;
    String     name = {};
};

struct PropPart
{
    ePropRole role   = ePropRole::Hurt;
    ePropWhen when   = ePropWhen::Intact;
    Vec2      size   = { 0.f, 0.f };
    Vec2      offset = { 0.f, 0.f };
};

struct PropExplosion
{
    String clip   = {};
    int    count  = 1;
    float  spread = 0.f;   // 중심 기준 ±spread 난수 오프셋 (px)
    Vec2   offset = { 0.f, 0.f };
};

struct PropDebris
{
    eDebrisSet set   = eDebrisSet::Wood;
    int        count = 0;
};

struct PropBurst
{
    Vector<PropExplosion> explosions = {};
    PropDebris            debris     = {};
};

struct PropDrop
{
    String item    = {};
    int    count   = 1;
    float  spreadX = 0.f;   // x ± spreadX 난수
    bool   bBounce = false;
};

struct PropChild
{
    String def    = {};
    Vec2   offset = { 0.f, 0.f };
};

struct PropFall
{
    float      gravity      = 700.f;
    PropVisual landed       = {};
    float      afterLandSec = 0.5f;
    float      halfHeight   = 0.f;
};

struct PropSecondary
{
    float     afterSec = 2.f;
    PropBurst burst    = {};
};

struct PropGroundPatch
{
    String sprite = {};
    Vec2   size   = { 0.f, 0.f };
    Vec2   inset  = { 0.f, 0.f };
};

struct PropVariant
{
    String           name  = {};
    float            dy    = 0.f;
    PropVisual       idle  = {};
    PropVisual       hit   = {};
    Vector<PropPart> parts = {};
};

struct PropDef
{
    String name        = {};
    int    hp          = 0;   // 0 이면 피격 불가 (불사)
    float  cullMargin  = 200.f;
    float  sortOffsetY = 0.f;              // 같은 레이어 안 그리기 순서 키 = pos.y + sortOffsetY (개별 보정용)
    bool   bSortByX    = false;
    int    depth       = kPropDepthBack;

    PropVisual idle        = {};
    PropVisual damaged     = {};
    PropVisual destroyed   = {};
    PropVisual hit         = {};
    int        damagedAtHp = -1;   // hp <= 이 값이면 damaged 를 쓴다 (-1: 없음)

    Vector<PropPart>    parts    = {};
    Vector<PropVariant> variants = {};

    bool              bRemoveOnDestroy   = false;
    float             destroyDurationSec = 0.5f;
    PropBurst         onDamaged          = {};
    PropBurst         onDestroy          = {};
    bool              bHasSecondary      = false;
    PropSecondary     secondary          = {};
    Vector<PropDrop>  drops              = {};
    Vector<PropChild> spawns             = {};
    bool              bHasFall           = false;
    PropFall          fall               = {};
    bool              bHasGroundPatch    = false;
    PropGroundPatch   groundPatch        = {};
};

struct PropBoxOverride
{
    Vec2 size   = { 0.f, 0.f };
    Vec2 offset = { 0.f, 0.f };
};

struct PropSpawnParams
{
    int                            hp              = -1;   // -1: def 값
    String                         idleSprite      = {};
    String                         destroySprite   = {};
    std::optional<PropBoxOverride> hitbox          = std::nullopt;
    std::optional<PropBoxOverride> standBox        = std::nullopt;
    bool                           bExplodeOnDeath = false;
    int                            variant         = -1;
    int                            z               = kDefaultZ;
};

enum class eItemKind : uint8_t
{
    Gun,
    Supply,
    Score
};

enum class eSupplyKind : uint8_t
{
    Bomb,
    Bullet
};

struct PropItemDef
{
    String      name   = {};
    eItemKind   kind   = eItemKind::Score;
    eWeapon     weapon = eWeapon::Pistol;
    eSupplyKind supply = eSupplyKind::Bomb;
    int         amount = 0;
    int         score  = 0;
    bool        bRoll  = false;
    PropVisual  visual = {};
    PropVisual  begin  = {};
    PropVisual  pickup = {};
};

struct PropItemGroup
{
    String         name  = {};
    Vector<String> names = {};
};

class PropDefLibrary
{
    friend class PropDefParser;

public:
    PropDefLibrary() = default;

    [[nodiscard]] static Result<PropDefLibrary> Load(
        const FilePath& _path,
        String*         _pMessageOrNull = nullptr);
    [[nodiscard]] static Result<PropDefLibrary> Parse(
        StringView _json,
        String*    _pMessageOrNull = nullptr);
    [[nodiscard]] const PropDef*      Find(StringView _name) const;
    [[nodiscard]] int                 FindIndex(StringView _name) const;
    [[nodiscard]] Span<const PropDef> GetDefs() const;

    [[nodiscard]] const PropItemDef*      FindItem(StringView _name) const;
    [[nodiscard]] int                     FindItemIndex(StringView _name) const;
    [[nodiscard]] Span<const PropItemDef> GetItems() const;

    [[nodiscard]] const PropItemGroup* FindGroup(StringView _name) const;

    [[nodiscard]] bool ParseSpawnParams(
        StringView        _defName,
        const JsonReader& _params,
        PropSpawnParams&  _out,
        String&           _error) const;

    [[nodiscard]] String FindMissingAsset(const AnimationLibrary& _anims, const SpriteLibrary& _sprites) const;

private:
    Vector<PropDef>           m_defs       = {};
    Vector<PropItemDef>       m_items      = {};
    Vector<PropItemGroup>     m_groups     = {};
    HashMap<String, uint32_t> m_defIndex   = {};
    HashMap<String, uint32_t> m_itemIndex  = {};
    HashMap<String, uint32_t> m_groupIndex = {};
};

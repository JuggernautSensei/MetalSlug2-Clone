#pragma once
#include <cstdint>
#include <optional>

#include "Base/Json.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Tuning.h"

namespace slug
{

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

[[nodiscard]] int          GetDebrisClipCount(eDebrisSet _set);
[[nodiscard]] slug::String  GetDebrisClipName(eDebrisSet _set, int _index);

struct PropVisual
{
    eVisualRef   ref  = eVisualRef::None;
    slug::String  name = {};
};

struct PropPart
{
    ePropRole    role  = ePropRole::Hurt;
    ePropWhen    when  = ePropWhen::Intact;
    Vec2 size  = { 0.f, 0.f };
    Vec2 offset = { 0.f, 0.f };   // 중심 기준 오프셋
};

struct PropExplosion
{
    slug::String  clip   = {};
    int          count  = 1;
    float        spread = 0.f;            // 중심 기준 ±spread 난수 오프셋 (px)
    Vec2 offset = { 0.f, 0.f };
};

struct PropDebris
{
    eDebrisSet set   = eDebrisSet::Wood;
    int        count = 0;
};

struct PropBurst
{
    slug::Vector<PropExplosion> explosions = {};
    PropDebris                 debris     = {};
};

struct PropDrop
{
    slug::String item    = {};
    int         count   = 1;
    float       spreadX = 0.f;   // x ± spreadX 난수
    bool        bBounce = false;
};

struct PropChild
{
    slug::String  def    = {};
    Vec2 offset = { 0.f, 0.f };
};

struct PropFall
{
    float       gravity       = 700.f;
    PropVisual  landed        = {};
    float       afterLandSec  = 0.5f;   // 착지 후 폭발까지
    float       halfHeight    = 0.f;    // 발(바닥) = 중심 + halfHeight 로 보고 지면과 맞춘다
};

struct PropSecondary
{
    float     afterSec = 2.f;
    PropBurst burst    = {};
};

struct PropGroundPatch
{
    slug::String  sprite = {};
    Vec2 size   = { 0.f, 0.f };   // 스프라이트 크기 (검증용, 로드된 스프라이트와 대조)
    Vec2 inset  = { 0.f, 0.f };   // 좌상단 = pos - (size/2 - inset)
};

struct PropVariant
{
    slug::String           name  = {};
    float                 dy    = 0.f;   // 스폰 y 보정 (레거시 FinalInit)
    PropVisual            idle  = {};
    PropVisual            hit   = {};
    slug::Vector<PropPart> parts = {};
};

struct PropDef
{
    slug::String name = {};
    int         hp   = 0;             // 새 HP (레거시 ×2). 0 이면 피격 불가 (불사)
    float       cullMargin = 200.f;   // 객체 x 가 camLeft - margin 보다 왼쪽이면 제거
    bool        bFront     = false;

    PropVisual idle      = {};
    PropVisual damaged   = {};
    PropVisual destroyed = {};
    PropVisual hit       = {};
    int        damagedAtHp = -1;      // hp <= 이 값이면 damaged 를 쓴다 (-1: 없음)

    slug::Vector<PropPart>    parts    = {};
    slug::Vector<PropVariant> variants = {};

    bool           bRemoveOnDestroy    = false;
    float          destroyDurationSec  = 0.5f;
    PropBurst      onDamaged           = {};
    PropBurst      onDestroy           = {};
    bool           bHasSecondary       = false;
    PropSecondary  secondary           = {};
    slug::Vector<PropDrop>  drops       = {};
    slug::Vector<PropChild> spawns      = {};
    bool           bHasFall            = false;
    PropFall       fall                = {};
    bool           bHasGroundPatch     = false;
    PropGroundPatch groundPatch        = {};
};

struct PropBoxOverride
{
    Vec2 size   = { 0.f, 0.f };
    Vec2 offset = { 0.f, 0.f };
};

struct PropSpawnParams
{
    int                          hp            = -1;     // -1: def 값
    slug::String                  idleSprite    = {};
    slug::String                  destroySprite = {};
    std::optional<PropBoxOverride> hitbox      = std::nullopt;
    std::optional<PropBoxOverride> standBox    = std::nullopt;
    bool                         bExplodeOnDeath = false;
    int                          variant         = -1;      // Truck 변종 강제 (-1 랜덤)
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
    slug::String name     = {};
    eItemKind   kind     = eItemKind::Score;
    eWeapon     weapon   = eWeapon::Pistol;
    eSupplyKind supply   = eSupplyKind::Bomb;
    int         amount   = 0;                 // Supply: 폭탄 수 (BULLET 은 무시)
    int         score    = 0;                 // Score 전용
    bool        bRoll    = false;
    PropVisual  visual   = {};
    PropVisual  begin    = {};
    PropVisual  pickup   = {};
};

struct PropItemGroup
{
    slug::String              name  = {};
    slug::Vector<slug::String> names = {};
};

class PropDefLibrary
{
public:
    [[nodiscard]] static slug::Result<PropDefLibrary> Load(const slug::FilePath& _path, slug::String* _pMessage = nullptr);
    [[nodiscard]] static slug::Result<PropDefLibrary> Parse(slug::StringView _json, slug::String* _pMessage = nullptr);
    PropDefLibrary() = default;

    [[nodiscard]] const PropDef* Find(slug::StringView _name) const;
    [[nodiscard]] int            FindIndex(slug::StringView _name) const;
    [[nodiscard]] slug::Span<const PropDef> GetDefs() const { return m_defs; }

    [[nodiscard]] const PropItemDef* FindItem(slug::StringView _name) const;
    [[nodiscard]] int                FindItemIndex(slug::StringView _name) const;
    [[nodiscard]] slug::Span<const PropItemDef> GetItems() const { return m_items; }

    [[nodiscard]] const PropItemGroup* FindGroup(slug::StringView _name) const;

    [[nodiscard]] bool ParseSpawnParams(slug::StringView _defName, const slug::JsonReader& _params, PropSpawnParams& _out, slug::String& _error) const;

    [[nodiscard]] slug::String FindMissingAsset(const AnimationLibrary& _anims, const SpriteLibrary& _sprites) const;

private:
    friend class PropDefParser;

    slug::Vector<PropDef>                                         m_defs       = {};
    slug::Vector<PropItemDef>                                     m_items      = {};
    slug::Vector<PropItemGroup>                                   m_groups     = {};
    slug::HashMap<slug::String, uint32_t>                          m_defIndex   = {};
    slug::HashMap<slug::String, uint32_t>                          m_itemIndex  = {};
    slug::HashMap<slug::String, uint32_t>                          m_groupIndex = {};
};
}

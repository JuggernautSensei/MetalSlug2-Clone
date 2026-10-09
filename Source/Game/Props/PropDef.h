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

// ===========================================
//  PropDef / PropDefLibrary
//   Assets/Defs/Props.json 을 읽어 보관하는 소품(파괴 가능한 장면 물체)과 아이템 정의.
//   스키마는 Docs/PropSpecs.md 가 정본이다. 모든 좌표는 "스프라이트 중심(legacyCenter)" 기준이다.
//   로드 후에는 읽기 전용이며, PropWorld 는 스폰 시점에 정의를 값으로 복사한다.
// ===========================================

enum class ePropRole : uint8_t
{
    Hurt,           // 피격 상자 (Neutral Hurtbox)
    Wall,           // 고체 (플레이어/적 모두 막힘)
    Platform,       // 원웨이 발판 (플레이어/적 모두 설 수 있음)
    EnemyPlatform   // 원웨이 발판 (적만. 플레이어는 통과)
};

// 이 파츠가 살아 있는 조건 (Intact: 파괴 전, Destroyed: 파괴 후에만, Always: 항상)
enum class ePropWhen : uint8_t
{
    Intact,
    Destroyed,
    Always
};

enum class eVisualRef : uint8_t
{
    None,
    Anim,      // 애니메이션 클립 이름 (anim.json)
    Sprite     // 정적 스프라이트 이름 (sprites.json)
};

enum class eDebrisSet : uint8_t
{
    Wood,      // Wood1..Wood6
    Metal,     // Metal1..Metal12
    Rock       // Rock1..Rock3
};

// 디브리 클립 이름 (Wood1..6 / Metal1..12 / Rock1..3)
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
    slug::String  clip   = {};   // 예: Explosion_Small
    int          count  = 1;
    float        spread = 0.f;            // 중심 기준 ±spread 난수 오프셋 (px)
    Vec2 offset = { 0.f, 0.f };   // 중심 기준 고정 오프셋 (레거시 CreateExplosionEffect(pos, offset, ...))
};

struct PropDebris
{
    eDebrisSet set   = eDebrisSet::Wood;
    int        count = 0;
};

// 한 번의 "파괴 연출" 묶음
struct PropBurst
{
    slug::Vector<PropExplosion> explosions = {};
    PropDebris                 debris     = {};
};

struct PropDrop
{
    slug::String item    = {};   // 아이템 id 또는 random 그룹 이름
    int         count   = 1;
    float       spreadX = 0.f;   // x ± spreadX 난수
    bool        bBounce = false;
};

struct PropChild
{
    slug::String  def    = {};    // 같은 Props.json 의 다른 정의 (예: sign_fall)
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
    float     afterSec = 2.f;   // 파괴 후 2차 폭발까지 (explodeOnDeath 인스턴스에서만)
    PropBurst burst    = {};
};

struct PropGroundPatch
{
    slug::String  sprite = {};   // *_PixelMap 스프라이트
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
    bool        bFront     = false;   // 렌더 레이어: false = background, true = front

    PropVisual idle      = {};
    PropVisual damaged   = {};
    PropVisual destroyed = {};
    PropVisual hit       = {};        // 피격 시 한 번 재생 (Truck_Hit 등)
    int        damagedAtHp = -1;      // hp <= 이 값이면 damaged 를 쓴다 (-1: 없음)

    slug::Vector<PropPart>    parts    = {};
    slug::Vector<PropVariant> variants = {};   // 비어 있으면 idle/hit/parts 를 그대로 쓴다. 있으면 스폰 시 랜덤 선택

    bool           bRemoveOnDestroy    = false;   // true: 파괴 즉시(연출 종료 시) 제거, 잔해 스프라이트 없음
    float          destroyDurationSec  = 0.5f;    // 파괴 후 IsDestroyFinished 까지 (destroyed 애니가 있으면 그 길이와 max)
    PropBurst      onDamaged           = {};      // 처음 damaged 상태로 들어갈 때 1회 (목재 부스러기 등)
    PropBurst      onDestroy           = {};
    bool           bHasSecondary       = false;
    PropSecondary  secondary           = {};
    slug::Vector<PropDrop>  drops       = {};
    slug::Vector<PropChild> spawns      = {};      // 파괴 시 생성 (간판 등)
    bool           bHasFall            = false;   // 낙하체: hp 없음. 스폰 후 중력 낙하 → 착지 → fall.landed → afterLandSec 뒤 onDestroy 후 제거
    PropFall       fall                = {};
    bool           bHasGroundPatch     = false;
    PropGroundPatch groundPatch        = {};
};

// 스폰 시 인스턴스별 덮어쓰기 (stage JSON 의 params). 알 수 없는 키는 파서가 거부한다.
struct PropBoxOverride
{
    Vec2 size   = { 0.f, 0.f };
    Vec2 offset = { 0.f, 0.f };
};

struct PropSpawnParams
{
    int                          hp            = -1;     // -1: def 값
    slug::String                  idleSprite    = {};     // sprites[0]
    slug::String                  destroySprite = {};     // sprites[1]
    std::optional<PropBoxOverride> hitbox      = std::nullopt;     // 첫 Hurt 파츠 덮어쓰기
    std::optional<PropBoxOverride> standBox    = std::nullopt;     // 첫 Platform 파츠 덮어쓰기
    bool                         bExplodeOnDeath = false;   // mob_boat: 2차 폭발 + 제거
    int                          variant         = -1;      // Truck 변종 강제 (-1 랜덤)
};

// ===========================================
//  아이템 정의
// ===========================================

enum class eItemKind : uint8_t
{
    Gun,      // 무기 교체/보충 (GunItem)
    Supply,   // 폭탄 +N / 탄약 +extra (SuppleItem)
    Score     // 점수 (FruitScoreItem / AnimeScoreItem)
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
    eWeapon     weapon   = eWeapon::Pistol;   // Gun 전용
    eSupplyKind supply   = eSupplyKind::Bomb; // Supply 전용
    int         amount   = 0;                 // Supply: 폭탄 수 (BULLET 은 무시)
    int         score    = 0;                 // Score 전용
    bool        bRoll    = false;             // Score(과일): 수평으로 굴러가다 감속
    PropVisual  visual   = {};                // 바닥 상태
    PropVisual  begin    = {};                // 첫 재생(Item_Mando_Begin) 후 visual 로 루프
    PropVisual  pickup   = {};                // 습득 연출 (Disappear 등)
};

struct PropItemGroup
{
    slug::String              name  = {};
    slug::Vector<slug::String> names = {};   // 균등 추첨 대상 아이템 이름
};

// ===========================================
//  PropDefLibrary
// ===========================================

class PropDefLibrary
{
public:
    [[nodiscard]] static slug::Result<PropDefLibrary> Load(const slug::FilePath& _path, slug::String* _pMessage = nullptr);
    [[nodiscard]] static slug::Result<PropDefLibrary> Parse(slug::StringView _json, slug::String* _pMessage = nullptr);
    PropDefLibrary() = default;

    [[nodiscard]] const PropDef* Find(slug::StringView _name) const;
    [[nodiscard]] int            FindIndex(slug::StringView _name) const;   // 없으면 -1
    [[nodiscard]] slug::Span<const PropDef> GetDefs() const { return m_defs; }

    [[nodiscard]] const PropItemDef* FindItem(slug::StringView _name) const;
    [[nodiscard]] int                FindItemIndex(slug::StringView _name) const;
    [[nodiscard]] slug::Span<const PropItemDef> GetItems() const { return m_items; }

    // random_fruit / random_anime / random_score 같은 그룹. 없으면 null.
    [[nodiscard]] const PropItemGroup* FindGroup(slug::StringView _name) const;

    // 스테이지 spawn.params(JSON 객체)를 def 기준으로 해석한다. 알 수 없는 키/형식 오류는 false + _error.
    [[nodiscard]] bool ParseSpawnParams(slug::StringView _defName, const slug::JsonReader& _params, PropSpawnParams& _out, slug::String& _error) const;

    // 정의가 참조하는 클립/스프라이트 이름이 모두 존재하는가. 없으면 첫 누락 이름을 "def: 이름" 형식으로 돌려준다 (정상이면 빈 문자열).
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

}   // namespace slug

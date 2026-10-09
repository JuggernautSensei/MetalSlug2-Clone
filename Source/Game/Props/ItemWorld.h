#pragma once
#include <cstdint>

#include "Base/Random.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Props/PropDef.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Geometry.h"
#include "World/Physics.h"

namespace slug
{

class PlayerController;
class BitmapFont;

inline constexpr uint32_t kItemIdBase        = 7000;
inline constexpr float    kItemLifetimeSec   = 15.f;
inline constexpr float    kItemBlinkStartSec = 10.5f;
inline constexpr float    kItemBlinkPeriod   = 0.2f;
inline constexpr float    kItemGravity       = 1400.f;   // px/s²
inline constexpr float    kItemPickupHideSec = 0.06f;
inline constexpr float    kScorePopupBlinkSec = 0.07f;
inline constexpr float    kScorePopupRiseDelaySec = 1.05f;
inline constexpr float    kScorePopupLifeSec = 2.8f;

struct ItemPickup
{
    uint32_t     id     = 0;
    slug::String  name   = {};
    eItemKind    kind   = eItemKind::Score;
    eWeapon      weapon = eWeapon::Pistol;
    eSupplyKind  supply = eSupplyKind::Bomb;
    int          amount = 0;   // 폭탄 개수 (Bomb)
    int          score  = 0;
    Vec2 pos    = { 0.f, 0.f };
};

struct ItemFxRequest
{
    slug::String  clip = {};
    Vec2 pos  = { 0.f, 0.f };
};

struct PropItem
{
    uint32_t     id        = 0;
    int          defIndex  = -1;          // PropDefLibrary::GetItems() 인덱스
    Vec2 feet      = { 0.f, 0.f };
    Body         body      = {};
    float        ageSec    = 0.f;
    bool         bBounce   = false;
    float        rollVx    = 0.f;
    bool         bBegun    = false;
    bool         bPickedUp = false;
    float        pickupSec = 0.f;         // 습득 후 남은 연출 시간 (bPickedUp 일 때만 의미)
    AnimPlayer   anim      = {};
    AnimPlayer   beginAnim = {};
    bool         bAnimBound = false;
};

struct ScorePopup
{
    Vec2 pos     = { 0.f, 0.f };
    int          score   = 0;
    float        ageSec  = 0.f;
};

class ItemWorld
{
public:
    explicit ItemWorld(const PropDefLibrary* _pLibrary = nullptr, uint64_t _seed = 1);

    void SetLibrary(const PropDefLibrary* _pLibrary) { m_pLibrary = _pLibrary; }
    void Reset(uint64_t _seed = 1);

    uint32_t Spawn(slug::StringView _name, Vec2 _feet, bool _bBounce = false);

    void Tick(const PhysicsWorld& _world, float _dt, float _cameraLeft, const AnimationLibrary* _pAnims);

    [[nodiscard]] slug::Vector<ItemPickup> Collect(const Aabb& _playerBox);

    [[nodiscard]] slug::Vector<ItemFxRequest> TakeFx();

    void Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont) const;

    void ClearAll();

    [[nodiscard]] slug::Span<const PropItem>    GetItems() const { return m_items; }
    [[nodiscard]] slug::Span<const ScorePopup>  GetPopups() const { return m_popups; }
    [[nodiscard]] const PropItem*              Find(uint32_t _id) const;
    [[nodiscard]] size_t                       GetCount() const { return m_items.size(); }
    [[nodiscard]] uint64_t                     HashState() const;

private:
    void SpawnPopup_(Vec2 _pos, int _score);
    void BindAnim_(PropItem& _item, const AnimationLibrary& _anims) const;

    const PropDefLibrary*          m_pLibrary = nullptr;
    slug::Vector<PropItem>          m_items    = {};
    slug::Vector<ScorePopup>        m_popups   = {};
    slug::Vector<ItemFxRequest>     m_fx       = {};
    slug::RngMt19937                m_rng      = slug::RngMt19937 { uint64_t { 1 } };
    uint32_t                       m_nextId   = kItemIdBase;
};

void ApplyItemPickup(PlayerController& _player, const ItemPickup& _pickup, const Tuning& _tuning);
}

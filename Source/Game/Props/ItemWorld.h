#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Props/PropDef.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Geometry.h"
#include "World/Physics.h"

class PlayerController;
class BitmapFont;

inline constexpr int      kItemRenderDepth        = 34;
inline constexpr int      kPopupRenderDepth       = 46;
inline constexpr uint32_t kItemIdBase             = 7000;
inline constexpr float    kItemLifetimeSec        = 15.f;
inline constexpr float    kItemBlinkStartSec      = 10.5f;
inline constexpr float    kItemBlinkPeriod        = 0.2f;
inline constexpr float    kItemGravity            = 1400.f;
inline constexpr float    kItemPickupHideSec      = 0.06f;
inline constexpr float    kScorePopupBlinkSec     = 0.07f;
inline constexpr float    kScorePopupRiseDelaySec = 1.05f;
inline constexpr float    kScorePopupLifeSec      = 2.8f;

struct ItemPickup
{
    uint32_t    id     = 0;
    String      name   = {};
    eItemKind   kind   = eItemKind::Score;
    eWeapon     weapon = eWeapon::Pistol;
    eSupplyKind supply = eSupplyKind::Bomb;
    int         amount = 0;
    int         score  = 0;
    VECTOR2     pos    = { 0.f, 0.f };
};

struct ItemFxRequest
{
    String  clip    = {};
    VECTOR2 pos     = { 0.f, 0.f };
    int     zOffset = 0;
};

struct PropItem
{
    uint32_t   id         = 0;
    int        defIndex   = -1;
    VECTOR2    feet       = { 0.f, 0.f };
    int        z          = kDefaultZ;
    Body       body       = {};
    float      ageSec     = 0.f;
    bool       bBounce    = false;
    float      rollVx     = 0.f;
    float      spinScale  = 1.f;   // 회전 애니 배속
    bool       bBegun     = false;
    bool       bPickedUp  = false;
    float      pickupSec  = 0.f;
    AnimPlayer anim       = {};
    AnimPlayer beginAnim  = {};
    bool       bAnimBound = false;
};

struct ScorePopup
{
    VECTOR2 pos    = { 0.f, 0.f };
    int     score  = 0;
    float   ageSec = 0.f;
    int     z      = kDefaultZ;
};

class ItemWorld
{
public:
    explicit ItemWorld(const PropDefLibrary* _pLibrary, uint64_t _seed = 1);
    void     SetLibrary(const PropDefLibrary* _pLibrary);
    void     Reset(uint64_t _seed = 1);
    uint32_t Spawn(StringView _name, VECTOR2 _feet, bool _bBounce = false, int _z = kDefaultZ);
    bool     SetZ(uint32_t _id, int _z);

    // _pTuning 이 null 이면 ItemTuning 기본값을 쓴다
    void                                 Tick(const PhysicsWorld& _world, float _dt, float _cameraLeft, const AnimationLibrary* _pAnims, const ItemTuning* _pTuning = nullptr);
    [[nodiscard]] Vector<ItemPickup>     Collect(const Aabb& _playerBox);
    [[nodiscard]] Vector<ItemFxRequest>  TakeFx();
    void                                 Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont) const;
    void                                 ClearAll();
    [[nodiscard]] Span<const PropItem>   GetItems() const;
    [[nodiscard]] Span<const ScorePopup> GetPopups() const;
    [[nodiscard]] const PropItem*        Find(uint32_t _id) const;
    [[nodiscard]] size_t                 GetCount() const;
    [[nodiscard]] uint64_t               HashState() const;

private:
    void SpawnPopup_(VECTOR2 _pos, int _score, int _z);
    void BindAnim_(PropItem& _item, const AnimationLibrary& _anims) const;

    const PropDefLibrary* m_pLibrary = nullptr;
    Vector<PropItem>      m_items    = {};
    Vector<ScorePopup>    m_popups   = {};
    Vector<ItemFxRequest> m_fx       = {};
    RngMt19937            m_rng      = RngMt19937 { uint64_t { 1 } };
    uint32_t              m_nextId   = kItemIdBase;
};

void ApplyItemPickup(PlayerController& _player, const ItemPickup& _pickup, const Tuning& _tuning);

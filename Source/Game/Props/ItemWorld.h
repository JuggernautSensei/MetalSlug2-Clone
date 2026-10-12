#pragma once
#include <cstdint>

#include "Base/Math.h"
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
inline constexpr float    kItemGravity            = 1400.f;   // px/s²
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
    int         amount = 0;   // 폭탄 개수 (Bomb)
    int         score  = 0;
    Vec2        pos    = { 0.f, 0.f };
};

struct ItemFxRequest
{
    String clip = {};
    Vec2   pos  = { 0.f, 0.f };
    int    zOffset = 0;   // 아이템 z 의 기본 depth 대비 이동량
};

struct PropItem
{
    uint32_t   id         = 0;
    int        defIndex   = -1;   // PropDefLibrary::GetItems() 인덱스
    Vec2       feet       = { 0.f, 0.f };
    int        z          = kDefaultZ;   // 그리기 depth (kDefaultZ: kItemRenderDepth)
    Body       body       = {};
    float      ageSec     = 0.f;
    bool       bBounce    = false;
    float      rollVx     = 0.f;
    float      spinScale  = 1.f;   // 회전 애니 배속 (0 이면 정지). 굴러가는 점수 아이템만 바뀐다
    bool       bBegun     = false;
    bool       bPickedUp  = false;
    float      pickupSec  = 0.f;   // 습득 후 남은 연출 시간 (bPickedUp 일 때만 의미)
    AnimPlayer anim       = {};
    AnimPlayer beginAnim  = {};
    bool       bAnimBound = false;
};

struct ScorePopup
{
    Vec2  pos    = { 0.f, 0.f };
    int   score  = 0;
    float ageSec = 0.f;
    int   z      = kDefaultZ;   // 그리기 depth (kDefaultZ: kPopupRenderDepth)
};

class ItemWorld
{
public:
    explicit ItemWorld(
        const PropDefLibrary* _pLibrary = nullptr,
        uint64_t              _seed     = 1);

    void SetLibrary(const PropDefLibrary* _pLibrary);
    void Reset(uint64_t _seed = 1);

    uint32_t Spawn(
        StringView _name,
        Vec2       _feet,
        bool       _bBounce = false,
        int        _z       = kDefaultZ);

    bool SetZ(uint32_t _id, int _z);   // 아이템 depth 를 즉시 바꾼다 (팝업은 따라 옮겨지지 않는다)

    // _pTuning 이 null 이면 ItemTuning 기본값을 쓴다 (테스트 호환)
    void Tick(
        const PhysicsWorld&     _world,
        float                   _dt,
        float                   _cameraLeft,
        const AnimationLibrary* _pAnims,
        const ItemTuning*       _pTuning = nullptr);

    [[nodiscard]] Vector<ItemPickup> Collect(const Aabb& _playerBox);

    [[nodiscard]] Vector<ItemFxRequest> TakeFx();

    void Submit(
        SpriteBatch&            _sprites,
        const AnimationLibrary& _anims,
        const SpriteLibrary&    _spriteLib,
        const BitmapFont*       _pFont) const;

    void ClearAll();

    [[nodiscard]] Span<const PropItem>   GetItems() const;
    [[nodiscard]] Span<const ScorePopup> GetPopups() const;
    [[nodiscard]] const PropItem*        Find(uint32_t _id) const;
    [[nodiscard]] size_t                 GetCount() const;
    [[nodiscard]] uint64_t               HashState() const;

private:
    void SpawnPopup_(Vec2 _pos, int _score, int _z);
    void BindAnim_(PropItem& _item, const AnimationLibrary& _anims) const;

    const PropDefLibrary* m_pLibrary = nullptr;
    Vector<PropItem>      m_items    = {};
    Vector<ScorePopup>    m_popups   = {};
    Vector<ItemFxRequest> m_fx       = {};
    RngMt19937            m_rng      = RngMt19937 { uint64_t { 1 } };
    uint32_t              m_nextId   = kItemIdBase;
};

void ApplyItemPickup(PlayerController& _player, const ItemPickup& _pickup, const Tuning& _tuning);

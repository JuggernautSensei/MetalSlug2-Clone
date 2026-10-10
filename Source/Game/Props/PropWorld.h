#pragma once
#include <array>
#include <cstdint>

#include "Base/Random.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Combat/CombatSystem.h"
#include "Game/Props/DecorWorld.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropDef.h"
#include "Game/Sandbox/Effects.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Combat.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{

class BitmapFont;

inline constexpr uint32_t kPropOwnerBase = 5000;
using PropHandle = uint32_t;
inline constexpr PropHandle kNoProp = 0;

inline constexpr int kPropLayerBack  = 12;   // 소품 (background)
inline constexpr int kPropLayerFront = 38;   // 소품 (front)
inline constexpr int kPropDebrisLayer = 39;  // 디브리
inline constexpr int kPropEffectLayer = 50;  // 폭발 연출 (EffectList 기본과 같음)

enum class ePropEventKind : uint8_t
{
    Damaged,
    Destroyed,
    DropSpawned,
    GroundPatch,
    SecondaryBurst,
    Removed,
    Culled
};

struct PropEvent
{
    ePropEventKind kind     = ePropEventKind::Damaged;
    PropHandle     id       = kNoProp;
    int            defIndex = -1;
    Vec2   pos      = { 0.f, 0.f };
    int            value    = 0;
    slug::String    name     = {};
};

enum class ePropVisual : uint8_t
{
    Idle,
    Damaged,
    Destroyed,
    Hit,
    Landed,
    Count
};

struct PropInstance
{
    PropHandle                 id                    = kNoProp;
    int                        defIndex              = -1;
    int                        variant               = -1;
    Vec2               pos                   = { 0.f, 0.f };   // 스프라이트 중심 (legacyCenter, 변종 dy 반영)
    bool                       bFront                = false;
    float                      cullMargin            = 200.f;
    Health                     health                = {};
    std::array<PropVisual, static_cast<size_t>(ePropVisual::Count)> visuals = {};
    slug::Vector<PropPart>      parts                 = {};

    ePropVisual                visual                = ePropVisual::Idle;
    bool                       bVisualBound          = false;
    AnimPlayer                 anim                  = {};
    bool                       bHitPlaying           = false;
    float                      hitTimerSec           = 0.f;
    bool                       bDamagedBurstDone     = false;

    bool                       bDestroyed            = false;
    float                      destroyTimerSec       = 0.f;
    float                      destroyDurationSec    = 0.f;
    bool                       bRemoveAfterDestroy   = false;
    bool                       bExplodeOnDeath       = false;
    float                      secondaryTimerSec     = -1.f;   // >= 0: 2차 폭발 대기
    bool                       bRemoveAfterSecondary = false;

    bool                       bFalling              = false;
    float                      fallVy                = 0.f;
    float                      fallGravity           = 700.f;
    float                      fallHalfHeight        = 0.f;
    float                      landTimerSec          = -1.f;    // >= 0: 착지 후 경과 (afterLandSec 뒤 폭발)

    bool                       bRemove               = false;
};

struct PropFxRequest
{
    slug::String  clip  = {};
    Vec2 pos   = { 0.f, 0.f };
    int          layer = kPropEffectLayer;
};

struct PropDebrisPiece
{
    Vec2 pos    = { 0.f, 0.f };
    Vec2 vel    = { 0.f, 0.f };
    float        ageSec = 0.f;
    slug::String  clip   = {};
    AnimPlayer   anim   = {};
    bool         bBound = false;
};

struct PropTickContext
{
    const GroundMap*        pGround = nullptr;
    const AnimationLibrary* pAnims  = nullptr;
    float                   cameraLeft = -1.0e9f;   // 이 x 에서 cullMargin 만큼 더 왼쪽 객체는 제거
    float                   dt         = kStepSec;
};

class PropWorld
{
public:
    explicit PropWorld(const PropDefLibrary* _pLibrary = nullptr, uint64_t _seed = 1);

    void SetLibrary(const PropDefLibrary* _pLibrary);
    [[nodiscard]] const PropDefLibrary* GetLibrary() const { return m_pLibrary; }

    void Reset(uint64_t _seed = 1);

    PropHandle Spawn(slug::StringView _defName, Vec2 _center, const PropSpawnParams& _params = {});
    PropHandle SpawnByIndex(int _defIndex, Vec2 _center, const PropSpawnParams& _params = {});

    uint32_t SpawnItem(slug::StringView _name, Vec2 _feet, bool _bBounce = false);

    void Tick(const PropTickContext& _ctx);
    void RegisterHurtboxes(CombatSystem& _combat) const;
    void OnHitEvents(slug::Span<const HitEvent> _events);

    [[nodiscard]] slug::Span<const Platform> GetPlatforms() const      { return m_platforms; }
    [[nodiscard]] slug::Span<const Platform> GetEnemyPlatforms() const { return m_enemyPlatforms; }
    [[nodiscard]] slug::Span<const Wall>     GetWalls() const          { return m_walls; }

    [[nodiscard]] bool IsDestroyed(PropHandle _handle) const;
    [[nodiscard]] bool IsDestroyFinished(PropHandle _handle) const;
    [[nodiscard]] bool IsAlive(PropHandle _handle) const;
    [[nodiscard]] int  GetHp(PropHandle _handle) const;
    [[nodiscard]] const PropInstance* Get(PropHandle _handle) const;
    [[nodiscard]] slug::Span<const PropInstance> GetProps() const { return m_props; }
    [[nodiscard]] size_t GetCount() const { return m_props.size(); }
    [[nodiscard]] slug::Span<const PropDebrisPiece>  GetDebris() const { return m_debris; }

    [[nodiscard]] slug::Vector<PropEvent> TakeEvents();

    [[nodiscard]] ItemWorld&       GetItems()       { return m_items; }
    [[nodiscard]] const ItemWorld& GetItems() const { return m_items; }

    // 시각 전용 장식 (anim_decor). 판정/피격 없음.
    uint32_t SpawnDecor(const DecorSpawn& _spawn) { return m_decor.Spawn(_spawn); }
    void     ClearDecorByLayerMask(uint32_t _layerMask) { m_decor.ClearByLayerMask(_layerMask); }
    [[nodiscard]] const DecorWorld& GetDecor() const { return m_decor; }

    void ClearByLayer(bool _bFront);
    void ClearAll();

    void Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont = nullptr);

    [[nodiscard]] uint64_t HashState() const;

private:
    struct PendingChild
    {
        int          defIndex = -1;
        Vec2 pos      = { 0.f, 0.f };
    };

    [[nodiscard]] size_t FindIndex_(PropHandle _id) const;
    [[nodiscard]] float  RollFloat_(float _lo, float _hi);
    [[nodiscard]] int    RollInt_(int _n);

    void Destroy_(PropInstance& _inst, const PropDef& _def, Vec2 _center);
    void Detonate_(const PropBurst& _burst, Vec2 _center);
    void QueueDrops_(const PropDef& _def, Vec2 _center);
    void StartHit_(PropInstance& _inst, const PropDef& _def);
    void UpdateVisual_(PropInstance& _inst, const AnimationLibrary* _pAnims, float _dt);
    [[nodiscard]] ePropVisual ComputeVisual_(const PropInstance& _inst) const;
    void RebuildCollision_();
    [[nodiscard]] static bool PartActive_(const PropPart& _part, const PropInstance& _inst);

    const PropDefLibrary* m_pLibrary = nullptr;
    slug::Vector<PropInstance>      m_props          = {};
    slug::Vector<PropEvent>         m_events         = {};
    slug::Vector<PropFxRequest>     m_pendingFx      = {};
    slug::Vector<PropDebrisPiece>   m_debris         = {};
    slug::Vector<PendingChild>      m_pendingChildren = {};
    slug::Vector<Platform>          m_platforms      = {};
    slug::Vector<Platform>          m_enemyPlatforms = {};
    slug::Vector<Wall>              m_walls          = {};
    ItemWorld                      m_items          { nullptr, uint64_t { 1 } };
    DecorWorld                     m_decor          = {};
    EffectList                     m_effects        = {};
    slug::RngMt19937                m_rng            = slug::RngMt19937 { uint64_t { 1 } };
    uint32_t                       m_nextId         = kPropOwnerBase;
};

// 지면 패치: 발자국 마스크의 고체 픽셀만 공기로 만든다 (Docs/PropSpecs.md §2 groundPatch). 가드에 걸리면 변경 없이 false.
bool ApplyGroundPatch(GroundMap& _map, Vec2 _topLeft, const GroundMap& _footprint);
}

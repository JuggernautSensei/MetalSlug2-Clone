#pragma once
#include <cstdint>

#include "Base/Types.h"
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

class BitmapFont;

inline constexpr uint32_t kPropOwnerBase = 5000;
using PropHandle                         = uint32_t;
inline constexpr PropHandle kNoProp      = 0;

inline constexpr int kPropDebrisDepth = 39;
inline constexpr int kPropEffectDepth = 50;

inline constexpr int kBuildingDepthXScale = 4096;

enum class ePropEventKind
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
    VECTOR2        pos      = { 0.f, 0.f };
    int            value    = 0;
    String         name     = {};
};

enum class ePropVisual
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
    PropHandle                                 id          = kNoProp;
    int                                        defIndex    = -1;
    int                                        variant     = -1;
    VECTOR2                                    pos         = { 0.f, 0.f };
    int                                        depth       = kPropDepthBack;
    float                                      cullMargin  = 200.f;
    float                                      sortOffsetY = 0.f;
    bool                                       bSortByX    = false;
    Health                                     health      = {};
    DIRECT_ENUM_ARRAY<ePropVisual, PropVisual> visuals     = {};
    Vector<PropPart>                           parts       = {};

    ePropVisual visual            = ePropVisual::Idle;
    bool        bVisualBound      = false;
    AnimPlayer  anim              = {};
    bool        bHitPlaying       = false;
    float       hitTimerSec       = 0.f;
    bool        bDamagedBurstDone = false;

    bool  bDestroyed            = false;
    float destroyTimerSec       = 0.f;
    float destroyDurationSec    = 0.f;
    bool  bRemoveAfterDestroy   = false;
    bool  bExplodeOnDeath       = false;
    float secondaryTimerSec     = -1.f;
    bool  bRemoveAfterSecondary = false;

    bool  bFalling       = false;
    float fallVy         = 0.f;
    float fallGravity    = 700.f;
    float fallHalfHeight = 0.f;
    float landTimerSec   = -1.f;

    bool bRemove = false;
};

struct PropFxRequest
{
    String  clip  = {};
    VECTOR2 pos   = { 0.f, 0.f };
    int     depth = kPropEffectDepth;
};

struct PropDebrisPiece
{
    VECTOR2    pos    = { 0.f, 0.f };
    VECTOR2    vel    = { 0.f, 0.f };
    float      ageSec = 0.f;
    String     clip   = {};
    AnimPlayer anim   = {};
    bool       bBound = false;
};

struct PropTickContext
{
    const GroundMap*        pGround      = nullptr;
    const AnimationLibrary* pAnims       = nullptr;
    float                   cameraLeft   = -1e9f;
    float                   cameraBottom = 1e9f;   // 카메라 하단 y
    float                   dt           = kStepSec;
    const ItemTuning*       pItemTuning  = nullptr;   // null 이면 기본값
};

class PropWorld
{
    struct PendingChild
    {
        int     defIndex = -1;
        VECTOR2 pos      = { 0.f, 0.f };
    };

public:
    explicit PropWorld(const PropDefLibrary* _pLibrary, uint64_t _seed = 1);
    void                                SetLibrary(const PropDefLibrary* _pLibrary);
    [[nodiscard]] const PropDefLibrary* GetLibrary() const;
    void                                Reset(uint64_t _seed = 1);
    PropHandle                          Spawn(StringView _defName, VECTOR2 _center, const PropSpawnParams& _params = {});
    PropHandle                          SpawnByIndex(int _defIndex, VECTOR2 _center, const PropSpawnParams& _params = {});
    uint32_t                            SpawnItem(StringView _name, VECTOR2 _feet, bool _bBounce = false, int _z = kDefaultZ);
    void                                Tick(const PropTickContext& _ctx);

    // 스프라이트 크기 조회용
    void                                      SetSpriteLibrary(const SpriteLibrary* _pSprites);
    [[nodiscard]] bool                        GetVisualBounds(const PropInstance& _inst, const AnimationLibrary* _pAnims, Aabb& _out) const;
    void                                      RegisterHurtboxes(CombatSystem& _combat) const;
    void                                      OnHitEvents(Span<const HitEvent> _events);
    [[nodiscard]] Span<const Platform>        GetPlatforms() const;
    [[nodiscard]] Span<const Platform>        GetEnemyPlatforms() const;
    [[nodiscard]] Span<const Wall>            GetWalls() const;
    [[nodiscard]] bool                        IsDestroyed(PropHandle _handle) const;
    [[nodiscard]] bool                        IsDestroyFinished(PropHandle _handle) const;
    [[nodiscard]] bool                        IsAlive(PropHandle _handle) const;
    [[nodiscard]] int                         GetHp(PropHandle _handle) const;
    [[nodiscard]] const PropInstance*         Get(PropHandle _handle) const;
    [[nodiscard]] Span<const PropInstance>    GetProps() const;
    [[nodiscard]] size_t                      GetCount() const;
    [[nodiscard]] Span<const PropDebrisPiece> GetDebris() const;
    [[nodiscard]] Vector<PropEvent>           TakeEvents();
    [[nodiscard]] ItemWorld&                  GetItems();
    [[nodiscard]] const ItemWorld&            GetItems() const;

    // 시각 전용 장식
    uint32_t                 SpawnDecor(const DecorSpawn& _spawn);
    void                     ClearDecorByLayerMask(uint32_t _layerMask);
    void                     ClearAll();
    void                     Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont = nullptr);
    [[nodiscard]] static int SortKey(const PropInstance& _inst);
    [[nodiscard]] uint64_t   HashState() const;

private:
    [[nodiscard]] size_t      FindIndex_(PropHandle _id) const;
    [[nodiscard]] float       RollFloat_(float _lo, float _hi);
    [[nodiscard]] int         RollInt_(int _n);
    void                      Destroy_(PropInstance& _inst, const PropDef& _def, VECTOR2 _center);
    void                      Detonate_(const PropBurst& _burst, VECTOR2 _center);
    void                      QueueDrops_(const PropDef& _def, VECTOR2 _center);
    void                      UpdateVisual_(PropInstance& _inst, const AnimationLibrary* _pAnims, float _dt);
    void                      RebuildCollision_();
    [[nodiscard]] ePropVisual ComputeVisual_(const PropInstance& _inst) const;
    [[nodiscard]] static bool PartActive_(const PropPart& _part, const PropInstance& _inst);

    const PropDefLibrary*   m_pLibrary        = nullptr;
    const SpriteLibrary*    m_pSprites        = nullptr;
    Vector<PropInstance>    m_props           = {};
    Vector<PropEvent>       m_events          = {};
    Vector<PropFxRequest>   m_pendingFx       = {};
    Vector<PropDebrisPiece> m_debris          = {};
    Vector<PendingChild>    m_pendingChildren = {};
    Vector<Platform>        m_platforms       = {};
    Vector<Platform>        m_enemyPlatforms  = {};
    Vector<Wall>            m_walls           = {};
    ItemWorld               m_items;
    DecorWorld              m_decor   = {};
    EffectList              m_effects = {};
    RngMt19937              m_rng     = RngMt19937 { uint64_t { 1 } };
    uint32_t                m_nextId  = kPropOwnerBase;
};

bool ApplyGroundPatch(GroundMap& _map, VECTOR2 _topLeft, const GroundMap& _footprint);

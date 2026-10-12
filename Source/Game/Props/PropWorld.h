#pragma once
#include <cstdint>

#include "Base/Math.h"
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

inline constexpr int kPropDebrisDepth = 39;   // 디브리
inline constexpr int kPropEffectDepth = 50;   // 폭발 연출 (EffectList 기본과 같음)

// 건물 그리기 키 = -x * kBuildingDepthXScale + y. x 는 1px 단위라 건물 사이 x 차이가 항상 키 순서를 정한다.
inline constexpr int kBuildingDepthXScale = 4096;

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
    Vec2           pos      = { 0.f, 0.f };
    int            value    = 0;
    String         name     = {};
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
    PropHandle                                                 id          = kNoProp;
    int                                                        defIndex    = -1;
    int                                                        variant     = -1;
    Vec2                                                       pos         = { 0.f, 0.f };     // 스프라이트 중심 (legacyCenter, 변종 dy 반영)
    int                                                        depth       = kPropDepthBack;   // 인스턴스 z
    float                                                      cullMargin  = 200.f;
    float                                                      sortOffsetY = 0.f;     // def.sortOffsetY 복사
    bool                                                       bSortByX    = false;   // def.bSortByX 복사 (건물 깊이 규칙)
    Health                                                     health      = {};
    ARRAY<PropVisual, static_cast<size_t>(ePropVisual::Count)> visuals     = {};
    Vector<PropPart>                                           parts       = {};

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
    float secondaryTimerSec     = -1.f;   // >= 0: 2차 폭발 대기
    bool  bRemoveAfterSecondary = false;

    bool  bFalling       = false;
    float fallVy         = 0.f;
    float fallGravity    = 700.f;
    float fallHalfHeight = 0.f;
    float landTimerSec   = -1.f;   // >= 0: 착지 후 경과 (afterLandSec 뒤 폭발)

    bool bRemove = false;
};

struct PropFxRequest
{
    String clip  = {};
    Vec2   pos   = { 0.f, 0.f };
    int    depth = kPropEffectDepth;
};

struct PropDebrisPiece
{
    Vec2       pos    = { 0.f, 0.f };
    Vec2       vel    = { 0.f, 0.f };
    float      ageSec = 0.f;
    String     clip   = {};
    AnimPlayer anim   = {};
    bool       bBound = false;
};

struct PropTickContext
{
    const GroundMap*        pGround      = nullptr;
    const AnimationLibrary* pAnims       = nullptr;
    float                   cameraLeft   = -1e9f;   // 이 x 에서 cullMargin 만큼 더 왼쪽 객체는 제거
    float                   cameraBottom = 1e9f;    // 카메라 하단 y. 잔해는 이 값 + kDebrisCullBelowMargin 을 넘으면 제거 (미설정이면 y 컬링 없음)
    float                   dt           = kStepSec;
    const ItemTuning*       pItemTuning  = nullptr;   // null 이면 기본값
};

class PropWorld
{
    struct PendingChild
    {
        int  defIndex = -1;
        Vec2 pos      = { 0.f, 0.f };
    };

public:
    explicit PropWorld(
        const PropDefLibrary* _pLibrary,
        uint64_t              _seed = 1);

    void                                SetLibrary(const PropDefLibrary* _pLibrary);
    [[nodiscard]] const PropDefLibrary* GetLibrary() const;

    void Reset(uint64_t _seed = 1);

    PropHandle Spawn(
        StringView             _defName,
        Vec2                   _center,
        const PropSpawnParams& _params = {});
    PropHandle SpawnByIndex(
        int                    _defIndex,
        Vec2                   _center,
        const PropSpawnParams& _params = {});

    uint32_t SpawnItem(
        StringView _name,
        Vec2       _feet,
        bool       _bBounce = false,
        int        _z       = kDefaultZ);

    void Tick(const PropTickContext& _ctx);

    // 스프라이트 크기 조회용 (컬링이 화면에 걸친 소품을 지우지 않게 한다). 없으면 pos 기준 컬링만 한다
    void SetSpriteLibrary(const SpriteLibrary* _pSprites);
    // 현재 시각 요소의 월드 사각형. 그릴 것이 없으면 false
    [[nodiscard]] bool GetVisualBounds(const PropInstance& _inst, const AnimationLibrary* _pAnims, Aabb& _out) const;
    void               RegisterHurtboxes(CombatSystem& _combat) const;
    void               OnHitEvents(Span<const HitEvent> _events);

    [[nodiscard]] Span<const Platform> GetPlatforms() const;
    [[nodiscard]] Span<const Platform> GetEnemyPlatforms() const;
    [[nodiscard]] Span<const Wall>     GetWalls() const;

    [[nodiscard]] bool                        IsDestroyed(PropHandle _handle) const;
    [[nodiscard]] bool                        IsDestroyFinished(PropHandle _handle) const;
    [[nodiscard]] bool                        IsAlive(PropHandle _handle) const;
    [[nodiscard]] int                         GetHp(PropHandle _handle) const;
    [[nodiscard]] const PropInstance*         Get(PropHandle _handle) const;
    [[nodiscard]] Span<const PropInstance>    GetProps() const;
    [[nodiscard]] size_t                      GetCount() const;
    [[nodiscard]] Span<const PropDebrisPiece> GetDebris() const;

    [[nodiscard]] Vector<PropEvent> TakeEvents();

    [[nodiscard]] ItemWorld&       GetItems();
    [[nodiscard]] const ItemWorld& GetItems() const;

    // 시각 전용 장식 (anim_decor). 판정/피격 없음.
    uint32_t                        SpawnDecor(const DecorSpawn& _spawn);
    void                            ClearDecorByLayerMask(uint32_t _layerMask);

    void ClearAll();

    void Submit(
        SpriteBatch&            _sprites,
        const AnimationLibrary& _anims,
        const SpriteLibrary&    _spriteLib,
        const BitmapFont*       _pFont = nullptr);

    // 같은 레이어 안 그리기 순서 키 (작을수록 먼저 = 뒤에 그려짐)
    [[nodiscard]] static int SortKey(const PropInstance& _inst);

    [[nodiscard]] uint64_t HashState() const;

private:
    [[nodiscard]] size_t FindIndex_(PropHandle _id) const;
    [[nodiscard]] float  RollFloat_(float _lo, float _hi);
    [[nodiscard]] int    RollInt_(int _n);

    void Destroy_(PropInstance& _inst, const PropDef& _def, Vec2 _center);
    void Detonate_(const PropBurst& _burst, Vec2 _center);
    void QueueDrops_(const PropDef& _def, Vec2 _center);
    void UpdateVisual_(PropInstance& _inst, const AnimationLibrary* _pAnims, float _dt);
    void RebuildCollision_();

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

// 지면 패치: 발자국 마스크의 고체 픽셀만 공기로 만든다 (Docs/PropSpecs.md §2 groundPatch). 가드에 걸리면 변경 없이 false.
bool ApplyGroundPatch(GroundMap& _map, Vec2 _topLeft, const GroundMap& _footprint);

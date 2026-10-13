#pragma once
#include <optional>

#include "Base/Types.h"
#include "Game/Enemies/Enemy.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Game/Sandbox/Effects.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

inline constexpr int kEnemyDepth   = 15;
inline constexpr int kOverlayDepth = 14;
inline constexpr int kShotDepth    = kEnemyShotDepth;
inline constexpr int kTrailDepth   = 29;

[[nodiscard]] int GetEnemyDefaultZ(eEnemyBehavior _behavior);

class EnemyWorld
{
    JUG_CLASS(EnemyWorld, NO_COPY, NO_MOVE)

public:
    explicit EnemyWorld(const EnemyDefLibrary* _pLibrary, uint64_t _seed = 1);
    void                                 SetLibrary(const EnemyDefLibrary* _pLibrary);
    [[nodiscard]] const EnemyDefLibrary* GetLibrary() const;
    void                                 Reset(uint64_t _seed = 1);
    void                                 ReseedRng(uint64_t _seed);
    EnemyHandle                          Spawn(StringView _defName, VECTOR2 _feet, const SpawnParams& _params = {}, const PhysicsWorld* _pSnapWorld = nullptr);
    EnemyHandle                          SpawnByIndex(int _defIndex, VECTOR2 _feet, const SpawnParams& _params = {}, const PhysicsWorld* _pSnapWorld = nullptr);
    void                                 RegisterHurtboxes(CombatSystem& _combat) const;
    void                                 Tick(const EnemyContext& _ctx);
    void                                 OnHitEvents(Span<const HitEvent> _events);
    void                                 BindAnims(const AnimationLibrary& _anims);
    void                                 Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const ProjectileSystem& _projectiles, float _alpha);
    void                                 SetSpriteLibrary(const SpriteLibrary* _pSprites);
    [[nodiscard]] const EnemyState*      Get(EnemyHandle _handle) const;
    [[nodiscard]] const Enemy*           FindEnemy(EnemyHandle _handle) const;
    bool                                 SetZ(EnemyHandle _handle, int _z);
    [[nodiscard]] bool                   IsDefeated(EnemyHandle _handle) const;
    [[nodiscard]] bool                   IsDestroyFinished(EnemyHandle _handle) const;
    [[nodiscard]] bool                   IsTagDefeated(uint32_t _userTag) const;
    [[nodiscard]] EnemyHandle            ResolveHandle(EnemyHandle _handle) const;
    [[nodiscard]] int                    CountAlive(const EnemyFilter& _filter = {}) const;
    [[nodiscard]] int                    CountAlive(StringView _defName) const;

    // 보스 자동 전진
    void  RequestCameraScroll(float _dx);
    float TakeCameraScroll();

    // 살아 있는 적의 def.solid 상자
    [[nodiscard]] Vector<Aabb>              GetSolidBoxes() const;
    bool                                    FireWeaponNow(EnemyHandle _handle, int _weapon);
    bool                                    PlayMotion(EnemyHandle _handle, int _weapon, int _fireFrame);
    [[nodiscard]] Span<const Scoped<Enemy>> GetEnemies() const;
    [[nodiscard]] size_t                    GetCount() const;
    [[nodiscard]] const EffectList&         GetEffects() const;
    [[nodiscard]] Vector<EnemyEvent>        TakeEvents();
    [[nodiscard]] Vector<String>            TakeSounds();
    void                                    ClearMonsters(bool _bIncludeNpc = false);
    void                                    RefreshBodies(int _defIndex, VECTOR2 _size);
    void                                    KillAll(bool _bAwardScore = false);
    [[nodiscard]] uint64_t                  HashState() const;
    [[nodiscard]] int                       Roll(int _n);
    void                                    PushEvent(EnemyEvent _event);
    void                                    RequestFx(EnemyFxRequest _request);
    EnemyHandle                             SpawnDeferred(int _defIndex, VECTOR2 _feet, const SpawnParams& _params);
    void                                    NoteSuccessor(EnemyHandle _old, EnemyHandle _new);
    [[nodiscard]] EnemyEvent                MakeEvent(const EnemyState& _e, eEnemyEventKind _kind) const;

private:
    struct MotionPlay
    {
        uint32_t id       = 0;
        int      weapon   = -1;
        int      clip     = -1;
        int      ticks    = 0;
        int      fireTick = 0;
        int      endTick  = 0;
    };

    struct Successor
    {
        uint32_t oldId = 0;
        uint32_t newId = 0;
    };

    struct DeferredSpawn
    {
        uint32_t    id       = 0;
        int         defIndex = -1;
        VECTOR2     feet     = { 0.f, 0.f };
        SpawnParams params   = {};
    };

    [[nodiscard]] size_t FindIndex_(uint32_t _id) const;
    void                 Create_(uint32_t _id, int _defIndex, VECTOR2 _feet, const SpawnParams& _params, const PhysicsWorld* _pSnapWorld);
    void                 BeginDying_(Enemy& _enemy, const EnemyDef& _def, EnemyEnv& _env);
    void                 RemoveGone_();
    void                 FlushFx_(const AnimationLibrary* _pAnims);
    void                 SpawnShotTrails_(const ProjectileSystem& _projectiles, const AnimationLibrary& _anims);

    MotionPlay m_play = {};

    const EnemyDefLibrary* m_pLibrary     = nullptr;
    const SpriteLibrary*   m_pSprites     = nullptr;
    EnemyContext           m_lastCtx      = {};
    float                  m_cameraScroll = 0.f;

    Vector<Scoped<Enemy>>  m_enemies    = {};
    Vector<EnemyEvent>     m_events     = {};
    Vector<String>         m_sounds     = {};
    Vector<EnemyFxRequest> m_pendingFx  = {};
    Vector<DeferredSpawn>  m_deferred   = {};
    Vector<Successor>      m_successors = {};
    EffectList             m_effects    = {};
    RngMt19937             m_rng        = RngMt19937 { uint64_t { 1 } };
    uint32_t               m_nextId     = kEnemyOwnerBase;

    Vector<Vector<ARRAY<AnimId, 2>>> m_clipIds        = {};
    Vector<ARRAY<AnimId, 2>>         m_projFlyIds     = {};
    Vector<ARRAY<AnimId, 2>>         m_projBeginIds   = {};
    Vector<ARRAY<AnimId, 2>>         m_projLandIds    = {};
    bool                             m_bAnimsBound    = false;
    size_t                           m_boundClipCount = 0;
};

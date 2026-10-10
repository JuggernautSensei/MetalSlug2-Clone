#pragma once
#include <optional>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyBehaviors.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Game/Sandbox/Effects.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{

class EnemyWorld
{
public:
    explicit EnemyWorld(
        const EnemyDefLibrary* _pLibrary = nullptr,
        uint64_t               _seed     = 1);

    void                                 SetLibrary(const EnemyDefLibrary* _pLibrary);
    [[nodiscard]] const EnemyDefLibrary* GetLibrary() const;

    void Reset(
        uint64_t _seed = 1);

    // 테스트 전용: 전투 중 난수열만 바꾼다 (적 목록 유지)
    void ReseedRng(uint64_t _seed);

    EnemyHandle Spawn(
        StringView          _defName,
        Vec2                _feet,
        const SpawnParams&  _params     = {},
        const PhysicsWorld* _pSnapWorld = nullptr);

    EnemyHandle SpawnByIndex(
        int                 _defIndex,
        Vec2                _feet,
        const SpawnParams&  _params     = {},
        const PhysicsWorld* _pSnapWorld = nullptr);

    void RegisterHurtboxes(CombatSystem& _combat) const;
    void Tick(const EnemyContext& _ctx);
    void OnHitEvents(Span<const HitEvent> _events);

    void BindAnims(const AnimationLibrary& _anims);

    void Submit(
        SpriteBatch&            _sprites,
        const AnimationLibrary& _anims,
        const ProjectileSystem& _projectiles,
        float                   _alpha);

    void SetSpriteLibrary(const SpriteLibrary* _pSprites);

    [[nodiscard]] const Enemy* Get(EnemyHandle _handle) const;

    [[nodiscard]] bool        IsDefeated(EnemyHandle _handle) const;
    void                      SendCommand(EnemyHandle _handle, StringView _command);
    [[nodiscard]] Health      GetHealth(EnemyHandle _handle) const;
    [[nodiscard]] bool        IsDestroyFinished(EnemyHandle _handle) const;
    void                      RequestPlayerMinX(float _minX);
    [[nodiscard]] bool        HasPlayerMinX() const;
    [[nodiscard]] float       GetPlayerMinX() const;
    void                      RequestPlayerMaxX(float _maxX);
    [[nodiscard]] bool        HasPlayerMaxX() const;
    [[nodiscard]] float       GetPlayerMaxX() const;
    [[nodiscard]] bool        IsTagDefeated(uint32_t _userTag) const;
    [[nodiscard]] EnemyHandle ResolveHandle(EnemyHandle _handle) const;

    [[nodiscard]] int CountAlive(
        const EnemyFilter& _filter = {}) const;

    [[nodiscard]] int CountAlive(StringView _defName) const;

    // 보스 자동 전진 (카메라 중심 하한 상승량, px). 행동이 틱 중 요청하고 StageState 가 TakeCameraScroll 로 가져가 StageRunner 에 넘긴다.
    void  RequestCameraScroll(float _dx);
    float TakeCameraScroll();

    // 카메라 흔들림 요청 (보스 등장 중 낮은 진폭). 가장 큰 진폭/긴 시간을 남기고, TakeCameraShake 가 가져간다.
    void RequestCameraShake(float _amplitude, float _seconds);
    void TakeCameraShake(float& _amplitude, float& _seconds);

    template<typename Fn>
    void ForEachAlive(
        Fn&& _fn) const
    {
        for (const Enemy& enemy: m_enemies)
        {
            if (enemy.life == eEnemyLife::Alive)
            {
                _fn(enemy);
            }
        }
    }

    // 살아 있는(Alive) 적의 def.solid 상자 (월드 AABB). StageState 가 벽으로 합친다.
    [[nodiscard]] Vector<Aabb> GetSolidBoxes() const;

    [[nodiscard]] Span<const Enemy> GetEnemies() const;
    [[nodiscard]] size_t            GetCount() const;

    [[nodiscard]] Vector<EnemyEvent> TakeEvents();

    void ClearMonsters(
        bool _bIncludeNpc = false);

    void KillAll(
        bool _bAwardScore = false);

    void CancelPending();

    [[nodiscard]] uint64_t HashState() const;

    [[nodiscard]] int        Roll(int _n);
    void                     PushEvent(EnemyEvent _event);
    void                     RequestFx(EnemyFxRequest _request);
    EnemyHandle              SpawnDeferred(int _defIndex, Vec2 _feet, const SpawnParams& _params);
    void                     NoteSuccessor(EnemyHandle _old, EnemyHandle _new);
    [[nodiscard]] EnemyEvent MakeEvent(const Enemy& _e, eEnemyEventKind _kind) const;

private:
    struct Successor
    {
        uint32_t oldId = 0;
        uint32_t newId = 0;
    };

    struct DeferredSpawn
    {
        uint32_t    id       = 0;
        int         defIndex = -1;
        Vec2        feet     = { 0.f, 0.f };
        SpawnParams params   = {};
    };

    [[nodiscard]] size_t FindIndex_(uint32_t _id) const;

    void Create_(
        uint32_t            _id,
        int                 _defIndex,
        Vec2                _feet,
        const SpawnParams&  _params,
        const PhysicsWorld* _pSnapWorld);

    void BeginDying_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
    void RemoveGone_();
    void FlushFx_(const AnimationLibrary* _pAnims);

    const EnemyDefLibrary* m_pLibrary       = nullptr;
    const SpriteLibrary*   m_pSprites       = nullptr;
    EnemyContext           m_lastCtx        = {};
    float                  m_playerMinX     = 0.f;
    bool                   m_bPlayerMinX    = false;
    float                  m_playerMaxX     = 0.f;
    float                  m_cameraScroll   = 0.f;
    float                  m_shakeAmplitude = 0.f;
    float                  m_shakeSeconds   = 0.f;
    bool                   m_bPlayerMaxX    = false;

    Vector<Enemy>          m_enemies    = {};
    Vector<EnemyEvent>     m_events     = {};
    Vector<EnemyFxRequest> m_pendingFx  = {};
    Vector<DeferredSpawn>  m_deferred   = {};
    Vector<Successor>      m_successors = {};
    EffectList             m_effects    = {};
    RngMt19937             m_rng        = RngMt19937 { uint64_t { 1 } };
    uint32_t               m_nextId     = kEnemyOwnerBase;

    Vector<Vector<ARRAY<AnimId, 2>>> m_clipIds        = {};
    Vector<ARRAY<AnimId, 2>>         m_projFlyIds     = {};
    Vector<ARRAY<AnimId, 2>>         m_projBeginIds   = {};
    Vector<ARRAY<AnimId, 2>>         m_projLandIds    = {};   // 구르는 탄의 착지 클립
    bool                             m_bAnimsBound    = false;
    size_t                           m_boundClipCount = 0;
};

}   // namespace slug

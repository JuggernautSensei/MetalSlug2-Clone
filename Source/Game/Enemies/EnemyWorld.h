#pragma once
#include <JugX/Random.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Enemies/EnemyBehaviors.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Game/Sandbox/Effects.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{

// ===========================================
//  EnemyWorld
//   적(과 포로) 전체를 값 타입으로 보관하고 결정적 순서(스폰 순서)로 갱신한다.
//
//   틱 순서 (호출자 기준 — SandboxState 참조):
//     combat.BeginTick()
//     enemies.RegisterHurtboxes(combat)           ← 플레이어 Tick 전 (칼 자동 발동 질의가 맞도록)
//     player.Tick(...)
//     enemies.Tick(ctx)                           ← AI / 이동 / 공격(Hitbox, 투사체) 등록
//     projectiles.Step(...)
//     events = combat.Resolve()
//     enemies.OnHitEvents(events); player.OnHitEvents(events); projectiles.OnHitEvents(events, combat)
//     for (ev : enemies.TakeEvents()) → 점수/드롭/게이트 처리
//
//   피해는 전부 HitEvent 로만 들어온다 (무기/적 종류별 분기 없음). 방패병의 방패도 별도 Hurtbox 로 표현한다.
// ===========================================

class EnemyWorld
{
public:
    explicit EnemyWorld(const EnemyDefLibrary* _pLibrary = nullptr, uint64_t _seed = 1);

    // 정의 라이브러리 (비소유, 월드보다 오래 살아야 한다)
    void SetLibrary(const EnemyDefLibrary* _pLibrary);
    [[nodiscard]] const EnemyDefLibrary* GetLibrary() const { return m_pLibrary; }

    // 모든 적/이펙트/이벤트를 지우고 RNG 를 다시 시드한다. ID 카운터는 처음(2000)으로 돌아간다.
    void Reset(uint64_t _seed = 1);

    // ===========================================
    //  스폰
    // ===========================================

    // _feet = 발 중앙 월드 좌표. _pSnapWorld 가 있으면 발 아래 지면에 붙인다 (최대 16px). 알 수 없는 def 면 null 핸들.
    EnemyHandle Spawn(jug::StringView _defName, jug::VECTOR2 _feet, const SpawnParams& _params = {}, const PhysicsWorld* _pSnapWorld = nullptr);
    EnemyHandle SpawnByIndex(int _defIndex, jug::VECTOR2 _feet, const SpawnParams& _params = {}, const PhysicsWorld* _pSnapWorld = nullptr);

    // ===========================================
    //  틱
    // ===========================================

    void RegisterHurtboxes(CombatSystem& _combat) const;
    void Tick(const EnemyContext& _ctx);
    void OnHitEvents(jug::Span<const HitEvent> _events);

    // ===========================================
    //  렌더 (SpriteBatch 에 제출만 한다. Flush 는 호출자)
    // ===========================================

    void BindAnims(const AnimationLibrary& _anims);   // 클립 이름 → AnimId 캐시 (Submit 이 필요하면 스스로 호출한다)
    void Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const ProjectileSystem& _projectiles, float _alpha);

    // ===========================================
    //  조회
    // ===========================================

    [[nodiscard]] const Enemy* Get(EnemyHandle _handle) const;
    [[nodiscard]] const Enemy* GetById(uint32_t _id) const;

    // 처치(또는 제거) 되었는가. null/낡은 핸들은 true. 변신한 적은 후계자를 따라간다. 대기(delay) 중이면 false.
    [[nodiscard]] bool IsDefeated(EnemyHandle _handle) const;
    // 같은 userTag 의 적이 하나도 살아있지 않은가 (대기 중 포함). 게이트용
    [[nodiscard]] bool IsTagDefeated(uint32_t _userTag) const;
    // 변신을 따라간 최신 핸들
    [[nodiscard]] EnemyHandle ResolveHandle(EnemyHandle _handle) const;

    // 살아 있는(대기 포함) 수
    [[nodiscard]] int CountAlive(const EnemyFilter& _filter = {}) const;
    [[nodiscard]] int CountAlive(jug::StringView _defName) const;

    template<typename Fn>
    void ForEachAlive(Fn&& _fn) const
    {
        for (const Enemy& enemy: m_enemies)
        {
            if (enemy.life == eEnemyLife::Alive)
            {
                _fn(enemy);
            }
        }
    }

    [[nodiscard]] jug::Span<const Enemy> GetEnemies() const { return m_enemies; }
    [[nodiscard]] size_t                 GetCount() const { return m_enemies.size(); }

    // ===========================================
    //  이벤트 / 정리
    // ===========================================

    // 쌓인 이벤트를 비우며 가져간다 (스폰 순서/발생 순서대로)
    [[nodiscard]] jug::Vector<EnemyEvent> TakeEvents();

    // 보스 시작 등: 적(Enemy 팀)을 연출 없이 즉시 제거한다. _bIncludeNpc 면 포로도. Removed 이벤트 발생
    void ClearMonsters(bool _bIncludeNpc = false);

    // 살아 있는 적을 전부 처치 처리한다 (Died 이벤트, _bAwardScore 면 점수 포함). 대기 중인 것은 취소
    void KillAll(bool _bAwardScore = false);

    // 대기 중인(아직 등장하지 않은) 스폰을 취소한다
    void CancelPending();

    // 시뮬레이션 상태 해시 (결정성 검증)
    [[nodiscard]] uint64_t HashState() const;

    // ===========================================
    //  행동 함수용 서비스 (EnemyBehaviors 가 사용)
    // ===========================================

    [[nodiscard]] int  Roll(int _n);                       // [0, _n)
    void               PushEvent(EnemyEvent _event);
    void               RequestFx(EnemyFxRequest _request);
    EnemyHandle        SpawnDeferred(int _defIndex, jug::VECTOR2 _feet, const SpawnParams& _params);   // 틱이 끝난 뒤에 생성된다
    void               NoteSuccessor(EnemyHandle _old, EnemyHandle _new);
    [[nodiscard]] EnemyEvent MakeEvent(const Enemy& _e, eEnemyEventKind _kind) const;

private:
    struct Successor
    {
        uint32_t oldId = 0;
        uint32_t newId = 0;
    };

    struct DeferredSpawn
    {
        uint32_t     id        = 0;
        int          defIndex  = -1;
        jug::VECTOR2 feet      = { 0.f, 0.f };
        SpawnParams  params    = {};
    };

    [[nodiscard]] size_t       FindIndex_(uint32_t _id) const;   // 없으면 size()
    void                       Create_(uint32_t _id, int _defIndex, jug::VECTOR2 _feet, const SpawnParams& _params, const PhysicsWorld* _pSnapWorld);
    void                       BeginDying_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
    void                       RemoveGone_();
    void                       FlushFx_(const AnimationLibrary* _pAnims);

    const EnemyDefLibrary* m_pLibrary = nullptr;
    EnemyContext           m_lastCtx  = {};   // 마지막 Tick 의 문맥 사본 (OnHitEvents 가 행동 onHit 에 넘긴다)

    jug::Vector<Enemy>          m_enemies    = {};
    jug::Vector<EnemyEvent>     m_events     = {};
    jug::Vector<EnemyFxRequest> m_pendingFx  = {};
    jug::Vector<DeferredSpawn>  m_deferred   = {};
    jug::Vector<Successor>      m_successors = {};
    EffectList                  m_effects    = {};
    jug::RngMt19937             m_rng        = jug::RngMt19937 { uint64_t { 1 } };
    uint32_t                    m_nextId     = kEnemyOwnerBase;

    // 렌더 캐시: [defIndex][clipIndex][side(0 Left, 1 Right)]
    jug::Vector<jug::Vector<std::array<AnimId, 2>>> m_clipIds       = {};
    jug::Vector<std::array<AnimId, 2>>              m_projFlyIds    = {};
    jug::Vector<std::array<AnimId, 2>>              m_projBeginIds  = {};
    bool                                            m_bAnimsBound   = false;
    size_t                                          m_boundClipCount = 0;
};

}   // namespace slug

#include "Game/Enemies/EnemyWorld.h"

#include <algorithm>
#include <cstring>

#include <JugX/CoreLogger.h>

#include "Game/Enemies/EnemyCommon.h"
#include "World/Constants.h"

namespace slug
{

namespace
{
    constexpr float kSpawnSnapPx     = 16.f;    // 스폰 시 발 아래 지면에 붙이는 최대 거리
    constexpr int   kEnemyLayer      = 15;      // 플레이어(20) 뒤, 지면 위
    constexpr int   kOverlayLayer    = 16;
    constexpr int   kShotLayer       = 32;      // 기존 탄(30)/폭탄(31) 위

    [[nodiscard]] jug::VECTOR2 Lerp_(
        const jug::VECTOR2 _a,
        const jug::VECTOR2 _b,
        const float        _t)
    {
        return { _a.x + (_b.x - _a.x) * _t, _a.y + (_b.y - _a.y) * _t };
    }

    // 베이크된 클립의 시간(애니 단위)에서 보이는 프레임 인덱스 (루프/비루프 규칙은 애니메이션과 같다)
    [[nodiscard]] int BakedFrameAt_(
        const AnimClip& _clip,
        const int64_t   _timeUnits)
    {
        const int n = _clip.GetFrameCount();
        if (n <= 1)
        {
            return 0;
        }
        const int64_t units = _clip.GetFrameDurationUnits();
        if (units <= 0)
        {
            return 0;
        }
        const int64_t index = (_timeUnits < 0 ? 0 : _timeUnits) / units;
        if (_clip.bLoop)
        {
            return static_cast<int>(index % n);
        }
        return static_cast<int>(index >= n ? n - 1 : index);
    }

    // 정의의 물리 상수를 적 한 마리용 PhysicsTuning 으로 바꾼다 (중력/낙하 상한은 def 에서)
    [[nodiscard]] PhysicsTuning MakePhys_(
        const EnemyDef& _def)
    {
        PhysicsTuning tuning;
        tuning.gravity          = _def.physics.gravity;
        tuning.terminalVelocity = _def.physics.maxFall;
        return tuning;
    }

    [[nodiscard]] int SideIndex_(
        const int _facing)
    {
        return _facing < 0 ? 0 : 1;
    }
}   // namespace

// ===========================================
//  생성 / 설정
// ===========================================

EnemyWorld::EnemyWorld(
    const EnemyDefLibrary* _pLibrary,
    const uint64_t         _seed)
    : m_pLibrary(_pLibrary)
    , m_rng(_seed)
{
}

void EnemyWorld::SetLibrary(
    const EnemyDefLibrary* _pLibrary)
{
    m_pLibrary     = _pLibrary;
    m_bAnimsBound  = false;
    m_clipIds.clear();
    m_projFlyIds.clear();
    m_projBeginIds.clear();
}

void EnemyWorld::Reset(
    const uint64_t _seed)
{
    m_enemies.clear();
    m_events.clear();
    m_pendingFx.clear();
    m_deferred.clear();
    m_successors.clear();
    m_effects.Clear();
    m_rng.Reseed(_seed);
    m_nextId      = kEnemyOwnerBase;
    m_bPlayerMinX = false;
}

// ===========================================
//  스폰
// ===========================================

EnemyHandle EnemyWorld::Spawn(
    const jug::StringView _defName,
    const jug::VECTOR2    _feet,
    const SpawnParams&    _params,
    const PhysicsWorld*   _pSnapWorld)
{
    if (!m_pLibrary)
    {
        return {};
    }
    return SpawnByIndex(m_pLibrary->FindIndex(_defName), _feet, _params, _pSnapWorld);
}

EnemyHandle EnemyWorld::SpawnByIndex(
    const int           _defIndex,
    const jug::VECTOR2  _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    if (!m_pLibrary || _defIndex < 0 || static_cast<size_t>(_defIndex) >= m_pLibrary->GetDefCount())
    {
        JUG_CORE_LOG_WARN("EnemyWorld::Spawn: 알 수 없는 정의 (index {})", _defIndex);
        return {};
    }
    const uint32_t id = m_nextId++;
    Create_(id, _defIndex, _feet, _params, _pSnapWorld);
    return { id };
}

EnemyHandle EnemyWorld::SpawnDeferred(
    const int           _defIndex,
    const jug::VECTOR2  _feet,
    const SpawnParams&  _params)
{
    const uint32_t id = m_nextId++;
    DeferredSpawn   spawn;
    spawn.id       = id;
    spawn.defIndex = _defIndex;
    spawn.feet     = _feet;
    spawn.params   = _params;
    m_deferred.push_back(spawn);
    return { id };
}

void EnemyWorld::Create_(
    const uint32_t      _id,
    const int           _defIndex,
    const jug::VECTOR2  _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    const EnemyDef& def = m_pLibrary->Get(_defIndex);

    Enemy e;
    e.id         = _id;
    e.defIndex   = _defIndex;
    e.behavior   = def.behavior;
    e.delayTicks = SecToTicks(_params.delaySec);
    e.life       = e.delayTicks > 0 ? eEnemyLife::Pending : eEnemyLife::Alive;

    e.body.position      = _feet;
    e.body.size          = { def.body.w, def.body.h };
    e.body.footHalfWidth = def.physics.footHalfWidth;
    e.body.bUseGravity   = def.physics.bGravity;
    if (_pSnapWorld)
    {
        (void)Physics::SnapToGround(e.body, *_pSnapWorld, kSpawnSnapPx);
    }
    e.prevFeet  = e.body.position;
    e.facing    = _params.facing < 0 ? -1 : 1;
    e.health    = Health::Make(_params.hp > 0 ? _params.hp : def.hp);
    e.bActive   = _params.bActivated || def.activation.bAlways;
    e.aiVariant = _params.aiVariant;
    e.userTag   = _params.userTag;
    e.drop      = _params.drop;
    e.arena     = _params.arena;
    e.clip      = -1;
    e.bVisible  = true;

    m_enemies.push_back(e);
}

// ===========================================
//  판정 등록 / 피해 적용
// ===========================================

void EnemyWorld::RegisterHurtboxes(
    CombatSystem& _combat) const
{
    if (!m_pLibrary)
    {
        return;
    }
    for (const Enemy& e: m_enemies)
    {
        if (e.life != eEnemyLife::Alive)
        {
            continue;
        }
        const EnemyDef&    def = m_pLibrary->Get(e.defIndex);
        const BehaviorOps& ops = GetBehaviorOps(e.behavior);
        if (ops.registerHurtboxes)
        {
            ops.registerHurtboxes(e, def, _combat);
            continue;
        }
        Hurtbox hurtbox;
        hurtbox.box   = def.hurtbox.At(e.body.position, e.facing);
        hurtbox.team  = def.team;
        hurtbox.owner = e.id;
        _combat.AddHurtbox(hurtbox);
    }
}

void EnemyWorld::OnHitEvents(
    const jug::Span<const HitEvent> _events)
{
    if (!m_pLibrary)
    {
        return;
    }
    for (const HitEvent& event: _events)
    {
        if (!IsEnemyOwner(event.target))
        {
            continue;
        }
        const uint32_t id  = event.target & kEnemyIdMask;   // 방패/포탑 파트 비트는 떼고 적 ID 만 본다
        const size_t   idx = FindIndex_(id);
        if (idx >= m_enemies.size())
        {
            continue;
        }
        Enemy& e = m_enemies[idx];
        if (e.life != eEnemyLife::Alive)
        {
            continue;
        }

        const EnemyDef&    def = m_pLibrary->Get(e.defIndex);
        const BehaviorOps& ops = GetBehaviorOps(e.behavior);
        EnemyEnv           env { *this, m_lastCtx, *m_pLibrary, MakePhys_(def) };

        // 행동이 먼저 본다 (방패 막기 / 포로 구출). true 면 기본 피해를 적용하지 않는다.
        if (ops.onHit && ops.onHit(e, def, env, event))
        {
            continue;
        }

        if (e.health.ApplyDamage(event.damage) > 0)
        {
            e.lastHitKind = event.kind;
        }
        if (e.health.IsDead())
        {
            BeginDying_(e, def, env);
        }
    }
}

void EnemyWorld::BeginDying_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.life = eEnemyLife::Dying;
    _e.body.velocity.x = 0.f;
    PushEvent(MakeEvent(_e, eEnemyEventKind::Died));
    const BehaviorOps& ops = GetBehaviorOps(_e.behavior);
    if (ops.onDeath)
    {
        ops.onDeath(_e, _def, _env);
    }
}

// ===========================================
//  틱
// ===========================================

void EnemyWorld::Tick(
    const EnemyContext& _ctx)
{
    if (!m_pLibrary)
    {
        return;
    }
    m_lastCtx     = _ctx;
    m_bPlayerMinX = false;   // 보스가 이번 틱에 다시 요청한다 (플레이어는 다음 틱 시작 전의 값을 읽는다)

    // 0) 투사체 적중/지형 접촉 연출 (직전 틱의 Step 결과)
    if (_ctx.pProjectiles)
    {
        const jug::Vector<ShotFx> shotFx = _ctx.pProjectiles->TakeShotFx();
        for (const ShotFx& fx: shotFx)
        {
            const int projIndex = static_cast<int>(fx.visual) - 1;
            if (projIndex < 0 || static_cast<size_t>(projIndex) >= m_pLibrary->GetProjectileCount() || !_ctx.pAnims)
            {
                continue;
            }
            const FxDef& explosion = m_pLibrary->GetProjectile(projIndex).explosion;
            if (!explosion.clip.empty())
            {
                m_effects.Spawn(*_ctx.pAnims, explosion.clip, { fx.position.x + explosion.offset.x, fx.position.y + explosion.offset.y }, 58);
            }
        }
    }

    // 1) 연출: 지연 이펙트 / 이펙트 진행
    FlushFx_(_ctx.pAnims);
    if (_ctx.pAnims)
    {
        m_effects.Update(*_ctx.pAnims, kStepSec);
    }

    // 2) 적 갱신 (스폰 순서). 틱 중에는 m_enemies 의 크기가 변하지 않는다 (스폰/변신은 SpawnDeferred 로 틱 끝에 반영).
    const Aabb& view = _ctx.cameraView;
    for (Enemy& e: m_enemies)
    {
        if (e.life == eEnemyLife::Gone)
        {
            continue;
        }
        const EnemyDef&    def = m_pLibrary->Get(e.defIndex);
        const BehaviorOps& ops = GetBehaviorOps(e.behavior);
        EnemyEnv           env { *this, m_lastCtx, *m_pLibrary, MakePhys_(def) };

        if (e.life == eEnemyLife::Pending)
        {
            if (--e.delayTicks > 0)
            {
                continue;
            }
            e.life = eEnemyLife::Alive;
        }

        if (!e.bInit)
        {
            e.bInit = true;
            if (ops.init)
            {
                ops.init(e, def, env);
            }
        }

        e.prevFeet = e.body.position;

        if (e.life == eEnemyLife::Alive)
        {
            // 활성화 (래치): 화면 오른쪽 + 여유 안으로 들어오면 행동 시작
            if (!e.bActive && (def.activation.bAlways || e.body.position.x <= view.max.x + def.activation.marginPx))
            {
                e.bActive = true;
            }
            // 정리: 카메라 뒤로 밀려났거나 화면 아래로 떨어졌다
            if (!def.bNoDespawn && (e.body.position.x < view.min.x - m_pLibrary->GetGlobals().despawnBehindCameraPx || e.body.position.y > view.max.y + m_pLibrary->GetGlobals().despawnBelowCameraPx))
            {
                e.life = eEnemyLife::Gone;
                continue;
            }

            ++e.stateTicks;
            ++e.clipTicks;
            if (ops.tick)
            {
                ops.tick(e, def, env);
            }
        }
        else if (e.life == eEnemyLife::Dying)
        {
            ++e.stateTicks;
            ++e.clipTicks;
            if (ops.tickDying)
            {
                ops.tickDying(e, def, env);
            }
            else
            {
                e.life = eEnemyLife::Gone;
            }
        }
    }

    // 3) 틱 끝: 예약된 스폰 반영 → 제거된 적 정리 (Removed 이벤트)
    for (const DeferredSpawn& spawn: m_deferred)
    {
        Create_(spawn.id, spawn.defIndex, spawn.feet, spawn.params, nullptr);
    }
    m_deferred.clear();
    RemoveGone_();

    // 4) 이번 틱 이펙트 요청 적재 (다음 틱에 나타남)
    if (!_ctx.pAnims)
    {
        m_pendingFx.clear();
    }
}

void EnemyWorld::RemoveGone_()
{
    size_t keep = 0;
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        Enemy& e = m_enemies[i];
        if (e.life != eEnemyLife::Gone)
        {
            if (keep != i)
            {
                m_enemies[keep] = e;
            }
            ++keep;
            continue;
        }
        if (!e.bTransformed)
        {
            PushEvent(MakeEvent(e, eEnemyEventKind::Removed));
        }
    }
    m_enemies.resize(keep);
}

void EnemyWorld::FlushFx_(
    const AnimationLibrary* _pAnims)
{
    if (!_pAnims)
    {
        m_pendingFx.clear();
        return;
    }
    size_t keep = 0;
    for (size_t i = 0; i < m_pendingFx.size(); ++i)
    {
        EnemyFxRequest& request = m_pendingFx[i];
        if (request.delayTicks > 0)
        {
            --request.delayTicks;
            if (keep != i)
            {
                m_pendingFx[keep] = request;
            }
            ++keep;
            continue;
        }
        m_effects.Spawn(*_pAnims, request.clip, request.position, request.layer);
    }
    m_pendingFx.resize(keep);
}

// ===========================================
//  렌더
// ===========================================

void EnemyWorld::BindAnims(
    const AnimationLibrary& _anims)
{
    m_clipIds.clear();
    m_projFlyIds.clear();
    m_projBeginIds.clear();
    if (m_pLibrary)
    {
        m_clipIds.resize(m_pLibrary->GetDefCount());
        for (size_t d = 0; d < m_pLibrary->GetDefCount(); ++d)
        {
            const EnemyDef& def = m_pLibrary->Get(static_cast<int>(d));
            m_clipIds[d].resize(def.clips.size());
            for (size_t c = 0; c < def.clips.size(); ++c)
            {
                for (int side = 0; side < 2; ++side)
                {
                    const jug::String name = ResolveSide(def.clips[c].clip.name, side == 0 ? -1 : 1);
                    m_clipIds[d][c][side]  = _anims.Find(name);
                }
            }
        }

        m_projFlyIds.resize(m_pLibrary->GetProjectileCount());
        m_projBeginIds.resize(m_pLibrary->GetProjectileCount());
        m_projLandIds.resize(m_pLibrary->GetProjectileCount());
        for (size_t p = 0; p < m_pLibrary->GetProjectileCount(); ++p)
        {
            const ProjectileDef& proj = m_pLibrary->GetProjectile(static_cast<int>(p));
            for (int side = 0; side < 2; ++side)
            {
                const int facing = side == 0 ? -1 : 1;
                m_projFlyIds[p][side]   = proj.fly.name.empty() ? AnimId {} : _anims.Find(ResolveSide(proj.fly.name, facing));
                m_projBeginIds[p][side] = proj.begin.name.empty() ? AnimId {} : _anims.Find(ResolveSide(proj.begin.name, facing));
                m_projLandIds[p][side]  = proj.land.name.empty() ? AnimId {} : _anims.Find(ResolveSide(proj.land.name, facing));
            }
        }
    }
    m_boundClipCount = _anims.GetClipCount();
    m_bAnimsBound    = true;
}

void EnemyWorld::Submit(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const ProjectileSystem& _projectiles,
    const float             _alpha)
{
    if (!m_pLibrary)
    {
        return;
    }
    if (!m_bAnimsBound || m_boundClipCount != _anims.GetClipCount())
    {
        BindAnims(_anims);
    }

    // --- 적 / 포로 ---
    for (const Enemy& e: m_enemies)
    {
        if (!e.bVisible || e.life == eEnemyLife::Pending || e.life == eEnemyLife::Gone)
        {
            continue;
        }
        const EnemyDef&    def  = m_pLibrary->Get(e.defIndex);
        const BehaviorOps& ops  = GetBehaviorOps(e.behavior);
        const int          side = SideIndex_(e.facing);
        const jug::VECTOR2 feet = Lerp_(e.prevFeet, e.body.position, _alpha);

        // 부가 파츠 (보스 리그 등): 본체 클립과 무관하게 매 틱 그린다
        if (ops.submitParts)
        {
            const EnemyRenderEnv render { _sprites, _anims, m_pSprites, _projectiles, feet, _alpha, kEnemyLayer };
            ops.submitParts(e, def, render);
        }
        if (e.clip < 0)
        {
            continue;
        }

        const AnimClip* pClip = _anims.GetOrNull(m_clipIds[static_cast<size_t>(e.defIndex)][static_cast<size_t>(e.clip)][side]);
        if (!pClip)
        {
            continue;
        }
        const AnimFrame* pFrame = &pClip->frames[static_cast<size_t>(BakedFrameAt_(*pClip, static_cast<int64_t>(e.clipTicks) * kAnimUnitsPerTick))];
        if (pFrame->IsEmpty())
        {
            continue;
        }
        const float lift = static_cast<float>(pClip->GetFootBaseline());
        _sprites.Submit(kEnemyLayer, static_cast<int>(feet.y), *pFrame, { feet.x, feet.y - lift });

        // 몸 위 오버레이 (낙하산 등)
        if (def.overlayClip >= 0)
        {
            const AnimClip* pOver = _anims.GetOrNull(m_clipIds[static_cast<size_t>(e.defIndex)][static_cast<size_t>(def.overlayClip)][side]);
            if (pOver)
            {
                const AnimFrame* pOverFrame = &pOver->frames[static_cast<size_t>(BakedFrameAt_(*pOver, static_cast<int64_t>(e.clipTicks) * kAnimUnitsPerTick))];
                if (!pOverFrame->IsEmpty())
                {
                    _sprites.Submit(kOverlayLayer, static_cast<int>(feet.y), *pOverFrame, { feet.x + def.overlayOffset.x, feet.y + def.overlayOffset.y - lift });
                }
            }
        }
    }

    // --- 적 투사체 (Shot) ---
    for (const Shot& shot: _projectiles.GetShots())
    {
        if (!shot.bAlive)
        {
            continue;
        }
        const int projIndex = static_cast<int>(shot.params.visual) - 1;
        if (projIndex < 0 || static_cast<size_t>(projIndex) >= m_projFlyIds.size())
        {
            continue;
        }
        // 구르는 탄: 방향은 굴러가는 쪽, 낙하 중에는 begin(낙하) → 착지 시 land → 굴러가며 fly
        const size_t pIdx     = static_cast<size_t>(projIndex);
        const bool   bRoll    = shot.params.rollSpeed > 0.f;
        const int    side     = bRoll ? (shot.params.rollDirX < 0.f ? 0 : 1) : (shot.velocity.x < 0.f ? 0 : 1);
        const int    landAge  = shot.ageTicks - shot.landedAge;
        const int    landTicks = m_pLibrary->GetProjectile(projIndex).land.GetDurationTicks();
        const bool   bLand    = bRoll && shot.bLanded && landAge < landTicks && _anims.IsValid(m_projLandIds[pIdx][side]);
        const bool   bBegin   = !bLand && (shot.ageTicks < shot.params.startDelayTicks || (bRoll && !shot.bLanded)) && _anims.IsValid(m_projBeginIds[pIdx][side]);
        const AnimId clipId   = bLand ? m_projLandIds[pIdx][side] : (bBegin ? m_projBeginIds[pIdx][side] : m_projFlyIds[pIdx][side]);
        const AnimClip* pClip = _anims.GetOrNull(clipId);
        if (!pClip)
        {
            continue;
        }
        int ticks = std::max(0, shot.ageTicks - shot.params.startDelayTicks);
        if (bLand)
        {
            ticks = landAge;
        }
        else if (bBegin)
        {
            ticks = shot.ageTicks;
        }
        else if (bRoll && shot.bLanded)
        {
            ticks = std::max(0, landAge - landTicks);
        }
        const AnimFrame* pFrame = &pClip->frames[static_cast<size_t>(BakedFrameAt_(*pClip, static_cast<int64_t>(ticks) * kAnimUnitsPerTick))];
        if (pFrame->IsEmpty())
        {
            continue;
        }
        const jug::VECTOR2 pos = Lerp_(shot.renderPrev, shot.position, _alpha);
        _sprites.Submit(kShotLayer, static_cast<int>(pos.y), *pFrame, pos);
    }

    m_effects.Submit(_anims, _sprites);
}

// ===========================================
//  조회
// ===========================================

size_t EnemyWorld::FindIndex_(
    const uint32_t _id) const
{
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        if (m_enemies[i].id == _id)
        {
            return i;
        }
    }
    return m_enemies.size();
}

const Enemy* EnemyWorld::Get(
    const EnemyHandle _handle) const
{
    const size_t idx = FindIndex_(_handle.id);
    return idx < m_enemies.size() ? &m_enemies[idx] : nullptr;
}

const Enemy* EnemyWorld::GetById(
    const uint32_t _id) const
{
    const size_t idx = FindIndex_(_id);
    return idx < m_enemies.size() ? &m_enemies[idx] : nullptr;
}

EnemyHandle EnemyWorld::ResolveHandle(
    const EnemyHandle _handle) const
{
    uint32_t id = _handle.id;
    for (int guard = 0; guard < 64; ++guard)
    {
        bool bMoved = false;
        for (const Successor& s: m_successors)
        {
            if (s.oldId == id)
            {
                id     = s.newId;
                bMoved = true;
                break;
            }
        }
        if (!bMoved)
        {
            break;
        }
    }
    return { id };
}

bool EnemyWorld::IsDefeated(
    const EnemyHandle _handle) const
{
    if (_handle.IsNull())
    {
        return true;
    }
    const Enemy* pEnemy = Get(ResolveHandle(_handle));
    if (!pEnemy)
    {
        return true;
    }
    if (pEnemy->life == eEnemyLife::Pending)
    {
        return false;
    }
    return pEnemy->life != eEnemyLife::Alive;
}

Health EnemyWorld::GetHealth(
    const EnemyHandle _handle) const
{
    const Enemy* pEnemy = Get(ResolveHandle(_handle));
    return pEnemy ? pEnemy->health : Health { 0, 0 };
}

bool EnemyWorld::IsDestroyFinished(
    const EnemyHandle _handle) const
{
    if (_handle.IsNull())
    {
        return true;
    }
    const Enemy* pEnemy = Get(ResolveHandle(_handle));
    if (!pEnemy || pEnemy->life == eEnemyLife::Gone)
    {
        return true;
    }
    if (pEnemy->life != eEnemyLife::Dying || !m_pLibrary)
    {
        return false;
    }
    const BehaviorOps& ops = GetBehaviorOps(pEnemy->behavior);
    return ops.deathDone ? ops.deathDone(*pEnemy, m_pLibrary->Get(pEnemy->defIndex)) : false;
}

void EnemyWorld::RequestPlayerMinX(
    const float _minX)
{
    m_playerMinX  = m_bPlayerMinX ? std::max(m_playerMinX, _minX) : _minX;
    m_bPlayerMinX = true;
}

bool EnemyWorld::IsTagDefeated(
    const uint32_t _userTag) const
{
    for (const Enemy& e: m_enemies)
    {
        if (e.userTag == _userTag && (e.life == eEnemyLife::Alive || e.life == eEnemyLife::Pending))
        {
            return false;
        }
    }
    return true;
}

int EnemyWorld::CountAlive(
    const EnemyFilter& _filter) const
{
    int count = 0;
    for (const Enemy& e: m_enemies)
    {
        if (e.life != eEnemyLife::Alive && e.life != eEnemyLife::Pending)
        {
            continue;
        }
        if (_filter.defIndex >= 0 && e.defIndex != _filter.defIndex)
        {
            continue;
        }
        if (_filter.userTag != 0 && e.userTag != _filter.userTag)
        {
            continue;
        }
        if (_filter.bHostileOnly && m_pLibrary && m_pLibrary->Get(e.defIndex).team != eTeam::Enemy)
        {
            continue;
        }
        ++count;
    }
    return count;
}

int EnemyWorld::CountAlive(
    const jug::StringView _defName) const
{
    EnemyFilter filter;
    filter.defIndex = m_pLibrary ? m_pLibrary->FindIndex(_defName) : -1;
    filter.bHostileOnly = false;
    if (filter.defIndex < 0)
    {
        return 0;
    }
    return CountAlive(filter);
}

// ===========================================
//  이벤트 / 정리
// ===========================================

jug::Vector<EnemyEvent> EnemyWorld::TakeEvents()
{
    jug::Vector<EnemyEvent> out = std::move(m_events);
    m_events.clear();
    return out;
}

void EnemyWorld::ClearMonsters(
    const bool _bIncludeNpc)
{
    if (!m_pLibrary)
    {
        return;
    }
    size_t keep = 0;
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        Enemy& e = m_enemies[i];
        const bool bRemove = m_pLibrary->Get(e.defIndex).team == eTeam::Enemy || _bIncludeNpc;
        if (!bRemove)
        {
            if (keep != i)
            {
                m_enemies[keep] = e;
            }
            ++keep;
            continue;
        }
        PushEvent(MakeEvent(e, eEnemyEventKind::Removed));
    }
    m_enemies.resize(keep);
}

void EnemyWorld::KillAll(
    const bool _bAwardScore)
{
    if (!m_pLibrary)
    {
        return;
    }
    size_t keep = 0;
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        Enemy& e = m_enemies[i];
        if (e.life == eEnemyLife::Pending)
        {
            continue;   // 대기 중인 것은 취소
        }
        if (e.life == eEnemyLife::Alive)
        {
            EnemyEvent event = MakeEvent(e, eEnemyEventKind::Died);
            event.score      = _bAwardScore ? m_pLibrary->Get(e.defIndex).score : 0;
            PushEvent(std::move(event));
            continue;   // 연출 없이 제거
        }
        if (keep != i)
        {
            m_enemies[keep] = e;
        }
        ++keep;
    }
    m_enemies.resize(keep);
}

void EnemyWorld::CancelPending()
{
    size_t keep = 0;
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        if (m_enemies[i].life == eEnemyLife::Pending)
        {
            continue;
        }
        if (keep != i)
        {
            m_enemies[keep] = m_enemies[i];
        }
        ++keep;
    }
    m_enemies.resize(keep);
    m_deferred.clear();
}

uint64_t EnemyWorld::HashState() const
{
    uint64_t   hash = 14695981039346656037ull;
    const auto Mix  = [&hash](const int64_t _value) {
        unsigned char bytes[sizeof(int64_t)];
        std::memcpy(bytes, &_value, sizeof(int64_t));
        for (const unsigned char byte: bytes)
        {
            hash ^= byte;
            hash *= 1099511628211ull;
        }
    };
    for (const Enemy& e: m_enemies)
    {
        Mix(static_cast<int64_t>(e.id));
        Mix(e.defIndex);
        Mix(static_cast<int64_t>(e.life));
        Mix(e.state);
        Mix(e.stateTicks);
        Mix(e.clipTicks);
        Mix(e.clip);
        Mix(e.health.hp);
        Mix(e.facing);
        Mix(e.counter);
        Mix(e.aux);
        Mix(e.aux2);
        Mix(e.aux3);
        Mix(e.aux4);
        Mix(e.shotsFired);
        Mix(static_cast<int64_t>(e.body.position.x * 16.f));
        Mix(static_cast<int64_t>(e.body.position.y * 16.f));
    }
    Mix(static_cast<int64_t>(m_events.size()));
    return hash;
}

// ===========================================
//  행동 함수용 서비스
// ===========================================

int EnemyWorld::Roll(
    const int _n)
{
    if (_n <= 1)
    {
        return 0;
    }
    return static_cast<int>(m_rng.Generate() % static_cast<uint64_t>(_n));
}

void EnemyWorld::PushEvent(
    EnemyEvent _event)
{
    m_events.push_back(std::move(_event));
}

void EnemyWorld::RequestFx(
    EnemyFxRequest _request)
{
    m_pendingFx.push_back(std::move(_request));
}

void EnemyWorld::NoteSuccessor(
    const EnemyHandle _old,
    const EnemyHandle _new)
{
    Successor successor;
    successor.oldId = _old.id;
    successor.newId = _new.id;
    m_successors.push_back(successor);
}

EnemyEvent EnemyWorld::MakeEvent(
    const Enemy&          _e,
    const eEnemyEventKind _kind) const
{
    EnemyEvent event;
    event.kind     = _kind;
    event.handle   = { _e.id };
    event.defIndex = _e.defIndex;
    event.pos      = _e.body.position;
    event.drop     = _e.drop;
    event.userTag  = _e.userTag;
    event.hitKind  = _e.lastHitKind;
    if (m_pLibrary && _e.defIndex >= 0)
    {
        const EnemyDef& def = m_pLibrary->Get(_e.defIndex);
        event.defName = def.name;
        event.score   = _kind == eEnemyEventKind::Died ? def.score : 0;
    }
    return event;
}

}   // namespace slug

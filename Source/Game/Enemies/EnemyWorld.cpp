#include "Game/Enemies/EnemyWorld.h"

#include <algorithm>
#include <cstring>

#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyCommon.h"
#include "World/Constants.h"

namespace slug
{

namespace
{
    constexpr float kSpawnSnapPx  = 16.f;   // 스폰 시 발 아래 지면에 붙이는 최대 거리
    constexpr int   kEnemyLayer   = 15;     // 플레이어(20) 뒤, 지면 위
    constexpr int   kOverlayLayer = 16;
    constexpr int   kShotLayer    = 32;   // 기존 탄(30)/폭탄(31) 위

    [[nodiscard]] Vec2 Lerp_(
        const Vec2  _a,
        const Vec2  _b,
        const float _t)
    {
        return { _a.x + (_b.x - _a.x) * _t, _a.y + (_b.y - _a.y) * _t };
    }

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

    [[nodiscard]] PhysicsTuning MakePhys_(
        const EnemyDef& _def)
    {
        PhysicsTuning tuning;
        tuning.gravity          = _def.physics.gravity;
        tuning.terminalVelocity = _def.physics.maxFall;
        return tuning;
    }

    void MixHash_(
        uint64_t&     _hash,
        const int64_t _value)
    {
        ARRAY<unsigned char, sizeof(int64_t)> bytes = {};
        std::memcpy(bytes.data(), &_value, sizeof(int64_t));
        for (const unsigned char byte: bytes)
        {
            _hash ^= byte;
            _hash *= 1'099'511'628'211ull;
        }
    }

    [[nodiscard]] int SideIndex_(
        const int _facing)
    {
        return _facing < 0 ? 0 : 1;
    }

}   // namespace

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
    m_pLibrary    = _pLibrary;
    m_bAnimsBound = false;
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
    m_nextId         = kEnemyOwnerBase;
    m_bPlayerMinX    = false;
    m_bPlayerMaxX    = false;
    m_cameraScroll   = 0.f;
    m_shakeAmplitude = 0.f;
    m_shakeSeconds   = 0.f;
}

EnemyHandle EnemyWorld::Spawn(
    const StringView    _defName,
    const Vec2          _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    if (!m_pLibrary)
    {
        return {};
    }
    return SpawnByIndex(m_pLibrary->FindIndex(_defName), _feet, _params, _pSnapWorld);
}

EnemyHandle EnemyWorld::SpawnByIndex(
    const int           _defIndex,
    const Vec2          _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    if (!m_pLibrary || _defIndex < 0 || static_cast<size_t>(_defIndex) >= m_pLibrary->GetDefCount())
    {
        SLUG_LOG_WARN("EnemyWorld::Spawn: 알 수 없는 정의 (index {})", _defIndex);
        return {};
    }
    const uint32_t id = m_nextId++;
    Create_(id, _defIndex, _feet, _params, _pSnapWorld);
    return { id };
}

EnemyHandle EnemyWorld::SpawnDeferred(
    const int          _defIndex,
    const Vec2         _feet,
    const SpawnParams& _params)
{
    const uint32_t id = m_nextId++;
    DeferredSpawn  spawn;
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
    const Vec2          _feet,
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
        hurtbox.box     = def.hurtbox.At(e.body.position, e.facing);
        hurtbox.team    = def.team;
        hurtbox.owner   = e.id;
        hurtbox.accepts = AcceptsFor(def.bHuman);
        _combat.AddHurtbox(hurtbox);
    }
}

void EnemyWorld::OnHitEvents(
    const Span<const HitEvent> _events)
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
        const uint32_t id  = event.target & kEnemyIdMask;
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

        if (ops.onHit && ops.onHit(e, def, env, event))
        {
            // 행동이 소비한 피격 (부위 피격 등): 부위 플래시만 건다 (선체 플래시 아님)
            if (def.bHitFlash)
            {
                TriggerPartHitFlash(e, GetPartIndex(event.target));
            }
            continue;
        }

        const int applied = e.health.ApplyDamage(event.damage);
        if (applied > 0)
        {
            e.lastHitKind = event.kind;
        }
        if (e.health.IsDead())
        {
            BeginDying_(e, def, env);
        }
        else if (applied > 0 && def.bHitFlash)
        {
            e.hitFlashTicks = kHitFlashTicks;
        }
    }
}

void EnemyWorld::BeginDying_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.life            = eEnemyLife::Dying;
    _e.body.velocity.x = 0.f;
    PushEvent(MakeEvent(_e, eEnemyEventKind::Died));
    const BehaviorOps& ops = GetBehaviorOps(_e.behavior);
    if (ops.onDeath)
    {
        ops.onDeath(_e, _def, _env);
    }
}

void EnemyWorld::Tick(
    const EnemyContext& _ctx)
{
    if (!m_pLibrary)
    {
        return;
    }
    m_lastCtx     = _ctx;
    m_bPlayerMinX = false;
    m_bPlayerMaxX = false;

    for (Enemy& e: m_enemies)
    {
        if (e.hitFlashTicks > 0)
        {
            --e.hitFlashTicks;
        }
        for (int& ticks: e.partFlashTicks)
        {
            if (ticks > 0)
            {
                --ticks;
            }
        }
    }

    if (_ctx.pProjectiles)
    {
        const Vector<ShotFx> shotFx = _ctx.pProjectiles->TakeShotFx();
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

    FlushFx_(_ctx.pAnims);
    if (_ctx.pAnims)
    {
        m_effects.Update(*_ctx.pAnims, kStepSec);
    }

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
            if (!e.bActive && (def.activation.bAlways || e.body.position.x <= view.max.x + def.activation.marginPx))
            {
                e.bActive = true;
            }
            if (!def.bNoDespawn
                && (e.body.position.x < view.min.x - m_pLibrary->GetGlobals().despawnBehindCameraPx || e.body.position.y > view.max.y + m_pLibrary->GetGlobals().despawnBelowCameraPx))
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
            // 체력이 0 이 된 살아 있는 적 (행동이 직접 체력을 깎은 경우, 예: 캐논 헤드) 은 여기서 사망 처리한다
            if (e.life == eEnemyLife::Alive && e.health.IsDead())
            {
                BeginDying_(e, def, env);
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

    for (const DeferredSpawn& spawn: m_deferred)
    {
        Create_(spawn.id, spawn.defIndex, spawn.feet, spawn.params, nullptr);
    }
    m_deferred.clear();
    RemoveGone_();

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
                    const String name     = ResolveSide(def.clips[c].clip.name, side == 0 ? -1 : 1);
                    m_clipIds[d][c][side] = _anims.Find(name);
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
                const int facing        = side == 0 ? -1 : 1;
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

    for (const Enemy& e: m_enemies)
    {
        if (!e.bVisible || e.life == eEnemyLife::Pending || e.life == eEnemyLife::Gone)
        {
            continue;
        }
        const EnemyDef&    def  = m_pLibrary->Get(e.defIndex);
        const BehaviorOps& ops  = GetBehaviorOps(e.behavior);
        const int          side = SideIndex_(e.facing);
        const Vec2         feet = Lerp_(e.prevFeet, e.body.position, _alpha);

        if (ops.submitParts)
        {
            const EnemyRenderEnv render { _sprites, _anims, m_pSprites, _projectiles, feet, _alpha, kEnemyLayer, HitFlashAmount(e) };
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
        const float       lift = def.bCenterAnchor ? 0.f : static_cast<float>(pClip->GetFootBaseline());   // 중심 앵커 def 는 pivot 을 원점에 둔다
        SpriteDrawOptions options;
        options.flashAmount = HitFlashAmount(e);
        _sprites.Submit(kEnemyLayer, static_cast<int>(feet.y), *pFrame, { feet.x, feet.y - lift }, options);

        if (def.overlayClip >= 0)
        {
            const AnimClip* pOver = _anims.GetOrNull(m_clipIds[static_cast<size_t>(e.defIndex)][static_cast<size_t>(def.overlayClip)][side]);
            if (pOver)
            {
                const AnimFrame* pOverFrame = &pOver->frames[static_cast<size_t>(BakedFrameAt_(*pOver, static_cast<int64_t>(e.clipTicks) * kAnimUnitsPerTick))];
                if (!pOverFrame->IsEmpty())
                {
                    _sprites.Submit(kOverlayLayer, static_cast<int>(feet.y), *pOverFrame, { feet.x + def.overlayOffset.x, feet.y + def.overlayOffset.y - lift }, options);
                }
            }
        }
    }

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
        const size_t    pIdx      = static_cast<size_t>(projIndex);
        const bool      bRoll     = shot.params.rollSpeed > 0.f;
        const int       side      = bRoll ? (shot.params.rollDirX < 0.f ? 0 : 1) : (shot.velocity.x < 0.f ? 0 : 1);
        const int       landAge   = shot.ageTicks - shot.landedAge;
        const int       landTicks = m_pLibrary->GetProjectile(projIndex).land.GetDurationTicks();
        const bool      bLand     = bRoll && shot.bLanded && landAge < landTicks && _anims.IsValid(m_projLandIds[pIdx][side]);
        const bool      bBegin    = !bLand && (shot.ageTicks < shot.params.startDelayTicks || (bRoll && !shot.bLanded)) && _anims.IsValid(m_projBeginIds[pIdx][side]);
        const AnimId    clipId    = bLand ? m_projLandIds[pIdx][side] : (bBegin ? m_projBeginIds[pIdx][side] : m_projFlyIds[pIdx][side]);
        const AnimClip* pClip     = _anims.GetOrNull(clipId);
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
        const Vec2 pos = Lerp_(shot.renderPrev, shot.position, _alpha);
        _sprites.Submit(kShotLayer, static_cast<int>(pos.y), *pFrame, pos);
    }

    m_effects.Submit(_anims, _sprites);
}

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

Vector<Aabb> EnemyWorld::GetSolidBoxes() const
{
    Vector<Aabb> boxes;
    if (!m_pLibrary)
    {
        return boxes;
    }
    for (const Enemy& e: m_enemies)
    {
        if (e.life != eEnemyLife::Alive)
        {
            continue;
        }
        const EnemyDef& def = m_pLibrary->Get(e.defIndex);
        if (!def.solid.IsEmpty())
        {
            boxes.push_back(def.solid.At(e.body.position, e.facing));
        }
    }
    return boxes;
}

void EnemyWorld::SendCommand(
    const EnemyHandle _handle,
    const StringView  _command)
{
    if (!m_pLibrary)
    {
        return;
    }
    const size_t idx = FindIndex_(ResolveHandle(_handle).id);
    if (idx >= m_enemies.size() || m_enemies[idx].life != eEnemyLife::Alive)
    {
        return;
    }
    Enemy&             e   = m_enemies[idx];
    const BehaviorOps& ops = GetBehaviorOps(e.behavior);
    if (ops.onCommand)
    {
        ops.onCommand(e, m_pLibrary->Get(e.defIndex), _command);
    }
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

float EnemyWorld::TakeCameraScroll()
{
    const float dx = m_cameraScroll;
    m_cameraScroll = 0.f;
    return dx;
}

void EnemyWorld::RequestCameraShake(
    const float _amplitude,
    const float _seconds)
{
    m_shakeAmplitude = std::max(m_shakeAmplitude, _amplitude);
    m_shakeSeconds   = std::max(m_shakeSeconds, _seconds);
}

void EnemyWorld::TakeCameraShake(
    float& _amplitude,
    float& _seconds)
{
    _amplitude       = m_shakeAmplitude;
    _seconds         = m_shakeSeconds;
    m_shakeAmplitude = 0.f;
    m_shakeSeconds   = 0.f;
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

void EnemyWorld::RequestPlayerMaxX(
    const float _maxX)
{
    m_playerMaxX  = m_bPlayerMaxX ? std::min(m_playerMaxX, _maxX) : _maxX;
    m_bPlayerMaxX = true;
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
    const StringView _defName) const
{
    EnemyFilter filter;
    filter.defIndex     = m_pLibrary ? m_pLibrary->FindIndex(_defName) : -1;
    filter.bHostileOnly = false;
    if (filter.defIndex < 0)
    {
        return 0;
    }
    return CountAlive(filter);
}

Vector<EnemyEvent> EnemyWorld::TakeEvents()
{
    Vector<EnemyEvent> out = std::move(m_events);
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
        Enemy&     e       = m_enemies[i];
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
            continue;
        }
        if (e.life == eEnemyLife::Alive)
        {
            EnemyEvent event = MakeEvent(e, eEnemyEventKind::Died);
            event.score      = _bAwardScore ? m_pLibrary->Get(e.defIndex).score : 0;
            PushEvent(std::move(event));
            continue;
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
    uint64_t hash = 14'695'981'039'346'656'037ull;
    for (const Enemy& e: m_enemies)
    {
        MixHash_(hash, static_cast<int64_t>(e.id));
        MixHash_(hash, e.defIndex);
        MixHash_(hash, static_cast<int64_t>(e.life));
        MixHash_(hash, e.state);
        MixHash_(hash, e.stateTicks);
        MixHash_(hash, e.clipTicks);
        MixHash_(hash, e.clip);
        MixHash_(hash, e.health.hp);
        MixHash_(hash, e.facing);
        MixHash_(hash, e.counter);
        MixHash_(hash, e.aux);
        MixHash_(hash, e.aux2);
        MixHash_(hash, e.aux3);
        MixHash_(hash, e.aux4);
        MixHash_(hash, e.shotsFired);
        MixHash_(hash, static_cast<int64_t>(e.body.position.x * 16.f));
        MixHash_(hash, static_cast<int64_t>(e.body.position.y * 16.f));
    }
    MixHash_(hash, static_cast<int64_t>(m_events.size()));
    return hash;
}

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
    if (m_pLibrary && _e.defIndex >= 0)
    {
        const EnemyDef& def = m_pLibrary->Get(_e.defIndex);
        event.defName       = def.name;
        event.score         = _kind == eEnemyEventKind::Died ? def.score : 0;
    }
    return event;
}

const EnemyDefLibrary* EnemyWorld::GetLibrary() const
{
    return m_pLibrary;
}

void EnemyWorld::ReseedRng(
    const uint64_t _seed)
{
    m_rng.Reseed(_seed);
}

void EnemyWorld::SetSpriteLibrary(
    const SpriteLibrary* _pSprites)
{
    m_pSprites = _pSprites;
}

bool EnemyWorld::HasPlayerMinX() const
{
    return m_bPlayerMinX;
}

float EnemyWorld::GetPlayerMinX() const
{
    return m_playerMinX;
}

bool EnemyWorld::HasPlayerMaxX() const
{
    return m_bPlayerMaxX;
}

float EnemyWorld::GetPlayerMaxX() const
{
    return m_playerMaxX;
}

void EnemyWorld::RequestCameraScroll(
    const float _dx)
{
    m_cameraScroll += _dx;
}

Span<const Enemy> EnemyWorld::GetEnemies() const
{
    return m_enemies;
}

size_t EnemyWorld::GetCount() const
{
    return m_enemies.size();
}

}   // namespace slug

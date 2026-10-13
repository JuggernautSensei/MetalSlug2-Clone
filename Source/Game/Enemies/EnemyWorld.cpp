#include "Game/Enemies/EnemyWorld.h"

#include <algorithm>
#include <cstring>

#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/BigShieeCfg.h"
#include "Game/Enemies/EnemyCommon.h"
#include "World/Constants.h"

constexpr float kSpawnSnapPx = 16.f;

namespace
{
[[nodiscard]] int FrameAt_(
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

// +y (아래) 를 향한 그림을 속도 방향에 맞추는 시계 방향 회전 (도, 아래 = 0)
[[nodiscard]] float RotationDeg_(
    const VECTOR2 _velocity)
{
    return std::atan2(-_velocity.x, _velocity.y) * kRad2Deg;
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
    Fnv1a&        _hasher,
    const int64_t _value)
{
    _hasher.Mix(_value);
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
    JUG_ASSERT(_pLibrary != nullptr, "EnemyWorld 는 정의 라이브러리 없이 만들 수 없다.\n");
}

void EnemyWorld::SetLibrary(
    const EnemyDefLibrary* _pLibrary)
{
    JUG_ASSERT(_pLibrary != nullptr, "정의 라이브러리는 null 일 수 없다.\n");
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
    m_play = {};
    m_events.clear();
    m_pendingFx.clear();
    m_deferred.clear();
    m_successors.clear();
    m_effects.Clear();
    m_rng.Reseed(_seed);
    m_nextId       = kEnemyOwnerBase;
    m_cameraScroll = 0.f;
}

int GetEnemyDefaultZ(
    const eEnemyBehavior _behavior)
{
    return _behavior == eEnemyBehavior::BigShiee ? kBigShieeDepthHull : kEnemyDepth;
}

EnemyHandle EnemyWorld::Spawn(
    const StringView    _defName,
    const VECTOR2       _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    return SpawnByIndex(m_pLibrary->FindIndex(_defName), _feet, _params, _pSnapWorld);
}

EnemyHandle EnemyWorld::SpawnByIndex(
    const int           _defIndex,
    const VECTOR2       _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    if (_defIndex < 0 || static_cast<size_t>(_defIndex) >= m_pLibrary->GetDefCount())
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
    const VECTOR2      _feet,
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
    const VECTOR2       _feet,
    const SpawnParams&  _params,
    const PhysicsWorld* _pSnapWorld)
{
    const EnemyDef& def = m_pLibrary->Get(_defIndex);

    Enemy e;
    e.id         = _id;
    e.defIndex   = _defIndex;
    e.behavior   = def.behavior;
    e.delayTicks = SecToTicks(_params.delaySec);
    e.z          = _params.z != kDefaultZ ? _params.z : GetEnemyDefaultZ(def.behavior);
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
    e.climbEnd  = _params.climbEnd;
    e.roofSpan  = _params.roofSpan;
    e.clip      = -1;
    e.bVisible  = true;

    m_enemies.push_back(e);
}

void EnemyWorld::RegisterHurtboxes(
    CombatSystem& _combat) const
{
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
        }
        else
        {
            Hurtbox hurtbox;
            hurtbox.box     = def.hurtbox.At(e.body.position, e.facing);
            hurtbox.team    = def.team;
            hurtbox.owner   = e.id;
            hurtbox.accepts = AcceptsFor(def.bHuman);
            _combat.AddHurtbox(hurtbox);
        }
        for (const ExtraBoxDef& box: def.hitboxes)
        {
            if (box.kind != eExtraBoxKind::Hurt)
            {
                continue;
            }
            Hurtbox extra;
            extra.box     = box.box.At(e.body.position, e.facing);
            extra.team    = def.team;
            extra.owner   = e.id;
            extra.accepts = AcceptsFor(def.bHuman);
            _combat.AddHurtbox(extra);
        }
    }
}

void EnemyWorld::OnHitEvents(
    const Span<const HitEvent> _events)
{
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
        EnemyEnv           env { *this, m_lastCtx, *m_pLibrary, MakePhys_(def), e.z - GetEnemyDefaultZ(e.behavior) };

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
            (void)StartHitFlash(e.hitFlashTicks);
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

namespace
{
void ReleaseExtraBoxHit_(
    Enemy&    _e,
    EnemyEnv& _env)
{
    if (_e.boxHitId != kNoHitId && _env.ctx.pCombat)
    {
        _env.ctx.pCombat->ReleaseHitId(_e.boxHitId);
    }
    _e.boxHitId = kNoHitId;
}

void TickExtraBoxes_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    if (_def.hitboxes.empty() || !_env.ctx.pCombat)
    {
        return;
    }
    int frame = 0;
    if (_e.clip >= 0 && _e.clip < static_cast<int>(_def.clips.size()))
    {
        const ClipDef& clip = _def.GetClip(_e.clip);
        if (clip.frameUnits > 0)
        {
            frame = static_cast<int>((static_cast<int64_t>(_e.clipTicks) * kAnimUnitsPerTick) / clip.frameUnits);
            if (clip.bLoop && clip.frames > 0)
            {
                frame %= clip.frames;
            }
        }
    }
    bool bActive = false;
    for (const ExtraBoxDef& box: _def.hitboxes)
    {
        if (box.kind != eExtraBoxKind::Attack || (box.frameTo >= 0 && (frame < box.frameFrom || frame > box.frameTo)))
        {
            continue;
        }
        if (_e.boxHitId == kNoHitId)
        {
            _e.boxHitId = _env.ctx.pCombat->NewHitId();
        }
        Hitbox hitbox;
        hitbox.box     = box.box.At(_e.body.position, _e.facing);
        hitbox.team    = eTeam::Enemy;
        hitbox.kind    = eHitKind::Melee;
        hitbox.damage  = box.damage;
        hitbox.bPierce = false;
        hitbox.owner   = _e.id;
        hitbox.id      = _e.boxHitId;
        _env.ctx.pCombat->AddHitbox(hitbox);
        bActive = true;
    }
    if (!bActive)
    {
        ReleaseExtraBoxHit_(_e, _env);
    }
}
}   // namespace

void EnemyWorld::Tick(
    const EnemyContext& _ctx)
{
    m_lastCtx = _ctx;

    for (Enemy& e: m_enemies)
    {
        TickHitFlash(e.hitFlashTicks);
        for (int& ticks: e.partFlashTicks)
        {
            TickHitFlash(ticks);
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
            const ProjectileDef& projectileDef = m_pLibrary->GetProjectile(projIndex);
            const FxDef&         explosion     = projectileDef.explosion;
            if (!projectileDef.explosionSfx.empty() && m_sounds.size() < 64)
            {
                m_sounds.push_back(projectileDef.explosionSfx);
            }
            if (!explosion.clip.empty())
            {
                m_effects.Spawn(*_ctx.pAnims, explosion.clip, { fx.position.x + explosion.offset.x, fx.position.y + explosion.offset.y }, 58 + fx.zOffset);
            }
        }
    }

    if (_ctx.pProjectiles && _ctx.pAnims)
    {
        SpawnShotTrails_(*_ctx.pProjectiles, *_ctx.pAnims);
    }

    FlushFx_(_ctx.pAnims);
    if (_ctx.pAnims)
    {
        m_effects.Update(*_ctx.pAnims, kStepSec, _ctx.cameraView.max.y);
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
        EnemyEnv           env { *this, m_lastCtx, *m_pLibrary, MakePhys_(def), e.z - GetEnemyDefaultZ(e.behavior) };

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
            if (m_play.id == e.id)
            {
                ++m_play.ticks;
                e.clip      = m_play.clip;
                e.clipTicks = m_play.ticks;
                if (m_play.ticks == m_play.fireTick)
                {
                    (void)FireWeapon(e, def, env, m_play.weapon, { static_cast<float>(e.facing < 0 ? -1 : 1), 0.f });
                }
                if (m_play.ticks >= m_play.endTick)
                {
                    m_play = {};
                }
            }
            TickWeaponEffects(e, def, env);
            TickExtraBoxes_(e, def, env);
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
            ReleaseExtraBoxHit_(e, env);
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

void EnemyWorld::SpawnShotTrails_(
    const ProjectileSystem& _projectiles,
    const AnimationLibrary& _anims)
{
    for (const Shot& shot: _projectiles.GetShots())
    {
        const int projIndex = static_cast<int>(shot.params.visual) - 1;
        if (!shot.bAlive || projIndex < 0 || static_cast<size_t>(projIndex) >= m_pLibrary->GetProjectileCount())
        {
            continue;
        }
        const ProjectileDef& proj = m_pLibrary->GetProjectile(projIndex);
        if (proj.trailTicks <= 0 || proj.trail.clip.empty() || shot.ageTicks <= shot.params.startDelayTicks || shot.ageTicks % proj.trailTicks != 0)
        {
            continue;
        }
        const int   facing = shot.velocity.x < 0.f ? -1 : 1;
        const float sign   = static_cast<float>(facing);
        VECTOR2     offset = { proj.trail.offset.x * sign * proj.scale, proj.trail.offset.y * proj.scale };
        if (proj.bRotate)
        {
            const float rad = RotationDeg_(shot.velocity) * kDeg2Rad;
            const float ox  = proj.trail.offset.x * proj.scale;
            const float oy  = proj.trail.offset.y * proj.scale;
            offset          = { ox * std::cos(rad) - oy * std::sin(rad), ox * std::sin(rad) + oy * std::cos(rad) };
        }
        m_effects.Spawn(_anims, ResolveSide(proj.trail.clip, facing), { shot.position.x + offset.x, shot.position.y + offset.y }, kTrailDepth + ZDelta(shot.params.z, kShotDepth));
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
        if (request.bDebris)
        {
            m_effects.SpawnDebris(*_pAnims, request.clip, request.position, request.depth, request.velocity, request.bFlipX);
        }
        else
        {
            m_effects.Spawn(*_pAnims, request.clip, request.position, request.depth, request.bFlipX);
        }
    }
    m_pendingFx.resize(keep);
}

void EnemyWorld::BindAnims(
    const AnimationLibrary& _anims)
{
    m_clipIds.clear();
    m_projFlyIds.clear();
    m_projBeginIds.clear();
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
    m_boundClipCount = _anims.GetClipCount();
    m_bAnimsBound    = true;
}

void EnemyWorld::Submit(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const ProjectileSystem& _projectiles,
    const float             _alpha)
{
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
        const VECTOR2      feet = Lerp(e.prevFeet, e.body.position, _alpha);

        if (ops.submitParts)
        {
            const int            zOffset = e.z - GetEnemyDefaultZ(e.behavior);
            const EnemyRenderEnv render { _sprites, _anims, m_pSprites, _projectiles, feet, _alpha, kEnemyDepth, HitFlashAmount(e), zOffset };
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
        const AnimFrame* pFrame = &pClip->frames[static_cast<size_t>(FrameAt_(*pClip, static_cast<int64_t>(e.clipTicks) * kAnimUnitsPerTick))];
        if (ops.bodyFrame)
        {
            if (const AnimFrame* pPose = ops.bodyFrame(e, def, _anims))
            {
                pFrame = pPose;
            }
        }
        if (pFrame->IsEmpty())
        {
            continue;
        }
        const float       lift = def.bCenterAnchor ? 0.f : static_cast<float>(pClip->GetFootBaseline());
        SpriteDrawOptions options;
        options.flashAmount = HitFlashAmount(e);
        options.bFlipX      = def.ShouldFlipArt(e.facing);
        _sprites.Submit(def.partBody.depth + e.z - GetEnemyDefaultZ(e.behavior), static_cast<int>(feet.y) + def.partBody.sort, *pFrame, { feet.x, feet.y - lift }, options);

        // 낙하산은 살아 있는 동안만 그린다. 사망 순간 즉시 사라지고 착지 변신 FX 는 FlyingBzk 가 맡는다.
        if (def.overlayClip >= 0 && e.life == eEnemyLife::Alive)
        {
            const AnimClip* pOver = _anims.GetOrNull(m_clipIds[static_cast<size_t>(e.defIndex)][static_cast<size_t>(def.overlayClip)][side]);
            if (pOver)
            {
                const AnimFrame* pOverFrame = &pOver->frames[static_cast<size_t>(FrameAt_(*pOver, static_cast<int64_t>(e.clipTicks) * kAnimUnitsPerTick))];
                if (!pOverFrame->IsEmpty())
                {
                    _sprites.Submit(def.partOverlay.depth + e.z - GetEnemyDefaultZ(e.behavior), static_cast<int>(feet.y) + def.partOverlay.sort, *pOverFrame, { feet.x + def.overlayOffset.x, feet.y + def.overlayOffset.y - lift }, options);
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

        int ticks = Max(0, shot.ageTicks - shot.params.startDelayTicks);
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
            ticks = Max(0, landAge - landTicks);
        }
        const AnimFrame* pFrame = &pClip->frames[static_cast<size_t>(FrameAt_(*pClip, static_cast<int64_t>(ticks) * kAnimUnitsPerTick))];
        if (pFrame->IsEmpty())
        {
            continue;
        }
        const VECTOR2     pos = Lerp(shot.renderPrev, shot.position, _alpha);
        SpriteDrawOptions options;
        options.scale = m_pLibrary->GetProjectile(projIndex).scale;
        if (m_pLibrary->GetProjectile(projIndex).bRotate)
        {
            options.rotationDeg = RotationDeg_(shot.velocity);
        }
        _sprites.Submit(ResolveZ(shot.params.z, kShotDepth), static_cast<int>(pos.y), *pFrame, pos, options);
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

bool EnemyWorld::FireWeaponNow(
    const EnemyHandle _handle,
    const int         _weapon)
{
    const size_t idx = FindIndex_(_handle.id);
    if (idx >= m_enemies.size())
    {
        return false;
    }
    Enemy&          e   = m_enemies[idx];
    const EnemyDef& def = m_pLibrary->Get(e.defIndex);
    if (_weapon < 0 || _weapon >= static_cast<int>(def.weapons.size()))
    {
        return false;
    }
    EnemyEnv env { *this, m_lastCtx, *m_pLibrary, MakePhys_(def), e.z - GetEnemyDefaultZ(e.behavior) };
    return FireWeapon(e, def, env, _weapon, { static_cast<float>(e.facing < 0 ? -1 : 1), 0.f });
}

bool EnemyWorld::PlayMotion(
    const EnemyHandle _handle,
    const int         _weapon,
    const int         _fireFrame)
{
    const size_t idx = FindIndex_(_handle.id);
    if (idx >= m_enemies.size())
    {
        return false;
    }
    const Enemy&    e   = m_enemies[idx];
    const EnemyDef& def = m_pLibrary->Get(e.defIndex);
    if (_weapon < 0 || _weapon >= static_cast<int>(def.weapons.size()))
    {
        return false;
    }
    const int clip = def.FindClip(def.weapons[static_cast<size_t>(_weapon)].motionClip);
    if (clip < 0)
    {
        return false;
    }
    const ClipDef& clipDef = def.GetClip(clip);
    m_play.id              = e.id;
    m_play.weapon          = _weapon;
    m_play.clip            = clip;
    m_play.ticks           = 0;
    m_play.fireTick        = clipDef.TickOfFrame(_fireFrame);
    m_play.endTick         = clipDef.GetDurationTicks() + 1;
    return true;
}

const Enemy* EnemyWorld::Get(
    const EnemyHandle _handle) const
{
    const size_t idx = FindIndex_(_handle.id);
    return idx < m_enemies.size() ? &m_enemies[idx] : nullptr;
}

bool EnemyWorld::SetZ(
    const EnemyHandle _handle,
    const int         _z)
{
    const size_t idx = FindIndex_(_handle.id);
    if (idx >= m_enemies.size())
    {
        return false;
    }
    m_enemies[idx].z = _z;
    return true;
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
        if (def.behavior == eEnemyBehavior::Bus && def.bus.landingW > 0.f)
        {
            // 버스 계단 위 승강대: 계단 병사가 올라서는 낮은 벽 (윗면 = stairsTop.y, 지면까지)
            const VECTOR2 top = { e.body.position.x + def.bus.stairsTop.x, e.body.position.y + def.bus.stairsTop.y };
            boxes.push_back(Aabb::FromMinMax({ top.x - def.bus.landingW * 0.5f, top.y }, { top.x + def.bus.landingW * 0.5f, e.body.position.y }));
        }
        if (def.behavior == eEnemyBehavior::Bus && def.bus.roofTop.y != 0.f && def.bus.roofSpan.y > def.bus.roofSpan.x)
        {
            constexpr float kRoofThickness = 12.f;
            const float     y              = e.body.position.y + def.bus.roofTop.y;
            boxes.push_back(Aabb::FromMinMax({ e.body.position.x + def.bus.roofSpan.x, y }, { e.body.position.x + def.bus.roofSpan.y, y + kRoofThickness }));
        }
    }
    return boxes;
}

void EnemyWorld::SendCommand(
    const EnemyHandle _handle,
    const StringView  _command)
{
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
    if (pEnemy->life != eEnemyLife::Dying)
    {
        return false;
    }
    const BehaviorOps& ops = GetBehaviorOps(pEnemy->behavior);
    return ops.deathDone ? ops.deathDone(*pEnemy, m_pLibrary->Get(pEnemy->defIndex)) : false;
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
        if (_filter.bHostileOnly && m_pLibrary->Get(e.defIndex).team != eTeam::Enemy)
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
    filter.defIndex     = m_pLibrary->FindIndex(_defName);
    filter.bHostileOnly = false;
    if (filter.defIndex < 0)
    {
        return 0;
    }
    return CountAlive(filter);
}

Vector<String> EnemyWorld::TakeSounds()
{
    Vector<String> out = std::move(m_sounds);
    m_sounds.clear();
    return out;
}

Vector<EnemyEvent> EnemyWorld::TakeEvents()
{
    Vector<EnemyEvent> out = std::move(m_events);
    m_events.clear();
    return out;
}

void EnemyWorld::RefreshBodies(
    const int     _defIndex,
    const VECTOR2 _size)
{
    for (Enemy& e: m_enemies)
    {
        if (e.defIndex == _defIndex)
        {
            e.body.size = _size;
        }
    }
}

void EnemyWorld::ClearMonsters(
    const bool _bIncludeNpc)
{
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

uint64_t EnemyWorld::HashState() const
{
    Fnv1a hasher;
    for (const Enemy& e: m_enemies)
    {
        MixHash_(hasher, static_cast<int64_t>(e.id));
        MixHash_(hasher, e.defIndex);
        MixHash_(hasher, static_cast<int64_t>(e.life));
        MixHash_(hasher, e.state);
        MixHash_(hasher, e.stateTicks);
        MixHash_(hasher, e.clipTicks);
        MixHash_(hasher, e.clip);
        MixHash_(hasher, e.health.hp);
        MixHash_(hasher, e.facing);
        MixHash_(hasher, e.counter);
        MixHash_(hasher, e.aux);
        MixHash_(hasher, e.aux2);
        MixHash_(hasher, e.aux3);
        MixHash_(hasher, e.aux4);
        MixHash_(hasher, e.shotsFired);
        MixHash_(hasher, static_cast<int64_t>(e.body.position.x * 16.f));
        MixHash_(hasher, static_cast<int64_t>(e.body.position.y * 16.f));
    }
    MixHash_(hasher, static_cast<int64_t>(m_events.size()));
    return hasher.Finalize();
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
    if (_e.defIndex >= 0)
    {
        const EnemyDef& def = m_pLibrary->Get(_e.defIndex);
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

void EnemyWorld::RequestCameraScroll(
    const float _dx)
{
    m_cameraScroll += _dx;
}

Span<const Enemy> EnemyWorld::GetEnemies() const
{
    return m_enemies;
}

const EffectList& EnemyWorld::GetEffects() const
{
    return m_effects;
}

size_t EnemyWorld::GetCount() const
{
    return m_enemies.size();
}

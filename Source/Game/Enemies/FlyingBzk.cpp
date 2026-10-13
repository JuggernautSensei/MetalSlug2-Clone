#include "Game/Enemies/FlyingBzkEnemy.h"

#include <cmath>
#include <format>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

void FlyingBzkEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    m_driftDirection    = -1;
    Go_(_def, eFlyingState::Descend);
}

void FlyingBzkEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&            _e     = GetMutableState();
    const FlyingBzkConfig& config = _def.flyingBzk;
    _e.body.velocity              = { 0.f, 0.f };
    if (!_e.bActive)
    {
        return;
    }

    if (++m_driftTicks >= config.driftFlipTicks)
    {
        m_driftTicks = 0;
        if (Roll(_env, 100) < config.flipPercent)
        {
            m_driftDirection = -m_driftDirection;
        }
    }
    _e.body.position.x += static_cast<float>(m_driftDirection) * config.driftSpeed * kStepSec;
    const float prevY = _e.body.position.y;
    float       step  = config.descendSpeed * kStepSec;
    if (config.fallGravity > 0.f)
    {
        const float speed = Min(config.descendSpeed + config.fallGravity * static_cast<float>(m_fallTicks) * kStepSec, config.fallMaxSpeed);
        ++m_fallTicks;
        step = speed * kStepSec;
    }
    _e.body.position.y += step;

    {
        // 가속 낙하는 한 틱에 지면을 지나칠 수 있어 이전 위치부터 이동 거리만큼 훑는다
        Body  probe     = _e.body;
        float probeDist = config.landProbe;
        if (config.fallGravity > 0.f)
        {
            probe.position.y = prevY;
            probeDist        = step + config.landProbe;
        }
        if (Physics::SnapToGround(probe, _env.ctx.physics, probeDist))
        {
            const int landedIndex = _env.lib.FindIndex(config.landedDef);
            if (landedIndex >= 0)
            {
                SpawnParams params;
                params.facing            = _e.facing;
                params.drop              = _e.drop;
                params.userTag           = _e.userTag;
                params.bActivated        = true;
                const EnemyHandle handle = _env.world.SpawnDeferred(landedIndex, probe.position, params);
                _env.world.NoteSuccessor(EnemyHandle { _e.id }, handle);

                EnemyEvent event = _env.world.MakeEvent(_e, eEnemyEventKind::Transformed);
                event.other      = handle;
                event.pos        = probe.position;
                _env.world.PushEvent(std::move(event));

                RequestFx(_env, config.paraDestroy, probe.position, _e.facing);
                _e.bTransformed = true;
                _e.life         = eEnemyLife::Gone;
                return;
            }
        }
    }

    FacePlayer(_e, _env);
    UpdatePosture_(config, _env);

    if (_e.state == static_cast<int>(eFlyingState::Descend))
    {
        SetClip(_e, config.clipReady[m_aimIndex]);
        ++m_fireTicks;
        if (m_fireTicks >= config.fireIntervalTicks && m_shotsFired < config.maxShots && IsOnScreen_(_env) && _env.ctx.player.bAlive)
        {
            m_fireTicks = 0;
            Go_(_def, eFlyingState::Shot);
        }
    }
    else
    {
        if (ClipReachedFrame(_e, _def, config.fireFrame))
        {
            const float   rad = config.fireDeg[m_aimIndex] * kDeg2Rad;
            const VECTOR2 dir = { std::cos(rad) * static_cast<float>(_e.facing), std::sin(rad) };
            if (FireWeapon(_e, _def, _env, config.weaponRocket, dir, { dir.x * config.muzzleDist, dir.y * config.muzzleDist }))
            {
                ++m_shotsFired;
            }
        }
        if (ClipFinished(_e, _def))
        {
            Go_(_def, eFlyingState::Descend);
        }
    }
}

void FlyingBzkEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    _e.bGrounded   = false;
    BeginSoldierDeath(_e, _def, _env);
}

void FlyingBzkEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickSoldierDeath(GetMutableState(), _def, _env);
}

void FlyingBzkEnemy::RegisterHurtboxes(
    const EnemyDef& _def,
    CombatSystem&   _combat) const
{
    const EnemyState& _e = GetState();
    if (_def.flyingBzk.bNoHurtbox)
    {
        return;
    }
    Hurtbox hurtbox;
    hurtbox.box     = _def.hurtbox.At(_e.body.position, _e.facing);
    hurtbox.team    = _def.team;
    hurtbox.owner   = _e.id;
    hurtbox.accepts = AcceptsFor(_def.bHuman);
    _combat.AddHurtbox(hurtbox);
}

void ParseFlyingBzkConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    FlyingBzkConfig& config = _def.flyingBzk;

    config.descendSpeed      = _r.Num("descendSpeed");
    config.driftSpeed        = _r.Num("driftSpeed");
    config.driftFlipTicks    = _r.Ticks("driftFlipSec");
    config.flipPercent       = _r.Int("flipPercent");
    config.fireIntervalTicks = _r.Ticks("fireIntervalSec");
    config.maxShots          = _r.Int("maxShots");
    config.fireFrame         = _r.Int("fireFrame");
    (void)_r.NumArray("postureDeg", config.postureDeg);
    (void)_r.NumArray("fireDeg", config.fireDeg);
    config.windowDeg    = _r.Num("windowDeg");
    config.muzzleBase   = _r.ReadVec2("muzzleBase", config.muzzleBase);
    config.muzzleDist   = _r.Num("muzzleDist");
    config.landProbe    = _r.Num("landProbe");
    config.landedDef    = _r.Str("landedDef");
    config.fallGravity  = _r.Num("fallGravity", config.fallGravity);
    config.fallMaxSpeed = _r.Num("fallMaxSpeed", config.fallMaxSpeed);
    config.bNoHurtbox   = _r.Bool("noHurtbox", false);

    for (int i = 0; i < 5; ++i)
    {
        config.clipReady[i] = _r.Clip(std::format("ready{}", i));
        config.clipShot[i]  = _r.Clip(std::format("shot{}", i));
    }
    config.weaponRocket = _r.Weapon("rocket");

    if (_def.overlayClip >= 0)
    {
        config.paraOffset = _def.overlayOffset;
    }
    config.paraDestroy.clip   = _r.Str("paraDestroyFx");
    config.paraDestroy.offset = _r.ReadVec2("paraDestroyOffset", config.paraOffset);
    config.paraDestroy.depth  = 20;
}

[[nodiscard]] bool FlyingBzkEnemy::IsOnScreen_(
    const EnemyEnv& _env) const
{
    const EnemyState& _e     = GetState();
    const VECTOR2     center = { _e.body.position.x, _e.body.position.y - _e.body.size.y * 0.5f };
    return _env.ctx.cameraView.Contains(center);
}

void FlyingBzkEnemy::UpdatePosture_(
    const FlyingBzkConfig& _config,
    const EnemyEnv&        _env)
{
    EnemyState& _e  = GetMutableState();
    const float dx  = Abs(_env.ctx.player.center.x - _e.body.position.x);
    const float dy  = _env.ctx.player.center.y - (_e.body.position.y + _config.muzzleBase.y);
    const float deg = dy > 0.f ? ATan2(dy, Max(dx, 0.001f)) * kRad2Deg : 0.f;
    for (int i = 0; i < 5; ++i)
    {
        if (Abs(deg - _config.postureDeg[i]) <= _config.windowDeg)
        {
            m_aimIndex = i;
            break;
        }
    }
}

void FlyingBzkEnemy::Go_(
    const EnemyDef&    _def,
    const eFlyingState _state)
{
    EnemyState&            _e     = GetMutableState();
    const FlyingBzkConfig& config = _def.flyingBzk;
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, _state == eFlyingState::Shot ? config.clipShot[m_aimIndex] : config.clipReady[m_aimIndex], true);
}

void FlyingBzkEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, m_shotsFired, m_fireTicks, m_aimIndex, m_driftDirection, m_driftTicks);
}

#include <cmath>
#include <format>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

enum class eFlyingState
{
    Descend,
    Shot
};

namespace
{
[[nodiscard]] bool IsOnScreen_(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    const Vec2 center = { _e.body.position.x, _e.body.position.y - _e.body.size.y * 0.5f };
    return _env.ctx.cameraView.Contains(center);
}

void UpdatePosture_(
    Enemy&              _e,
    const FlyingBzkCfg& _cfg,
    const EnemyEnv&     _env)
{
    const float dx  = std::abs(_env.ctx.player.center.x - _e.body.position.x);
    const float dy  = _env.ctx.player.center.y - (_e.body.position.y + _cfg.muzzleBase.y);
    const float deg = dy > 0.f ? std::atan2(dy, Max(dx, 0.001f)) * kRad2Deg : 0.f;
    for (int i = 0; i < 5; ++i)
    {
        if (std::abs(deg - _cfg.postureDeg[i]) <= _cfg.windowDeg)
        {
            _e.aux2 = i;
            break;
        }
    }
}

void Go_(
    Enemy&             _e,
    const EnemyDef&    _def,
    const eFlyingState _state)
{
    const FlyingBzkCfg& cfg = _def.flyingBzk;
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, _state == eFlyingState::Shot ? cfg.clipShot[_e.aux2] : cfg.clipReady[_e.aux2], true);
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&)
{
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    _e.aux3             = -1;
    Go_(_e, _def, eFlyingState::Descend);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const FlyingBzkCfg& cfg = _def.flyingBzk;
    _e.body.velocity        = { 0.f, 0.f };
    if (!_e.bActive)
    {
        return;
    }

    if (++_e.aux4 >= cfg.driftFlipTicks)
    {
        _e.aux4 = 0;
        if (Roll(_env, 100) < cfg.flipPercent)
        {
            _e.aux3 = -_e.aux3;
        }
    }
    _e.body.position.x += static_cast<float>(_e.aux3) * cfg.driftSpeed * kStepSec;
    const float prevY = _e.body.position.y;
    float       step  = cfg.descendSpeed * kStepSec;
    if (cfg.fallGravity > 0.f)
    {
        const float speed = Min(cfg.descendSpeed + cfg.fallGravity * static_cast<float>(_e.ext[0]) * kStepSec, cfg.fallMaxSpeed);
        ++_e.ext[0];
        step = speed * kStepSec;
    }
    _e.body.position.y += step;

    {
        // 가속 낙하는 한 틱에 지면을 지나칠 수 있어 이전 위치부터 이동 거리만큼 훑는다
        Body  probe     = _e.body;
        float probeDist = cfg.landProbe;
        if (cfg.fallGravity > 0.f)
        {
            probe.position.y = prevY;
            probeDist        = step + cfg.landProbe;
        }
        if (Physics::SnapToGround(probe, _env.ctx.physics, probeDist))
        {
            const int landedIndex = _env.lib.FindIndex(cfg.landedDef);
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

                RequestFx(_env, cfg.paraDestroy, probe.position, _e.facing);
                _e.bTransformed = true;
                _e.life         = eEnemyLife::Gone;
                return;
            }
        }
    }

    FacePlayer(_e, _env);
    UpdatePosture_(_e, cfg, _env);

    if (_e.state == static_cast<int>(eFlyingState::Descend))
    {
        SetClip(_e, cfg.clipReady[_e.aux2]);
        ++_e.aux;
        if (_e.aux >= cfg.fireIntervalTicks && _e.counter < cfg.maxShots && IsOnScreen_(_e, _env) && _env.ctx.player.bAlive)
        {
            _e.aux = 0;
            Go_(_e, _def, eFlyingState::Shot);
        }
    }
    else
    {
        if (ClipReachedFrame(_e, _def, cfg.fireFrame))
        {
            const float rad = cfg.fireDeg[_e.aux2] * kDeg2Rad;
            const Vec2  dir = { std::cos(rad) * static_cast<float>(_e.facing), std::sin(rad) };
            if (FireWeapon(_e, _def, _env, cfg.weaponRocket, dir, { dir.x * cfg.muzzleDist, dir.y * cfg.muzzleDist }))
            {
                ++_e.counter;
            }
        }
        if (ClipFinished(_e, _def))
        {
            Go_(_e, _def, eFlyingState::Descend);
        }
    }
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.bGrounded = false;
    BeginSoldierDeath(_e, _def, _env);
}

// 낙하 인질 (bNoHurtbox) 은 판정 상자를 등록하지 않아 총알/폭발이 닿지 않는다. 일반 낙하산 병은 기본 판정 그대로
void RegisterHurtboxes_(
    const Enemy&    _e,
    const EnemyDef& _def,
    CombatSystem&   _combat)
{
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
}   // namespace

const BehaviorOps kFlyingBzkOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = nullptr,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickSoldierDeath,
    .registerHurtboxes = &RegisterHurtboxes_,
};

void ParseFlyingBzkCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    FlyingBzkCfg& c = _def.flyingBzk;

    c.descendSpeed      = _r.Num("descendSpeed");
    c.driftSpeed        = _r.Num("driftSpeed");
    c.driftFlipTicks    = _r.Ticks("driftFlipSec");
    c.flipPercent       = _r.Int("flipPercent");
    c.fireIntervalTicks = _r.Ticks("fireIntervalSec");
    c.maxShots          = _r.Int("maxShots");
    c.fireFrame         = _r.Int("fireFrame");
    (void)_r.NumArray("postureDeg", c.postureDeg);
    (void)_r.NumArray("fireDeg", c.fireDeg);
    c.windowDeg    = _r.Num("windowDeg");
    c.muzzleBase   = _r.ReadVec2("muzzleBase", c.muzzleBase);
    c.muzzleDist   = _r.Num("muzzleDist");
    c.landProbe    = _r.Num("landProbe");
    c.landedDef    = _r.Str("landedDef");
    c.fallGravity  = _r.Num("fallGravity", c.fallGravity);
    c.fallMaxSpeed = _r.Num("fallMaxSpeed", c.fallMaxSpeed);
    c.bNoHurtbox   = _r.Bool("noHurtbox", false);

    for (int i = 0; i < 5; ++i)
    {
        c.clipReady[i] = _r.Clip(std::format("ready{}", i));
        c.clipShot[i]  = _r.Clip(std::format("shot{}", i));
    }
    c.weaponRocket = _r.Weapon("rocket");

    if (_def.overlayClip >= 0)
    {
        c.paraOffset = _def.overlayOffset;
    }
    c.paraDestroy.clip   = _r.Str("paraDestroyFx");
    c.paraDestroy.offset = _r.ReadVec2("paraDestroyOffset", c.paraOffset);
    c.paraDestroy.depth  = 20;
}

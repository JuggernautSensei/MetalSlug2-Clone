#include <cmath>
#include <format>

#include "Base/Math.h"

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

// ===========================================
//  FlyingBzk — 낙하산 바주카 병사 (Docs/EnemySpecs.md §1.4)
//   중력 없이 일정 속도로 하강하며 수평으로 천천히 흔들린다. 화면 안에 있는 동안 주기적으로 플레이어 쪽으로 쏜다.
//   발이 지면에 닿으면 낙하산이 터지고 같은 자리에 soldier_bzk 로 변신한다 (userTag / drop 승계).
// ===========================================

namespace
{
    enum eFlyingState : int
    {
        kDescend,
        kShot
    };

    // e.aux  = 발사 타이머 (틱)
    // e.aux2 = 자세 (0..4)
    // e.aux3 = 수평 흔들림 방향 (-1 / +1)
    // e.aux4 = 흔들림 타이머 (틱)
    // e.counter = 쏜 횟수

    [[nodiscard]] bool IsOnScreen_(const Enemy& _e, const EnemyEnv& _env)
    {
        const Vec2 center = { _e.body.position.x, _e.body.position.y - _e.body.size.y * 0.5f };
        return _env.ctx.cameraView.Contains(center);
    }

    void UpdatePosture_(Enemy& _e, const FlyingBzkCfg& _cfg, const EnemyEnv& _env)
    {
        const float dx  = std::abs(_env.ctx.player.center.x - _e.body.position.x);
        const float dy  = _env.ctx.player.center.y - (_e.body.position.y + _cfg.muzzleBase.y);
        const float deg = dy > 0.f ? std::atan2(dy, slug::Max(dx, 0.001f)) * slug::kRad2Deg : 0.f;
        for (int i = 0; i < 5; ++i)
        {
            if (std::abs(deg - _cfg.postureDeg[i]) <= _cfg.windowDeg)
            {
                _e.aux2 = i;   // 창 사이 간격에서는 직전 자세 유지
                break;
            }
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, const int _state)
    {
        const FlyingBzkCfg& cfg = _def.flyingBzk;
        EnterState(_e, _state);
        SetClip(_e, _state == kShot ? cfg.clipShot[_e.aux2] : cfg.clipReady[_e.aux2], true);
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv&)
    {
        PrepareBody(_e, _def);
        _e.body.bUseGravity = false;
        _e.aux3             = -1;   // 초기 방향 LEFT
        Go_(_e, _def, kDescend);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const FlyingBzkCfg& cfg = _def.flyingBzk;
        _e.body.velocity        = { 0.f, 0.f };
        if (!_e.bActive)
        {
            return;
        }

        // --- 이동: 하강 + 수평 흔들림 ---
        if (++_e.aux4 >= cfg.driftFlipTicks)
        {
            _e.aux4 = 0;
            if (Roll(_env, 100) < cfg.flipPercent)
            {
                _e.aux3 = -_e.aux3;
            }
        }
        _e.body.position.x += static_cast<float>(_e.aux3) * cfg.driftSpeed * kStepSec;
        _e.body.position.y += cfg.descendSpeed * kStepSec;

        // --- 착지 → 병사로 변신 ---
        {
            Body probe = _e.body;
            if (Physics::SnapToGround(probe, _env.ctx.physics, cfg.landProbe))
            {
                const int landedIndex = _env.lib.FindIndex(cfg.landedDef);
                if (landedIndex >= 0)
                {
                    SpawnParams params;
                    params.facing     = _e.facing;
                    params.drop       = _e.drop;
                    params.userTag    = _e.userTag;
                    params.bActivated = true;
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

        // --- 조준 / 발사 ---
        FacePlayer(_e, _env);
        UpdatePosture_(_e, cfg, _env);

        if (_e.state == kDescend)
        {
            SetClip(_e, cfg.clipReady[_e.aux2]);
            ++_e.aux;
            if (_e.aux >= cfg.fireIntervalTicks && _e.counter < cfg.maxShots && IsOnScreen_(_e, _env) && _env.ctx.player.bAlive)
            {
                _e.aux = 0;
                Go_(_e, _def, kShot);
            }
        }
        else
        {
            if (ClipReachedFrame(_e, _def, cfg.fireFrame))
            {
                const float rad = cfg.fireDeg[_e.aux2] * slug::kDeg2Rad;
                const Vec2 dir = { std::cos(rad) * static_cast<float>(_e.facing), std::sin(rad) };
                if (FireWeapon(_e, _def, _env, cfg.weaponRocket, dir, { dir.x * cfg.muzzleDist, dir.y * cfg.muzzleDist }))
                {
                    ++_e.counter;
                }
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, kDescend);
            }
        }
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        _e.bGrounded = false;
        BeginSoldierDeath(_e, _def, _env);
    }
}   // namespace

const BehaviorOps kFlyingBzkOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = nullptr,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickSoldierDeath,
    .registerHurtboxes = nullptr,
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
    (void)_r.NumArray("postureDeg", c.postureDeg, 5);
    (void)_r.NumArray("fireDeg", c.fireDeg, 5);
    c.windowDeg         = _r.Num("windowDeg");
    c.muzzleBase        = _r.ReadVec2("muzzleBase", c.muzzleBase);
    c.muzzleDist        = _r.Num("muzzleDist");
    c.landProbe         = _r.Num("landProbe");
    c.landedDef         = _r.Str("landedDef");

    for (int i = 0; i < 5; ++i)
    {
        c.clipReady[i] = _r.Clip(std::format("ready{}", i));
        c.clipShot[i]  = _r.Clip(std::format("shot{}", i));
    }
    c.weaponRocket = _r.Weapon("rocket");

    // 낙하산 파괴 이펙트 (착지 변신 시)
    if (_def.overlayClip >= 0)
    {
        c.clipPara   = _def.overlayClip;
        c.paraOffset = _def.overlayOffset;
    }
    c.paraDestroy.clip   = _r.Str("paraDestroyFx");
    c.paraDestroy.offset = _r.ReadVec2("paraDestroyOffset", c.paraOffset);
    c.paraDestroy.layer  = 20;
}

}   // namespace slug

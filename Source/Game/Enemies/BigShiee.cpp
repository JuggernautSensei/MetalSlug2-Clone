#include <algorithm>
#include <cmath>

#include "World/Constants.h"
#include "Game/Enemies/BigShiee.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    using namespace bigshiee;

    constexpr int   kPersistTicks  = 1 << 20;   // 끝난 비루프 연출을 마지막 프레임에 머물게 하는 값
    constexpr int   kGunShootTicks = 40;        // Gun_Shoot_k: 8 프레임 x 5 틱
    constexpr int   kGunFrameTicks = 5;         // Gun_Aim 프레임 간격 (1/12 s)
    constexpr int   kAimBins       = 9;         // Gun_Aim 프레임 수 (-135..-45 도, 11.25 도 간격)
    constexpr int   kCannonSource  = kTurretCount;
    constexpr float kAimMinDeg     = -135.f;
    constexpr float kAimMaxDeg     = -45.f;
    constexpr float kRadToDeg      = 57.29578f;
    constexpr float kHullMinX      = -250.f;   // 폭발 영역 (원점 기준, 선체 범위)
    constexpr float kHullMaxX      = 240.f;
    constexpr float kHullMinY      = -200.f;
    constexpr float kHullMaxY      = 110.f;
    constexpr float kDestroyedDy   = 28.f;     // 파괴 잔해 (598x360) 중심 = 원점 + 28
    constexpr float kSinkDy        = 24.f;     // 침몰 연출 최대 하강 (px)
    constexpr int   kSinkTicks     = 120;      // 침몰에 걸리는 틱 (2 s)
    constexpr int   kMinDriveGap   = 20;       // 새 주행 목표는 현재 목표와 이만큼 떨어져야 한다

    constexpr int kKindNone   = 0;
    constexpr int kKindTracer = 1;             // 청록 트레이서 (직선, 플레이어 조준)
    constexpr int kKindShell  = 2;             // 주황 포탄 (포물선, 착탄 시 blast 예약)
    constexpr int kKindBlast  = 3;             // 착탄 폭발 상자 (60x60, 0.3 s)

    constexpr const char* kTurretIdle[kTurretCount]      = { "BigShiee_Turret_A_Idle", "BigShiee_Turret_B_Idle", "BigShiee_Turret_C_Idle", "BigShiee_Turret_D_Idle", "BigShiee_Turret_E_Idle", "BigShiee_Turret_F_Idle", "BigShiee_Turret_G_Idle" };
    constexpr const char* kTurretShooting[kTurretCount]  = { "BigShiee_Turret_A_Shooting", "BigShiee_Turret_B_Shooting", "BigShiee_Turret_C_Shooting", "BigShiee_Turret_D_Shooting", "BigShiee_Turret_E_Shooting", "BigShiee_Turret_F_Shooting", "BigShiee_Turret_G_Shooting" };
    constexpr const char* kTurretDestroyed[kTurretCount] = { "BigShiee_Turret_A_Destroyed", "BigShiee_Turret_B_Destroyed", "BigShiee_Turret_C_Destroyed", "BigShiee_Turret_D_Destroyed", "BigShiee_Turret_E_Destroyed", "BigShiee_Turret_F_Destroyed", "BigShiee_Turret_G_Destroyed" };
    constexpr const char* kGunShoot[5]                   = { "BigShiee_Gun_Shoot_1", "BigShiee_Gun_Shoot_2", "BigShiee_Gun_Shoot_3", "BigShiee_Gun_Shoot_4", "BigShiee_Gun_Shoot_5" };
    constexpr const char* kDeployClips[4]                = { "BigShiee_Cannon_Deploy1", "BigShiee_Cannon_Deploy3", "BigShiee_Cannon_Deploy6", "BigShiee_Cannon_Deploy9" };

    [[nodiscard]] Vec2 OriginOf_(
        const Enemy&          _e,
        const BigShieeCfg&    _c)
    {
        return { _e.body.position.x, _e.body.position.y - _c.originDepth };
    }

    // 화면 좌표 = 월드 x - 아레나 왼쪽. 아레나가 없으면 0 기준.
    [[nodiscard]] float BaseX_(
        const Enemy& _e)
    {
        return _e.arena.IsEmpty() ? 0.f : _e.arena.min.x;
    }

    [[nodiscard]] Vec2 TurretCentreOf_(
        const Enemy&       _e,
        const BigShieeCfg& _c,
        const int          _i)
    {
        const Vec2 o = OriginOf_(_e, _c);
        return { o.x + _c.turretOffset[_i].x, o.y + _c.turretOffset[_i].y };
    }

    [[nodiscard]] Vec2 SourcePos_(
        const Enemy&       _e,
        const BigShieeCfg& _c,
        const int          _source)
    {
        if (_source == kCannonSource)
        {
            const Vec2 o = OriginOf_(_e, _c);
            return { o.x + _c.cannonMuzzle.x, o.y + _c.cannonMuzzle.y };
        }
        const Vec2 t = TurretCentreOf_(_e, _c, _source);
        return { t.x + _c.turretMuzzle.x, t.y + _c.turretMuzzle.y };
    }

    [[nodiscard]] int TurretHp_(
        const Enemy& _e,
        const int    _i)
    {
        return _e.ext[kExtTurret + _i] & 0xFF;
    }

    [[nodiscard]] int TurretTele_(
        const Enemy& _e,
        const int    _i)
    {
        return (_e.ext[kExtTurret + _i] >> 8) & 0xFF;
    }

    void SetTurret_(
        Enemy&    _e,
        const int _i,
        const int _hp,
        const int _tele)
    {
        _e.ext[kExtTurret + _i] = std::clamp(_hp, 0, 255) | (std::clamp(_tele, 0, 255) << 8);
    }

    [[nodiscard]] bool CannonDead_(
        const Enemy& _e)
    {
        return (_e.ext[kExtFlags] & 2) != 0;
    }

    [[nodiscard]] bool IsEnragedImpl_(
        const Enemy& _e)
    {
        return _e.health.hp * 2 <= _e.health.maxHp || CannonDead_(_e);
    }

    [[nodiscard]] int PickAliveTurret_(
        const Enemy& _e,
        EnemyEnv&    _env)
    {
        int alive[kTurretCount] = {};
        int n                   = 0;
        for (int i = 0; i < kTurretCount; ++i)
        {
            if (TurretHp_(_e, i) > 0)
            {
                alive[n++] = i;
            }
        }
        return n == 0 ? -1 : alive[Roll(_env, n)];
    }

    [[nodiscard]] int IntervalTicks_(
        EnemyEnv&          _env,
        const BigShieeCfg& _c,
        const int          _minTicks,
        const int          _maxTicks,
        const bool         _bEnraged)
    {
        int t = _minTicks + Roll(_env, std::max(1, _maxTicks - _minTicks + 1));
        if (_bEnraged)
        {
            t = static_cast<int>(static_cast<float>(t) / _c.enragedRate);
        }
        return std::max(1, t);
    }

    // 예약 큐 항목 = fireAt(14bit) | kind(2bit) << 14 | source(3bit) << 16 | (dx+2048)(12bit) << 19
    [[nodiscard]] int PackItem_(
        const int _fireAt,
        const int _kind,
        const int _source,
        const int _dx)
    {
        return (_fireAt & 0x3FFF) | (_kind << 14) | (_source << 16) | ((std::clamp(_dx, -2047, 2047) + 2048) << 19);
    }
    [[nodiscard]] int ItemFireAt_(const int _v) { return _v & 0x3FFF; }
    [[nodiscard]] int ItemKind_(const int _v) { return (_v >> 14) & 0x3; }
    [[nodiscard]] int ItemSource_(const int _v) { return (_v >> 16) & 0x7; }
    [[nodiscard]] int ItemDx_(const int _v) { return ((_v >> 19) & 0xFFF) - 2048; }

    void Enqueue_(
        Enemy& _e,
        const int _item)
    {
        for (int i = 0; i < kExtQueueCount; ++i)
        {
            if (ItemKind_(_e.ext[kExtQueue + i]) == kKindNone)
            {
                _e.ext[kExtQueue + i] = _item;
                return;
            }
        }
    }

    void ScheduleTracer_(
        Enemy&             _e,
        const BigShieeCfg& _c,
        const int          _fireAt,
        const int          _source)
    {
        if (_source < kTurretCount)
        {
            // 사격 예고: 발사 시각까지 Shooting 클립을 보여 준다
            SetTurret_(_e, _source, TurretHp_(_e, _source), std::max(TurretTele_(_e, _source), std::max(_c.telegraphTicks, _fireAt - _e.aux)));
        }
        Enqueue_(_e, PackItem_(_fireAt, kKindTracer, _source, 0));
    }

    void Fx_(
        EnemyEnv&         _env,
        const char*       _clip,
        const Vec2 _at)
    {
        EnemyFxRequest fx;
        fx.clip     = _clip;
        fx.position = _at;
        _env.world.RequestFx(fx);
    }

    [[nodiscard]] Vec2 Scatter_(
        EnemyEnv&   _env,
        const Vec2  _c,
        const float _half)
    {
        const int   span = static_cast<int>(_half) * 2 + 1;
        const float dx   = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        const float dy   = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        return { _c.x + dx, _c.y + dy };
    }

    void HullExplosion_(
        const Enemy& _e,
        const EnemyDef& _def,
        EnemyEnv&    _env,
        const char*  _clip)
    {
        const Vec2 o = OriginOf_(_e, _def.bigShiee);
        const float dx = kHullMinX + static_cast<float>(Roll(_env, static_cast<int>(kHullMaxX - kHullMinX) + 1));
        const float dy = kHullMinY + static_cast<float>(Roll(_env, static_cast<int>(kHullMaxY - kHullMinY) + 1));
        Fx_(_env, _clip, { o.x + dx, o.y + dy });
    }

    void KillTurret_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _i)
    {
        SetTurret_(_e, _i, 0, 0);
        const Vec2 at = TurretCentreOf_(_e, _def.bigShiee, _i);
        Fx_(_env, "Explosion_Small", Scatter_(_env, at, 15.f));
        Fx_(_env, "Explosion_Mid", Scatter_(_env, at, 15.f));
        Fx_(_env, "Explosion_Large", Scatter_(_env, at, 15.f));
    }

    void KillCannon_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        _e.ext[kExtFlags] |= 2;
        _e.ext[kExtCannonHp] = 0;
        const Vec2 o  = OriginOf_(_e, _def.bigShiee);
        const Vec2 at = { o.x + _def.bigShiee.cannonOffset.x, o.y + _def.bigShiee.cannonOffset.y };
        Fx_(_env, "Explosion_Large", Scatter_(_env, at, 30.f));
        Fx_(_env, "Explosion_Mid", Scatter_(_env, at, 40.f));
    }

    // 전투 중 한 번: 캐논 조준 구간 (목표 방향을 -135..-45 도로 나눈 9 구간)
    [[nodiscard]] int AimBinOf_(
        const Enemy&       _e,
        const BigShieeCfg& _c,
        const EnemyEnv&    _env)
    {
        const Vec2  from  = SourcePos_(_e, _c, kCannonSource);
        const float angle = std::atan2(_env.ctx.player.center.y - from.y, _env.ctx.player.center.x - from.x) * kRadToDeg;
        const float t     = (std::clamp(angle, kAimMinDeg, kAimMaxDeg) - kAimMinDeg) / (kAimMaxDeg - kAimMinDeg);
        return std::clamp(static_cast<int>(std::lround(t * static_cast<float>(kAimBins - 1))), 0, kAimBins - 1);
    }

    void FireTracer_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _source)
    {
        const BigShieeCfg& c    = _def.bigShiee;
        const Vec2         from = SourcePos_(_e, c, _source);
        Fx_(_env, _source < 4 ? "BigShiee_Fire_ABCD_1" : "BigShiee_Fire_EFG_1", from);
        if (_source == kCannonSource)
        {
            _e.ext[kExtLastShot] = _e.aux + 1;   // 캐논 발사 플래시 (Gun_Shoot)
        }
        if (!_env.ctx.pProjectiles || !_env.ctx.pCombat || !_env.ctx.player.bAlive)
        {
            return;
        }
        const int projIndex = _env.lib.FindProjectileIndex(c.tracerProjectile);
        if (projIndex < 0)
        {
            return;
        }
        const ProjectileDef& proj = _env.lib.GetProjectile(projIndex);

        Vec2       d   = { _env.ctx.player.center.x - from.x, _env.ctx.player.center.y - from.y };
        const float len = std::sqrt(d.x * d.x + d.y * d.y);
        d = len < 0.0001f ? Vec2 { -1.f, 0.f } : Vec2 { d.x / len, d.y / len };

        ShotSpawn spawn;
        spawn.team            = eTeam::Enemy;
        spawn.owner           = _e.id;
        spawn.position        = from;
        spawn.velocity        = { d.x * c.streakSpeed, d.y * c.streakSpeed };
        spawn.gravity         = proj.gravity;
        spawn.maxFallSpeed    = proj.maxFall;
        spawn.size            = proj.size;
        spawn.damage          = proj.damage;
        spawn.kind            = eHitKind::Bullet;
        spawn.lifetimeTicks   = SecToTicks(proj.lifetimeSec);
        spawn.bExplodeOnSolid = proj.bExplodeOnSolid;
        spawn.visual          = static_cast<uint16_t>(projIndex + 1);
        _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
        ++_e.shotsFired;
    }

    // 착탄 시각은 같은 이산 적분(Shot 과 동일한 반-오일러)으로 미리 계산해 blast 를 예약한다.
    void FireShell_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BigShieeCfg& c    = _def.bigShiee;
        const Vec2         from = SourcePos_(_e, c, kCannonSource);
        Fx_(_env, "BigShiee_Fire_EFG_1", from);
        _e.ext[kExtLastShot] = _e.aux + 1;
        if (!_env.ctx.pProjectiles || !_env.ctx.pCombat || !_env.ctx.player.bAlive)
        {
            return;
        }
        const int projIndex = _env.lib.FindProjectileIndex(c.shellProjectile);
        if (projIndex < 0)
        {
            return;
        }
        const ProjectileDef& proj = _env.lib.GetProjectile(projIndex);

        // 비행 시간 T 에 발사 높이에서 지면까지 떨어지도록 수직 초속을 구한다: H = vy*T + g*T^2/2 (y 아래 +)
        const float groundY = _e.body.position.y;
        const float g       = proj.gravity;
        const float flight  = std::max(0.1f, c.shellFlightSec);
        const float vy0     = ((groundY - from.y) - 0.5f * g * flight * flight) / flight;
        float       vy      = vy0;
        float       y       = from.y;
        int         n       = 0;
        while (n < 600)
        {
            ++n;
            vy = std::min(vy + g * kStepSec, proj.maxFall);
            y += vy * kStepSec;
            if (y >= groundY)
            {
                break;
            }
        }

        // 착탄 = 발사 시점 플레이어 발 x (jitter 0 이면 정확히, 그 밖이면 ± 값)
        const int   jitter = static_cast<int>(c.shellJitter);
        const float target = _env.ctx.player.feet.x + (jitter > 0 ? static_cast<float>(Roll(_env, 2 * jitter + 1) - jitter) : 0.f);
        const float vx     = (target - from.x) / (static_cast<float>(n) * kStepSec);

        ShotSpawn spawn;
        spawn.team            = eTeam::Enemy;
        spawn.owner           = _e.id;
        spawn.position        = from;
        spawn.velocity        = { vx, vy0 };
        spawn.gravity         = g;
        spawn.maxFallSpeed    = proj.maxFall;
        spawn.size            = proj.size;
        spawn.damage          = proj.damage;
        spawn.kind            = eHitKind::Bullet;
        spawn.lifetimeTicks   = SecToTicks(proj.lifetimeSec);
        spawn.bExplodeOnSolid = proj.bExplodeOnSolid;
        spawn.visual          = static_cast<uint16_t>(projIndex + 1);
        _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
        ++_e.shotsFired;

        if (_env.lib.FindProjectileIndex(c.blastProjectile) >= 0)
        {
            const int dx = static_cast<int>(std::lround(target - BaseX_(_e)));
            Enqueue_(_e, PackItem_(_e.aux + n, kKindBlast, 0, dx));
        }
    }

    void FireBlast_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _dx)
    {
        if (!_env.ctx.pProjectiles || !_env.ctx.pCombat || !_env.ctx.player.bAlive)
        {
            return;
        }
        const int projIndex = _env.lib.FindProjectileIndex(_def.bigShiee.blastProjectile);
        if (projIndex < 0)
        {
            return;
        }
        const ProjectileDef& proj = _env.lib.GetProjectile(projIndex);
        const float          x    = BaseX_(_e) + static_cast<float>(_dx);
        const float          gy   = _e.body.position.y;

        Fx_(_env, "Explosion_Mid", { x, gy - 20.f });

        ShotSpawn spawn;
        spawn.team            = eTeam::Enemy;
        spawn.owner           = _e.id;
        spawn.position        = { x, gy - proj.size.y * 0.5f };
        spawn.velocity        = { 0.f, 0.f };
        spawn.gravity         = 0.f;
        spawn.maxFallSpeed    = 0.f;
        spawn.size            = proj.size;
        spawn.damage          = proj.damage;
        spawn.kind            = eHitKind::Blast;
        spawn.lifetimeTicks   = SecToTicks(proj.lifetimeSec);
        spawn.bExplodeOnSolid = false;
        spawn.visual          = 0;   // 그리지 않는 판정 전용 상자
        _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
    }

    void TickQueue_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        for (int i = 0; i < kExtQueueCount; ++i)
        {
            const int item = _e.ext[kExtQueue + i];
            if (ItemKind_(item) == kKindNone || ItemFireAt_(item) > _e.aux)
            {
                continue;
            }
            _e.ext[kExtQueue + i] = 0;
            switch (ItemKind_(item))
            {
                case kKindTracer:
                {
                    const int source = ItemSource_(item);
                    if (source < kTurretCount ? TurretHp_(_e, source) > 0 : !CannonDead_(_e))
                    {
                        FireTracer_(_e, _def, _env, source);
                    }
                    break;
                }
                case kKindShell:
                    if (!CannonDead_(_e))
                    {
                        FireShell_(_e, _def, _env);
                    }
                    break;
                case kKindBlast:
                    FireBlast_(_e, _def, _env, ItemDx_(item));
                    break;
                default: break;
            }
        }
    }

    void TickDrive_(
        Enemy&             _e,
        const BigShieeCfg& _c,
        EnemyEnv&          _env)
    {
        if (_e.ext[kExtDriveRest] > 0)
        {
            --_e.ext[kExtDriveRest];
            _e.body.velocity.x = 0.f;
            return;
        }
        const float target = BaseX_(_e) + static_cast<float>(_e.ext[kExtDriveTarget]);
        const float dist   = target - _e.body.position.x;
        const float step   = _c.driveSpeed * kStepSec;
        if (std::abs(dist) <= step)
        {
            _e.body.position.x = target;
            _e.body.velocity.x = 0.f;
            _e.ext[kExtDriveRest] = _c.driveRestTicks;
            const int lo   = static_cast<int>(_c.driveMinX);
            const int span = std::max(1, static_cast<int>(_c.driveMaxX) - lo + 1);
            int       next = _e.ext[kExtDriveTarget];
            for (int t = 0; t < 4; ++t)
            {
                next = lo + Roll(_env, span);
                if (std::abs(next - _e.ext[kExtDriveTarget]) >= kMinDriveGap)
                {
                    break;
                }
            }
            _e.ext[kExtDriveTarget] = next;
            return;
        }
        const float dir = dist > 0.f ? 1.f : -1.f;
        _e.body.position.x += dir * step;
        _e.body.velocity.x = dir * _c.driveSpeed;
    }

    void TickEntrance_(
        Enemy&             _e,
        const BigShieeCfg& _c,
        EnemyEnv&          _env)
    {
        const float home = BaseX_(_e) + _c.homeX;
        const int   total = std::max(1, _c.entranceTicks);
        const float s     = std::min(1.f, static_cast<float>(_e.stateTicks) / static_cast<float>(total));
        const float k     = 1.f - s;
        if (_e.stateTicks >= total)
        {
            _e.body.position.x = home;
            _e.body.velocity.x = 0.f;
            _e.ext[kExtFlags] |= 1;
            _e.ext[kExtDriveTarget] = static_cast<int>(_c.homeX);
            _e.ext[kExtDriveRest]   = _c.driveRestTicks;
            _e.aux                  = 0;
            _e.aux2                 = 0;
            EnterState(_e, kAssault);
            return;
        }
        _e.body.position.x = home + _c.entranceFromX * k * k;
        _e.body.velocity.x = -2.f * _c.entranceFromX * k / (static_cast<float>(total) * kStepSec);
        if (_e.stateTicks % std::max(1, _c.rumbleTicks) == 0)
        {
            Fx_(_env, "Explosion_Small", Scatter_(_env, { _e.body.position.x, _e.body.position.y - 20.f }, 200.f));
        }
    }

    void TickAssault_(
        Enemy&             _e,
        const BigShieeCfg& _c,
        EnemyEnv&          _env)
    {
        if (_e.stateTicks >= _c.assaultTicks)
        {
            _e.aux2 = _e.aux + _c.telegraphTicks;
            EnterState(_e, kDeploy);
            return;
        }
        if (_e.aux >= _e.aux2 && _env.ctx.player.bAlive)
        {
            for (int k = 0; k < _c.burstCount; ++k)
            {
                const int src = PickAliveTurret_(_e, _env);
                if (src < 0)
                {
                    break;
                }
                ScheduleTracer_(_e, _c, _e.aux + _c.telegraphTicks + k * _c.streakStaggerTicks, src);
            }
            // 연사 버스트는 격앙 배율을 받지 않는다 (격앙은 캐논 볼리 간격만 줄인다)
            _e.aux2 = _e.aux + IntervalTicks_(_env, _c, _c.burstTicks, _c.burstTicks, false);
        }
    }

    // 트레이서 1발의 수명 (틱). 볼리가 끝나고 이 시간이 지나야 다음 볼리가 허용된다.
    [[nodiscard]] int TracerLifeTicks_(
        EnemyEnv&          _env,
        const BigShieeCfg& _c)
    {
        const int idx = _env.lib.FindProjectileIndex(_c.tracerProjectile);
        return idx < 0 ? 0 : SecToTicks(_env.lib.GetProjectile(idx).lifetimeSec);
    }

    void TickCannon_(
        Enemy&             _e,
        const BigShieeCfg& _c,
        EnemyEnv&          _env)
    {
        if (_e.aux < _e.aux2 || !_env.ctx.player.bAlive)
        {
            return;
        }
        // 한 번에 볼리 하나만 비행한다: 직전 볼리의 트레이서가 모두 사라질 때까지 대기
        if (_e.aux < _e.ext[kExtVolleyBusy])
        {
            _e.aux2 = _e.ext[kExtVolleyBusy];
            return;
        }
        const bool bEnraged = IsEnragedImpl_(_e);
        ++_e.aux3;   // 볼리 번호
        for (int k = 0; k < _c.volleyStreaks; ++k)
        {
            int src = PickAliveTurret_(_e, _env);
            if (src < 0)
            {
                src = CannonDead_(_e) ? -1 : kCannonSource;
            }
            if (src < 0)
            {
                break;
            }
            ScheduleTracer_(_e, _c, _e.aux + _c.telegraphTicks + k * _c.streakStaggerTicks, src);
        }
        _e.ext[kExtVolleyBusy] = _e.aux + _c.telegraphTicks + std::max(0, _c.volleyStreaks - 1) * _c.streakStaggerTicks + TracerLifeTicks_(_env, _c);
        if (_c.shellEvery > 0 && _e.aux3 % _c.shellEvery == 0 && !CannonDead_(_e))
        {
            Enqueue_(_e, PackItem_(_e.aux + _c.telegraphTicks, kKindShell, kCannonSource, 0));
        }
        _e.aux2 = _e.aux + IntervalTicks_(_env, _c, _c.volleyMinTicks, _c.volleyMaxTicks, bEnraged);
    }

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_env;
        const BigShieeCfg& c = _def.bigShiee;
        PrepareBody(_e, _def);
        _e.facing = 1;
        for (int i = 0; i < kTurretCount; ++i)
        {
            SetTurret_(_e, i, c.turretHp, 0);
        }
        for (int i = 0; i < kExtQueueCount; ++i)
        {
            _e.ext[kExtQueue + i] = 0;
        }
        _e.ext[kExtCannonHp]    = c.cannonHp;
        _e.ext[kExtFlags]       = 0;
        _e.ext[kExtDriveTarget] = static_cast<int>(c.homeX);
        _e.ext[kExtDriveRest]   = c.driveRestTicks;
        _e.ext[kExtLastShot]    = 0;
        _e.ext[kExtAimBin]      = kAimBins / 2;
        _e.aux                  = 0;
        _e.aux2                 = 0;
        _e.aux3                 = 0;
        const float home = BaseX_(_e) + c.homeX;
        _e.body.velocity.x = 0.f;
        if (!_e.arena.IsEmpty())
        {
            _e.body.position.x = home + c.entranceFromX;   // 오른쪽 화면 밖에서 굴러 들어온다
            EnterState(_e, kEntrance);
        }
        else
        {
            _e.body.position.x = home;
            _e.ext[kExtFlags] |= 1;   // 아레나 없이 스폰: 즉시 전투
            EnterState(_e, kAssault);
        }
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BigShieeCfg& c = _def.bigShiee;
        _e.facing            = 1;

        if (_e.state == kEntrance)
        {
            TickEntrance_(_e, c, _env);
        }
        else
        {
            ++_e.aux;   // 전투 시계 (등장 종료 이후)
            for (int i = 0; i < kTurretCount; ++i)
            {
                const int tele = TurretTele_(_e, i);
                if (tele > 0)
                {
                    SetTurret_(_e, i, TurretHp_(_e, i), tele - 1);
                }
            }
            TickQueue_(_e, _def, _env);

            switch (_e.state)
            {
                case kAssault:
                    TickAssault_(_e, c, _env);
                    break;
                case kDeploy:
                    if (_e.stateTicks >= c.deployTicks)
                    {
                        _e.aux2 = _e.aux + c.telegraphTicks;
                        EnterState(_e, kCannon);
                    }
                    break;
                case kCannon:
                    TickCannon_(_e, c, _env);
                    _e.ext[kExtAimBin] = AimBinOf_(_e, c, _env);
                    break;
                default: break;
            }

            if (_e.state == kAssault || _e.state == kCannon)
            {
                TickDrive_(_e, c, _env);
            }
            else
            {
                _e.body.velocity.x = 0.f;
            }
        }

        // 플레이어 벽: 아레나 왼쪽 + playerWallX 에 고정 (보스 주행과 무관). 사망 후에는 걸지 않는다 (TickDying_).
        _env.world.RequestPlayerMaxX(BaseX_(_e) + c.playerWallX);
    }

    bool OnHit_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const HitEvent& _event)
    {
        const int part = GetPartIndex(_event.target);
        if (part < 0)
        {
            return false;   // 선체: 기본 체력 감소 경로
        }
        if (part < kTurretCount)
        {
            const int hp = TurretHp_(_e, part);
            if (hp <= 0)
            {
                return true;
            }
            const int left = std::max(0, hp - _event.damage);
            SetTurret_(_e, part, left, TurretTele_(_e, part));
            if (left == 0)
            {
                KillTurret_(_e, _def, _env, part);
            }
            return true;
        }
        if (part == kCannonPart && !CannonDead_(_e))
        {
            const int left = std::max(0, _e.ext[kExtCannonHp] - _event.damage);
            _e.ext[kExtCannonHp] = left;
            if (left == 0)
            {
                KillCannon_(_e, _def, _env);
            }
        }
        return true;
    }

    void OnDeath_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        for (int i = 0; i < kTurretCount; ++i)
        {
            if (TurretHp_(_e, i) > 0)
            {
                KillTurret_(_e, _def, _env, i);
            }
        }
        if (!CannonDead_(_e))
        {
            KillCannon_(_e, _def, _env);
        }
        for (int i = 0; i < kExtQueueCount; ++i)
        {
            _e.ext[kExtQueue + i] = 0;   // 사망 후 더 이상 공격하지 않는다
        }
        _e.deathTicks = 0;
    }

    void TickDying_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BigShieeCfg& c        = _def.bigShiee;
        const int          chain    = SecToTicks(c.destroyChainSec);
        const int          step     = std::max(1, SecToTicks(c.destroyStepSec));
        const int          smoke    = std::max(1, SecToTicks(c.smokeStepSec));
        const int          finish   = SecToTicks(c.destroyFinishSec);
        ++_e.deathTicks;

        if (_e.deathTicks <= chain && _e.deathTicks % step == 0)
        {
            HullExplosion_(_e, _def, _env, "Explosion_Small");
            if (Roll(_env, 2) == 0)
            {
                HullExplosion_(_e, _def, _env, "Explosion_Mid");
            }
            if (Roll(_env, 3) == 0)
            {
                HullExplosion_(_e, _def, _env, "Explosion_Large");
            }
        }
        if (_e.deathTicks == chain)
        {
            const Vec2 o = OriginOf_(_e, c);
            Fx_(_env, "Explosion_Huge", { o.x - 30.f, o.y - 50.f });
            for (int i = 0; i < 6; ++i)
            {
                HullExplosion_(_e, _def, _env, "Explosion_Mid");
                HullExplosion_(_e, _def, _env, "Explosion_Large");
            }
        }
        if (_e.deathTicks > chain && _e.deathTicks < finish && _e.deathTicks % smoke == 0)
        {
            const Vec2 o = OriginOf_(_e, c);
            Fx_(_env, "BigShiee_Smoke", Scatter_(_env, { o.x, o.y + kDestroyedDy }, 120.f));
        }
    }

    bool DeathDone_(
        const Enemy&    _e,
        const EnemyDef& _def)
    {
        return _e.deathTicks >= SecToTicks(_def.bigShiee.destroyFinishSec);
    }

    void RegisterHurtboxes_(
        const Enemy&    _e,
        const EnemyDef& _def,
        CombatSystem&   _combat)
    {
        const BigShieeCfg& c = _def.bigShiee;
        const Vec2         o = OriginOf_(_e, c);

        Hurtbox hull;
        hull.box     = c.hullBox.At(o, 1);
        hull.team    = _def.team;
        hull.owner   = _e.id;
        hull.accepts = AcceptsFor(_def.bHuman);
        _combat.AddHurtbox(hull);

        if (!CannonDead_(_e))
        {
            Hurtbox cannon = hull;
            cannon.box     = c.cannonBox.At(o, 1);
            cannon.owner   = CannonOwner(_e.id);
            _combat.AddHurtbox(cannon);
        }

        for (int i = 0; i < kTurretCount; ++i)
        {
            if (TurretHp_(_e, i) <= 0)
            {
                continue;
            }
            Hurtbox turret = hull;
            turret.box     = c.turretBox.At(TurretCentreOf_(_e, c, i), 1);
            turret.owner   = TurretOwner(_e.id, i);
            _combat.AddHurtbox(turret);
        }
    }

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _r)
    {
        const BigShieeCfg& c     = _def.bigShiee;
        const Vec2         o     = { _r.feet.x, _r.feet.y - c.originDepth };
        const int          sort  = static_cast<int>(_r.feet.y);
        const int          layer = _r.layer;
        const int          chain = SecToTicks(c.destroyChainSec);

        if (_e.life == eEnemyLife::Dying && _e.deathTicks >= chain)
        {
            const int sinkTicks = std::min(kSinkTicks, _e.deathTicks - chain);
            const float dy      = kDestroyedDy + kSinkDy * static_cast<float>(sinkTicks) / static_cast<float>(kSinkTicks);
            SubmitNamedPart(_r, "BigShiee_Destroyed_Body", { o.x, o.y + dy }, kPersistTicks, false, sort, layer);
            SubmitNamedPart(_r, "BigShiee_Destroyed_Cannon", { o.x + c.wreckCannonOffset.x, o.y + c.wreckCannonOffset.y + dy }, kPersistTicks, false, sort + 1, layer);
            return;
        }

        // 선체: 전개 단계 / 캐논 전개 후 주행 / 기본 주행
        const bool bMoving = std::abs(_e.body.velocity.x) > 0.01f;
        const bool bRight  = _e.body.velocity.x > 0.f;
        if (_e.state == kDeploy)
        {
            const int stageLen = std::max(1, c.deployTicks / 4);
            const int stage    = std::min(3, _e.stateTicks / stageLen);
            SubmitNamedPart(_r, kDeployClips[stage], o, _e.stateTicks - stage * stageLen, false, sort, layer);
        }
        else if (_e.state == kCannon || _e.life == eEnemyLife::Dying)
        {
            SubmitNamedPart(_r, bMoving ? (bRight ? "BigShiee_Cannon_DriveForward" : "BigShiee_Cannon_DriveBack") : "BigShiee_Cannon_Idle", o, _e.clipTicks, true, sort, layer);
        }
        else
        {
            SubmitNamedPart(_r, bMoving ? (bRight ? "BigShiee_Body_DriveForward" : "BigShiee_Body_DriveBack") : "BigShiee_Body_Idle", o, _e.clipTicks, true, sort, layer);
        }

        // 캐논 헤드 (Gun_* 클립 피벗 = 캐논 중심)
        const Vec2 gun = { o.x + c.cannonOffset.x, o.y + c.cannonOffset.y };
        if (CannonDead_(_e))
        {
            SubmitNamedPart(_r, "BigShiee_Destroyed_Cannon", gun, kPersistTicks, false, sort + 2, layer);
        }
        else if (_e.state == kCannon && _e.life == eEnemyLife::Alive)
        {
            const int bin       = std::clamp(_e.ext[kExtAimBin], 0, kAimBins - 1);
            const int lastShot  = _e.ext[kExtLastShot];
            const int since     = lastShot > 0 ? _e.aux - (lastShot - 1) : kGunShootTicks;
            if (since >= 0 && since < kGunShootTicks)
            {
                SubmitNamedPart(_r, kGunShoot[std::min(4, bin / 2)], gun, since, false, sort + 2, layer);
            }
            else
            {
                SubmitNamedPart(_r, "BigShiee_Gun_Aim", gun, bin * kGunFrameTicks, false, sort + 2, layer);
            }
        }

        // 포탑 A..G: 살아 있으면 Idle (사격 예고 중에는 Shooting), 파괴되면 잔해
        for (int i = 0; i < kTurretCount; ++i)
        {
            const Vec2 t = TurretCentreOf_(_e, c, i);
            if (TurretHp_(_e, i) <= 0)
            {
                SubmitNamedPart(_r, kTurretDestroyed[i], t, kPersistTicks, false, sort + 3, layer);
                continue;
            }
            const int tele = TurretTele_(_e, i);
            if (tele > 0 && _e.life == eEnemyLife::Alive)
            {
                SubmitNamedPart(_r, kTurretShooting[i], t, std::max(0, c.telegraphTicks - tele), false, sort + 3, layer);
            }
            else
            {
                SubmitNamedPart(_r, kTurretIdle[i], t, _e.clipTicks, true, sort + 3, layer);
            }
        }

        if (_e.life == eEnemyLife::Alive)
        {
            SubmitNamedPart(_r, "BigShiee_Emblem_Spin_A", { o.x + c.emblemOffset.x, o.y + c.emblemOffset.y }, _e.clipTicks, true, sort + 4, layer);
        }
    }
}

const BehaviorOps kBigShieeOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = &RegisterHurtboxes_,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
};

namespace bigshiee
{

bool IsEnraged(const Enemy& _e)
{
    return IsEnragedImpl_(_e);
}

bool IsFightStarted(const Enemy& _e)
{
    return (_e.ext[kExtFlags] & 1) != 0;
}

bool IsCannonDestroyed(const Enemy& _e)
{
    return CannonDead_(_e);
}

int GetTurretHp(const Enemy& _e, const int _index)
{
    return TurretHp_(_e, _index);
}

int GetCannonHp(const Enemy& _e)
{
    return _e.ext[kExtCannonHp];
}

int GetVolleyCount(const Enemy& _e)
{
    return _e.aux3;
}

int GetQueuedCount(const Enemy& _e)
{
    int n = 0;
    for (int i = 0; i < kExtQueueCount; ++i)
    {
        n += ItemKind_(_e.ext[kExtQueue + i]) != kKindNone ? 1 : 0;
    }
    return n;
}

Vec2 GetOrigin(const Enemy& _e, const EnemyDef& _def)
{
    return OriginOf_(_e, _def.bigShiee);
}
}

void ParseBigShieeCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BigShieeCfg& c = _def.bigShiee;

    c.originDepth      = _r.Num("originDepth");
    c.homeX            = _r.Num("homeX");
    c.driveMinX        = _r.Num("driveMinX");
    c.driveMaxX        = _r.Num("driveMaxX");
    c.driveSpeed       = _r.Num("driveSpeed");
    c.driveRestTicks   = _r.Ticks("driveRestSec");
    c.entranceFromX    = _r.Num("entranceFromX");
    c.entranceTicks    = _r.Ticks("entranceSec");
    c.rumbleTicks      = _r.Ticks("rumbleSec");
    c.playerWallX      = _r.Num("playerWallX");

    c.assaultTicks     = _r.Ticks("assaultSec");
    c.burstTicks       = _r.Ticks("burstSec");
    c.burstCount       = _r.Int("burstCount");
    c.deployTicks      = _r.Ticks("deploySec");
    c.volleyMinTicks   = _r.Ticks("volleyMinSec");
    c.volleyMaxTicks   = _r.Ticks("volleyMaxSec");
    c.volleyStreaks    = _r.Int("volleyStreaks");
    c.shellEvery       = _r.Int("shellEvery");
    c.telegraphTicks   = _r.Ticks("telegraphSec");
    c.streakStaggerTicks = _r.Ticks("streakStaggerSec");
    c.shellJitter      = _r.Num("shellJitter");
    c.shellFlightSec   = _r.Num("shellFlightSec");
    c.enragedRate      = _r.Num("enragedRate");
    c.streakSpeed      = _r.Num("streakSpeed");

    c.turretHp         = _r.Int("turretHp");
    c.cannonHp         = _r.Int("cannonHp");
    {
        float offsets[kBigShieeTurretCount * 2] = {};
        (void)_r.NumArray("turretOffsets", offsets, kBigShieeTurretCount * 2);
        for (int i = 0; i < kBigShieeTurretCount; ++i)
        {
            c.turretOffset[i] = { offsets[i * 2], offsets[i * 2 + 1] };
        }
    }
    c.turretMuzzle     = _r.ReadVec2("turretMuzzle", { -40.f, -10.f });
    c.turretBox        = _r.Box("turretBox");
    c.cannonOffset     = _r.ReadVec2("cannonOffset", { -133.f, -75.f });
    c.cannonMuzzle     = _r.ReadVec2("cannonMuzzle", { -173.f, -64.f });
    c.cannonBox        = _r.Box("cannonBox");
    c.hullBox          = _r.Box("hullBox");
    c.emblemOffset     = _r.ReadVec2("emblemOffset", { 245.f, 25.f });
    c.wreckCannonOffset = _r.ReadVec2("wreckCannonOffset", { -150.f, 60.f });

    c.tracerProjectile = _r.Str("tracerProjectile");
    c.shellProjectile  = _r.Str("shellProjectile");
    c.blastProjectile  = _r.Str("blastProjectile");

    c.destroyChainSec  = _r.Num("destroyChainSec");
    c.destroyStepSec   = _r.Num("destroyStepSec");
    c.destroyFinishSec = _r.Num("destroyFinishSec");
    c.smokeStepSec     = _r.Num("smokeStepSec");
}
}

#include <algorithm>
#include "Base/Math.h"
#include <cmath>

#include "World/Constants.h"
#include "Game/Enemies/BattleShip.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    using namespace battleship;

    constexpr int   kPersistTicks = 1 << 20;   // 끝난 비루프 연출을 마지막 프레임에 머물게 하는 값
    constexpr int   kBulletLayer  = 32;
    constexpr float kHullMinX     = -250.f;    // 폭발 영역 (원점 기준, 레거시 선체 영역)
    constexpr float kHullMaxX     = 240.f;
    constexpr float kHullMinY     = -200.f;
    constexpr float kHullMaxY     = 110.f;
    constexpr float kRadToDeg     = 57.29578f;
    constexpr float kDegToRad     = 0.0174533f;

    constexpr const char* kCannonIdleNames[9] = {
        "BattleShip_Cannon_Idle_0", "BattleShip_Cannon_Idle_1", "BattleShip_Cannon_Idle_2", "BattleShip_Cannon_Idle_3", "BattleShip_Cannon_Idle_4",
        "BattleShip_Cannon_Idle_5", "BattleShip_Cannon_Idle_6", "BattleShip_Cannon_Idle_7", "BattleShip_Cannon_Idle_8"
    };
    constexpr const char* kCannonShotNames[5] = {
        "BattleShip_Cannon_Shot_0", "BattleShip_Cannon_Shot_1", "BattleShip_Cannon_Shot_2", "BattleShip_Cannon_Shot_3", "BattleShip_Cannon_Shot_4"
    };
    constexpr const char* kBulletNames[5] = {
        "BattleShip_Bullet_0", "BattleShip_Bullet_1", "BattleShip_Bullet_2", "BattleShip_Bullet_3", "BattleShip_Bullet_4"
    };
    constexpr const char* kDirNames[3]        = { "Left", "Mid", "Right" };
    constexpr const char* kDirFxNames[3]      = { "BattleShip_DCannon_Shot_Effect_Left", "BattleShip_DCannon_Shot_Effect_Mid", "BattleShip_DCannon_Shot_Effect_Right" };
    constexpr const char* kTurretIdleNames[3] = { "BattleShip_DCannon_Left", "BattleShip_DCannon_Mid", "BattleShip_DCannon_Right" };
    constexpr const char* kTurretShotNames[3] = { "BattleShip_DCannon_Shot_Left", "BattleShip_DCannon_Shot_Mid", "BattleShip_DCannon_Shot_Right" };

    constexpr int kActNone      = 0;
    constexpr int kActTurnLeft  = 1;
    constexpr int kActTurnRight = 2;
    constexpr int kActShot      = 3;

    [[nodiscard]] Vec2 OriginOf_(
        const Enemy&          _e,
        const BattleShipCfg& _c)
    {
        return { _e.body.position.x, _e.body.position.y - _c.originDepth };
    }

    [[nodiscard]] Vec2 TurretOriginOf_(
        const Enemy&          _e,
        const BattleShipCfg& _c,
        const int             _i)
    {
        const Vec2 o = OriginOf_(_e, _c);
        return { o.x + _c.turretOffset[_i].x, o.y + _c.turretOffset[_i].y };
    }

    [[nodiscard]] Vec2 CannonOriginOf_(
        const Enemy&          _e,
        const BattleShipCfg& _c)
    {
        const Vec2 o = OriginOf_(_e, _c);
        return { o.x + _c.cannonOffset.x, o.y + _c.cannonOffset.y };
    }

    [[nodiscard]] float SlotX_(
        const Enemy&          _e,
        const BattleShipCfg& _c,
        const int             _slot)
    {
        return _e.arena.min.x + _c.railSlot[_slot];
    }

    [[nodiscard]] int* Turret_(
        Enemy&  _e,
        const int _i)
    {
        return &_e.ext[kTurretBase + _i * kTurretStride];
    }

    [[nodiscard]] int CurrentSlot_(const Enemy& _e)
    {
        return _e.ext[kExtRailSlot];
    }

    bool RailStep_(
        Enemy&               _e,
        const BattleShipCfg& _c,
        const float          _targetX)
    {
        const float dt   = kStepSec;
        const float x    = _e.body.position.x;
        const float v    = _e.body.velocity.x;
        const float dist = _targetX - x;
        if (std::abs(dist) < 0.5f && std::abs(v) < 1.f)
        {
            _e.body.position.x   = _targetX;
            _e.body.velocity.x   = 0.f;
            return true;
        }
        const float dir      = dist >= 0.f ? 1.f : -1.f;
        const float stopDist = (v * v) / (2.f * _c.railDecel);
        float       nv       = v;
        if (v * dir > 0.f && std::abs(dist) <= stopDist)
        {
            nv = v - (v > 0.f ? 1.f : -1.f) * _c.railDecel * dt;
            if (nv * v < 0.f)
            {
                nv = 0.f;
            }
        }
        else
        {
            const float step = _c.railAccel * dt;
            nv               = v + std::clamp(dir * _c.railMaxSpeed - v, -step, step);
        }
        float nx = x + nv * dt;
        if ((_targetX - nx) * dir < 0.f)   // 목표를 지나쳤다
        {
            nx = _targetX;
            nv = 0.f;
        }
        _e.body.position.x = nx;
        _e.body.velocity.x = nv;
        if (nx == _targetX)
        {
            _e.body.velocity.x = 0.f;
            return true;
        }
        return false;
    }

    [[nodiscard]] int PickOtherSlot_(
        EnemyEnv& _env,
        const int _current)
    {
        int others[2] = { 0, 0 };
        int n         = 0;
        for (int s = 0; s < 3; ++s)
        {
            if (s != _current && n < 2)
            {
                others[n++] = s;
            }
        }
        return others[Roll(_env, 2)];
    }

    [[nodiscard]] bool IsEnragedImpl_(const Enemy& _e)
    {
        return _e.health.hp * 2 <= _e.health.maxHp;
    }

    void SetAngle_(
        Enemy& _e,
        const float _deg)
    {
        _e.ext[kExtAngle] = static_cast<int>(std::lround(_deg * 100.f));
    }

    void TrackCannon_(
        Enemy&               _e,
        const BattleShipCfg& _c,
        const EnemyEnv&      _env)
    {
        const Vec2 cannon = CannonOriginOf_(_e, _c);
        const float        aim    = std::atan2(_env.ctx.player.center.y - cannon.y, _env.ctx.player.center.x - cannon.x) * kRadToDeg;
        const float        target = std::clamp(aim, _c.cannonAngleMin, _c.cannonAngleMax);
        const float        rate   = (IsEnragedImpl_(_e) ? _c.cannonTrackDegEnraged : _c.cannonTrackDeg) * kStepSec;
        const float        now    = static_cast<float>(_e.ext[kExtAngle]) / 100.f;
        SetAngle_(_e, now + std::clamp(target - now, -rate, rate));
    }

    [[nodiscard]] int AngleFrame_(
        const float          _deg,
        const BattleShipCfg& _c)
    {
        const float t = (_deg - _c.cannonAngleMin) / (_c.cannonAngleMax - _c.cannonAngleMin);
        return std::clamp(static_cast<int>(std::lround(t * 8.f)), 0, 8);   // Cannon_Idle_0..8
    }

    [[nodiscard]] int AngleBin_(
        const float          _deg,
        const BattleShipCfg& _c)
    {
        const float t = (_deg - _c.cannonAngleMin) / (_c.cannonAngleMax - _c.cannonAngleMin);
        return std::clamp(static_cast<int>(std::lround(t * 4.f)), 0, 4);   // Cannon_Shot_0..4 (샷 클립 = 탄 스프라이트)
    }

    void FireCannonBullet_(
        Enemy&               _e,
        const EnemyDef&      _def,
        EnemyEnv&            _env,
        const int            _bin)
    {
        const BattleShipCfg& c = _def.battleShip;
        if (!_env.ctx.pProjectiles || !_env.ctx.pCombat || !_env.ctx.player.bAlive)
        {
            return;
        }
        const int projIndex = _env.lib.FindProjectileIndex(c.bulletProjectile);
        if (projIndex < 0)
        {
            return;
        }
        const ProjectileDef& proj = _env.lib.GetProjectile(projIndex);

        const Vec2 o   = OriginOf_(_e, c);
        const Vec2 pos = { o.x + c.cannonOffset.x + static_cast<float>(_bin - 2) * c.bulletSpread, o.y + c.cannonOffset.y + 10.f };
        Vec2       d   = { _env.ctx.player.center.x - pos.x, std::max(_env.ctx.player.center.y - pos.y, 1.f) };
        const float        lim = 1.7f * d.y;
        if (std::abs(d.x) > lim)
        {
            d.x = d.x < 0.f ? -lim : lim;
        }
        const float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 0.0001f)
        {
            d = { 0.f, 1.f };
        }
        else
        {
            d = { d.x / len, d.y / len };
        }

        ShotSpawn spawn;
        spawn.team            = eTeam::Enemy;
        spawn.owner           = _e.id;
        spawn.position        = pos;
        spawn.velocity        = { d.x * c.bulletSpeed, d.y * c.bulletSpeed };
        spawn.size            = proj.size;
        spawn.damage          = proj.damage;
        spawn.kind            = eHitKind::Bullet;
        spawn.gravity         = 0.f;
        spawn.maxFallSpeed    = proj.maxFall;
        spawn.lifetimeTicks   = SecToTicks(proj.lifetimeSec);
        spawn.bExplodeOnSolid = proj.bExplodeOnSolid;
        spawn.visual          = static_cast<uint16_t>(projIndex + 1);
        spawn.tag             = static_cast<uint8_t>(_bin);
        _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
        ++_e.shotsFired;
    }

    void FireTurret_(
        Enemy&               _e,
        const EnemyDef&      _def,
        EnemyEnv&            _env,
        const int            _i)
    {
        const BattleShipCfg& c    = _def.battleShip;
        int*                 t    = Turret_(_e, _i);
        const int            dir  = std::clamp(t[kTurretDir], 0, 2);
        const Vec2   to   = TurretOriginOf_(_e, c, _i);
        const Vec2   muzzle = { to.x + c.turretMuzzle[dir].x, to.y + c.turretMuzzle[dir].y };

        EnemyFxRequest fx;
        fx.clip       = kDirFxNames[dir];
        fx.position   = muzzle;
        fx.delayTicks = 0;
        _env.world.RequestFx(fx);

        if (!_env.ctx.pProjectiles || !_env.ctx.pCombat || !_env.ctx.player.bAlive)
        {
            return;
        }
        const int projIndex = _env.lib.FindProjectileIndex(c.turretProjectile);
        if (projIndex < 0)
        {
            return;
        }
        const ProjectileDef& proj = _env.lib.GetProjectile(projIndex);
        const float          vy0  = proj.v0.y;
        const float          g    = proj.gravity > 0.f ? proj.gravity : 1.f;

        const int   jitter  = static_cast<int>(c.turretAimJitter);
        const float targetX = _env.ctx.player.feet.x + static_cast<float>(Roll(_env, 2 * jitter + 1) - jitter);
        const float targetY = _env.ctx.player.feet.y;
        const float dy      = targetY - muzzle.y;
        const float disc    = vy0 * vy0 + 2.f * g * std::max(dy, -30.f);
        float       flight  = disc >= 0.f ? (-vy0 + std::sqrt(disc)) / g : -vy0 / g;
        flight              = std::max(flight, 0.1f);
        const float dx      = targetX - muzzle.x;
        const float speedX  = std::clamp(std::abs(dx) / flight, c.turretArcMinVx, c.turretArcMaxVx);

        ShotSpawn spawn;
        spawn.team            = eTeam::Enemy;
        spawn.owner           = _e.id;
        spawn.position        = muzzle;
        spawn.velocity        = { dx < 0.f ? -speedX : speedX, vy0 };
        spawn.gravity         = g;
        spawn.size            = proj.size;
        spawn.damage          = proj.damage;
        spawn.kind            = eHitKind::Bullet;
        spawn.maxFallSpeed    = proj.maxFall;
        spawn.lifetimeTicks   = SecToTicks(proj.lifetimeSec);
        spawn.bExplodeOnSolid = proj.bExplodeOnSolid;
        spawn.visual          = static_cast<uint16_t>(projIndex + 1);
        _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
        ++_e.shotsFired;
    }

    [[nodiscard]] int DesiredDir_(
        const EnemyEnv&      _env,
        const BattleShipCfg& _c,
        const Vec2   _turretOrigin)
    {
        const float dx = _env.ctx.player.feet.x - _turretOrigin.x;
        if (std::abs(dx) <= _c.turretMidDx)
        {
            return 1;
        }
        return dx < 0.f ? 0 : 2;
    }

    void TickTurret_(
        Enemy&               _e,
        const EnemyDef&      _def,
        EnemyEnv&            _env,
        const int            _i)
    {
        const BattleShipCfg& c = _def.battleShip;
        int*                 t = Turret_(_e, _i);
        if (t[kTurretHp] <= 0)
        {
            return;
        }
        if (t[kTurretAction] != kActNone)
        {
            if (--t[kTurretBusy] > 0)
            {
                return;
            }
            if (t[kTurretAction] == kActTurnLeft || t[kTurretAction] == kActTurnRight)
            {
                t[kTurretDir] = std::clamp(t[kTurretDir] + (t[kTurretAction] == kActTurnRight ? 1 : -1), 0, 2);
                t[kTurretCool] = std::max(0, t[kTurretCool] - SecToTicks(0.25f));   // 회전 후 쿨타임 -0.25 s 로 재판단
            }
            t[kTurretAction] = kActNone;
            t[kTurretBusy]   = 0;
            return;
        }

        const int desired = DesiredDir_(_env, c, TurretOriginOf_(_e, c, _i));
        if (t[kTurretDir] != desired)
        {
            t[kTurretAction] = desired > t[kTurretDir] ? kActTurnRight : kActTurnLeft;
            t[kTurretBusy]   = SecToTicks(c.turretTurnSec);
            return;
        }
        if (t[kTurretCool] > 0)
        {
            --t[kTurretCool];
            return;
        }
        t[kTurretAction] = kActShot;
        t[kTurretBusy]   = SecToTicks(c.turretShotSec);
        FireTurret_(_e, _def, _env, _i);
        const float cool = IsEnragedImpl_(_e) ? c.turretCoolEnragedSec : c.turretCoolSec;
        t[kTurretCool]   = SecToTicks(cool) + Roll(_env, std::max(1, SecToTicks(c.turretRandomSec)));
    }

    void KillTurret_(
        Enemy&               _e,
        const EnemyDef&      _def,
        EnemyEnv&            _env,
        const int            _i)
    {
        int*               t = Turret_(_e, _i);
        t[kTurretHp]       = 0;
        t[kTurretAction]   = kActNone;
        t[kTurretBusy]     = 0;
        const Vec2 at = TurretOriginOf_(_e, _def.battleShip, _i);
        const char*        clips[3] = { "Explosion_Small", "Explosion_Mid", "Explosion_Large" };
        for (const char* clip: clips)
        {
            EnemyFxRequest fx;
            fx.clip     = clip;
            fx.position = { at.x + static_cast<float>(Roll(_env, 31) - 15), at.y + static_cast<float>(Roll(_env, 31) - 15) };
            _env.world.RequestFx(fx);
        }
    }

    void HullExplosion_(
        const Enemy&    _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const char*     _clip)
    {
        const Vec2 o  = OriginOf_(_e, _def.battleShip);
        const float        dx = kHullMinX + static_cast<float>(Roll(_env, static_cast<int>(kHullMaxX - kHullMinX) + 1));
        const float        dy = kHullMinY + static_cast<float>(Roll(_env, static_cast<int>(kHullMaxY - kHullMinY) + 1));
        EnemyFxRequest     fx;
        fx.clip     = _clip;
        fx.position = { o.x + dx, o.y + dy };
        _env.world.RequestFx(fx);
    }

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_env;
        const BattleShipCfg& c = _def.battleShip;
        PrepareBody(_e, _def);
        _e.facing = 1;
        SetAngle_(_e, c.cannonAngleStart);
        _e.ext[kExtRailSlot]   = c.homeSlot;
        _e.ext[kExtRailTarget] = c.homeSlot;
        _e.ext[kExtPatterns]   = 0;
        for (int i = 0; i < kTurretCount; ++i)
        {
            int* t = Turret_(_e, i);
            t[kTurretHp]     = c.turretHp;
            t[kTurretDir]    = 1;   // 가운데
            t[kTurretCool]   = SecToTicks(c.turretFirstStepSec * static_cast<float>(i));
            t[kTurretBusy]   = 0;
            t[kTurretAction] = kActNone;
        }
        if (!_e.arena.IsEmpty())
        {
            _e.body.position.x = _e.arena.min.x + c.entranceOffsetX;
            EnterState(_e, kEntrance);
        }
        else
        {
            _e.ext[kExtFightStarted] = 1;   // 아레나 없이 스폰: 즉시 전투
            EnterState(_e, kIdle);
        }
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BattleShipCfg& c        = _def.battleShip;
        const bool           bEnraged = IsEnragedImpl_(_e);
        _e.facing                     = 1;

        if (_e.ext[kExtFightStarted] != 0)
        {
            for (int i = 0; i < kTurretCount; ++i)
            {
                TickTurret_(_e, _def, _env, i);
            }
        }

        switch (_e.state)
        {
            case kEntrance:
                if (RailStep_(_e, c, SlotX_(_e, c, c.homeSlot)))
                {
                    _e.ext[kExtRailSlot]     = c.homeSlot;
                    _e.ext[kExtFightStarted] = 1;
                    EnterState(_e, kIdle);
                }
                break;

            case kIdle:
                if (_e.stateTicks >= (bEnraged ? c.idleEnragedTicks : c.idleTicks))
                {
                    const int pattern = ++_e.ext[kExtPatterns];
                    if (pattern % 2 == 1)
                    {
                        EnterState(_e, kHeadOpen);
                    }
                    else
                    {
                        _e.ext[kExtRailTarget] = PickOtherSlot_(_env, CurrentSlot_(_e));
                        EnterState(_e, kRailMove);
                    }
                }
                break;

            case kRailMove:
                if (RailStep_(_e, c, SlotX_(_e, c, _e.ext[kExtRailTarget])))
                {
                    _e.ext[kExtRailSlot] = _e.ext[kExtRailTarget];
                    EnterState(_e, kIdle);
                }
                break;

            case kHeadOpen:
                if (_e.stateTicks >= c.headOpenTicks)
                {
                    _e.ext[kExtShotsLeft] = bEnraged ? c.cannonShotsEnraged : c.cannonShots;
                    SetAngle_(_e, c.cannonAngleStart);
                    EnterState(_e, kCannonStart);
                }
                break;

            case kCannonStart:
                TrackCannon_(_e, c, _env);
                if (_e.stateTicks >= c.cannonStartTicks)
                {
                    EnterState(_e, kCannonIdle);
                }
                break;

            case kCannonIdle:
                TrackCannon_(_e, c, _env);
                if (_e.stateTicks >= (bEnraged ? c.cannonIdleEnragedTicks : c.cannonIdleTicks))
                {
                    EnterState(_e, kCannonShot);
                    FireCannonBullet_(_e, _def, _env, AngleBin_(static_cast<float>(_e.ext[kExtAngle]) / 100.f, c));
                    --_e.ext[kExtShotsLeft];
                }
                break;

            case kCannonShot:
                TrackCannon_(_e, c, _env);
                if (_e.stateTicks >= SecToTicks(c.cannonShotSec))
                {
                    EnterState(_e, _e.ext[kExtShotsLeft] > 0 ? kCannonIdle : kCannonEnd);
                }
                break;

            case kCannonEnd:
                if (_e.stateTicks >= c.cannonEndTicks)
                {
                    EnterState(_e, kHeadClose);
                }
                break;

            case kHeadClose:
                if (_e.stateTicks >= c.headCloseTicks)
                {
                    EnterState(_e, kIdle);
                }
                break;

            default: break;
        }

        if (bEnraged && _e.ext[kExtFightStarted] != 0)
        {
            if (++_e.ext[kExtEnragedTimer] >= c.enragedSmallTicks)
            {
                _e.ext[kExtEnragedTimer] = 0;
                HullExplosion_(_e, _def, _env, "Explosion_Small");
            }
        }

        _env.world.RequestPlayerMinX(_e.body.position.x + c.hullBlockDx);
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
            return false;
        }
        if (part >= kTurretCount)
        {
            return true;
        }
        int* t = Turret_(_e, part);
        if (t[kTurretHp] <= 0)
        {
            return true;
        }
        t[kTurretHp] = std::max(0, t[kTurretHp] - _event.damage);
        if (t[kTurretHp] == 0)
        {
            KillTurret_(_e, _def, _env, part);
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
            if (Turret_(_e, i)[kTurretHp] > 0)
            {
                KillTurret_(_e, _def, _env, i);
            }
        }
        _e.deathTicks = 0;
    }

    void TickDying_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BattleShipCfg& c         = _def.battleShip;
        const int            finish    = SecToTicks(c.destroyFinishSec);
        const int            chainEnd  = SecToTicks(c.destroyChainSec);
        const int            step      = std::max(1, SecToTicks(c.destroyChainStepSec));
        ++_e.deathTicks;

        if (_e.deathTicks < finish)
        {
            _env.world.RequestPlayerMinX(_e.body.position.x + c.hullBlockDx);
        }
        if (_e.deathTicks <= chainEnd && _e.deathTicks % step == 0)
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
        if (_e.deathTicks == chainEnd)
        {
            const Vec2 o = OriginOf_(_e, c);
            EnemyFxRequest     flash;
            flash.clip     = "Explosion_Huge";
            flash.position = { o.x - 30.f, o.y - 50.f };
            _env.world.RequestFx(flash);
            for (int i = 0; i < 6; ++i)
            {
                HullExplosion_(_e, _def, _env, "Explosion_Mid");
                HullExplosion_(_e, _def, _env, "Explosion_Large");
            }
        }
    }

    bool DeathDone_(
        const Enemy&    _e,
        const EnemyDef& _def)
    {
        return _e.deathTicks >= SecToTicks(_def.battleShip.destroyFinishSec);
    }

    void RegisterHurtboxes_(
        const Enemy&    _e,
        const EnemyDef& _def,
        CombatSystem&   _combat)
    {
        const BattleShipCfg& c = _def.battleShip;
        const Vec2   o = OriginOf_(_e, c);

        Hurtbox bow;
        bow.box     = c.bow.At(o, 1);
        bow.team    = _def.team;
        bow.owner   = _e.id;
        bow.accepts = AcceptsFor(_def.bHuman);
        _combat.AddHurtbox(bow);

        Hurtbox bridge = bow;
        bridge.box     = c.bridge.At(o, 1);
        _combat.AddHurtbox(bridge);

        Hurtbox aft = bow;
        aft.box     = c.aft.At(o, 1);
        _combat.AddHurtbox(aft);

        for (int i = 0; i < kTurretCount; ++i)
        {
            if (_e.ext[kTurretBase + i * kTurretStride + kTurretHp] <= 0)
            {
                continue;
            }
            Hurtbox turret;
            turret.box   = c.turretBox.At(TurretOriginOf_(_e, c, i), 1);
            turret.team  = _def.team;
            turret.owner   = MakePartOwner(_e.id, static_cast<uint32_t>(i));
            turret.accepts = AcceptsFor(_def.bHuman);
            _combat.AddHurtbox(turret);
        }
    }

    void SubmitRail_(
        const Enemy&          _e,
        const BattleShipCfg& _c,
        const EnemyRenderEnv& _r,
        const Vec2    _at,
        const int             _sort,
        const bool            _bFront)
    {
        const float vx   = _e.body.velocity.x;
        const bool  bFast = std::abs(vx) > 0.7f * _c.railMaxSpeed;
        const char* name  = nullptr;
        if (std::abs(vx) < 1.f)
        {
            name = _bFront ? "BattleShip_Front_Rail_Idle" : "BattleShip_Back_Rail_Idle";
        }
        else if (_bFront)
        {
            name = vx > 0.f ? (bFast ? "BattleShip_Front_Rail_Forward_Fast" : "BattleShip_Front_Rail_Forward")
                            : (bFast ? "BattleShip_Front_Rail_Backward_Fast" : "BattleShip_Front_Rail_Backward");
        }
        else
        {
            name = vx > 0.f ? (bFast ? "BattleShip_Back_Rail_Forward_Fast" : "BattleShip_Back_Rail_Forward")
                            : (bFast ? "BattleShip_Back_Rail_Backward_Fast" : "BattleShip_Back_Rail_Backward");
        }
        SubmitNamedPart(_r, name, _at, _e.clipTicks, true, _sort, _r.layer);
    }

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _r)
    {
        const BattleShipCfg& c    = _def.battleShip;
        const Vec2   o    = { _r.feet.x, _r.feet.y - c.originDepth };
        const int            sort = static_cast<int>(_r.feet.y);
        const int            layer = _r.layer;
        const int            chainEnd = SecToTicks(c.destroyChainSec);

        if (_e.life == eEnemyLife::Dying && _e.deathTicks >= chainEnd)
        {
            SubmitNamedPart(_r, "BattleShip_Destroy", { o.x - 30.f, o.y - 52.f }, kPersistTicks, false, sort, layer);
            return;
        }

        const int  state = _e.state;
        const bool bCannonPhase = state == kCannonStart || state == kCannonIdle || state == kCannonShot || state == kCannonEnd;
        const int  sinceState   = _e.stateTicks;

        SubmitNamedPart(_r, "BattleShip_Tail", { o.x - 213.f, o.y + 52.f }, _e.clipTicks, true, sort, layer);
        SubmitNamedPart(_r, "BattleShip_Body", { o.x - 57.f, o.y - 114.f }, _e.clipTicks, true, sort, layer);
        SubmitNamedPart(_r, "BattleShip_Cannon_Deco", { o.x + 55.f, o.y - 24.f }, kPersistTicks, false, sort, layer);

        const Vec2 head   = { o.x + 131.f, o.y + 5.f };
        const Vec2 cannon = { o.x + c.cannonOffset.x, o.y + c.cannonOffset.y };
        const float        angle  = static_cast<float>(_e.ext[kExtAngle]) / 100.f;
        if (state == kHeadOpen)
        {
            SubmitNamedPart(_r, "BattleShip_Head_Swiching_Start", head, sinceState, false, sort, layer);
        }
        else if (state == kHeadClose)
        {
            SubmitNamedPart(_r, "BattleShip_Head_Swiching_End", head, sinceState, false, sort, layer);
        }
        else if (!bCannonPhase)
        {
            SubmitNamedPart(_r, "BattleShip_Head", head, _e.clipTicks, true, sort, layer);
        }
        if (state == kCannonStart)
        {
            SubmitNamedPart(_r, "BattleShip_Cannon_Start", cannon, sinceState, false, sort, layer);
        }
        else if (state == kCannonIdle)
        {
            SubmitNamedPart(_r, kCannonIdleNames[AngleFrame_(angle, c)], cannon, _e.clipTicks, true, sort, layer);
        }
        else if (state == kCannonShot)
        {
            SubmitNamedPart(_r, kCannonShotNames[AngleBin_(angle, c)], cannon, sinceState, false, sort, layer);
        }
        else if (state == kCannonEnd)
        {
            SubmitNamedPart(_r, "BattleShip_Cannon_End", cannon, sinceState, false, sort, layer);
        }

        const bool bAntennaTurn = state == kHeadOpen || state == kHeadClose;
        SubmitNamedPart(_r, bAntennaTurn ? "BattleShip_Antena_Turn" : "BattleShip_Antena", { o.x + 2.f, o.y - 217.f }, _e.clipTicks, true, sort, layer);

        SubmitRail_(_e, c, _r, { o.x + 226.f, o.y + 66.f }, sort, false);
        SubmitRail_(_e, c, _r, { o.x - 67.f, o.y + 63.f }, sort, true);

        for (int i = 0; i < kTurretCount; ++i)
        {
            const int*         t  = &_e.ext[kTurretBase + i * kTurretStride];
            const Vec2 to = { o.x + c.turretOffset[i].x, o.y + c.turretOffset[i].y };
            if (t[kTurretHp] <= 0)
            {
                SubmitNamedPart(_r, "BattleShip_Cannon_Destroy_Type5", to, kPersistTicks, false, sort, layer);
                continue;
            }
            const int dir = std::clamp(t[kTurretDir], 0, 2);
            switch (t[kTurretAction])
            {
                case kActTurnLeft:
                    SubmitNamedPart(_r, "BattleShip_DCannon_TurnLeft_0", to, SecToTicks(c.turretTurnSec) - t[kTurretBusy], false, sort + 1, layer);
                    break;
                case kActTurnRight:
                    SubmitNamedPart(_r, "BattleShip_DCannon_TurnRight_0", to, SecToTicks(c.turretTurnSec) - t[kTurretBusy], false, sort + 1, layer);
                    break;
                case kActShot:
                    SubmitNamedPart(_r, kTurretShotNames[dir], to, SecToTicks(c.turretShotSec) - t[kTurretBusy], false, sort + 1, layer);
                    break;
                default:
                    SubmitNamedPart(_r, kTurretIdleNames[dir], to, _e.clipTicks, true, sort + 1, layer);
                    break;
            }
        }

        for (const Shot& shot: _r.projectiles.GetShots())
        {
            if (!shot.bAlive || shot.params.owner != _e.id)
            {
                continue;
            }
            const Vec2 p = { shot.renderPrev.x + (shot.position.x - shot.renderPrev.x) * _r.alpha, shot.renderPrev.y + (shot.position.y - shot.renderPrev.y) * _r.alpha };
            SubmitNamedPart(_r, kBulletNames[std::clamp(static_cast<int>(shot.params.tag), 0, 4)], p, 0, true, static_cast<int>(p.y), kBulletLayer);
        }
    }
}

const BehaviorOps kBattleShipOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = &RegisterHurtboxes_,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
};

namespace battleship
{

bool IsEnraged(const Enemy& _e)
{
    return IsEnragedImpl_(_e);
}

bool IsFightStarted(const Enemy& _e)
{
    return _e.ext[kExtFightStarted] != 0;
}

int GetPatternCount(const Enemy& _e)
{
    return _e.ext[kExtPatterns];
}

int GetTurretHp(const Enemy& _e, const int _index)
{
    return _e.ext[kTurretBase + _index * kTurretStride + kTurretHp];
}

float GetCannonAngle(const Enemy& _e)
{
    return static_cast<float>(_e.ext[kExtAngle]) / 100.f;
}

Vec2 GetOrigin(const Enemy& _e, const EnemyDef& _def)
{
    return OriginOf_(_e, _def.battleShip);
}
}

void ParseBattleShipCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BattleShipCfg& c = _def.battleShip;

    c.originDepth     = _r.Num("originDepth");
    c.entranceOffsetX = _r.Num("entranceOffsetX");
    (void)_r.NumArray("railSlots", c.railSlot, 3);
    c.homeSlot        = _r.Int("homeSlot");
    c.railMaxSpeed    = _r.Num("railMax");
    c.railAccel       = _r.Num("railAccel");
    c.railDecel       = _r.Num("railDecel");

    c.idleTicks            = _r.Ticks("idleSec");
    c.idleEnragedTicks     = _r.Ticks("idleEnragedSec");
    c.headOpenTicks        = _r.Ticks("headOpenSec");
    c.cannonStartTicks     = _r.Ticks("cannonStartSec");
    c.cannonIdleTicks      = _r.Ticks("cannonIdleSec");
    c.cannonIdleEnragedTicks = _r.Ticks("cannonIdleEnragedSec");
    c.cannonShots          = _r.Int("cannonShots");
    c.cannonShotsEnraged   = _r.Int("cannonShotsEnraged");
    c.cannonEndTicks       = _r.Ticks("cannonEndSec");
    c.headCloseTicks       = _r.Ticks("headCloseSec");
    c.cannonShotSec        = _r.Num("cannonShotSec");
    c.cannonTrackDeg       = _r.Num("cannonTrackDeg");
    c.cannonTrackDegEnraged = _r.Num("cannonTrackDegEnraged");
    c.cannonAngleStart     = _r.Num("cannonAngleStart");
    c.cannonAngleMin       = _r.Num("cannonAngleMin");
    c.cannonAngleMax       = _r.Num("cannonAngleMax");
    c.cannonOffset         = _r.ReadVec2("cannonOffset", { 123.f, -149.f });
    c.bulletSpeed          = _r.Num("bulletSpeed");
    c.bulletSpread         = _r.Num("bulletSpread");
    c.bulletProjectile     = _r.Str("bulletProjectile");

    c.bow    = _r.Box("bow");
    c.bridge = _r.Box("bridge");
    c.aft    = _r.Box("aft");
    c.hullBlockDx = _r.Num("hullBlockDx");

    c.turretOffset[0] = _r.ReadVec2("turret0Offset", { -200.f, -70.f });
    c.turretOffset[1] = _r.ReadVec2("turret1Offset", { -100.f, -120.f });
    c.turretBox       = _r.Box("turretBox");
    {
        float muzzle[6] = { -30.f, -30.f, 0.f, -35.f, 30.f, -30.f };
        (void)_r.NumArray("turretMuzzle", muzzle, 6);
        for (int i = 0; i < 3; ++i)
        {
            c.turretMuzzle[i] = { muzzle[i * 2], muzzle[i * 2 + 1] };
        }
    }
    c.turretHp             = _r.Int("turretHp");
    c.turretCoolSec        = _r.Num("turretCoolSec");
    c.turretCoolEnragedSec = _r.Num("turretCoolEnragedSec");
    c.turretFirstStepSec   = _r.Num("turretFirstStepSec");
    c.turretTurnSec        = _r.Num("turretTurnSec");
    c.turretShotSec        = _r.Num("turretShotSec");
    c.turretMidDx          = _r.Num("turretMidDx");
    c.turretAimJitter      = _r.Num("turretAimJitter");
    c.turretRandomSec      = _r.Num("turretRandomSec");
    c.turretArcMinVx       = _r.Num("turretArcMinVx");
    c.turretArcMaxVx       = _r.Num("turretArcMaxVx");
    c.turretProjectile     = _r.Str("turretProjectile");

    c.enragedSmallTicks  = _r.Ticks("enragedSmallSec");
    c.destroyChainSec    = _r.Num("destroyChainSec");
    c.destroyChainStepSec = _r.Num("destroyChainStepSec");
    c.destroyFinishSec   = _r.Num("destroyFinishSec");
}
}

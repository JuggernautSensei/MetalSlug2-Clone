#include <algorithm>
#include <cmath>

#include "World/Constants.h"
#include "Game/Enemies/BossShiee.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    using namespace bossshiee;

    constexpr int   kPersistTicks  = 1 << 20;   // 끝난 비루프 연출은 마지막 프레임에 머문다
    constexpr int   kAimBins       = 9;         // Gun_Aim 프레임 수 (-135..-45 도, 11.25 도 간격)
    constexpr int   kAimFrameTicks = 5;         // Gun_Aim 프레임 간격 (12 fps)
    constexpr float kAimMinDeg     = -135.f;
    constexpr float kAimMaxDeg     = -45.f;
    constexpr float kRadToDeg      = 57.29578f;
    constexpr float kDegToRad      = 0.0174533f;
    constexpr float kChainHalfW    = 230.f;     // 선체 연쇄 폭발 범위 (원점 기준 반폭)
    constexpr float kChainHalfH    = 150.f;     // 원점 기준 반높이 (위 방향)

    constexpr int kKindNone   = 0;
    constexpr int kKindTracer = 1;   // 트레이서 (직선, 발사 시점 플레이어 조준 + 부채꼴 오프셋)
    constexpr int kKindShell  = 2;   // 주황 포탄 (포물선, 착탄 시 blast 예약)
    constexpr int kKindBlast  = 3;   // 착탄 폭발 상자 (60x60, 0.3 s)

    constexpr int kCannonSource = kBossCannonPart;   // 발사 원천 번호 (포탑 0..6, 캐논 7)

    // 렌더 레이어: 선체는 모든 동적 객체 뒤 (배경 바로 위), 포탑/캐논 파츠는 선체 위 + 소품 (12) 아래. 플레이어 20, 탄 30 이상은 항상 앞.
    constexpr int   kBossHullLayer   = 2;
    constexpr int   kBossPartLayer   = 11;
    constexpr float kEntranceShakeAmp = 2.5f;   // 등장 흔들림 진폭 (px)
    constexpr float kRumbleJitterPx   = 1.5f;   // 등장 중 선체 떨림 (px)
    constexpr int   kRumbleJitterTicks = 3;     // 떨림 주기 (틱)

    constexpr const char* kTurretIdleClip[kTurretCount]      = { "Boss_Turret_A_Idle", "Boss_Turret_B_Idle", "Boss_Turret_C_Idle", "Boss_Turret_D_Idle", "Boss_Turret_E_Idle", "Boss_Turret_F_Idle", "Boss_Turret_G_Idle" };
    constexpr const char* kTurretTurningClip[kTurretCount]   = { "Boss_Turret_A_Turning", "Boss_Turret_B_Turning", "Boss_Turret_C_Turning", "Boss_Turret_D_Turning", "Boss_Turret_E_Turning", "Boss_Turret_F_Turning", "Boss_Turret_G_Turning" };
    constexpr const char* kTurretShootingClip[kTurretCount]  = { "Boss_Turret_A_Shooting", "Boss_Turret_B_Shooting", "Boss_Turret_C_Shooting", "Boss_Turret_D_Shooting", "Boss_Turret_E_Shooting", "Boss_Turret_F_Shooting", "Boss_Turret_G_Shooting" };
    constexpr const char* kTurretWreckClip[kTurretCount] = { "Boss_Turret_A_Destroyed", "Boss_Turret_B_Destroyed", "Boss_Turret_C_Destroyed", "Boss_Turret_D_Destroyed", "Boss_Turret_E_Destroyed", "Boss_Turret_F_Destroyed", "Boss_Turret_G_Destroyed" };
    constexpr const char* kGunShoot[5]                   = { "Boss_Gun_Shoot_1", "Boss_Gun_Shoot_2", "Boss_Gun_Shoot_3", "Boss_Gun_Shoot_4", "Boss_Gun_Shoot_5" };
    constexpr const char* kDeployClips[4]                = { "Boss_Cannon_Deploy1", "Boss_Cannon_Deploy3", "Boss_Cannon_Deploy6", "Boss_Cannon_Deploy9" };
    constexpr int         kDeployStageTicks              = 40;   // 4 단계 x 40 틱 (= deployTicks 160)

    // ---------------------------------------------------------------- 공용 도우미

    [[nodiscard]] Vec2 OriginOf_(
        const Enemy&       _e,
        const BossShieeCfg& _c)
    {
        return { _e.body.position.x, _e.body.position.y - _c.originDepth };
    }

    [[nodiscard]] Vec2 TurretCentreOf_(
        const Enemy&        _e,
        const BossShieeCfg& _c,
        const int           _i)
    {
        const Vec2 o = OriginOf_(_e, _c);
        return { o.x + _c.turretOffset[_i].x, o.y + _c.turretOffset[_i].y };
    }

    [[nodiscard]] Vec2 MuzzleOf_(
        const Enemy&        _e,
        const BossShieeCfg& _c,
        const int           _source)
    {
        if (_source == kCannonSource)
        {
            const Vec2 o = OriginOf_(_e, _c);
            return { o.x + _c.cannonMuzzle.x, o.y + _c.cannonMuzzle.y };
        }
        const Vec2 t = TurretCentreOf_(_e, _c, _source);
        return { t.x + _c.turretMuzzle.x, t.y + _c.turretMuzzle.y };
    }

    [[nodiscard]] bool Enraged_(
        const Enemy& _e)
    {
        return _e.health.hp * 2 <= _e.health.maxHp;
    }

    [[nodiscard]] bool CannonDead_(
        const Enemy& _e)
    {
        return (_e.ext[kExtFlags] & 2) != 0;
    }

    [[nodiscard]] int TurretHp_(
        const Enemy& _e,
        const int    _i)
    {
        return _e.ext[kExtTurret + _i] & 0xFF;
    }

    [[nodiscard]] int TurretPhase_(
        const Enemy& _e,
        const int    _i)
    {
        return (_e.ext[kExtTurret + _i] >> 8) & 0x3;
    }

    [[nodiscard]] int TurretTicks_(
        const Enemy& _e,
        const int    _i)
    {
        return (_e.ext[kExtTurret + _i] >> 10) & 0x3FF;
    }

    void SetTurret_(
        Enemy&    _e,
        const int _i,
        const int _hp,
        const int _phase,
        const int _ticks)
    {
        _e.ext[kExtTurret + _i] = std::clamp(_hp, 0, 255) | (std::clamp(_phase, 0, 3) << 8) | (std::clamp(_ticks, 0, 1023) << 10);
    }

    [[nodiscard]] int IntervalTicks_(
        EnemyEnv& _env,
        const int _minTicks,
        const int _maxTicks)
    {
        return _minTicks + Roll(_env, std::max(1, _maxTicks - _minTicks + 1));
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
        Enemy&    _e,
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

    void Fx_(
        EnemyEnv&   _env,
        const char* _clip,
        const Vec2  _at,
        const int   _delayTicks = 0)
    {
        EnemyFxRequest request;
        request.clip       = _clip;
        request.position   = _at;
        request.delayTicks = _delayTicks;
        _env.world.RequestFx(request);
    }

    [[nodiscard]] Vec2 Scatter_(
        EnemyEnv&   _env,
        const Vec2  _c,
        const float _halfW,
        const float _halfH)
    {
        const int   spanX = static_cast<int>(_halfW) * 2 + 1;
        const int   spanY = static_cast<int>(_halfH) * 2 + 1;
        const float dx    = static_cast<float>(Roll(_env, spanX) - static_cast<int>(_halfW));
        const float dy    = static_cast<float>(Roll(_env, spanY) - static_cast<int>(_halfH));
        return { _c.x + dx, _c.y + dy };
    }

    // 선체 위의 무작위 지점 (원점 기준 상자 안)
    [[nodiscard]] Vec2 HullPoint_(
        EnemyEnv&           _env,
        const Enemy&        _e,
        const BossShieeCfg& _c)
    {
        const Vec2 o = OriginOf_(_e, _c);
        return Scatter_(_env, { o.x, o.y + 20.f }, kChainHalfW, kChainHalfH);
    }

    // 파츠 파괴 점수 (스코어 이벤트만 남긴다: 적 사망/게이트와 무관)
    void PushPartScore_(
        Enemy&       _e,
        EnemyEnv&    _env,
        const int    _score,
        const Vec2   _at)
    {
        EnemyEvent event = _env.world.MakeEvent(_e, eEnemyEventKind::PartDestroyed);
        event.score      = _score;
        event.pos        = _at;
        _env.world.PushEvent(event);
    }

    void KillTurret_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _i,
        const bool      _bScore)
    {
        const BossShieeCfg& c  = _def.bossShiee;
        const Vec2          at = TurretCentreOf_(_e, c, _i);
        SetTurret_(_e, _i, 0, kTurretIdle, 0);
        Fx_(_env, "Explosion_Small", Scatter_(_env, at, 14.f, 12.f));
        Fx_(_env, "Explosion_Mid", Scatter_(_env, at, 14.f, 12.f), 10);
        if (_bScore)
        {
            PushPartScore_(_e, _env, c.turretScore, at);
        }
    }

    // 파괴된 포탑 수 (A..G)
    [[nodiscard]] int CountTurretsDown_(
        const Enemy& _e)
    {
        int n = 0;
        for (int i = 0; i < kTurretCount; ++i)
        {
            n += TurretHp_(_e, i) <= 0 ? 1 : 0;
        }
        return n;
    }

    [[nodiscard]] int CountTurretsAlive_(
        const Enemy& _e)
    {
        return kTurretCount - CountTurretsDown_(_e);
    }

    // 사격 대상 방향 (플레이어 중심) + 부채꼴 오프셋 (_spread 칸, 1 칸 = streakSpreadDeg)
    [[nodiscard]] Vec2 AimDir_(
        const Vec2  _from,
        const EnemyEnv& _env,
        const float _spreadDeg)
    {
        Vec2       d   = { _env.ctx.player.center.x - _from.x, _env.ctx.player.center.y - _from.y };
        const float len = std::sqrt(d.x * d.x + d.y * d.y);
        d = len < 0.0001f ? Vec2 { -1.f, 0.f } : Vec2 { d.x / len, d.y / len };
        if (_spreadDeg != 0.f)
        {
            const float a = _spreadDeg * kDegToRad;
            const float c = std::cos(a);
            const float s = std::sin(a);
            d             = { d.x * c - d.y * s, d.x * s + d.y * c };
        }
        return d;
    }

    // 트레이서 1발 (포탑 또는 캐논 포구). 발사 시점 플레이어 중심을 조준 (리드 없음), _spreadIdx 로 부채꼴.
    void FireTracer_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _source,
        const int       _spreadIdx)
    {
        const BossShieeCfg& c    = _def.bossShiee;
        const Vec2          from = MuzzleOf_(_e, c, _source);
        if (_source == kCannonSource)
        {
            Fx_(_env, "Boss_Fire_EFG_1", from);
            _e.ext[kExtLastShot] = _e.aux + 1;
        }
        else
        {
            Fx_(_env, _source < 4 ? "Boss_Fire_ABCD_1" : "Boss_Fire_EFG_1", from);
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
        const Vec2           d    = AimDir_(from, _env, c.streakSpreadDeg * static_cast<float>(_spreadIdx));

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

    // 주황 포탄: 비행 시간 T 고정. 착탄점 = 발사 시점 플레이어 발 x. 착탄 틱을 같은 이산 적분으로 미리 구해 blast 를 예약한다.
    void FireShell_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BossShieeCfg& c    = _def.bossShiee;
        const Vec2          from = MuzzleOf_(_e, c, kCannonSource);
        Fx_(_env, "Boss_Fire_EFG_1", from);
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

        const float target = _env.ctx.player.feet.x;
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
            const int dx = static_cast<int>(std::lround(target - _e.body.position.x));
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
        const int projIndex = _env.lib.FindProjectileIndex(_def.bossShiee.blastProjectile);
        if (projIndex < 0)
        {
            return;
        }
        const ProjectileDef& proj = _env.lib.GetProjectile(projIndex);
        const float          x    = _e.body.position.x + static_cast<float>(_dx);
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
                    FireTracer_(_e, _def, _env, ItemSource_(item), ItemDx_(item));
                    break;
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

    // 캐논 조준 구간 (목표 방향을 -135..-45 도로 나눈 9 구간)
    [[nodiscard]] int AimBinOf_(
        const Enemy&        _e,
        const BossShieeCfg& _c,
        const EnemyEnv&     _env)
    {
        const Vec2  from  = MuzzleOf_(_e, _c, kCannonSource);
        const float angle = std::atan2(_env.ctx.player.center.y - from.y, _env.ctx.player.center.x - from.x) * kRadToDeg;
        const float t     = (std::clamp(angle, kAimMinDeg, kAimMaxDeg) - kAimMinDeg) / (kAimMaxDeg - kAimMinDeg);
        return std::clamp(static_cast<int>(std::lround(t * static_cast<float>(kAimBins - 1))), 0, kAimBins - 1);
    }

    // ---------------------------------------------------------------- 상태 틱

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BossShieeCfg& c = _def.bossShiee;
        PrepareBody(_e, _def);
        _e.facing = 1;
        for (int i = 0; i < kExtQueueCount; ++i)
        {
            _e.ext[kExtQueue + i] = 0;
        }
        for (int i = 0; i < kTurretCount; ++i)
        {
            SetTurret_(_e, i, c.turretHp, kTurretIdle, 0);
        }
        _e.ext[kExtFlags]       = 0;
        _e.ext[kExtScrollTimer] = 0;
        _e.ext[kExtScrollLeft]  = static_cast<int>(std::lround(c.scrollBudget * 1000.f));
        _e.ext[kExtNextTurret]  = 0;
        _e.ext[kExtNextVolley]  = 0;
        _e.ext[kExtVolleyCount] = 0;
        _e.ext[kExtAimStart]    = 0;
        _e.ext[kExtAimBin]      = kAimBins / 2;
        _e.ext[kExtLastShot]    = 0;
        _e.aux                  = 0;
        _e.body.velocity.x      = 0.f;

        if (!_e.arena.IsEmpty())
        {
            // 화면 왼쪽 밖에서 굴러 들어온다 (카메라 기준, 매 틱 갱신)
            _e.body.position.x = _env.ctx.cameraView.min.x + c.entranceFromX;
            EnterState(_e, kEntrance);
        }
        else
        {
            // 아레나 없이 스폰 (샌드박스 테스트): 즉시 전투
            _e.ext[kExtFlags]       = 1;
            _e.ext[kExtNextTurret]  = c.turretFirstTicks;
            EnterState(_e, kAssault);
        }
    }

    void TickEntrance_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env)
    {
        const int   total = std::max(1, _c.entranceTicks);
        const float s     = std::min(1.f, static_cast<float>(_e.stateTicks) / static_cast<float>(total));
        const float e     = s * s * (3.f - 2.f * s);   // smoothstep: 천천히 들어와 천천히 멈춘다 (화면 왼쪽 밖에서 굴러 들어옴)
        const float screenX = _c.entranceFromX + (_c.stationX - _c.entranceFromX) * e;
        const float newX    = _env.ctx.cameraView.min.x + screenX;
        if (_e.stateTicks == 0)
        {
            _env.world.RequestCameraShake(kEntranceShakeAmp, static_cast<float>(total) / 60.f);
        }
        _e.body.velocity.x  = (newX - _e.body.position.x) * 60.f;
        _e.body.position.x  = newX;

        if (_e.stateTicks % std::max(1, _c.rumbleTicks) == 0)
        {
            Fx_(_env, "Explosion_Small", Scatter_(_env, { _e.body.position.x - 80.f, _e.body.position.y - 40.f }, 120.f, 30.f));
        }

        if (_e.stateTicks >= total)
        {
            _e.ext[kExtFlags]      |= 1;
            _e.ext[kExtNextTurret]  = _c.turretFirstTicks;
            _e.body.velocity.x      = 0.f;
            _e.aux                  = 0;
            EnterState(_e, kAssault);
        }
    }

    // 전투 중 선체 전진 (카메라 자동 전진과 같은 양). 버스트 주기: scrollOn 틱 전진, scrollOff 틱 정지.
    void TickScroll_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env)
    {
        const int cycle = std::max(1, _c.scrollOnTicks + _c.scrollOffTicks);
        const bool bOn  = _e.ext[kExtScrollTimer] < _c.scrollOnTicks;
        _e.ext[kExtScrollTimer] = (_e.ext[kExtScrollTimer] + 1) % cycle;

        if (!bOn || _e.ext[kExtScrollLeft] <= 0)
        {
            return;
        }
        // 엔드리스 드리프트: 카메라 하한을 scrollSpeed 로 민다 (플레이어가 멈춰도 계속 우측으로 간다)
        const int step = std::min(_e.ext[kExtScrollLeft], static_cast<int>(std::lround(_c.scrollSpeed * 1000.f * kStepSec)));
        _e.ext[kExtScrollLeft] -= step;
        _env.world.RequestCameraScroll(static_cast<float>(step) * 0.001f);
    }

    // 선체는 화면에 고정된다 (stationX): 카메라가 어떤 이유로 움직이든 (드리프트 + 플레이어 밀기) 같은 화면 자리에 있다.
    // 월드 배경/소품/지면은 카메라와 반대로 미끄러진다. 속도는 drive 클립 선택용.
    void FollowCamera_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env)
    {
        const float newX   = _env.ctx.cameraView.min.x + _c.stationX;
        _e.body.velocity.x = (newX - _e.body.position.x) * 60.f;
        _e.body.position.x = newX;
    }

    // 포탑 사격 상태 기계 (예고 Turning -> Shooting -> 발사 -> Idle)
    void TickTurrets_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env,
        const EnemyDef&     _def)
    {
        for (int i = 0; i < kTurretCount; ++i)
        {
            const int hp = TurretHp_(_e, i);
            if (hp <= 0)
            {
                continue;
            }
            const int phase = TurretPhase_(_e, i);
            int       t     = TurretTicks_(_e, i) + 1;
            if (phase == kTurretTurning && t >= _c.turretTurnTicks)
            {
                SetTurret_(_e, i, hp, kTurretShooting, 0);
            }
            else if (phase == kTurretShooting)
            {
                if (t == _c.turretFireTicks)
                {
                    FireTracer_(_e, _def, _env, i, 0);
                }
                if (t >= _c.turretShootTicks)
                {
                    SetTurret_(_e, i, hp, kTurretIdle, 0);
                }
                else
                {
                    SetTurret_(_e, i, hp, kTurretShooting, t);
                }
            }
            else if (phase == kTurretTurning)
            {
                SetTurret_(_e, i, hp, kTurretTurning, t);
            }
        }
    }

    // 포탑 사격 주기: 전투 중 일정 간격으로 살아 있는 쉬는 포탑 한 기를 골라 예고를 시작한다
    void TickTurretCadence_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env)
    {
        if (_e.aux < _e.ext[kExtNextTurret] || !_env.ctx.player.bAlive)
        {
            return;
        }
        int idle[kTurretCount] = {};
        int n                  = 0;
        for (int i = 0; i < kTurretCount; ++i)
        {
            if (TurretHp_(_e, i) > 0 && TurretPhase_(_e, i) == kTurretIdle)
            {
                idle[n++] = i;
            }
        }
        if (n == 0)
        {
            _e.ext[kExtNextTurret] = _e.aux + 10;
            return;
        }
        const int pick = idle[Roll(_env, n)];
        SetTurret_(_e, pick, TurretHp_(_e, pick), kTurretTurning, 0);
        int gap = IntervalTicks_(_env, _c.turretGapMinTicks, _c.turretGapMaxTicks);
        _e.ext[kExtNextTurret] = _e.aux + gap;
    }

    void EnterDeploy_(
        Enemy& _e)
    {
        EnterState(_e, kDeploy);
    }

    void TickAssault_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env)
    {
        (void)_env;
        if (CountTurretsDown_(_e) >= _c.deployAfterDown || CountTurretsAlive_(_e) == 0 || _e.aux >= _c.assaultMaxTicks)
        {
            EnterDeploy_(_e);
        }
    }

    void TickDeploy_(
        Enemy&              _e,
        const BossShieeCfg& _c)
    {
        if (_e.stateTicks >= _c.deployTicks)
        {
            _e.ext[kExtNextVolley] = _e.aux + _c.volleyFirstTicks;
            EnterState(_e, kCannon);
        }
    }

    // 캐논 볼리: Gun_Aim 예고 (volleyAimTicks) 뒤 트레이서 n 발을 stagger 간격으로 부채꼴 발사. 두 볼리마다 주황 포탄 1발.
    void TickCannon_(
        Enemy&              _e,
        const BossShieeCfg& _c,
        EnemyEnv&           _env,
        const EnemyDef&     _def)
    {
        if (CannonDead_(_e) || _e.aux < _e.ext[kExtNextVolley] || !_env.ctx.player.bAlive)
        {
            return;
        }
        const bool bEnraged = Enraged_(_e);
        const int  n        = _c.volleyStreaks + (bEnraged ? 1 : 0);
        _e.ext[kExtAimBin]   = AimBinOf_(_e, _c, _env);
        _e.ext[kExtAimStart] = _e.aux + 1;
        const int fireAt0    = _e.aux + _c.volleyAimTicks;
        for (int k = 0; k < n; ++k)
        {
            const int spreadIdx = k - n / 2;   // n=3: -1 0 1, n=4: -2 -1 0 1
            Enqueue_(_e, PackItem_(fireAt0 + k * _c.streakStaggerTicks, kKindTracer, kCannonSource, spreadIdx));
        }
        ++_e.ext[kExtVolleyCount];
        if (_c.shellEvery > 0 && _e.ext[kExtVolleyCount] % _c.shellEvery == 0)
        {
            Enqueue_(_e, PackItem_(fireAt0, kKindShell, kCannonSource, 0));
        }
        int interval = IntervalTicks_(_env, _c.volleyMinTicks, _c.volleyMaxTicks);
        if (bEnraged)
        {
            interval = static_cast<int>(static_cast<float>(interval) / _c.enragedRate);
        }
        _e.ext[kExtNextVolley] = _e.aux + std::max(interval, _c.volleyAimTicks + n * _c.streakStaggerTicks);
        (void)_def;
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BossShieeCfg& c = _def.bossShiee;
        _e.facing             = 1;

        if (_e.state == kEntrance)
        {
            TickEntrance_(_e, c, _env);
            return;
        }

        ++_e.aux;   // 전투 시계 (등장 종료 이후)
        TickQueue_(_e, _def, _env);
        TickTurrets_(_e, c, _env, _def);

        switch (_e.state)
        {
            case kAssault:
                TickTurretCadence_(_e, c, _env);
                TickScroll_(_e, c, _env);
                FollowCamera_(_e, c, _env);
                TickAssault_(_e, c, _env);
                break;
            case kDeploy:
                TickScroll_(_e, c, _env);
                FollowCamera_(_e, c, _env);
                TickDeploy_(_e, c);
                break;
            case kCannon:
                TickTurretCadence_(_e, c, _env);
                TickScroll_(_e, c, _env);
                FollowCamera_(_e, c, _env);
                TickCannon_(_e, c, _env, _def);
                break;
            default:
                _e.body.velocity.x = 0.f;
                break;
        }
    }

    // ---------------------------------------------------------------- 피격 / 사망

    bool OnHit_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const HitEvent& _event)
    {
        const BossShieeCfg& c    = _def.bossShiee;
        const int           part = GetPartIndex(_event.target);
        if (part < 0)
        {
            return true;   // 선체: 파괴 불가. 탄은 흡수되고 (스파크 효과만) 체력은 변하지 않는다
        }
        if (part < kTurretCount)
        {
            const int hp = TurretHp_(_e, part);
            if (hp <= 0)
            {
                return true;
            }
            const int left = std::max(0, hp - _event.damage);
            if (left > 0)
            {
                SetTurret_(_e, part, left, TurretPhase_(_e, part), TurretTicks_(_e, part));
            }
            else
            {
                KillTurret_(_e, _def, _env, part, true);
            }
            return true;
        }
        if (part == kCannonPart && (_e.state == kCannon) && !CannonDead_(_e))
        {
            _e.health.ApplyDamage(_event.damage);
            if (_e.health.hp <= 0)
            {
                _e.ext[kExtFlags] |= 2;
                const Vec2 o      = OriginOf_(_e, c);
                const Vec2 cannon = { o.x + c.cannonOffset.x, o.y + c.cannonOffset.y };
                Fx_(_env, "Explosion_Large", Scatter_(_env, cannon, 30.f, 30.f));
                PushPartScore_(_e, _env, c.cannonScore, cannon);
            }
        }
        return true;
    }

    void OnDeath_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BossShieeCfg& c = _def.bossShiee;
        for (int i = 0; i < kExtQueueCount; ++i)
        {
            _e.ext[kExtQueue + i] = 0;   // 사망 후 더 이상 공격하지 않는다
        }
        for (int i = 0; i < kTurretCount; ++i)
        {
            if (TurretHp_(_e, i) > 0)
            {
                KillTurret_(_e, _def, _env, i, false);
            }
        }
        _e.ext[kExtFlags] |= 2;
        _e.deathTicks = 0;
        const Vec2 cannon = { OriginOf_(_e, c).x + c.cannonOffset.x, OriginOf_(_e, c).y + c.cannonOffset.y };
        Fx_(_env, "Explosion_Huge", cannon);
    }

    // 사망 연출 (Dying): 캐논 폭발 2.5 s -> 선체 연쇄 폭발 7 s (연기) -> 분리 -> 잔해. 10 s 에 IsDestroyFinished.
    void TickDying_(
        Enemy&              _e,
        const EnemyDef&     _def,
        EnemyEnv&           _env)
    {
        const BossShieeCfg& c = _def.bossShiee;
        ++_e.deathTicks;
        const int t = _e.deathTicks;
        const Vec2 o = OriginOf_(_e, c);

        if (t < c.cannonBlastTicks && t % 12 == 0)
        {
            Fx_(_env, "Explosion_Mid", Scatter_(_env, { o.x + c.cannonOffset.x, o.y + c.cannonOffset.y }, 45.f, 40.f));
        }
        else if (t >= c.cannonBlastTicks && t < c.finishTicks && t % std::max(1, c.chainStepTicks) == 0)
        {
            const Vec2 p = HullPoint_(_env, _e, c);
            Fx_(_env, "Explosion_Small", p);
            if (Roll(_env, 2) == 0)
            {
                Fx_(_env, "Explosion_Mid", HullPoint_(_env, _e, c));
            }
            if (Roll(_env, 3) == 0)
            {
                Fx_(_env, "Explosion_Large", HullPoint_(_env, _e, c));
            }
        }
        if (t >= c.cannonBlastTicks && t < c.wreckTicks && t % std::max(1, c.smokeStepTicks) == 0)
        {
            Fx_(_env, "Boss_Smoke", HullPoint_(_env, _e, c));
        }
    }

    bool DeathDone_(
        const Enemy&    _e,
        const EnemyDef& _def)
    {
        return _e.deathTicks >= _def.bossShiee.finishTicks;
    }

    // 선체 HP 바: 포탑 + 캐논 (체력 합) / (최대 합). 등장 전은 숨김.
    std::optional<float> HpRatio_(
        const Enemy&    _e,
        const EnemyDef& _def)
    {
        if (!IsFightStarted(_e))
        {
            return std::nullopt;
        }
        const BossShieeCfg& c     = _def.bossShiee;
        int                 hp    = std::max(0, _e.health.hp);
        int                 maxHp = std::max(1, _e.health.maxHp);
        for (int i = 0; i < kTurretCount; ++i)
        {
            hp += TurretHp_(_e, i);
            maxHp += c.turretHp;
        }
        return static_cast<float>(hp) / static_cast<float>(maxHp);
    }

    void RegisterHurtboxes_(
        const Enemy&    _e,
        const EnemyDef& _def,
        CombatSystem&   _combat)
    {
        const BossShieeCfg& c = _def.bossShiee;
        const Vec2          o = OriginOf_(_e, c);

        // 포탑/캐논을 먼저 넣는다 (HitResolver 는 먼저 맞는 상자 하나에만 피해를 준다).
        // 파츠 상자는 파츠 중심에서 발 (선체 바닥) 까지 이어진 기둥이다: 아래에서 위로 쏜 탄도 선체 몸통보다 먼저 파츠에 닿는다.
        const float feetY = o.y + c.originDepth;
        for (int i = 0; i < kTurretCount; ++i)
        {
            if (TurretHp_(_e, i) <= 0)
            {
                continue;
            }
            const Vec2 centre = TurretCentreOf_(_e, c, i);
            Hurtbox    turret;
            turret.box     = Aabb::FromMinMax({ centre.x - c.turretBox.w * 0.5f, centre.y + c.turretBox.cy - c.turretBox.h * 0.5f }, { centre.x + c.turretBox.w * 0.5f, feetY });
            turret.team    = _def.team;
            turret.owner   = TurretOwner(_e.id, i);
            turret.accepts = AcceptsFor(false);
            _combat.AddHurtbox(turret);
        }
        if (_e.state == kCannon && !CannonDead_(_e) && _e.life == eEnemyLife::Alive)
        {
            Hurtbox cannon;
            const float cx = o.x + c.cannonBox.cx;
            cannon.box     = Aabb::FromMinMax({ cx - c.cannonBox.w * 0.5f, o.y + c.cannonBox.cy - c.cannonBox.h * 0.5f }, { cx + c.cannonBox.w * 0.5f, feetY });
            cannon.team    = _def.team;
            cannon.owner   = CannonOwner(_e.id);
            cannon.accepts = AcceptsFor(false);
            _combat.AddHurtbox(cannon);
        }

        // 선체는 판정이 없다 (탄/폭탄이 통과한다). 피격 상자는 포탑과 캐논뿐이다.
    }

    // ---------------------------------------------------------------- 렌더

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _r)
    {
        const BossShieeCfg& c    = _def.bossShiee;
        const Vec2          o    = { _r.feet.x, _r.feet.y - c.originDepth };
        const int           sort = static_cast<int>(_r.feet.y);
        const bool          bScrolling = _e.body.velocity.x > 0.01f;
        EnemyRenderEnv      hullR = _r;
        EnemyRenderEnv      partR = _r;
        hullR.layer = kBossHullLayer;
        partR.layer = kBossPartLayer;
        // 등장 중 선체 떨림 (시각 전용, 위치 판정과 무관): 3 틱마다 좌우 1.5 px
        const float jitter = (_e.state == kEntrance && (_e.stateTicks / kRumbleJitterTicks) % 2 == 0) ? -kRumbleJitterPx : (_e.state == kEntrance ? kRumbleJitterPx : 0.f);
        const Vec2  hullO  = { o.x + jitter, o.y };

        if (_e.life == eEnemyLife::Dying)
        {
            const int t = _e.deathTicks;
            if (t < c.splitTicks)
            {
                SubmitNamedPart(hullR, "Boss_Body_Idle", hullO, t, true, sort, hullR.layer);
            }
            else if (t < c.wreckTicks)
            {
                SubmitNamedPart(hullR, "Boss_Destroyed_Body", o, t - c.splitTicks, false, sort, hullR.layer);
            }
            else
            {
                SubmitNamedPart(hullR, "Boss_Wreck", hullO, kPersistTicks, false, sort, hullR.layer);
            }
            SubmitNamedPart(partR, "Boss_Destroyed_Cannon", { o.x + c.cannonOffset.x, o.y + c.cannonOffset.y }, kPersistTicks, false, sort + 2, partR.layer);
            for (int i = 0; i < kTurretCount; ++i)
            {
                SubmitNamedPart(partR, kTurretWreckClip[i], TurretCentreOf_(_e, c, i), kPersistTicks, false, sort + 3, partR.layer);
            }
            return;
        }

        // 선체
        switch (_e.state)
        {
            case kEntrance:
                SubmitNamedPart(hullR, "Boss_Body_DriveForward", hullO, _e.stateTicks, true, sort, hullR.layer);
                break;
            case kAssault:
                SubmitNamedPart(hullR, bScrolling ? "Boss_Body_DriveForward" : "Boss_Body_Idle", hullO, _e.stateTicks, true, sort, hullR.layer);
                break;
            case kDeploy:
            {
                const int stage = std::min(3, _e.stateTicks / kDeployStageTicks);
                SubmitNamedPart(partR, kDeployClips[stage], o, _e.stateTicks - stage * kDeployStageTicks, false, sort, partR.layer);
                break;
            }
            case kCannon:
                SubmitNamedPart(partR, bScrolling ? "Boss_Cannon_DriveForward" : "Boss_Cannon_Idle", o, _e.stateTicks, true, sort, partR.layer, false, PartFlashAmount(_e, kCannonPart));
                break;
            default: break;
        }

        // 캐논 헤드: 볼리 예고 (Gun_Aim) 와 발사 (Gun_Shoot_k)
        if (_e.state == kCannon && !CannonDead_(_e))
        {
            const Vec2 gun   = { o.x + c.cannonOffset.x, o.y + c.cannonOffset.y };
            const int  bin   = std::clamp(_e.ext[kExtAimBin], 0, kAimBins - 1);
            const int  since = _e.ext[kExtLastShot] > 0 ? _e.aux - (_e.ext[kExtLastShot] - 1) : c.gunShootTicks;
            if (since >= 0 && since < c.gunShootTicks)
            {
                SubmitNamedPart(partR, kGunShoot[std::min(4, bin / 2)], gun, since, false, sort + 2, partR.layer, false, PartFlashAmount(_e, kCannonPart));
            }
            else if (_e.ext[kExtAimStart] > 0)
            {
                const int aimed = _e.aux - (_e.ext[kExtAimStart] - 1);
                SubmitNamedPart(partR, "Boss_Gun_Aim", gun, std::min(aimed, 8 * kAimFrameTicks), true, sort + 2, partR.layer, false, PartFlashAmount(_e, kCannonPart));
            }
        }

        // 포탑 A..G
        for (int i = 0; i < kTurretCount; ++i)
        {
            const Vec2 t = TurretCentreOf_(_e, c, i);
            if (TurretHp_(_e, i) <= 0)
            {
                SubmitNamedPart(partR, kTurretWreckClip[i], t, kPersistTicks, false, sort + 3, partR.layer);
                continue;
            }
            const float flash = PartFlashAmount(_e, i);
            switch (TurretPhase_(_e, i))
            {
                case kTurretTurning:
                    SubmitNamedPart(partR, kTurretTurningClip[i], t, TurretTicks_(_e, i), false, sort + 3, partR.layer, false, flash);
                    break;
                case kTurretShooting:
                    SubmitNamedPart(partR, kTurretShootingClip[i], t, TurretTicks_(_e, i), false, sort + 3, partR.layer, false, flash);
                    break;
                default:
                    SubmitNamedPart(partR, kTurretIdleClip[i], t, _e.stateTicks, true, sort + 3, partR.layer, false, flash);
                    break;
            }
        }
    }
}

const BehaviorOps kBossShieeOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = &RegisterHurtboxes_,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
    .onCommand         = nullptr,
    .bossHpRatio       = &HpRatio_,
};

namespace bossshiee
{

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

int GetTurretPhase(const Enemy& _e, const int _index)
{
    return TurretPhase_(_e, _index);
}

int GetTurretTicks(const Enemy& _e, const int _index)
{
    return TurretTicks_(_e, _index);
}

int GetTurretsDestroyed(const Enemy& _e)
{
    return CountTurretsDown_(_e);
}

int GetCannonHp(const Enemy& _e)
{
    return _e.health.hp;
}

int GetVolleyCount(const Enemy& _e)
{
    return _e.ext[kExtVolleyCount];
}

int GetScrollLeftPx(const Enemy& _e)
{
    return _e.ext[kExtScrollLeft] / 1000;
}

int GetFightClock(const Enemy& _e)
{
    return _e.aux;
}

int GetAimStartTick(const Enemy& _e)
{
    return _e.ext[kExtAimStart] > 0 ? _e.ext[kExtAimStart] - 1 : -1;
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
    return OriginOf_(_e, _def.bossShiee);
}

Vec2 GetTurretCentre(const Enemy& _e, const EnemyDef& _def, const int _index)
{
    return TurretCentreOf_(_e, _def.bossShiee, _index);
}

Vec2 GetCannonMuzzle(const Enemy& _e, const EnemyDef& _def)
{
    return MuzzleOf_(_e, _def.bossShiee, kCannonSource);
}
}

void ParseBossShieeCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BossShieeCfg& c = _def.bossShiee;

    c.originDepth      = _r.Num("originDepth");
    c.stationX         = _r.Num("stationX");
    c.entranceFromX    = _r.Num("entranceFromX");
    c.entranceTicks    = _r.Ticks("entranceSec");
    c.rumbleTicks      = _r.Ticks("rumbleSec");
    c.scrollSpeed      = _r.Num("scrollSpeed");
    c.scrollOnTicks    = _r.Ticks("scrollOnSec");
    c.scrollOffTicks   = _r.Ticks("scrollOffSec");
    c.scrollBudget     = _r.Num("scrollBudget");

    c.turretHp         = _r.Int("turretHp");
    c.turretFirstTicks = _r.Ticks("turretFirstSec");
    c.turretGapMinTicks = _r.Ticks("turretGapMinSec");
    c.turretGapMaxTicks = _r.Ticks("turretGapMaxSec");
    c.turretTurnTicks  = _r.Ticks("turretTurnSec");
    c.turretFireTicks  = _r.Ticks("turretFireSec");
    c.turretShootTicks = _r.Ticks("turretShootSec");
    c.streakSpeed      = _r.Num("streakSpeed");
    c.deployAfterDown  = _r.Int("deployAfterDown");
    c.assaultMaxTicks  = _r.Ticks("assaultMaxSec");
    c.deployTicks      = _r.Ticks("deploySec");
    c.cannonHp         = _r.Int("cannonHp");
    c.volleyFirstTicks = _r.Ticks("volleyFirstSec");
    c.volleyMinTicks   = _r.Ticks("volleyMinSec");
    c.volleyMaxTicks   = _r.Ticks("volleyMaxSec");
    c.volleyAimTicks   = _r.Ticks("volleyAimSec");
    c.volleyStreaks    = _r.Int("volleyStreaks");
    c.streakStaggerTicks = _r.Ticks("streakStaggerSec");
    c.streakSpreadDeg  = _r.Num("streakSpreadDeg");
    c.shellEvery       = _r.Int("shellEvery");
    c.shellFlightSec   = _r.Num("shellFlightSec");
    c.enragedRate      = _r.Num("enragedRate");
    c.gunShootTicks    = _r.Ticks("gunShootSec");
    c.cannonBlastTicks = _r.Ticks("cannonBlastSec");
    c.chainStepTicks   = _r.Ticks("chainStepSec");
    c.splitTicks       = _r.Ticks("splitSec");
    c.wreckTicks       = _r.Ticks("wreckSec");
    c.finishTicks      = _r.Ticks("finishSec");
    c.smokeStepTicks   = _r.Ticks("smokeStepSec");
    c.turretScore      = _r.Int("turretScore");
    c.cannonScore      = _r.Int("cannonScore");
    {
        float offsets[kBossTurretCount * 2] = {};
        (void)_r.NumArray("turretOffsets", offsets, kBossTurretCount * 2);
        for (int i = 0; i < kBossTurretCount; ++i)
        {
            c.turretOffset[i] = { offsets[i * 2], offsets[i * 2 + 1] };
        }
    }
    c.turretMuzzle     = _r.ReadVec2("turretMuzzle", { -40.f, -10.f });
    c.turretBox        = _r.Box("turretBox");
    c.cannonOffset     = _r.ReadVec2("cannonOffset", { 0.f, 0.f });
    c.cannonMuzzle     = _r.ReadVec2("cannonMuzzle", { 0.f, 0.f });
    c.cannonBox        = _r.Box("cannonBox");
    c.hullBox          = _r.Box("hullBox");

    c.tracerProjectile = _r.Str("tracerProjectile");
    c.shellProjectile  = _r.Str("shellProjectile");
    c.blastProjectile  = _r.Str("blastProjectile");
}
}

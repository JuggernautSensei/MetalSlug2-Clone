#include "Game/Enemies/BigShieeEnemy.h"

#include <algorithm>
#include <cmath>

#include "Base/Log.h"
#include "Game/Enemies/BigShiee.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 최종보스 전함 '빅 셰이'

namespace
{
// 보스 파츠는 총알과 폭발만 받는다
const HitKindMask kBigShieeAccepts = HitKindMask { eHitKind::Bullet, eHitKind::Blast };
}   // namespace

// 낙하산 인질 정의 와 구출 시 드롭 무기
constexpr StringView kChuteDef = "pow_chute";
namespace
{
const ARRAY<StringView, 4> kChuteDrops = { "gunitem_mg", "gunitem_rl", "gunitem_sg", "gunitem_fs" };
}   // namespace

constexpr int kTurnMax      = 8;
constexpr int kTurnMid      = 4;
constexpr int kAimRows      = 5;   // 주포 사격 행
constexpr int kCasingFrames = 6;
constexpr int kCasingDepth  = 54;
constexpr int kShotFrames   = 8;
constexpr int kYawMax       = 8;   // 주포 yaw 프레임

namespace
{
const char* const kLetters = "ABCDEFG";
}   // namespace

struct TurretNames
{
    ARRAY<String, 3> idle = {};
    ARRAY<String, 3> shot = {};
    String           turn = {};
    String           dead = {};
    String           puff = {};
};

namespace
{
[[nodiscard]] ARRAY<TurretNames, kBigShieeTurretCount> MakeNames_()
{
    ARRAY<TurretNames, kBigShieeTurretCount> names = {};
    for (size_t i = 0; i < names.size(); ++i)
    {
        const String letter = String(1, kLetters[i]);
        for (size_t a = 0; a < 3; ++a)
        {
            names[i].idle[a] = "Boss_Turret_" + letter + "_Idle" + std::to_string(a + 1);
            names[i].shot[a] = "Boss_Turret_" + letter + "_Shot" + std::to_string(a + 1);
        }
        names[i].turn = "Boss_Turret_" + letter + "_Turn";
        names[i].dead = "Boss_Turret_" + letter + "_Dead";
        names[i].puff = i < 4 ? "Boss_Blast_ABCD1" : "Boss_Blast_EFG1";
    }
    return names;
}

[[nodiscard]] const TurretNames& GetNames_(
    const char _letter)
{
    static const ARRAY<TurretNames, kBigShieeTurretCount> kNames = MakeNames_();
    const size_t                                          index  = static_cast<size_t>(Clamp(_letter - 'A', 0, kBigShieeTurretCount - 1));
    return kNames[index];
}

[[nodiscard]] const char* CannonModeName_(
    const eBigShieeCannonMode _mode)
{
    switch (_mode)
    {
        case eBigShieeCannonMode::Deploy: return "Deploy";
        case eBigShieeCannonMode::Aim: return "Aim";
        case eBigShieeCannonMode::Ready: return "Ready";
        case eBigShieeCannonMode::Shot: return "Shot";
        default: return "Hidden";
    }
}

[[nodiscard]] VECTOR2 World_(
    const VECTOR2 _origin,
    const VECTOR2 _rel)
{
    return { _origin.x + _rel.x, _origin.y + _rel.y };
}

// 키 표 를 선형 보간
[[nodiscard]] float Interp_(
    const Span<const VECTOR2> _keys,
    const float               _t)
{
    if (_keys.empty())
    {
        return 0.f;
    }
    if (_t <= _keys.front().x)
    {
        return _keys.front().y;
    }
    for (size_t i = 1; i < _keys.size(); ++i)
    {
        if (_t <= _keys[i].x)
        {
            const float span = _keys[i].x - _keys[i - 1].x;
            const float u    = span > 0.f ? (_t - _keys[i - 1].x) / span : 1.f;
            return _keys[i - 1].y + (_keys[i].y - _keys[i - 1].y) * u;
        }
    }
    return _keys.back().y;
}

[[nodiscard]] float LastKeyTime_(
    const Span<const VECTOR2> _keys)
{
    return _keys.empty() ? 0.f : _keys.back().x;
}

// 주포 단계 기준 시간

// 누적 스크롤이 예산에 닿았으면 카메라는 더 가지 않는다

// 클립 길이
[[nodiscard]] int ClipTicks_(
    const EnemyEnv&  _env,
    const StringView _name,
    const int        _fallback)
{
    return _env.ctx.pAnims ? Max(1, GetNamedPartTicks(*_env.ctx.pAnims, _name)) : _fallback;
}

// 사격 클립 길이
[[nodiscard]] int ShotTicks_(
    const EnemyEnv&  _env,
    const StringView _name)
{
    const AnimClip* pClip = _env.ctx.pAnims ? _env.ctx.pAnims->FindClip(_name) : nullptr;
    const int       count = pClip ? pClip->GetFrameCount() : kShotFrames;
    return Max(1, ClipTicks_(_env, _name, 34) * Max(1, count - 1) / Max(1, count));
}

[[nodiscard]] int RandTicks_(
    EnemyEnv&              _env,
    const ARRAY<float, 2>& _rangeSec)
{
    const int lo = SecToTicks(_rangeSec[0]);
    const int hi = SecToTicks(_rangeSec[1]);
    return lo + Roll(_env, Max(1, hi - lo + 1));
}

bool SubmitFrame_(
    const EnemyRenderEnv& _render,
    const StringView      _name,
    const int             _frame,
    const VECTOR2         _origin,
    const int             _depth,
    const int             _sort,
    const float           _flash)
{
    const AnimClip* pClip = _render.anims.FindClip(_name);
    if (!pClip || pClip->frames.empty())
    {
        return false;
    }
    const AnimFrame& frame = pClip->frames[static_cast<size_t>(Clamp(_frame, 0, pClip->GetFrameCount() - 1))];
    if (frame.IsEmpty())
    {
        return false;
    }
    SpriteDrawOptions options;
    options.flashAmount = _flash;
    _render.sprites.Submit(_depth + _render.zOffset, _sort, frame, _origin, options);
    return true;
}

[[nodiscard]] float MuzzleX_(
    const BigShieeTurretConfig& _turret,
    const int                   _pos)
{
    return _pos <= 0 ? _turret.muzzle.x : (_pos >= kTurnMax ? -_turret.muzzle.x : 0.f);
}

[[nodiscard]] float YawDeg_(
    const BigShieeConfig& _config,
    const int             _yaw)
{
    const size_t low = static_cast<size_t>(_yaw / 2);
    if (_yaw % 2 == 0)
    {
        return _config.cannonAimDeg[low];
    }
    return (_config.cannonAimDeg[low] + _config.cannonAimDeg[low + 1]) * 0.5f;
}

[[nodiscard]] int NearestYaw_(
    const BigShieeConfig& _config,
    const float           _deg)
{
    int best = 0;
    for (int yaw = 1; yaw <= kYawMax; ++yaw)
    {
        if (Abs(YawDeg_(_config, yaw) - _deg) < Abs(YawDeg_(_config, best) - _deg))
        {
            best = yaw;
        }
    }
    return best;
}

// 전개 틱을 클립 틱으로 바꾼다
[[nodiscard]] int DeployClipTicks_(
    const BigShieeConfig& _config,
    const EnemyRenderEnv& _render,
    const int             _ticks)
{
    const int clipTicks = GetNamedPartTicks(_render.anims, "Boss_Cannon_Deploy");
    if (_config.cannonDeployTicks <= 0 || clipTicks <= 0)
    {
        return _ticks;
    }
    return _ticks * clipTicks / _config.cannonDeployTicks;
}

}   // namespace

void BigShieeEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const BigShieeConfig& config = _def.bigShiee;
    PrepareBody(_e, _def);
    _e.facing     = 1;
    m_ageTicks    = 0;
    _e.deathTicks = 0;
    m_boss        = {};

    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        const BigShieeTurretConfig& turret            = config.turrets[static_cast<size_t>(i)];
        m_boss.turretHp[static_cast<size_t>(i)]       = turret.hp;
        m_boss.turretMode[static_cast<size_t>(i)]     = eBigShieeTurretMode::Idle;
        m_boss.turretPosition[static_cast<size_t>(i)] = turret.side * kTurnMax;
    }
    m_boss.nextVolley  = SecToTicks(config.firstVolleySec);
    m_boss.nextChute   = SecToTicks(config.chuteFirstSec);
    m_boss.deployStart = -1;
    m_boss.lidStart    = -1;
    m_boss.lastKill    = -1;
    SetCannonMode_(eBigShieeCannonMode::Hidden);

    // 선체는 카메라 왼쪽 밖에서 시작한다
    const Aabb& view = _env.ctx.cameraView;
    if (view.max.x > view.min.x)
    {
        _e.body.position.x = view.min.x + config.startScreenX;
    }
    _e.prevFeet = _e.body.position;
    EnterState(_e, static_cast<int>(eBigShieePhase::Entrance));
}

void BigShieeEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const BigShieeConfig& config = _def.bigShiee;
    ++m_ageTicks;
    _e.body.velocity = { 0.f, 0.f };
    const float t    = AgeSec_();

    const bool bScrolling = t >= config.scrollStartSec && !IsScrollPaused_(config);
    const bool bStopped   = IsScrollStopped_(config);
    if (bScrolling && !bStopped)
    {
        _env.world.RequestCameraScroll(config.scrollSpeed * kStepSec);
        ++m_boss.scrollTicks;
    }

    const Aabb& view  = _env.ctx.cameraView;
    float       speed = 0.f;
    if (Phase_() == eBigShieePhase::Entrance)
    {
        const Span<const VECTOR2> keys { config.entranceKeys.data(), static_cast<size_t>(config.numEntranceKeys) };
        const float               now  = Interp_(keys, t);
        const float               prev = Interp_(keys, t - kStepSec);
        speed                          = (now - prev) / kStepSec;
        if (view.max.x > view.min.x)
        {
            _e.body.position.x = view.min.x + now;
        }
        if (t >= LastKeyTime_(keys))
        {
            EventLogFormat("bigshiee.entrance_done", "\"boss\":{:.1f}", _e.body.position.x);
            EnterState(_e, static_cast<int>(eBigShieePhase::Battery));
        }
    }
    else if ((Phase_() == eBigShieePhase::Battery || Phase_() == eBigShieePhase::Cannon) && view.max.x > view.min.x)
    {
        const float screenX = _e.body.position.x - view.min.x;
        int&        driving = m_boss.driving;
        int&        drift   = m_boss.driftTicks;
        const float td      = DeploySec_();
        const bool  bBack   = config.reverseSec > 0.f && driving == 0 && td >= config.reverseAtSec && td < config.reverseAtSec + config.reverseSec;
        if (bBack)
        {
            _e.body.position.x -= config.reverseSpeed * kStepSec;
            speed = -config.reverseSpeed;
        }
        else if (driving == 0 && bStopped && bScrolling && config.shuttleDriftSec > 0.f)
        {
            _e.body.position.x -= config.scrollSpeed * kStepSec;
            if (++drift >= SecToTicks(config.shuttleDriftSec))
            {
                driving = 1;
            }
        }
        if (driving == 0 && screenX < config.driveTriggerX)
        {
            driving = 1;
        }
        if (driving != 0)
        {
            drift = 0;
        }
        if (driving != 0)
        {
            const float remaining = config.driveTargetX - screenX;
            const float prev      = static_cast<float>(m_boss.speedX);
            speed                 = std::min({ config.driveSpeed, prev + config.driveAccel * kStepSec, Sqrt(Max(2.f * config.driveAccel * remaining, 0.f)) });
            speed                 = Max(14.f, speed);
            if (remaining <= speed * kStepSec)
            {
                _e.body.position.x = view.min.x + config.driveTargetX;
                speed              = 0.f;
                driving            = 0;
            }
            else
            {
                _e.body.position.x += speed * kStepSec;
            }
        }
    }
    m_boss.moving = Abs(speed) > 12.f ? 1 : 0;
    m_boss.speedX = static_cast<int>(std::lround(speed));

    if (m_ageTicks % 30 == 0)
    {
        EventLogFormat("bigshiee.cam", "\"cam\":{:.1f},\"boss\":{:.1f}", view.min.x, _e.body.position.x);
    }

    if (Phase_() == eBigShieePhase::Entrance || Phase_() == eBigShieePhase::Battery)
    {
        TickVolley_(_def, _env);
    }
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        TickTurret_(_def, _env, i);
    }

    if (_e.life == eEnemyLife::Alive && config.lowHpFxBelow > 0.f)
    {
        int curHp = _e.health.hp;
        int maxHp = _e.health.maxHp;
        for (int i = 0; i < kBigShieeTurretCount; ++i)
        {
            curHp += m_boss.turretHp[static_cast<size_t>(i)];
            maxHp += config.turrets[static_cast<size_t>(i)].hp;
        }
        const float ratio = maxHp > 0 ? static_cast<float>(curHp) / static_cast<float>(maxHp) : 1.f;
        if (ratio < config.lowHpFxBelow && m_ageTicks >= m_boss.nextDamageFx)
        {
            const float k       = Clamp(1.f - ratio / config.lowHpFxBelow, 0.f, 1.f);
            const float sec     = config.lowHpFxSec[0] + (config.lowHpFxSec[1] - config.lowHpFxSec[0]) * k;
            m_boss.nextDamageFx = m_ageTicks + Max(1, SecToTicks(sec));
            const float w       = config.lowHpFxArea[2] - config.lowHpFxArea[0];
            const float h       = config.lowHpFxArea[3] - config.lowHpFxArea[1];
            const float rx      = config.lowHpFxArea[0] + w * static_cast<float>(Roll(_env, 1000)) / 1000.f;
            const float ry      = config.lowHpFxArea[1] + h * static_cast<float>(Roll(_env, 1000)) / 1000.f;
            RequestFxAt(_env, config.lowHpFxClip.c_str(), World_(_e.body.position, { rx, ry }), _e.facing, 57);
        }
    }

    if ((Phase_() == eBigShieePhase::Battery || Phase_() == eBigShieePhase::Entrance) && AliveTurrets_() == 0)
    {
        const int lastKill = m_boss.lastKill;
        if (lastKill >= 0 && m_ageTicks - lastKill >= SecToTicks(config.deployDelaySec))
        {
            EnterState(_e, static_cast<int>(eBigShieePhase::Deploy));
            m_boss.lidStart = m_ageTicks;
        }
    }
    if (Phase_() == eBigShieePhase::Deploy && m_boss.lidStart >= 0)
    {
        const int sinceLid = m_ageTicks - m_boss.lidStart;
        if (sinceLid % config.hullStepTicks == 0 && sinceLid / config.hullStepTicks <= 4)
        {
            EventLogFormat("bigshiee.lid", "\"step\":{}", sinceLid / config.hullStepTicks);
        }
    }
    // 선수 가 다
    if (Phase_() == eBigShieePhase::Deploy && m_boss.cannonMode == eBigShieeCannonMode::Hidden
        && m_ageTicks - m_boss.lidStart >= SecToTicks(config.cannonDeployDelaySec))
    {
        m_boss.deployStart = m_ageTicks;
        SetCannonMode_(eBigShieeCannonMode::Deploy);
    }
    TickCannon_(_def, _env);
    const int casingStart = m_boss.casingTick;
    if (casingStart > 0 && m_ageTicks - casingStart == static_cast<int>(Ceil((CasingLandSec_(config) + config.casingLingerSec) / kStepSec)))
    {
        EventLogFormat("bigshiee.casing_gone", "\"x\":{}", m_boss.casingX);
    }
    TickChute_(_def, _env);
}

bool BigShieeEnemy::OnHit(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const HitEvent& _event)
{
    EnemyState&           _e     = GetMutableState();
    const BigShieeConfig& config = _def.bigShiee;
    const int             part   = GetPartIndex(_event.target);

    if (part >= 0 && part < kBigShieeTurretCount)
    {
        int& hp = m_boss.turretHp[static_cast<size_t>(part)];
        if (hp <= 0)
        {
            return true;
        }
        hp -= _event.damage;
        if (hp <= 0)
        {
            hp = 0;
            SetTurretMode_(part, eBigShieeTurretMode::Dead);
            m_boss.turretPosition[static_cast<size_t>(part)] = config.turrets[static_cast<size_t>(part)].side * kTurnMax;
            m_boss.lastKill                                  = m_ageTicks;
            EventLogFormat("bigshiee.turret_dead", "\"turret\":{}", part);

            const BigShieeTurretConfig& turret    = config.turrets[static_cast<size_t>(part)];
            const VECTOR2               at        = World_(_e.body.position, { turret.anchor.x + config.turretBlastOffset.x, turret.anchor.y + config.turretBlastOffset.y });
            EnemyEvent                  destroyed = _env.world.MakeEvent(_e, eEnemyEventKind::PartDestroyed);
            destroyed.score                       = config.turretScore;
            destroyed.pos                         = at;
            _env.world.PushEvent(destroyed);
            RequestFxAt(_env, turret.bTwin ? "Explosion_Large" : "Explosion_Mid", at, _e.facing, 56);
            RequestFxAt(_env, "Boss_Smoke_Big", { at.x + config.turretSmokeOffset.x, at.y + config.turretSmokeOffset.y }, _e.facing, 54, 0.25f);
        }
        return true;
    }

    if (part == kBigShieeCannonPart)
    {
        if (CannonVulnerable_())
        {
            (void)_e.health.ApplyDamage(_event.damage);
        }
        return true;
    }
    return true;
}

void BigShieeEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const BigShieeConfig& config = _def.bigShiee;
    _e.deathTicks                = 0;
    EventLogFormat("bigshiee.death", "\"x\":{:.1f}", _e.body.position.x);
    const VECTOR2 top = World_(_e.body.position, { config.cannonBoxOffset.x, config.cannonBoxOffset.y - config.cannonBoxSize.y * 0.4f });
    RequestFxAt(_env, "PlayerBombExplosion", { top.x + config.deathFlameOffset.x, top.y + config.deathFlameOffset.y }, _e.facing, 56);
    RequestFxAt(_env, "Explosion_Large", top, _e.facing, 57, 0.15f);
}

void BigShieeEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const BigShieeConfig& config = _def.bigShiee;
    _e.body.velocity             = { 0.f, 0.f };
    ++_e.deathTicks;
    const float   t      = static_cast<float>(_e.deathTicks) * kStepSec;
    const VECTOR2 origin = _e.body.position;

    const bool bWreck = t >= config.dyingWreckSec;
    if (bWreck && static_cast<float>(_e.deathTicks - 1) * kStepSec < config.dyingWreckSec)
    {
        EventLogFormat("bigshiee.wreck", "\"x\":{:.1f}", origin.x);
    }
    if (t >= config.dyingDoneSec && static_cast<float>(_e.deathTicks - 1) * kStepSec < config.dyingDoneSec)
    {
        EventLogFormat("bigshiee.done", "\"x\":{:.1f}", origin.x);
    }
    if (bWreck)
    {
        const int interval = Max(1, SecToTicks(config.wreckSmokeIntervalSec));
        if (_e.deathTicks % interval == 0 && t < config.dyingDoneSec + 1.f)
        {
            const float rx = static_cast<float>(Roll(_env, 440)) - 220.f;
            const float ry = static_cast<float>(Roll(_env, 260)) - 260.f;
            RequestFxAt(_env, "Boss_Smoke_Big", World_(origin, { rx, ry }), _e.facing, 54);
        }
        return;
    }

    const float progress = Clamp(t / config.dyingWreckSec, 0.f, 1.f);
    const int   interval = Max(1, static_cast<int>(static_cast<float>(SecToTicks(config.dyingBlastIntervalSec)) * (1.4f - progress)));
    if (_e.deathTicks % interval == 0)
    {
        const float rx   = static_cast<float>(Roll(_env, 480)) - 260.f;
        const float ry   = static_cast<float>(Roll(_env, 330)) - 340.f;
        const int   kind = Roll(_env, 3);
        RequestFxAt(_env, kind == 0 ? "Explosion_Large" : "Explosion_Mid", World_(origin, { rx, ry }), _e.facing, 57);
    }
    if (_e.deathTicks == SecToTicks(1.4f) || _e.deathTicks == SecToTicks(2.2f) || _e.deathTicks == SecToTicks(3.1f))
    {
        const float rx = static_cast<float>(Roll(_env, 300)) - 200.f;
        const float ry = static_cast<float>(Roll(_env, 160)) - 260.f;
        RequestFxAt(_env, "PlayerBombExplosion", World_(origin, { rx, ry + config.deathBlastDy }), _e.facing, 56);
    }
    if (_e.deathTicks == SecToTicks(config.dyingWreckSec) - 6)
    {
        RequestFxAt(_env, "Explosion_Huge", World_(origin, config.deathHugeOffset), _e.facing, 58);
    }
}

void BigShieeEnemy::RegisterHurtboxes(
    const EnemyDef& _def,
    CombatSystem&   _combat) const
{
    const EnemyState& _e = GetState();
    // 첫 틱 전에는 위치가 스폰 지점 그대로라 판정을 등록하지
    if (!_e.bInit)
    {
        return;
    }
    const BigShieeConfig& config = _def.bigShiee;
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        if (m_boss.turretHp[static_cast<size_t>(i)] <= 0)
        {
            continue;
        }
        const BigShieeTurretConfig& turret = config.turrets[static_cast<size_t>(i)];
        const VECTOR2               center = World_(_e.body.position, { turret.anchor.x + turret.boxOffset.x, turret.anchor.y + turret.boxOffset.y });
        Hurtbox                     hurtbox;
        hurtbox.box     = Aabb::FromCenterSize(center, turret.boxSize);
        hurtbox.team    = _def.team;
        hurtbox.owner   = MakePartOwner(_e.id, static_cast<uint32_t>(i));
        hurtbox.accepts = kBigShieeAccepts;
        _combat.AddHurtbox(hurtbox);
    }
    if (CannonVulnerable_())
    {
        const VECTOR2 center = World_(_e.body.position, config.cannonBoxOffset);
        Hurtbox       hurtbox;
        hurtbox.box     = Aabb::FromCenterSize(center, config.cannonBoxSize);
        hurtbox.team    = _def.team;
        hurtbox.owner   = MakePartOwner(_e.id, static_cast<uint32_t>(kBigShieeCannonPart));
        hurtbox.accepts = kBigShieeAccepts;
        _combat.AddHurtbox(hurtbox);
    }
}

void BigShieeEnemy::SubmitParts(
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render) const
{
    const EnemyState& _e = GetState();
    // 첫 틱 전에는 스폰 지점에 그려지므로 숨긴다
    if (!_e.bInit)
    {
        return;
    }
    const BigShieeConfig& config = _def.bigShiee;
    const VECTOR2         feet   = _render.feet;

    if (_e.life == eEnemyLife::Dying)
    {
        const float t = static_cast<float>(_e.deathTicks) * kStepSec;
        if (t >= config.dyingWreckSec)
        {
            (void)SubmitNamedPart(_render, "Boss_Wreck", World_(feet, config.wreckOffset), 0, false, config.partWreck.sort, config.partWreck.depth);
            (void)SubmitNamedPart(_render, "Boss_CannonWreck", World_(feet, config.cannonWreckOffset), 0, false, config.partCannon.sort, config.partCannon.depth);
            return;
        }
        SubmitHull_(config, _render, feet);
        (void)SubmitNamedPart(_render, "Boss_AntennaP", World_(feet, config.antennaP), 0, false, config.partAntenna.sort, config.partAntenna.depth);
        (void)SubmitNamedPart(_render, "Boss_AntennaQ", World_(feet, config.antennaQ), 0, false, config.partAntenna.sort, config.partAntenna.depth);
        for (int i = 0; i < kBigShieeTurretCount; ++i)
        {
            SubmitTurret_(config, _render, feet, i);
        }
        SubmitCannon_(config, _render, feet, (_e.deathTicks / 3) % 2 == 0 ? 1.f : 0.f);
        return;
    }

    SubmitHull_(config, _render, feet);
    (void)SubmitNamedPart(_render, "Boss_AntennaP", World_(feet, config.antennaP), 0, false, config.partAntenna.sort, config.partAntenna.depth);
    (void)SubmitNamedPart(_render, "Boss_AntennaQ", World_(feet, config.antennaQ), 0, false, config.partAntenna.sort, config.partAntenna.depth);
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        SubmitTurret_(config, _render, feet, i);
    }
    SubmitCannon_(config, _render, feet, PartFlashAmount(_e, kBigShieeCannonPart));
    SubmitCasing_(config, _render);
}

bool BigShieeEnemy::DeathDone(
    const EnemyDef& _def) const
{
    const EnemyState& _e = GetState();
    return static_cast<float>(_e.deathTicks) * kStepSec >= _def.bigShiee.dyingDoneSec;
}

namespace
{
template<size_t kCapacity>
int ReadKeys_(
    DefReader&                 _r,
    const StringView           _key,
    ARRAY<VECTOR2, kCapacity>& _out)
{
    ARRAY<float, kCapacity * 2> flat  = {};
    const int                   count = _r.NumList(_key, flat);
    if (count < 0 || count % 2 != 0)
    {
        return -1;
    }
    for (int i = 0; i < count / 2; ++i)
    {
        _out[static_cast<size_t>(i)] = { flat[static_cast<size_t>(i) * 2], flat[static_cast<size_t>(i) * 2 + 1] };
    }
    return count / 2;
}
}   // namespace

void ParseBigShieeConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    BigShieeConfig& config = _def.bigShiee;

    ARRAY<float, kBigShieeTurretCount * 2> anchors  = {};
    ARRAY<float, kBigShieeTurretCount * 4> boxes    = {};
    ARRAY<float, kBigShieeTurretCount * 2> muzzles  = {};
    ARRAY<float, kBigShieeTurretCount>     sides    = {};
    ARRAY<float, kBigShieeTurretCount>     twins    = {};
    ARRAY<float, kBigShieeTurretCount>     hps      = {};
    const bool                             bAnchors = _r.NumArray("turretAnchor", anchors);
    const bool                             bBoxes   = _r.NumArray("turretBox", boxes);
    const bool                             bMuzzles = _r.NumArray("turretMuzzle", muzzles);
    const bool                             bSides   = _r.NumArray("turretSide", sides);
    const bool                             bTwins   = _r.NumArray("turretTwin", twins);
    const bool                             bHps     = _r.NumArray("turretHp", hps);
    const float                            gap      = _r.Num("muzzleGap", 16.f);
    for (size_t i = 0; i < static_cast<size_t>(kBigShieeTurretCount); ++i)
    {
        BigShieeTurretConfig& t = config.turrets[i];
        t.letter                = kLetters[i];
        t.muzzleGap             = gap;
        if (bAnchors)
        {
            t.anchor = { anchors[i * 2], anchors[i * 2 + 1] };
        }
        if (bBoxes)
        {
            t.boxSize   = { boxes[i * 4], boxes[i * 4 + 1] };
            t.boxOffset = { boxes[i * 4 + 2], boxes[i * 4 + 3] };
        }
        if (bMuzzles)
        {
            t.muzzle = { muzzles[i * 2], muzzles[i * 2 + 1] };
        }
        if (bSides)
        {
            t.side = static_cast<int>(sides[i]);
        }
        if (bTwins)
        {
            t.bTwin = twins[i] != 0.f;
        }
        if (bHps)
        {
            t.hp = static_cast<int>(hps[i]);
        }
    }
    config.turretScore = _r.Int("turretScore", config.turretScore);
    config.antennaP    = _r.ReadVec2("antennaP", config.antennaP);
    config.antennaQ    = _r.ReadVec2("antennaQ", config.antennaQ);

    config.startScreenX = _r.Num("startScreenX", config.startScreenX);
    if (const int n = ReadKeys_(_r, "entranceKeys", config.entranceKeys); n >= 0)
    {
        config.numEntranceKeys = n;
    }

    config.scrollSpeed         = _r.Num("scrollSpeed", config.scrollSpeed);
    config.scrollStartSec      = _r.Num("scrollStartSec", config.scrollStartSec);
    config.deployPauseDelaySec = _r.Num("deployPauseDelaySec", config.deployPauseDelaySec);
    config.deployPauseSec      = _r.Num("deployPauseSec", config.deployPauseSec);
    config.latePauseFirstSec   = _r.Num("latePauseFirstSec", config.latePauseFirstSec);
    config.latePausePeriodSec  = _r.Num("latePausePeriodSec", config.latePausePeriodSec);
    config.latePauseSec        = _r.Num("latePauseSec", config.latePauseSec);
    if (const int n = ReadKeys_(_r, "scrollPauses", config.scrollPauses); n >= 0)
    {
        config.numScrollPauses = n;
    }

    config.driveTriggerX = _r.Num("driveTriggerX", config.driveTriggerX);
    config.driveTargetX  = _r.Num("driveTargetX", config.driveTargetX);
    config.driveSpeed    = _r.Num("driveSpeed", config.driveSpeed);
    config.driveAccel    = _r.Num("driveAccel", config.driveAccel);

    config.scrollBudgetPx  = _r.Num("scrollBudgetPx", config.scrollBudgetPx);
    config.shuttleDriftSec = _r.Num("shuttleDriftSec", config.shuttleDriftSec);
    config.reverseAtSec    = _r.Num("reverseAtSec", config.reverseAtSec);
    config.reverseSec      = _r.Num("reverseSec", config.reverseSec);
    config.reverseSpeed    = _r.Num("reverseSpeed", config.reverseSpeed);

    (void)_r.NumArray("volleyIntervalSec", config.volleyIntervalSec);
    config.firstVolleySec   = _r.Num("firstVolleySec", config.firstVolleySec);
    config.prepTicks        = _r.Int("prepTicks", config.prepTicks);
    config.turnFrameTicks   = Max(1, _r.Int("turnFrameTicks", config.turnFrameTicks));
    config.shotTicks        = _r.Int("shotTicks", config.shotTicks);
    config.fireTick         = _r.Int("fireTick", config.fireTick);
    config.pairGapTicks     = _r.Int("pairGapTicks", config.pairGapTicks);
    config.turretAimBandPx  = _r.Num("turretAimBandPx", config.turretAimBandPx);
    config.turretTrackTicks = _r.Int("turretTrackTicks", config.turretTrackTicks);

    config.cannonAnchor         = _r.ReadVec2("cannonAnchor", config.cannonAnchor);
    config.cannonBoxSize        = _r.ReadVec2("cannonBoxSize", config.cannonBoxSize);
    config.cannonBoxOffset      = _r.ReadVec2("cannonBoxOffset", config.cannonBoxOffset);
    config.cannonAimOrigin      = _r.ReadVec2("cannonAimOrigin", config.cannonAimOrigin);
    config.deployDelaySec       = _r.Num("deployDelaySec", config.deployDelaySec);
    config.firstMissileSec      = _r.Num("firstMissileSec", config.firstMissileSec);
    config.missileIntervalSec   = _r.Num("missileIntervalSec", config.missileIntervalSec);
    config.missileFireTick      = _r.Int("missileFireTick", config.missileFireTick);
    config.hullStepTicks        = Max(1, _r.Int("hullStepTicks", config.hullStepTicks));
    config.cannonDeployDelaySec = _r.Num("cannonDeployDelaySec", config.cannonDeployDelaySec);
    config.cannonDeployTicks    = _r.Int("cannonDeployTicks", config.cannonDeployTicks);
    config.cannonTurnTicks      = Max(1, _r.Int("cannonTurnTicks", config.cannonTurnTicks));
    (void)_r.NumArray("cannonAimDeg", config.cannonAimDeg);
    (void)_r.NumArray("cannonMuzzleX", config.cannonMuzzleX);

    config.casingOffset    = _r.ReadVec2("casingOffset", config.casingOffset);
    config.casingScale     = _r.Num("casingScale", config.casingScale);
    config.casingVel       = _r.ReadVec2("casingVel", config.casingVel);
    config.casingGravity   = _r.Num("casingGravity", config.casingGravity);
    config.casingGroundDy  = _r.Num("casingGroundDy", config.casingGroundDy);
    config.casingLingerSec = _r.Num("casingLingerSec", config.casingLingerSec);
    config.casingFps       = _r.Num("casingFps", config.casingFps);

    config.lowHpFxBelow = _r.Num("lowHpFxBelow", config.lowHpFxBelow);
    (void)_r.NumArray("lowHpFxSec", config.lowHpFxSec);
    (void)_r.NumArray("lowHpFxArea", config.lowHpFxArea);
    if (_r.FindRaw("lowHpFxClip") != nullptr)
    {
        config.lowHpFxClip = _r.Str("lowHpFxClip");
    }

    config.dyingBlastIntervalSec = _r.Num("dyingBlastIntervalSec", config.dyingBlastIntervalSec);
    config.dyingWreckSec         = _r.Num("dyingWreckSec", config.dyingWreckSec);
    config.dyingDoneSec          = _r.Num("dyingDoneSec", config.dyingDoneSec);
    config.wreckSmokeIntervalSec = _r.Num("wreckSmokeIntervalSec", config.wreckSmokeIntervalSec);
    config.wreckOffset           = _r.ReadVec2("wreckOffset", config.wreckOffset);
    config.cannonWreckOffset     = _r.ReadVec2("cannonWreckOffset", config.cannonWreckOffset);

    config.chuteFirstSec     = _r.Num("chuteFirstSec", config.chuteFirstSec);
    config.chuteIntervalSec  = _r.Num("chuteIntervalSec", config.chuteIntervalSec);
    config.chuteSpreadX      = _r.Num("chuteSpreadX", config.chuteSpreadX);
    config.chuteRightMarginX = _r.Num("chuteRightMarginX", config.chuteRightMarginX);
    config.chuteStartAboveY  = _r.Num("chuteStartAboveY", config.chuteStartAboveY);
    config.chuteEdgeX        = _r.Num("chuteEdgeX", config.chuteEdgeX);

    config.turretBlastOffset = _r.ReadVec2("turretBlastOffset", config.turretBlastOffset);
    config.turretSmokeOffset = _r.ReadVec2("turretSmokeOffset", config.turretSmokeOffset);
    config.deathFlameOffset  = _r.ReadVec2("deathFlameOffset", config.deathFlameOffset);
    config.deathBlastDy      = _r.Num("deathBlastDy", config.deathBlastDy);
    config.deathHugeOffset   = _r.ReadVec2("deathHugeOffset", config.deathHugeOffset);
    config.deathDiscOffset   = _r.ReadVec2("deathDiscOffset", config.deathDiscOffset);
}

[[nodiscard]] eBigShieePhase BigShieeEnemy::Phase_() const
{
    const EnemyState& _e = GetState();
    return static_cast<eBigShieePhase>(_e.state);
}

void BigShieeEnemy::SetTurretMode_(
    const int                 _i,
    const eBigShieeTurretMode _mode)
{
    m_boss.turretMode[static_cast<size_t>(_i)]  = _mode;
    m_boss.turretTicks[static_cast<size_t>(_i)] = 0;
}

void BigShieeEnemy::SetCannonMode_(
    const eBigShieeCannonMode _mode)
{
    EventLogFormat("bigshiee.cannon", "\"mode\":\"{}\",\"yaw\":{}", CannonModeName_(_mode), m_boss.cannonYaw);
    m_boss.cannonMode  = _mode;
    m_boss.cannonTicks = 0;
}

[[nodiscard]] int BigShieeEnemy::AliveTurrets_() const
{
    int count = 0;
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        if (m_boss.turretHp[static_cast<size_t>(i)] > 0)
        {
            ++count;
        }
    }
    return count;
}

[[nodiscard]] float BigShieeEnemy::AgeSec_() const
{
    return static_cast<float>(m_ageTicks) * kStepSec;
}

[[nodiscard]] float BigShieeEnemy::DeploySec_() const
{
    const int start = m_boss.deployStart;
    return start < 0 ? -1.f : static_cast<float>(m_ageTicks - start) * kStepSec;
}

[[nodiscard]] bool BigShieeEnemy::IsScrollPaused_(
    const BigShieeConfig& _config) const
{
    const float t = AgeSec_();
    for (int i = 0; i < _config.numScrollPauses; ++i)
    {
        const VECTOR2 pause = _config.scrollPauses[static_cast<size_t>(i)];
        if (t >= pause.x && t < pause.x + pause.y)
        {
            return true;
        }
    }
    const float td = DeploySec_();
    if (td < 0.f)
    {
        return false;
    }
    if (td >= _config.deployPauseDelaySec && td < _config.deployPauseDelaySec + _config.deployPauseSec)
    {
        return true;
    }
    const float late = td - _config.latePauseFirstSec;
    return late >= 0.f && std::fmod(late, _config.latePausePeriodSec) < _config.latePauseSec;
}

[[nodiscard]] bool BigShieeEnemy::IsScrollStopped_(
    const BigShieeConfig& _config) const
{
    return _config.scrollBudgetPx > 0.f && static_cast<float>(m_boss.scrollTicks) * _config.scrollSpeed * kStepSec >= _config.scrollBudgetPx;
}

[[nodiscard]] VECTOR2 BigShieeEnemy::ExtraFromWorld_(
    const EnemyDef& _def,
    const int       _weapon,
    const VECTOR2   _muzzle) const
{
    const EnemyState& _e = GetState();
    if (_weapon < 0)
    {
        return { 0.f, 0.f };
    }
    const VECTOR2 base = GetMuzzleWorld(_e, _def, _weapon);
    return { _muzzle.x - base.x, _muzzle.y - base.y };
}

[[nodiscard]] int BigShieeEnemy::AimPos_(
    const BigShieeConfig&       _config,
    const BigShieeTurretConfig& _turret,
    const EnemyEnv&             _env,
    const int                   _pos) const
{
    const EnemyState& _e = GetState();
    if (!_env.ctx.player.bAlive)
    {
        return _pos;
    }
    const float dx = _env.ctx.player.feet.x - (_e.body.position.x + _turret.anchor.x + _turret.muzzle.x);
    if (dx < -_config.turretAimBandPx)
    {
        return 0;
    }
    return dx > _config.turretAimBandPx ? kTurnMax : kTurnMid;
}

void BigShieeEnemy::FireShell_(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _turret,
    const int       _barrel)
{
    EnemyState&                 _e     = GetMutableState();
    const BigShieeConfig&       config = _def.bigShiee;
    const BigShieeTurretConfig& turret = config.turrets[static_cast<size_t>(_turret)];
    const int                   aim    = m_boss.turretPosition[static_cast<size_t>(_turret)] / kTurnMid + 1;
    const float                 gap    = turret.bTwin ? (_barrel == 0 ? -0.5f : 0.5f) * turret.muzzleGap : 0.f;
    const VECTOR2               muzzle = World_(_e.body.position, { turret.anchor.x + MuzzleX_(turret, m_boss.turretPosition[static_cast<size_t>(_turret)]) + gap, turret.anchor.y + turret.muzzle.y });
    const int                   weapon = _def.FindWeapon("shell");
    EventLogFormat("bigshiee.turret_fire", "\"turret\":{},\"barrel\":{},\"aim\":{},\"x\":{:.1f},\"y\":{:.1f}", _turret, _barrel, aim, muzzle.x, muzzle.y);
    (void)FireWeapon(_e, _def, _env, weapon, { 0.f, 0.f }, ExtraFromWorld_(_def, weapon, muzzle));
}

void BigShieeEnemy::TickTurret_(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _i)
{
    EnemyState&                 _e     = GetMutableState();
    const BigShieeConfig&       config = _def.bigShiee;
    const BigShieeTurretConfig& turret = config.turrets[static_cast<size_t>(_i)];
    const eBigShieeTurretMode   mode   = m_boss.turretMode[static_cast<size_t>(_i)];
    if (mode == eBigShieeTurretMode::Dead)
    {
        return;
    }
    int& ticks = m_boss.turretTicks[static_cast<size_t>(_i)];
    int& pos   = m_boss.turretPosition[static_cast<size_t>(_i)];
    ++ticks;

    switch (mode)
    {
        case eBigShieeTurretMode::Idle:
        {
            // 쉬는 동안 turretTrackTicks 마다
            const int target = AimPos_(config, turret, _env, pos);
            if (config.turretTrackTicks > 0 && ticks >= config.turretTrackTicks && pos != target)
            {
                pos += target > pos ? 1 : -1;
                ticks = 0;
            }
            break;
        }
        case eBigShieeTurretMode::Prep:
            if (ticks >= config.prepTicks)
            {
                SetTurretMode_(_i, pos != AimPos_(config, turret, _env, pos) ? eBigShieeTurretMode::Turn : eBigShieeTurretMode::Shot);
            }
            break;
        case eBigShieeTurretMode::Turn:
            if (ticks % config.turnFrameTicks == 0)
            {
                const int target = AimPos_(config, turret, _env, pos);
                pos += target > pos ? 1 : (target < pos ? -1 : 0);
                if (pos == target)
                {
                    SetTurretMode_(_i, eBigShieeTurretMode::Shot);
                }
            }
            break;
        case eBigShieeTurretMode::Shot:
            if (ticks == config.fireTick)
            {
                FireShell_(_def, _env, _i, 0);
                const VECTOR2 puff = World_(_e.body.position, { turret.anchor.x + MuzzleX_(turret, pos), turret.anchor.y + turret.muzzle.y });
                RequestFxAt(_env, GetNames_(turret.letter).puff, puff, _e.facing, 55);
            }
            else if (turret.bTwin && ticks == config.fireTick + config.pairGapTicks)
            {
                FireShell_(_def, _env, _i, 1);
            }
            if (ticks >= config.shotTicks)
            {
                SetTurretMode_(_i, eBigShieeTurretMode::Idle);
            }
            break;
        default:
            break;
    }
}

void BigShieeEnemy::TickVolley_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeConfig& config = _def.bigShiee;
    if (m_ageTicks < m_boss.nextVolley)
    {
        return;
    }
    ARRAY<int, kBigShieeTurretCount> idle  = {};
    int                              count = 0;
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        if (m_boss.turretHp[static_cast<size_t>(i)] > 0 && m_boss.turretMode[static_cast<size_t>(i)] == eBigShieeTurretMode::Idle)
        {
            idle[static_cast<size_t>(count++)] = i;
        }
    }
    if (count <= 0)
    {
        m_boss.nextVolley = m_ageTicks + 30;
        return;
    }
    const int pick = idle[static_cast<size_t>(Roll(_env, count))];
    SetTurretMode_(pick, eBigShieeTurretMode::Prep);
    m_boss.nextVolley = m_ageTicks + RandTicks_(_env, config.volleyIntervalSec);
}

void BigShieeEnemy::TickChute_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeConfig& config = _def.bigShiee;
    if (m_ageTicks < m_boss.nextChute)
    {
        return;
    }
    m_boss.nextChute = m_ageTicks + SecToTicks(config.chuteIntervalSec);

    const Aabb& view  = _env.ctx.cameraView;
    const int   index = _env.lib.FindIndex(kChuteDef);
    if (index < 0 || view.max.x <= view.min.x || !_env.ctx.player.bAlive)
    {
        return;
    }
    // 낙하 중에도 카메라가 자동 스크롤로 오른쪽으로 간다
    const float fallDist = Max(_env.ctx.player.feet.y - (view.min.y - config.chuteStartAboveY), 0.f);
    const float descend  = _env.lib.Get(index).flyingBzk.descendSpeed;
    const float fallSec  = descend > 0.f ? fallDist / descend : 0.f;
    const float scroll   = AgeSec_() >= config.scrollStartSec && !IsScrollPaused_(config) && !IsScrollStopped_(config) ? config.scrollSpeed * fallSec : 0.f;

    const int   spread = Max(1, static_cast<int>(config.chuteSpreadX));
    const float dx     = static_cast<float>(Roll(_env, 2 * spread + 1) - spread);
    float       x      = view.max.x + scroll - config.chuteRightMarginX + dx;
    const float lo     = view.min.x + scroll + config.chuteEdgeX;
    const float hi     = view.max.x + scroll - config.chuteEdgeX;
    if (lo <= hi)
    {
        x = Clamp(x, lo, hi);
    }
    const VECTOR2 feet = { x, view.min.y - config.chuteStartAboveY };
    SpawnParams   params;
    params.facing     = 1;
    params.bActivated = true;
    params.drop       = String(kChuteDrops[static_cast<size_t>(Roll(_env, static_cast<int>(kChuteDrops.size())))]);
    (void)_env.world.SpawnDeferred(index, feet, params);
}

[[nodiscard]] bool BigShieeEnemy::CannonVulnerable_() const
{
    const eBigShieeCannonMode mode = m_boss.cannonMode;
    if (mode == eBigShieeCannonMode::Hidden)
    {
        return false;
    }
    return mode != eBigShieeCannonMode::Deploy || m_boss.cannonTicks >= 30;
}

[[nodiscard]] float BigShieeEnemy::PlayerAngleDeg_(
    const BigShieeConfig& _config,
    const EnemyEnv&       _env) const
{
    const EnemyState& _e     = GetState();
    const VECTOR2     muzzle = World_(_e.body.position, _config.cannonAimOrigin);
    const float       dx     = _env.ctx.player.center.x - muzzle.x;
    const float       dy     = Max(_env.ctx.player.center.y - muzzle.y, 40.f);
    return ATan2(dx, dy) * kRad2Deg;
}

[[nodiscard]] int BigShieeEnemy::TargetYaw_(
    const BigShieeConfig& _config,
    const EnemyEnv&       _env,
    const bool            _bFire) const
{
    if (!_env.ctx.player.bAlive)
    {
        return m_boss.cannonYaw;
    }
    const float deg = PlayerAngleDeg_(_config, _env);
    if (!_bFire)
    {
        return NearestYaw_(_config, deg);
    }
    int best = 0;
    for (int row = 1; row < kAimRows; ++row)
    {
        if (Abs(_config.cannonAimDeg[static_cast<size_t>(row)] - deg) < Abs(_config.cannonAimDeg[static_cast<size_t>(best)] - deg))
        {
            best = row;
        }
    }
    return best * 2;
}

void BigShieeEnemy::TurnCannon_(
    const BigShieeConfig& _config,
    const int             _target)
{
    int& yaw  = m_boss.cannonYaw;
    int& wait = m_boss.cannonTurn;
    if (yaw == _target)
    {
        wait = 0;
        return;
    }
    if (++wait >= _config.cannonTurnTicks)
    {
        wait = 0;
        yaw += _target > yaw ? 1 : -1;
        EventLogFormat("bigshiee.yaw", "\"yaw\":{}", yaw);
    }
}

[[nodiscard]] float BigShieeEnemy::CasingLandSec_(
    const BigShieeConfig& _config) const
{
    const EnemyState& _e    = GetState();
    const float       fromY = static_cast<float>(m_boss.casingY);
    const float       drop  = Max(_e.body.position.y + _config.casingGroundDy - fromY, 0.f);
    const float       disc  = _config.casingVel.y * _config.casingVel.y + 2.f * _config.casingGravity * drop;
    return (-_config.casingVel.y + Sqrt(disc)) / _config.casingGravity;
}

bool BigShieeEnemy::CasingNow_(
    const BigShieeConfig& _config,
    VECTOR2&              _outAt,
    int&                  _outFrame,
    float&                _outAlpha) const
{
    const int start = m_boss.casingTick;
    if (start <= 0)
    {
        return false;
    }
    const float   t     = static_cast<float>(m_ageTicks - start) * kStepSec;
    const VECTOR2 from  = { static_cast<float>(m_boss.casingX), static_cast<float>(m_boss.casingY) };
    const float   tLand = CasingLandSec_(_config);
    if (t < 0.f || t > tLand + _config.casingLingerSec)
    {
        return false;
    }
    const float tFly = Min(tLand, t);
    _outAt           = { from.x + _config.casingVel.x * tFly, from.y + _config.casingVel.y * tFly + 0.5f * _config.casingGravity * tFly * tFly };
    _outFrame        = static_cast<int>(t * _config.casingFps) % kCasingFrames;
    _outAlpha        = t <= tLand || _config.casingLingerSec <= 0.f ? 1.f : Clamp(1.f - (t - tLand) / _config.casingLingerSec, 0.f, 1.f);
    return true;
}

void BigShieeEnemy::FireMissile_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const BigShieeConfig& config = _def.bigShiee;
    const size_t          row    = static_cast<size_t>(Clamp(m_boss.cannonRow, 0, kAimRows - 1));
    const int             weapon = _def.FindWeapon("missile");
    const float           sign   = static_cast<float>(_e.facing < 0 ? -1 : 1);
    const VECTOR2         delta  = { (config.cannonMuzzleX[row] - config.cannonMuzzleX[kAimRows / 2]) * sign, 0.f };
    const VECTOR2         base   = weapon >= 0 ? GetMuzzleWorld(_e, _def, weapon) : _e.body.position;
    const VECTOR2         muzzle = { base.x + delta.x, base.y + delta.y };
    const float           rad    = config.cannonAimDeg[row] * kDeg2Rad;
    const VECTOR2         dir    = { std::sin(rad), std::cos(rad) };
    EventLogFormat("bigshiee.missile_fire", "\"row\":{},\"x\":{:.1f},\"y\":{:.1f},\"dx\":{:.3f},\"dy\":{:.3f}", row, muzzle.x, muzzle.y, dir.x, dir.y);
    (void)FireWeapon(_e, _def, _env, weapon, dir, delta);
    // 사격 순간 포구 섬광 과 연기
    EventLogFormat("bigshiee.muzzle_fx", "\"x\":{:.1f},\"y\":{:.1f}", muzzle.x, muzzle.y);
    RequestFxAt(_env, "Bazuka_Shot_Effect", muzzle, _e.facing, 55);
    RequestFxAt(_env, "Boss_Smoke_Small", muzzle, _e.facing, 54);
    const VECTOR2 casing = { muzzle.x + config.casingOffset.x, muzzle.y + config.casingOffset.y };
    m_boss.casingTick    = m_ageTicks;
    m_boss.casingX       = static_cast<int>(std::lround(casing.x));
    m_boss.casingY       = static_cast<int>(std::lround(casing.y));
    EventLogFormat("bigshiee.casing", "\"x\":{:.1f},\"y\":{:.1f}", casing.x, casing.y);
}

void BigShieeEnemy::TickCannon_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&               _e     = GetMutableState();
    const BigShieeConfig&     config = _def.bigShiee;
    const eBigShieeCannonMode mode   = m_boss.cannonMode;
    if (mode == eBigShieeCannonMode::Hidden)
    {
        return;
    }
    int& ticks = m_boss.cannonTicks;
    ++ticks;

    if (mode == eBigShieeCannonMode::Deploy)
    {
        const int deployTicks = config.cannonDeployTicks > 0 ? config.cannonDeployTicks : ClipTicks_(_env, "Boss_Cannon_Deploy", 70);
        if (ticks >= deployTicks)
        {
            SetCannonMode_(eBigShieeCannonMode::Aim);
            m_boss.cannonYaw  = 0;
            m_boss.cannonTurn = 0;
        }
    }
    else if (mode == eBigShieeCannonMode::Aim)
    {
        const int target = TargetYaw_(config, _env, false);
        TurnCannon_(config, target);
        if (m_boss.cannonYaw == target)
        {
            SetCannonMode_(eBigShieeCannonMode::Ready);
            m_boss.nextMissile = m_ageTicks + SecToTicks(config.firstMissileSec);
            EnterState(_e, static_cast<int>(eBigShieePhase::Cannon));
        }
    }
    else if (mode == eBigShieeCannonMode::Ready)
    {
        const bool bFire    = m_ageTicks >= m_boss.nextMissile && _env.ctx.player.bAlive;
        const int  target   = TargetYaw_(config, _env, bFire);
        const bool bAligned = m_boss.cannonYaw == target;
        TurnCannon_(config, target);
        if (bFire && bAligned)
        {
            m_boss.cannonRow   = target / 2;
            m_boss.nextMissile = m_ageTicks + SecToTicks(config.missileIntervalSec);
            SetCannonMode_(eBigShieeCannonMode::Shot);
        }
    }
    else if (mode == eBigShieeCannonMode::Shot)
    {
        if (ticks == config.missileFireTick)
        {
            FireMissile_(_def, _env);
        }
        const String shot = "Boss_Cannon_Shot" + std::to_string(m_boss.cannonRow + 1);
        if (ticks >= ShotTicks_(_env, shot))
        {
            SetCannonMode_(eBigShieeCannonMode::Ready);
        }
    }
}

void BigShieeEnemy::SubmitHull_(
    const BigShieeConfig& _config,
    const EnemyRenderEnv& _render,
    const VECTOR2         _feet) const
{
    const EnemyState&    _e      = GetState();
    const bool           bMoving = m_boss.moving != 0;
    const eBigShieePhase phase   = Phase_();
    const int            age     = m_ageTicks;
    const char*          clip    = bMoving ? "Boss_Hull_Drive" : "Boss_Hull_Idle";
    bool                 bLoop   = true;
    int                  ticks   = age;

    if (_e.life == eEnemyLife::Dying)
    {
        clip  = "Boss_Hull_Open";
        ticks = 0;
    }
    else if (phase == eBigShieePhase::Deploy)
    {
        const int step = (age - m_boss.lidStart) / Max(1, _config.hullStepTicks);
        clip           = step <= 0 ? "Boss_Hull_D1" : (step == 1 ? "Boss_Hull_D3" : (step == 2 ? "Boss_Hull_D6" : (step == 3 ? "Boss_Hull_D9" : "Boss_Hull_Open")));
        ticks          = step >= 4 ? age : 0;
    }
    else if (phase == eBigShieePhase::Cannon)
    {
        clip = bMoving ? "Boss_Hull_OpenDrive" : "Boss_Hull_Open";
    }
    SubmitNamedPart(_render, clip, _feet, ticks, bLoop, _config.partHull.sort, _config.partHull.depth);
}

void BigShieeEnemy::SubmitTurret_(
    const BigShieeConfig& _config,
    const EnemyRenderEnv& _render,
    const VECTOR2         _feet,
    const int             _i) const
{
    const EnemyState&           _e     = GetState();
    const BigShieeTurretConfig& turret = _config.turrets[static_cast<size_t>(_i)];
    const EnemyPartDepth&       part   = _config.partTurrets[static_cast<size_t>(_i)];
    const TurretNames&          names  = GetNames_(turret.letter);
    const eBigShieeTurretMode   mode   = m_boss.turretMode[static_cast<size_t>(_i)];
    const VECTOR2               origin = World_(_feet, turret.anchor);
    const int                   ticks  = m_boss.turretTicks[static_cast<size_t>(_i)];
    const int                   pos    = m_boss.turretPosition[static_cast<size_t>(_i)];
    const float                 flash  = PartFlashAmount(_e, _i);

    switch (mode)
    {
        case eBigShieeTurretMode::Dead:
            (void)SubmitNamedPart(_render, names.dead, origin, 0, false, part.sort, part.depth);
            break;
        case eBigShieeTurretMode::Turn:
            (void)SubmitFrame_(_render, names.turn, pos, origin, part.depth, part.sort - _i, flash);
            break;
        case eBigShieeTurretMode::Shot:
            (void)SubmitNamedPart(_render, names.shot[static_cast<size_t>(Min(2, pos / kTurnMid))], origin, ticks, false, part.sort, part.depth, false, flash);
            break;
        default:
            if (pos % kTurnMid != 0)
            {
                (void)SubmitFrame_(_render, names.turn, pos, origin, part.depth, part.sort - _i, flash);
            }
            else
            {
                (void)SubmitNamedPart(_render, names.idle[static_cast<size_t>(pos / kTurnMid)], origin, m_ageTicks + _i * 7, true, part.sort, part.depth, false, flash);
            }
            break;
    }
}

void BigShieeEnemy::SubmitCannon_(
    const BigShieeConfig& _config,
    const EnemyRenderEnv& _render,
    const VECTOR2         _feet,
    const float           _flash) const
{
    const eBigShieeCannonMode mode   = m_boss.cannonMode;
    const int                 ticks  = m_boss.cannonTicks;
    const int                 row    = Clamp(m_boss.cannonRow, 0, kAimRows - 1);
    const int                 yaw    = Clamp(m_boss.cannonYaw, 0, kYawMax);
    const VECTOR2             origin = World_(_feet, _config.cannonAnchor);
    const String              shot   = "Boss_Cannon_Shot" + std::to_string(row + 1);
    switch (mode)
    {
        case eBigShieeCannonMode::Deploy:
            (void)SubmitNamedPart(_render, "Boss_Cannon_Deploy", origin, DeployClipTicks_(_config, _render, ticks), false, _config.partCannon.sort, _config.partCannon.depth, false, _flash);
            break;
        case eBigShieeCannonMode::Aim:
        case eBigShieeCannonMode::Ready:
            (void)SubmitFrame_(_render, "Boss_Cannon_Aim", yaw, origin, _config.partCannon.depth, _config.partCannon.sort, _flash);
            break;
        case eBigShieeCannonMode::Shot:
            (void)SubmitNamedPart(_render, shot, origin, ticks, false, _config.partCannon.sort, _config.partCannon.depth, false, _flash);
            break;
        default:
            break;
    }
}

void BigShieeEnemy::SubmitCasing_(
    const BigShieeConfig& _config,
    const EnemyRenderEnv& _render) const
{
    VECTOR2 at    = { 0.f, 0.f };
    int     frame = 0;
    float   alpha = 1.f;
    if (!_render.pSprites || !CasingNow_(_config, at, frame, alpha))
    {
        return;
    }
    const SpriteInfo* pInfo = _render.pSprites->Find("Boss_CasingStrip");
    if (!pInfo || !pInfo->bStandalone)
    {
        return;
    }
    const int handle = _render.sprites.FindStandalone(pInfo->file);
    if (handle < 0)
    {
        return;
    }
    const int         cellW = pInfo->width / kCasingFrames;
    SpriteDrawOptions options;
    options.tint.a = static_cast<uint8_t>(std::lround(alpha * 255.f));
    options.scale  = _config.casingScale;
    _render.sprites.SubmitStandalone(kCasingDepth + _render.zOffset, 0, handle, { frame * cellW, 0, cellW, pInfo->height }, at, options);
}

void BigShieeEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, m_ageTicks);
}

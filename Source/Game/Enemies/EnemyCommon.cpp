#include "Game/Enemies/EnemyCommon.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Sandbox/Effects.h"
#include "Render/SpriteBatch.h"

constexpr float kFaceDeadzone = 4.f;

enum class eDeathPhase
{
    None  = 0,
    Stand = 1,
    Fall  = 2,
    Land  = 3,
    Slash = 4,
    Bomb  = 5,
    Jump  = 6
};

namespace
{
[[nodiscard]] bool IsFinalPhase_(
    eDeathPhase _phase)
{
    return _phase == eDeathPhase::Land || _phase == eDeathPhase::Slash || _phase == eDeathPhase::Bomb;
}

void EnterDeathLand_(
    Enemy&          _e,
    const DeathDef& _death)
{
    _e.deathPhase = static_cast<int>(eDeathPhase::Land);
    _e.deathTicks = 0;
    SetClip(_e, _death.clipLand, true);
}
}   // namespace

float DistToPlayer(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    const float dx = DxToPlayer(_e, _env);
    const float dy = DyToPlayer(_e, _env);
    return Sqrt(dx * dx + dy * dy);
}

int SideToPlayer(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    const float dx = DxToPlayer(_e, _env);
    if (Abs(dx) <= kFaceDeadzone)
    {
        return _e.facing;
    }
    return dx < 0.f ? -1 : 1;
}

void FacePlayer(
    Enemy&          _e,
    const EnemyEnv& _env)
{
    _e.facing = SideToPlayer(_e, _env);
}

void EnterState(
    Enemy& _e,
    int    _state)
{
    _e.state      = _state;
    _e.stateTicks = 0;
}

void SetClip(
    Enemy& _e,
    int    _clip,
    bool   _bRestart)
{
    if (_clip != _e.clip || _bRestart)
    {
        _e.clip      = _clip;
        _e.clipTicks = 0;
    }
}

bool ClipReachedFrame(
    const Enemy&    _e,
    const EnemyDef& _def,
    int             _frame)
{
    if (_e.clip < 0)
    {
        return false;
    }
    return _e.clipTicks == _def.GetClip(_e.clip).TickOfFrame(_frame);
}

bool ClipFinished(
    const Enemy&    _e,
    const EnemyDef& _def)
{
    return _e.clip >= 0 && _def.GetClip(_e.clip).IsFinished(_e.clipTicks);
}

bool ClimbStep(
    Enemy&          _e,
    const EnemyDef& _def)
{
    VECTOR2 target = _e.arena.max;
    if (_e.aux == 0)
    {
        target = _e.arena.min;
    }
    else if (_e.aux == 2)
    {
        target = _e.climbEnd;
    }
    const VECTOR2 delta = { target.x - _e.body.position.x, target.y - _e.body.position.y };
    const float   dist  = Sqrt(delta.x * delta.x + delta.y * delta.y);
    const float   step  = _def.physics.speed * kStepSec;

    _e.body.velocity = { 0.f, 0.f };
    if (Abs(delta.x) > 0.01f)
    {
        _e.facing = delta.x < 0.f ? -1 : 1;
    }
    if (dist > step)
    {
        _e.body.position.x += delta.x / dist * step;
        _e.body.position.y += delta.y / dist * step;
        return false;
    }

    _e.body.position   = target;
    const bool bHasEnd = _e.climbEnd.y != 0.f;
    if (_e.aux == 0 || (_e.aux == 1 && bHasEnd))
    {
        ++_e.aux;
        return false;
    }
    return true;
}

void ClimbLeap(
    Enemy&          _e,
    const EnemyDef& _def)
{
    _e.body.velocity = { static_cast<float>(_e.facing) * _def.physics.speed, _def.physics.jumpVy };
    _e.bLeap         = true;
}

constexpr float kRoofEdgeMargin = 4.f;
constexpr float kRoofLeaveZone  = 30.f;

bool OnRoof(
    const Enemy& _e)
{
    return _e.roofSpan.y > _e.roofSpan.x && _e.bGrounded && Abs(_e.body.position.y - _e.climbEnd.y) < 2.f;
}

bool AtRoofEdge(
    const Enemy& _e)
{
    if (!OnRoof(_e))
    {
        return false;
    }
    return _e.facing < 0 ? _e.body.position.x <= _e.roofSpan.x + kRoofEdgeMargin + 0.5f : _e.body.position.x >= _e.roofSpan.y - kRoofEdgeMargin - 0.5f;
}

bool CanLeaveRoof(
    const Enemy& _e)
{
    if (!OnRoof(_e))
    {
        return true;
    }
    return _e.facing < 0 ? _e.body.position.x <= _e.roofSpan.x + kRoofLeaveZone : _e.body.position.x >= _e.roofSpan.y - kRoofLeaveZone;
}

void HoldOnRoof(
    Enemy& _e)
{
    if (!OnRoof(_e))
    {
        return;
    }
    const float x = Clamp(_e.body.position.x, _e.roofSpan.x + kRoofEdgeMargin, _e.roofSpan.y - kRoofEdgeMargin);
    if (x != _e.body.position.x)
    {
        _e.body.position.x = x;
        _e.body.velocity.x = 0.f;
    }
}

void PrepareBody(
    Enemy&          _e,
    const EnemyDef& _def)
{
    _e.body.size          = { _def.body.w, _def.body.h };
    _e.body.footHalfWidth = _def.physics.footHalfWidth;
}

MoveResult StepPhysics(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    const MoveResult result = Physics::Step(_e.body, _env.ctx.physics, _env.phys);
    _e.bGrounded            = _e.body.bGrounded;
    _e.bOnOneWay            = false;
    if (_e.body.bGrounded && result.platformIndex >= 0 && static_cast<size_t>(result.platformIndex) < _env.ctx.physics.platforms.size())
    {
        _e.bOnOneWay = _env.ctx.physics.platforms[static_cast<size_t>(result.platformIndex)].bOneWay;
    }
    return result;
}

bool WallAhead(
    const Enemy&    _e,
    const EnemyEnv& _env,
    float           _w,
    float           _h,
    float           _x,
    float           _y)
{
    const float         dir   = static_cast<float>(_e.facing < 0 ? -1 : 1);
    const Aabb          probe = Aabb::FromCenterSize({ _e.body.position.x + _x * dir, _e.body.position.y + _y }, { _w, _h });
    const PhysicsWorld& world = _env.ctx.physics;

    if (world.pGround)
    {
        if (world.pGround->IsRectSolid(FloorToInt(probe.min.x), FloorToInt(probe.min.y), CeilToInt(probe.max.x) - 1, CeilToInt(probe.max.y) - 1))
        {
            return true;
        }
    }
    for (const Wall& wall: world.walls)
    {
        if (probe.Overlaps(wall.box))
        {
            return true;
        }
    }
    for (const Platform& platform: world.platforms)
    {
        if (!platform.bOneWay && probe.Overlaps(platform.box))
        {
            return true;
        }
    }
    return false;
}

bool FloorAhead(
    const Enemy&    _e,
    const EnemyEnv& _env,
    float           _dist,
    float           _drop)
{
    Body probe = _e.body;
    probe.position.x += static_cast<float>(_e.facing < 0 ? -1 : 1) * _dist;
    probe.position.y -= 8.f;
    probe.bGrounded = false;
    return Physics::SnapToGround(probe, _env.ctx.physics, _drop + 8.f);
}

String ResolveSide(
    const String& _name,
    int           _facing)
{
    const size_t pos = _name.find("{side}");
    if (pos == String::npos)
    {
        return _name;
    }
    String result = _name;
    result.replace(pos, 6, _facing < 0 ? "Left" : "Right");
    return result;
}

void RequestFx(
    EnemyEnv&    _env,
    const FxDef& _fx,
    VECTOR2      _origin,
    int          _facing)
{
    if (_fx.clip.empty())
    {
        return;
    }
    EnemyFxRequest request;
    request.clip       = ResolveSide(_fx.clip, _facing);
    request.position   = { _origin.x + _fx.offset.x * static_cast<float>(_facing < 0 ? -1 : 1), _origin.y + _fx.offset.y };
    request.depth      = _fx.depth + _env.zOffset;
    request.delayTicks = SecToTicks(_fx.delaySec);
    request.bDebris    = _fx.bDebris;
    request.bFlipX     = _fx.bMirror && _facing > 0;
    if (_fx.bDebris)
    {
        // 잔해는 위로 솟구쳤다가 떨어진다
        const float vx   = kDebrisVxMin + static_cast<float>(_env.world.Roll(static_cast<int>(kDebrisVxMax - kDebrisVxMin) + 1));
        const float vy   = kDebrisVyMax - static_cast<float>(_env.world.Roll(static_cast<int>(kDebrisVyMax - kDebrisVyMin) + 1));
        request.velocity = { vx, vy };
    }
    _env.world.RequestFx(std::move(request));
}

VECTOR2 GetMuzzleWorld(
    const Enemy&    _e,
    const EnemyDef& _def,
    int             _weapon)
{
    const WeaponDef& weapon = _def.weapons[static_cast<size_t>(_weapon)];
    return { _e.body.position.x + weapon.muzzle.x * static_cast<float>(_e.facing < 0 ? -1 : 1), _e.body.position.y + weapon.muzzle.y };
}

void TickWeaponEffects(
    const Enemy&    _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    for (const WeaponDef& weapon: _def.weapons)
    {
        if (weapon.effects.empty() || _e.clip < 0 || _def.FindClip(weapon.motionClip) != _e.clip)
        {
            continue;
        }
        const ClipDef& clip = _def.GetClip(_e.clip);
        for (const WeaponEffect& effect: weapon.effects)
        {
            if (clip.TickOfFrame(effect.frame) != _e.clipTicks)
            {
                continue;
            }
            FxDef fx;
            fx.clip   = effect.clip;
            fx.offset = effect.offset;
            fx.depth  = effect.depth;
            RequestFx(_env, fx, _e.body.position, _e.facing);
        }
    }
}

bool FireWeapon(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    int             _weapon,
    VECTOR2         _dir,
    VECTOR2         _muzzleExtra)
{
    if (_weapon < 0 || !_env.ctx.pProjectiles || !_env.ctx.pCombat || !_env.ctx.player.bAlive)
    {
        return false;
    }
    const WeaponDef& weapon = _def.weapons[static_cast<size_t>(_weapon)];
    if (weapon.projectile < 0)
    {
        return false;
    }
    const ProjectileDef& proj   = _env.lib.GetProjectile(weapon.projectile);
    const float          facing = static_cast<float>(_e.facing < 0 ? -1 : 1);
    const VECTOR2        base   = GetMuzzleWorld(_e, _def, _weapon);
    const VECTOR2        muzzle = { base.x + _muzzleExtra.x, base.y + _muzzleExtra.y };

    VECTOR2 dir = weapon.aim == eWeaponAim::Given ? _dir : VECTOR2 { facing, 0.f };
    if (weapon.tiltThreshold > 0.f)
    {
        const float dy = _env.ctx.player.center.y - muzzle.y;
        if (Abs(dy) > weapon.tiltThreshold)
        {
            dir.y += dy > 0.f ? weapon.tiltDy : -weapon.tiltDy;
        }
    }
    {
        const float length = Sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.0001f)
        {
            dir = { dir.x / length, dir.y / length };
        }
        else
        {
            dir = { facing, 0.f };
        }
    }

    ShotSpawn spawn;
    spawn.team            = eTeam::Enemy;
    spawn.owner           = _e.id;
    spawn.position        = muzzle;
    spawn.size            = proj.size;
    spawn.damage          = proj.damage;
    spawn.kind            = eHitKind::Bullet;
    spawn.gravity         = proj.gravity;
    spawn.maxFallSpeed    = proj.maxFall;
    spawn.lifetimeTicks   = SecToTicks(proj.lifetimeSec);
    spawn.startDelayTicks = SecToTicks(proj.startDelaySec);
    spawn.bExplodeOnSolid = proj.bExplodeOnSolid;
    spawn.visual          = static_cast<uint16_t>(weapon.projectile + 1);
    spawn.rollSpeed       = proj.rollSpeed;
    spawn.rollDirX        = facing;
    spawn.rollTicks       = proj.rollSpeed > 0.f ? SecToTicks(proj.rollSec) : 0;
    spawn.bExpireFx       = proj.bExpireFx;
    spawn.groundProbeY    = proj.groundProbeY;
    spawn.blastSize       = proj.blastSize;
    spawn.blastUp         = proj.blastUp;
    spawn.blastTicks      = SecToTicks(proj.blastSec);
    spawn.blastDamage     = proj.blastDamage;

    if (proj.speed > 0.f)
    {
        spawn.velocity = { dir.x * proj.speed, dir.y * proj.speed };
    }
    else if (proj.bRamp)
    {
        spawn.velocity = { dir.x * proj.rampFrom, dir.y * proj.rampFrom };
        spawn.accel    = { dir.x * proj.rampAccel, dir.y * proj.rampAccel };
        spawn.maxSpeed = proj.rampMax;
    }
    else
    {
        spawn.velocity = { proj.v0.x * facing, proj.v0.y };
    }

    if (weapon.bArc)
    {
        const float vy0  = proj.v0.y;
        const float g    = proj.gravity;
        const float tx   = _env.ctx.player.feet.x;
        const float ty   = _env.ctx.player.center.y;
        const float dy   = ty - muzzle.y;
        const float disc = vy0 * vy0 + 2.f * g * dy;
        float       t    = g > 0.f ? (disc >= 0.f ? (-vy0 + Sqrt(disc)) / g : -vy0 / g) : 1.f;
        if (t < 0.1f)
        {
            t = 0.1f;
        }
        const float dx   = tx - muzzle.x;
        const float sign = Abs(dx) < 1.f ? facing : (dx < 0.f ? -1.f : 1.f);
        const float vx   = Clamp(Abs(dx) / t, weapon.arcMinVx, weapon.arcMaxVx);
        spawn.velocity   = { vx * sign, vy0 };
    }

    spawn.z = _env.zOffset != 0 ? kEnemyShotDepth + _env.zOffset : kDefaultZ;
    _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
    ++_e.shotsFired;
    EventLogFormat("enemy.shot", "\"def\":\"{}\",\"weapon\":{},\"x\":{:.1f},\"y\":{:.1f},\"vx\":{:.1f},\"vy\":{:.1f}", _def.name, _weapon, muzzle.x, muzzle.y, spawn.velocity.x, spawn.velocity.y);

    if (!weapon.muzzleFx.clip.empty())
    {
        RequestFx(_env, weapon.muzzleFx, muzzle, _e.facing);
    }
    return true;
}

void StartMelee(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    int             _weapon)
{
    CancelMelee(_e, _env);
    const WeaponDef& weapon = _def.weapons[static_cast<size_t>(_weapon)];
    _e.meleeWeapon          = _weapon;
    _e.meleeTicksLeft       = SecToTicks(weapon.hitboxLifeSec);
    _e.meleeOrigin          = _e.body.position;
    _e.meleeHitId           = _env.ctx.pCombat ? _env.ctx.pCombat->NewHitId() : kNoHitId;
}

void TickMelee(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    if (_e.meleeTicksLeft <= 0)
    {
        if (_e.meleeHitId != kNoHitId && _env.ctx.pCombat)
        {
            _env.ctx.pCombat->ReleaseHitId(_e.meleeHitId);
        }
        _e.meleeHitId = kNoHitId;
        return;
    }
    if (!_env.ctx.pCombat)
    {
        --_e.meleeTicksLeft;
        return;
    }

    const WeaponDef& weapon = _def.weapons[static_cast<size_t>(_e.meleeWeapon)];
    const VECTOR2    origin = weapon.bHitboxFollows ? _e.body.position : _e.meleeOrigin;

    Hitbox hitbox;
    hitbox.box     = weapon.hitbox.At(origin, _e.facing);
    hitbox.team    = eTeam::Enemy;
    hitbox.kind    = eHitKind::Melee;
    hitbox.damage  = weapon.hitboxDamage;
    hitbox.bPierce = false;
    hitbox.owner   = _e.id;
    hitbox.id      = _e.meleeHitId;
    _env.ctx.pCombat->AddHitbox(hitbox);
    --_e.meleeTicksLeft;
}

void CancelMelee(
    Enemy&    _e,
    EnemyEnv& _env)
{
    if (_e.meleeHitId != kNoHitId && _env.ctx.pCombat)
    {
        _env.ctx.pCombat->ReleaseHitId(_e.meleeHitId);
    }
    _e.meleeHitId     = kNoHitId;
    _e.meleeTicksLeft = 0;
}

void BeginSoldierDeath(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const DeathDef& death = _def.death;
    CancelMelee(_e, _env);

    _e.body.velocity.x  = 0.f;
    _e.body.bUseGravity = true;
    _e.deathTicks       = 0;

    if (_e.lastHitKind == eHitKind::Blast && death.clipBomb >= 0)
    {
        const float away = SideToPlayer(_e, _env) < 0 ? 1.f : -1.f;

        _e.deathPhase = static_cast<int>(eDeathPhase::Bomb);
        _e.aux        = static_cast<int>(_e.body.position.y);
        _e.body.position.y -= 30.f;
        _e.body.velocity       = { away * death.bombVelocity.x, death.bombVelocity.y };
        _e.body.bCollideGround = false;
        _e.body.bCollideSolids = false;
        _e.body.bCollideOneWay = false;
        _e.body.bGrounded      = false;
        _e.bGrounded           = false;
        SetClip(_e, death.clipBomb, true);
    }
    else if (_e.lastHitKind == eHitKind::Melee && death.clipSlash >= 0)
    {
        _e.deathPhase = static_cast<int>(eDeathPhase::Slash);
        SetClip(_e, death.clipSlash, true);
    }
    else if (_e.bGrounded && death.clipStand1 >= 0)
    {
        const bool bFirst = death.clipStand2 < 0 || _env.world.Roll(2) == 0;
        _e.deathPhase     = static_cast<int>(eDeathPhase::Stand);
        _e.aux            = bFirst ? death.fallAt1 : death.fallAt2;
        SetClip(_e, bFirst ? death.clipStand1 : death.clipStand2, true);
    }
    else
    {
        _e.deathPhase = static_cast<int>(eDeathPhase::Jump);
        SetClip(_e, death.clipJump >= 0 ? death.clipJump : death.clipFall, true);
    }

    for (const FxDef& fx: death.fx)
    {
        RequestFx(_env, fx, _e.body.position, _e.facing);
    }

    if (_def.bHuman && _e.lastHitKind != eHitKind::Blast)
    {
        const float    side = _e.facing < 0 ? -1.f : 1.f;
        EnemyFxRequest request;
        request.clip = _e.lastHitKind == eHitKind::Melee ? "Fx_Blood_Splat_" : "Fx_Blood_Hit_";
        request.clip += (_e.id & 1u) != 0u ? "Right" : "Left";
        request.position = { _e.body.position.x + _def.body.cx * side, _e.body.position.y + _def.body.cy + death.bloodOffsetY };
        request.depth    = 60 + _env.zOffset;
        _env.world.RequestFx(std::move(request));
    }
}

void TickSoldierDeath(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const DeathDef& death   = _def.death;
    const bool      bFlying = _e.deathPhase == static_cast<int>(eDeathPhase::Bomb);
    if (!bFlying)
    {
        _e.body.velocity.x = 0.f;
    }
    _e.body.bUseGravity = true;
    (void)StepPhysics(_e, _def, _env);

    if (bFlying)
    {
        ++_e.deathTicks;
        constexpr float kSpriteHeight = 90.f;
        const Aabb&     view          = _env.ctx.cameraView;
        const bool      bFell         = _e.body.velocity.y > 0.f && _e.body.position.y > static_cast<float>(_e.aux) + 60.f;
        if (view.max.y > view.min.y)
        {
            const bool bOut = _e.body.velocity.y > 0.f && _e.body.position.y - kSpriteHeight > view.max.y;
            if (bOut || (bFell && _e.deathTicks > 600))
            {
                _e.life = eEnemyLife::Gone;
            }
        }
        else if (bFell)
        {
            _e.life = eEnemyLife::Gone;
        }
        return;
    }

    // 칼 사망은 시체를 남기지
    if (_e.deathPhase == static_cast<int>(eDeathPhase::Slash))
    {
        if (ClipFinished(_e, _def))
        {
            _e.bVisible = false;
            _e.life     = eEnemyLife::Gone;
        }
        return;
    }

    switch (static_cast<eDeathPhase>(_e.deathPhase))
    {
        case eDeathPhase::Stand:
        {
            const int fallTicks = _def.GetClip(_e.clip).TickOfFrame(_e.aux);
            if (_e.clipTicks >= fallTicks)
            {
                if (_e.bGrounded)
                {
                    EnterDeathLand_(_e, death);
                }
                else
                {
                    _e.deathPhase = static_cast<int>(eDeathPhase::Fall);
                    SetClip(_e, death.clipFall >= 0 ? death.clipFall : death.clipLand, true);
                }
            }
            break;
        }
        case eDeathPhase::Fall:
        case eDeathPhase::Jump:
            if (_e.bGrounded)
            {
                EnterDeathLand_(_e, death);
            }
            break;
        default: break;
    }

    if (IsFinalPhase_(static_cast<eDeathPhase>(_e.deathPhase)))
    {
        ++_e.deathTicks;
        const int blinkAt  = SecToTicks(death.blinkAtSec);
        const int blinkFor = SecToTicks(death.blinkForSec);
        const int period   = Max(1, SecToTicks(death.blinkPeriodSec));
        if (_e.deathTicks >= blinkAt && _e.deathTicks < blinkAt + blinkFor)
        {
            _e.bVisible = (((_e.deathTicks - blinkAt) / period) % 2) == 0;
        }
        else
        {
            _e.bVisible = true;
        }
        if (_e.deathTicks >= SecToTicks(death.removeAtSec))
        {
            _e.life = eEnemyLife::Gone;
        }
    }
}

bool SubmitNamedPart(
    const EnemyRenderEnv& _render,
    StringView            _name,
    VECTOR2               _origin,
    int                   _ticks,
    bool                  _bLoop,
    int                   _sortKey,
    int                   _depth,
    bool                  _bFlipX,
    float                 _partFlash)
{
    const float flash = Max(_partFlash, _render.flash);
    if (const AnimClip* pClip = _render.anims.FindClip(_name))
    {
        if (pClip->frames.empty())
        {
            return false;
        }
        const int64_t units = pClip->GetFrameDurationUnits();
        int           index = 0;
        if (units > 0)
        {
            index = static_cast<int>((static_cast<int64_t>(Max(0, _ticks)) * kAnimUnitsPerTick) / units);
        }
        const int count        = pClip->GetFrameCount();
        index                  = _bLoop ? index % count : Min(index, count - 1);
        const AnimFrame& frame = pClip->frames[static_cast<size_t>(index)];
        if (frame.IsEmpty())
        {
            return false;
        }
        SpriteDrawOptions options;
        options.bFlipX      = _bFlipX;
        options.flashAmount = flash;
        _render.sprites.Submit(_depth + _render.zOffset, _sortKey, frame, _origin, options);
        return true;
    }
    if (_render.pSprites)
    {
        if (const SpriteInfo* pSprite = _render.pSprites->Find(_name))
        {
            if (!pSprite->bStandalone && !pSprite->frame.IsEmpty())
            {
                SpriteDrawOptions options;
                options.bFlipX      = _bFlipX;
                options.flashAmount = flash;
                _render.sprites.Submit(_depth + _render.zOffset, _sortKey, pSprite->frame, _origin, options);
                return true;
            }
        }
    }
    return false;
}

int GetNamedPartTicks(
    const AnimationLibrary& _anims,
    StringView              _name)
{
    const AnimClip* pClip = _anims.FindClip(_name);
    if (!pClip)
    {
        return 0;
    }
    return static_cast<int>(pClip->GetTotalDurationUnits() / kAnimUnitsPerTick);
}

float DxToPlayer(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    return _env.ctx.player.feet.x - _e.body.position.x;
}

float DyToPlayer(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    return _env.ctx.player.feet.y - _e.body.position.y;
}

int Roll(
    EnemyEnv& _env,
    int       _n)
{
    return _env.world.Roll(_n);
}

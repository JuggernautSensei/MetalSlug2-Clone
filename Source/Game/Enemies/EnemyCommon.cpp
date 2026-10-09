#include "Game/Enemies/EnemyCommon.h"
#include "Base/Types.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "Base/Math.h"

namespace slug
{

namespace
{
    constexpr float kFaceDeadzone = 4.f;   // 플레이어와 가로 거리가 이 이내면 방향을 바꾸지 않는다

    enum eDeathPhase : int
    {
        kDeathNone  = 0,
        kDeathStand = 1,   // 서서 쓰러지는 중 (fallAt 프레임까지)
        kDeathFall  = 2,   // 공중에서 낙하 (Die_Fall)
        kDeathLand  = 3,   // 최종 (시체)
        kDeathSlash = 4,   // 최종: 칼에 베임
        kDeathBomb  = 5,   // 최종: 폭발로 날아감
        kDeathJump  = 6    // 공중 사망 → 착지하면 Land
    };

    [[nodiscard]] bool IsFinalPhase_(
        const int _phase)
    {
        return _phase == kDeathLand || _phase == kDeathSlash || _phase == kDeathBomb;
    }
}

float DistToPlayer(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    const float dx = DxToPlayer(_e, _env);
    const float dy = DyToPlayer(_e, _env);
    return std::sqrt(dx * dx + dy * dy);
}

int SideToPlayer(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    const float dx = DxToPlayer(_e, _env);
    if (std::abs(dx) <= kFaceDeadzone)
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
    Enemy&    _e,
    const int _state)
{
    _e.state      = _state;
    _e.stateTicks = 0;
}

void SetClip(
    Enemy&    _e,
    const int _clip,
    const bool _bRestart)
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
    const int       _frame)
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
    const float     _w,
    const float     _h,
    const float     _x,
    const float     _y)
{
    const float  dir    = static_cast<float>(_e.facing < 0 ? -1 : 1);
    const Aabb   probe  = Aabb::FromCenterSize({ _e.body.position.x + _x * dir, _e.body.position.y + _y }, { _w, _h });
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
    const float     _dist,
    const float     _drop)
{
    Body probe        = _e.body;
    probe.position.x += static_cast<float>(_e.facing < 0 ? -1 : 1) * _dist;
    probe.position.y -= 8.f;
    probe.bGrounded   = false;
    return Physics::SnapToGround(probe, _env.ctx.physics, _drop + 8.f);
}

slug::String ResolveSide(
    const slug::String& _name,
    const int          _facing)
{
    const size_t pos = _name.find("{side}");
    if (pos == slug::String::npos)
    {
        return _name;
    }
    slug::String result = _name;
    result.replace(pos, 6, _facing < 0 ? "Left" : "Right");
    return result;
}

void RequestFx(
    EnemyEnv&         _env,
    const FxDef&      _fx,
    const Vec2 _origin,
    const int         _facing)
{
    if (_fx.clip.empty())
    {
        return;
    }
    EnemyFxRequest request;
    request.clip       = ResolveSide(_fx.clip, _facing);
    request.position   = { _origin.x + _fx.offset.x * static_cast<float>(_facing < 0 ? -1 : 1), _origin.y + _fx.offset.y };
    request.layer      = _fx.layer;
    request.delayTicks = SecToTicks(_fx.delaySec);
    _env.world.RequestFx(std::move(request));
}

Vec2 GetMuzzleWorld(
    const Enemy&    _e,
    const EnemyDef& _def,
    const int       _weapon)
{
    const WeaponDef& weapon = _def.weapons[static_cast<size_t>(_weapon)];
    return { _e.body.position.x + weapon.muzzle.x * static_cast<float>(_e.facing < 0 ? -1 : 1), _e.body.position.y + weapon.muzzle.y };
}

bool FireWeapon(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _weapon,
    const Vec2 _dir,
    const Vec2 _muzzleExtra)
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
    const Vec2   base   = GetMuzzleWorld(_e, _def, _weapon);
    const Vec2   muzzle = { base.x + _muzzleExtra.x, base.y + _muzzleExtra.y };

    Vec2 dir = weapon.aim == eWeaponAim::Given ? _dir : Vec2 { facing, 0.f };
    if (weapon.tiltThreshold > 0.f)
    {
        const float dy = _env.ctx.player.center.y - muzzle.y;
        if (std::abs(dy) > weapon.tiltThreshold)
        {
            dir.y += dy > 0.f ? weapon.tiltDy : -weapon.tiltDy;
        }
    }
    {
        const float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
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
        const float vy0 = proj.v0.y;
        const float g   = proj.gravity;
        const float tx  = _env.ctx.player.feet.x;
        const float ty  = _env.ctx.player.center.y;
        const float dy  = ty - muzzle.y;
        const float disc = vy0 * vy0 + 2.f * g * dy;
        float       t    = g > 0.f ? (disc >= 0.f ? (-vy0 + std::sqrt(disc)) / g : -vy0 / g) : 1.f;
        if (t < 0.1f)
        {
            t = 0.1f;
        }
        const float dx   = tx - muzzle.x;
        const float sign = std::abs(dx) < 1.f ? facing : (dx < 0.f ? -1.f : 1.f);
        const float vx   = slug::Clamp(std::abs(dx) / t, weapon.arcMinVx, weapon.arcMaxVx);
        spawn.velocity   = { vx * sign, vy0 };
    }

    _env.ctx.pProjectiles->SpawnShot(spawn, *_env.ctx.pCombat);
    ++_e.shotsFired;

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
    const int       _weapon)
{
    CancelMelee(_e, _env);
    const WeaponDef& weapon = _def.weapons[static_cast<size_t>(_weapon)];
    _e.meleeWeapon    = _weapon;
    _e.meleeTicksLeft = SecToTicks(weapon.hitboxLifeSec);
    _e.meleeOrigin    = _e.body.position;
    _e.meleeHitId     = _env.ctx.pCombat ? _env.ctx.pCombat->NewHitId() : kNoHitId;
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
    const Vec2 origin = weapon.bHitboxFollows ? _e.body.position : _e.meleeOrigin;

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

    _e.body.velocity.x = 0.f;
    _e.body.bUseGravity = true;
    _e.deathTicks      = 0;

    if (_e.lastHitKind == eHitKind::Blast && death.clipBomb >= 0)
    {
        _e.deathPhase = kDeathBomb;
        SetClip(_e, death.clipBomb, true);
    }
    else if (_e.lastHitKind == eHitKind::Melee && death.clipSlash >= 0)
    {
        _e.deathPhase = kDeathSlash;
        SetClip(_e, death.clipSlash, true);
    }
    else if (_e.bGrounded && death.clipStand1 >= 0)
    {
        const bool bFirst = death.clipStand2 < 0 || _env.world.Roll(2) == 0;
        _e.deathPhase     = kDeathStand;
        _e.aux            = bFirst ? death.fallAt1 : death.fallAt2;
        SetClip(_e, bFirst ? death.clipStand1 : death.clipStand2, true);
    }
    else
    {
        _e.deathPhase = kDeathJump;
        SetClip(_e, death.clipJump >= 0 ? death.clipJump : death.clipFall, true);
    }

    for (const FxDef& fx: death.fx)
    {
        RequestFx(_env, fx, _e.body.position, _e.facing);
    }
}

void TickSoldierDeath(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const DeathDef& death = _def.death;
    _e.body.velocity.x    = 0.f;
    _e.body.bUseGravity   = true;
    (void)StepPhysics(_e, _def, _env);

    const auto EnterLand = [&]() {
        _e.deathPhase = kDeathLand;
        _e.deathTicks = 0;
        SetClip(_e, death.clipLand, true);
    };

    switch (_e.deathPhase)
    {
        case kDeathStand:
        {
            const int fallTicks = _def.GetClip(_e.clip).TickOfFrame(_e.aux);
            if (_e.clipTicks >= fallTicks)
            {
                if (_e.bGrounded)
                {
                    EnterLand();
                }
                else
                {
                    _e.deathPhase = kDeathFall;
                    SetClip(_e, death.clipFall >= 0 ? death.clipFall : death.clipLand, true);
                }
            }
            break;
        }
        case kDeathFall:
        case kDeathJump:
            if (_e.bGrounded)
            {
                EnterLand();
            }
            break;
        default: break;
    }

    if (IsFinalPhase_(_e.deathPhase))
    {
        ++_e.deathTicks;
        const int blinkAt  = SecToTicks(death.blinkAtSec);
        const int blinkFor = SecToTicks(death.blinkForSec);
        const int period   = slug::Max(1, SecToTicks(death.blinkPeriodSec));
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
    const slug::StringView _name,
    const Vec2    _origin,
    const int             _ticks,
    const bool            _bLoop,
    const int             _sortKey,
    const int             _layer,
    const bool            _bFlipX)
{
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
            index = static_cast<int>((static_cast<int64_t>(std::max(0, _ticks)) * kAnimUnitsPerTick) / units);
        }
        const int count = pClip->GetFrameCount();
        index           = _bLoop ? index % count : std::min(index, count - 1);
        const AnimFrame& frame = pClip->frames[static_cast<size_t>(index)];
        if (frame.IsEmpty())
        {
            return false;
        }
        SpriteDrawOptions options;
        options.bFlipX = _bFlipX;
        _render.sprites.Submit(_layer, _sortKey, frame, _origin, options);
        return true;
    }
    if (_render.pSprites)
    {
        if (const SpriteInfo* pSprite = _render.pSprites->Find(_name))
        {
            if (!pSprite->bStandalone && !pSprite->frame.IsEmpty())
            {
                SpriteDrawOptions options;
                options.bFlipX = _bFlipX;
                _render.sprites.Submit(_layer, _sortKey, pSprite->frame, _origin, options);
                return true;
            }
        }
    }
    return false;
}

int GetNamedPartTicks(
    const AnimationLibrary& _anims,
    const slug::StringView   _name)
{
    const AnimClip* pClip = _anims.FindClip(_name);
    if (!pClip)
    {
        return 0;
    }
    return static_cast<int>(pClip->GetTotalDurationUnits() / kAnimUnitsPerTick);
}
}

#include <algorithm>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace slug
{

namespace
{
    // 지면/벽/발판(윗면을 가로지르는 경우) 접촉 검사. _from → _to 는 접촉 판정 점의 이전/현재 위치
    [[nodiscard]] bool Touches_(
        const PhysicsWorld& _world,
        const Vec2          _from,
        const Vec2          _to)
    {
        if (_world.pGround && _world.pGround->IsSolidAt(_to))
        {
            return true;
        }
        for (const Wall& wall: _world.walls)
        {
            if (wall.box.Contains(_to))
            {
                return true;
            }
        }
        for (const Platform& platform: _world.platforms)
        {
            const float top = platform.box.min.y;
            if (platform.bOneWay)
            {
                // 원웨이 발판: 위에서 아래로 윗면을 가로지를 때만
                if (_from.y <= top && _to.y >= top && _to.x >= platform.box.min.x && _to.x <= platform.box.max.x)
                {
                    return true;
                }
            }
            else if (platform.box.Contains(_to))
            {
                return true;
            }
        }
        return false;
    }

    void AddBombHitbox_(
        const Enemy&         _e,
        const Vec2           _size,
        const int            _damage,
        const eHitKind       _kind,
        EnemyEnv&            _env)
    {
        if (!_env.ctx.pCombat || _e.meleeHitId == kNoHitId)
        {
            return;
        }
        Hitbox hitbox;
        hitbox.box     = Aabb::FromCenterSize(_e.body.position, _size);
        hitbox.team    = eTeam::Enemy;
        hitbox.kind    = _kind;
        hitbox.damage  = _damage;
        hitbox.bPierce = false;
        hitbox.owner   = _e.id;
        hitbox.id      = _e.meleeHitId;
        _env.ctx.pCombat->AddHitbox(hitbox);
    }

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        PrepareBody(_e, _def);
        _e.body.bUseGravity = false;
        _e.body.velocity    = { 0.f, _def.heliBomb.startVy };
        _e.meleeHitId       = _env.ctx.pCombat ? _env.ctx.pCombat->NewHitId() : kNoHitId;
        SetClip(_e, _def.heliBomb.clipFall, true);
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const HeliBombCfg& c = _def.heliBomb;
        if (!_e.bActive)
        {
            return;
        }

        const Vec2 probeFrom = { _e.body.position.x, _e.body.position.y + c.groundProbeY };
        _e.body.velocity.y   = Min(_e.body.velocity.y + c.gravity * kStepSec, c.maxFall);
        _e.body.position.y += _e.body.velocity.y * kStepSec;

        const Vec2 probeTo = { _e.body.position.x, _e.body.position.y + c.groundProbeY };
        if (Touches_(_env.ctx.physics, probeFrom, probeTo))
        {
            (void)_e.health.ApplyDamage(_e.health.hp);   // 착지: 터진다 (사망 처리 → onDeath)
            return;
        }
        AddBombHitbox_(_e, c.hitSize, c.blastDamage, eHitKind::Bullet, _env);
    }

    void OnDeath_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        BeginVehicleDeath(_e, _def, _env, -1);
        _e.bVisible = false;
        _e.aux      = 0;   // 폭발 상자 경과 틱
    }

    void TickDying_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const HeliBombCfg& c = _def.heliBomb;
        if (_e.aux < c.blastTicks)
        {
            AddBombHitbox_(_e, c.blastSize, c.blastDamage, eHitKind::Blast, _env);
            ++_e.aux;
            return;
        }
        if (_e.meleeHitId != kNoHitId && _env.ctx.pCombat)
        {
            _env.ctx.pCombat->ReleaseHitId(_e.meleeHitId);
        }
        _e.meleeHitId = kNoHitId;
        _e.life       = eEnemyLife::Gone;
    }

}   // namespace

const BehaviorOps kHeliBombOps = {
    .init      = &Init_,
    .tick      = &Tick_,
    .onDeath   = &OnDeath_,
    .tickDying = &TickDying_,
};

void ParseHeliBombCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    HeliBombCfg& c = _def.heliBomb;

    c.startVy      = _r.Num("startVy", c.startVy);
    c.gravity      = _r.Num("gravity", c.gravity);
    c.maxFall      = _r.Num("maxFall", c.maxFall);
    c.hitSize      = _r.ReadVec2("hitSize", c.hitSize);
    c.blastSize    = _r.ReadVec2("blastSize", c.blastSize);
    c.blastTicks   = _r.Ticks("blastSec", 0.13f);
    c.blastDamage  = _r.Int("blastDamage", c.blastDamage);
    c.groundProbeY = _r.Num("groundProbeY", c.groundProbeY);

    c.clipFall = _r.Clip("fall");
}

}   // namespace slug

#include "Game/Combat/Projectiles.h"

#include <algorithm>
#include <cmath>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Render/SpriteBatch.h"
#include "World/Geometry.h"

namespace
{
[[nodiscard]] bool SegmentHitsGround_(
    const GroundMap& _ground,
    const Vec2       _from,
    const Vec2       _to,
    Vec2&            _outLastFree)
{
    const float dx   = _to.x - _from.x;
    const float dy   = _to.y - _from.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    const int   n    = Max(1, CeilToInt(dist / 2.f));

    Vec2 lastFree = _from;
    for (int i = 1; i <= n; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(n);
        const Vec2  p = { _from.x + dx * t, _from.y + dy * t };
        if (_ground.IsSolidAt(p))
        {
            _outLastFree = lastFree;
            return true;
        }
        lastFree = p;
    }
    return false;
}

// 발판 윗면을 가로지르는 선분 (낙하 중에만). 가장 먼저 닿는 윗면과 그 교점을 돌려준다
[[nodiscard]] bool SegmentHitsPlatform_(
    Span<const Platform> _platforms,
    const Vec2           _from,
    const Vec2           _to,
    Vec2&                _outTop)
{
    bool  bHit  = false;
    float bestT = 2.f;
    for (const Platform& platform: _platforms)
    {
        const float top = platform.box.min.y;
        if (_to.y <= _from.y || _from.y > top || _to.y < top)
        {
            continue;
        }
        const float t = (top - _from.y) / (_to.y - _from.y);
        const float x = _from.x + (_to.x - _from.x) * t;
        if (t >= bestT || x < platform.box.min.x || x > platform.box.max.x)
        {
            continue;
        }
        bestT   = t;
        _outTop = { x, top };
        bHit    = true;
    }
    return bHit;
}

[[nodiscard]] Aabb SweptBox_(
    const Vec2 _a,
    const Vec2 _b,
    const Vec2 _size)
{
    return Aabb::FromMinMax(
        { Min(_a.x, _b.x) - _size.x * 0.5f, Min(_a.y, _b.y) - _size.y * 0.5f },
        { Max(_a.x, _b.x) + _size.x * 0.5f, Max(_a.y, _b.y) + _size.y * 0.5f });
}
}   // namespace

void ProjectileSystem::Clear()
{
    m_bullets.clear();
    m_bombs.clear();
    m_blasts.clear();
    m_fx.clear();
    m_shots.clear();
    m_shotFx.clear();
}

void ProjectileSystem::SpawnBullet(
    const BulletSpawn& _spawn,
    CombatSystem&      _combat)
{
    Bullet bullet;
    bullet.position     = _spawn.position;
    bullet.renderPrev   = _spawn.position;
    bullet.hitFrom      = _spawn.sweepFrom;
    bullet.velocity     = { _spawn.direction.x * _spawn.speed, _spawn.direction.y * _spawn.speed };
    bullet.accel        = _spawn.accel;
    bullet.maxSpeed     = _spawn.maxSpeed;
    bullet.size         = _spawn.size;
    bullet.range        = _spawn.range;
    bullet.damage       = _spawn.damage;
    bullet.team         = _spawn.team;
    bullet.kind         = _spawn.kind;
    bullet.owner        = _spawn.owner;
    bullet.bSharedHitId = _spawn.hitId != kNoHitId;
    bullet.hitId        = bullet.bSharedHitId ? _spawn.hitId : _combat.NewHitId();
    bullet.sizeStart    = _spawn.size;
    bullet.sizeEnd      = _spawn.sizeEnd;
    bullet.scaleStart   = _spawn.scaleStart;
    bullet.scaleEnd     = _spawn.scaleEnd;
    bullet.bPierce      = _spawn.bPierce;
    bullet.tag          = _spawn.tag;
    bullet.blastSize    = _spawn.blastSize;
    bullet.blastDamage  = _spawn.blastDamage;
    bullet.blastTicks   = _spawn.blastTicks;
    bullet.bTrail       = _spawn.bTrail;
    bullet.z            = _spawn.z;
    m_bullets.push_back(bullet);
}

void ProjectileSystem::SpawnBomb(
    const BombSpawn& _spawn)
{
    Bomb bomb;
    bomb.position   = _spawn.position;
    bomb.renderPrev = _spawn.position;
    bomb.velocity   = _spawn.velocity;
    bomb.params     = _spawn;
    m_bombs.push_back(bomb);
}

void ProjectileSystem::SpawnShot(
    const ShotSpawn& _spawn,
    CombatSystem&    _combat)
{
    Shot shot;
    shot.position   = _spawn.position;
    shot.renderPrev = _spawn.position;
    shot.hitFrom    = _spawn.position;
    shot.velocity   = _spawn.velocity;
    shot.params     = _spawn;
    shot.hitId      = _combat.NewHitId();
    m_shots.push_back(shot);
}

void ProjectileSystem::ExplodeBomb_(
    Bomb&         _bomb,
    const Vec2    _at,
    CombatSystem& _combat)
{
    _bomb.bAlive = false;

    Blast blast;
    blast.box       = Aabb::FromCenterSize({ _at.x, _at.y - _bomb.params.blastUp }, _bomb.params.blastSize);
    blast.ticksLeft = _bomb.params.blastTicks;
    blast.damage    = _bomb.params.damage;
    blast.team      = _bomb.params.team;
    blast.owner     = _bomb.params.owner;
    blast.hitId     = _combat.NewHitId();
    m_blasts.push_back(blast);

    m_fx.push_back({ eFxKind::BombExplosion, _at, ZDelta(_bomb.params.z, kPlayerBombDepth) });
}

void ProjectileSystem::SpawnShotBlast_(
    const ShotSpawn& _params,
    const Vec2       _at,
    CombatSystem&    _combat)
{
    if (_params.blastSize.x <= 0.f || _params.blastSize.y <= 0.f || _params.blastTicks <= 0)
    {
        return;
    }

    Blast blast;
    blast.box       = Aabb::FromCenterSize({ _at.x, _at.y - _params.blastUp }, _params.blastSize);
    blast.ticksLeft = _params.blastTicks;
    blast.damage    = _params.blastDamage;
    blast.team      = _params.team;
    blast.owner     = _params.owner;
    blast.hitId     = _combat.NewHitId();
    m_blasts.push_back(blast);
}

void ProjectileSystem::SpawnBulletBlast_(
    const Bullet& _bullet,
    CombatSystem& _combat)
{
    if (_bullet.blastSize.x <= 0.f || _bullet.blastSize.y <= 0.f)
    {
        return;
    }

    Blast blast;
    blast.box       = Aabb::FromCenterSize(_bullet.position, _bullet.blastSize);
    blast.ticksLeft = _bullet.blastTicks;
    blast.damage    = _bullet.blastDamage;
    blast.team      = _bullet.team;
    blast.owner     = _bullet.owner;
    blast.hitId     = _combat.NewHitId();
    m_blasts.push_back(blast);

    m_fx.push_back({ eFxKind::RocketExplosion, _bullet.position, ZDelta(_bullet.z, kPlayerBulletDepth) });
}

void ProjectileSystem::Step(
    const GroundMap*           _pGround,
    CombatSystem&              _combat,
    const float                _dt,
    const Span<const Platform> _platforms)
{
    for (Bullet& bullet: m_bullets)
    {
        bullet.renderPrev = bullet.position;
        if (bullet.accel != 0.f)
        {
            const float speed = std::sqrt(bullet.velocity.x * bullet.velocity.x + bullet.velocity.y * bullet.velocity.y);
            if (speed > 0.f)
            {
                float next = speed + bullet.accel * _dt;
                if (bullet.maxSpeed > 0.f)
                {
                    next = Min(next, bullet.maxSpeed);
                }
                const float k = next / speed;
                bullet.velocity.x *= k;
                bullet.velocity.y *= k;
            }
        }
        bullet.position.x += bullet.velocity.x * _dt;
        bullet.position.y += bullet.velocity.y * _dt;
        bullet.travelled += std::sqrt(bullet.velocity.x * bullet.velocity.x + bullet.velocity.y * bullet.velocity.y) * _dt;
        ++bullet.ageTicks;

        if (bullet.travelled > bullet.range)
        {
            bullet.bAlive = false;
            SpawnBulletBlast_(bullet, _combat);
        }
        else if (_pGround)
        {
            Vec2 lastFree;
            if (SegmentHitsGround_(*_pGround, bullet.renderPrev, bullet.position, lastFree))
            {
                bullet.bAlive = false;
                if (bullet.blastSize.x > 0.f)
                {
                    SpawnBulletBlast_(bullet, _combat);
                }
                else
                {
                    m_fx.push_back({ eFxKind::BulletImpact, bullet.position, ZDelta(bullet.z, kPlayerBulletDepth) });
                }
            }
        }

        if (!bullet.bAlive)
        {
            if (!bullet.bSharedHitId)
            {
                _combat.ReleaseHitId(bullet.hitId);
            }
            continue;
        }

        if (bullet.bTrail && bullet.ageTicks % 9 == 1)
        {
            m_fx.push_back({ eFxKind::RocketSmoke, bullet.position, ZDelta(bullet.z, kPlayerBulletDepth) });
        }

        if (bullet.sizeEnd.x > 0.f && bullet.range > 0.f)
        {
            const float t = Clamp(bullet.travelled / bullet.range, 0.f, 1.f);
            bullet.size   = { Lerp(bullet.sizeStart.x, bullet.sizeEnd.x, t), Lerp(bullet.sizeStart.y, bullet.sizeEnd.y, t) };
        }

        Hitbox hitbox;
        hitbox.box     = SweptBox_(bullet.hitFrom, bullet.position, bullet.size);
        hitbox.team    = bullet.team;
        hitbox.kind    = bullet.kind;
        hitbox.damage  = bullet.damage;
        hitbox.bPierce = bullet.bPierce;
        hitbox.owner   = bullet.owner;
        hitbox.id      = bullet.hitId;
        _combat.AddHitbox(hitbox);
        bullet.hitFrom = bullet.position;
    }
    std::erase_if(m_bullets, [](const Bullet& _b) { return !_b.bAlive; });

    for (Shot& shot: m_shots)
    {
        const ShotSpawn& params = shot.params;
        shot.renderPrev         = shot.position;
        ++shot.ageTicks;

        if (shot.ageTicks > params.startDelayTicks)
        {
            shot.velocity.x += params.accel.x * _dt;
            shot.velocity.y += params.accel.y * _dt;
            if (params.maxSpeed > 0.f)
            {
                const float speed = std::sqrt(shot.velocity.x * shot.velocity.x + shot.velocity.y * shot.velocity.y);
                if (speed > params.maxSpeed)
                {
                    const float scale = params.maxSpeed / speed;
                    shot.velocity.x *= scale;
                    shot.velocity.y *= scale;
                }
            }
            shot.velocity.y = Min(shot.velocity.y + params.gravity * _dt, params.maxFallSpeed);
            shot.position.x += shot.velocity.x * _dt;
            shot.position.y += shot.velocity.y * _dt;
        }

        const bool bRoll = params.rollSpeed > 0.f;
        if (bRoll && shot.bLanded)
        {
            shot.velocity.x = params.rollDirX * params.rollSpeed;
        }
        const bool bExpired = (bRoll && shot.bLanded && params.rollTicks > 0) ? (shot.ageTicks - shot.landedAge > params.rollTicks) : (shot.ageTicks > params.lifetimeTicks);

        if (bExpired)
        {
            shot.bAlive = false;
            if (params.bExpireFx)
            {
                m_shotFx.push_back({ shot.position, params.visual, ZDelta(params.z, kEnemyShotDepth) });
                SpawnShotBlast_(params, shot.position, _combat);
            }
        }
        else if ((_pGround || !_platforms.empty()) && (params.bExplodeOnSolid || bRoll) && (shot.position.x != shot.renderPrev.x || shot.position.y != shot.renderPrev.y))
        {
            // 접촉은 스프라이트 바닥(중심 + groundProbeY) 기준으로 판정한다: 중심으로 판정하면 반쯤 지면에 묻힌 뒤 터진다
            const float probe      = params.groundProbeY;
            const Vec2  from       = { shot.renderPrev.x, shot.renderPrev.y + probe };
            const Vec2  to         = { shot.position.x, shot.position.y + probe };
            Vec2        lastFree   = from;
            Vec2        topHit     = to;
            const bool  bGroundHit = _pGround && SegmentHitsGround_(*_pGround, from, to, lastFree);
            const bool  bPlatHit   = !bGroundHit && SegmentHitsPlatform_(_platforms, from, to, topHit);
            if (bGroundHit || bPlatHit)
            {
                if (bRoll)
                {
                    Vec2 standing = shot.position;
                    if (bGroundHit)
                    {
                        for (int guard = 0; guard < 64 && _pGround->IsSolidAt({ standing.x, standing.y + probe }); ++guard)
                        {
                            standing.y -= 1.f;
                        }
                    }
                    else
                    {
                        standing.y = topHit.y - probe;   // 발판 윗면에 얹는다
                    }
                    shot.position   = standing;
                    shot.velocity.y = 0.f;
                    if (!shot.bLanded)
                    {
                        shot.bLanded    = true;
                        shot.landedAge  = shot.ageTicks;
                        shot.velocity.x = params.rollDirX * params.rollSpeed;
                    }
                }
                else
                {
                    shot.bAlive   = false;
                    const Vec2 at = bGroundHit ? lastFree : topHit;
                    m_shotFx.push_back({
                        { at.x, at.y - probe },
                        params.visual,
                        ZDelta(params.z, kEnemyShotDepth)
                    });
                    SpawnShotBlast_(params, { at.x, at.y - probe }, _combat);
                }
            }
        }

        if (!shot.bAlive)
        {
            _combat.ReleaseHitId(shot.hitId);
            continue;
        }

        Hitbox hitbox;
        hitbox.box     = SweptBox_(shot.hitFrom, shot.position, params.size);
        hitbox.team    = params.team;
        hitbox.kind    = params.kind;
        hitbox.damage  = params.damage;
        hitbox.bPierce = false;
        hitbox.owner   = params.owner;
        hitbox.id      = shot.hitId;
        _combat.AddHitbox(hitbox);
        shot.hitFrom = shot.position;
    }
    std::erase_if(m_shots, [](const Shot& _s) { return !_s.bAlive; });

    for (Bomb& bomb: m_bombs)
    {
        bomb.renderPrev = bomb.position;
        ++bomb.ageTicks;
        bomb.velocity.y += bomb.params.gravity * _dt;

        if (bomb.params.bBounce)
        {
            constexpr float kBounceDamping  = 0.4f;
            constexpr float kGroundFriction = 0.82f;
            Vec2            lastFree;

            const Vec2 nextX = { bomb.position.x + bomb.velocity.x * _dt, bomb.position.y };
            if (_pGround && SegmentHitsGround_(*_pGround, bomb.position, nextX, lastFree))
            {
                bomb.velocity.x = -bomb.velocity.x * kBounceDamping;
            }
            else
            {
                bomb.position = nextX;
            }

            const Vec2 nextY = { bomb.position.x, bomb.position.y + bomb.velocity.y * _dt };
            if (_pGround && SegmentHitsGround_(*_pGround, bomb.position, nextY, lastFree))
            {
                bomb.position = lastFree;
                if (bomb.velocity.y > 0.f)
                {
                    bomb.velocity.y = -bomb.velocity.y * kBounceDamping;
                    bomb.velocity.x *= kGroundFriction;
                    if (bomb.velocity.y > -40.f)
                    {
                        bomb.velocity.y = 0.f;
                    }
                }
                else
                {
                    bomb.velocity.y = 0.f;
                }
            }
            else
            {
                bomb.position = nextY;
            }

            const Aabb contactBounce = Aabb::FromCenterSize(bomb.position, bomb.params.hitSize);
            if (bomb.ageTicks >= bomb.params.fuseTicks || _combat.HasTargetIn(bomb.params.team, eHitKind::Blast, contactBounce, bomb.params.owner))
            {
                ExplodeBomb_(bomb, bomb.position, _combat);
            }
            continue;
        }

        const Vec2 next = { bomb.position.x + bomb.velocity.x * _dt, bomb.position.y + bomb.velocity.y * _dt };

        Vec2 lastFree;
        if (_pGround && SegmentHitsGround_(*_pGround, bomb.position, next, lastFree))
        {
            bomb.position = lastFree;
            ExplodeBomb_(bomb, lastFree, _combat);
            continue;
        }

        bomb.position = next;

        const Aabb contact = Aabb::FromCenterSize(bomb.position, bomb.params.hitSize);
        if (_combat.HasTargetIn(bomb.params.team, eHitKind::Blast, contact, bomb.params.owner))
        {
            ExplodeBomb_(bomb, bomb.position, _combat);
        }
    }
    std::erase_if(m_bombs, [](const Bomb& _b) { return !_b.bAlive; });

    for (const Blast& blast: m_blasts)
    {
        if (blast.ticksLeft <= 0)
        {
            _combat.ReleaseHitId(blast.hitId);
        }
    }
    std::erase_if(m_blasts, [](const Blast& _b) { return _b.ticksLeft <= 0; });

    for (Blast& blast: m_blasts)
    {
        Hitbox hitbox;
        hitbox.box     = blast.box;
        hitbox.team    = blast.team;
        hitbox.kind    = eHitKind::Blast;
        hitbox.damage  = blast.damage;
        hitbox.bPierce = true;
        hitbox.owner   = blast.owner;
        hitbox.id      = blast.hitId;
        _combat.AddHitbox(hitbox);
        --blast.ticksLeft;
    }
}

void ProjectileSystem::OnHitEvents(
    const Span<const HitEvent> _events,
    CombatSystem&              _combat)
{
    for (const HitEvent& event: _events)
    {
        if (event.kind == eHitKind::Blast || event.hitId == kNoHitId)
        {
            continue;
        }
        for (Shot& shot: m_shots)
        {
            if (shot.hitId == event.hitId && shot.bAlive)
            {
                shot.bAlive = false;
                m_shotFx.push_back({ shot.position, shot.params.visual, ZDelta(shot.params.z, kEnemyShotDepth) });
                SpawnShotBlast_(shot.params, shot.position, _combat);
                _combat.ReleaseHitId(shot.hitId);
                break;
            }
        }
        for (Bullet& bullet: m_bullets)
        {
            if (bullet.hitId == event.hitId && !bullet.bPierce && bullet.bAlive)
            {
                bullet.bAlive = false;
                if (bullet.blastSize.x > 0.f)
                {
                    SpawnBulletBlast_(bullet, _combat);
                }
                else
                {
                    m_fx.push_back({ eFxKind::BulletImpact, bullet.position, ZDelta(bullet.z, kPlayerBulletDepth) });
                }
                _combat.ReleaseHitId(bullet.hitId);
                break;
            }
        }
    }
    std::erase_if(m_bullets, [](const Bullet& _b) { return !_b.bAlive; });
    std::erase_if(m_shots, [](const Shot& _s) { return !_s.bAlive; });
}

int ProjectileSystem::CountBullets(
    const uint32_t _owner,
    const uint8_t  _tag) const
{
    int count = 0;
    for (const Bullet& bullet: m_bullets)
    {
        if (bullet.bAlive && bullet.owner == _owner && bullet.tag == _tag)
        {
            ++count;
        }
    }
    return count;
}

int ProjectileSystem::CountBombs(
    const uint32_t _owner) const
{
    int count = 0;
    for (const Bomb& bomb: m_bombs)
    {
        if (bomb.bAlive && bomb.params.owner == _owner)
        {
            ++count;
        }
    }
    return count;
}

int ProjectileSystem::CountShots(
    const uint32_t _owner) const
{
    int count = 0;
    for (const Shot& shot: m_shots)
    {
        if (shot.bAlive && shot.params.owner == _owner)
        {
            ++count;
        }
    }
    return count;
}

Vector<ShotFx> ProjectileSystem::TakeShotFx()
{
    Vector<ShotFx> out;
    out.swap(m_shotFx);
    return out;
}

Vector<FxRequest> ProjectileSystem::TakeFx()
{
    Vector<FxRequest> out;
    out.swap(m_fx);
    return out;
}

Span<const Bullet> ProjectileSystem::GetBullets() const
{
    return m_bullets;
}

Span<const Bomb> ProjectileSystem::GetBombs() const
{
    return m_bombs;
}

Span<const Blast> ProjectileSystem::GetBlasts() const
{
    return m_blasts;
}

Span<const Shot> ProjectileSystem::GetShots() const
{
    return m_shots;
}

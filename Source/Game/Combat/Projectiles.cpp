#include "Game/Combat/Projectiles.h"

#include <algorithm>
#include <cmath>

#include <JugX/Math.h>

#include "World/Geometry.h"

namespace slug
{

namespace
{
    // 선분 위를 _stepPx 간격으로 훑어 첫 고체 지점을 찾는다. 찾으면 직전의 비고체 지점을 _outLastFree 에 돌려준다.
    [[nodiscard]] bool SegmentHitsGround_(
        const GroundMap&   _ground,
        const jug::VECTOR2 _from,
        const jug::VECTOR2 _to,
        jug::VECTOR2&      _outLastFree)
    {
        const float dx   = _to.x - _from.x;
        const float dy   = _to.y - _from.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        const int   n    = jug::Max(1, CeilToInt(dist / 2.f));

        jug::VECTOR2 lastFree = _from;
        for (int i = 1; i <= n; ++i)
        {
            const float        t = static_cast<float>(i) / static_cast<float>(n);
            const jug::VECTOR2 p = { _from.x + dx * t, _from.y + dy * t };
            if (_ground.IsSolidAt(p))
            {
                _outLastFree = lastFree;
                return true;
            }
            lastFree = p;
        }
        return false;
    }

    [[nodiscard]] Aabb SweptBox_(
        const jug::VECTOR2 _a,
        const jug::VECTOR2 _b,
        const jug::VECTOR2 _size)
    {
        return Aabb::FromMinMax(
            { jug::Min(_a.x, _b.x) - _size.x * 0.5f, jug::Min(_a.y, _b.y) - _size.y * 0.5f },
            { jug::Max(_a.x, _b.x) + _size.x * 0.5f, jug::Max(_a.y, _b.y) + _size.y * 0.5f });
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
    bullet.position   = _spawn.position;
    bullet.renderPrev = _spawn.position;
    bullet.hitFrom    = _spawn.sweepFrom;
    bullet.velocity   = { _spawn.direction.x * _spawn.speed, _spawn.direction.y * _spawn.speed };
    bullet.size       = _spawn.size;
    bullet.range      = _spawn.range;
    bullet.damage     = _spawn.damage;
    bullet.team       = _spawn.team;
    bullet.kind       = _spawn.kind;
    bullet.owner      = _spawn.owner;
    bullet.hitId      = _combat.NewHitId();
    bullet.bPierce    = _spawn.bPierce;
    bullet.tag        = _spawn.tag;
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
    Bomb&              _bomb,
    const jug::VECTOR2 _at,
    CombatSystem&      _combat)
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

    m_fx.push_back({ eFxKind::BombExplosion, _at });
}

void ProjectileSystem::Step(
    const GroundMap* _pGround,
    CombatSystem&    _combat,
    const float      _dt)
{
    // --- 탄 ---
    for (Bullet& bullet: m_bullets)
    {
        bullet.renderPrev = bullet.position;
        bullet.position.x += bullet.velocity.x * _dt;
        bullet.position.y += bullet.velocity.y * _dt;
        bullet.travelled += std::sqrt(bullet.velocity.x * bullet.velocity.x + bullet.velocity.y * bullet.velocity.y) * _dt;

        if (bullet.travelled > bullet.range)
        {
            bullet.bAlive = false;
        }
        else if (_pGround)
        {
            jug::VECTOR2 lastFree;
            if (SegmentHitsGround_(*_pGround, bullet.renderPrev, bullet.position, lastFree))
            {
                bullet.bAlive = false;
                m_fx.push_back({ eFxKind::BulletImpact, bullet.position });
            }
        }

        if (!bullet.bAlive)
        {
            _combat.ReleaseHitId(bullet.hitId);
            continue;
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

    // --- 샷 (적 투사체: 중력/가속/점화 대기) ---
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
            shot.velocity.y = jug::Min(shot.velocity.y + params.gravity * _dt, params.maxFallSpeed);
            shot.position.x += shot.velocity.x * _dt;
            shot.position.y += shot.velocity.y * _dt;
        }

        // 구르는 탄: 착지 후에는 rollTicks 뒤에 소멸한다 (착지 전에는 lifetimeTicks 가 상한)
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
                m_shotFx.push_back({ shot.position, params.visual });
            }
        }
        else if (_pGround && (params.bExplodeOnSolid || bRoll) && (shot.position.x != shot.renderPrev.x || shot.position.y != shot.renderPrev.y))
        {
            jug::VECTOR2 lastFree;
            if (SegmentHitsGround_(*_pGround, shot.renderPrev, shot.position, lastFree))
            {
                if (bRoll)
                {
                    // 착지 (또는 지면을 따라 구르는 중): 이번 틱의 새 x 에서 고체 밖으로 밀어 올려 세우고 수직 속도를 없앤다.
                    // (구간 판정의 마지막 빈 지점을 쓰면 첫 샘플부터 고체인 경우 제자리에 멈춘다)
                    jug::VECTOR2 standing = shot.position;
                    for (int guard = 0; guard < 64 && _pGround->IsSolidAt(standing); ++guard)
                    {
                        standing.y -= 1.f;
                    }
                    shot.position   = standing;
                    shot.velocity.y = 0.f;
                    if (!shot.bLanded)
                    {
                        shot.bLanded   = true;
                        shot.landedAge = shot.ageTicks;
                        shot.velocity.x = params.rollDirX * params.rollSpeed;
                    }
                }
                else
                {
                    shot.bAlive = false;
                    m_shotFx.push_back({ lastFree, params.visual });
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

    // --- 폭탄 ---
    for (Bomb& bomb: m_bombs)
    {
        bomb.renderPrev = bomb.position;
        bomb.velocity.y += bomb.params.gravity * _dt;
        const jug::VECTOR2 next = { bomb.position.x + bomb.velocity.x * _dt, bomb.position.y + bomb.velocity.y * _dt };

        jug::VECTOR2 lastFree;
        if (_pGround && SegmentHitsGround_(*_pGround, bomb.position, next, lastFree))
        {
            bomb.position = lastFree;
            ExplodeBomb_(bomb, lastFree, _combat);
            continue;
        }

        bomb.position = next;

        // 표적과 닿으면 공중에서도 터진다
        const Aabb contact = Aabb::FromCenterSize(bomb.position, bomb.params.hitSize);
        if (_combat.HasTargetIn(bomb.params.team, eHitKind::Blast, contact, bomb.params.owner))
        {
            ExplodeBomb_(bomb, bomb.position, _combat);
        }
    }
    std::erase_if(m_bombs, [](const Bomb& _b) { return !_b.bAlive; });

    // --- 폭발 (폭탄이 이번 틱에 터졌다면 같은 틱에 판정) ---
    // 끝난 폭발의 HitId 는 판정이 해소된 다음 틱에 푼다 (이번 틱 Resolve 가 기록을 다시 만들지 않도록).
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
    const jug::Span<const HitEvent> _events,
    CombatSystem&                   _combat)
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
                m_shotFx.push_back({ shot.position, shot.params.visual });
                _combat.ReleaseHitId(shot.hitId);
                break;
            }
        }
        for (Bullet& bullet: m_bullets)
        {
            if (bullet.hitId == event.hitId && !bullet.bPierce && bullet.bAlive)
            {
                bullet.bAlive = false;
                m_fx.push_back({ eFxKind::BulletImpact, bullet.position });
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

void ProjectileSystem::CullShots(
    const Aabb&   _keep,
    CombatSystem& _combat)
{
    for (Shot& shot: m_shots)
    {
        if (shot.bAlive && !_keep.Contains(shot.position))
        {
            shot.bAlive = false;
            _combat.ReleaseHitId(shot.hitId);
        }
    }
    std::erase_if(m_shots, [](const Shot& _s) { return !_s.bAlive; });
}

jug::Vector<ShotFx> ProjectileSystem::TakeShotFx()
{
    jug::Vector<ShotFx> out;
    out.swap(m_shotFx);
    return out;
}

jug::Vector<FxRequest> ProjectileSystem::TakeFx()
{
    jug::Vector<FxRequest> out;
    out.swap(m_fx);
    return out;
}

}   // namespace slug

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

jug::Vector<FxRequest> ProjectileSystem::TakeFx()
{
    jug::Vector<FxRequest> out;
    out.swap(m_fx);
    return out;
}

}   // namespace slug

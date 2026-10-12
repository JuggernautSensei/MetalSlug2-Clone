#include "Game/Player/SlugSystem.h"

#include "Base/Math.h"
#include "Base/Types.h"

void SlugSystem::Reset(
    const SlugTuning& _tuning)
{
    m_tuning      = _tuning;
    m_riddenIndex = -1;
    m_slugs.clear();
}

int SlugSystem::Spawn(
    const Vec2 _feet,
    const int  _facing,
    const int  _hp)
{
    if (m_slugs.size() >= kMaxSlugs)
    {
        return -1;
    }
    const int index = static_cast<int>(m_slugs.size());
    SlugVehicle slug;
    slug.Init(m_tuning, _feet, _facing, kSlugOwnerBase + static_cast<uint32_t>(index), _hp);
    m_slugs.push_back(std::move(slug));
    return index;
}

void SlugSystem::Tick(
    const SlugContext&  _ctx,
    const InputHistory& _input,
    PlayerController&   _player)
{
    SlugContext ctx = _ctx;
    ctx.pTuning     = &m_tuning;

    // 탑승 판정: 걸어가거나 점프해서 차량 상자에 닿으면 탄다
    if (m_riddenIndex < 0 && _player.IsAlive() && !_player.IsRiding() && _player.GetState() != ePlayerState::Victory)
    {
        const Aabb playerBox = _player.GetBodyBox();
        for (size_t i = 0; i < m_slugs.size(); ++i)
        {
            SlugVehicle& slug = m_slugs[i];
            if (slug.CanBoard() && playerBox.Overlaps(slug.GetBoardBox()))
            {
                slug.BeginBoarding();
                _player.BeginRide();
                m_riddenIndex = static_cast<int>(i);
                break;
            }
        }
    }

    for (size_t i = 0; i < m_slugs.size(); ++i)
    {
        SlugVehicle& slug = m_slugs[i];
        slug.Tick(ctx, _input);

        const SlugEjectRequest request = slug.TakeEjectRequest();
        if (static_cast<int>(i) == m_riddenIndex)
        {
            if (request.bValid)
            {
                _player.EndRide(request.feet, request.velocity, request.facing, request.invincibleTicks);
                m_riddenIndex = -1;
            }
            else if (slug.GetMode() == eSlugMode::Gone)
            {
                _player.EndRide(slug.GetFeet(), { 0.f, -200.f }, slug.GetFacing(), 0);
                m_riddenIndex = -1;
            }
            else
            {
                _player.SetRideFeet(slug.GetFeet(), slug.GetFacing());
            }
        }
    }
}

void SlugSystem::OnHitEvents(
    const Span<const HitEvent> _events,
    const Span<const Hitbox>   _hitboxes)
{
    for (SlugVehicle& slug: m_slugs)
    {
        slug.OnHitEvents(_events, _hitboxes);
    }
}

bool SlugSystem::OnBombSupply(
    const int _amount)
{
    SlugVehicle* pSlug = GetRiddenOrNull();
    if (pSlug == nullptr || pSlug->GetMode() != eSlugMode::Driven)
    {
        return false;
    }
    pSlug->AddCannonAmmo(_amount > 0 ? m_tuning.cannon.pickupAmmo : 0);
    return true;
}

int SlugSystem::GetCount() const
{
    return static_cast<int>(m_slugs.size());
}

SlugVehicle& SlugSystem::Get(
    const int _index)
{
    return m_slugs[static_cast<size_t>(_index)];
}

const SlugVehicle& SlugSystem::Get(
    const int _index) const
{
    return m_slugs[static_cast<size_t>(_index)];
}

Span<const SlugVehicle> SlugSystem::GetAll() const
{
    return m_slugs;
}

int SlugSystem::GetRiddenIndex() const
{
    return m_riddenIndex;
}

const SlugVehicle* SlugSystem::GetRiddenOrNull() const
{
    return m_riddenIndex >= 0 ? &m_slugs[static_cast<size_t>(m_riddenIndex)] : nullptr;
}

SlugVehicle* SlugSystem::GetRiddenOrNull()
{
    return m_riddenIndex >= 0 ? &m_slugs[static_cast<size_t>(m_riddenIndex)] : nullptr;
}

bool SlugSystem::IsRiding() const
{
    return m_riddenIndex >= 0;
}

Vector<Wall> SlugSystem::MakeEnemyWalls() const
{
    Vector<Wall> walls;
    for (const SlugVehicle& slug: m_slugs)
    {
        if (slug.IsAlive())
        {
            walls.push_back({ slug.GetBodyBox() });
        }
    }
    return walls;
}

const SlugTuning& SlugSystem::GetTuning() const
{
    return m_tuning;
}

uint64_t SlugSystem::HashState() const
{
    uint64_t hash = 0;
    for (const SlugVehicle& slug: m_slugs)
    {
        hash = hash * 1'099'511'628'211ull + slug.HashState();
    }
    return hash;
}

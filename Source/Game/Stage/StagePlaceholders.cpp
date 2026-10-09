#include "Game/Stage/StagePlaceholders.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include <algorithm>
#include <cmath>
#include <string_view>

#include "Base/Log.h"

#include "Audio/AudioSink.h"
#include "Core/Tuning.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Ui/UiSystem.h"

namespace slug
{

namespace
{
    constexpr float kTickRate = 60.f;

    [[nodiscard]] int SecondsToTicks_(
        const float _seconds)
    {
        return _seconds <= 0.f ? 0 : static_cast<int>(std::lround(static_cast<double>(_seconds) * static_cast<double>(kTickRate)));
    }

    struct DefaultSpec
    {
        int          hp   = 1;
        Vec2 size = { 40.f, 40.f };
    };

    [[nodiscard]] DefaultSpec GetDefaultSpec_(
        const std::string_view _def,
        const eEntityKind      _kind)
    {
        if (_def == "jeep")          return { 20, { 75.f, 60.f } };
        if (_def == "truck")         return { 15, { 80.f, 60.f } };
        if (_def == "missile_tank")  return { 30, { 100.f, 100.f } };
        if (_def == "btank" || _def == "sarubia") return { 40, { 100.f, 100.f } };
        if (_def == "aparch")        return { 15, { 60.f, 40.f } };
        if (_def == "mb_truck")      return { 60, { 180.f, 260.f } };
        if (_def == "battleship")    return { 100, { 200.f, 200.f } };
        if (_def == "mob_boat")      return { 30, { 150.f, 150.f } };
        if (_def == "big_house")     return { 40, { 310.f, 120.f } };
        if (_def == "ground_house")  return { 20, { 200.f, 150.f } };
        if (_def == "tower")         return { 10, { 90.f, 210.f } };
        if (_def.starts_with("shop_"))   return { 15, { 100.f, 100.f } };
        if (_def.starts_with("bridge_") || _def.starts_with("house_") || _def == "ground_mini_house")
        {
            return { 10, { 150.f, 100.f } };
        }

        switch (_kind)
        {
        case eEntityKind::Enemy:    return { 3, { 40.f, 75.f } };
        case eEntityKind::Npc:      return { 1, { 40.f, 80.f } };
        case eEntityKind::Prop:     return { 10, { 60.f, 60.f } };
        case eEntityKind::Decor:    return { 1, { 40.f, 40.f } };
        case eEntityKind::Item:     return { 1, { 24.f, 24.f } };
        case eEntityKind::Collider: return { 1, { 16.f, 16.f } };
        }
        return {};
    }

    [[nodiscard]] eStageLayer GetDefaultLayer_(
        const eEntityKind _kind)
    {
        switch (_kind)
        {
        case eEntityKind::Enemy:    return eStageLayer::Monster;
        case eEntityKind::Npc:      return eStageLayer::Npc;
        case eEntityKind::Prop:     return eStageLayer::Monster;
        case eEntityKind::Decor:    return eStageLayer::Background;
        case eEntityKind::Item:     return eStageLayer::Item;
        case eEntityKind::Collider: return eStageLayer::Collider;
        }
        return eStageLayer::Monster;
    }

    [[nodiscard]] bool IsHittableKind_(
        const eEntityKind _kind)
    {
        return _kind == eEntityKind::Enemy || _kind == eEntityKind::Npc || _kind == eEntityKind::Prop;
    }
}

void StagePlaceholders::AddGroundMap(
    const slug::StringView _name,
    GroundMap             _map)
{
    m_grounds.push_back({ slug::String { _name }, std::move(_map) });
    if (m_activeGround < 0)
    {
        m_activeGround = 0;
    }
}

void StagePlaceholders::BindPlayer(
    PlayerController* _pPlayer,
    const Tuning*     _pTuning)
{
    m_pPlayer = _pPlayer;
    m_pTuning = _pTuning;
}

void StagePlaceholders::BindUi(
    UiSystem* _pUi)
{
    m_pUi = _pUi;
}

void StagePlaceholders::Reset()
{
    m_entities.clear();
    m_spawnCount       = 0;
    m_lastOwner        = kPlaceholderOwnerBase;
    m_invincibleTicks  = 0;
    m_lastSectionId.clear();
    m_lastSectionIndex = -1;
    m_bEndRequested    = false;
    m_endStateName.clear();
}

const GroundMap* StagePlaceholders::GetActiveGround() const
{
    if (m_activeGround < 0 || m_activeGround >= static_cast<int>(m_grounds.size()))
    {
        return nullptr;
    }
    return &m_grounds[static_cast<size_t>(m_activeGround)].map;
}

GroundMap* StagePlaceholders::GetActiveGroundMut()
{
    if (m_activeGround < 0 || m_activeGround >= static_cast<int>(m_grounds.size()))
    {
        return nullptr;
    }
    return &m_grounds[static_cast<size_t>(m_activeGround)].map;
}

const slug::String& StagePlaceholders::GetActiveGroundName() const
{
    static const slug::String kNone = "";
    if (m_activeGround < 0 || m_activeGround >= static_cast<int>(m_grounds.size()))
    {
        return kNone;
    }
    return m_grounds[static_cast<size_t>(m_activeGround)].name;
}

void StagePlaceholders::RegisterHurtboxes(
    CombatSystem& _combat) const
{
    for (const PlaceholderEntity& entity: m_entities)
    {
        if (!entity.bHittable || !entity.IsAlive())
        {
            continue;
        }
        Hurtbox hurtbox;
        hurtbox.box   = entity.GetBox();
        hurtbox.team  = eTeam::Enemy;
        hurtbox.owner = entity.id;
        _combat.AddHurtbox(hurtbox);
    }
}

int StagePlaceholders::ApplyEvents(
    const slug::Span<const HitEvent> _events)
{
    int kills = 0;
    for (const HitEvent& event: _events)
    {
        PlaceholderEntity* pEntity = FindAlive_(event.target);
        if (!pEntity || !pEntity->bHittable)
        {
            continue;
        }

        if (pEntity->health.ApplyDamage(event.damage) > 0)
        {
            pEntity->flashTicks = 5;
        }
        if (pEntity->health.IsDead())
        {
            pEntity->deadTicks = 0;
            ++kills;
        }
    }
    return kills;
}

void StagePlaceholders::Tick()
{
    if (m_invincibleTicks > 0)
    {
        --m_invincibleTicks;
    }

    for (PlaceholderEntity& entity: m_entities)
    {
        if (entity.flashTicks > 0)
        {
            --entity.flashTicks;
        }
        if (entity.deadTicks >= 0)
        {
            ++entity.deadTicks;
        }
    }

    std::erase_if(m_entities, [](const PlaceholderEntity& _entity) { return _entity.deadTicks >= kPlaceholderDeathTicks; });
}

uint64_t StagePlaceholders::HashState() const
{
    uint64_t hash = static_cast<uint64_t>(m_entities.size());
    for (const PlaceholderEntity& entity: m_entities)
    {
        hash = hash * 1099511628211ull ^ entity.id;
        hash = hash * 1099511628211ull ^ static_cast<uint64_t>(static_cast<uint32_t>(entity.health.hp));
        hash = hash * 1099511628211ull ^ static_cast<uint64_t>(static_cast<uint32_t>(entity.deadTicks + 1));
    }
    return hash;
}

PlaceholderEntity* StagePlaceholders::FindAlive_(
    const SpawnId _id)
{
    for (PlaceholderEntity& entity: m_entities)
    {
        if (entity.id == _id && entity.IsAlive())
        {
            return &entity;
        }
    }
    return nullptr;
}

const PlaceholderEntity* StagePlaceholders::Find_(
    const SpawnId _id) const
{
    for (const PlaceholderEntity& entity: m_entities)
    {
        if (entity.id == _id)
        {
            return &entity;
        }
    }
    return nullptr;
}

SpawnId StagePlaceholders::SpawnEntity(
    const SpawnRequest& _request)
{
    const DefaultSpec spec = GetDefaultSpec_(_request.def, _request.kind);

    PlaceholderEntity entity;
    entity.id        = ++m_lastOwner;
    entity.def       = _request.def;
    entity.kind      = _request.kind;
    entity.layer     = _request.bHasLayer ? _request.layer : GetDefaultLayer_(_request.kind);
    entity.size      = (_request.kind == eEntityKind::Collider && _request.size.x > 0.f && _request.size.y > 0.f) ? _request.size : spec.size;
    entity.bHittable = IsHittableKind_(_request.kind);
    entity.health    = Health::Make(_request.bHasHp ? _request.hp : spec.hp);

    entity.feet = _request.positionMode == ePositionMode::Feet
        ? _request.position
        : Vec2 { _request.position.x, _request.position.y + entity.size.y * 0.5f };

    m_entities.push_back(entity);
    ++m_spawnCount;
    return entity.id;
}

bool StagePlaceholders::IsDefeated(
    const SpawnId _id) const
{
    const PlaceholderEntity* pEntity = Find_(_id);
    return !pEntity || !pEntity->IsAlive();
}

bool StagePlaceholders::IsDestroyFinished(
    const SpawnId _id) const
{
    const PlaceholderEntity* pEntity = Find_(_id);
    return !pEntity || pEntity->deadTicks >= kPlaceholderDeathTicks;
}

void StagePlaceholders::ClearEntities(
    const EntityFilter& _filter)
{
    std::erase_if(m_entities, [&_filter](const PlaceholderEntity& _entity) { return (_filter.layerMask & LayerBit(_entity.layer)) != 0; });
}

void StagePlaceholders::SendCommand(
    SpawnId,
    slug::StringView)
{
}

void StagePlaceholders::SetGround(
    const slug::StringView _name)
{
    for (size_t i = 0; i < m_grounds.size(); ++i)
    {
        if (m_grounds[i].name == _name)
        {
            m_activeGround = static_cast<int>(i);
            return;
        }
    }
    SLUG_LOG_WARN("StagePlaceholders: 지면 '{}' 이(가) 없습니다", std::string { _name });
}

std::optional<float> StagePlaceholders::GetGroundFindY(
    const float _x,
    const float _fromY) const
{
    const GroundMap* pGround = GetActiveGround();
    if (!pGround)
    {
        return std::nullopt;
    }
    const int x = static_cast<int>(std::floor(_x));
    const int y = static_cast<int>(std::floor(_fromY));
    if (const std::optional<int> row = pGround->FindGroundY(x, y, pGround->GetHeight()))
    {
        return static_cast<float>(*row);
    }
    return std::nullopt;
}

Vec2 StagePlaceholders::PlayerFeet() const
{
    return m_pPlayer ? m_pPlayer->GetFeet() : Vec2 { 0.f, 0.f };
}

bool StagePlaceholders::PlayerAlive() const
{
    return m_pPlayer && m_pPlayer->IsAlive();
}

bool StagePlaceholders::IsGameOver() const
{
    return m_pPlayer && m_pPlayer->IsGameOver();
}

void StagePlaceholders::SetPlayerInvincible(
    const float _seconds)
{
    m_invincibleTicks = SecondsToTicks_(_seconds);
}

void StagePlaceholders::WarpPlayer(
    const Vec2 _feet)
{
    if (!m_pPlayer || !m_pTuning)
    {
        return;
    }
    const int score = m_pPlayer->GetScore();
    m_pPlayer->Reset(*m_pTuning, _feet);
    m_pPlayer->AddScore(score);
}

void StagePlaceholders::SetBackdrop(
    slug::StringView)
{
}

void StagePlaceholders::ShowBanner(
    const slug::String& _text,
    const float        _seconds)
{
    if (m_pUi)
    {
        m_pUi->ShowBanner(_text, _seconds);
    }
}

void StagePlaceholders::FadeTo(
    const eFadeTarget _target,
    const float       _seconds)
{
    if (!m_pUi)
    {
        return;
    }
    const int ticks = SecondsToTicks_(_seconds);
    if (_target == eFadeTarget::Black)
    {
        m_pUi->GetFade().FadeToBlack(ticks);
    }
    else
    {
        m_pUi->GetFade().FadeToClear(ticks);
    }
}

void StagePlaceholders::PlayBgm(
    const slug::StringView _name,
    const bool            _bLoop)
{
    if (m_pCtx)
    {
        m_pCtx->GetAudio().PlayBgm(_name, _bLoop);
    }
}

void StagePlaceholders::EndStage(
    const slug::StringView _nextState)
{
    m_bEndRequested = true;
    m_endStateName  = slug::String { _nextState };
}

void StagePlaceholders::OnSectionEntered(
    const int          _index,
    const slug::StringView _id)
{
    m_lastSectionIndex = _index;
    m_lastSectionId    = slug::String { _id };
}
}

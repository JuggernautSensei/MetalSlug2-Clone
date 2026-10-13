#include "Game/Stage/StageWorldHost.h"

#include <algorithm>
#include <cmath>

#include "Audio/AudioSystem.h"
#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropWorld.h"
#include "Game/Stage/StageMapping.h"
#include "Game/Ui/UiSystem.h"

constexpr float kTickRate = 60.f;

namespace
{
[[nodiscard]] int SecondsToTicks_(
    const float _seconds)
{
    return _seconds <= 0.f ? 0 : static_cast<int>(std::lround(static_cast<double>(_seconds) * static_cast<double>(kTickRate)));
}

[[nodiscard]] VECTOR2 EnemyFeet_(
    const SpawnRequest& _request)
{
    if (_request.positionMode == ePositionMode::LegacyCenter)
    {
        return { _request.position.x, _request.position.y + _request.halfHeight };
    }
    return _request.position;
}
}   // namespace

struct DefaultSpec
{
    int     hp   = 1;
    VECTOR2 size = { 40.f, 40.f };
};

namespace
{
[[nodiscard]] DefaultSpec MakeSpec_(
    const int   _hp,
    const float _width,
    const float _height)
{
    DefaultSpec spec = {};
    spec.hp          = _hp;
    spec.size        = { _width, _height };
    return spec;
}

[[nodiscard]] DefaultSpec GetDefaultSpec_(
    const StringView  _def,
    const eEntityKind _kind)
{
    if (_def == "jeep")
    {
        return MakeSpec_(20, 75.f, 60.f);
    }
    if (_def == "truck")
    {
        return MakeSpec_(15, 80.f, 60.f);
    }
    if (_def == "missile_tank")
    {
        return MakeSpec_(30, 100.f, 100.f);
    }
    if (_def == "btank" || _def == "sarubia")
    {
        return MakeSpec_(40, 100.f, 100.f);
    }
    if (_def == "mob_boat")
    {
        return MakeSpec_(30, 150.f, 150.f);
    }
    if (_def == "big_house")
    {
        return MakeSpec_(40, 310.f, 120.f);
    }
    if (_def == "ground_house")
    {
        return MakeSpec_(20, 200.f, 150.f);
    }
    if (_def == "tower")
    {
        return MakeSpec_(10, 90.f, 210.f);
    }
    if (_def.starts_with("shop_"))
    {
        return MakeSpec_(15, 100.f, 100.f);
    }
    if (_def.starts_with("bridge_") || _def.starts_with("house_") || _def == "ground_mini_house")
    {
        return MakeSpec_(10, 150.f, 100.f);
    }

    switch (_kind)
    {
        case eEntityKind::Enemy: return MakeSpec_(3, 40.f, 75.f);
        case eEntityKind::Npc: return MakeSpec_(1, 40.f, 80.f);
        case eEntityKind::Prop: return MakeSpec_(10, 60.f, 60.f);
        case eEntityKind::Decor: return MakeSpec_(1, 40.f, 40.f);
        case eEntityKind::Item: return MakeSpec_(1, 24.f, 24.f);
        case eEntityKind::Collider: return MakeSpec_(1, 16.f, 16.f);
    }
    return {};
}

[[nodiscard]] eStageLayer GetDefaultLayer_(
    const eEntityKind _kind)
{
    switch (_kind)
    {
        case eEntityKind::Enemy: return eStageLayer::Monster;
        case eEntityKind::Npc: return eStageLayer::Npc;
        case eEntityKind::Prop: return eStageLayer::Monster;
        case eEntityKind::Decor: return eStageLayer::Background;
        case eEntityKind::Item: return eStageLayer::Item;
        case eEntityKind::Collider: return eStageLayer::Collider;
    }
    return eStageLayer::Monster;
}

[[nodiscard]] bool IsHittableKind_(
    const eEntityKind _kind)
{
    return _kind == eEntityKind::Enemy || _kind == eEntityKind::Npc || _kind == eEntityKind::Prop;
}
}   // namespace

Aabb PlaceholderEntity::GetBox() const
{
    return Aabb::FromBottomCenter(feet, size);
}

bool PlaceholderEntity::IsAlive() const
{
    return deadTicks < 0 && health.IsAlive();
}

void StageWorldHost::BindWorld(
    EnemyWorld* const _pEnemies,
    PropWorld* const  _pProps)
{
    m_pEnemies = _pEnemies;
    m_pProps   = _pProps;
}

void StageWorldHost::BindRects(
    const Span<const NamedRect> _rects)
{
    m_rects.clear();
    for (const NamedRect& rect: _rects)
    {
        m_rects.push_back(rect);
    }
}

void StageWorldHost::BindPlayer(
    PlayerController* const _pPlayer,
    const Tuning* const     _pTuning)
{
    m_pPlayer = _pPlayer;
    m_pTuning = _pTuning;
}

void StageWorldHost::BindUi(
    UiSystem* const _pUi)
{
    m_pUi = _pUi;
}

void StageWorldHost::AddGroundMap(
    const StringView _name,
    GroundMap        _map)
{
    m_grounds.push_back({ String { _name }, std::move(_map) });
    if (m_activeGround < 0)
    {
        m_activeGround = 0;
    }
}

void StageWorldHost::Reset()
{
    m_placeholderEntities.clear();
    m_lastOwner = kPlaceholderOwnerBase;
    m_lastSectionId.clear();
    m_lastSectionIndex = -1;
    m_bEndRequested    = false;
    m_endStateName.clear();
    m_routes.clear();
    m_colliders.clear();
    m_nextId = 1;
}

void StageWorldHost::RegisterPlaceholderHurtboxes(
    CombatSystem& _combat) const
{
    for (const PlaceholderEntity& entity: m_placeholderEntities)
    {
        if (!entity.bHittable || !entity.IsAlive())
        {
            continue;
        }
        Hurtbox hurtbox = {};
        hurtbox.box     = entity.GetBox();
        hurtbox.team    = eTeam::Enemy;
        hurtbox.owner   = entity.id;
        _combat.AddHurtbox(hurtbox);
    }
}

int StageWorldHost::ApplyPlaceholderEvents(
    const Span<const HitEvent> _events)
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
            (void)StartHitFlash(pEntity->flashTicks);
        }
        if (pEntity->health.IsDead())
        {
            pEntity->deadTicks = 0;
            ++kills;
        }
    }
    return kills;
}

void StageWorldHost::TickPlaceholders()
{
    for (PlaceholderEntity& entity: m_placeholderEntities)
    {
        TickHitFlash(entity.flashTicks);
        if (entity.deadTicks >= 0)
        {
            ++entity.deadTicks;
        }
    }

    std::erase_if(m_placeholderEntities, [](const PlaceholderEntity& _entity) { return _entity.deadTicks >= kPlaceholderDeathTicks; });
}

const GroundMap* StageWorldHost::GetActiveGround() const
{
    if (m_activeGround < 0 || m_activeGround >= static_cast<int>(m_grounds.size()))
    {
        return nullptr;
    }
    return &m_grounds[static_cast<size_t>(m_activeGround)].map;
}

GroundMap* StageWorldHost::GetActiveGroundMut()
{
    if (m_activeGround < 0 || m_activeGround >= static_cast<int>(m_grounds.size()))
    {
        return nullptr;
    }
    return &m_grounds[static_cast<size_t>(m_activeGround)].map;
}

const String& StageWorldHost::GetActiveGroundName() const
{
    static const String kNone = "";
    if (m_activeGround < 0 || m_activeGround >= static_cast<int>(m_grounds.size()))
    {
        return kNone;
    }
    return m_grounds[static_cast<size_t>(m_activeGround)].name;
}

Span<const PlaceholderEntity> StageWorldHost::GetPlaceholderEntities() const
{
    return m_placeholderEntities;
}

const String& StageWorldHost::GetLastSectionId() const
{
    return m_lastSectionId;
}

int StageWorldHost::GetLastSectionIndex() const
{
    return m_lastSectionIndex;
}

const String& StageWorldHost::GetEndStateName() const
{
    return m_endStateName;
}

bool StageWorldHost::IsEndRequested() const
{
    return m_bEndRequested;
}

Span<const StageCollider> StageWorldHost::GetColliders() const
{
    return m_colliders;
}

const SpawnRoute* StageWorldHost::FindRoute(
    const SpawnId _id) const
{
    for (const SpawnRoute& route: m_routes)
    {
        if (route.id == _id)
        {
            return &route;
        }
    }
    return nullptr;
}

SpawnId StageWorldHost::AllocId_()
{
    return m_nextId++;
}

void StageWorldHost::AddRoute_(
    const SpawnId    _id,
    const eRouteKind _kind,
    const uint32_t   _handle)
{
    SpawnRoute route = {};
    route.id         = _id;
    route.kind       = _kind;
    route.handle     = _handle;
    m_routes.push_back(route);
}

PlaceholderEntity* StageWorldHost::FindAlive_(
    const SpawnId _id)
{
    for (PlaceholderEntity& entity: m_placeholderEntities)
    {
        if (entity.id == _id && entity.IsAlive())
        {
            return &entity;
        }
    }
    return nullptr;
}

const PlaceholderEntity* StageWorldHost::Find_(
    const SpawnId _id) const
{
    for (const PlaceholderEntity& entity: m_placeholderEntities)
    {
        if (entity.id == _id)
        {
            return &entity;
        }
    }
    return nullptr;
}

uint64_t StageWorldHost::HashState() const
{
    uint64_t hash = static_cast<uint64_t>(m_placeholderEntities.size());
    for (const PlaceholderEntity& entity: m_placeholderEntities)
    {
        hash = hash * 1099511628211ull ^ entity.id;
        hash = hash * 1099511628211ull ^ static_cast<uint64_t>(static_cast<uint32_t>(entity.health.hp));
        hash = hash * 1099511628211ull ^ static_cast<uint64_t>(static_cast<uint32_t>(entity.deadTicks + 1));
    }
    for (const SpawnRoute& route: m_routes)
    {
        hash = hash * 1099511628211ull ^ route.id;
        hash = hash * 1099511628211ull ^ static_cast<uint64_t>(route.kind);
        hash = hash * 1099511628211ull ^ route.handle;
    }
    for (const StageCollider& collider: m_colliders)
    {
        hash = hash * 1099511628211ull ^ collider.id;
    }
    return hash;
}

SpawnId StageWorldHost::SpawnPlaceholder_(
    const SpawnRequest& _request)
{
    const DefaultSpec spec = GetDefaultSpec_(_request.def, _request.kind);

    PlaceholderEntity entity = {};
    entity.id                = ++m_lastOwner;
    entity.def               = _request.def;
    entity.kind              = _request.kind;
    entity.layer             = _request.bHasLayer ? _request.layer : GetDefaultLayer_(_request.kind);
    entity.size              = (_request.kind == eEntityKind::Collider && _request.size.x > 0.f && _request.size.y > 0.f) ? _request.size : spec.size;
    entity.bHittable         = IsHittableKind_(_request.kind);
    entity.health            = Health::Make(_request.bHasHp ? _request.hp : spec.hp);

    entity.feet = _request.positionMode == ePositionMode::Feet
                    ? _request.position
                    : VECTOR2 { _request.position.x, _request.position.y + entity.size.y * 0.5f };

    m_placeholderEntities.push_back(entity);
    return entity.id;
}

SpawnId StageWorldHost::SpawnEntity(
    const SpawnRequest& _request)
{
    const SpawnId id = AllocId_();

    if (_request.kind == eEntityKind::Collider)
    {
        StageCollider collider = {};
        collider.id            = id;
        collider.box           = MakeColliderBox(_request);
        collider.layer         = _request.collisionLayer;
        collider.stageLayer    = _request.bHasLayer ? _request.layer : eStageLayer::Collider;
        m_colliders.push_back(collider);
        AddRoute_(id, eRouteKind::Collider, 0);
        return id;
    }

    const ParamValue* pParams = _request.pParams;

    if (m_pEnemies && (_request.kind == eEntityKind::Enemy || _request.kind == eEntityKind::Npc))
    {
        SpawnParams params = {};
        params.facing      = _request.facing != 0 ? _request.facing : -1;
        params.hp          = _request.bHasHp ? _request.hp : -1;
        params.z           = _request.bHasZ ? _request.z : kDefaultZ;
        params.drop        = _request.drop;
        params.userTag     = id;
        if (pParams)
        {
            if (const ParamValue* pVariant = pParams->Find("variant"))
            {
                params.aiVariant = pVariant->AsInt(0);
            }
        }

        const VECTOR2     feet   = EnemyFeet_(_request);
        const EnemyHandle handle = m_pEnemies->Spawn(_request.def, feet, params, nullptr);
        if (!handle.IsNull())
        {
            AddRoute_(id, eRouteKind::Enemy, handle.id);
            return id;
        }
    }

    if (m_pProps && _request.kind == eEntityKind::Prop)
    {
        const PropHandle handle = m_pProps->Spawn(_request.def, _request.position, MakePropSpawnParams(pParams, _request.bHasZ ? _request.z : kDefaultZ));
        if (handle != kNoProp)
        {
            AddRoute_(id, eRouteKind::Prop, handle);
            return id;
        }
    }

    if (m_pProps && _request.kind == eEntityKind::Item)
    {
        const VECTOR2  feet   = _request.positionMode == ePositionMode::Feet ? _request.position : ItemFeetFromCenter(_request.position);
        const uint32_t itemId = m_pProps->SpawnItem(_request.def, feet, false);
        if (itemId != 0)
        {
            AddRoute_(id, eRouteKind::Item, itemId);
            return id;
        }
    }

    if (m_pProps && _request.kind == eEntityKind::Decor && _request.def == "anim_decor" && !_request.anim.empty())
    {
        // 시각 전용 장식: 판정 없이 PropWorld 의 DecorWorld 가 재생/이동한다
        const eStageLayer layer = _request.bHasLayer ? _request.layer : eStageLayer::Background;
        m_pProps->SpawnDecor(MakeDecorSpawn(_request.anim, layer, _request.position, pParams));
    }

    const SpawnId inner = SpawnPlaceholder_(_request);
    AddRoute_(id, eRouteKind::Placeholder, inner);
    return id;
}

bool StageWorldHost::IsDefeated(
    const SpawnId _id) const
{
    const SpawnRoute* pRoute = FindRoute(_id);
    if (!pRoute)
    {
        return true;
    }
    switch (pRoute->kind)
    {
        case eRouteKind::Placeholder:
        {
            const PlaceholderEntity* pEntity = Find_(pRoute->handle);
            return !pEntity || !pEntity->IsAlive();
        }
        case eRouteKind::Enemy: return !m_pEnemies || m_pEnemies->IsDefeated(EnemyHandle { pRoute->handle });
        case eRouteKind::Prop: return !m_pProps || m_pProps->IsDestroyed(pRoute->handle);
        case eRouteKind::Item: return !m_pProps || m_pProps->GetItems().Find(pRoute->handle) == nullptr;
        case eRouteKind::Collider:
            return std::none_of(m_colliders.begin(), m_colliders.end(), [_id](const StageCollider& _c) { return _c.id == _id; });
    }
    return true;
}

bool StageWorldHost::IsDestroyFinished(
    const SpawnId _id) const
{
    const SpawnRoute* pRoute = FindRoute(_id);
    if (!pRoute)
    {
        return true;
    }
    switch (pRoute->kind)
    {
        case eRouteKind::Placeholder:
        {
            const PlaceholderEntity* pEntity = Find_(pRoute->handle);
            return !pEntity || pEntity->deadTicks >= kPlaceholderDeathTicks;
        }
        case eRouteKind::Enemy: return !m_pEnemies || m_pEnemies->IsDestroyFinished(EnemyHandle { pRoute->handle });
        case eRouteKind::Prop: return !m_pProps || m_pProps->IsDestroyFinished(pRoute->handle);
        case eRouteKind::Item:
        case eRouteKind::Collider: return IsDefeated(_id);
    }
    return true;
}

void StageWorldHost::ClearEntities(
    const EntityFilter& _filter)
{
    std::erase_if(m_placeholderEntities, [&_filter](const PlaceholderEntity& _entity) { return (_filter.layerMask & LayerBit(_entity.layer)) != 0; });
    if (m_pProps)
    {
        m_pProps->ClearDecorByLayerMask(_filter.layerMask);
    }

    if (m_pEnemies && (_filter.layerMask & LayerBit(eStageLayer::Monster)) != 0)
    {
        m_pEnemies->ClearMonsters((_filter.layerMask & LayerBit(eStageLayer::Npc)) != 0);
    }

    std::erase_if(m_colliders, [&_filter](const StageCollider& _c) { return (_filter.layerMask & LayerBit(_c.stageLayer)) != 0; });
}

void StageWorldHost::SendCommand(
    const SpawnId    _id,
    const StringView _command)
{
    if (const SpawnRoute* pRoute = FindRoute(_id); pRoute && pRoute->kind == eRouteKind::Enemy && m_pEnemies)
    {
        m_pEnemies->SendCommand(EnemyHandle { pRoute->handle }, _command);
        return;
    }
    static bool bWarnedOnce = false;
    if (!bWarnedOnce)
    {
        bWarnedOnce = true;
        SLUG_LOG_WARN("스테이지 명령 '{}' (대상 {}) 은 아직 구현되지 않았습니다", String { _command }, _id);
    }
}

void StageWorldHost::SetGround(
    const StringView _name)
{
    for (size_t i = 0; i < m_grounds.size(); ++i)
    {
        if (m_grounds[i].name == _name)
        {
            m_activeGround = static_cast<int>(i);
            return;
        }
    }
    SLUG_LOG_WARN("StageWorldHost: 지면 '{}' 이(가) 없습니다", String { _name });
}

std::optional<float> StageWorldHost::GetGroundFindY(
    const float _x,
    const float _fromY) const
{
    const GroundMap* pGround = GetActiveGround();
    if (!pGround)
    {
        return std::nullopt;
    }
    const int x = static_cast<int>(Floor(_x));
    const int y = static_cast<int>(Floor(_fromY));
    if (const std::optional<int> row = pGround->FindGroundY(x, y, pGround->GetHeight()))
    {
        return static_cast<float>(*row);
    }
    return std::nullopt;
}

VECTOR2 StageWorldHost::PlayerFeet() const
{
    return m_pPlayer ? m_pPlayer->GetLogicalFeet() : VECTOR2 { 0.f, 0.f };
}

bool StageWorldHost::IsGameOver() const
{
    return m_pPlayer && m_pPlayer->IsGameOver();
}

void StageWorldHost::SetPlayerInvincible(
    const float _seconds)
{
    if (m_pPlayer)
    {
        m_pPlayer->ExtendInvincibility(SecondsToTicks_(_seconds));
    }
}

void StageWorldHost::WarpPlayer(
    const VECTOR2 _feet)
{
    if (!m_pPlayer || !m_pTuning)
    {
        return;
    }
    const int score = m_pPlayer->GetScore();
    m_pPlayer->Reset(*m_pTuning, _feet);
    m_pPlayer->AddScore(score);
}

void StageWorldHost::SetBackdrop(
    StringView)
{
}

void StageWorldHost::ShowBanner(
    const String& _text,
    const float   _seconds)
{
    if (m_pUi)
    {
        m_pUi->ShowBanner(_text, _seconds);
    }
}

void StageWorldHost::FadeTo(
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

void StageWorldHost::PlayBgm(
    const StringView _name,
    const bool       _bLoop)
{
    AudioSystem::GetSingleton().PlayBgm(_name, _bLoop);
}

void StageWorldHost::ChangeBgm(
    const StringView _name,
    const bool       _bLoop,
    const float      _fadeOutSec)
{
    AudioSystem::GetSingleton().ChangeBgm(_name, _fadeOutSec, _bLoop);
}

void StageWorldHost::PlaySfx(
    const StringView _name)
{
    AudioSystem::GetSingleton().Play(_name);
}

void StageWorldHost::EndStage(
    const StringView _nextState)
{
    m_bEndRequested = true;
    m_endStateName  = String { _nextState };
}

void StageWorldHost::OnSectionEntered(
    const int        _index,
    const StringView _id)
{
    m_lastSectionIndex = _index;
    m_lastSectionId    = String { _id };
}

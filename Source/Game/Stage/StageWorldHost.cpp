#include "Game/Stage/StageWorldHost.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include <algorithm>
#include <cmath>
#include <string_view>

#include "Base/Log.h"

#include "Core/Tuning.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/Player/PlayerController.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropWorld.h"
#include "Game/Stage/StageMapping.h"

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

    // 배틀십(최종보스): 레거시 SetArena 와 같이 아레나 왼쪽 밖(-300)에서 레일로 들어온다 (SandboxState::SpawnBossLab_ 과 같은 규칙)
    constexpr float kBossEntryOffsetX = 300.f;

    [[nodiscard]] Vec2 EnemyFeet_(
        const SpawnRequest& _request)
    {
        if (_request.positionMode == ePositionMode::LegacyCenter)
        {
            return { _request.position.x, _request.position.y + _request.halfHeight };
        }
        return _request.position;
    }
}   // namespace

// ===========================================
//  구성
// ===========================================

void StageWorldHost::BindWorld(
    EnemyWorld* _pEnemies,
    PropWorld*  _pProps)
{
    m_pEnemies = _pEnemies;
    m_pProps   = _pProps;
}

void StageWorldHost::BindRects(
    const slug::Span<const NamedRect> _rects)
{
    m_rects.clear();
    for (const NamedRect& rect: _rects)
    {
        m_rects.push_back(rect);
    }
}

void StageWorldHost::BindPlayer(
    PlayerController* _pPlayer,
    const Tuning*     _pTuning)
{
    m_pPlayer = _pPlayer;
    m_placeholders.BindPlayer(_pPlayer, _pTuning);
}

void StageWorldHost::Reset()
{
    m_placeholders.Reset();
    m_routes.clear();
    m_colliders.clear();
    m_nextId = 1;
}

// ===========================================
//  라우팅 표
// ===========================================

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

SpawnRoute* StageWorldHost::FindRouteMut_(
    const SpawnId _id)
{
    for (SpawnRoute& route: m_routes)
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
    const SpawnId     _id,
    const eRouteKind  _kind,
    const uint32_t    _handle)
{
    SpawnRoute route;
    route.id     = _id;
    route.kind   = _kind;
    route.handle = _handle;
    m_routes.push_back(route);
}

const NamedRect* StageWorldHost::FindRect_(
    const slug::StringView _name) const
{
    for (const NamedRect& rect: m_rects)
    {
        if (rect.name == _name)
        {
            return &rect;
        }
    }
    return nullptr;
}

uint64_t StageWorldHost::HashState() const
{
    uint64_t hash = m_placeholders.HashState();
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

// ===========================================
//  IStageHost: 엔티티
// ===========================================

SpawnId StageWorldHost::SpawnEntity(
    const SpawnRequest& _request)
{
    const SpawnId id = AllocId_();

    // 충돌체: 월드 객체가 아니라 스테이지 충돌 목록이다 (정의 이름과 무관)
    if (_request.kind == eEntityKind::Collider)
    {
        StageCollider collider;
        collider.id         = id;
        collider.box        = MakeColliderBox(_request);
        collider.layer      = _request.collisionLayer;
        collider.stageLayer = _request.bHasLayer ? _request.layer : eStageLayer::Collider;
        m_colliders.push_back(collider);
        AddRoute_(id, eRouteKind::Collider, 0);
        return id;
    }

    const ParamValue* pParams = _request.pParams;

    // 적 / NPC → EnemyWorld
    if (m_pEnemies && (_request.kind == eEntityKind::Enemy || _request.kind == eEntityKind::Npc))
    {
        SpawnParams params;
        params.facing = _request.facing != 0 ? _request.facing : -1;
        params.hp     = _request.bHasHp ? _request.hp : -1;
        params.drop   = _request.drop;
        params.userTag = id;

        Vec2 feet = EnemyFeet_(_request);
        if (_request.def == "battleship")
        {
            // 최종보스: params.arena = 아레나 사각형, 등장 위치는 아레나 왼쪽 밖
            if (pParams)
            {
                if (const ParamValue* pArena = pParams->Find("arena"); pArena && pArena->IsString())
                {
                    if (const NamedRect* pRect = FindRect_(pArena->AsString()))
                    {
                        params.arena = pRect->rect;
                        feet.x       = pRect->rect.min.x - kBossEntryOffsetX;
                    }
                }
            }
        }

        const EnemyHandle handle = m_pEnemies->Spawn(_request.def, feet, params, nullptr);
        if (!handle.IsNull())
        {
            AddRoute_(id, eRouteKind::Enemy, handle.id);
            return id;
        }
        // 정의가 없으면 아래 플레이스홀더로 떨어진다
    }

    // 소품 → PropWorld (스프라이트 중심 기준)
    if (m_pProps && _request.kind == eEntityKind::Prop)
    {
        const PropHandle handle = m_pProps->Spawn(_request.def, _request.position, MakePropSpawnParams(pParams));
        if (handle != kNoProp)
        {
            AddRoute_(id, eRouteKind::Prop, handle);
            return id;
        }
    }

    // 아이템 → PropWorld 가 가진 ItemWorld (이름 또는 random_* 그룹)
    if (m_pProps && _request.kind == eEntityKind::Item)
    {
        const Vec2 feet   = _request.positionMode == ePositionMode::Feet ? _request.position : ItemFeetFromCenter(_request.position);
        const uint32_t     itemId = m_pProps->SpawnItem(_request.def, feet, false);
        if (itemId != 0)
        {
            AddRoute_(id, eRouteKind::Item, itemId);
            return id;
        }
    }

    // 나머지 (정의가 아직 없는 적/소품/장식/아이템, decor) → 플레이스홀더
    const SpawnId inner = m_placeholders.SpawnEntity(_request);
    AddRoute_(id, eRouteKind::Placeholder, inner);
    return id;
}

bool StageWorldHost::IsDefeated(
    const SpawnId _id) const
{
    const SpawnRoute* pRoute = FindRoute(_id);
    if (!pRoute)
    {
        return true;   // 알 수 없는 ID 는 격파로 본다
    }
    switch (pRoute->kind)
    {
    case eRouteKind::Placeholder: return m_placeholders.IsDefeated(pRoute->handle);
    case eRouteKind::Enemy:       return !m_pEnemies || m_pEnemies->IsDefeated(EnemyHandle { pRoute->handle });
    case eRouteKind::Prop:        return !m_pProps || m_pProps->IsDestroyed(pRoute->handle);
    case eRouteKind::Item:        return !m_pProps || m_pProps->GetItems().Find(pRoute->handle) == nullptr;
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
    case eRouteKind::Placeholder: return m_placeholders.IsDestroyFinished(pRoute->handle);
    case eRouteKind::Enemy:       return !m_pEnemies || m_pEnemies->IsDestroyFinished(EnemyHandle { pRoute->handle });
    case eRouteKind::Prop:        return !m_pProps || m_pProps->IsDestroyFinished(pRoute->handle);
    case eRouteKind::Item:
    case eRouteKind::Collider:    return IsDefeated(_id);
    }
    return true;
}

void StageWorldHost::ClearEntities(
    const EntityFilter& _filter)
{
    m_placeholders.ClearEntities(_filter);

    // 적 (보병/차량/보스). 포로(npc) 는 Npc 비트가 있을 때만 지운다. 소품/아이템은 레이어 정리 대상이 아니다.
    if (m_pEnemies && (_filter.layerMask & LayerBit(eStageLayer::Monster)) != 0)
    {
        m_pEnemies->ClearMonsters((_filter.layerMask & LayerBit(eStageLayer::Npc)) != 0);
    }

    std::erase_if(m_colliders, [&_filter](const StageCollider& _c) { return (_filter.layerMask & LayerBit(_c.stageLayer)) != 0; });
}

void StageWorldHost::SendCommand(
    const SpawnId          _id,
    const slug::StringView  _command)
{
    // 버스(mb_truck) 의 startBattle 은 문 접촉으로 스스로 시작한다 (Bus.cpp). 그 밖의 명령은 아직 없다.
    static bool bWarnedOnce = false;
    if (_command == "startBattle")
    {
        return;
    }
    if (!bWarnedOnce)
    {
        bWarnedOnce = true;
        SLUG_LOG_WARN("스테이지 명령 '{}' (대상 {}) 은 아직 구현되지 않았습니다", std::string { _command }, _id);
    }
}

// ===========================================
//  IStageHost: 지면 / 플레이어 / 연출
// ===========================================

void StageWorldHost::SetGround(
    const slug::StringView _name)
{
    m_placeholders.SetGround(_name);
}

std::optional<float> StageWorldHost::GetGroundFindY(
    const float _x,
    const float _fromY) const
{
    return m_placeholders.GetGroundFindY(_x, _fromY);
}

Vec2 StageWorldHost::PlayerFeet() const
{
    return m_placeholders.PlayerFeet();
}

bool StageWorldHost::PlayerAlive() const
{
    return m_placeholders.PlayerAlive();
}

bool StageWorldHost::IsGameOver() const
{
    return m_placeholders.IsGameOver();
}

void StageWorldHost::SetPlayerInvincible(
    const float _seconds)
{
    m_placeholders.SetPlayerInvincible(_seconds);
    if (m_pPlayer)
    {
        m_pPlayer->ExtendInvincibility(SecondsToTicks_(_seconds));
    }
}

void StageWorldHost::WarpPlayer(
    const Vec2 _feet)
{
    m_placeholders.WarpPlayer(_feet);
}

void StageWorldHost::SetBackdrop(
    const slug::StringView _modeId)
{
    m_placeholders.SetBackdrop(_modeId);
}

void StageWorldHost::ShowBanner(
    const slug::String& _text,
    const float        _seconds)
{
    m_placeholders.ShowBanner(_text, _seconds);
}

void StageWorldHost::FadeTo(
    const eFadeTarget _target,
    const float       _seconds)
{
    m_placeholders.FadeTo(_target, _seconds);
}

void StageWorldHost::PlayBgm(
    const slug::StringView _name,
    const bool            _bLoop)
{
    m_placeholders.PlayBgm(_name, _bLoop);
}

void StageWorldHost::EndStage(
    const slug::StringView _nextState)
{
    m_placeholders.EndStage(_nextState);
}

void StageWorldHost::OnSectionEntered(
    const int          _index,
    const slug::StringView _id)
{
    m_placeholders.OnSectionEntered(_index, _id);
}

}   // namespace slug

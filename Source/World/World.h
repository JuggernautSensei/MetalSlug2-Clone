#pragma once
#include "Base/Types.h"
#include "Base/Assert.h"
#include <algorithm>
#include <tuple>
#include <type_traits>

#include "Base/Handle.h"
#include "Base/HandleStorage.h"
#include "Base/RGBA.h"
#include "Base/ResourcePool.h"
#include "Base/Math.h"

#include "Render/Animation.h"
#include "World/Combat.h"
#include "World/Physics.h"

namespace slug
{

// ===========================================
//  Entity
//   세대(token)가 있는 32bit 핸들. 파괴된 엔티티의 핸들은 슬롯이 재사용되어도 IsAlive == false 이다.
// ===========================================

struct EntityTag;
using Entity = slug::Handle<EntityTag>;

// ===========================================
//  컴포넌트 (단순 POD 데이터)
//   새 컴포넌트는 struct 를 만들고 아래 ComponentStorages 목록에 한 줄 추가한다.
// ===========================================

// 위치 + 직전 틱 위치(렌더 보간용) + 바라보는 방향
struct Transform
{
    Vec2 position     = { 0.f, 0.f };
    Vec2 prevPosition = { 0.f, 0.f };
    int          facing       = 1;   // +1 오른쪽, -1 왼쪽
};

// 스프라이트: 애니메이션 재생기 + 그리기 정보
struct Sprite
{
    AnimPlayer anim     = {};
    int        layer    = 0;
    int        sortKey  = 0;
    bool       bVisible = true;
    bool       bFlipX   = false;
    slug::RGBA  tint     = { 255, 255, 255, 255 };
};

// 소속 팀 (판정의 기본값 제공용)
struct Team
{
    eTeam team = eTeam::Neutral;
};

// Body(World/Physics.h), Health(World/Combat.h) 는 그대로 컴포넌트로 쓴다.

// ===========================================
//  World
//   엔티티 + 컴포넌트 저장소. 반복 순서는 항상 "생성 순서" 이므로 결정적이다.
//   파괴는 지연된다: Destroy() 는 대기열에 넣고 IsAlive() 를 즉시 false 로 만들지만,
//   컴포넌트 데이터는 FlushDestroyed() (스텝 끝) 까지 읽을 수 있다.
//   주의: Create()/Add() 는 저장소를 재할당할 수 있으므로 그 전에 받아 둔 컴포넌트 참조/포인터를 유지하지 말 것.
// ===========================================

class World
{
public:
    // ----- 엔티티 수명 -----
    [[nodiscard]] Entity Create();
    void                 Destroy(Entity _entity);   // 지연 파괴. 이미 죽었거나 중복 호출은 무시
    void                 FlushDestroyed();          // 대기열의 엔티티와 컴포넌트를 실제로 제거
    void                 Clear();

    [[nodiscard]] bool   IsAlive(Entity _entity) const;   // 파괴 요청된 엔티티는 false
    [[nodiscard]] bool   IsValid(Entity _entity) const;   // 실제 슬롯이 아직 남아 있는가 (파괴 대기 중이면 true)
    [[nodiscard]] size_t GetEntityCount() const { return m_order.size() - m_pendingDestroy.size(); }

    // 생성 순서대로 정렬된 엔티티 목록 (파괴 대기 포함 — 순회 시 IsAlive 확인 권장). ForEach 계열이 이를 대신해 준다.
    [[nodiscard]] slug::Span<const Entity> GetEntities() const { return m_order; }

    // ----- 컴포넌트 -----
    template<typename C>
    C& Add(
        const Entity _entity,
        const C&     _value = {})
    {
        SLUG_ASSERT(IsAlive(_entity), "죽은 엔티티에 컴포넌트를 추가할 수 없다.\n");
        return Storage_<C>().InsertOrReplace(_entity, _value);
    }

    template<typename C>
    [[nodiscard]] bool Has(
        const Entity _entity) const
    {
        return Storage_<C>().Contains(_entity);
    }

    template<typename C>
    [[nodiscard]] C& Get(
        const Entity _entity)
    {
        return Storage_<C>().Get(_entity);
    }

    template<typename C>
    [[nodiscard]] const C& Get(
        const Entity _entity) const
    {
        return Storage_<C>().Get(_entity);
    }

    template<typename C>
    [[nodiscard]] C* TryGet(
        const Entity _entity)
    {
        return Storage_<C>().GetOrNull(_entity);
    }

    template<typename C>
    [[nodiscard]] const C* TryGet(
        const Entity _entity) const
    {
        return Storage_<C>().GetOrNull(_entity);
    }

    template<typename C>
    void Remove(
        const Entity _entity)
    {
        if (Storage_<C>().Contains(_entity))
        {
            Storage_<C>().Erase(_entity);
        }
    }

    // ----- 순회 -----
    // 생성 순서대로 모든 (살아 있는) 엔티티에 대해 _func(Entity) 호출. 순회 중 Create/Destroy 가능
    // (새로 만든 엔티티는 이번 순회에 포함되지 않고, 파괴는 지연된다).
    template<typename Func>
    void ForEachEntity(
        Func&& _func)
    {
        const size_t count = m_order.size();
        for (size_t i = 0; i < count; ++i)
        {
            const Entity entity = m_order[i];
            if (IsAlive(entity))
            {
                _func(entity);
            }
        }
    }

    // 모든 Cs 를 가진 엔티티에 대해 _func(Entity, Cs&...) 호출 (생성 순서).
    template<typename... Cs, typename Func>
    void ForEach(
        Func&& _func)
    {
        static_assert(sizeof...(Cs) > 0, "ForEach 에는 컴포넌트 타입이 하나 이상 필요하다.");
        const size_t count = m_order.size();
        for (size_t i = 0; i < count; ++i)
        {
            const Entity entity = m_order[i];
            if (!IsAlive(entity))
            {
                continue;
            }

            const std::tuple<Cs*...> components = { Storage_<Cs>().GetOrNull(entity)... };
            if (((std::get<Cs*>(components) != nullptr) && ...))
            {
                _func(entity, *std::get<Cs*>(components)...);
            }
        }
    }

private:
    struct EntityRecord
    {
        bool bPendingDestroy = false;
    };

    using ComponentStorages = std::tuple<
        slug::HandleStorage<Entity, Transform>,
        slug::HandleStorage<Entity, Body>,
        slug::HandleStorage<Entity, Sprite>,
        slug::HandleStorage<Entity, Health>,
        slug::HandleStorage<Entity, Team>>;

    template<typename C>
    [[nodiscard]] slug::HandleStorage<Entity, C>& Storage_()
    {
        return std::get<slug::HandleStorage<Entity, C>>(m_storages);
    }

    template<typename C>
    [[nodiscard]] const slug::HandleStorage<Entity, C>& Storage_() const
    {
        return std::get<slug::HandleStorage<Entity, C>>(m_storages);
    }

    slug::ResourcePool<Entity, EntityRecord> m_entities       = {};
    slug::Vector<Entity>                     m_order          = {};   // 생성 순서 (파괴 대기 포함)
    slug::Vector<Entity>                     m_pendingDestroy = {};
    ComponentStorages                       m_storages       = {};
};

}   // namespace slug

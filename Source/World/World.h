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
struct EntityTag;
using Entity = slug::Handle<EntityTag>;

struct Transform
{
    Vec2 position     = { 0.f, 0.f };
    Vec2 prevPosition = { 0.f, 0.f };
    int          facing       = 1;   // +1 오른쪽, -1 왼쪽
};

struct Sprite
{
    AnimPlayer anim     = {};
    int        layer    = 0;
    int        sortKey  = 0;
    bool       bVisible = true;
    bool       bFlipX   = false;
    slug::RGBA  tint     = { 255, 255, 255, 255 };
};

struct Team
{
    eTeam team = eTeam::Neutral;
};

//   주의: Create()/Add() 는 저장소를 재할당할 수 있으므로 그 전에 받아 둔 컴포넌트 참조/포인터를 유지하지 말 것.

class World
{
public:
    [[nodiscard]] Entity Create();
    void                 Destroy(Entity _entity);
    void                 FlushDestroyed();
    void                 Clear();

    [[nodiscard]] bool   IsAlive(Entity _entity) const;
    [[nodiscard]] bool   IsValid(Entity _entity) const;
    [[nodiscard]] size_t GetEntityCount() const { return m_order.size() - m_pendingDestroy.size(); }

    [[nodiscard]] slug::Span<const Entity> GetEntities() const { return m_order; }

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
    slug::Vector<Entity>                     m_order          = {};
    slug::Vector<Entity>                     m_pendingDestroy = {};
    ComponentStorages                       m_storages       = {};
};
}

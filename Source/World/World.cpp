#include "World/World.h"

namespace slug
{

Entity World::Create()
{
    const Entity entity = m_entities.Emplace();
    m_order.push_back(entity);
    return entity;
}

void World::Destroy(
    const Entity _entity)
{
    if (!IsAlive(_entity))
    {
        return;
    }

    m_entities.Get(_entity).bPendingDestroy = true;
    m_pendingDestroy.push_back(_entity);
}

void World::FlushDestroyed()
{
    if (m_pendingDestroy.empty())
    {
        return;
    }

    for (const Entity entity: m_pendingDestroy)
    {
        std::apply(
            [entity](auto&... _storages) {
                ((_storages.Contains(entity) ? _storages.Erase(entity) : void()), ...);
            },
            m_storages);
        m_entities.Erase(entity);
    }
    m_pendingDestroy.clear();

    std::erase_if(m_order, [this](const Entity _entity) {
        return !m_entities.IsValid(_entity);
    });
}

void World::Clear()
{
    for (const Entity entity: m_order)
    {
        if (m_entities.IsValid(entity))
        {
            m_entities.Erase(entity);
        }
    }

    std::apply(
        [](auto&... _storages) {
            (_storages.Clear(), ...);
        },
        m_storages);
    m_order.clear();
    m_pendingDestroy.clear();
}

bool World::IsAlive(
    const Entity _entity) const
{
    const EntityRecord* pRecord = m_entities.GetOrNull(_entity);
    return m_entities.IsValid(_entity) && pRecord && !pRecord->bPendingDestroy;
}

bool World::IsValid(
    const Entity _entity) const
{
    return m_entities.IsValid(_entity);
}
}

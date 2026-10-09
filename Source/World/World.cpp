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
        return;   // 이미 죽었거나 파괴 대기 중인 핸들, 낡은 핸들은 무시
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
        m_entities.Erase(entity);   // 슬롯 토큰이 올라가 이전 핸들은 영구히 무효가 된다
    }
    m_pendingDestroy.clear();

    // 생성 순서를 유지한 채 무효가 된 핸들만 제거 (안정적)
    std::erase_if(m_order, [this](const Entity _entity) {
        return !m_entities.IsValid(_entity);
    });
}

void World::Clear()
{
    // Pool::Clear() 는 세대를 0 으로 되돌려 낡은 핸들이 되살아날 수 있으므로, 하나씩 지워 세대를 올린다.
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

}   // namespace slug

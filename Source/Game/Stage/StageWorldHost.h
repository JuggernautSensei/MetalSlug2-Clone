#pragma once
#include <cstdint>
#include <optional>
#include <utility>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Combat/CombatSystem.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "Game/Stage/StagePlaceholders.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{

class EnemyWorld;
class PropWorld;
class PlayerController;
class UiSystem;
struct Tuning;
struct GameContext;

enum class eRouteKind : uint8_t
{
    Placeholder,
    Enemy,
    Prop,
    Item,
    Collider
};

struct SpawnRoute
{
    SpawnId    id     = kInvalidSpawnId;
    eRouteKind kind   = eRouteKind::Placeholder;
    uint32_t   handle = 0;
};

struct StageCollider
{
    SpawnId         id         = kInvalidSpawnId;
    Aabb            box        = {};
    eCollisionLayer layer      = eCollisionLayer::Wall;
    eStageLayer     stageLayer = eStageLayer::Collider;
};

class StageWorldHost final : public IStageHost
{
public:
    void BindWorld(EnemyWorld* _pEnemies, PropWorld* _pProps);
    void BindRects(slug::Span<const NamedRect> _rects);

    void AddGroundMap(slug::StringView _name, GroundMap _map) { m_placeholders.AddGroundMap(_name, std::move(_map)); }
    void BindPlayer(PlayerController* _pPlayer, const Tuning* _pTuning);
    void BindUi(UiSystem* _pUi) { m_placeholders.BindUi(_pUi); }
    void SetContext(GameContext* _pCtx) { m_placeholders.SetContext(_pCtx); }

    void Reset();

    void RegisterPlaceholderHurtboxes(CombatSystem& _combat) const { m_placeholders.RegisterHurtboxes(_combat); }
    int  ApplyPlaceholderEvents(slug::Span<const HitEvent> _events) { return m_placeholders.ApplyEvents(_events); }
    void TickPlaceholders() { m_placeholders.Tick(); }

    [[nodiscard]] const StagePlaceholders&  GetPlaceholders() const { return m_placeholders; }
    [[nodiscard]] slug::Span<const StageCollider> GetColliders() const { return m_colliders; }
    [[nodiscard]] const SpawnRoute*         FindRoute(SpawnId _id) const;
    [[nodiscard]] size_t                    GetRouteCount() const { return m_routes.size(); }
    [[nodiscard]] const GroundMap*          GetActiveGround() const { return m_placeholders.GetActiveGround(); }
    [[nodiscard]] GroundMap*                GetActiveGroundMut() { return m_placeholders.GetActiveGroundMut(); }
    [[nodiscard]] const slug::String&        GetActiveGroundName() const { return m_placeholders.GetActiveGroundName(); }
    [[nodiscard]] uint64_t                  HashState() const;
    [[nodiscard]] bool                      IsEndRequested() const { return m_placeholders.IsEndRequested(); }

    SpawnId                SpawnEntity(const SpawnRequest& _request) override;
    [[nodiscard]] bool     IsDefeated(SpawnId _id) const override;
    [[nodiscard]] bool     IsDestroyFinished(SpawnId _id) const override;
    void                   ClearEntities(const EntityFilter& _filter) override;
    void                   SendCommand(SpawnId _id, slug::StringView _command) override;
    void                   SetGround(slug::StringView _name) override;
    [[nodiscard]] std::optional<float> GetGroundFindY(float _x, float _fromY) const override;
    [[nodiscard]] Vec2         PlayerFeet() const override;
    [[nodiscard]] bool                 PlayerAlive() const override;
    [[nodiscard]] bool                 IsGameOver() const override;
    void                               SetPlayerInvincible(float _seconds) override;
    void                               WarpPlayer(Vec2 _feet) override;
    void                               SetBackdrop(slug::StringView _modeId) override;
    void                               ShowBanner(const slug::String& _text, float _seconds) override;
    void                               FadeTo(eFadeTarget _target, float _seconds) override;
    void                               PlayBgm(slug::StringView _name, bool _bLoop) override;
    void                               EndStage(slug::StringView _nextState) override;
    void                               OnSectionEntered(int _index, slug::StringView _id) override;

private:
    [[nodiscard]] SpawnRoute*                FindRouteMut_(SpawnId _id);
    [[nodiscard]] const NamedRect*           FindRect_(slug::StringView _name) const;
    [[nodiscard]] SpawnId                    AllocId_();
    void                                     AddRoute_(SpawnId _id, eRouteKind _kind, uint32_t _handle);

    StagePlaceholders          m_placeholders = {};
    EnemyWorld*                m_pEnemies     = nullptr;
    PropWorld*                 m_pProps       = nullptr;
    PlayerController*          m_pPlayer      = nullptr;
    slug::Vector<NamedRect>     m_rects        = {};
    slug::Vector<SpawnRoute>    m_routes       = {};
    slug::Vector<StageCollider> m_colliders    = {};
    SpawnId                    m_nextId       = 1;
};
}

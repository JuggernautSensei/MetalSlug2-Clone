#pragma once
#include <cstdint>
#include <optional>
#include <utility>

#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

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

// ===========================================
//  StageWorldHost
//   IStageHost 의 실제 구현. 스크립트 스폰을 가장 알맞은 월드로 보낸다.
//     enemy / npc  → EnemyWorld (def 가 있으면)
//     prop         → PropWorld  (def 가 있으면)
//     item         → PropWorld::SpawnItem (아이템/그룹 이름이 있으면)
//     collider     → 스테이지 충돌체 목록 (벽 / 적 전용 바닥). StageState 가 매 틱 물리 세계에 합친다.
//     그 밖(decor, 정의가 아직 없는 def 등) → StagePlaceholders (상자, 피해 판정 가능 여부는 kind 따라)
//   스폰마다 러너용 SpawnId 를 새로 주고, 라우팅 표로 그 ID 가 어느 월드의 무엇인지 기억한다.
//   (플레이스홀더 owner ID 와 적/소품 owner ID 가 겹치지 않도록 SpawnId 는 독립된 번호다.)
//   지면 / 플레이어 / UI / 오디오 / 흐름 호출은 StagePlaceholders 에 위임한다 (바인딩된 객체를 쓴다).
// ===========================================

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
    uint32_t   handle = 0;   // Enemy: EnemyHandle.id, Prop: PropHandle, Item: 아이템 id, Placeholder: 내부 SpawnId
};

// box_collider (또는 collider kind): wall = 플레이어/적 벽, groundExceptPlayer = 적만 서는 원웨이 바닥
struct StageCollider
{
    SpawnId         id         = kInvalidSpawnId;
    Aabb            box        = {};
    eCollisionLayer layer      = eCollisionLayer::Wall;
    eStageLayer     stageLayer = eStageLayer::Collider;   // ClearEntities 필터용
};

class StageWorldHost final : public IStageHost
{
public:
    // --- 구성 (StageState 가 로드/재시작 때 호출, 비소유) ---
    void BindWorld(EnemyWorld* _pEnemies, PropWorld* _pProps);
    void BindRects(jug::Span<const NamedRect> _rects);   // params.arena 이름 해소용 (복사한다)

    // 아래는 StagePlaceholders 와 같은 의미로 위임한다
    void AddGroundMap(jug::StringView _name, GroundMap _map) { m_placeholders.AddGroundMap(_name, std::move(_map)); }
    void BindPlayer(PlayerController* _pPlayer, const Tuning* _pTuning);
    void BindUi(UiSystem* _pUi) { m_placeholders.BindUi(_pUi); }
    void SetContext(GameContext* _pCtx) { m_placeholders.SetContext(_pCtx); }

    // 재시작: 라우팅/충돌체/플레이스홀더를 비운다 (월드 자체는 호출자가 Reset 한다)
    void Reset();

    // --- 틱 훅 ---
    void RegisterPlaceholderHurtboxes(CombatSystem& _combat) const { m_placeholders.RegisterHurtboxes(_combat); }
    int  ApplyPlaceholderEvents(jug::Span<const HitEvent> _events) { return m_placeholders.ApplyEvents(_events); }
    void TickPlaceholders() { m_placeholders.Tick(); }

    // --- 조회 ---
    [[nodiscard]] const StagePlaceholders&  GetPlaceholders() const { return m_placeholders; }
    [[nodiscard]] jug::Span<const StageCollider> GetColliders() const { return m_colliders; }
    [[nodiscard]] const SpawnRoute*         FindRoute(SpawnId _id) const;
    [[nodiscard]] size_t                    GetRouteCount() const { return m_routes.size(); }
    [[nodiscard]] const GroundMap*          GetActiveGround() const { return m_placeholders.GetActiveGround(); }
    [[nodiscard]] GroundMap*                GetActiveGroundMut() { return m_placeholders.GetActiveGroundMut(); }
    [[nodiscard]] const jug::String&        GetActiveGroundName() const { return m_placeholders.GetActiveGroundName(); }
    [[nodiscard]] uint64_t                  HashState() const;
    [[nodiscard]] bool                      IsEndRequested() const { return m_placeholders.IsEndRequested(); }

    // --- IStageHost ---
    SpawnId                SpawnEntity(const SpawnRequest& _request) override;
    [[nodiscard]] bool     IsDefeated(SpawnId _id) const override;
    [[nodiscard]] bool     IsDestroyFinished(SpawnId _id) const override;
    void                   ClearEntities(const EntityFilter& _filter) override;
    void                   SendCommand(SpawnId _id, jug::StringView _command) override;
    void                   SetGround(jug::StringView _name) override;
    [[nodiscard]] std::optional<float> GetGroundFindY(float _x, float _fromY) const override;
    [[nodiscard]] jug::VECTOR2         PlayerFeet() const override;
    [[nodiscard]] bool                 PlayerAlive() const override;
    [[nodiscard]] bool                 IsGameOver() const override;
    void                               SetPlayerInvincible(float _seconds) override;
    void                               WarpPlayer(jug::VECTOR2 _feet) override;
    void                               SetBackdrop(jug::StringView _modeId) override;
    void                               ShowBanner(const jug::String& _text, float _seconds) override;
    void                               FadeTo(eFadeTarget _target, float _seconds) override;
    void                               PlayBgm(jug::StringView _name, bool _bLoop) override;
    void                               EndStage(jug::StringView _nextState) override;
    void                               OnSectionEntered(int _index, jug::StringView _id) override;

private:
    [[nodiscard]] SpawnRoute*                FindRouteMut_(SpawnId _id);
    [[nodiscard]] const NamedRect*           FindRect_(jug::StringView _name) const;
    [[nodiscard]] SpawnId                    AllocId_();
    void                                     AddRoute_(SpawnId _id, eRouteKind _kind, uint32_t _handle);

    StagePlaceholders          m_placeholders = {};
    EnemyWorld*                m_pEnemies     = nullptr;
    PropWorld*                 m_pProps       = nullptr;
    PlayerController*          m_pPlayer      = nullptr;
    jug::Vector<NamedRect>     m_rects        = {};
    jug::Vector<SpawnRoute>    m_routes       = {};
    jug::Vector<StageCollider> m_colliders    = {};
    SpawnId                    m_nextId       = 1;
};

}   // namespace slug

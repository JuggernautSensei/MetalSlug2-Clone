#pragma once
#include <cstdint>
#include <optional>
#include <utility>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/HitFlash.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

class EnemyWorld;
class PropWorld;
class PlayerController;
class UiSystem;
struct Tuning;
struct GameContext;

inline constexpr uint32_t kPlaceholderOwnerBase  = 9000;
inline constexpr int      kPlaceholderDeathTicks = 30;    // 사망 후 제거까지 (사망 연출 구간)
inline constexpr int      kPlaceholderKillScore  = 100;   // 처치 점수 (플레이스홀더 한 기 기준)

// 실제 적/소품이 아직 없는 이름을 상자로 그리는 플레이스홀더 한 개
struct PlaceholderEntity
{
    SpawnId     id         = kInvalidSpawnId;
    String      def        = {};
    eEntityKind kind       = eEntityKind::Enemy;
    eStageLayer layer      = eStageLayer::Monster;
    Vec2        feet       = { 0.f, 0.f };   // 발 중앙 (상자는 발 중앙 기준으로 그린다)
    Vec2        size       = { 40.f, 75.f };
    Health      health     = {};
    bool        bHittable  = true;
    int         flashTicks = 0;   // 피격 번쩍임 타이머 (HitFlash.h)
    int         deadTicks  = -1;

    [[nodiscard]] Aabb GetBox() const;
    [[nodiscard]] bool IsAlive() const;
};

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

// 스테이지의 IStageHost 구현. 적/소품/아이템은 실제 월드로 보내고, 나머지 이름은 플레이스홀더로 만든다.
class StageWorldHost final : public IStageHost
{
public:
    void BindWorld(EnemyWorld* _pEnemies, PropWorld* _pProps);
    void BindRects(Span<const NamedRect> _rects);
    void BindPlayer(PlayerController* _pPlayer, const Tuning* _pTuning);
    void BindUi(UiSystem* _pUi);
    void SetContext(GameContext* _pCtx);
    void AddGroundMap(StringView _name, GroundMap _map);

    void Reset();

    void RegisterPlaceholderHurtboxes(CombatSystem& _combat) const;
    int  ApplyPlaceholderEvents(Span<const HitEvent> _events);
    void TickPlaceholders();

    [[nodiscard]] Span<const PlaceholderEntity> GetPlaceholderEntities() const;
    [[nodiscard]] const String&                 GetLastSectionId() const;
    [[nodiscard]] int                           GetLastSectionIndex() const;
    [[nodiscard]] const String&                 GetEndStateName() const;
    [[nodiscard]] bool                          IsEndRequested() const;

    [[nodiscard]] Span<const StageCollider> GetColliders() const;
    [[nodiscard]] const SpawnRoute*         FindRoute(SpawnId _id) const;
    [[nodiscard]] const GroundMap*          GetActiveGround() const;
    [[nodiscard]] GroundMap*                GetActiveGroundMut();
    [[nodiscard]] const String&             GetActiveGroundName() const;
    [[nodiscard]] uint64_t                  HashState() const;

    SpawnId                            SpawnEntity(const SpawnRequest& _request) override;
    [[nodiscard]] bool                 IsDefeated(SpawnId _id) const override;
    [[nodiscard]] bool                 IsDestroyFinished(SpawnId _id) const override;
    void                               ClearEntities(const EntityFilter& _filter) override;
    void                               SendCommand(SpawnId _id, StringView _command) override;
    void                               SetGround(StringView _name) override;
    [[nodiscard]] std::optional<float> GetGroundFindY(float _x, float _fromY) const override;
    [[nodiscard]] Vec2                 PlayerFeet() const override;
    [[nodiscard]] bool                 PlayerAlive() const override;
    [[nodiscard]] bool                 IsGameOver() const override;
    void                               SetPlayerInvincible(float _seconds) override;
    void                               WarpPlayer(Vec2 _feet) override;
    void                               SetBackdrop(StringView _modeId) override;
    void                               ShowBanner(const String& _text, float _seconds) override;
    void                               FadeTo(eFadeTarget _target, float _seconds) override;
    void                               PlayBgm(StringView _name, bool _bLoop) override;
    void                               ChangeBgm(StringView _name, bool _bLoop, float _fadeOutSec) override;
    void                               PlaySfx(StringView _name) override;
    void                               EndStage(StringView _nextState) override;
    void                               OnSectionEntered(int _index, StringView _id) override;

private:
    struct NamedGround
    {
        String    name = {};
        GroundMap map  = {};
    };

    [[nodiscard]] SpawnRoute*              FindRouteMut_(SpawnId _id);
    [[nodiscard]] SpawnId                  AllocId_();
    void                                   AddRoute_(SpawnId _id, eRouteKind _kind, uint32_t _handle);
    [[nodiscard]] SpawnId                  SpawnPlaceholder_(const SpawnRequest& _request);
    [[nodiscard]] PlaceholderEntity*       FindAlive_(SpawnId _id);
    [[nodiscard]] const PlaceholderEntity* Find_(SpawnId _id) const;

    // 플레이스홀더 상태
    Vector<NamedGround>       m_grounds             = {};
    int                       m_activeGround        = -1;
    Vector<PlaceholderEntity> m_placeholderEntities = {};
    uint32_t                  m_lastOwner           = kPlaceholderOwnerBase;
    String                    m_lastSectionId       = {};
    int                       m_lastSectionIndex    = -1;
    bool                      m_bEndRequested       = false;
    String                    m_endStateName        = {};

    // 연결된 월드/플레이어/UI (Bind* 로 설정, 없으면 해당 종류는 플레이스홀더가 된다)
    EnemyWorld*       m_pEnemies = nullptr;
    PropWorld*        m_pProps   = nullptr;
    PlayerController* m_pPlayer  = nullptr;
    const Tuning*     m_pTuning  = nullptr;
    UiSystem*         m_pUi      = nullptr;
    GameContext*      m_pCtx     = nullptr;

    Vector<NamedRect>     m_rects     = {};
    Vector<SpawnRoute>    m_routes    = {};
    Vector<StageCollider> m_colliders = {};
    SpawnId               m_nextId    = 1;
};

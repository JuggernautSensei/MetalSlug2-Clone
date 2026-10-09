#pragma once
#include <cstdint>
#include <optional>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Combat/CombatSystem.h"
#include "Game/Stage/StageHost.h"
#include "World/Combat.h"
#include "World/GroundMap.h"

namespace slug
{

class PlayerController;
struct Tuning;
class UiSystem;
struct GameContext;

inline constexpr uint32_t kPlaceholderOwnerBase = 9000;
inline constexpr int      kPlaceholderDeathTicks = 30;    // 사망 후 제거까지 (사망 연출 구간)
inline constexpr int      kPlaceholderKillScore  = 100;   // 처치 점수 (플레이스홀더 한 기 기준)

struct PlaceholderEntity
{
    SpawnId      id         = kInvalidSpawnId;
    slug::String  def        = {};
    eEntityKind  kind       = eEntityKind::Enemy;
    eStageLayer  layer      = eStageLayer::Monster;
    Vec2 feet       = { 0.f, 0.f };   // 발 중앙 (상자는 발 중앙 기준으로 그린다)
    Vec2 size       = { 40.f, 75.f };
    Health       health     = {};
    bool         bHittable  = true;
    int          flashTicks = 0;               // 피격 번쩍임 남은 틱
    int          deadTicks  = -1;

    [[nodiscard]] Aabb GetBox() const { return Aabb::FromBottomCenter(feet, size); }
    [[nodiscard]] bool IsAlive() const { return deadTicks < 0 && health.IsAlive(); }
};

class StagePlaceholders final : public IStageHost
{
public:
    void AddGroundMap(slug::StringView _name, GroundMap _map);
    void BindPlayer(PlayerController* _pPlayer, const Tuning* _pTuning);
    void BindUi(UiSystem* _pUi);
    void SetContext(GameContext* _pCtx) { m_pCtx = _pCtx; }

    void Reset();

    void RegisterHurtboxes(CombatSystem& _combat) const;
    int  ApplyEvents(slug::Span<const HitEvent> _events);
    void Tick();

    [[nodiscard]] slug::Span<const PlaceholderEntity> GetEntities() const { return m_entities; }
    [[nodiscard]] const GroundMap*                   GetActiveGround() const;
    [[nodiscard]] GroundMap*                         GetActiveGroundMut();
    [[nodiscard]] const slug::String&                 GetActiveGroundName() const;
    [[nodiscard]] const slug::String&                 GetLastSectionId() const { return m_lastSectionId; }
    [[nodiscard]] int                                GetLastSectionIndex() const { return m_lastSectionIndex; }
    [[nodiscard]] bool                               IsEndRequested() const { return m_bEndRequested; }
    [[nodiscard]] const slug::String&                 GetEndStateName() const { return m_endStateName; }
    [[nodiscard]] bool                               IsPlayerInvincible() const { return m_invincibleTicks > 0; }
    [[nodiscard]] int                                GetSpawnCount() const { return m_spawnCount; }
    [[nodiscard]] uint64_t                           HashState() const;

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
    struct NamedGround
    {
        slug::String name = {};
        GroundMap   map  = {};
    };

    [[nodiscard]] PlaceholderEntity* FindAlive_(SpawnId _id);
    [[nodiscard]] const PlaceholderEntity* Find_(SpawnId _id) const;

    slug::Vector<NamedGround>      m_grounds        = {};
    int                           m_activeGround   = -1;
    slug::Vector<PlaceholderEntity> m_entities      = {};
    uint32_t                      m_lastOwner      = kPlaceholderOwnerBase;
    int                           m_spawnCount     = 0;
    int                           m_invincibleTicks = 0;

    PlayerController* m_pPlayer = nullptr;
    const Tuning*     m_pTuning = nullptr;
    UiSystem*         m_pUi     = nullptr;
    GameContext*      m_pCtx    = nullptr;

    slug::String m_lastSectionId  = {};
    int         m_lastSectionIndex = -1;
    bool        m_bEndRequested  = false;
    slug::String m_endStateName   = {};
};
}

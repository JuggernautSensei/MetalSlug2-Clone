#pragma once
#include <cstdint>
#include <optional>

#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

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

// ===========================================
//  StagePlaceholders
//   IStageHost 의 플레이스홀더 구현. 스크립트가 만드는 적/소품/NPC 를 "색 상자 + def 이름 + Hurtbox + Health" 로 대신한다.
//   실제 적/소품 구현이 들어오면 같은 IStageHost 를 다른 구현으로 바꿔 끼우면 된다 (StageRunner 는 변경 없음).
//   - 플레이어에게 피해를 주지 않는다 (사격/공격 없음). 칼/총/폭탄에 맞으면 HP 가 줄고 0 이면 사망 연출 후 제거된다.
//   - 지면: StageData 의 ground.maps 를 AddGroundMap 으로 등록한다. SetGround 가 활성 맵을 바꾼다.
//   - 모든 호출은 틱 안에서 일어나며, SetContext 로 넘겨준 GameContext 는 오디오 요청에만 쓴다.
// ===========================================

// 플레이스홀더 엔티티 owner ID 는 kPlaceholderOwnerBase + 순번 (플레이어 owner 1 과 겹치지 않는다). SpawnId 와 같은 값이다.
inline constexpr uint32_t kPlaceholderOwnerBase = 2000;
inline constexpr int      kPlaceholderDeathTicks = 30;    // 사망 후 제거까지 (사망 연출 구간)
inline constexpr int      kPlaceholderKillScore  = 100;   // 처치 점수 (플레이스홀더 한 기 기준)

struct PlaceholderEntity
{
    SpawnId      id         = kInvalidSpawnId;
    jug::String  def        = {};
    eEntityKind  kind       = eEntityKind::Enemy;
    eStageLayer  layer      = eStageLayer::Monster;
    jug::VECTOR2 feet       = { 0.f, 0.f };   // 발 중앙 (상자는 발 중앙 기준으로 그린다)
    jug::VECTOR2 size       = { 40.f, 75.f };
    Health       health     = {};
    bool         bHittable  = true;            // Hurtbox 를 가지는가 (enemy/npc/prop)
    int          flashTicks = 0;               // 피격 번쩍임 남은 틱
    int          deadTicks  = -1;              // -1: 생존, 0..: 사망 후 경과 틱 (kPlaceholderDeathTicks 에 제거)

    [[nodiscard]] Aabb GetBox() const { return Aabb::FromBottomCenter(feet, size); }
    [[nodiscard]] bool IsAlive() const { return deadTicks < 0 && health.IsAlive(); }
};

class StagePlaceholders final : public IStageHost
{
public:
    // --- 구성 (StageState 가 로드 시 호출) ---
    void AddGroundMap(jug::StringView _name, GroundMap _map);
    void BindPlayer(PlayerController* _pPlayer, const Tuning* _pTuning);   // 비소유. 둘 다 호스트보다 오래 산다
    void BindUi(UiSystem* _pUi);                                          // 비소유
    void SetContext(GameContext* _pCtx) { m_pCtx = _pCtx; }                 // 오디오 요청용 (틱 동안만 유효)

    // 재시작: 엔티티/종료 요청/섹션 기록을 비운다. 지면 맵과 바인딩은 유지된다.
    void Reset();

    // --- 틱 훅 (StageState 가 순서대로 호출) ---
    void RegisterHurtboxes(CombatSystem& _combat) const;   // 살아 있는 엔티티의 Hurtbox (플레이어 Tick 전)
    int  ApplyEvents(jug::Span<const HitEvent> _events);   // Resolve 결과 반영. 반환: 이번 틱 처치 수
    void Tick();                                          // 번쩍임/사망 연출 타이머, 제거

    // --- 조회 (렌더/디버그/테스트) ---
    [[nodiscard]] jug::Span<const PlaceholderEntity> GetEntities() const { return m_entities; }
    [[nodiscard]] const GroundMap*                   GetActiveGround() const;
    [[nodiscard]] const jug::String&                 GetActiveGroundName() const;
    [[nodiscard]] const jug::String&                 GetLastSectionId() const { return m_lastSectionId; }
    [[nodiscard]] int                                GetLastSectionIndex() const { return m_lastSectionIndex; }
    [[nodiscard]] bool                               IsEndRequested() const { return m_bEndRequested; }
    [[nodiscard]] const jug::String&                 GetEndStateName() const { return m_endStateName; }
    [[nodiscard]] bool                               IsPlayerInvincible() const { return m_invincibleTicks > 0; }
    [[nodiscard]] int                                GetSpawnCount() const { return m_spawnCount; }
    [[nodiscard]] uint64_t                           HashState() const;

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
    struct NamedGround
    {
        jug::String name = {};
        GroundMap   map  = {};
    };

    [[nodiscard]] PlaceholderEntity* FindAlive_(SpawnId _id);
    [[nodiscard]] const PlaceholderEntity* Find_(SpawnId _id) const;

    jug::Vector<NamedGround>      m_grounds        = {};
    int                           m_activeGround   = -1;
    jug::Vector<PlaceholderEntity> m_entities      = {};
    uint32_t                      m_lastOwner      = kPlaceholderOwnerBase;
    int                           m_spawnCount     = 0;
    int                           m_invincibleTicks = 0;

    PlayerController* m_pPlayer = nullptr;
    const Tuning*     m_pTuning = nullptr;
    UiSystem*         m_pUi     = nullptr;
    GameContext*      m_pCtx    = nullptr;

    jug::String m_lastSectionId  = {};
    int         m_lastSectionIndex = -1;
    bool        m_bEndRequested  = false;
    jug::String m_endStateName   = {};
};

}   // namespace slug

#pragma once
#include <cstdint>
#include <optional>

#include <JugX/Random.h>
#include <JugX/Scoped.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Props/PropDef.h"
#include "Game/Props/PropWorld.h"
#include "Game/Sandbox/Effects.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageRunner.h"
#include "Game/Stage/StageWorldHost.h"
#include "Game/Ui/ScreenFlow.h"
#include "Game/Ui/UiSystem.h"
#include "Render/Camera.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{

// ===========================================
//  StageState ("stage0")
//   Assets/Stage/Stage0.json 을 StageRunner 로 실행하는 게임 상태.
//   스크립트 엔티티는 StageWorldHost 가 실제 월드로 보낸다 (적 → EnemyWorld, 소품/아이템 → PropWorld,
//   정의가 없는 것 → StagePlaceholders). 플레이어/전투/투사체/카메라/HUD 는 SandboxState 와 같은 경로.
//
//   틱 순서 (SandboxState 와 같은 규칙):
//     combat.BeginTick → props.Tick → 벽/발판 안정 저장소 갱신 → 플레이스홀더/적/소품 Hurtbox 등록(플레이어 Tick 전)
//     → 플레이어 → 아이템 습득 → 적 AI/이동 → 투사체 Step → Resolve → 피해 적용(소품/적/플레이스홀더/플레이어/투사체)
//     → 이벤트 소비(점수/드롭/지면 패치/소리) → 연출 → StageRunner::Tick (섹션/게이트/카메라) → 끝 장면 판정
//
//   물리 세계(발판/벽)는 StageState 멤버 벡터에 매 틱 한 번 복사해 두고 그 주소만 넘긴다 (Span 수명 규칙).
//   끝 장면: 보스 처치 등으로 endStage 가 요청되면 MISSION COMPLETE, 플레이어 게임 오버면 GAME OVER 를 ScreenFlow 로 보이고 타이틀로 간다.
//   F1 디버그 중 F5 = 다음 섹션으로 워프, F6 = 보스 아레나로 워프 (GameContext::debugWarp 로 전달된다).
// ===========================================

class StageState final : public IGameState
{
public:
    StageState();

    void OnEnter(GameContext& _ctx) override;
    void OnExit(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] jug::Scoped<IGameState> TakeNextState() override;
    [[nodiscard]] bool                    IsFinished() const override;

    // --- 테스트/도구용 접근자 ---
    [[nodiscard]] bool                     IsLoaded() const { return m_bLoaded; }
    [[nodiscard]] const StageData&         GetData() const { return m_data; }
    [[nodiscard]] const StageRunner&       GetRunner() const { return *m_runner; }
    [[nodiscard]] const StageWorldHost&    GetHost() const { return m_host; }
    [[nodiscard]] PlayerController&        GetPlayer() { return m_player; }
    [[nodiscard]] const PlayerController&  GetPlayer() const { return m_player; }
    [[nodiscard]] const Camera&            GetCamera() const { return m_camera; }
    [[nodiscard]] const CombatSystem&      GetCombat() const { return m_combat; }
    [[nodiscard]] const EnemyWorld&        GetEnemies() const { return m_enemies; }
    [[nodiscard]] EnemyWorld&              GetEnemies() { return m_enemies; }
    [[nodiscard]] const PropWorld&         GetProps() const { return m_props; }
    [[nodiscard]] PropWorld&               GetProps() { return m_props; }
    [[nodiscard]] const UiSystem&          GetUi() const { return m_ui; }
    [[nodiscard]] UiSystem&                GetUi() { return m_ui; }
    [[nodiscard]] uint64_t                 GetTickCount() const { return m_tickCount; }
    [[nodiscard]] eFlowPhase               GetFlowPhase() const { return m_flow.GetPhase(); }
    [[nodiscard]] uint64_t                 HashState() const;

private:
    // 끝 장면 종류 (ScreenFlow 를 쓰는 경우)
    enum class eEndKind : uint8_t
    {
        None,
        Mission,
        GameOver
    };

    // 지면 패치 마스크 캐시 (GroundPatch 이벤트의 스프라이트 이름별, 실패도 기록한다)
    struct PatchMask
    {
        jug::String name    = {};
        bool        bLoaded = false;
        GroundMap   mask    = {};
    };

    void Load_(GameContext& _ctx);
    void Restart_(GameContext& _ctx);
    void HandleDebugWarp_(GameContext& _ctx);
    void StartFlow_(GameContext& _ctx, eEndKind _kind);
    [[nodiscard]] bool IsFlowing_() const { return m_endKind != eEndKind::None && !m_flow.IsDone(); }

    // 발판/벽 안정 저장소 갱신 (틱마다 한 번, 소품 Tick 뒤 / 플레이어 Tick 전)
    void UpdateWalls_();
    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;        // 플레이어: 지면 + 벽 + 발판
    [[nodiscard]] PhysicsWorld MakeEnemyPhysicsWorld_() const;   // 적: 지면 + 벽(화면 벽 제외) + 발판 (적 전용 바닥 포함)
    [[nodiscard]] jug::VECTOR2 GetRespawnFeet_() const;

    void ConsumeEnemyEvents_(GameContext& _ctx);
    void ConsumePropEvents_(GameContext& _ctx);
    void ApplyGroundPatch_(GameContext& _ctx, const jug::String& _sprite, jug::VECTOR2 _topLeft);
    [[nodiscard]] const GroundMap* FindPatchMask_(GameContext& _ctx, const jug::String& _sprite);
    void RequestSounds_(GameContext& _ctx, const jug::Vector<FxRequest>& _fx);

    void DrawBackdrop_(GameContext& _ctx, jug::VECTOR2 _cameraPos);
    void DrawWorld_(GameContext& _ctx, jug::VECTOR2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, jug::VECTOR2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx, jug::VECTOR2 _cameraPos);

    StageData                  m_data    = {};
    bool                       m_bLoaded = false;
    bool                       m_bFailed = false;   // 로드 실패: 바로 종료하고 타이틀로 보낸다
    bool                       m_bTakenNext = false;
    jug::FilePath              m_assetRoot = {};
    std::optional<StageRunner> m_runner  = std::nullopt;
    jug::RngMt19937            m_rng     = jug::RngMt19937 { 0x57A6E0ull };

    StageWorldHost    m_host   = {};
    Tuning            m_tuning = {};
    Camera            m_camera = {};
    jug::VECTOR2      m_prevCameraPos = { 0.f, 0.f };
    jug::VECTOR2      m_startFeet     = { 0.f, 0.f };

    // 물리 입력 저장소 (UpdateWalls_ 가 매 틱 채운다. 스팬은 이 벡터들을 가리킨다)
    jug::Vector<Wall>     m_walls         = {};   // 플레이어 벽 (화면 벽 + 소품 벽 + 충돌체 벽)
    jug::Vector<Wall>     m_enemyWalls    = {};   // 적 벽 (화면 벽 제외)
    jug::Vector<Platform> m_playerPlatforms = {};   // 플레이어 발판 (소품)
    jug::Vector<Platform> m_enemyPlatforms  = {};   // 적 발판 (소품 + 적 전용 바닥)

    // 정의 라이브러리 (월드보다 먼저 선언되어야 한다)
    EnemyDefLibrary  m_enemyLib = {};
    PropDefLibrary   m_propDefs = {};
    EnemyWorld       m_enemies  { &m_enemyLib };
    PropWorld        m_props    { nullptr, 1 };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    EffectList       m_effects     = {};
    AnimPlayer       m_bombAnim    = {};

    jug::Vector<PatchMask> m_patchMasks = {};

    // 렌더 자원 (렌더러가 있을 때만 유효)
    SpriteLibrary m_spriteLib = {};
    int           m_bgHandle  = -1;
    jug::Vector<int> m_backdropHandles = {};   // m_data.backdrops 와 같은 순서, 독립 파일이 아니면 -1
    bool          m_bRigBound = false;
    AnimId        m_bombClip  = jug::kNullHandle;

    // UI / 연출 (시뮬레이션 상태가 아니다)
    UiSystem    m_ui         = {};
    ScreenFlow  m_flow       = {};
    eEndKind    m_endKind    = eEndKind::None;
    PlayerStats m_prevStats  = {};
    eWeapon     m_prevWeapon = eWeapon::Pistol;
    int         m_prevScore  = 0;

    uint64_t m_tickCount = 0;
};

}   // namespace slug

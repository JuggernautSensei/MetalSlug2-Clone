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
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Sandbox/Effects.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StagePlaceholders.h"
#include "Game/Stage/StageRunner.h"
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
//   플레이어 / 전투 / 투사체 / 카메라 / HUD 는 SandboxState 와 같은 경로를 쓰고, 스테이지 스크립트의 엔티티만
//   StagePlaceholders(IStageHost) 가 임시 상자로 대신한다. 실제 적/소품은 같은 인터페이스로 나중에 끼운다.
//   틱 순서: 판정 시작 → 화면 벽 → Hurtbox 등록(플레이어 Tick 전) → 플레이어 → 투사체 → Resolve → 플레이스홀더 피해 →
//            디버그 워프 → StageRunner::Tick (섹션/예약/경계/게이트 + 카메라 추적)
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
    [[nodiscard]] bool                    IsLoaded() const { return m_bLoaded; }
    [[nodiscard]] const StageData&        GetData() const { return m_data; }
    [[nodiscard]] const StageRunner&      GetRunner() const { return *m_runner; }
    [[nodiscard]] const StagePlaceholders& GetHost() const { return m_host; }
    [[nodiscard]] PlayerController&       GetPlayer() { return m_player; }
    [[nodiscard]] const PlayerController& GetPlayer() const { return m_player; }
    [[nodiscard]] const Camera&           GetCamera() const { return m_camera; }
    [[nodiscard]] const CombatSystem&     GetCombat() const { return m_combat; }
    [[nodiscard]] uint64_t                GetTickCount() const { return m_tickCount; }
    [[nodiscard]] uint64_t                HashState() const;

private:
    void Load_(GameContext& _ctx);
    void Restart_(GameContext& _ctx);
    void HandleDebugWarp_(GameContext& _ctx);

    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;
    [[nodiscard]] jug::VECTOR2 GetRespawnFeet_() const;
    void                       UpdateWalls_();
    void                       RequestSounds_(GameContext& _ctx, const jug::Vector<FxRequest>& _fx);

    void DrawBackdrop_(GameContext& _ctx, jug::VECTOR2 _cameraPos);
    void DrawWorld_(GameContext& _ctx, jug::VECTOR2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, jug::VECTOR2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx, jug::VECTOR2 _cameraPos);

    StageData                  m_data    = {};
    bool                       m_bLoaded = false;
    bool                       m_bFailed = false;   // 로드 실패: 바로 종료하고 타이틀로 보낸다
    bool                       m_bTakenNext = false;
    std::optional<StageRunner> m_runner  = std::nullopt;
    jug::RngMt19937            m_rng     = jug::RngMt19937 { 0x57A6E0ull };

    StagePlaceholders m_host   = {};
    Tuning            m_tuning = {};
    Camera            m_camera = {};
    jug::VECTOR2      m_prevCameraPos = { 0.f, 0.f };
    jug::VECTOR2      m_startFeet     = { 0.f, 0.f };
    jug::Vector<Wall> m_walls         = {};

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    EffectList       m_effects     = {};
    AnimPlayer       m_bombAnim    = {};

    // 렌더 자원 (렌더러가 있을 때만 유효)
    SpriteLibrary m_spriteLib = {};
    int           m_bgHandle  = -1;
    bool          m_bRigBound = false;
    AnimId        m_bombClip  = jug::kNullHandle;

    // UI / 연출 (시뮬레이션 상태가 아니다)
    UiSystem    m_ui         = {};
    PlayerStats m_prevStats  = {};
    eWeapon     m_prevWeapon = eWeapon::Pistol;
    int         m_prevScore  = 0;

    uint64_t m_tickCount = 0;
};

}   // namespace slug

#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Sandbox/Effects.h"
#include "Game/Sandbox/TrainingTargets.h"
#include "Render/Camera.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{

// ===========================================
//  SandboxConfig
//   기본값 = 실제 게임 샌드박스(Stage0 거리 + 훈련 표적). 테스트는 값을 바꿔서 작은 맵을 주입한다.
// ===========================================

struct SandboxConfig
{
    bool                  bLoadBakedGround = true;   // Baked/Ground/Stage0_0.bin (없으면 평지로 대체)
    GroundMap             ground           = {};     // bLoadBakedGround == false 일 때 사용
    jug::Vector<Platform> platforms        = {};     // 추가 플랫폼 (테스트용 원웨이/솔리드)
    Aabb                  cameraBounds     = Aabb::FromMinMax({ 0.f, 0.f }, { 3520.f, 448.f });
    jug::VECTOR2          spawnFeet        = { 100.f, 378.f };
    bool                  bSpawnTargets    = true;   // 기본 훈련 표적/사수 배치
    bool                  bLoadTuningFile  = true;   // Assets/Tuning/Tuning.json 로드 (+ 라이브에서 핫리로드)
    Tuning                tuning           = {};     // bLoadTuningFile == false 일 때 사용
    bool                  bBackground      = true;   // Stage0_Main 배경 (렌더러가 있을 때만)
};

// ===========================================
//  SandboxState
//   M2 플레이어 테스트 맵: Stage0 거리 + 훈련 표적 + 사수 + 디버그 오버레이.
//   Tick 은 헤드리스에서도 동일하게 동작한다 (렌더러/애셋 없이도 판정과 이동은 같다).
//   틱 순서: 표적 Hurtbox 등록 → 플레이어 → 사수 → 투사체 → Resolve → 이벤트 적용 → 카메라/이펙트
// ===========================================

class SandboxState final : public IGameState
{
public:
    SandboxState();
    explicit SandboxState(SandboxConfig _config);

    void OnEnter(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    // --- 테스트/도구용 접근자 ---
    [[nodiscard]] PlayerController&       GetPlayer() { return m_player; }
    [[nodiscard]] const PlayerController& GetPlayer() const { return m_player; }
    [[nodiscard]] TrainingTargets&        GetTargets() { return m_targets; }
    [[nodiscard]] ProjectileSystem&       GetProjectiles() { return m_projectiles; }
    [[nodiscard]] CombatSystem&           GetCombat() { return m_combat; }
    [[nodiscard]] Tuning&                 GetTuning() { return m_tuning; }
    [[nodiscard]] const GroundMap&        GetGround() const { return m_ground; }
    [[nodiscard]] const Camera&           GetCamera() const { return m_camera; }
    [[nodiscard]] PlayerRig&              GetRig() { return m_rig; }
    [[nodiscard]] const PlayerRig&        GetRig() const { return m_rig; }
    [[nodiscard]] uint64_t                GetTickCount() const { return m_tickCount; }

    // 표적을 직접 추가 (테스트). 반환: 소유자 ID
    uint32_t AddTarget(eDummyKind _kind, jug::VECTOR2 _feet, bool _bShooter = false);

    // 플레이어와 표적을 처음 상태로 되돌린다
    void RestartGame();

    // 시뮬레이션 상태 해시 (결정성 검증: 플레이어 + 표적 + 투사체 수)
    [[nodiscard]] uint64_t HashState() const;

private:
    void                       BuildWorld_(GameContext& _ctx);
    void                       AddDefaultTargets_();
    void                       UpdateWalls_();
    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;
    [[nodiscard]] jug::VECTOR2 GetRespawnFeet_() const;

    void DrawWorld_(GameContext& _ctx, jug::VECTOR2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, jug::VECTOR2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx) const;

    SandboxConfig m_config  = {};
    Tuning        m_tuning  = {};
    TuningWatcher m_watcher = {};
    bool          m_bWatch  = false;

    GroundMap         m_ground        = {};
    jug::Vector<Wall> m_walls         = {};
    Camera            m_camera        = {};
    jug::VECTOR2      m_prevCameraPos = { 0.f, 0.f };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    TrainingTargets  m_targets     = {};
    EffectList       m_effects     = {};
    AnimPlayer       m_bombAnim    = {};

    // 렌더 자원 (렌더러가 있을 때만 유효)
    SpriteLibrary m_spriteLib = {};
    int           m_bgHandle  = -1;
    bool          m_bRigBound = false;
    AnimId        m_bombClip  = jug::kNullHandle;

    uint64_t m_tickCount = 0;
};

}   // namespace slug

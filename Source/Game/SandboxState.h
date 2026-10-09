#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Sandbox/Effects.h"
#include "Game/Sandbox/TrainingTargets.h"
#include "Game/Ui/UiSystem.h"
#include "Render/Camera.h"
#include "Game/Props/PropWorld.h"
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
    jug::FilePath         enemyDefPath     = {};     // 적 정의 JSON 파일 또는 정의 폴더. 비면 <assetRoot>/Defs 폴더 전체 (없으면 적 없이 동작)
    bool                  bEnemyLab        = false;  // 적 검증용: 보병 정의 8종을 플레이어 앞에 배치 (상태 이름 "enemylab")
    bool                  bVehicleLab      = false;  // 차량 검증용: 차량 5종을 화면 안에 배치 (상태 이름 "vehiclelab")
    bool                  bBossLab         = false;  // 보스 검증: 최종보스를 아레나(bossArena)에 등장시킨다 (상태 이름 "bosslab")
    Aabb                  bossArena        = Aabb::FromMinMax({ 0.f, 0.f }, { 640.f, 448.f });   // 640x448, 바닥 bossGroundY
    float                 bossGroundY      = 378.f;
    bool                  bPropLab         = false;  // 소품 검증: Stage0 상점 거리에 상점/트럭/아이템 배치 (상태 이름 "proplab")
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
    void OnExit(GameContext& _ctx) override;
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
    [[nodiscard]] UiSystem&               GetUi() { return m_ui; }
    [[nodiscard]] EnemyWorld&             GetEnemies() { return m_enemies; }
    [[nodiscard]] const EnemyWorld&       GetEnemies() const { return m_enemies; }
    [[nodiscard]] const EnemyDefLibrary&  GetEnemyLib() const { return m_enemyLib; }

    // 표적을 직접 추가 (테스트). 반환: 소유자 ID
    uint32_t AddTarget(eDummyKind _kind, jug::VECTOR2 _feet, bool _bShooter = false);

    // 적을 직접 스폰 (테스트/도구). 발 위치 아래 지면에 붙인다. 알 수 없는 정의면 null 핸들.
    EnemyHandle SpawnEnemy(jug::StringView _defName, jug::VECTOR2 _feet, const SpawnParams& _params = {});

    // 플레이어와 표적을 처음 상태로 되돌린다
    void RestartGame();

    // 시뮬레이션 상태 해시 (결정성 검증: 플레이어 + 표적 + 투사체 수)
    [[nodiscard]] uint64_t HashState() const;

private:
    void                       BuildWorld_(GameContext& _ctx);
    void                       AddDefaultTargets_();
    void                       UpdateWalls_();
    void                       UpdatePropCollision_();   // 소품 발판을 m_platforms 에 모은다
    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;
    [[nodiscard]] jug::VECTOR2 GetRespawnFeet_() const;
    [[nodiscard]] PhysicsWorld MakeEnemyPhysicsWorld_() const;   // 적용: 지면 + 플랫폼 (카메라 왼쪽 벽 제외)
    void                       SpawnDebugEnemy_(int _slot, float _aheadPx = 180.f);   // 숫자키 1..8 (보병 정의 8종), 9 = 차량 5종 순환 (플레이어 앞)
    void                       SpawnBossLab_();                                         // bosslab: 최종보스를 아레나 왼쪽 밖에서 등장시킨다

    void DrawWorld_(GameContext& _ctx, jug::VECTOR2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, jug::VECTOR2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx);
    void RequestSounds_(GameContext& _ctx, const jug::Vector<FxRequest>& _fx);

    SandboxConfig m_config  = {};
    Tuning        m_tuning  = {};
    TuningWatcher m_watcher = {};
    bool          m_bWatch  = false;

    GroundMap         m_ground        = {};
    jug::Vector<Wall> m_walls         = {};
    jug::Vector<Platform> m_platforms      = {};     // 플레이어 발판 (설정 + 소품). 적 발판은 설정값만 쓴다
    PropDefLibrary        m_propDefs       = {};     // Assets/Defs/Props/Props.json
    PropWorld             m_props          { nullptr, 1 };
    Camera            m_camera        = {};
    jug::VECTOR2      m_prevCameraPos = { 0.f, 0.f };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    TrainingTargets  m_targets     = {};
    EnemyDefLibrary  m_enemyLib    = {};
    EnemyWorld       m_enemies     { &m_enemyLib };   // m_enemyLib 보다 뒤에 선언되어야 한다
    int              m_vehicleCursor = 0;              // 숫자키 9: 다음에 뽑을 차량 번호
    EffectList       m_effects     = {};
    AnimPlayer       m_bombAnim    = {};

    // 렌더 자원 (렌더러가 있을 때만 유효)
    SpriteLibrary m_spriteLib = {};
    int           m_bgHandle  = -1;
    bool          m_bRigBound = false;
    AnimId        m_bombClip  = jug::kNullHandle;

    // UI / 오디오 (시뮬레이션 상태가 아니다: HashState 에 포함되지 않는다)
    UiSystem    m_ui                 = {};
    PlayerStats m_prevStats          = {};
    eWeapon     m_prevWeapon         = eWeapon::Pistol;
    int         m_prevScore          = 0;
    bool        m_bGameOverAnnounced = false;

    uint64_t m_tickCount = 0;
};

}   // namespace slug

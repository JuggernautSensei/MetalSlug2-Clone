#pragma once
#include "Base/Handle.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Player/SlugSystem.h"
#include "Game/Player/SlugTypes.h"
#include "Game/Props/PropWorld.h"
#include "Game/Sandbox/Effects.h"
#include "Game/Sandbox/TrainingTargets.h"
#include "Game/Ui/UiSystem.h"
#include "Render/Camera.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{

struct SandboxConfig
{
    bool             bLoadBakedGround = true;
    GroundMap        ground           = {};
    Vector<Platform> platforms        = {};
    Aabb             cameraBounds     = Aabb::FromMinMax({ 0.f, 0.f }, { 3520.f, 448.f });
    Vec2             spawnFeet        = { 100.f, 378.f };
    bool             bSpawnTargets    = true;
    bool             bLoadTuningFile  = true;
    Tuning           tuning           = {};
    bool             bBackground      = true;
    FilePath         enemyDefPath     = {};
    bool             bEnemyLab        = false;
    bool             bVehicleLab      = false;
    bool             bPropLab         = false;
    bool             bBusLab          = false;   // 버스 중간보스 연습장: 실제 스테이지 좌표, 카메라 락, 사망 후 Stage0_1 지면
    bool             bHeliLab         = false;
    bool             bGodMode         = false;   // 랩 전용: 플레이어 무적 (캡처 비교용)
    bool             bSlugLab         = false;   // sluglab: 평지에 주차된 SV-001 과 표적
    Vector<Vec2>     slugFeet         = {};      // 시작할 때 놓을 주차 SV-001 발 위치
    SlugTuning       slugTuning       = {};      // bLoadSlugFile 이 꺼져 있거나 파일이 없을 때 쓰는 값
    bool             bLoadSlugFile    = true;
};

class SandboxState final : public IGameState
{
public:
    SandboxState();
    explicit SandboxState(SandboxConfig _config);

    void OnEnter(GameContext& _ctx) override;
    void OnExit(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] PlayerController&       GetPlayer() { return m_player; }
    [[nodiscard]] const PlayerController& GetPlayer() const { return m_player; }
    [[nodiscard]] TrainingTargets&        GetTargets() { return m_targets; }
    [[nodiscard]] ProjectileSystem&       GetProjectiles() { return m_projectiles; }
    [[nodiscard]] CombatSystem&           GetCombat() { return m_combat; }
    [[nodiscard]] Tuning&                 GetTuning() { return m_tuning; }
    [[nodiscard]] const Camera&           GetCamera() const { return m_camera; }
    [[nodiscard]] PlayerRig&              GetRig() { return m_rig; }
    [[nodiscard]] const PlayerRig&        GetRig() const { return m_rig; }
    [[nodiscard]] uint64_t                GetTickCount() const { return m_tickCount; }
    [[nodiscard]] UiSystem&               GetUi() { return m_ui; }
    [[nodiscard]] EnemyWorld&             GetEnemies() { return m_enemies; }
    [[nodiscard]] const EnemyWorld&       GetEnemies() const { return m_enemies; }
    [[nodiscard]] const EnemyDefLibrary&  GetEnemyLib() const { return m_enemyLib; }
    [[nodiscard]] SlugSystem&             GetSlugs() { return m_slugs; }
    [[nodiscard]] const SlugSystem&       GetSlugs() const { return m_slugs; }

    uint32_t AddTarget(eDummyKind _kind, Vec2 _feet, bool _bShooter = false);

    EnemyHandle SpawnEnemy(StringView _defName, Vec2 _feet, const SpawnParams& _params = {});

    void RestartGame();

    [[nodiscard]] uint64_t HashState() const;

private:
    void                       BuildWorld_(GameContext& _ctx);
    void                       AddDefaultTargets_();
    void                       UpdateWalls_();
    void                       UpdatePropCollision_();
    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;
    [[nodiscard]] Vec2         GetRespawnFeet_() const;
    [[nodiscard]] PhysicsWorld MakeEnemyPhysicsWorld_() const;
    void                       SpawnDebugEnemy_(int _slot, float _aheadPx = 180.f);
    void                       TickBusLab_(GameContext& _ctx);

    void DrawWorld_(GameContext& _ctx, Vec2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, Vec2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx);
    void RequestSounds_(GameContext& _ctx, const Vector<FxRequest>& _fx);

    SandboxConfig m_config  = {};
    Tuning        m_tuning  = {};
    TuningWatcher m_watcher = {};
    bool          m_bWatch  = false;

    GroundMap        m_ground    = {};
    Vector<Wall>     m_walls     = {};
    Vector<Platform> m_platforms = {};
    PropDefLibrary   m_propDefs  = {};
    PropWorld        m_props { nullptr, 1 };
    Camera           m_camera        = {};
    Vec2             m_prevCameraPos = { 0.f, 0.f };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    TrainingTargets  m_targets     = {};
    EnemyDefLibrary  m_enemyLib    = {};
    EnemyWorld       m_enemies { &m_enemyLib };
    EnemyHandle      m_busHandle     = {};      // buslab: 버스 (파괴 연출이 끝나면 카메라 락을 푼다)
    bool             m_bBusUnlocked  = false;
    int              m_vehicleCursor = 0;   // 숫자키 9: 다음에 뽑을 차량 번호
    EffectList       m_effects       = {};
    AnimPlayer       m_bombAnim      = {};
    SlugSystem       m_slugs         = {};
    SlugTuning       m_slugTuning    = {};
    Vector<Wall>     m_slugWalls     = {};   // 적이 SV-001 위로 올라타지 못하게 막는 벽
    SlugStats        m_prevSlugStats = {};

    SpriteLibrary m_spriteLib = {};
    int           m_bgHandle  = -1;
    bool          m_bRigBound = false;
    AnimId        m_bombClip  = kNullHandle;

    UiSystem    m_ui                 = {};
    PlayerStats m_prevStats          = {};
    eWeapon     m_prevWeapon         = eWeapon::Pistol;
    int         m_prevScore          = 0;
    bool        m_bGameOverAnnounced = false;

    uint64_t m_tickCount = 0;
};

}   // namespace slug

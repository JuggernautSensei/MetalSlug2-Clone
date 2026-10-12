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

struct SandboxConfig
{
    bool             bLoadGround     = true;
    GroundMap        ground          = {};
    Vector<Platform> platforms       = {};
    Aabb             cameraBounds    = Aabb::FromMinMax({ 0.f, 0.f }, { 3520.f, 448.f });
    Vec2             spawnFeet       = { 100.f, 378.f };
    bool             bSpawnTargets   = true;
    bool             bLoadTuningFile = true;
    Tuning           tuning          = {};
    bool             bBackground     = true;
    FilePath         enemyDefPath    = {};
    bool             bEnemyLab       = false;
    bool             bVehicleLab     = false;
    bool             bPropLab        = false;
    bool             bBusLab         = false;
    bool             bBusBot         = false;
    bool             bHeliLab        = false;
    bool             bHeliBot        = false;
    int              heliHp          = -1;   // 헬기 랩 전용: 헬기 체력 (-1: 정의 기본값)
    bool             bGodMode        = false;
    bool             bSlugLab        = false;
    Vector<Vec2>     slugFeet        = {};
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

    [[nodiscard]] PlayerController&       GetPlayer();
    [[nodiscard]] const PlayerController& GetPlayer() const;
    [[nodiscard]] TrainingTargets&        GetTargets();
    [[nodiscard]] ProjectileSystem&       GetProjectiles();
    [[nodiscard]] CombatSystem&           GetCombat();
    [[nodiscard]] Tuning&                 GetTuning();
    [[nodiscard]] const Camera&           GetCamera() const;
    [[nodiscard]] PlayerRig&              GetRig();
    [[nodiscard]] const PlayerRig&        GetRig() const;
    [[nodiscard]] uint64_t                GetTickCount() const;
    [[nodiscard]] EnemyWorld&             GetEnemies();
    [[nodiscard]] const EnemyWorld&       GetEnemies() const;
    [[nodiscard]] const EnemyDefLibrary&  GetEnemyLib() const;
    [[nodiscard]] EnemyDefLibrary&        GetEnemyLibMutable();

    uint32_t AddTarget(eDummyKind _kind, Vec2 _feet, bool _bShooter = false);

    EnemyHandle SpawnEnemy(StringView _defName, Vec2 _feet, const SpawnParams& _params = {});

    void RestartGame();

    [[nodiscard]] bool ReloadEnemyDefs(const FilePath& _assetRoot);
    // 정의/투사체 이름 목록이 같을 때만 적을 유지한 채 정의를 교체한다. 다르면 false (아무것도 바꾸지 않음)
    [[nodiscard]] bool             ReloadEnemyDefsKeep(const FilePath& _assetRoot);
    [[nodiscard]] const GroundMap& GetGround() const;

    [[nodiscard]] uint64_t HashState() const;
    [[nodiscard]] uint64_t DebugHash() const override;

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
    [[nodiscard]] bool         LoadEnemyDefs_(const FilePath& _assetRoot, EnemyDefLibrary& _out) const;

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
    PropWorld        m_props { &m_propDefs, 1 };
    Camera           m_camera        = {};
    Vec2             m_prevCameraPos = { 0.f, 0.f };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    TrainingTargets  m_targets     = {};
    EnemyDefLibrary  m_enemyLib    = {};
    EnemyWorld       m_enemies { &m_enemyLib };
    EnemyHandle      m_busHandle     = {};
    bool             m_bBusUnlocked  = false;
    int              m_vehicleCursor = 0;   // 숫자키 9: 다음에 뽑을 차량 번호
    EffectList       m_effects       = {};
    AnimPlayer       m_bombAnim      = {};
    SlugSystem       m_slugs         = {};
    SlugTuning       m_slugTuning    = {};
    Vector<Wall>     m_slugWalls     = {};   // 적이 SV-001 위로 올라타지 못하게 막는 벽
    Vector<Wall>     m_enemyWalls    = {};
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

    uint64_t     m_tickCount = 0;
    InputHistory m_botInput  = {};
};

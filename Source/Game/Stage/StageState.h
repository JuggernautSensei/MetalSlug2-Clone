#pragma once
#include <cstdint>
#include <memory>
#include <optional>

#include "Base/Types.h"
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
#include "Game/Ui/Hud.h"
#include "Game/Ui/ScreenFlow.h"
#include "Game/Ui/UiSystem.h"
#include "Render/Camera.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

struct StageStateConfig
{
    String stageFile   = "Stage0.json";
    bool   bBot        = false;
    bool   bGod        = false;
    bool   bHmg        = false;
    bool   bAutoDamage = false;   // 보스 파트에 일정 속도 로 판정 피해를 준다
};

class StageState : public IGameState
{
public:
    StageState();
    explicit StageState(const StageStateConfig& _config);
    StageState(const StageState&)                                 = delete;
    StageState& operator=(const StageState&)                      = delete;
    StageState(StageState&&)                                      = delete;
    StageState&                           operator=(StageState&&) = delete;
    void                                  OnEnter() override;
    void                                  OnExit() override;
    void                                  Tick(const InputHistory& _input) override;
    void                                  Render(float _alpha) override;
    [[nodiscard]] Scoped<IGameState>      TakeNextState() override;
    [[nodiscard]] bool                    IsFinished() const override;
    [[nodiscard]] bool                    IsLoaded() const;
    [[nodiscard]] const StageData&        GetData() const;
    [[nodiscard]] const StageRunner&      GetRunner() const;
    [[nodiscard]] const StageWorldHost&   GetHost() const;
    [[nodiscard]] PlayerController&       GetPlayer();
    [[nodiscard]] const PlayerController& GetPlayer() const;
    [[nodiscard]] const Camera&           GetCamera() const;
    [[nodiscard]] const CombatSystem&     GetCombat() const;
    [[nodiscard]] const EnemyWorld&       GetEnemies() const;
    [[nodiscard]] EnemyWorld&             GetEnemies();
    [[nodiscard]] const PropWorld&        GetProps() const;
    [[nodiscard]] PropWorld&              GetProps();
    [[nodiscard]] const ProjectileSystem& GetProjectiles() const;
    [[nodiscard]] uint64_t                GetTickCount() const;
    [[nodiscard]] uint64_t                HashState() const;
    [[nodiscard]] uint64_t                DebugHash() const override;
    [[nodiscard]] HudModel                BuildHudModel() const;

private:
    enum class eEndKind
    {
        None,
        Mission,
        GameOver
    };

    struct PatchMask
    {
        String    name    = {};
        bool      bLoaded = false;
        GroundMap mask    = {};
    };

    [[nodiscard]] const InputHistory& MakeBotInput_();
    void                              Load_();
    void                              Restart_();
    void                              HandleDebugWarp_();
    void                              StartFlow_(eEndKind _kind);
    [[nodiscard]] bool                IsFlowing_() const;
    void                              UpdateWalls_();
    [[nodiscard]] PhysicsWorld        MakePhysicsWorld_() const;
    [[nodiscard]] PhysicsWorld        MakeEnemyPhysicsWorld_() const;
    [[nodiscard]] VECTOR2             GetRespawnFeet_() const;
    void                              ConsumeEnemyEvents_();
    void                              ConsumePropEvents_();
    void                              ApplyGroundPatch_(const String& _sprite, VECTOR2 _topLeft);
    [[nodiscard]] const GroundMap*    FindPatchMask_(const String& _sprite);
    void                              RequestSounds_(const Vector<FxRequest>& _fx);
    void                              DrawBackdrop_(VECTOR2 _cameraPos);
    void                              DrawWorld_(VECTOR2 _cameraPos, float _alpha);
    void                              DrawDebugBoxes_(Renderer& _renderer, VECTOR2 _cameraPos) const;
    void                              DrawHud_(VECTOR2 _cameraPos);

    StageStateConfig           m_config     = {};
    InputHistory               m_botInput   = {};
    bool                       m_bLabSetUp  = false;
    StageData                  m_data       = {};
    bool                       m_bLoaded    = false;
    bool                       m_bFailed    = false;
    bool                       m_bTakenNext = false;
    FilePath                   m_assetRoot  = {};
    std::optional<StageRunner> m_runner     = std::nullopt;
    RngMt19937                 m_rng        = RngMt19937 { 0x57A6E0ull };

    StageWorldHost m_host          = {};
    Tuning         m_tuning        = {};
    Camera         m_camera        = {};
    VECTOR2        m_prevCameraPos = { 0.f, 0.f };
    VECTOR2        m_startFeet     = { 0.f, 0.f };

    Vector<Wall>     m_walls           = {};
    Vector<Wall>     m_enemyWalls      = {};
    Vector<Platform> m_playerPlatforms = {};
    Vector<Platform> m_enemyPlatforms  = {};

    EnemyDefLibrary m_enemyLib = {};
    PropDefLibrary  m_propDefs = {};
    EnemyWorld      m_enemies { &m_enemyLib };
    PropWorld       m_props { &m_propDefs, 1 };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    EffectList       m_effects     = {};
    AnimPlayer       m_bombAnim    = {};
    AnimPlayer       m_chuteAnim   = {};

    Vector<PatchMask> m_patchMasks = {};

    SpriteLibrary m_spriteLib       = {};
    int           m_bgHandle        = -1;
    Vector<int>   m_backdropHandles = {};
    bool          m_bRigBound       = false;
    AnimId        m_bombClip        = kNullHandle;
    AnimId        m_chuteClip       = kNullHandle;

    UiSystem    m_ui         = {};
    ScreenFlow  m_flow       = {};
    eEndKind    m_endKind    = eEndKind::None;
    PlayerStats m_prevStats  = {};
    eWeapon     m_prevWeapon = eWeapon::Pistol;
    int         m_prevScore  = 0;

    DisplayTimer m_timer          = {};   // 화면 표시용 카운트다운
    String       m_timerSectionId = {};
    int          m_rescuedPows    = 0;

    uint64_t m_tickCount = 0;
};

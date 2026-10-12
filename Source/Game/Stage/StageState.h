#pragma once
#include <cstdint>
#include <memory>
#include <optional>

#include "Base/Handle.h"
#include "Base/Math.h"
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

// 상태 변형 (연습장): 다른 스테이지 파일, 봇 입력, 무적. 기본값은 정규 스테이지 (stage0)
struct StageStateConfig
{
    String stageFile   = "Stage0.json";   // Assets/Stage/ 아래 파일 이름
    bool   bBot        = false;           // 봇 입력: 보스 아레나로 걸어가 포탑/주포 아래에서 위로 쏜다 (보스 랩 캡처용)
    bool   bGod        = false;           // 플레이어 무적 (비교 캡처용)
    bool   bHmg        = false;           // 헤비머신건으로 시작
    bool   bAutoDamage = false;           // 보스 파트에 일정 속도 (약 20 dps) 로 판정 피해를 준다 (보스전 전 과정을 결정적으로 캡처하는 용도)
};

class StageState final : public IGameState
{
public:
    StageState();
    explicit StageState(const StageStateConfig& _config);

    StageState(const StageState&)            = delete;
    StageState& operator=(const StageState&) = delete;
    StageState(StageState&&)                 = delete;
    StageState& operator=(StageState&&)      = delete;

    void OnEnter(GameContext& _ctx) override;
    void OnExit(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] Scoped<IGameState> TakeNextState() override;
    [[nodiscard]] bool               IsFinished() const override;

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
    [[nodiscard]] const ProjectileSystem& GetProjectiles() const;   // 테스트/봇 관찰용 (읽기 전용)
    [[nodiscard]] const UiSystem&         GetUi() const;
    [[nodiscard]] UiSystem&               GetUi();
    [[nodiscard]] uint64_t                GetTickCount() const;
    [[nodiscard]] uint64_t                HashState() const;
    [[nodiscard]] uint64_t                DebugHash() const override;
    [[nodiscard]] HudModel                BuildHudModel() const;
    [[nodiscard]] int                     GetRescuedPows() const;

private:
    enum class eEndKind : uint8_t
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
    void                              Load_(GameContext& _ctx);
    void                              Restart_(GameContext& _ctx);
    void                              HandleDebugWarp_(GameContext& _ctx);
    void                              StartFlow_(GameContext& _ctx, eEndKind _kind);
    [[nodiscard]] bool                IsFlowing_() const;

    void                       UpdateWalls_();
    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;
    [[nodiscard]] PhysicsWorld MakeEnemyPhysicsWorld_() const;
    [[nodiscard]] Vec2         GetRespawnFeet_() const;

    void                           ConsumeEnemyEvents_(GameContext& _ctx);
    void                           ConsumePropEvents_(GameContext& _ctx);
    void                           ApplyGroundPatch_(GameContext& _ctx, const String& _sprite, Vec2 _topLeft);
    [[nodiscard]] const GroundMap* FindPatchMask_(GameContext& _ctx, const String& _sprite);
    void                           RequestSounds_(GameContext& _ctx, const Vector<FxRequest>& _fx);

    void DrawBackdrop_(GameContext& _ctx, Vec2 _cameraPos);
    void DrawWorld_(GameContext& _ctx, Vec2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, Vec2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx, Vec2 _cameraPos);

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
    Vec2           m_prevCameraPos = { 0.f, 0.f };
    Vec2           m_startFeet     = { 0.f, 0.f };

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
    AnimPlayer       m_chuteAnim   = {};   // 시작 낙하산 (적 낙하산 병사 클립 재사용)

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

    DisplayTimer m_timer          = {};   // 화면 표시용 카운트다운 (사망과 무관)
    String       m_timerSectionId = {};   // 구간이 바뀔 때 타이머를 60 으로 되돌린다
    int          m_rescuedPows    = 0;    // 구출한 POW 수 (미션 결과 상자)

    uint64_t m_tickCount = 0;
};

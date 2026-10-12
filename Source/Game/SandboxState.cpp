#include "Game/SandboxState.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>

#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Combat/BulletVisual.h"
#include "Game/Player/PlayerHitboxes.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Player/SlugDef.h"
#include "Game/Player/SlugRender.h"
#include "Game/Stage/StageMapping.h"
#include "Game/Ui/ScreenFlow.h"
#include "World/Constants.h"

namespace
{
[[nodiscard]] CameraTuning MakeCameraTuning_(
    const CameraSettings& _settings)
{
    CameraTuning tuning;
    tuning.viewSize     = { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(Renderer::kLogicalHeight) };
    tuning.deadzoneHalf = _settings.deadzoneHalf;
    tuning.smoothTime   = _settings.smoothTime;
    return tuning;
}

[[nodiscard]] Vec2 Lerp_(
    const Vec2  _a,
    const Vec2  _b,
    const float _t)
{
    return { _a.x + (_b.x - _a.x) * _t, _a.y + (_b.y - _a.y) * _t };
}

[[nodiscard]] Vec2 ToScreen_(
    const Vec2 _world,
    const Vec2 _camera)
{
    return { std::floor(_world.x - _camera.x), std::floor(_world.y - _camera.y) };
}

void DrawBox_(
    Renderer&   _renderer,
    const Aabb& _box,
    const Vec2  _camera,
    const RGBA  _color)
{
    const Vec2 topLeft     = ToScreen_(_box.min, _camera);
    const Vec2 bottomRight = ToScreen_(_box.max, _camera);
    _renderer.DrawRect(topLeft, { Max(1.f, bottomRight.x - topLeft.x), Max(1.f, bottomRight.y - topLeft.y) }, _color);
}
}   // namespace

SandboxState::SandboxState() = default;

SandboxState::SandboxState(
    SandboxConfig _config)
    : m_config(std::move(_config))
{
}

void SandboxState::OnEnter(
    GameContext& _ctx)
{
    BuildWorld_(_ctx);
    m_ui.Init(_ctx);
    m_prevStats          = m_player.GetStats();
    m_prevWeapon         = m_player.GetWeapon().weapon;
    m_prevScore          = m_player.GetScore();
    m_bGameOverAnnounced = false;

    _ctx.GetAudio().PlayBgm(kBgmStage, true);
}

void SandboxState::OnExit(
    GameContext& _ctx)
{
    _ctx.GetAudio().AllStop();
}

void SandboxState::BuildWorld_(
    GameContext& _ctx)
{
    m_platforms = m_config.platforms;

    m_tuning = m_config.tuning;
    m_bWatch = false;
    if (m_config.bLoadTuningFile && !_ctx.assetRoot.empty())
    {
        const FilePath path = GetDefaultTuningPath(_ctx.assetRoot);
        if (Result<Tuning> loaded = LoadTuning(path))
        {
            m_tuning = loaded.Take();
            if (!_ctx.IsHeadless())
            {
                m_watcher.Init(path, 60);
                m_bWatch = true;
            }
        }
        else
        {
            SLUG_LOG_WARN("Tuning.json 을 읽지 못해 기본값을 사용합니다: {}", path.string());
        }
    }

    m_slugTuning = m_config.slugTuning;
    if (m_config.bLoadSlugFile && !_ctx.assetRoot.empty())
    {
        const FilePath slugPath = GetDefaultSlugTuningPath(_ctx.assetRoot);
        if (std::filesystem::exists(slugPath))
        {
            if (Result<SlugTuning> slugLoaded = LoadSlugTuning(slugPath))
            {
                m_slugTuning = slugLoaded.Take();
            }
            else
            {
                SLUG_LOG_WARN("Slug.json 을 읽지 못해 기본값을 사용합니다: {}", slugPath.string());
            }
        }
    }

    if (m_config.bLoadGround)
    {
        bool bLoaded = false;
        if (!_ctx.assetRoot.empty())
        {
            if (std::optional<GroundMap> ground = LoadGroundMask(_ctx.assetRoot / "Ground" / "Stage0_0.png"))
            {
                m_ground = std::move(*ground);
                bLoaded  = true;
            }
        }
        if (!bLoaded)
        {
            SLUG_LOG_WARN("Stage0_0 지면을 읽지 못해 평지를 사용합니다");
            m_ground = GroundMap::MakeEmpty(3520, 448);
            m_ground.FillRect(0, 378, 3519, 447, true);
        }
    }
    else
    {
        m_ground = m_config.ground;
    }

    m_camera = Camera { MakeCameraTuning_(m_tuning.camera) };
    m_camera.SetBounds(m_config.cameraBounds);
    m_camera.SetForwardOnly(true);

    m_enemyLib = {};
    (void)LoadEnemyDefs_(_ctx.assetRoot, m_enemyLib);
    m_enemies.SetLibrary(&m_enemyLib);

    RestartGame();

    if (!_ctx.assetRoot.empty())
    {
        String message;
        if (Result<PropDefLibrary> lib = PropDefLibrary::Load(_ctx.assetRoot / "Defs" / "Props" / "Props.json", &message))
        {
            m_propDefs = lib.Take();
        }
        else
        {
            SLUG_LOG_WARN("소품 정의를 읽지 못해 소품을 배치하지 않습니다: {}", message);
        }
    }
    m_props.Reset(1);
    m_props.SetLibrary(&m_propDefs);
    if (m_config.bPropLab)
    {
        m_props.Spawn("shop_0", { 1274.f, 228.f });
        m_props.Spawn("shop_1", { 1489.f, 230.f });
        m_props.Spawn("shop_2", { 1661.f, 252.f });
        m_props.Spawn("truck", { 1430.f, 342.f });
        m_props.Spawn("truck", { 1810.f, 342.f });
        m_props.SpawnItem("gunitem_mg", { 1560.f, 378.f }, true);
        m_props.SpawnItem("random_score", { 1600.f, 378.f }, false);
        m_props.SpawnItem("bomb_supple", { 1625.f, 378.f }, false);
    }
    UpdatePropCollision_();

    m_bRigBound = false;
    if (_ctx.pAnims && _ctx.pAnims->GetClipCount() > 0)
    {
        (void)m_rig.Bind(*_ctx.pAnims, m_tuning.rig);
        m_bRigBound = true;
        m_bombClip  = _ctx.pAnims->Find("PlayerBomb");
        m_bombAnim.Play(m_bombClip, true);
    }

    if (!_ctx.IsHeadless())
    {
        if (Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(_ctx.assetRoot))
        {
            m_spriteLib = library.Take();
        }
        m_enemies.SetSpriteLibrary(&m_spriteLib);
        if (m_config.bBackground && _ctx.pSprites)
        {
            if (const Result<int> handle = _ctx.pSprites->LoadStandaloneTexture(_ctx.assetRoot / "Textures" / "Stage0_Main.png"))
            {
                m_bgHandle = handle.GetValue();
            }
            else
            {
                SLUG_LOG_WARN("Stage0_Main 배경을 읽지 못했습니다");
            }
        }
    }
}

void SandboxState::RestartGame()
{
    m_combat.GetResolver().Clear();
    m_combat.BeginTick();
    m_projectiles.Clear();
    m_effects.Clear();
    m_targets.Clear();
    m_enemies.Reset(1);

    m_player.Reset(m_tuning, m_config.spawnFeet);
    m_player.SnapToGround(MakePhysicsWorld_(), 16.f);

    m_slugs.Reset(m_slugTuning);
    m_slugWalls.clear();
    m_prevSlugStats = {};
    for (const Vec2& feet: m_config.slugFeet)
    {
        (void)m_slugs.Spawn(feet, 1);
    }

    m_camera.SetTarget({ m_config.spawnFeet.x, m_config.spawnFeet.y - 60.f });
    m_camera.SnapToTarget();
    m_prevCameraPos = m_camera.GetPositionNoShake();

    if (m_config.bSpawnTargets)
    {
        AddDefaultTargets_();
    }
    if (m_config.bEnemyLab)
    {
        for (int slot = 0; slot < 8; ++slot)
        {
            SpawnDebugEnemy_(slot, 200.f + 50.f * static_cast<float>(slot));
        }
    }
    if (m_config.bHeliLab)
    {
        // 헬기 랩: 화면 위 오른쪽에서 진입 (Docs/Spec_Helicopter.md 의 스테이지 배치 규약과 같은 위치)
        const Vec2  cam = m_camera.GetPositionNoShake();
        SpawnParams params;
        params.facing     = -1;
        params.bActivated = true;
        params.hp         = m_config.heliHp;
        (void)SpawnEnemy("heli", { cam.x + static_cast<float>(Renderer::kLogicalWidth) - 133.f, cam.y - 30.f }, params);
        if (m_config.bGodMode)
        {
            m_player.ExtendInvincibility(60 * 60 * 60);   // 비교 캡처용: 폭탄/미사일에 죽지 않는다
        }
    }
    if (m_config.bSlugLab)
    {
        SpawnParams left;
        left.facing = -1;
        (void)SpawnEnemy("soldier_sword", { 1000.f, 378.f }, left);
        (void)SpawnEnemy("soldier_sword", { 1060.f, 378.f }, left);
        (void)SpawnEnemy("soldier_bomb", { 1240.f, 378.f }, left);
        (void)SpawnEnemy("soldier_bzk", { 1400.f, 378.f }, left);
        (void)SpawnEnemy("jeep", { 1700.f, 378.f }, left);
        (void)SpawnEnemy("btank", { 1950.f, 378.f }, left);
    }
    if (m_config.bVehicleLab)
    {
        const Vec2  feet = m_player.GetFeet();
        SpawnParams left;
        left.facing = -1;
        (void)SpawnEnemy("jeep", { 200.f, feet.y }, left);
        (void)SpawnEnemy("btank", { 380.f, feet.y }, left);
        (void)SpawnEnemy("missle_tank", { 500.f, feet.y }, left);
        (void)SpawnEnemy("sarubia", { 600.f, feet.y }, left);
    }
    m_busHandle    = {};
    m_bBusUnlocked = false;
    if (m_config.bBusLab)
    {
        // 원작 락: 카메라 오른쪽 끝 3136 에서 멈춘다. 버스 앵커는 화면 왼쪽 끝 + 489 = 2985 (Stage0_1 잔해 지형의 중심과 같다)
        SpawnParams params;
        params.facing = -1;
        // 스폰 = 정지 앵커 (처음부터 그 자리). 앵커는 화면 왼쪽 끝 + 489
        m_busHandle = SpawnEnemy("bus", { m_config.cameraBounds.max.x - 640.f + 489.f, 378.f }, params);
        if (m_config.bBusBot)
        {
            m_player.PickUpWeapon(eWeapon::Hmg, m_tuning);
            m_player.ExtendInvincibility(60 * 60 * 60);
        }
    }
}

void SandboxState::TickBusLab_(
    GameContext& _ctx)
{
    if (!m_config.bBusLab || m_bBusUnlocked || m_busHandle.IsNull() || !m_enemies.IsDestroyFinished(m_busHandle))
    {
        return;
    }
    // 스테이지 데이터의 gate(destroyFinished) 와 같다: 지면을 잔해가 있는 Stage0_1 로 바꾸고 카메라 락을 푼다
    m_bBusUnlocked = true;
    if (!_ctx.assetRoot.empty())
    {
        if (std::optional<GroundMap> ground = LoadGroundMask(_ctx.assetRoot / "Ground" / "Stage0_1.png"))
        {
            m_ground = std::move(*ground);
        }
    }
    m_config.cameraBounds.max.x = 4200.f;
    m_camera.SetBounds(m_config.cameraBounds);
}

void SandboxState::AddDefaultTargets_()
{
    const float          groundY = m_config.spawnFeet.y;
    const SandboxTuning& sb      = m_tuning.sandbox;

    m_targets.Add(eDummyKind::Grunt, { 330.f, groundY }, false, sb);
    m_targets.Add(eDummyKind::Grunt, { 400.f, groundY }, false, sb);
    m_targets.Add(eDummyKind::Jeep, { 640.f, groundY }, false, sb);
    m_targets.Add(eDummyKind::Bus, { 940.f, groundY }, false, sb);
    m_targets.Add(eDummyKind::Grunt, { 1300.f, groundY }, true, sb);
    m_targets.Add(eDummyKind::Grunt, { 1520.f, groundY }, false, sb);
    m_targets.Add(eDummyKind::Jeep, { 1760.f, groundY }, false, sb);
    m_targets.Add(eDummyKind::Grunt, { 2100.f, groundY }, true, sb);
}

uint32_t SandboxState::AddTarget(
    const eDummyKind _kind,
    const Vec2       _feet,
    const bool       _bShooter)
{
    return m_targets.Add(_kind, _feet, _bShooter, m_tuning.sandbox);
}

bool SandboxState::LoadEnemyDefs_(
    const FilePath&  _assetRoot,
    EnemyDefLibrary& _out) const
{
    FilePath defsPath = m_config.enemyDefPath;
    if (defsPath.empty() && !_assetRoot.empty())
    {
        defsPath = EnemyDefLibrary::GetDefaultDirectory(_assetRoot);
    }
    if (defsPath.empty())
    {
        return false;
    }
    String message;
    if (Result<EnemyDefLibrary> lib = std::filesystem::is_directory(defsPath) ? EnemyDefLibrary::LoadDirectory(defsPath, &message) : EnemyDefLibrary::Load(defsPath, &message))
    {
        _out = lib.Take();
        return true;
    }
    SLUG_LOG_WARN("적 정의를 읽지 못해 적을 배치하지 않습니다: {}", message);
    return false;
}

bool SandboxState::ReloadEnemyDefs(
    const FilePath& _assetRoot)
{
    EnemyDefLibrary lib;
    if (!LoadEnemyDefs_(_assetRoot, lib))
    {
        return false;
    }
    m_enemyLib = std::move(lib);
    m_projectiles.Clear();
    m_enemies.Reset(1);
    m_enemies.SetLibrary(&m_enemyLib);
    return true;
}

bool SandboxState::ReloadEnemyDefsKeep(
    const FilePath& _assetRoot)
{
    EnemyDefLibrary lib;
    if (!LoadEnemyDefs_(_assetRoot, lib))
    {
        return false;
    }
    const Span<const EnemyDef>      newDefs  = lib.GetDefs();
    const Span<const EnemyDef>      oldDefs  = m_enemyLib.GetDefs();
    const Span<const ProjectileDef> newProjs = lib.GetProjectiles();
    const Span<const ProjectileDef> oldProjs = m_enemyLib.GetProjectiles();
    if (newDefs.size() != oldDefs.size() || newProjs.size() != oldProjs.size())
    {
        return false;
    }
    for (size_t i = 0; i < newDefs.size(); ++i)
    {
        if (newDefs[i].name != oldDefs[i].name || newDefs[i].behavior != oldDefs[i].behavior || newDefs[i].clips.size() != oldDefs[i].clips.size())
        {
            return false;
        }
    }
    for (size_t i = 0; i < newProjs.size(); ++i)
    {
        if (newProjs[i].name != oldProjs[i].name)
        {
            return false;
        }
    }
    m_enemyLib = std::move(lib);
    m_enemies.SetLibrary(&m_enemyLib);
    return true;
}

const GroundMap& SandboxState::GetGround() const
{
    return m_ground;
}

EnemyHandle SandboxState::SpawnEnemy(
    const StringView   _defName,
    const Vec2         _feet,
    const SpawnParams& _params)
{
    const PhysicsWorld world = MakeEnemyPhysicsWorld_();
    return m_enemies.Spawn(_defName, _feet, _params, &world);
}

void SandboxState::SpawnDebugEnemy_(
    const int   _slot,
    const float _aheadPx)
{
    if (_slot == 8)
    {
        static constexpr ARRAY<const char*, 4> kVehicles     = { "jeep", "btank", "missle_tank", "sarubia" };
        constexpr int                          kVehicleCount = static_cast<int>(kVehicles.size());
        const Vec2                             feet          = m_player.GetFeet();
        const StringView                       name          = kVehicles[m_vehicleCursor % kVehicleCount];
        m_vehicleCursor                                      = (m_vehicleCursor + 1) % kVehicleCount;
        SpawnParams params;
        params.facing = -1;
        (void)SpawnEnemy(name, { feet.x + 420.f, feet.y }, params);
        return;
    }

    static constexpr ARRAY<const char*, 8> kSlots     = { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder", "pow" };
    constexpr int                          kSlotCount = static_cast<int>(kSlots.size());
    if (_slot < 0 || _slot >= kSlotCount)
    {
        return;
    }

    const StringView name = kSlots[_slot];
    const Vec2       feet = m_player.GetFeet();
    Vec2             at   = { feet.x + _aheadPx, feet.y };
    if (name == "flying_bzk")
    {
        at.y = feet.y - 150.f;   // 낙하산: 공중에서 내려온다
    }
    else if (name == "soldier_hang")
    {
        at.y = feet.y - 120.f;   // 감시탑 아래에 매달림
    }
    SpawnParams params;
    params.facing = -1;
    (void)SpawnEnemy(name, at, params);
}

PhysicsWorld SandboxState::MakeEnemyPhysicsWorld_() const
{
    PhysicsWorld world;
    world.pGround   = &m_ground;
    world.platforms = m_config.platforms;
    world.walls     = m_enemyWalls;
    return world;
}

void SandboxState::UpdateWalls_()
{
    const float camLeft = m_camera.GetPositionNoShake().x;
    m_walls.clear();
    m_walls.push_back({ Aabb::FromMinMax({ camLeft - 64.f, -4000.f }, { camLeft, 4000.f }) });
    m_walls.push_back({ Aabb::FromMinMax({ m_config.cameraBounds.max.x, -4000.f }, { m_config.cameraBounds.max.x + 64.f, 4000.f }) });

    for (const Wall& wall: m_props.GetWalls())
    {
        m_walls.push_back(wall);
    }
    for (const Aabb& box: m_enemies.GetSolidBoxes())
    {
        m_walls.push_back({ box });   // 살아 있는 적의 def.solid (버스 차체)
    }
    UpdatePropCollision_();
}

void SandboxState::UpdatePropCollision_()
{
    m_platforms.clear();
    m_platforms.insert(m_platforms.end(), m_config.platforms.begin(), m_config.platforms.end());
    for (const Platform& platform: m_props.GetPlatforms())
    {
        m_platforms.push_back(platform);
    }
}

PhysicsWorld SandboxState::MakePhysicsWorld_() const
{
    PhysicsWorld world;
    world.pGround   = &m_ground;
    world.walls     = m_walls;
    world.platforms = m_platforms;
    return world;
}

Vec2 SandboxState::GetRespawnFeet_() const
{
    const Vec2 cam = m_camera.GetPositionNoShake();
    return { cam.x + m_tuning.player.respawnOffsetX, cam.y - m_tuning.player.respawnDropY };
}

void SandboxState::Tick(
    GameContext&        _ctx,
    const InputHistory& _input)
{
    ++m_tickCount;

    const InputHistory* pInput = &_input;
    if (m_config.bBusBot)
    {
        // 봇: 버스 앞 (x 2780) 까지 걸어가 멈추고 계속 사격한다 (버스 랩 캡처용)
        InputFrame frame = {};
        frame.Set(eAction::Right, m_player.GetFeet().x < 2780.f);
        frame.Set(eAction::Shot, m_tickCount >= 420 && m_player.GetFeet().x >= 2700.f && (m_tickCount % 20) < 3);
        m_botInput.Push(frame);
        pInput = &m_botInput;
    }
    if (m_config.bHeliBot)
    {
        // 봇: 120 틱마다 좌우로 걷고 위로 계속 사격한다 (헬기 랩 캡처용, 오른쪽 이동으로 시작)
        InputFrame frame  = {};
        const bool bRight = (m_tickCount / 120) % 2 == 0;
        frame.Set(eAction::Right, bRight);
        frame.Set(eAction::Left, !bRight);
        frame.Set(eAction::Up, true);
        frame.Set(eAction::Shot, (m_tickCount % 6) < 3);
        m_botInput.Push(frame);
        pInput = &m_botInput;
    }

    if (m_bWatch && m_watcher.Poll(m_tuning))
    {
        m_camera.SetTuning(MakeCameraTuning_(m_tuning.camera));
        if (_ctx.pAnims && m_bRigBound)
        {
            (void)m_rig.Bind(*_ctx.pAnims, m_tuning.rig);
        }
    }

    m_combat.BeginTick();
    {
        PropTickContext propCtx;
        propCtx.pGround      = &m_ground;
        propCtx.pAnims       = _ctx.pAnims;
        propCtx.cameraLeft   = m_camera.GetPositionNoShake().x;
        propCtx.cameraBottom = m_camera.GetPositionNoShake().y + Renderer::kLogicalHeight;
        propCtx.pItemTuning  = &m_tuning.items;
        m_props.Tick(propCtx);
    }
    UpdateWalls_();
    if (_ctx.debugSpawn > 0)
    {
        SpawnDebugEnemy_(_ctx.debugSpawn - 1);
        _ctx.debugSpawn = 0;
    }
    if (_ctx.debugWeapon > 0)
    {
        m_player.PickUpWeapon(WeaponFromDebugSlot(_ctx.debugWeapon), m_tuning);
        _ctx.debugWeapon = 0;
    }
    m_targets.RegisterHurtboxes(m_combat);
    m_enemies.RegisterHurtboxes(m_combat);
    m_props.RegisterHurtboxes(m_combat);

    PlayerContext playerCtx;
    playerCtx.pTuning       = &m_tuning;
    playerCtx.physics       = MakePhysicsWorld_();
    playerCtx.pCombat       = &m_combat;
    playerCtx.pProjectiles  = &m_projectiles;
    playerCtx.respawnFeet   = GetRespawnFeet_();
    playerCtx.godMode       = _ctx.godMode;
    playerCtx.bInfiniteAmmo = _ctx.bInfiniteAmmo;

    SlugContext slugCtx;
    slugCtx.pTuning      = &m_slugTuning;
    slugCtx.pInputMove   = &m_tuning.move;
    slugCtx.physics      = playerCtx.physics;
    slugCtx.pCombat      = &m_combat;
    slugCtx.pProjectiles = &m_projectiles;
    slugCtx.godMode      = _ctx.godMode;
    m_slugs.Tick(slugCtx, *pInput, m_player);
    m_player.Tick(playerCtx, *pInput);

    if (m_enemies.HasPlayerMinX())
    {
        m_player.PushFeetX(m_enemies.GetPlayerMinX());
        if (SlugVehicle* pRidden = m_slugs.GetRiddenOrNull())
        {
            pRidden->PushFeetX(m_enemies.GetPlayerMinX());
        }
    }
    if (m_enemies.HasPlayerMaxX())
    {
        m_player.PushFeetXMax(m_enemies.GetPlayerMaxX());
        if (SlugVehicle* pRidden = m_slugs.GetRiddenOrNull())
        {
            pRidden->PushFeetXMax(m_enemies.GetPlayerMaxX());
        }
    }

    for (const ItemPickup& pickup: m_props.GetItems().Collect(m_player.GetBodyBox()))
    {
        if (pickup.kind == eItemKind::Supply && pickup.supply == eSupplyKind::Bomb && m_slugs.OnBombSupply(pickup.amount))
        {
            continue;   // 탑승 중 폭탄 보급 상자 = 캐논탄
        }
        ApplyItemPickup(m_player, pickup, m_tuning);
    }

    m_targets.TickShooters(m_tuning.sandbox, m_player.GetFeet(), m_player.IsAlive(), m_projectiles, m_combat);

    m_slugWalls  = m_slugs.MakeEnemyWalls();
    m_enemyWalls = m_slugWalls;
    for (const Aabb& box: m_enemies.GetSolidBoxes())
    {
        m_enemyWalls.push_back({ box });
    }

    EnemyContext enemyCtx;
    enemyCtx.pTuning              = &m_tuning;
    enemyCtx.physics              = MakeEnemyPhysicsWorld_();
    enemyCtx.pCombat              = &m_combat;
    enemyCtx.pProjectiles         = &m_projectiles;
    enemyCtx.player.feet          = m_player.GetFeet();
    enemyCtx.player.bodyBox       = m_player.GetBodyBox();
    enemyCtx.player.center        = m_player.GetBodyBox().GetCenter();
    enemyCtx.player.bAlive        = m_player.IsAlive();
    enemyCtx.player.bInvulnerable = m_player.GetInvincibleTicks() > 0;
    if (const SlugVehicle* pRidden = m_slugs.GetRiddenOrNull())
    {
        // SV-001 에 타고 있으면 적은 차량을 표적으로 삼는다
        enemyCtx.player.feet          = pRidden->GetFeet();
        enemyCtx.player.bodyBox       = pRidden->MakeHurtbox().box;
        enemyCtx.player.center        = enemyCtx.player.bodyBox.GetCenter();
        enemyCtx.player.bInvulnerable = pRidden->GetMode() != eSlugMode::Driven;
    }
    enemyCtx.cameraView = Aabb::FromMinSize(m_camera.GetPositionNoShake(), { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(Renderer::kLogicalHeight) });
    enemyCtx.pAnims     = _ctx.pAnims;
    m_enemies.Tick(enemyCtx);

    m_projectiles.Step(&m_ground, m_combat, kStepSec);

    const Vector<HitEvent>& events = m_combat.Resolve();
    m_props.OnHitEvents(events);
    (void)m_props.TakeEvents();
    m_enemies.OnHitEvents(events);
    int enemyScore = 0;
    for (const EnemyEvent& event: m_enemies.TakeEvents())
    {
        enemyScore += event.score;
    }
    TickBusLab_(_ctx);
    const int score = m_targets.ApplyEvents(events, m_tuning.sandbox) + enemyScore;
    m_player.AddScore(score);
    m_player.OnHitEvents(events);
    m_slugs.OnHitEvents(events, m_combat.GetHitboxes());
    m_projectiles.OnHitEvents(events, m_combat);
    m_targets.Tick(m_tuning.sandbox);

    const Vector<FxRequest> fx = m_projectiles.TakeFx();
    RequestSounds_(_ctx, fx);
    m_ui.Tick();
    if (_ctx.pAnims)
    {
        for (const FxRequest& request: fx)
        {
            if (request.kind == eFxKind::BombExplosion)
            {
                m_effects.Spawn(*_ctx.pAnims, "PlayerBombExplosion", request.position, kBombBlastDepth + request.zOffset);
            }
            else if (request.kind == eFxKind::RocketExplosion)
            {
                m_effects.Spawn(*_ctx.pAnims, "Explosion_Mid", request.position, 56 + request.zOffset);
            }
            else if (request.kind == eFxKind::RocketSmoke)
            {
                m_effects.Spawn(*_ctx.pAnims, "Rocket_Smoke", request.position, 28 + request.zOffset);
            }
            else
            {
                m_effects.Spawn(*_ctx.pAnims, "Bullet_Destroy_Effect", request.position, kBulletHitDepth + request.zOffset);
            }
        }
        m_effects.Update(*_ctx.pAnims, kStepSec);
        m_bombAnim.Update(*_ctx.pAnims, kStepSec);

        for (TrainingDummy& dummy: m_targets.GetDummies())
        {
            if (dummy.kind != eDummyKind::Grunt)
            {
                continue;
            }
            const char* side = dummy.facing < 0 ? "Left" : "Right";
            if (dummy.deadTicks >= 0)
            {
                const char* death = dummy.lastKind == eHitKind::Melee ? "SD_Die_Slash" : (dummy.lastKind == eHitKind::Blast ? "SD_Die_Bomb" : "SD_Die_Stand_1");
                dummy.anim.Play(_ctx.pAnims->Find(std::format("{}_{}", death, side)), false);
            }
            else
            {
                dummy.anim.Play(_ctx.pAnims->Find(std::format("SD_Idle_{}", side)), false);
            }
            dummy.anim.Update(*_ctx.pAnims, kStepSec);
        }

        if (m_bRigBound)
        {
            m_rig.Update(m_player.MakeRigState());
        }
    }

    m_prevCameraPos = m_camera.GetPositionNoShake();
    m_camera.SetTarget({ m_player.GetFeet().x + m_tuning.camera.lookAheadX, m_player.GetFeet().y - 60.f });
    m_camera.Update(kStepSec);

    if (m_player.IsGameOver() && m_player.GetGameOverTicks() >= m_tuning.player.gameOverTicks)
    {
        RestartGame();
        m_ui.GetBanner().Clear();
        m_ui.GetFade().SetClear();
        m_prevStats          = m_player.GetStats();
        m_prevWeapon         = m_player.GetWeapon().weapon;
        m_prevScore          = m_player.GetScore();
        m_bGameOverAnnounced = false;
        _ctx.GetAudio().PlayBgm(kBgmStage, true);
    }
}

void SandboxState::RequestSounds_(
    GameContext&             _ctx,
    const Vector<FxRequest>& _fx)
{
    IAudioSink&        audio  = _ctx.GetAudio();
    const PlayerStats& stats  = m_player.GetStats();
    const eWeapon      weapon = m_player.GetWeapon().weapon;

    for (int i = m_prevStats.shotsFired; i < stats.shotsFired; ++i)
    {
        PlayShotSfx(audio, weapon, i);
    }
    for (int i = m_prevStats.knifeSlashes; i < stats.knifeSlashes; ++i)
    {
        audio.Play((i & 1) == 0 ? kKnife1 : kKnife2, 0.9f, 0.05f);
    }
    for (int i = m_prevStats.bombsThrown; i < stats.bombsThrown; ++i)
    {
        audio.Play(kBombThrow);
    }
    for (int i = m_prevStats.deaths; i < stats.deaths; ++i)
    {
        audio.Play(kPlayerDeath);
    }
    SlugStats slugTotal;
    for (const SlugVehicle& slug: m_slugs.GetAll())
    {
        const SlugStats& st = slug.GetStats();
        slugTotal.vulcanShots += st.vulcanShots;
        slugTotal.cannonShots += st.cannonShots;
        slugTotal.explosions += st.explosions;
    }
    for (int i = m_prevSlugStats.vulcanShots; i < slugTotal.vulcanShots; ++i)
    {
        PlayShotSfx(audio, eWeapon::Hmg, i);
    }
    for (int i = m_prevSlugStats.cannonShots; i < slugTotal.cannonShots; ++i)
    {
        audio.Play(kBombExplode, 0.8f, 0.05f);
    }
    for (int i = m_prevSlugStats.explosions; i < slugTotal.explosions; ++i)
    {
        audio.Play(kExplosion, 1.f, 0.03f);
    }
    m_prevSlugStats = slugTotal;

    for (const FxRequest& request: _fx)
    {
        if (request.kind == eFxKind::BombExplosion)
        {
            audio.Play(kBombExplode, 1.f, 0.03f);
        }
        else if (request.kind == eFxKind::RocketExplosion)
        {
            audio.Play(kBombExplode, 0.8f, 0.05f);
        }
    }

    if (weapon != m_prevWeapon)
    {
        PlayWeaponPickupSfx(audio, weapon);
    }
    if (m_player.GetScore() > m_prevScore)
    {
        audio.Play(kEnemyDeath, 0.7f, 0.1f);   // 에셋 없음
    }

    if (m_player.IsGameOver() && !m_bGameOverAnnounced)
    {
        m_bGameOverAnnounced          = true;
        const ScreenFlowConfig config = MakeGameOverFlowConfig();
        BannerSpec             spec   = MakeBannerSpec(config.bannerText, static_cast<float>(m_tuning.player.gameOverTicks) / 60.f);
        spec.color                    = config.bannerColor;
        m_ui.GetBanner().Show(std::move(spec));
        audio.FadeOutBgm(config.bgmFadeSeconds);
        audio.Play(config.sfx);
    }

    m_prevStats  = stats;
    m_prevWeapon = weapon;
    m_prevScore  = m_player.GetScore();
}

uint64_t SandboxState::HashState() const
{
    uint64_t hash = m_player.HashState();
    hash ^= m_targets.HashState() * 1099511628211ull;
    if (m_enemies.GetCount() > 0)
    {
        hash ^= m_enemies.HashState() * 1099511628211ull;   // 적이 없으면 기존 해시와 같다
    }
    if (m_props.GetCount() > 0)
    {
        hash ^= m_props.HashState() * 1099511628211ull;   // 소품이 없으면 기존 해시와 같다
    }
    if (m_slugs.GetCount() > 0)
    {
        hash ^= m_slugs.HashState() * 1099511628211ull;   // 차량이 없으면 기존 해시와 같다
    }
    hash ^= static_cast<uint64_t>(m_projectiles.GetBullets().size()) << 8;
    hash ^= static_cast<uint64_t>(m_projectiles.GetBombs().size()) << 16;
    hash *= 1099511628211ull;
    return hash;
}

void SandboxState::Render(
    GameContext& _ctx,
    const float  _alpha)
{
    Renderer& renderer = *_ctx.pRenderer;
    renderer.BeginFrame({ 156, 190, 224, 255 });

    const Vec2 camNow    = m_camera.GetPositionNoShake();
    const Vec2 cameraPos = { std::floor(m_prevCameraPos.x + (camNow.x - m_prevCameraPos.x) * _alpha), std::floor(m_prevCameraPos.y + (camNow.y - m_prevCameraPos.y) * _alpha) };

    if (m_bgHandle >= 0 && _ctx.pSprites)
    {
        _ctx.pSprites->DrawTexture(m_bgHandle, { 0.f, 0.f }, cameraPos, Renderer::kLogicalWidth, Renderer::kLogicalHeight);
    }

    DrawWorld_(_ctx, cameraPos, _alpha);
    if (_ctx.bDebugDraw)
    {
        DrawDebugBoxes_(renderer, cameraPos);
    }
    DrawHud_(_ctx);
}

void SandboxState::DrawWorld_(
    GameContext& _ctx,
    const Vec2   _cameraPos,
    const float  _alpha)
{
    Renderer&               renderer = *_ctx.pRenderer;
    SpriteBatch&            sprites  = *_ctx.pSprites;
    const AnimationLibrary* pAnims   = _ctx.pAnims;

    for (const TrainingDummy& dummy: m_targets.GetDummies())
    {
        const bool bDying = dummy.deadTicks >= 0;
        if (dummy.kind == eDummyKind::Grunt && pAnims)
        {
            if (const AnimFrame* pFrame = dummy.anim.GetFrameData(*pAnims))
            {
                const AnimClip*   pClip = pAnims->GetOrNull(dummy.anim.GetClip());
                const float       lift  = pClip ? static_cast<float>(pClip->GetFootBaseline()) : 0.f;
                SpriteDrawOptions options;
                options.flashAmount = IsHitFlashOn(dummy.flashTicks) ? 1.f : 0.f;
                sprites.Submit(10, static_cast<int>(dummy.feet.y), *pFrame, { dummy.feet.x, dummy.feet.y - lift }, options);
            }
            continue;
        }

        if (bDying)
        {
            continue;
        }

        const Aabb box   = Aabb::FromBottomCenter(dummy.feet, dummy.size);
        const RGBA color = IsHitFlashOn(dummy.flashTicks) ? RGBA { 255, 150, 150, 255 } : (dummy.kind == eDummyKind::Bus ? RGBA { 196, 150, 70, 255 } : RGBA { 96, 120, 70, 255 });
        const Vec2 tl    = ToScreen_(box.min, _cameraPos);
        renderer.FillRect(tl, dummy.size, color);
        renderer.DrawRect(tl, dummy.size, { 20, 20, 20, 255 });
        renderer.DrawDebugText({ tl.x + 4.f, tl.y + 4.f }, std::format("{}\n{}/{}", GetDummyName(dummy.kind), dummy.health.hp, dummy.health.maxHp), { 255, 255, 255, 255 });
    }

    if (pAnims)
    {
        m_enemies.Submit(sprites, *pAnims, m_projectiles, _alpha);
    }

    if (pAnims)
    {
        for (const SlugVehicle& slug: m_slugs.GetAll())
        {
            SubmitSlug(sprites, *pAnims, slug, _alpha);
        }
    }

    {
        const Vec2 feet = Lerp_(m_player.GetPrevFeet(), m_player.GetFeet(), _alpha);
        if (m_bRigBound && pAnims)
        {
            const RigState rigState = m_player.MakeRigState();
            if (rigState.bVisible)
            {
                m_rig.Submit(sprites, feet, ResolveZ(m_player.GetZ(), kPlayerDepth), static_cast<int>(feet.y));
            }
        }
        else if (m_player.IsAlive())
        {
            const Aabb box = Aabb::FromBottomCenter(feet, m_player.GetBody().size);
            renderer.FillRect(ToScreen_(box.min, _cameraPos), box.GetSize(), { 60, 120, 255, 255 });
        }
    }

    {
        for (const Bullet& bullet: m_projectiles.GetBullets())
        {
            const Vec2 pos = Lerp_(bullet.renderPrev, bullet.position, _alpha);
            if (pAnims && IsSlugOwner(bullet.owner) && SubmitSlugBullet(sprites, *pAnims, bullet, pos, ResolveZ(bullet.z, kPlayerBulletDepth)))
            {
                continue;
            }
            if (bullet.team == eTeam::Player && SubmitPlayerBullet(sprites, pAnims, m_spriteLib, bullet, pos, ResolveZ(bullet.z, kPlayerBulletDepth)))
            {
                continue;
            }
            const RGBA color = bullet.team == eTeam::Player ? RGBA { 255, 240, 120, 255 } : RGBA { 255, 80, 60, 255 };
            renderer.FillRect(ToScreen_({ pos.x - bullet.size.x * 0.5f, pos.y - bullet.size.y * 0.5f }, _cameraPos), bullet.size, color);
        }

        for (const Bomb& bomb: m_projectiles.GetBombs())
        {
            const Vec2 pos = Lerp_(bomb.renderPrev, bomb.position, _alpha);
            if (pAnims && IsSlugOwner(bomb.params.owner) && SubmitSlugShell(sprites, *pAnims, bomb, pos, ResolveZ(bomb.params.z, kPlayerBombDepth)))
            {
                continue;
            }
            if (pAnims)
            {
                if (const AnimFrame* pFrame = m_bombAnim.GetFrameData(*pAnims))
                {
                    sprites.Submit(ResolveZ(bomb.params.z, kPlayerBombDepth), 0, *pFrame, pos);
                    continue;
                }
            }
            renderer.FillRect(ToScreen_({ pos.x - 5.f, pos.y - 5.f }, _cameraPos), { 10.f, 10.f }, { 40, 40, 40, 255 });
        }
    }

    if (pAnims)
    {
        m_props.Submit(sprites, *pAnims, m_spriteLib);
        m_effects.Submit(*pAnims, sprites);
    }

    sprites.Flush(_cameraPos);
}

void SandboxState::DrawDebugBoxes_(
    Renderer&  _renderer,
    const Vec2 _cameraPos) const
{
    for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
    {
        DrawBox_(_renderer, hurtbox.box, _cameraPos, hurtbox.bInvulnerable ? RGBA { 120, 200, 255, 255 } : RGBA { 60, 255, 90, 255 });
    }
    for (const Hitbox& hitbox: m_combat.GetHitboxes())
    {
        DrawBox_(_renderer, hitbox.box, _cameraPos, { 255, 50, 50, 255 });
    }

    const Aabb body = m_player.GetBodyBox();
    DrawBox_(_renderer, body, _cameraPos, { 80, 120, 255, 255 });

    const Vec2  feet = m_player.GetFeet();
    const float fh   = m_player.GetBody().footHalfWidth;
    _renderer.DrawLine(ToScreen_({ feet.x - fh, feet.y }, _cameraPos), ToScreen_({ feet.x + fh, feet.y }, _cameraPos), { 255, 255, 0, 255 });
    _renderer.DrawLine(ToScreen_({ feet.x, feet.y - 4.f }, _cameraPos), ToScreen_({ feet.x, feet.y + 4.f }, _cameraPos), { 255, 255, 0, 255 });

    const Aabb trigger = MakeKnifeTriggerBox(feet, m_player.GetFacing(), body.GetWidth() * 0.5f, m_tuning.knife);
    DrawBox_(_renderer, trigger, _cameraPos, m_player.IsKnifeTargetInRange() ? RGBA { 255, 220, 40, 255 } : RGBA { 120, 110, 40, 255 });

    for (const Wall& wall: m_walls)
    {
        DrawBox_(_renderer, wall.box, _cameraPos, { 200, 80, 200, 255 });
    }
}

void SandboxState::DrawHud_(
    GameContext& _ctx)
{
    Renderer&               renderer = *_ctx.pRenderer;
    const PlayerController& p        = m_player;

    HudModel hud = MakeHudModel(p);
    if (const SlugVehicle* pRidden = m_slugs.GetRiddenOrNull())
    {
        hud.bSlugMode  = true;
        hud.cannonAmmo = pRidden->GetCannonAmmo();
        hud.bShowGauge = true;
        hud.gaugeRatio = pRidden->GetHpRatio();
    }
    m_ui.DrawHud(_ctx, hud);

    if (_ctx.bDebugDraw)
    {
        const Vec2   vel   = p.GetVelocity();
        const Vec2   pos   = p.GetFeet();
        const String debug = std::format(
            "STATE {} {}  AIM {}  OVL {}\nPOS {:.1f},{:.1f}  VEL {:.0f},{:.0f}  GND {}\nINV {}  KNIFE-RANGE {}  TICK {}  FPS {:.0f}\nF1 BOX  F2 PAUSE  F3 STEP  F4 SLOW  J SHOT  K JUMP  L BOMB",
            GetStateName(p.GetState()),
            p.GetFacing() > 0 ? "R" : "L",
            GetAimName(p.GetAim()),
            GetOverlayName(p.GetOverlay()),
            pos.x,
            pos.y,
            vel.x,
            vel.y,
            p.IsGrounded() ? 1 : 0,
            p.GetInvincibleTicks(),
            p.IsKnifeTargetInRange() ? 1 : 0,
            m_tickCount,
            _ctx.fps);
        renderer.DrawDebugText({ 9.f, 61.f }, debug, { 0, 0, 0, 255 });
        renderer.DrawDebugText({ 8.f, 60.f }, debug, { 255, 255, 255, 255 });
    }

    if (_ctx.bPaused)
    {
        TextStyle style;
        style.align = eTextAlign::Center;
        m_ui.DrawText(_ctx, Renderer::kLogicalWidth / 2, 216, "PAUSED", style);
    }

    m_ui.DrawOverlays(_ctx);
}

PlayerController& SandboxState::GetPlayer()
{
    return m_player;
}

const PlayerController& SandboxState::GetPlayer() const
{
    return m_player;
}

TrainingTargets& SandboxState::GetTargets()
{
    return m_targets;
}

ProjectileSystem& SandboxState::GetProjectiles()
{
    return m_projectiles;
}

CombatSystem& SandboxState::GetCombat()
{
    return m_combat;
}

Tuning& SandboxState::GetTuning()
{
    return m_tuning;
}

const Camera& SandboxState::GetCamera() const
{
    return m_camera;
}

PlayerRig& SandboxState::GetRig()
{
    return m_rig;
}

const PlayerRig& SandboxState::GetRig() const
{
    return m_rig;
}

uint64_t SandboxState::GetTickCount() const
{
    return m_tickCount;
}

UiSystem& SandboxState::GetUi()
{
    return m_ui;
}

EnemyWorld& SandboxState::GetEnemies()
{
    return m_enemies;
}

const EnemyWorld& SandboxState::GetEnemies() const
{
    return m_enemies;
}

const EnemyDefLibrary& SandboxState::GetEnemyLib() const
{
    return m_enemyLib;
}

EnemyDefLibrary& SandboxState::GetEnemyLibMutable()
{
    return m_enemyLib;
}

SlugSystem& SandboxState::GetSlugs()
{
    return m_slugs;
}

const SlugSystem& SandboxState::GetSlugs() const
{
    return m_slugs;
}

uint64_t SandboxState::DebugHash() const
{
    return HashState();
}

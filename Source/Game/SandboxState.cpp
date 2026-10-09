#include "Game/SandboxState.h"
#include "Base/Types.h"
#include "Base/Result.h"
#include "Base/RGBA.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>

#include "Base/Log.h"
#include "Base/Math.h"

#include "Game/Combat/BulletVisual.h"
#include "Game/Player/PlayerHitboxes.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Ui/ScreenFlow.h"
#include "World/Constants.h"

namespace slug
{

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
        const Vec2 _a,
        const Vec2 _b,
        const float        _t)
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
        Renderer&          _renderer,
        const Aabb&        _box,
        const Vec2 _camera,
        const slug::RGBA    _color)
    {
        const Vec2 topLeft     = ToScreen_(_box.min, _camera);
        const Vec2 bottomRight = ToScreen_(_box.max, _camera);
        _renderer.DrawRect(topLeft, { slug::Max(1.f, bottomRight.x - topLeft.x), slug::Max(1.f, bottomRight.y - topLeft.y) }, _color);
    }
}

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

    _ctx.GetAudio().PlayBgm(sfx::kBgmStage, true);
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
        const slug::FilePath path = GetDefaultTuningPath(_ctx.assetRoot);
        if (slug::Result<Tuning> loaded = LoadTuning(path))
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

    if (m_config.bLoadBakedGround)
    {
        bool bLoaded = false;
        if (!_ctx.assetRoot.empty())
        {
            if (slug::Result<GroundMap> ground = GroundMap::Load(_ctx.assetRoot / "Baked" / "Ground" / "Stage0_0.bin"))
            {
                m_ground = ground.Take();
                bLoaded  = true;
            }
        }
        if (!bLoaded)
        {
            SLUG_LOG_WARN("Stage0_0 지면을 읽지 못해 평지를 사용합니다 (Scripts/Bake.ps1 필요)");
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

    {
        slug::FilePath defsPath = m_config.enemyDefPath;
        if (defsPath.empty() && !_ctx.assetRoot.empty())
        {
            defsPath = EnemyDefLibrary::GetDefaultDirectory(_ctx.assetRoot);
        }
        m_enemyLib = {};
        if (!defsPath.empty())
        {
            slug::String message;
            if (slug::Result<EnemyDefLibrary> lib = std::filesystem::is_directory(defsPath) ? EnemyDefLibrary::LoadDirectory(defsPath, &message) : EnemyDefLibrary::Load(defsPath, &message))
            {
                m_enemyLib = lib.Take();
            }
            else
            {
                SLUG_LOG_WARN("적 정의를 읽지 못해 적을 배치하지 않습니다: {}", message);
            }
        }
        m_enemies.SetLibrary(&m_enemyLib);
    }

    RestartGame();

    if (!_ctx.assetRoot.empty())
    {
        slug::String message;
        if (slug::Result<PropDefLibrary> lib = PropDefLibrary::Load(_ctx.assetRoot / "Defs" / "Props" / "Props.json", &message))
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
        if (slug::Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(_ctx.assetRoot / "Baked"))
        {
            m_spriteLib = library.Take();
        }
        m_enemies.SetSpriteLibrary(&m_spriteLib);
        if (m_config.bBackground && _ctx.pSprites)
        {
            if (const slug::Result<int> handle = _ctx.pSprites->LoadStandaloneTexture(_ctx.assetRoot / "Baked" / "Textures" / "Stage0_Main.png"))
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
    if (m_config.bVehicleLab)
    {
        const Vec2 feet = m_player.GetFeet();
        SpawnParams        left;
        left.facing = -1;
        (void)SpawnEnemy("jeep", { 200.f, feet.y }, left);
        (void)SpawnEnemy("btank", { 380.f, feet.y }, left);
        (void)SpawnEnemy("missle_tank", { 500.f, feet.y }, left);
        (void)SpawnEnemy("sarubia", { 600.f, feet.y }, left);
        (void)SpawnEnemy("aparch", { 420.f, 150.f }, left);
    }
    if (m_config.bBossLab)
    {
        SpawnBossLab_();
    }
}

void SandboxState::SpawnBossLab_()
{
    SpawnParams params;
    params.facing = 1;
    params.arena  = m_config.bossArena;
    (void)m_enemies.Spawn("bigshiee", { m_config.bossArena.min.x - 300.f, m_config.bossGroundY }, params, nullptr);
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
    const eDummyKind   _kind,
    const Vec2 _feet,
    const bool         _bShooter)
{
    return m_targets.Add(_kind, _feet, _bShooter, m_tuning.sandbox);
}

EnemyHandle SandboxState::SpawnEnemy(
    const slug::StringView _defName,
    const Vec2    _feet,
    const SpawnParams&    _params)
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
        static constexpr const char* kVehicles[] = { "jeep", "btank", "missle_tank", "sarubia", "aparch" };
        constexpr int                kVehicleCount = static_cast<int>(sizeof(kVehicles) / sizeof(kVehicles[0]));
        const Vec2           feet          = m_player.GetFeet();
        const slug::StringView        name          = kVehicles[m_vehicleCursor % kVehicleCount];
        m_vehicleCursor                            = (m_vehicleCursor + 1) % kVehicleCount;
        SpawnParams params;
        params.facing = -1;
        (void)SpawnEnemy(name, { feet.x + 420.f, name == "aparch" ? feet.y - 150.f : feet.y }, params);
        return;
    }

    static constexpr const char* kSlots[] ={ "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder", "pow" };
    constexpr int                kSlotCount = static_cast<int>(sizeof(kSlots) / sizeof(kSlots[0]));
    if (_slot < 0 || _slot >= kSlotCount)
    {
        return;
    }

    const slug::StringView name = kSlots[_slot];
    const Vec2     feet = m_player.GetFeet();
    Vec2           at   = { feet.x + _aheadPx, feet.y };
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
        propCtx.pGround    = &m_ground;
        propCtx.pAnims     = _ctx.pAnims;
        propCtx.cameraLeft = m_camera.GetPositionNoShake().x;
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
    playerCtx.pTuning      = &m_tuning;
    playerCtx.physics      = MakePhysicsWorld_();
    playerCtx.pCombat      = &m_combat;
    playerCtx.pProjectiles = &m_projectiles;
    playerCtx.respawnFeet  = GetRespawnFeet_();
    m_player.Tick(playerCtx, _input);

    if (m_enemies.HasPlayerMinX())
    {
        m_player.PushFeetX(m_enemies.GetPlayerMinX());
    }
    if (m_enemies.HasPlayerMaxX())
    {
        m_player.PushFeetXMax(m_enemies.GetPlayerMaxX());
    }

    for (const ItemPickup& pickup: m_props.GetItems().Collect(m_player.GetBodyBox()))
    {
        ApplyItemPickup(m_player, pickup, m_tuning);
    }

    m_targets.TickShooters(m_tuning.sandbox, m_player.GetFeet(), m_player.IsAlive(), m_projectiles, m_combat);

    EnemyContext enemyCtx;
    enemyCtx.pTuning      = &m_tuning;
    enemyCtx.physics      = MakeEnemyPhysicsWorld_();
    enemyCtx.pCombat      = &m_combat;
    enemyCtx.pProjectiles = &m_projectiles;
    enemyCtx.player.feet        = m_player.GetFeet();
    enemyCtx.player.bodyBox     = m_player.GetBodyBox();
    enemyCtx.player.center      = m_player.GetBodyBox().GetCenter();
    enemyCtx.player.bAlive      = m_player.IsAlive();
    enemyCtx.player.bInvulnerable = m_player.GetInvincibleTicks() > 0;
    enemyCtx.cameraView   = Aabb::FromMinSize(m_camera.GetPositionNoShake(), { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(Renderer::kLogicalHeight) });
    enemyCtx.pAnims       = _ctx.pAnims;
    m_enemies.Tick(enemyCtx);

    m_projectiles.Step(&m_ground, m_combat, kStepSec);

    const slug::Vector<HitEvent>& events = m_combat.Resolve();
    m_props.OnHitEvents(events);
    (void)m_props.TakeEvents();
    m_enemies.OnHitEvents(events);
    int enemyScore = 0;
    for (const EnemyEvent& event: m_enemies.TakeEvents())
    {
        enemyScore += event.score;
    }
    const int score = m_targets.ApplyEvents(events, m_tuning.sandbox) + enemyScore;
    m_player.AddScore(score);
    m_player.OnHitEvents(events);
    m_projectiles.OnHitEvents(events, m_combat);
    m_targets.Tick(m_tuning.sandbox);

    const slug::Vector<FxRequest> fx = m_projectiles.TakeFx();
    RequestSounds_(_ctx, fx);
    m_ui.Tick();
    if (_ctx.pAnims)
    {
        for (const FxRequest& request: fx)
        {
            if (request.kind == eFxKind::BombExplosion)
            {
                m_effects.Spawn(*_ctx.pAnims, "PlayerBombExplosion", request.position, 60);
            }
            else if (request.kind == eFxKind::RocketExplosion)
            {
                m_effects.Spawn(*_ctx.pAnims, "Explosion_Mid", request.position, 56);
            }
            else if (request.kind == eFxKind::RocketSmoke)
            {
                m_effects.Spawn(*_ctx.pAnims, "Rocket_Smoke", request.position, 28);
            }
            else
            {
                m_effects.Spawn(*_ctx.pAnims, "Bullet_Destroy_Effect", request.position, 55);
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
        _ctx.GetAudio().PlayBgm(sfx::kBgmStage, true);
    }
}

void SandboxState::RequestSounds_(
    GameContext&                  _ctx,
    const slug::Vector<FxRequest>& _fx)
{
    IAudioSink&        audio = _ctx.GetAudio();
    const PlayerStats& stats = m_player.GetStats();
    const eWeapon      weapon = m_player.GetWeapon().weapon;

    for (int i = m_prevStats.shotsFired; i < stats.shotsFired; ++i)
    {
        PlayShotSfx(audio, weapon, i);
    }
    for (int i = m_prevStats.knifeSlashes; i < stats.knifeSlashes; ++i)
    {
        audio.Play((i & 1) == 0 ? sfx::kKnife1 : sfx::kKnife2, 0.9f, 0.05f);
    }
    for (int i = m_prevStats.bombsThrown; i < stats.bombsThrown; ++i)
    {
        audio.Play(sfx::kBombThrow);
    }
    for (int i = m_prevStats.deaths; i < stats.deaths; ++i)
    {
        audio.Play(sfx::kPlayerDeath);
    }
    for (const FxRequest& request: _fx)
    {
        if (request.kind == eFxKind::BombExplosion)
        {
            audio.Play(sfx::kBombExplode, 1.f, 0.03f);
        }
        else if (request.kind == eFxKind::RocketExplosion)
        {
            audio.Play(sfx::kBombExplode, 0.8f, 0.05f);
        }
    }

    if (weapon != m_prevWeapon)
    {
        PlayWeaponPickupSfx(audio, weapon);
    }
    if (m_player.GetScore() > m_prevScore)
    {
        audio.Play(sfx::kEnemyDeath, 0.7f, 0.1f);   // 에셋 없음
    }

    if (m_player.IsGameOver() && !m_bGameOverAnnounced)
    {
        m_bGameOverAnnounced = true;
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
    GameContext&       _ctx,
    const Vec2 _cameraPos,
    const float        _alpha)
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
                if (dummy.flashTicks > 0)
                {
                    options.tint = { 255, 110, 110, 255 };
                }
                sprites.Submit(10, static_cast<int>(dummy.feet.y), *pFrame, { dummy.feet.x, dummy.feet.y - lift }, options);
            }
            continue;
        }

        if (bDying)
        {
            continue;
        }

        const Aabb         box   = Aabb::FromBottomCenter(dummy.feet, dummy.size);
        const slug::RGBA    color = dummy.flashTicks > 0 ? slug::RGBA { 255, 150, 150, 255 } : (dummy.kind == eDummyKind::Bus ? slug::RGBA { 196, 150, 70, 255 } : slug::RGBA { 96, 120, 70, 255 });
        const Vec2 tl    = ToScreen_(box.min, _cameraPos);
        renderer.FillRect(tl, dummy.size, color);
        renderer.DrawRect(tl, dummy.size, { 20, 20, 20, 255 });
        renderer.DrawDebugText({ tl.x + 4.f, tl.y + 4.f }, std::format("{}\n{}/{}", GetDummyName(dummy.kind), dummy.health.hp, dummy.health.maxHp), { 255, 255, 255, 255 });
    }

    if (pAnims)
    {
        m_enemies.Submit(sprites, *pAnims, m_projectiles, _alpha);
    }

    {
        const Vec2 feet = Lerp_(m_player.GetPrevFeet(), m_player.GetFeet(), _alpha);
        if (m_bRigBound && pAnims)
        {
            const RigState rigState = m_player.MakeRigState();
            if (rigState.bVisible)
            {
                m_rig.Submit(sprites, feet, 20, static_cast<int>(feet.y));
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
            if (bullet.team == eTeam::Player && SubmitPlayerBullet(sprites, pAnims, m_spriteLib, bullet, pos, 30))
            {
                continue;
            }
            const slug::RGBA color = bullet.team == eTeam::Player ? slug::RGBA { 255, 240, 120, 255 } : slug::RGBA { 255, 80, 60, 255 };
            renderer.FillRect(ToScreen_({ pos.x - bullet.size.x * 0.5f, pos.y - bullet.size.y * 0.5f }, _cameraPos), bullet.size, color);
        }

        for (const Bomb& bomb: m_projectiles.GetBombs())
        {
            const Vec2 pos = Lerp_(bomb.renderPrev, bomb.position, _alpha);
            if (pAnims)
            {
                if (const AnimFrame* pFrame = m_bombAnim.GetFrameData(*pAnims))
                {
                    sprites.Submit(31, 0, *pFrame, pos);
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
    Renderer&          _renderer,
    const Vec2 _cameraPos) const
{
    for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
    {
        DrawBox_(_renderer, hurtbox.box, _cameraPos, hurtbox.bInvulnerable ? slug::RGBA { 120, 200, 255, 255 } : slug::RGBA { 60, 255, 90, 255 });
    }
    for (const Hitbox& hitbox: m_combat.GetHitboxes())
    {
        DrawBox_(_renderer, hitbox.box, _cameraPos, { 255, 50, 50, 255 });
    }

    const Aabb body = m_player.GetBodyBox();
    DrawBox_(_renderer, body, _cameraPos, { 80, 120, 255, 255 });

    const Vec2 feet = m_player.GetFeet();
    const float        fh   = m_player.GetBody().footHalfWidth;
    _renderer.DrawLine(ToScreen_({ feet.x - fh, feet.y }, _cameraPos), ToScreen_({ feet.x + fh, feet.y }, _cameraPos), { 255, 255, 0, 255 });
    _renderer.DrawLine(ToScreen_({ feet.x, feet.y - 4.f }, _cameraPos), ToScreen_({ feet.x, feet.y + 4.f }, _cameraPos), { 255, 255, 0, 255 });

    const Aabb trigger = MakeKnifeTriggerBox(feet, m_player.GetFacing(), body.GetWidth() * 0.5f, m_tuning.knife);
    DrawBox_(_renderer, trigger, _cameraPos, m_player.IsKnifeTargetInRange() ? slug::RGBA { 255, 220, 40, 255 } : slug::RGBA { 120, 110, 40, 255 });

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

    // StageState::BuildHudModel 과 같은 규칙: 보스가 살아 있으면 보스 HP 바를 표시한다.
    HudModel hud = MakeHudModel(p);
    if (const std::optional<float> ratio = m_enemies.GetBossHpRatio())
    {
        hud.bShowBoss   = true;
        hud.bossHpRatio = *ratio;
    }
    m_ui.DrawHud(_ctx, hud);

    if (_ctx.bDebugDraw)
    {
        const Vec2 vel   = p.GetVelocity();
        const Vec2 pos   = p.GetFeet();
        const slug::String  debug = std::format(
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
}

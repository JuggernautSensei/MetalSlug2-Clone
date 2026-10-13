#include "Game/Stage/StageState.h"

#include "Core/AssetPath.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>

#include "Audio/AudioSystem.h"
#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Combat/BulletVisual.h"
#include "Game/Enemies/BigShieeEnemy.h"
#include "Game/Player/PlayerHitboxes.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Stage/StageMapping.h"
#include "Game/StateFactory.h"
#include "Game/Ui/Hud.h"
#include "World/Constants.h"

constexpr int   kAutoTowerDamage    = 24;
constexpr float kAutoTowerMarginPx  = 120.f;
constexpr int   kAutoTurretInterval = 12;

namespace
{
[[nodiscard]] RGBA GetPlaceholderColor_(
    const PlaceholderEntity& _entity)
{
    if (IsHitFlashOn(_entity.flashTicks))
    {
        return { 255, 150, 150, 255 };
    }
    if (!_entity.bHittable)
    {
        return { 150, 150, 170, 255 };
    }
    return _entity.kind == eEntityKind::Npc ? RGBA { 120, 170, 230, 255 } : RGBA { 96, 120, 70, 255 };
}

[[nodiscard]] String JoinIds_(
    const Vector<String>& _ids)
{
    String joined = {};
    for (const String& id: _ids)
    {
        if (!joined.empty())
        {
            joined += ",";
        }
        joined += id;
    }
    return joined;
}
}   // namespace

StageState::StageState() = default;

StageState::StageState(
    const StageStateConfig& _config)
    : m_config(_config)
{
}

void StageState::OnEnter()
{
    Load_();
    if (!m_bLoaded)
    {
        return;
    }

    m_ui.Init();
    Restart_();
    m_bTakenNext = false;
}

void StageState::OnExit()
{
    AudioSystem::GetSingleton().AllStop();
}

const InputHistory& StageState::MakeBotInput_()
{
    InputFrame    frame = {};
    const VECTOR2 feet  = m_player.GetFeet();

    bool  bTarget = false;
    float bestDx  = 0.f;
    for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
    {
        if (hurtbox.team != eTeam::Enemy || !IsEnemyOwner(hurtbox.owner) || GetPartIndex(hurtbox.owner) < 0)
        {
            continue;
        }
        const float dx = hurtbox.box.GetCenter().x - feet.x;
        if (!bTarget || Abs(dx) < Abs(bestDx))
        {
            bTarget = true;
            bestDx  = dx;
        }
    }

    if (!bTarget)
    {
        frame.Set(eAction::Right, true);
    }
    else if (Abs(bestDx) > 14.f)
    {
        frame.Set(bestDx > 0.f ? eAction::Right : eAction::Left, true);
        frame.Set(eAction::Up, true);
    }
    else
    {
        frame.Set(eAction::Up, true);
        frame.Set(eAction::Shot, (m_tickCount % 10) < 3);
    }
    m_botInput.Push(frame);
    return m_botInput;
}

void StageState::Load_()
{
    m_bLoaded   = false;
    m_bFailed   = false;
    m_assetRoot = FindAssetRoot();

    String            loadMessage;
    Result<StageData> data = LoadStageFromFile(FindAssetRoot() / "Stage" / m_config.stageFile, &loadMessage);
    if (!data)
    {
        SLUG_LOG_ERROR("{} 을 읽지 못했습니다: {}", m_config.stageFile, loadMessage);
        m_bFailed = true;
        return;
    }
    m_data = data.Take();

    // 소품 컬링이 스프라이트 크기를
    if (Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(FindAssetRoot()))
    {
        m_spriteLib = library.Take();
    }
    m_props.SetSpriteLibrary(&m_spriteLib);
    {

        const FilePath   bgPath = FindAssetRoot() / "Textures" / (m_data.background + ".png");
        Vector<FilePath> paths  = { bgPath };
        Vector<size_t>   owners = {};
        for (size_t i = 0; i < m_data.backdrops.size(); ++i)
        {
            const SpriteInfo* pInfo = m_spriteLib.Find(m_data.backdrops[i].sprite);
            if (pInfo && pInfo->bStandalone)
            {
                paths.push_back(FindAssetRoot() / FilePath { pInfo->file });
                owners.push_back(i);
            }
        }

        const Vector<int> handles = SpriteBatch::GetSingleton().LoadStandaloneTexturesParallel(paths, g_debug.numLoadWorkers);
        m_bgHandle                = handles[0];
        if (m_bgHandle < 0)
        {
            SLUG_LOG_WARN("원경 '{}' 을 읽지 못했습니다", bgPath.string());
        }
        m_backdropHandles.assign(m_data.backdrops.size(), -1);
        for (size_t k = 0; k < owners.size(); ++k)
        {
            m_backdropHandles[owners[k]] = handles[k + 1];
            if (handles[k + 1] < 0)
            {
                SLUG_LOG_WARN("원경 스프라이트 '{}' 을 읽지 못했습니다", m_data.backdrops[owners[k]].sprite);
            }
        }
    }

    m_host               = StageWorldHost {};
    const int flatWidth  = static_cast<int>(m_data.worldSize.x);
    const int flatHeight = static_cast<int>(m_data.worldSize.y);
    for (const String& name: m_data.groundMaps)
    {
        const FilePath path = FindAssetRoot() / "Ground" / (name + ".png");

        if (std::optional<GroundMap> map = LoadGroundMask(path))
        {
            GroundMap ground = std::move(*map);
            if (m_data.endless.bEnabled)
            {
                ground.SetPeriodicTail(static_cast<int>(m_data.endless.fromX), static_cast<int>(m_data.endless.tileX), static_cast<int>(m_data.endless.tileW));
                ground.MaterializeTail(3);
            }
            m_host.AddGroundMap(name, std::move(ground));
        }
        else
        {
            SLUG_LOG_WARN("지면 '{}' 을 읽지 못해 평지를 사용합니다", name);
            GroundMap flat = GroundMap::MakeEmpty(flatWidth, flatHeight);
            flat.FillRect(0, 378, flatWidth - 1, flatHeight - 1, true);
            m_host.AddGroundMap(name, std::move(flat));
        }
    }

    m_tuning = {};
    if (Result<Tuning> loaded = LoadTuning(GetDefaultTuningPath(FindAssetRoot())))
    {
        m_tuning = loaded.Take();
        if (g_debug.lives > 0)
        {
            m_tuning.player.lives = g_debug.lives;
        }
    }
    else
    {
        SLUG_LOG_WARN("Tuning.json 을 읽지 못해 기본값을 사용합니다");
    }

    {

        String         message  = {};
        const FilePath defsPath = EnemyDefLibrary::GetDefaultDirectory(FindAssetRoot());
        m_enemyLib              = {};
        if (Result<EnemyDefLibrary> lib = std::filesystem::is_directory(defsPath) ? EnemyDefLibrary::LoadDirectory(defsPath, &message) : EnemyDefLibrary::Load(defsPath, &message))
        {
            m_enemyLib = lib.Take();
        }
        else
        {
            SLUG_LOG_WARN("적 정의를 읽지 못해 플레이스홀더로 배치합니다: {}", message);
        }
        m_enemies.SetLibrary(&m_enemyLib);
    }
    {

        String message = {};
        if (Result<PropDefLibrary> lib = PropDefLibrary::Load(FindAssetRoot() / "Defs" / "Props" / "Props.json", &message))
        {
            m_propDefs = lib.Take();
        }
        else
        {
            SLUG_LOG_WARN("소품 정의를 읽지 못해 플레이스홀더로 배치합니다: {}", message);
        }
    }

    m_host.BindPlayer(&m_player, &m_tuning);
    m_host.BindUi(&m_ui);
    m_host.BindWorld(&m_enemies, &m_props);
    m_host.BindRects(m_data.rects);

    m_camera = Camera { MakeCameraTuning(m_tuning.camera) };
    m_camera.SetForwardOnly(true);
    m_runner.emplace(m_data, m_host, m_camera, m_rng);

    m_bRigBound                   = false;
    const AnimationLibrary& anims = AnimationLibrary::GetSingleton();
    if (anims.GetClipCount() > 0)
    {
        (void)m_rig.Bind(anims, m_tuning.rig);
        m_bRigBound = true;
        m_bombClip  = anims.Find("PlayerBomb");
        m_bombAnim.Play(m_bombClip, true);
        m_chuteClip = anims.Find("Bazuka_Para_Idle");
        m_chuteAnim.Play(m_chuteClip, true);
    }

    m_enemies.SetSpriteLibrary(&m_spriteLib);

    m_bLoaded = true;
}

void StageState::Restart_()
{
    m_host.Reset();
    m_enemies.Reset(1);
    m_props.Reset(1);
    m_combat.GetResolver().Clear();
    m_combat.BeginTick();
    m_projectiles.Clear();
    m_effects.Clear();
    m_patchMasks.clear();
    m_rng = RngMt19937 { 0x57A6E0ull };
    m_flow.Reset();
    m_endKind = eEndKind::None;
    m_timer.Reset();
    m_timerSectionId.clear();
    m_rescuedPows = 0;

    m_runner->Begin();

    m_startFeet = m_runner->ResolvePlayerStart().value_or(VECTOR2 { m_data.player.x, 0.f });
    UpdateWalls_();
    m_player.Reset(m_tuning, m_startFeet);
    m_player.SnapToGround(MakePhysicsWorld_(), 16.f);
    m_player.BeginChuteDescent(m_tuning);

    m_camera.SetTarget(m_runner->ComputeCameraTarget(m_player.GetLogicalFeet()));
    m_camera.SnapToTarget();
    m_prevCameraPos = m_camera.GetPosition();

    m_ui.GetBanner().Clear();
    m_ui.GetFade().SetClear();
    m_prevStats  = m_player.GetStats();
    m_prevWeapon = m_player.GetWeapon().weapon;
    m_prevScore  = m_player.GetScore();
}

void StageState::Tick(
    const InputHistory& _input)
{
    ++m_tickCount;
    if (!m_bLoaded)
    {
        return;
    }

    if (IsFlowing_())
    {
        m_ui.Tick();
        m_flow.Tick(m_ui, AudioSystem::GetSingleton());
        return;
    }
    if (m_runner->IsFinished())
    {
        return;
    }

    HandleDebugWarp_();

    m_combat.BeginTick();
    {
        PropTickContext propCtx = {};
        propCtx.pGround         = m_host.GetActiveGround();
        propCtx.pAnims          = &AnimationLibrary::GetSingleton();
        propCtx.cameraLeft      = m_camera.GetPosition().x;
        propCtx.cameraBottom    = m_camera.GetPosition().y + Renderer::kLogicalHeight;
        propCtx.pItemTuning     = &m_tuning.items;
        m_props.Tick(propCtx);
    }
    UpdateWalls_();

    m_host.RegisterPlaceholderHurtboxes(m_combat);
    m_enemies.RegisterHurtboxes(m_combat);
    m_props.RegisterHurtboxes(m_combat);

    PlayerContext playerCtx = {};
    playerCtx.pTuning       = &m_tuning;
    playerCtx.physics       = MakePhysicsWorld_();
    playerCtx.pCombat       = &m_combat;
    playerCtx.pProjectiles  = &m_projectiles;
    playerCtx.respawnFeet   = GetRespawnFeet_();
    playerCtx.godMode       = g_debug.godMode;
    playerCtx.bInfiniteAmmo = g_debug.bInfiniteAmmo;
    if (!m_bLabSetUp)
    {
        m_bLabSetUp = true;
        if (m_config.bHmg)
        {
            m_player.PickUpWeapon(eWeapon::Hmg, m_tuning);
        }
        if (m_config.bGod)
        {
            m_player.ExtendInvincibility(60 * 60 * 60);
        }
    }
    m_player.Tick(playerCtx, m_config.bBot ? MakeBotInput_() : _input);

    for (const ItemPickup& pickup: m_props.GetItems().Collect(m_player.GetBodyBox()))
    {
        ApplyItemPickup(m_player, pickup, m_tuning);
        switch (pickup.kind)
        {
            case eItemKind::Gun: AudioSystem::GetSingleton().Play(kPowerUp); break;
            case eItemKind::Supply: AudioSystem::GetSingleton().Play(kOk); break;
            case eItemKind::Score: AudioSystem::GetSingleton().Play(kPickupCoin); break;
        }
    }

    EnemyContext enemyCtx         = {};
    enemyCtx.pTuning              = &m_tuning;
    enemyCtx.physics              = MakeEnemyPhysicsWorld_();
    enemyCtx.pCombat              = &m_combat;
    enemyCtx.pProjectiles         = &m_projectiles;
    enemyCtx.player.feet          = m_player.GetFeet();
    enemyCtx.player.bodyBox       = m_player.GetBodyBox();
    enemyCtx.player.center        = m_player.GetBodyBox().GetCenter();
    enemyCtx.player.bAlive        = m_player.IsAlive();
    enemyCtx.player.bInvulnerable = m_player.GetInvincibleTicks() > 0;
    enemyCtx.cameraView           = Aabb::FromMinSize(m_camera.GetPosition(), { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(Renderer::kLogicalHeight) });
    enemyCtx.pAnims               = &AnimationLibrary::GetSingleton();
    m_enemies.Tick(enemyCtx);
    m_runner->AddCameraScroll(m_enemies.TakeCameraScroll());

    if (m_config.bAutoDamage && m_tickCount % 6 == 0)
    {
        Vector<HitEvent> auto_;
        for (const Scoped<Enemy>& pEnemy: m_enemies.GetEnemies())
        {
            const EnemyState& enemy = pEnemy->GetState();
            if (enemy.life != eEnemyLife::Alive || enemy.behavior != eEnemyBehavior::BigShiee)
            {
                continue;
            }
            const BigShieeData& boss = static_cast<const BigShieeEnemy&>(*pEnemy).GetBossData();
            int                 part = kBigShieeCannonPart;
            for (int i = 0; i < kBigShieeTurretCount; ++i)
            {
                if (boss.turretHp[static_cast<size_t>(i)] > 0)
                {
                    part = i;
                    break;
                }
            }
            if (part != kBigShieeCannonPart && m_tickCount % kAutoTurretInterval != 0)
            {
                continue;
            }
            HitEvent event;
            event.damage       = 2;
            event.kind         = eHitKind::Bullet;
            event.attackerTeam = eTeam::Player;
            event.attacker     = 1;
            event.target       = MakePartOwner(enemy.id, static_cast<uint32_t>(part));
            auto_.push_back(event);
        }
        if (!auto_.empty())
        {
            m_enemies.OnHitEvents(Span<const HitEvent> { auto_.data(), auto_.size() });
        }
    }

    if (m_config.bAutoDamage)
    {
        const int        towerDef = m_props.GetLibrary()->FindIndex("tower");
        const float      right    = m_camera.GetPosition().x + static_cast<float>(Renderer::kLogicalWidth) - kAutoTowerMarginPx;
        Vector<HitEvent> towerHits;
        for (const PropInstance& prop: m_props.GetProps())
        {
            if (prop.defIndex != towerDef || prop.bDestroyed || prop.pos.x > right || prop.pos.x < m_camera.GetPosition().x)
            {
                continue;
            }
            HitEvent event;
            event.damage       = kAutoTowerDamage;
            event.kind         = eHitKind::Blast;
            event.attackerTeam = eTeam::Player;
            event.attacker     = 1;
            event.target       = prop.id;
            towerHits.push_back(event);
        }
        if (!towerHits.empty())
        {
            m_props.OnHitEvents(Span<const HitEvent> { towerHits.data(), towerHits.size() });
        }
    }

    m_projectiles.Step(m_host.GetActiveGround(), m_combat, kStepSec, m_enemyPlatforms);

    const Vector<HitEvent>& events = m_combat.Resolve();
    m_props.OnHitEvents(events);
    m_enemies.OnHitEvents(events);
    const int kills = m_host.ApplyPlaceholderEvents(events);
    if (kills > 0)
    {
        AudioSystem::GetSingleton().Play(kEnemyDeath, 0.7f, 0.1f);
    }
    m_player.AddScore(kills * kPlaceholderKillScore);
    m_player.OnHitEvents(events);
    m_projectiles.OnHitEvents(events, m_combat);
    m_host.TickPlaceholders();

    ConsumeEnemyEvents_();
    ConsumePropEvents_();

    const Vector<FxRequest> fx = m_projectiles.TakeFx();
    RequestSounds_(fx);
    m_ui.Tick();
    const AnimationLibrary& anims = AnimationLibrary::GetSingleton();
    for (const FxRequest& request: fx)
    {
        if (request.kind == eFxKind::BombExplosion)
        {
            m_effects.Spawn(anims, "PlayerBombExplosion", request.position, kBombBlastDepth + request.zOffset);
        }
        else if (request.kind == eFxKind::RocketExplosion)
        {
            m_effects.Spawn(anims, "Explosion_Mid", request.position, 56 + request.zOffset);
        }
        else if (request.kind == eFxKind::RocketSmoke)
        {
            m_effects.Spawn(anims, "Rocket_Smoke", request.position, 28 + request.zOffset);
        }
        else
        {
            m_effects.Spawn(anims, "Bullet_Destroy_Effect", request.position, kBulletHitDepth + request.zOffset);
        }
    }
    m_effects.Update(anims, kStepSec);
    m_bombAnim.Update(anims, kStepSec);
    m_chuteAnim.Update(anims, kStepSec);
    if (m_bRigBound)
    {
        m_rig.Update(m_player.MakeRigState());
    }

    m_prevCameraPos = m_camera.GetPosition();
    m_runner->Tick();
    if (m_host.GetLastSectionId() != m_timerSectionId)
    {
        m_timerSectionId = m_host.GetLastSectionId();
        m_timer.Reset();
    }
    m_timer.Tick();

    if (m_endKind == eEndKind::None)
    {
        if (m_host.IsEndRequested())
        {
            StartFlow_(eEndKind::Mission);
        }
        else if (m_player.IsGameOver())
        {
            StartFlow_(eEndKind::GameOver);
        }
    }
}

void StageState::StartFlow_(
    const eEndKind _kind)
{
    m_endKind = _kind;
    if (_kind == eEndKind::Mission)
    {
        m_player.AddScore(MissionBonusScore(m_rescuedPows));
    }
    const ScreenFlowConfig config = _kind == eEndKind::Mission ? MakeMissionCompleteFlowConfig(m_rescuedPows) : MakeGameOverFlowConfig();
    m_flow.Start(config, m_ui, AudioSystem::GetSingleton());
}

void StageState::HandleDebugWarp_()
{
    if (g_debug.weapon > 0)
    {
        if (g_debug.bDebugDraw)
        {
            m_player.PickUpWeapon(WeaponFromDebugSlot(g_debug.weapon), m_tuning);
        }
        g_debug.weapon = 0;
    }

    const int request = g_debug.warp;
    g_debug.warp      = 0;
    if (request == 0 || (!g_debug.bDebugDraw && request != 3))
    {
        return;
    }

    if (request == 1)
    {
        (void)m_runner->WarpToSection(m_runner->GetNextSectionIndex());
    }
    else if (request == 2)
    {
        (void)m_runner->WarpToBoss();
    }
    else if (request == 3)
    {
        (void)m_runner->WarpToX(g_debug.warpX);
    }
}

bool StageState::IsFlowing_() const
{
    return m_endKind != eEndKind::None && !m_flow.IsDone();
}

void StageState::UpdateWalls_()
{
    const ScreenWalls screen = m_runner->MakeScreenWalls();
    m_walls.clear();
    m_enemyWalls.clear();
    m_playerPlatforms.clear();
    m_enemyPlatforms.clear();
    if (screen.bHasLeft)
    {
        m_walls.push_back({ screen.left });
    }
    if (screen.bHasRight)
    {
        m_walls.push_back({ screen.right });
    }

    for (const Wall& wall: m_props.GetWalls())
    {
        m_walls.push_back(wall);
        m_enemyWalls.push_back(wall);
    }
    for (const Platform& platform: m_props.GetPlatforms())
    {
        m_playerPlatforms.push_back(platform);
        m_enemyPlatforms.push_back(platform);
    }
    for (const Platform& platform: m_props.GetEnemyPlatforms())
    {
        m_enemyPlatforms.push_back(platform);
    }

    for (const StageCollider& collider: m_host.GetColliders())
    {
        if (collider.layer == eCollisionLayer::Wall)
        {
            m_walls.push_back({ collider.box });
            m_enemyWalls.push_back({ collider.box });
        }
        else
        {
            m_enemyPlatforms.push_back({ collider.box, true });
        }
    }

    for (const Aabb& box: m_enemies.GetSolidBoxes())
    {
        m_walls.push_back({ box });
        m_enemyWalls.push_back({ box });
    }
}

PhysicsWorld StageState::MakePhysicsWorld_() const
{
    PhysicsWorld world = {};
    world.pGround      = m_host.GetActiveGround();
    world.walls        = m_walls;
    world.platforms    = m_playerPlatforms;
    return world;
}

PhysicsWorld StageState::MakeEnemyPhysicsWorld_() const
{
    PhysicsWorld world = {};
    world.pGround      = m_host.GetActiveGround();
    world.walls        = m_enemyWalls;
    world.platforms    = m_enemyPlatforms;
    return world;
}

VECTOR2 StageState::GetRespawnFeet_() const
{
    const VECTOR2 cam = m_camera.GetPosition();
    return { cam.x + m_tuning.player.respawnOffsetX, cam.y - m_tuning.player.respawnDropY };
}

void StageState::ConsumeEnemyEvents_()
{
    AudioSystem& audio = AudioSystem::GetSingleton();
    for (const String& sound: m_enemies.TakeSounds())
    {
        audio.Play(sound, 1.f, 0.03f);
    }
    for (const EnemyEvent& event: m_enemies.TakeEvents())
    {
        switch (event.kind)
        {
            case eEnemyEventKind::Died:
                m_player.AddScore(event.score);
                audio.Play(kEnemyDeath, 0.7f, 0.1f);
                if (!event.drop.empty())
                {
                    (void)m_props.SpawnItem(event.drop, event.pos, false);
                }
                break;
            case eEnemyEventKind::PartDestroyed:
                m_player.AddScore(event.score);
                audio.Play(kEnemyDeath, 0.5f, 0.1f);
                break;
            case eEnemyEventKind::ItemDropped:
                (void)m_props.SpawnItem(event.drop, event.pos, true);
                break;
            case eEnemyEventKind::Freed:
                ++m_rescuedPows;
                break;
            case eEnemyEventKind::Transformed:
            case eEnemyEventKind::Removed:
                break;
        }
    }
}

void StageState::ConsumePropEvents_()
{
    AudioSystem& audio = AudioSystem::GetSingleton();
    for (const PropEvent& event: m_props.TakeEvents())
    {
        switch (event.kind)
        {
            case ePropEventKind::Destroyed:
                audio.Play(kExplosion, 0.9f, 0.05f);
                break;
            case ePropEventKind::SecondaryBurst:
                audio.Play(kBombExplode, 1.f, 0.03f);
                break;
            case ePropEventKind::GroundPatch:
                ApplyGroundPatch_(event.name, event.pos);
                break;
            case ePropEventKind::Damaged:
            case ePropEventKind::DropSpawned:
            case ePropEventKind::Removed:
            case ePropEventKind::Culled:
                break;
        }
    }
}

const GroundMap* StageState::FindPatchMask_(
    const String& _sprite)
{
    for (const PatchMask& patch: m_patchMasks)
    {
        if (patch.name == _sprite)
        {
            return patch.bLoaded ? &patch.mask : nullptr;
        }
    }

    PatchMask patch     = {};
    patch.name          = _sprite;
    const FilePath path = FindAssetRoot() / "Textures" / (_sprite + ".png");
    if (std::optional<GroundMap> mask = LoadGroundMask(path))
    {
        if (mask->CountSolidPixels() == 0)
        {
            // 빈 발자국은 아무것도 지우지
            SLUG_LOG_ERROR("지면 패치 마스크 '{}' 에 발자국 픽셀이 없습니다 (무시)", path.string());
        }
        else
        {
            patch.mask    = std::move(*mask);
            patch.bLoaded = true;
        }
    }
    else
    {
        SLUG_LOG_WARN("지면 패치 마스크 '{}' 을 읽지 못했습니다", path.string());
    }
    m_patchMasks.push_back(std::move(patch));
    return m_patchMasks.back().bLoaded ? &m_patchMasks.back().mask : nullptr;
}

void StageState::ApplyGroundPatch_(
    const String& _sprite,
    const VECTOR2 _topLeft)
{
    GroundMap*       pGround = m_host.GetActiveGroundMut();
    const GroundMap* pMask   = FindPatchMask_(_sprite);
    if (pGround && pMask && !ApplyGroundPatch(*pGround, _topLeft, *pMask))
    {
        SLUG_LOG_ERROR("지면 패치 '{}' 가드에 걸려 적용하지 않았습니다 (바닥을 지우는 마스크)", _sprite.c_str());
    }
}

void StageState::RequestSounds_(
    const Vector<FxRequest>& _fx)
{
    AudioSystem&       audio  = AudioSystem::GetSingleton();
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

    m_prevStats  = stats;
    m_prevWeapon = weapon;
    m_prevScore  = m_player.GetScore();
}

uint64_t StageState::GetTickCount() const
{
    return m_tickCount;
}

uint64_t StageState::HashState() const
{
    uint64_t hash = m_player.HashState();
    hash ^= m_host.HashState() * 1099511628211ull;
    if (m_enemies.GetCount() > 0)
    {
        hash ^= m_enemies.HashState() * 1099511628211ull;
    }
    if (m_props.GetCount() > 0)
    {
        hash ^= m_props.HashState() * 1099511628211ull;
    }
    hash ^= static_cast<uint64_t>(m_projectiles.GetBullets().size()) << 8;
    hash ^= static_cast<uint64_t>(m_projectiles.GetBombs().size()) << 16;
    hash ^= m_runner->HashState() * 1099511628211ull;
    hash *= 1099511628211ull;
    return hash;
}

bool StageState::IsFinished() const
{
    if (m_bFailed)
    {
        return true;
    }
    if (!m_bLoaded)
    {
        return false;
    }
    if (m_endKind != eEndKind::None)
    {
        return m_flow.IsDone();
    }
    return m_runner->IsFinished();
}

Scoped<IGameState> StageState::TakeNextState()
{
    if (!IsFinished() || m_bTakenNext)
    {
        return nullptr;
    }
    m_bTakenNext = true;

    if (!m_bFailed && m_endKind == eEndKind::Mission && m_host.IsEndRequested())
    {
        return CreateState(m_host.GetEndStateName());
    }
    return CreateState("title");
}

void StageState::Render(
    const float _alpha)
{
    Renderer& renderer = Renderer::GetSingleton();
    if (!m_bLoaded)
    {
        renderer.BeginFrame({ 0, 0, 0, 255 });
        return;
    }
    renderer.BeginFrame({ 156, 190, 224, 255 });

    const VECTOR2 camNow    = m_camera.GetPosition();
    const VECTOR2 cameraPos = { std::floor(m_prevCameraPos.x + (camNow.x - m_prevCameraPos.x) * _alpha), std::floor(m_prevCameraPos.y + (camNow.y - m_prevCameraPos.y) * _alpha) };

    DrawBackdrop_(cameraPos);
    DrawWorld_(cameraPos, _alpha);
    if (g_debug.bDebugDraw)
    {
        DrawDebugBoxes_(renderer, cameraPos);
    }
    DrawHud_(cameraPos);
}

void StageState::DrawBackdrop_(
    const VECTOR2 _cameraPos)
{
    const BackdropMode* pMode = m_data.FindBackdrop(m_runner->GetBackdropMode());
    if (!pMode)
    {
        return;
    }
    const SpriteInfo* pSprite = m_spriteLib.Find(pMode->sprite);
    if (!pSprite || pSprite->width <= 0)
    {
        return;
    }
    const size_t modeIndex = static_cast<size_t>(pMode - m_data.backdrops.data());
    const int    handle    = modeIndex < m_backdropHandles.size() ? m_backdropHandles[modeIndex] : -1;
    if (pSprite->bStandalone && handle < 0)
    {
        return;
    }

    const float camCenterX = _cameraPos.x + Renderer::kLogicalWidth * 0.5f;
    const float centerX    = Renderer::kLogicalWidth * 0.5f + pMode->offset.x - pMode->parallaxX * camCenterX;
    const float centerY    = Renderer::kLogicalHeight * 0.5f + pMode->offset.y;

    const int pivotX = pSprite->bStandalone ? pSprite->width / 2 : pSprite->frame.pivotX;
    const int pivotY = pSprite->bStandalone ? pSprite->height / 2 : pSprite->frame.pivotY;
    int       left   = static_cast<int>(Floor(centerX)) - pivotX;
    const int top    = static_cast<int>(Floor(centerY)) - pivotY;

    const bool bTile = pMode->period > 0.f;
    const int  tileW = bTile ? static_cast<int>(pMode->period) : pSprite->width;
    if (!bTile && pSprite->width >= Renderer::kLogicalWidth)
    {
        left = Clamp(left, Renderer::kLogicalWidth - pSprite->width, 0);
    }
    int x = bTile ? left % tileW : left;
    if (bTile && x > 0)
    {
        x -= tileW;
    }
    for (; x < Renderer::kLogicalWidth; x += tileW)
    {
        if (x + pSprite->width <= 0)
        {
            continue;
        }
        if (pSprite->bStandalone)
        {
            SpriteBatch::GetSingleton().DrawTexture(handle, { static_cast<float>(x) + _cameraPos.x, static_cast<float>(top) + _cameraPos.y }, _cameraPos, Renderer::kLogicalWidth, Renderer::kLogicalHeight);
        }
        else
        {
            SpriteBatch::GetSingleton().DrawFrame(pSprite->frame, { static_cast<float>(x + pivotX) + _cameraPos.x, static_cast<float>(top + pivotY) + _cameraPos.y }, _cameraPos);
        }
    }
}

void StageState::DrawWorld_(
    const VECTOR2 _cameraPos,
    const float   _alpha)
{
    Renderer&               renderer = Renderer::GetSingleton();
    SpriteBatch&            sprites  = SpriteBatch::GetSingleton();
    const AnimationLibrary& anims    = AnimationLibrary::GetSingleton();

    m_enemies.Submit(sprites, anims, m_projectiles, _alpha);

    sprites.FlushBelowDepth(_cameraPos, m_data.terrainDepth);
    if (m_bgHandle >= 0)
    {
        sprites.DrawTexture(m_bgHandle, { 0.f, 0.f }, _cameraPos, Renderer::kLogicalWidth, Renderer::kLogicalHeight);
        if (m_data.endless.bEnabled)
        {
            sprites.DrawTextureRepeatX(m_bgHandle, { 0.f, 0.f }, _cameraPos, Renderer::kLogicalWidth, Renderer::kLogicalHeight, m_data.endless.fromX, static_cast<int>(m_data.endless.tileX), static_cast<int>(m_data.endless.tileW));
        }
    }

    for (const PlaceholderEntity& entity: m_host.GetPlaceholderEntities())
    {
        if (!g_debug.bDebugDraw)
        {
            continue;
        }
        const Aabb    box   = entity.GetBox();
        const VECTOR2 tl    = ToScreen(box.min, _cameraPos);
        const RGBA    color = entity.IsAlive() ? GetPlaceholderColor_(entity) : RGBA { 90, 90, 90, 255 };
        if (entity.bHittable || !entity.IsAlive())
        {
            renderer.FillRect(tl, entity.size, color);
            renderer.DrawRect(tl, entity.size, { 20, 20, 20, 255 });
            renderer.DrawDebugText({ tl.x + 2.f, tl.y + 2.f }, std::format("{}\n{}/{}", entity.def, Max(0, entity.health.hp), entity.health.maxHp), { 255, 255, 255, 255 });
        }
        else
        {
            renderer.DrawRect(tl, entity.size, { 230, 230, 120, 255 });
            renderer.DrawDebugText({ tl.x + 2.f, tl.y + 2.f }, entity.def, { 255, 255, 120, 255 });
        }
    }

    {
        const VECTOR2 feet = Lerp(m_player.GetPrevFeet(), m_player.GetFeet(), _alpha);
        if (m_player.IsChuteVisible(m_tuning))
        {
            if (const AnimFrame* pFrame = m_chuteAnim.GetFrameData(anims))
            {
                sprites.Submit(ResolveZ(m_player.GetZ(), kPlayerDepth) - 1, static_cast<int>(feet.y), *pFrame, feet + m_tuning.player.chuteOffset);
            }
        }
        if (m_bRigBound)
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
            renderer.FillRect(ToScreen(box.min, _cameraPos), box.GetSize(), { 60, 120, 255, 255 });
        }
    }

    {
        for (const Bullet& bullet: m_projectiles.GetBullets())
        {
            const VECTOR2 pos = Lerp(bullet.renderPrev, bullet.position, _alpha);
            if (bullet.team == eTeam::Player && SubmitPlayerBullet(sprites, &anims, m_spriteLib, bullet, pos, ResolveZ(bullet.z, kPlayerBulletDepth)))
            {
                continue;
            }
            const RGBA color = bullet.team == eTeam::Player ? RGBA { 255, 240, 120, 255 } : RGBA { 255, 80, 60, 255 };
            renderer.FillRect(ToScreen({ pos.x - bullet.size.x * 0.5f, pos.y - bullet.size.y * 0.5f }, _cameraPos), bullet.size, color);
        }

        for (const Bomb& bomb: m_projectiles.GetBombs())
        {
            const VECTOR2 pos = Lerp(bomb.renderPrev, bomb.position, _alpha);
            if (const AnimFrame* pFrame = m_bombAnim.GetFrameData(anims))
            {
                sprites.Submit(ResolveZ(bomb.params.z, kPlayerBombDepth), 0, *pFrame, pos);
                continue;
            }
            renderer.FillRect(ToScreen({ pos.x - 5.f, pos.y - 5.f }, _cameraPos), { 10.f, 10.f }, { 40, 40, 40, 255 });
        }
    }

    m_props.Submit(sprites, anims, m_spriteLib);
    m_effects.Submit(anims, sprites);

    sprites.Flush(_cameraPos);
}

void StageState::DrawDebugBoxes_(
    Renderer&     _renderer,
    const VECTOR2 _cameraPos) const
{
    for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
    {
        DrawBox(
            _renderer,
            hurtbox.box,
            _cameraPos,
            hurtbox.bInvulnerable ? RGBA { 120, 200, 255, 255 } : RGBA { 60, 255, 90, 255 });
    }
    for (const Hitbox& hitbox: m_combat.GetHitboxes())
    {
        DrawBox(
            _renderer,
            hitbox.box,
            _cameraPos,
            { 255, 50, 50, 255 });
    }

    const Aabb body = m_player.GetBodyBox();
    DrawBox(
        _renderer,
        body,
        _cameraPos,
        { 80, 120, 255, 255 });

    const VECTOR2 feet = m_player.GetFeet();
    const float   fh   = m_player.GetBody().footHalfWidth;
    _renderer.DrawLine(ToScreen({ feet.x - fh, feet.y }, _cameraPos), ToScreen({ feet.x + fh, feet.y }, _cameraPos), { 255, 255, 0, 255 });
    _renderer.DrawLine(ToScreen({ feet.x, feet.y - 4.f }, _cameraPos), ToScreen({ feet.x, feet.y + 4.f }, _cameraPos), { 255, 255, 0, 255 });

    const Aabb trigger = MakeKnifeTriggerBox(feet, m_player.GetFacing(), body.GetWidth() * 0.5f, m_tuning.knife);
    DrawBox(
        _renderer,
        trigger,
        _cameraPos,
        m_player.IsKnifeTargetInRange() ? RGBA { 255, 220, 40, 255 } : RGBA { 120, 110, 40, 255 });

    for (const Wall& wall: m_walls)
    {
        DrawBox(
            _renderer,
            wall.box,
            _cameraPos,
            { 200, 80, 200, 255 });
    }
    for (const Platform& platform: m_playerPlatforms)
    {
        DrawBox(
            _renderer,
            platform.box,
            _cameraPos,
            { 255, 160, 60, 255 });
    }
    for (const Platform& platform: m_enemyPlatforms)
    {
        DrawBox(
            _renderer,
            platform.box,
            _cameraPos,
            { 120, 230, 230, 255 });
    }
}

HudModel StageState::BuildHudModel() const
{
    HudModel model     = MakeHudModel(m_player);
    model.bShowTimer   = true;
    model.timerSeconds = m_timer.GetSeconds();
    return model;
}

void StageState::DrawHud_(
    const VECTOR2)
{
    Renderer& renderer = Renderer::GetSingleton();

    m_ui.DrawHud(BuildHudModel());

    if (g_debug.bDebugDraw)
    {
        const String&              lastId = m_host.GetLastSectionId();
        const String               gates  = JoinIds_(m_runner->GetOpenGateIds());
        const VECTOR2              pos    = m_player.GetFeet();
        const VECTOR2              vel    = m_player.GetVelocity();
        const std::optional<float> nextX  = m_runner->GetNextTriggerX();

        const String debug = std::format(
            "STAGE {} SECTION {} (#{}/{})  NEXT #{} x>{}  GATES {} [{}]\n"
            "ENEMY {} (SPAWNED {})  PROP {}  ITEM {}  COLL {}  PH {}  SKIPPED {}  LOCK {}  BG {}\n"
            "POS {:.1f},{:.1f}  VEL {:.0f},{:.0f}  GND {}  GROUND {}  TICK {}  FPS {:.0f}\n"
            "F1 BOX  F2 PAUSE  F3 STEP  F4 SLOW  F5 NEXT SECTION  F6 STAGE END",
            m_data.name,
            lastId.empty() ? String { "-" } : lastId,
            m_host.GetLastSectionIndex(),
            m_data.sections.size(),
            m_runner->GetNextSectionIndex(),
            nextX ? std::format("{:.0f}", *nextX) : String { "-" },
            m_runner->GetOpenGateCount(),
            gates.empty() ? String { "-" } : gates,
            m_enemies.CountAlive(),
            m_runner->GetSpawnedCount(),
            m_props.GetCount(),
            m_props.GetItems().GetCount(),
            m_host.GetColliders().size(),
            m_host.GetPlaceholderEntities().size(),
            m_runner->GetSkippedCount(),
            m_runner->GetLockDepth(),
            m_runner->GetBackdropMode(),
            pos.x,
            pos.y,
            vel.x,
            vel.y,
            m_player.IsGrounded() ? 1 : 0,
            m_host.GetActiveGroundName(),
            m_tickCount,
            g_debug.fps);
        renderer.DrawDebugText({ 9.f, 61.f }, debug, { 0, 0, 0, 255 });
        renderer.DrawDebugText({ 8.f, 60.f }, debug, { 255, 255, 255, 255 });
    }

    if (g_debug.bPaused)
    {
        TextStyle style = {};
        style.align     = eTextAlign::Center;
        m_ui.DrawString(Renderer::kLogicalWidth / 2, 216, "PAUSED", style);
    }

    m_ui.SetMissionResults(m_flow.GetResultsView());
    m_ui.SetGameOverArt(m_flow.GetArtView());
    m_ui.DrawOverlays();
}
bool StageState::IsLoaded() const
{
    return m_bLoaded;
}

const StageData& StageState::GetData() const
{
    return m_data;
}

const StageRunner& StageState::GetRunner() const
{
    return *m_runner;
}

const StageWorldHost& StageState::GetHost() const
{
    return m_host;
}

PlayerController& StageState::GetPlayer()
{
    return m_player;
}

const PlayerController& StageState::GetPlayer() const
{
    return m_player;
}

const Camera& StageState::GetCamera() const
{
    return m_camera;
}

const CombatSystem& StageState::GetCombat() const
{
    return m_combat;
}

const ProjectileSystem& StageState::GetProjectiles() const
{
    return m_projectiles;
}

const EnemyWorld& StageState::GetEnemies() const
{
    return m_enemies;
}

EnemyWorld& StageState::GetEnemies()
{
    return m_enemies;
}

const PropWorld& StageState::GetProps() const
{
    return m_props;
}

PropWorld& StageState::GetProps()
{
    return m_props;
}

uint64_t StageState::DebugHash() const
{
    return HashState();
}

#include "Game/Stage/StageState.h"
#include "Base/Types.h"
#include "Base/Result.h"
#include "Base/Random.h"
#include "Base/RGBA.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>

#include "Base/Log.h"
#include "Base/Math.h"

#include "Audio/AudioSink.h"
#include "Game/Combat/BulletVisual.h"
#include "Game/Player/PlayerHitboxes.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/StateFactory.h"
#include "Game/Stage/StageMapping.h"
#include "Game/Ui/Hud.h"
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

    [[nodiscard]] slug::RGBA GetPlaceholderColor_(
        const PlaceholderEntity& _entity)
    {
        if (_entity.flashTicks > 0)
        {
            return { 255, 150, 150, 255 };
        }
        if (!_entity.bHittable)
        {
            return { 150, 150, 170, 255 };
        }
        return _entity.kind == eEntityKind::Npc ? slug::RGBA { 120, 170, 230, 255 } : slug::RGBA { 96, 120, 70, 255 };
    }

    [[nodiscard]] slug::String JoinIds_(
        const slug::Vector<slug::String>& _ids)
    {
        slug::String joined;
        for (const slug::String& id: _ids)
        {
            if (!joined.empty())
            {
                joined += ",";
            }
            joined += id;
        }
        return joined;
    }
}

StageState::StageState() = default;

void StageState::OnEnter(
    GameContext& _ctx)
{
    Load_(_ctx);
    if (!m_bLoaded)
    {
        return;
    }

    m_ui.Init(_ctx);
    Restart_(_ctx);
    m_bTakenNext = false;
}

void StageState::OnExit(
    GameContext& _ctx)
{
    _ctx.GetAudio().AllStop();
}

void StageState::Load_(
    GameContext& _ctx)
{
    m_bLoaded = false;
    m_bFailed = false;
    m_assetRoot = _ctx.assetRoot;

    StageLoadDiagnostics diag;
    slug::Result<StageData> data = LoadStageFromFile(_ctx.assetRoot / "Stage" / "Stage0.json", &diag);
    if (!data)
    {
        SLUG_LOG_ERROR("Stage0.json 을 읽지 못했습니다: {}", diag.message);
        m_bFailed = true;
        return;
    }
    m_data = data.Take();

    m_host = StageWorldHost {};
    const int flatWidth  = static_cast<int>(m_data.worldSize.x);
    const int flatHeight = static_cast<int>(m_data.worldSize.y);
    for (const slug::String& name: m_data.groundMaps)
    {
        const slug::FilePath path = _ctx.assetRoot / "Baked" / "Ground" / (name + ".bin");
        if (slug::Result<GroundMap> map = GroundMap::Load(path))
        {
            m_host.AddGroundMap(name, map.Take());
        }
        else
        {
            SLUG_LOG_WARN("지면 '{}' 을 읽지 못해 평지를 사용합니다 (Scripts/Bake.ps1 필요)", name);
            GroundMap flat = GroundMap::MakeEmpty(flatWidth, flatHeight);
            flat.FillRect(0, 378, flatWidth - 1, flatHeight - 1, true);
            m_host.AddGroundMap(name, std::move(flat));
        }
    }

    m_tuning = {};
    if (slug::Result<Tuning> loaded = LoadTuning(GetDefaultTuningPath(_ctx.assetRoot)))
    {
        m_tuning = loaded.Take();
    }
    else
    {
        SLUG_LOG_WARN("Tuning.json 을 읽지 못해 기본값을 사용합니다");
    }

    {
        slug::String message;
        const slug::FilePath defsPath = EnemyDefLibrary::GetDefaultDirectory(_ctx.assetRoot);
        m_enemyLib = {};
        if (slug::Result<EnemyDefLibrary> lib = std::filesystem::is_directory(defsPath) ? EnemyDefLibrary::LoadDirectory(defsPath, &message) : EnemyDefLibrary::Load(defsPath, &message))
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
        slug::String message;
        if (slug::Result<PropDefLibrary> lib = PropDefLibrary::Load(_ctx.assetRoot / "Defs" / "Props" / "Props.json", &message))
        {
            m_propDefs = lib.Take();
        }
        else
        {
            SLUG_LOG_WARN("소품 정의를 읽지 못해 플레이스홀더로 배치합니다: {}", message);
        }
        m_props.SetLibrary(&m_propDefs);
    }

    m_host.BindPlayer(&m_player, &m_tuning);
    m_host.BindUi(&m_ui);
    m_host.BindWorld(&m_enemies, &m_props);
    m_host.BindRects(m_data.rects);

    m_camera = Camera { MakeCameraTuning_(m_tuning.camera) };
    m_camera.SetForwardOnly(true);
    m_runner.emplace(m_data, m_host, m_camera, m_rng);

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
        if (_ctx.pSprites)
        {
            const slug::FilePath bgPath = _ctx.assetRoot / "Baked" / "Textures" / (m_data.background + ".png");
            if (const slug::Result<int> handle = _ctx.pSprites->LoadStandaloneTexture(bgPath))
            {
                m_bgHandle = handle.GetValue();
            }
            else
            {
                SLUG_LOG_WARN("원경 '{}' 을 읽지 못했습니다", bgPath.string());
            }

            m_backdropHandles.clear();
            for (const BackdropMode& mode: m_data.backdrops)
            {
                int handle = -1;
                if (const SpriteInfo* pInfo = m_spriteLib.Find(mode.sprite); pInfo && pInfo->bStandalone)
                {
                    if (const slug::Result<int> loaded = _ctx.pSprites->LoadStandaloneTexture(_ctx.assetRoot / "Baked" / slug::FilePath { pInfo->file }))
                    {
                        handle = loaded.GetValue();
                    }
                    else
                    {
                        SLUG_LOG_WARN("원경 스프라이트 '{}' 을 읽지 못했습니다", mode.sprite);
                    }
                }
                m_backdropHandles.push_back(handle);
            }
        }
    }

    m_bLoaded = true;
}

void StageState::Restart_(
    GameContext& _ctx)
{
    m_host.SetContext(&_ctx);
    m_host.Reset();
    m_enemies.Reset(1);
    m_props.Reset(1);
    m_combat.GetResolver().Clear();
    m_combat.BeginTick();
    m_projectiles.Clear();
    m_effects.Clear();
    m_patchMasks.clear();
    m_rng = slug::RngMt19937 { 0x57A6E0ull };
    m_flow.Reset();
    m_endKind = eEndKind::None;

    m_runner->Begin();

    m_startFeet = m_runner->ResolvePlayerStart().value_or(Vec2 { m_data.player.x, 0.f });
    UpdateWalls_();
    m_player.Reset(m_tuning, m_startFeet);
    m_player.SnapToGround(MakePhysicsWorld_(), 16.f);

    m_camera.SetTarget(m_runner->ComputeCameraTarget(m_player.GetFeet()));
    m_camera.SnapToTarget();
    m_prevCameraPos = m_camera.GetPositionNoShake();

    m_ui.GetBanner().Clear();
    m_ui.GetFade().SetClear();
    m_prevStats  = m_player.GetStats();
    m_prevWeapon = m_player.GetWeapon().weapon;
    m_prevScore  = m_player.GetScore();
    m_host.SetContext(nullptr);
}

void StageState::Tick(
    GameContext&        _ctx,
    const InputHistory& _input)
{
    ++m_tickCount;
    if (!m_bLoaded)
    {
        return;
    }

    if (IsFlowing_())
    {
        m_host.SetContext(&_ctx);
        m_ui.Tick();
        m_flow.Tick(m_ui, _ctx.GetAudio());
        m_host.SetContext(nullptr);
        return;
    }
    if (m_runner->IsFinished())
    {
        return;
    }

    HandleDebugWarp_(_ctx);
    m_host.SetContext(&_ctx);

    m_combat.BeginTick();
    {
        PropTickContext propCtx;
        propCtx.pGround    = m_host.GetActiveGround();
        propCtx.pAnims     = _ctx.pAnims;
        propCtx.cameraLeft = m_camera.GetPositionNoShake().x;
        m_props.Tick(propCtx);
    }
    UpdateWalls_();

    m_host.RegisterPlaceholderHurtboxes(m_combat);
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

    for (const ItemPickup& pickup: m_props.GetItems().Collect(m_player.GetBodyBox()))
    {
        ApplyItemPickup(m_player, pickup, m_tuning);
        switch (pickup.kind)
        {
        case eItemKind::Gun:    _ctx.GetAudio().Play(sfx::kPowerUp); break;
        case eItemKind::Supply: _ctx.GetAudio().Play(sfx::kOk); break;
        case eItemKind::Score:  _ctx.GetAudio().Play(sfx::kPickupCoin); break;
        }
    }

    EnemyContext enemyCtx;
    enemyCtx.pTuning            = &m_tuning;
    enemyCtx.physics            = MakeEnemyPhysicsWorld_();
    enemyCtx.pCombat            = &m_combat;
    enemyCtx.pProjectiles       = &m_projectiles;
    enemyCtx.player.feet        = m_player.GetFeet();
    enemyCtx.player.bodyBox     = m_player.GetBodyBox();
    enemyCtx.player.center      = m_player.GetBodyBox().GetCenter();
    enemyCtx.player.bAlive      = m_player.IsAlive();
    enemyCtx.player.bInvulnerable = m_player.GetInvincibleTicks() > 0;
    enemyCtx.cameraView         = Aabb::FromMinSize(m_camera.GetPositionNoShake(), { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(Renderer::kLogicalHeight) });
    enemyCtx.pAnims             = _ctx.pAnims;
    m_enemies.Tick(enemyCtx);

    m_projectiles.Step(m_host.GetActiveGround(), m_combat, kStepSec);

    const slug::Vector<HitEvent>& events = m_combat.Resolve();
    m_props.OnHitEvents(events);
    m_enemies.OnHitEvents(events);
    const int kills = m_host.ApplyPlaceholderEvents(events);
    if (kills > 0)
    {
        _ctx.GetAudio().Play(sfx::kEnemyDeath, 0.7f, 0.1f);
    }
    m_player.AddScore(kills * kPlaceholderKillScore);
    m_player.OnHitEvents(events);
    m_projectiles.OnHitEvents(events, m_combat);
    m_host.TickPlaceholders();

    ConsumeEnemyEvents_(_ctx);
    ConsumePropEvents_(_ctx);

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
        if (m_bRigBound)
        {
            m_rig.Update(m_player.MakeRigState());
        }
    }

    m_prevCameraPos = m_camera.GetPositionNoShake();
    m_runner->Tick();

    if (m_endKind == eEndKind::None)
    {
        if (m_host.IsEndRequested())
        {
            StartFlow_(_ctx, eEndKind::Mission);
        }
        else if (m_player.IsGameOver())
        {
            StartFlow_(_ctx, eEndKind::GameOver);
        }
    }
    m_host.SetContext(nullptr);
}

void StageState::StartFlow_(
    GameContext& _ctx,
    const eEndKind _kind)
{
    m_endKind = _kind;
    const ScreenFlowConfig config = _kind == eEndKind::Mission ? MakeMissionCompleteFlowConfig() : MakeGameOverFlowConfig();
    m_flow.Start(config, m_ui, _ctx.GetAudio());
}

void StageState::HandleDebugWarp_(
    GameContext& _ctx)
{
    if (_ctx.debugWeapon > 0)
    {
        if (_ctx.bDebugDraw)
        {
            m_player.PickUpWeapon(WeaponFromDebugSlot(_ctx.debugWeapon), m_tuning);
        }
        _ctx.debugWeapon = 0;
    }

    const int request = _ctx.debugWarp;
    _ctx.debugWarp    = 0;
    if (request == 0 || !_ctx.bDebugDraw)
    {
        return;
    }

    m_host.SetContext(&_ctx);
    if (request == 1)
    {
        (void)m_runner->WarpToSection(m_runner->GetNextSectionIndex());
    }
    else if (request == 2)
    {
        (void)m_runner->WarpToBoss();
    }
    m_host.SetContext(nullptr);
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
}

PhysicsWorld StageState::MakePhysicsWorld_() const
{
    PhysicsWorld world;
    world.pGround   = m_host.GetActiveGround();
    world.walls     = m_walls;
    world.platforms = m_playerPlatforms;
    return world;
}

PhysicsWorld StageState::MakeEnemyPhysicsWorld_() const
{
    PhysicsWorld world;
    world.pGround   = m_host.GetActiveGround();
    world.walls     = m_enemyWalls;
    world.platforms = m_enemyPlatforms;
    return world;
}

Vec2 StageState::GetRespawnFeet_() const
{
    const Vec2 cam = m_camera.GetPositionNoShake();
    return { cam.x + m_tuning.player.respawnOffsetX, cam.y - m_tuning.player.respawnDropY };
}

void StageState::ConsumeEnemyEvents_(
    GameContext& _ctx)
{
    IAudioSink& audio = _ctx.GetAudio();
    for (const EnemyEvent& event: m_enemies.TakeEvents())
    {
        switch (event.kind)
        {
        case eEnemyEventKind::Died:
            m_player.AddScore(event.score);
            audio.Play(sfx::kEnemyDeath, 0.7f, 0.1f);
            if (!event.drop.empty())
            {
                (void)m_props.SpawnItem(event.drop, event.pos, false);
            }
            break;
        case eEnemyEventKind::ItemDropped:
            (void)m_props.SpawnItem(event.drop, event.pos, true);
            break;
        case eEnemyEventKind::Freed:
        case eEnemyEventKind::Transformed:
        case eEnemyEventKind::Removed:
            break;
        }
    }
}

void StageState::ConsumePropEvents_(
    GameContext& _ctx)
{
    IAudioSink& audio = _ctx.GetAudio();
    for (const PropEvent& event: m_props.TakeEvents())
    {
        switch (event.kind)
        {
        case ePropEventKind::Destroyed:
            audio.Play(sfx::kExplosion, 0.9f, 0.05f);
            break;
        case ePropEventKind::SecondaryBurst:
            audio.Play(sfx::kBombExplode, 1.f, 0.03f);
            break;
        case ePropEventKind::GroundPatch:
            ApplyGroundPatch_(_ctx, event.name, event.pos);
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
    GameContext&       _ctx,
    const slug::String& _sprite)
{
    for (const PatchMask& patch: m_patchMasks)
    {
        if (patch.name == _sprite)
        {
            return patch.bLoaded ? &patch.mask : nullptr;
        }
    }

    PatchMask patch;
    patch.name = _sprite;
    const slug::FilePath path = _ctx.assetRoot / "Baked" / "Textures" / (_sprite + ".png");
    if (std::optional<GroundMap> mask = LoadGroundPatchMask(path))
    {
        patch.mask    = std::move(*mask);
        patch.bLoaded = true;
    }
    else
    {
        SLUG_LOG_WARN("지면 패치 마스크 '{}' 을 읽지 못했습니다", path.string());
    }
    m_patchMasks.push_back(std::move(patch));
    return m_patchMasks.back().bLoaded ? &m_patchMasks.back().mask : nullptr;
}

void StageState::ApplyGroundPatch_(
    GameContext&       _ctx,
    const slug::String& _sprite,
    const Vec2 _topLeft)
{
    GroundMap* pGround = m_host.GetActiveGroundMut();
    const GroundMap* pMask = FindPatchMask_(_ctx, _sprite);
    if (pGround && pMask)
    {
        ApplyGroundPatch(*pGround, _topLeft, *pMask);
    }
}

void StageState::RequestSounds_(
    GameContext&                  _ctx,
    const slug::Vector<FxRequest>& _fx)
{
    IAudioSink&        audio   = _ctx.GetAudio();
    const PlayerStats& stats   = m_player.GetStats();
    const eWeapon      weapon  = m_player.GetWeapon().weapon;

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

    m_prevStats  = stats;
    m_prevWeapon = weapon;
    m_prevScore  = m_player.GetScore();
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

std::unique_ptr<IGameState> StageState::TakeNextState()
{
    if (!IsFinished() || m_bTakenNext)
    {
        return nullptr;
    }
    m_bTakenNext = true;

    if (!m_bFailed && m_endKind == eEndKind::Mission && m_host.IsEndRequested())
    {
        return CreateState(m_host.GetPlaceholders().GetEndStateName());
    }
    return CreateState("title");
}

void StageState::Render(
    GameContext& _ctx,
    const float  _alpha)
{
    Renderer& renderer = *_ctx.pRenderer;
    if (!m_bLoaded)
    {
        renderer.BeginFrame({ 0, 0, 0, 255 });
        return;
    }
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
    DrawHud_(_ctx, cameraPos);
}

void StageState::DrawBackdrop_(
    GameContext&       _ctx,
    const Vec2 _cameraPos)
{
    const BackdropMode* pMode = m_data.FindBackdrop(m_runner->GetBackdropMode());
    if (!pMode || !_ctx.pSprites)
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
    const int left   = static_cast<int>(std::floor(centerX)) - pivotX;
    const int top    = static_cast<int>(std::floor(centerY)) - pivotY;

    const int tileW = pSprite->width;
    int       x     = left % tileW;
    if (x > 0)
    {
        x -= tileW;
    }
    for (; x < Renderer::kLogicalWidth; x += tileW)
    {
        if (pSprite->bStandalone)
        {
            _ctx.pSprites->DrawTexture(handle, { static_cast<float>(x) + _cameraPos.x, static_cast<float>(top) + _cameraPos.y }, _cameraPos, Renderer::kLogicalWidth, Renderer::kLogicalHeight);
        }
        else
        {
            _ctx.pSprites->Submit(0, 0, pSprite->frame, { static_cast<float>(x + pivotX) + _cameraPos.x, static_cast<float>(top + pivotY) + _cameraPos.y });
        }
    }
}

void StageState::DrawWorld_(
    GameContext&       _ctx,
    const Vec2 _cameraPos,
    const float        _alpha)
{
    Renderer& renderer = *_ctx.pRenderer;
    if (!_ctx.pSprites)
    {
        return;
    }
    SpriteBatch& sprites  = *_ctx.pSprites;

    DrawBackdrop_(_ctx, _cameraPos);

    for (const PlaceholderEntity& entity: m_host.GetPlaceholders().GetEntities())
    {
        if (!entity.bHittable && !_ctx.bDebugDraw)
        {
            continue;
        }
        const Aabb box = entity.GetBox();
        const Vec2 tl = ToScreen_(box.min, _cameraPos);
        const slug::RGBA color = entity.IsAlive() ? GetPlaceholderColor_(entity) : slug::RGBA { 90, 90, 90, 255 };
        if (entity.bHittable || !entity.IsAlive())
        {
            renderer.FillRect(tl, entity.size, color);
            renderer.DrawRect(tl, entity.size, { 20, 20, 20, 255 });
            renderer.DrawDebugText({ tl.x + 2.f, tl.y + 2.f }, std::format("{}\n{}/{}", entity.def, std::max(0, entity.health.hp), entity.health.maxHp), { 255, 255, 255, 255 });
        }
        else
        {
            renderer.DrawRect(tl, entity.size, { 230, 230, 120, 255 });
            renderer.DrawDebugText({ tl.x + 2.f, tl.y + 2.f }, entity.def, { 255, 255, 120, 255 });
        }
    }

    const AnimationLibrary* pAnims = _ctx.pAnims;

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

void StageState::DrawDebugBoxes_(
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
    for (const Platform& platform: m_playerPlatforms)
    {
        DrawBox_(_renderer, platform.box, _cameraPos, { 255, 160, 60, 255 });
    }
    for (const Platform& platform: m_enemyPlatforms)
    {
        DrawBox_(_renderer, platform.box, _cameraPos, { 120, 230, 230, 255 });
    }
}

void StageState::DrawHud_(
    GameContext& _ctx,
    const Vec2)
{
    Renderer& renderer = *_ctx.pRenderer;

    m_ui.DrawHud(_ctx, MakeHudModel(m_player));

    if (_ctx.bDebugDraw)
    {
        const StagePlaceholders& placeholders = m_host.GetPlaceholders();
        const slug::String&       lastId       = placeholders.GetLastSectionId();
        const slug::String        gates        = JoinIds_(m_runner->GetOpenGateIds());
        const Vec2       pos          = m_player.GetFeet();
        const Vec2       vel          = m_player.GetVelocity();
        const std::optional<float> nextX      = m_runner->GetNextTriggerX();

        const slug::String debug = std::format(
            "STAGE {} SECTION {} (#{}/{})  NEXT #{} x>{}  GATES {} [{}]\n"
            "ENEMY {} (SPAWNED {})  PROP {}  ITEM {}  COLL {}  PH {}  SKIPPED {}  LOCK {}  BG {}\n"
            "POS {:.1f},{:.1f}  VEL {:.0f},{:.0f}  GND {}  GROUND {}  TICK {}  FPS {:.0f}\n"
            "F1 BOX  F2 PAUSE  F3 STEP  F4 SLOW  F5 NEXT SECTION  F6 BOSS ARENA",
            m_data.name,
            lastId.empty() ? slug::String { "-" } : lastId,
            placeholders.GetLastSectionIndex(),
            m_data.sections.size(),
            m_runner->GetNextSectionIndex(),
            nextX ? std::format("{:.0f}", *nextX) : slug::String { "-" },
            m_runner->GetOpenGateCount(),
            gates.empty() ? slug::String { "-" } : gates,
            m_enemies.CountAlive(),
            m_runner->GetSpawnedCount(),
            m_props.GetCount(),
            m_props.GetItems().GetCount(),
            m_host.GetColliders().size(),
            placeholders.GetEntities().size(),
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

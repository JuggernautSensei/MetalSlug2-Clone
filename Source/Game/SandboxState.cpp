#include "Game/SandboxState.h"

#include <cmath>
#include <cstring>
#include <format>

#include <JugX/CoreLogger.h>
#include <JugX/Math.h>

#include "Game/Player/PlayerHitboxes.h"
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

    [[nodiscard]] jug::VECTOR2 Lerp_(
        const jug::VECTOR2 _a,
        const jug::VECTOR2 _b,
        const float        _t)
    {
        return { _a.x + (_b.x - _a.x) * _t, _a.y + (_b.y - _a.y) * _t };
    }

    [[nodiscard]] jug::VECTOR2 ToScreen_(
        const jug::VECTOR2 _world,
        const jug::VECTOR2 _camera)
    {
        return { std::floor(_world.x - _camera.x), std::floor(_world.y - _camera.y) };
    }

    void DrawBox_(
        Renderer&          _renderer,
        const Aabb&        _box,
        const jug::VECTOR2 _camera,
        const jug::RGBA    _color)
    {
        const jug::VECTOR2 topLeft     = ToScreen_(_box.min, _camera);
        const jug::VECTOR2 bottomRight = ToScreen_(_box.max, _camera);
        _renderer.DrawRect(topLeft, { jug::Max(1.f, bottomRight.x - topLeft.x), jug::Max(1.f, bottomRight.y - topLeft.y) }, _color);
    }

    [[nodiscard]] const char* GetWeaponName_(
        const eWeapon _weapon)
    {
        switch (_weapon)
        {
            case eWeapon::Hmg: return "H.M.GUN";
            case eWeapon::Shotgun: return "SHOTGUN";
            default: return "PISTOL";
        }
    }
}   // namespace

SandboxState::SandboxState() = default;

SandboxState::SandboxState(
    SandboxConfig _config)
    : m_config(std::move(_config))
{
}

// ===========================================
//  월드 구성
// ===========================================

void SandboxState::OnEnter(
    GameContext& _ctx)
{
    BuildWorld_(_ctx);
}

void SandboxState::BuildWorld_(
    GameContext& _ctx)
{
    // --- Tuning ---
    m_tuning = m_config.tuning;
    m_bWatch = false;
    if (m_config.bLoadTuningFile && !_ctx.assetRoot.empty())
    {
        const jug::FilePath path = GetDefaultTuningPath(_ctx.assetRoot);
        if (jug::Result<Tuning> loaded = LoadTuning(path))
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
            JUG_CORE_LOG_WARN("Tuning.json 을 읽지 못해 기본값을 사용합니다: {}", path.string());
        }
    }

    // --- 지면 ---
    if (m_config.bLoadBakedGround)
    {
        bool bLoaded = false;
        if (!_ctx.assetRoot.empty())
        {
            if (jug::Result<GroundMap> ground = GroundMap::Load(_ctx.assetRoot / "Baked" / "Ground" / "Stage0_0.bin"))
            {
                m_ground = ground.Take();
                bLoaded  = true;
            }
        }
        if (!bLoaded)
        {
            JUG_CORE_LOG_WARN("Stage0_0 지면을 읽지 못해 평지를 사용합니다 (Scripts/Bake.ps1 필요)");
            m_ground = GroundMap::MakeEmpty(3520, 448);
            m_ground.FillRect(0, 378, 3519, 447, true);
        }
    }
    else
    {
        m_ground = m_config.ground;
    }

    // --- 카메라 ---
    m_camera = Camera { MakeCameraTuning_(m_tuning.camera) };
    m_camera.SetBounds(m_config.cameraBounds);
    m_camera.SetForwardOnly(true);

    // --- 플레이어 / 표적 ---
    RestartGame();

    // --- 애니메이션 / 렌더 자원 ---
    m_bRigBound = false;
    if (_ctx.pAnims && _ctx.pAnims->GetClipCount() > 0)
    {
        (void)m_rig.Bind(*_ctx.pAnims, m_tuning.rig);   // 없는 클립은 폴백 + 로그
        m_bRigBound = true;
        m_bombClip  = _ctx.pAnims->Find("PlayerBomb");
        m_bombAnim.Play(m_bombClip, true);
    }

    if (!_ctx.IsHeadless())
    {
        if (jug::Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(_ctx.assetRoot / "Baked"))
        {
            m_spriteLib = library.Take();
        }
        if (m_config.bBackground && _ctx.pSprites)
        {
            if (const jug::Result<int> handle = _ctx.pSprites->LoadStandaloneTexture(_ctx.assetRoot / "Baked" / "Textures" / "Stage0_Main.png"))
            {
                m_bgHandle = handle.GetValue();
            }
            else
            {
                JUG_CORE_LOG_WARN("Stage0_Main 배경을 읽지 못했습니다");
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

    m_player.Reset(m_tuning, m_config.spawnFeet);
    m_player.SnapToGround(MakePhysicsWorld_(), 16.f);

    m_camera.SetTarget({ m_config.spawnFeet.x, m_config.spawnFeet.y - 60.f });
    m_camera.SnapToTarget();
    m_prevCameraPos = m_camera.GetPositionNoShake();

    if (m_config.bSpawnTargets)
    {
        AddDefaultTargets_();
    }
}

void SandboxState::AddDefaultTargets_()
{
    const float          groundY = m_config.spawnFeet.y;
    const SandboxTuning& sb      = m_tuning.sandbox;

    // 칼 연습: 가까운 보병 둘 / 차량 / 버스 / 사격 연습: 사수 둘
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
    const jug::VECTOR2 _feet,
    const bool         _bShooter)
{
    return m_targets.Add(_kind, _feet, _bShooter, m_tuning.sandbox);
}

void SandboxState::UpdateWalls_()
{
    // 카메라 왼쪽 가장자리(되돌아갈 수 없음)와 구간 오른쪽 끝을 보이지 않는 벽으로 막는다.
    const float camLeft = m_camera.GetPositionNoShake().x;
    m_walls.clear();
    m_walls.push_back({ Aabb::FromMinMax({ camLeft - 64.f, -4000.f }, { camLeft, 4000.f }) });
    m_walls.push_back({ Aabb::FromMinMax({ m_config.cameraBounds.max.x, -4000.f }, { m_config.cameraBounds.max.x + 64.f, 4000.f }) });
}

PhysicsWorld SandboxState::MakePhysicsWorld_() const
{
    PhysicsWorld world;
    world.pGround   = &m_ground;
    world.walls     = m_walls;
    world.platforms = m_config.platforms;
    return world;
}

jug::VECTOR2 SandboxState::GetRespawnFeet_() const
{
    const jug::VECTOR2 cam = m_camera.GetPositionNoShake();
    return { cam.x + m_tuning.player.respawnOffsetX, cam.y - m_tuning.player.respawnDropY };
}

// ===========================================
//  Tick
// ===========================================

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

    // 1) 판정 모으기 시작: 표적이 먼저 Hurtbox 를 등록해야 플레이어의 칼 자동 발동 질의가 맞는다.
    m_combat.BeginTick();
    UpdateWalls_();
    m_targets.RegisterHurtboxes(m_combat);

    // 2) 플레이어
    PlayerContext playerCtx;
    playerCtx.pTuning      = &m_tuning;
    playerCtx.physics      = MakePhysicsWorld_();
    playerCtx.pCombat      = &m_combat;
    playerCtx.pProjectiles = &m_projectiles;
    playerCtx.respawnFeet  = GetRespawnFeet_();
    m_player.Tick(playerCtx, _input);

    // 3) 사수 → 투사체 이동/판정 등록
    m_targets.TickShooters(m_tuning.sandbox, m_player.GetFeet(), m_player.IsAlive(), m_projectiles, m_combat);
    m_projectiles.Step(&m_ground, m_combat, kStepSec);

    // 4) 판정 해소 → 모든 피해는 HitEvent 로만 적용된다
    const jug::Vector<HitEvent>& events = m_combat.Resolve();
    const int                    score  = m_targets.ApplyEvents(events, m_tuning.sandbox);
    m_player.AddScore(score);
    m_player.OnHitEvents(events);
    m_projectiles.OnHitEvents(events, m_combat);
    m_targets.Tick(m_tuning.sandbox);

    // 5) 연출 / 카메라
    const jug::Vector<FxRequest> fx = m_projectiles.TakeFx();
    if (_ctx.pAnims)
    {
        for (const FxRequest& request: fx)
        {
            if (request.kind == eFxKind::BombExplosion)
            {
                m_effects.Spawn(*_ctx.pAnims, "PlayerBombExplosion", request.position, 60);
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
                // 죽은 원인에 맞는 임시 사망 연출 (칼: 피, 폭발: 날아감, 그 외: 쓰러짐)
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

    // 6) 게임 오버 후 자동 재시작
    if (m_player.IsGameOver() && m_player.GetGameOverTicks() >= m_tuning.player.gameOverTicks)
    {
        RestartGame();
    }
}

uint64_t SandboxState::HashState() const
{
    uint64_t hash = m_player.HashState();
    hash ^= m_targets.HashState() * 1099511628211ull;
    hash ^= static_cast<uint64_t>(m_projectiles.GetBullets().size()) << 8;
    hash ^= static_cast<uint64_t>(m_projectiles.GetBombs().size()) << 16;
    hash *= 1099511628211ull;
    return hash;
}

// ===========================================
//  Render
// ===========================================

void SandboxState::Render(
    GameContext& _ctx,
    const float  _alpha)
{
    Renderer& renderer = *_ctx.pRenderer;
    renderer.BeginFrame({ 156, 190, 224, 255 });

    // 카메라도 보간한다 (플레이어와 같은 위상으로 움직여야 고주사율에서 상대 떨림이 없다)
    const jug::VECTOR2 camNow    = m_camera.GetPositionNoShake();
    const jug::VECTOR2 cameraPos = { std::floor(m_prevCameraPos.x + (camNow.x - m_prevCameraPos.x) * _alpha), std::floor(m_prevCameraPos.y + (camNow.y - m_prevCameraPos.y) * _alpha) };

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
    const jug::VECTOR2 _cameraPos,
    const float        _alpha)
{
    Renderer&               renderer = *_ctx.pRenderer;
    SpriteBatch&            sprites  = *_ctx.pSprites;
    const AnimationLibrary* pAnims   = _ctx.pAnims;

    // --- 표적 ---
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
        const jug::RGBA    color = dummy.flashTicks > 0 ? jug::RGBA { 255, 150, 150, 255 } : (dummy.kind == eDummyKind::Bus ? jug::RGBA { 196, 150, 70, 255 } : jug::RGBA { 96, 120, 70, 255 });
        const jug::VECTOR2 tl    = ToScreen_(box.min, _cameraPos);
        renderer.FillRect(tl, dummy.size, color);
        renderer.DrawRect(tl, dummy.size, { 20, 20, 20, 255 });
        renderer.DrawDebugText({ tl.x + 4.f, tl.y + 4.f }, std::format("{}\n{}/{}", GetDummyName(dummy.kind), dummy.health.hp, dummy.health.maxHp), { 255, 255, 255, 255 });
    }

    // --- 플레이어 ---
    {
        const jug::VECTOR2 feet = Lerp_(m_player.GetPrevFeet(), m_player.GetFeet(), _alpha);
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

    // --- 투사체 ---
    {
        const SpriteInfo* pBulletH = m_spriteLib.Find("Player_Bullet_Basic_H");
        const SpriteInfo* pBulletV = m_spriteLib.Find("Player_Bullet_Basic_V");
        for (const Bullet& bullet: m_projectiles.GetBullets())
        {
            const jug::VECTOR2 pos = Lerp_(bullet.renderPrev, bullet.position, _alpha);
            if (bullet.team == eTeam::Player && pBulletH && pBulletV)
            {
                const bool bVertical = std::abs(bullet.velocity.y) > std::abs(bullet.velocity.x);
                sprites.Submit(30, 0, (bVertical ? pBulletV : pBulletH)->frame, pos);
            }
            else
            {
                const jug::RGBA color = bullet.team == eTeam::Player ? jug::RGBA { 255, 240, 120, 255 } : jug::RGBA { 255, 80, 60, 255 };
                renderer.FillRect(ToScreen_({ pos.x - bullet.size.x * 0.5f, pos.y - bullet.size.y * 0.5f }, _cameraPos), bullet.size, color);
            }
        }

        for (const Bomb& bomb: m_projectiles.GetBombs())
        {
            const jug::VECTOR2 pos = Lerp_(bomb.renderPrev, bomb.position, _alpha);
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
        m_effects.Submit(*pAnims, sprites);
    }

    sprites.Flush(_cameraPos);
}

void SandboxState::DrawDebugBoxes_(
    Renderer&          _renderer,
    const jug::VECTOR2 _cameraPos) const
{
    for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
    {
        DrawBox_(_renderer, hurtbox.box, _cameraPos, hurtbox.bInvulnerable ? jug::RGBA { 120, 200, 255, 255 } : jug::RGBA { 60, 255, 90, 255 });
    }
    for (const Hitbox& hitbox: m_combat.GetHitboxes())
    {
        DrawBox_(_renderer, hitbox.box, _cameraPos, { 255, 50, 50, 255 });
    }

    // 몸통 / 발 센서 / 칼 자동 발동 영역
    const Aabb body = m_player.GetBodyBox();
    DrawBox_(_renderer, body, _cameraPos, { 80, 120, 255, 255 });

    const jug::VECTOR2 feet = m_player.GetFeet();
    const float        fh   = m_player.GetBody().footHalfWidth;
    _renderer.DrawLine(ToScreen_({ feet.x - fh, feet.y }, _cameraPos), ToScreen_({ feet.x + fh, feet.y }, _cameraPos), { 255, 255, 0, 255 });
    _renderer.DrawLine(ToScreen_({ feet.x, feet.y - 4.f }, _cameraPos), ToScreen_({ feet.x, feet.y + 4.f }, _cameraPos), { 255, 255, 0, 255 });

    const Aabb trigger = MakeKnifeTriggerBox(feet, m_player.GetFacing(), body.GetWidth() * 0.5f, m_tuning.knife);
    DrawBox_(_renderer, trigger, _cameraPos, m_player.IsKnifeTargetInRange() ? jug::RGBA { 255, 220, 40, 255 } : jug::RGBA { 120, 110, 40, 255 });

    // 카메라 왼쪽 벽
    for (const Wall& wall: m_walls)
    {
        DrawBox_(_renderer, wall.box, _cameraPos, { 200, 80, 200, 255 });
    }
}

void SandboxState::DrawHud_(
    GameContext& _ctx) const
{
    Renderer&               renderer = *_ctx.pRenderer;
    const PlayerController& p        = m_player;
    const WeaponState&      weapon   = p.GetWeapon();

    const jug::String ammo = weapon.HasFiniteAmmo() ? std::to_string(weapon.ammo) : jug::String { "INF" };
    const jug::String hud  = std::format("LIVES {}  BOMB {}  ARMS {} {}  SCORE {}", p.GetLives(), weapon.bombs, GetWeaponName_(weapon.weapon), ammo, p.GetScore());
    renderer.DrawDebugText({ 9.f, 9.f }, hud, { 0, 0, 0, 255 });
    renderer.DrawDebugText({ 8.f, 8.f }, hud, { 255, 240, 120, 255 });

    const jug::VECTOR2 vel   = p.GetVelocity();
    const jug::VECTOR2 pos   = p.GetFeet();
    const jug::String  debug = std::format(
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
    renderer.DrawDebugText({ 9.f, 33.f }, debug, { 0, 0, 0, 255 });
    renderer.DrawDebugText({ 8.f, 32.f }, debug, { 255, 255, 255, 255 });

    if (p.IsGameOver())
    {
        renderer.DrawDebugText({ 268.f, 200.f }, "GAME OVER", { 255, 60, 60, 255 });
    }
    if (_ctx.bPaused)
    {
        renderer.DrawDebugText({ 282.f, 220.f }, "PAUSED", { 255, 255, 255, 255 });
    }
}

}   // namespace slug

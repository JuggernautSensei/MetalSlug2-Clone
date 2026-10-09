#include "Game/Stage/StageState.h"

#include <algorithm>
#include <cmath>
#include <format>

#include <JugX/CoreLogger.h>
#include <JugX/Math.h>

#include "Game/Player/PlayerHitboxes.h"
#include "Game/StateFactory.h"
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

    // 플레이스홀더 상자 색: 피격 번쩍임 > 종류별 기본색
    [[nodiscard]] jug::RGBA GetPlaceholderColor_(
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
        return _entity.kind == eEntityKind::Npc ? jug::RGBA { 120, 170, 230, 255 } : jug::RGBA { 96, 120, 70, 255 };
    }

    [[nodiscard]] jug::String JoinIds_(
        const jug::Vector<jug::String>& _ids)
    {
        jug::String joined;
        for (const jug::String& id: _ids)
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

// ===========================================
//  수명
// ===========================================

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

    StageLoadDiagnostics diag;
    jug::Result<StageData> data = LoadStageFromFile(_ctx.assetRoot / "Stage" / "Stage0.json", &diag);
    if (!data)
    {
        JUG_CORE_LOG_ERROR("Stage0.json 을 읽지 못했습니다: {}", diag.message);
        m_bFailed = true;
        return;
    }
    m_data = data.Take();

    // --- 지면: 이름 목록의 각 맵을 Baked/Ground 에서 읽는다 (없으면 평지로 대체) ---
    m_host = StagePlaceholders {};
    const int flatWidth  = static_cast<int>(m_data.worldSize.x);
    const int flatHeight = static_cast<int>(m_data.worldSize.y);
    for (const jug::String& name: m_data.groundMaps)
    {
        const jug::FilePath path = _ctx.assetRoot / "Baked" / "Ground" / (name + ".bin");
        if (jug::Result<GroundMap> map = GroundMap::Load(path))
        {
            m_host.AddGroundMap(name, map.Take());
        }
        else
        {
            JUG_CORE_LOG_WARN("지면 '{}' 을 읽지 못해 평지를 사용합니다 (Scripts/Bake.ps1 필요)", name);
            GroundMap flat = GroundMap::MakeEmpty(flatWidth, flatHeight);
            flat.FillRect(0, 378, flatWidth - 1, flatHeight - 1, true);
            m_host.AddGroundMap(name, std::move(flat));
        }
    }

    // --- 튜닝 (Assets/Tuning/Tuning.json, 없으면 기본값) ---
    m_tuning = {};
    if (jug::Result<Tuning> loaded = LoadTuning(GetDefaultTuningPath(_ctx.assetRoot)))
    {
        m_tuning = loaded.Take();
    }
    else
    {
        JUG_CORE_LOG_WARN("Tuning.json 을 읽지 못해 기본값을 사용합니다");
    }

    m_host.BindPlayer(&m_player, &m_tuning);
    m_host.BindUi(&m_ui);

    // --- 카메라 / 러너 ---
    m_camera = Camera { MakeCameraTuning_(m_tuning.camera) };
    m_camera.SetForwardOnly(true);
    m_runner.emplace(m_data, m_host, m_camera, m_rng);

    // --- 애니메이션 / 렌더 자원 ---
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
        if (jug::Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(_ctx.assetRoot / "Baked"))
        {
            m_spriteLib = library.Take();
        }
        if (_ctx.pSprites)
        {
            const jug::FilePath bgPath = _ctx.assetRoot / "Baked" / "Textures" / (m_data.background + ".png");
            if (const jug::Result<int> handle = _ctx.pSprites->LoadStandaloneTexture(bgPath))
            {
                m_bgHandle = handle.GetValue();
            }
            else
            {
                JUG_CORE_LOG_WARN("원경 '{}' 을 읽지 못했습니다", bgPath.string());
            }
        }
    }

    m_bLoaded = true;
}

void StageState::Restart_(
    GameContext& _ctx)
{
    // 1) 전투/연출/호스트 상태 초기화
    m_host.SetContext(&_ctx);
    m_host.Reset();
    m_combat.GetResolver().Clear();
    m_combat.BeginTick();
    m_projectiles.Clear();
    m_effects.Clear();
    m_rng = jug::RngMt19937 { 0x57A6E0ull };

    // 2) 러너 시작: 지면/원경/BGM/경계/섹션 상태 (호스트 SetGround, PlayBgm 이 여기서 일어난다)
    m_runner->Begin();

    // 3) 플레이어: 스크립트 시작 위치 (지면 스냅 실패 시 x 만 쓰고 중력에 맡긴다)
    m_startFeet = m_runner->ResolvePlayerStart().value_or(jug::VECTOR2 { m_data.player.x, 0.f });
    m_player.Reset(m_tuning, m_startFeet);
    m_player.SnapToGround(MakePhysicsWorld_(), 16.f);

    // 4) 카메라: 시작 위치로 즉시 맞춘다
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

// ===========================================
//  Tick
// ===========================================

void StageState::Tick(
    GameContext&        _ctx,
    const InputHistory& _input)
{
    ++m_tickCount;
    if (!m_bLoaded || m_runner->IsFinished())
    {
        return;
    }

    HandleDebugWarp_(_ctx);
    m_host.SetContext(&_ctx);

    // 1) 판정 모으기 시작: 플레이스홀더가 Hurtbox 를 먼저 등록해야 칼 자동 발동 질의가 맞는다.
    m_combat.BeginTick();
    UpdateWalls_();
    m_host.RegisterHurtboxes(m_combat);

    // 2) 플레이어
    PlayerContext playerCtx;
    playerCtx.pTuning      = &m_tuning;
    playerCtx.physics      = MakePhysicsWorld_();
    playerCtx.pCombat      = &m_combat;
    playerCtx.pProjectiles = &m_projectiles;
    playerCtx.respawnFeet  = GetRespawnFeet_();
    m_player.Tick(playerCtx, _input);

    // 3) 투사체 이동/판정 등록 (플레이스홀더는 쏘지 않는다)
    m_projectiles.Step(m_host.GetActiveGround(), m_combat, kStepSec);

    // 4) 판정 해소 → 피해는 HitEvent 로만 적용된다
    const jug::Vector<HitEvent>& events = m_combat.Resolve();
    const int                    kills  = m_host.ApplyEvents(events);
    m_player.AddScore(kills * kPlaceholderKillScore);
    m_player.OnHitEvents(events);
    m_projectiles.OnHitEvents(events, m_combat);
    m_host.Tick();

    // 5) 연출
    const jug::Vector<FxRequest> fx = m_projectiles.TakeFx();
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

    // 6) 스크립트 한 틱 (섹션/예약/경계/게이트 + 카메라 추적). 카메라 보간용으로 이전 위치를 남긴다.
    m_prevCameraPos = m_camera.GetPositionNoShake();
    m_runner->Tick();

    // 7) 게임 오버 후 스테이지 재시작
    if (m_player.IsGameOver() && m_player.GetGameOverTicks() >= m_tuning.player.gameOverTicks)
    {
        Restart_(_ctx);
    }
    m_host.SetContext(nullptr);
}

void StageState::HandleDebugWarp_(
    GameContext& _ctx)
{
    // 요청은 한 번만 소비한다 (같은 프레임의 다음 틱에서 다시 워프하지 않도록)
    const int request = _ctx.debugWarp;
    _ctx.debugWarp    = 0;
    if (request == 0 || !_ctx.bDebugDraw)
    {
        return;
    }

    m_host.SetContext(&_ctx);
    if (request == 1)
    {
        // F5: 아직 발동하지 않은 다음 섹션으로 (silent 실행으로 건너뛴 구간의 소품/경계/지면만 맞춘다)
        (void)m_runner->WarpToSection(m_runner->GetNextSectionIndex());
    }
    else if (request == 2)
    {
        // F6: 보스 아레나 (id "boss" 섹션, 없으면 마지막)
        (void)m_runner->WarpToBoss();
    }
    m_host.SetContext(nullptr);
}

PhysicsWorld StageState::MakePhysicsWorld_() const
{
    PhysicsWorld world;
    world.pGround   = m_host.GetActiveGround();
    world.walls     = m_walls;
    return world;
}

jug::VECTOR2 StageState::GetRespawnFeet_() const
{
    const jug::VECTOR2 cam = m_camera.GetPositionNoShake();
    return { cam.x + m_tuning.player.respawnOffsetX, cam.y - m_tuning.player.respawnDropY };
}

void StageState::UpdateWalls_()
{
    // 왼쪽(되돌아갈 수 없음) 항상, 오른쪽은 락/경계 끝일 때만 (StageRunner::MakeScreenWalls)
    const ScreenWalls screen = m_runner->MakeScreenWalls();
    m_walls.clear();
    if (screen.bHasLeft)
    {
        m_walls.push_back({ screen.left });
    }
    if (screen.bHasRight)
    {
        m_walls.push_back({ screen.right });
    }
}

// 플레이어 소리 요청: 틱의 결과를 읽기만 한다 (시뮬레이션 상태를 바꾸지 않는다)
void StageState::RequestSounds_(
    GameContext&                  _ctx,
    const jug::Vector<FxRequest>& _fx)
{
    IAudioSink&        audio   = _ctx.GetAudio();
    const PlayerStats& stats   = m_player.GetStats();
    const eWeapon      weapon  = m_player.GetWeapon().weapon;

    for (int i = m_prevStats.shotsFired; i < stats.shotsFired; ++i)
    {
        audio.Play(sfx::kPistolShot, weapon == eWeapon::Pistol ? 0.8f : 0.55f, weapon == eWeapon::Pistol ? 0.04f : 0.12f);
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
    }
    if (m_player.GetScore() > m_prevScore)
    {
        audio.Play(sfx::kEnemyDeath, 0.7f, 0.1f);
    }

    m_prevStats  = stats;
    m_prevWeapon = weapon;
    m_prevScore  = m_player.GetScore();
}

uint64_t StageState::HashState() const
{
    uint64_t hash = m_player.HashState();
    hash ^= m_host.HashState() * 1099511628211ull;
    hash ^= static_cast<uint64_t>(m_projectiles.GetBullets().size()) << 8;
    hash ^= static_cast<uint64_t>(m_projectiles.GetBombs().size()) << 16;
    hash ^= m_runner->HashState() * 1099511628211ull;
    hash *= 1099511628211ull;
    return hash;
}

// ===========================================
//  종료
// ===========================================

bool StageState::IsFinished() const
{
    return m_bFailed || (m_bLoaded && m_runner->IsFinished());
}

jug::Scoped<IGameState> StageState::TakeNextState()
{
    if (!IsFinished() || m_bTakenNext)
    {
        return nullptr;
    }
    m_bTakenNext = true;

    if (m_bFailed)
    {
        return CreateState("title");
    }
    return CreateState(m_host.IsEndRequested() ? m_host.GetEndStateName() : jug::String { "title" });
}

// ===========================================
//  Render
// ===========================================

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

    // 카메라도 보간한다 (플레이어와 같은 위상)
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
    DrawHud_(_ctx, cameraPos);
}

void StageState::DrawBackdrop_(
    GameContext&       _ctx,
    const jug::VECTOR2 _cameraPos)
{
    // 화면 좌표의 원경 중심 = (320 + offset.x - parallaxX*camCenterX, 224 + offset.y)  (Docs/StageFormat.md §4)
    const BackdropMode* pMode = m_data.FindBackdrop(m_runner->GetBackdropMode());
    if (!pMode || !_ctx.pSprites)
    {
        return;
    }
    const SpriteInfo* pSprite = m_spriteLib.Find(pMode->sprite);
    if (!pSprite)
    {
        return;
    }

    const float        camCenterX = _cameraPos.x + Renderer::kLogicalWidth * 0.5f;
    const jug::VECTOR2 screenCenter = { Renderer::kLogicalWidth * 0.5f + pMode->offset.x - pMode->parallaxX * camCenterX, Renderer::kLogicalHeight * 0.5f + pMode->offset.y };
    _ctx.pSprites->Submit(0, 0, pSprite->frame, { screenCenter.x + _cameraPos.x, screenCenter.y + _cameraPos.y });
}

void StageState::DrawWorld_(
    GameContext&       _ctx,
    const jug::VECTOR2 _cameraPos,
    const float        _alpha)
{
    Renderer& renderer = *_ctx.pRenderer;
    if (!_ctx.pSprites)
    {
        return;
    }
    SpriteBatch& sprites  = *_ctx.pSprites;

    DrawBackdrop_(_ctx, _cameraPos);

    // --- 플레이스홀더 엔티티: 상자 + def 이름 + 체력 (사망 연출 중에는 회색) ---
    for (const PlaceholderEntity& entity: m_host.GetEntities())
    {
        const Aabb box = entity.GetBox();
        const jug::VECTOR2 tl = ToScreen_(box.min, _cameraPos);
        const jug::RGBA color = entity.IsAlive() ? GetPlaceholderColor_(entity) : jug::RGBA { 90, 90, 90, 255 };
        if (entity.bHittable || !entity.IsAlive())
        {
            renderer.FillRect(tl, entity.size, color);
            renderer.DrawRect(tl, entity.size, { 20, 20, 20, 255 });
            renderer.DrawDebugText({ tl.x + 2.f, tl.y + 2.f }, std::format("{}\n{}/{}", entity.def, std::max(0, entity.health.hp), entity.health.maxHp), { 255, 255, 255, 255 });
        }
        else
        {
            // 판정이 없는 엔티티(소품 장식/아이템/충돌체): 외곽선과 이름만 (배경 스프라이트를 가리지 않는다)
            renderer.DrawRect(tl, entity.size, { 230, 230, 120, 255 });
            renderer.DrawDebugText({ tl.x + 2.f, tl.y + 2.f }, entity.def, { 255, 255, 120, 255 });
        }
    }

    // --- 플레이어 ---
    {
        const jug::VECTOR2 feet = Lerp_(m_player.GetPrevFeet(), m_player.GetFeet(), _alpha);
        if (m_bRigBound && _ctx.pAnims)
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
            if (_ctx.pAnims)
            {
                if (const AnimFrame* pFrame = m_bombAnim.GetFrameData(*_ctx.pAnims))
                {
                    sprites.Submit(31, 0, *pFrame, pos);
                    continue;
                }
            }
            renderer.FillRect(ToScreen_({ pos.x - 5.f, pos.y - 5.f }, _cameraPos), { 10.f, 10.f }, { 40, 40, 40, 255 });
        }
    }

    if (_ctx.pAnims)
    {
        m_effects.Submit(*_ctx.pAnims, sprites);
    }

    sprites.Flush(_cameraPos);
}

void StageState::DrawDebugBoxes_(
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

    const Aabb body = m_player.GetBodyBox();
    DrawBox_(_renderer, body, _cameraPos, { 80, 120, 255, 255 });

    const jug::VECTOR2 feet = m_player.GetFeet();
    const float        fh   = m_player.GetBody().footHalfWidth;
    _renderer.DrawLine(ToScreen_({ feet.x - fh, feet.y }, _cameraPos), ToScreen_({ feet.x + fh, feet.y }, _cameraPos), { 255, 255, 0, 255 });
    _renderer.DrawLine(ToScreen_({ feet.x, feet.y - 4.f }, _cameraPos), ToScreen_({ feet.x, feet.y + 4.f }, _cameraPos), { 255, 255, 0, 255 });

    const Aabb trigger = MakeKnifeTriggerBox(feet, m_player.GetFacing(), body.GetWidth() * 0.5f, m_tuning.knife);
    DrawBox_(_renderer, trigger, _cameraPos, m_player.IsKnifeTargetInRange() ? jug::RGBA { 255, 220, 40, 255 } : jug::RGBA { 120, 110, 40, 255 });

    for (const Wall& wall: m_walls)
    {
        DrawBox_(_renderer, wall.box, _cameraPos, { 200, 80, 200, 255 });
    }
}

void StageState::DrawHud_(
    GameContext& _ctx,
    const jug::VECTOR2 /*_cameraPos*/)
{
    Renderer& renderer = *_ctx.pRenderer;

    m_ui.DrawHud(_ctx, MakeHudModel(m_player));

    // --- 디버그 오버레이: F1 (현재 섹션, 열린 게이트, 워프 키) ---
    if (_ctx.bDebugDraw)
    {
        const jug::String& lastId  = m_host.GetLastSectionId();
        const jug::String  gates   = JoinIds_(m_runner->GetOpenGateIds());
        const jug::VECTOR2 pos     = m_player.GetFeet();
        const jug::VECTOR2 vel     = m_player.GetVelocity();
        const std::optional<float> nextX = m_runner->GetNextTriggerX();

        const jug::String debug = std::format(
            "STAGE {} SECTION {} (#{}/{})  NEXT #{} x>{}  GATES {} [{}]\n"
            "ENT {}  SPAWNED {}  SKIPPED {}  LOCK {}  BOUNDS {:.0f},{:.0f}-{:.0f},{:.0f}  BG {}\n"
            "POS {:.1f},{:.1f}  VEL {:.0f},{:.0f}  GND {}  GROUND {}  TICK {}  FPS {:.0f}\n"
            "F1 BOX  F2 PAUSE  F3 STEP  F4 SLOW  F5 NEXT SECTION  F6 BOSS ARENA",
            m_data.name,
            lastId.empty() ? jug::String { "-" } : lastId,
            m_host.GetLastSectionIndex(),
            m_data.sections.size(),
            m_runner->GetNextSectionIndex(),
            nextX ? std::format("{:.0f}", *nextX) : jug::String { "-" },
            m_runner->GetOpenGateCount(),
            gates.empty() ? jug::String { "-" } : gates,
            m_host.GetEntities().size(),
            m_runner->GetSpawnedCount(),
            m_runner->GetSkippedCount(),
            m_runner->GetLockDepth(),
            m_runner->GetEffectiveBounds().min.x,
            m_runner->GetEffectiveBounds().min.y,
            m_runner->GetEffectiveBounds().max.x,
            m_runner->GetEffectiveBounds().max.y,
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

    // 오버레이(페이드 → 배너)는 항상 마지막
    m_ui.DrawOverlays(_ctx);
}

}   // namespace slug

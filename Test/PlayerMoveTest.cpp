// 플레이어 이동 테스트: 시작 상태, 달리기, 웅크리기, 점프, 조준, 결정성 검증
#include "Base/Types.h"
#include "Game/StateFactory.h"
#include "PlayerTestUtil.h"

TEST(PlayerStart, StartsWithPistolAndTenBombs)
{
    Sim sim;
    EXPECT_EQ(sim.Player().GetWeapon().weapon, eWeapon::Pistol);
    EXPECT_EQ(sim.Player().GetWeapon().bombs, 10);
    EXPECT_FALSE(sim.Player().GetWeapon().HasFiniteAmmo());
}

TEST(PlayerStart, StartsGroundedIdleFacingRightWithThreeLives)
{
    Sim sim;
    sim.Step(2);
    EXPECT_TRUE(sim.Player().IsGrounded());
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Idle);
    EXPECT_EQ(sim.Player().GetFacing(), 1);
    EXPECT_EQ(sim.Player().GetLives(), 3);
    EXPECT_EQ(sim.Player().GetScore(), 0);
    EXPECT_FLOAT_EQ(sim.Y(), 378.f);
}

// 시작 낙하산: 낙하 중 입력 잠금, 공격키로 깜빡이며 소멸 후 조작, 착지 시 자동 소멸
TEST(PlayerChute, DescendsSlowlyFromStartHeightWithoutPhysics)
{
    Sim         sim;
    const float landY = sim.Y();
    sim.Player().BeginChuteDescent(sim.Tune());
    EXPECT_EQ(sim.Player().GetChutePhase(), eChutePhase::Descend);
    EXPECT_FLOAT_EQ(sim.Y(), landY - sim.Tune().player.chuteStartHeightPx);
    EXPECT_FLOAT_EQ(sim.Player().GetLogicalFeet().y, landY);

    sim.Step(60);
    EXPECT_NEAR(sim.Y(), landY - sim.Tune().player.chuteStartHeightPx + sim.Tune().player.chuteFallSpeedPxSec, 0.5f);
    EXPECT_FALSE(sim.Player().IsGrounded());
    EXPECT_EQ(sim.Player().GetChutePhase(), eChutePhase::Descend);
}

TEST(PlayerChute, InputIsLockedUntilAttackPressed)
{
    Sim sim;
    sim.Player().BeginChuteDescent(sim.Tune());
    const float startX = sim.X();

    sim.Hold(eAction::Right);
    sim.Step(30);
    sim.Release(eAction::Right);
    EXPECT_FLOAT_EQ(sim.X(), startX);
    EXPECT_EQ(sim.Player().GetChutePhase(), eChutePhase::Descend);

    sim.Tap(eAction::Shot);
    EXPECT_EQ(sim.Player().GetChutePhase(), eChutePhase::Release);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 0);
}

TEST(PlayerChute, AttackBlinksChuteThenVanishes)
{
    Sim sim;
    sim.Player().BeginChuteDescent(sim.Tune());
    sim.Tap(eAction::Shot);

    const int blinkTicks = sim.Tune().player.chuteBlinkTicks;
    int       toggles    = 0;
    bool      bLastVis   = sim.Player().IsChuteVisible(sim.Tune());
    int       guard      = 0;
    while (sim.Player().GetChutePhase() != eChutePhase::None && guard < blinkTicks + 4)
    {
        sim.Step();
        ++guard;
        const bool bVis = sim.Player().IsChuteVisible(sim.Tune());
        if (bVis != bLastVis)
        {
            ++toggles;
            bLastVis = bVis;
        }
    }
    EXPECT_EQ(sim.Player().GetChutePhase(), eChutePhase::None);
    EXPECT_GE(toggles, 2);
    EXPECT_FALSE(sim.Player().IsChuteVisible(sim.Tune()));
}

TEST(PlayerChute, LandingReleasesChuteAndStartsNormalPlay)
{
    Sim         sim;
    const float landY = sim.Y();
    sim.Player().BeginChuteDescent(sim.Tune());
    sim.Step(400);
    EXPECT_EQ(sim.Player().GetChutePhase(), eChutePhase::None);
    EXPECT_TRUE(sim.Player().IsGrounded());
    EXPECT_NEAR(sim.Y(), landY, 0.5f);
    EXPECT_EQ(sim.Player().GetLives(), 3);
}

TEST(PlayerStart, DefaultStateNameIsSandbox)
{
    EXPECT_EQ(GetDefaultStateName(), "title");
    EXPECT_TRUE(static_cast<bool>(CreateState("sandbox")));
}

TEST(PlayerMove, RightPressMovesOnTheSameTick)
{
    Sim sim;
    sim.Step(5);
    const float before = sim.X();

    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_GT(sim.X(), before);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Run);
}

TEST(PlayerMove, LeftPressMovesAndFlipsFacingOnTheSameTick)
{
    Sim sim;
    sim.Step(5);
    const float before = sim.X();

    sim.Hold(eAction::Left);
    sim.Step();
    EXPECT_LT(sim.X(), before);
    EXPECT_EQ(sim.Player().GetFacing(), -1);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Run);
}

TEST(PlayerMove, RunSpeedMatchesTuning)
{
    Sim sim;
    sim.Step(2);
    const float start = sim.X();
    sim.Hold(eAction::Right);
    sim.Step(60);
    EXPECT_NEAR(sim.X() - start, sim.Tune().move.runSpeed, 1.5f);
}

TEST(PlayerMove, RunSpeedFeelsLikeMetalSlug)
{
    EXPECT_GE(Tuning {}.move.runSpeed, 240.f);
    EXPECT_LE(Tuning {}.move.runSpeed, 260.f);
}

TEST(PlayerMove, StopsInstantlyOnReleaseAndStateReturnsToIdle)
{
    Sim sim;
    sim.Hold(eAction::Right);
    sim.Step(10);
    sim.Release(eAction::Right);
    const float x = sim.X();
    sim.Step();
    EXPECT_FLOAT_EQ(sim.X(), x);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Idle);
    EXPECT_EQ(sim.Player().GetFacing(), 1);   // 방향은 유지
}

TEST(PlayerMove, FacingIsKeptWhenReleased)
{
    Sim sim;
    sim.Hold(eAction::Left);
    sim.Step(3);
    sim.ReleaseAll();
    sim.Step(5);
    EXPECT_EQ(sim.Player().GetFacing(), -1);
}

TEST(PlayerMove, BothDirectionsHeldCancelOut)
{
    Sim sim;
    sim.Step(2);
    const float x = sim.X();
    sim.Hold(eAction::Left);
    sim.Hold(eAction::Right);
    sim.Step(5);
    EXPECT_FLOAT_EQ(sim.X(), x);
}

TEST(PlayerMove, MovementIsIndependentOfRenderRate)
{
    Sim a;
    Sim b;
    a.Hold(eAction::Right);
    b.Hold(eAction::Right);
    a.Step(100);
    b.Step(50);
    b.Step(50);
    EXPECT_FLOAT_EQ(a.X(), b.X());
}

TEST(PlayerMove, HotReloadedTuningChangesSpeedImmediately)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Right);
    sim.Step(10);
    const float x0           = sim.X();
    sim.Tune().move.runSpeed = 60.f;   // 핫리로드와 같은 경로: 같은 Tuning 객체를 덮어쓴다
    sim.Step(10);
    EXPECT_NEAR(sim.X() - x0, 10.f, 0.01f);
}

TEST(PlayerMove, BlockedByInvisibleRightWall)
{
    SandboxConfig config = MakeFlatConfig();
    config.cameraBounds  = Aabb::FromMinMax({ 0.f, 0.f }, { 400.f, 448.f });
    Sim sim { std::move(config) };

    sim.Hold(eAction::Right);
    sim.Step(400);
    EXPECT_LE(sim.X(), 400.f - 10.f + 0.01f);
    EXPECT_FALSE(sim.Player().IsMoving());
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Idle);
}

TEST(PlayerMove, CannotWalkLeftOfCameraEdge)
{
    Sim sim;
    sim.Hold(eAction::Left);
    sim.Step(300);
    EXPECT_GE(sim.X(), 10.f - 0.01f);
}

TEST(PlayerCrouch, DownCrouchesOnTheSameTickAndShrinksBody)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Crouch);
    EXPECT_FLOAT_EQ(sim.Player().GetBody().size.y, sim.Tune().body.crouchSize.y);
    EXPECT_EQ(sim.Player().GetPose(), ePose::Crouch);
}

TEST(PlayerCrouch, CrouchWalkUsesCrouchSpeed)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Down);
    sim.Hold(eAction::Right);
    sim.Step();
    const float x0 = sim.X();
    sim.Step(60);
    EXPECT_NEAR(sim.X() - x0, sim.Tune().move.crouchSpeed, 1.5f);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::CrouchMove);
}

TEST(PlayerCrouch, CrouchWalkDoesNotMoveWhileShotIsHeld)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Down);
    sim.Step(5);
    sim.Hold(eAction::Right);
    sim.Hold(eAction::Shot);
    sim.Step();
    const float x0 = sim.X();
    sim.Step(60);
    EXPECT_NEAR(sim.X(), x0, 0.01f);
    EXPECT_EQ(sim.Player().GetPose(), ePose::Crouch);
}

TEST(PlayerCrouch, CrouchWalkResumesAtCrouchSpeedAfterShotIsReleased)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Down);
    sim.Step(5);
    sim.Hold(eAction::Right);
    sim.Hold(eAction::Shot);
    sim.Step(5);
    sim.Release(eAction::Shot);
    sim.Step(60);   // 사격 연출(shot overlay) 이 끝날 때까지 기다린다
    const float x0 = sim.X();
    sim.Step(60);
    EXPECT_NEAR(sim.X() - x0, sim.Tune().move.crouchSpeed, 1.5f);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::CrouchMove);
}

TEST(PlayerCrouch, StandingUpRestoresBody)
{
    Sim sim;
    sim.Hold(eAction::Down);
    sim.Step(5);
    sim.Release(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Idle);
    EXPECT_FLOAT_EQ(sim.Player().GetBody().size.y, sim.Tune().body.standSize.y);
}

TEST(PlayerCrouch, StaysCrouchedUnderLowCeiling)
{
    GroundMap ground = MakeFlatGround();
    ground.FillRect(0, 300, 1599, 331, true);
    Sim sim { MakeConfig(std::move(ground), { 100.f, 378.f }) };

    sim.Hold(eAction::Down);
    sim.Step(5);
    sim.Release(eAction::Down);
    sim.Step(5);
    EXPECT_EQ(sim.Player().GetPose(), ePose::Crouch);
}

TEST(PlayerAim, UpAimsUp)
{
    Sim sim;
    sim.Hold(eAction::Up);
    sim.Step();
    EXPECT_EQ(sim.Player().GetAim(), eAim::Up);
}

TEST(PlayerAim, UpPlusDirectionAimsStraightUpForNonHmgWeapons)
{
    Sim sim;
    sim.Hold(eAction::Up);
    sim.Hold(eAction::Right);
    const float x0 = sim.X();
    sim.Step();
    EXPECT_EQ(sim.Player().GetAim(), eAim::Up);
    EXPECT_GT(sim.X(), x0);
}

TEST(PlayerAim, UpPlusDirectionAimsStraightUpForHmgWhenNotFiring)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Hold(eAction::Up);
    sim.Hold(eAction::Right);
    const float x0 = sim.X();
    sim.Step(kAimSettleTicks);   // 사격하지 않으면 조준각은 1000도/초로 90도에 금방 붙는다 (45도에 머물지 않는다)
    EXPECT_EQ(sim.Player().GetAim(), eAim::Up);
    EXPECT_GT(sim.X(), x0);
}

TEST(PlayerAim, AimChangeIsInstant)
{
    Sim sim;
    sim.Hold(eAction::Up);
    sim.Step();
    EXPECT_EQ(sim.Player().GetAim(), eAim::Up);
    sim.Release(eAction::Up);
    sim.Step();
    EXPECT_EQ(sim.Player().GetAim(), eAim::Forward);
}

TEST(PlayerAim, DownOnGroundCrouchesInsteadOfAimingDown)
{
    Sim sim;
    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.Player().GetAim(), eAim::Forward);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Crouch);
}

TEST(PlayerAim, DownInAirAimsDown)
{
    Sim sim;
    sim.Tap(eAction::Jump);
    sim.Step(5);
    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_FALSE(sim.Player().IsGrounded());
    EXPECT_EQ(sim.Player().GetAim(), eAim::Down);
    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_FALSE(sim.Player().IsGrounded());
    EXPECT_EQ(sim.Player().GetAim(), eAim::Down);   // 방향키를 같이 눌러도 권총은 똑바로 아래

    // HMG 도 아래 키를 누르면 똑바로 아래 (조준각 -90도, 대각에 머물지 않는다)
    Sim hmg;
    hmg.Player().PickUpWeapon(eWeapon::Hmg, hmg.Tune());
    hmg.Tap(eAction::Jump);
    hmg.Step(5);
    hmg.Hold(eAction::Down);
    hmg.Hold(eAction::Right);
    hmg.Step(kAimSettleTicks);
    EXPECT_FALSE(hmg.Player().IsGrounded());
    EXPECT_EQ(hmg.Player().GetAim(), eAim::Down);
}

TEST(PlayerJump, JumpStartsOnTheSameTickAsThePress)
{
    Sim sim;
    sim.Step(3);
    sim.Hold(eAction::Jump);
    sim.Step();
    EXPECT_LT(sim.Y(), 378.f);   // 눌린 틱에 이미 떠올랐다
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Jump);
    EXPECT_FALSE(sim.Player().IsGrounded());
}

struct JumpMeasure
{
    float apex  = 0.f;
    int   ticks = 0;
};

[[nodiscard]] static JumpMeasure MeasureJump_(
    Sim&       _sim,
    const bool _bHold)
{
    const float groundY = _sim.Y();
    _sim.Hold(eAction::Jump);
    float minY  = groundY;
    int   ticks = 0;
    do
    {
        _sim.Step();
        if (!_bHold)
        {
            _sim.Release(eAction::Jump);
        }
        minY = std::min(minY, _sim.Y());
        ++ticks;
    } while (!_sim.Player().IsGrounded() && ticks < 300);
    return { groundY - minY, ticks };
}

TEST(PlayerJump, HeldJumpApexAndAirTimeFeelRight)
{
    Sim sim;
    sim.Step(2);
    const JumpMeasure held = MeasureJump_(sim, true);
    EXPECT_GE(held.apex, 120.f);
    EXPECT_LE(held.apex, 135.f);
    const float airSec = static_cast<float>(held.ticks) / 60.f;
    EXPECT_GE(airSec, 0.8f);
    EXPECT_LE(airSec, 0.92f);
}

TEST(PlayerJump, TapAndHoldJumpTheSameHeight)
{
    Sim tapSim;
    tapSim.Step(2);
    const JumpMeasure tap = MeasureJump_(tapSim, false);

    Sim heldSim;
    heldSim.Step(2);
    const JumpMeasure held = MeasureJump_(heldSim, true);

    EXPECT_FLOAT_EQ(tap.apex, held.apex);
    EXPECT_EQ(tap.ticks, held.ticks);
}

TEST(PlayerJump, LandsBackOnTheGround)
{
    Sim sim;
    sim.Step(2);
    (void)MeasureJump_(sim, true);
    EXPECT_TRUE(sim.Player().IsGrounded());
    EXPECT_FLOAT_EQ(sim.Y(), 378.f);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Idle);
}

TEST(PlayerJump, HoldingJumpDoesNotBounce)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Jump);
    sim.Step(120);
    EXPECT_EQ(sim.Player().GetStats().jumps, 1);
}

TEST(PlayerJump, FullAirControl)
{
    Sim sim;
    sim.Step(2);
    sim.Tap(eAction::Jump);
    sim.Step(3);

    const float x0 = sim.X();
    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_NEAR(sim.X() - x0, sim.Tune().move.airSpeed / 60.f, 0.01f);   // 공중에서도 즉시 최고 속도
    EXPECT_EQ(sim.Player().GetFacing(), 1);

    sim.Release(eAction::Right);
    sim.Hold(eAction::Left);
    const float x1 = sim.X();
    sim.Step();
    EXPECT_LT(sim.X(), x1);
    EXPECT_EQ(sim.Player().GetFacing(), -1);
}

// 지면 위 38px 에서 시작하는 Sim: 곧 착지하는 점프 버퍼 시험용
[[nodiscard]] static Sim MakeLandingSim_()
{
    return Sim { MakeConfig(MakeFlatGround(), { 100.f, 340.f }) };
}

TEST(PlayerJump, JumpBufferTriggersOnLanding)
{
    Sim       probe   = MakeLandingSim_();
    const int landing = probe.StepUntilGrounded(100);
    ASSERT_GT(landing, 6);   // landing = 착지가 일어난 틱까지 진행한 수

    Sim a = MakeLandingSim_();
    a.Step(landing - 3);
    a.Tap(eAction::Jump);
    a.Step(2);   // 눌린 다음 두 틱 → 착지 틱 포함
    EXPECT_EQ(a.Player().GetStats().jumps, 1);
    EXPECT_LT(a.Player().GetVelocity().y, 0.f);

    Sim b = MakeLandingSim_();
    b.Step(landing - 8);
    b.Tap(eAction::Jump);
    b.Step(12);
    EXPECT_EQ(b.Player().GetStats().jumps, 0);
    EXPECT_TRUE(b.Player().IsGrounded());
}

TEST(PlayerJump, JumpBufferBoundaryIsFourTicks)
{
    Sim       probe   = MakeLandingSim_();
    const int landing = probe.StepUntilGrounded(100);

    Sim ok = MakeLandingSim_();
    ok.Step(landing - 4);   // 눌림 틱은 착지 3 틱 전
    ok.Tap(eAction::Jump);
    ok.Step(6);
    EXPECT_EQ(ok.Player().GetStats().jumps, 1);

    Sim late = MakeLandingSim_();
    late.Step(landing - 5);   // 한 틱 더 일찍(착지 4 틱 전): 버퍼(4틱)를 벗어난다
    late.Tap(eAction::Jump);
    late.Step(8);
    EXPECT_EQ(late.Player().GetStats().jumps, 0);
}

[[nodiscard]] static SandboxConfig MakeLedgeConfig_()
{
    GroundMap ground = GroundMap::MakeEmpty(1600, 448);
    ground.FillRect(0, 378, 399, 447, true);
    ground.FillRect(400, 440, 1599, 447, true);
    return MakeConfig(std::move(ground), { 300.f, 378.f });
}

TEST(PlayerJump, CoyoteJumpWorksShortlyAfterWalkingOffALedge)
{
    Sim sim { MakeLedgeConfig_() };
    sim.Hold(eAction::Right);
    const int walked = sim.StepUntilAirborne(200);
    ASSERT_GE(walked, 0);   // 이 시점이 낭떠러지를 벗어난 첫 틱 T

    sim.Step(2);   // T+1, T+2
    sim.Hold(eAction::Jump);
    sim.Step();
    EXPECT_EQ(sim.Player().GetStats().jumps, 1);
    EXPECT_LT(sim.Player().GetVelocity().y, 0.f);
}

TEST(PlayerJump, CoyoteJumpIsRejectedLongAfterTheLedge)
{
    Sim sim { MakeLedgeConfig_() };
    sim.Hold(eAction::Right);
    ASSERT_GE(sim.StepUntilAirborne(200), 0);

    sim.Step(7);   // T+1 .. T+7
    sim.Hold(eAction::Jump);
    sim.Step();
    EXPECT_EQ(sim.Player().GetStats().jumps, 0);
    EXPECT_GT(sim.Player().GetVelocity().y, 0.f);   // 계속 떨어진다
}

TEST(PlayerJump, CoyoteWindowEndsAfterConfiguredTicks)
{
    const int coyote = Tuning {}.move.coyoteTicks;
    for (const int offset: { coyote, coyote + 1 })
    {
        Sim sim { MakeLedgeConfig_() };
        sim.Hold(eAction::Right);
        ASSERT_GE(sim.StepUntilAirborne(200), 0);
        sim.Step(offset - 1);
        sim.Hold(eAction::Jump);
        sim.Step();
        EXPECT_EQ(sim.Player().GetStats().jumps, offset <= coyote ? 1 : 0) << "offset=" << offset;
    }
}

TEST(PlayerJump, CoyoteDoesNotAllowDoubleJump)
{
    Sim sim;
    sim.Step(2);
    sim.Tap(eAction::Jump);
    sim.Step(2);
    sim.Tap(eAction::Jump);
    sim.Step(10);
    EXPECT_EQ(sim.Player().GetStats().jumps, 1);
}

TEST(PlayerJump, NoJumpWhenCeilingIsRightAboveTheHead)
{
    GroundMap ground = MakeFlatGround();
    ground.FillRect(0, 300, 1599, 319, true);   // 서면 머리 y=320 바로 위
    Sim sim { MakeConfig(std::move(ground)) };

    sim.Step(2);
    sim.Hold(eAction::Jump);
    sim.Step(10);
    EXPECT_EQ(sim.Player().GetStats().jumps, 0);
    EXPECT_TRUE(sim.Player().IsGrounded());
    EXPECT_FLOAT_EQ(sim.Y(), 378.f);
}

TEST(PlayerJump, HeadBumpsCeilingAndFallsBack)
{
    GroundMap ground = MakeFlatGround();
    ground.FillRect(0, 280, 1599, 299, true);   // 천장 하단 y=300: 서 있는 머리(320) 위 20 px 여유
    Sim sim { MakeConfig(std::move(ground)) };

    sim.Step(2);
    sim.Hold(eAction::Jump);
    float minHeadY = 1000.f;
    for (int i = 0; i < 80; ++i)
    {
        sim.Step();
        minHeadY = std::min(minHeadY, sim.Y() - sim.Tune().body.standSize.y);
    }
    EXPECT_GE(minHeadY, 300.f);   // 천장을 뚫지 않는다
    EXPECT_TRUE(sim.Player().IsGrounded());
}

TEST(PlayerJump, DownPlusJumpDropsThroughOneWayPlatform)
{
    SandboxConfig config = MakeConfig(MakeFlatGround(), { 100.f, 300.f });
    config.platforms.push_back({ Aabb::FromMinMax({ 20.f, 300.f }, { 400.f, 308.f }), true });
    Sim sim { std::move(config) };

    sim.Step(3);
    ASSERT_TRUE(sim.Player().IsGrounded());
    ASSERT_FLOAT_EQ(sim.Y(), 300.f);

    sim.Hold(eAction::Down);
    sim.Hold(eAction::Jump);
    sim.Step(2);
    sim.ReleaseAll();
    sim.Step(60);
    EXPECT_EQ(sim.Player().GetStats().jumps, 0);   // 점프가 아니라 통과
    EXPECT_FLOAT_EQ(sim.Y(), 378.f);
}

TEST(PlayerJump, JumpFromOneWayPlatformLandsBackOnIt)
{
    SandboxConfig config = MakeConfig(MakeFlatGround(), { 100.f, 300.f });
    config.platforms.push_back({ Aabb::FromMinMax({ 20.f, 300.f }, { 400.f, 308.f }), true });
    Sim sim { std::move(config) };

    sim.Step(3);
    sim.Hold(eAction::Jump);
    sim.Step(60);
    EXPECT_EQ(sim.Player().GetStats().jumps, 1);
    EXPECT_FLOAT_EQ(sim.Y(), 300.f);
}

TEST(PlayerJump, WalkingOffALedgeStartsFalling)
{
    Sim sim { MakeLedgeConfig_() };
    sim.Hold(eAction::Right);
    ASSERT_GE(sim.StepUntilAirborne(200), 0);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Jump);
    EXPECT_EQ(sim.Player().GetStats().jumps, 0);
}

[[nodiscard]] static Vector<InputFrame> MakeNoisyScript_(
    const int      _ticks,
    const uint64_t _seed)
{
    RngMt19937         rng { _seed };
    Vector<InputFrame> frames;
    InputFrame         cur;
    for (int i = 0; i < _ticks; ++i)
    {
        if (i % 8 == 0)
        {
            const eAction action = static_cast<eAction>(Random<int>(rng, 0, 6));
            cur.Set(action, !cur.IsHeld(action));
        }
        frames.push_back(cur);
    }
    return frames;
}

[[nodiscard]] static uint64_t RunScript_(
    const Vector<InputFrame>& _frames)
{
    SandboxConfig config = MakeFlatConfig();
    config.bSpawnTargets = true;
    Sim sim { std::move(config) };
    for (const InputFrame& frame: _frames)
    {
        sim.StepWith(frame);
    }
    return sim.State().HashState();
}

TEST(PlayerDeterminism, SameScriptTwiceGivesIdenticalStateHash)
{
    const Vector<InputFrame> script = MakeNoisyScript_(1500, 42);
    EXPECT_EQ(RunScript_(script), RunScript_(script));
}

TEST(PlayerDeterminism, DifferentScriptsGiveDifferentHashes)
{
    EXPECT_NE(RunScript_(MakeNoisyScript_(1500, 42)), RunScript_(MakeNoisyScript_(1500, 43)));
}

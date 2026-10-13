// 플레이어 리그 테스트: 애니메이션 이름 매핑, 발 위치, 포즈 선택 검증
#include "Base/Types.h"
#include "Game/Player/PlayerRig.h"
#include "PlayerTestUtil.h"

namespace
{
[[nodiscard]] AnimId Id_(
    const AnimationLibrary& _anims,
    const char*             _name)
{
    const AnimId id = _anims.Find(_name);
    EXPECT_FALSE(id.IsNull()) << "클립 없음: " << _name;
    return id;
}

[[nodiscard]] const AnimFrame& LowerFrame_(
    Sim&                    _sim,
    const AnimationLibrary& _anims)
{
    const PlayerRig& rig = _sim.State().GetRig();
    return _anims.Get(rig.GetLowerClip()).frames[static_cast<size_t>(rig.GetLowerFrame())];
}
}   // namespace

TEST(PlayerRigNames, EveryRequiredSlotResolvesForEveryWeaponAndSide)
{
    SLUG_REQUIRE_BAKED(anims);

    PlayerRig                   rig;
    const PlayerRig::BindReport report = rig.Bind(anims, RigTuning {});
    EXPECT_EQ(report.missingRequired, 0) << (report.missingNames.empty() ? "" : report.missingNames.front());

    for (const eWeapon weapon: { eWeapon::Pistol, eWeapon::Hmg, eWeapon::Shotgun })
    {
        for (const int facing: { -1, 1 })
        {
            for (size_t s = 0; s < kRigClipCount; ++s)
            {
                const eRigClip slot = static_cast<eRigClip>(s);
                if (PlayerRig::IsSlotRequired(slot))
                {
                    EXPECT_FALSE(PlayerRig::ResolveClipName(anims, slot, weapon, facing).empty()) << GetRigClipName(slot) << " weapon=" << static_cast<int>(weapon) << " facing=" << facing;
                }
                EXPECT_FALSE(rig.GetSlotClip(weapon, facing, slot).IsNull()) << GetRigClipName(slot);
            }
        }
    }
}

TEST(PlayerRigNames, TableFollowsTheLegacyNames)
{
    SLUG_REQUIRE_BAKED(anims);

    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Idle, eWeapon::Pistol, 1), "Player_Stand_Idle_DG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Move, eWeapon::Pistol, -1), "Player_Stand_Move_DG_Left");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::LowerMove, eWeapon::Pistol, -1), "Player_Stand_Move_Lower_Left");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::LowerJumpMove, eWeapon::Pistol, 1), "Player_Jump_Move_Lower_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Shot, eWeapon::Pistol, -1), "Player_Stand_Shot_DG_Left");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::JumpShotDown, eWeapon::Pistol, -1), "Player_Jump_Shot_Down_DG_Left");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Death, eWeapon::Pistol, 1), "Player_Death_byBullet_DG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Victory, eWeapon::Pistol, 1), "Player_Victory_DG");

    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Shot, eWeapon::Hmg, 1), "Player_Stand_Shot_MG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Idle, eWeapon::Hmg, 1), "Player_Stand_Idle_SG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::CrouchShot, eWeapon::Hmg, -1), "Player_Crouch_Shot_MG_Left");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::JumpShotDown, eWeapon::Hmg, 1), "Player_Jump_Shot_Lookdown_MG_Right");

    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Shot, eWeapon::Shotgun, 1), "Player_Stand_Shot_SG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::JumpShotDown, eWeapon::Shotgun, 1), "Player_Jump_Shot_Lookdown_SG_Right");
}

TEST(PlayerRigNames, DgHasNoDiagonalShotClipSoItFallsBackToTheAimPose)
{
    SLUG_REQUIRE_BAKED(anims);
    PlayerRig rig;
    (void)rig.Bind(anims, RigTuning {});
    EXPECT_TRUE(PlayerRig::ResolveClipName(anims, eRigClip::ShotDiag3, eWeapon::Pistol, 1).empty());
    EXPECT_EQ(rig.GetSlotClip(eWeapon::Pistol, 1, eRigClip::ShotDiag3), rig.GetSlotClip(eWeapon::Pistol, 1, eRigClip::LookUpDiag3));
    EXPECT_FALSE(PlayerRig::ResolveClipName(anims, eRigClip::ShotDiag3, eWeapon::Hmg, 1).empty());
}

TEST(PlayerRigPose, IdleUsesIdleUpperAndLowerClips)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(3);
    const PlayerRig& rig = sim.State().GetRig();
    EXPECT_EQ(rig.GetUpperClip(), Id_(anims, "Player_Stand_Idle_DG_Right"));
    EXPECT_EQ(rig.GetLowerClip(), Id_(anims, "Player_Stand_Idle_Lower_Right"));
}

TEST(PlayerRigPose, RunSwitchesToRunClipsOnTheSameTickAndFacingPicksTheSide)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(3);

    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Move_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id_(anims, "Player_Stand_Move_Lower_Right"));

    sim.Release(eAction::Right);
    sim.Hold(eAction::Left);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Move_DG_Left"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id_(anims, "Player_Stand_Move_Lower_Left"));
}

TEST(PlayerRigPose, UpperAndLowerShareOnePhaseWhileRunning)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Hold(eAction::Right);
    for (int i = 0; i < 90; ++i)
    {
        sim.Step();
        EXPECT_EQ(sim.State().GetRig().GetUpperFrame(), sim.State().GetRig().GetLowerFrame()) << "tick " << i;
    }
}

TEST(PlayerRigPose, UpperKeepsThePhaseAfterAShotOverlayEndsWhileRunning)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Hold(eAction::Right);
    sim.Step(10);
    sim.Tap(eAction::Shot);
    sim.Step(sim.Tune().weapons.pistol.shotOverlayTicks + 3);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Move_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetUpperFrame(), sim.State().GetRig().GetLowerFrame());
}

TEST(PlayerRigPose, CrouchJumpAndAirClips)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(2);

    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Crouch_Idle_DG_Right"));
    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Crouch_Move_DG_Right"));

    sim.ReleaseAll();
    sim.Step(2);
    sim.Hold(eAction::Jump);
    sim.Step(3);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Jump_Idle_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id_(anims, "Player_Jump_Idle_Lower_Right"));

    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Jump_Move_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id_(anims, "Player_Jump_Move_Lower_Right"));

    sim.ReleaseAll();
    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Jump_Lookdown_DG_Right"));
}

TEST(PlayerRigPose, ShotSlashAndThrowOverlaysPickTheirClips)
{
    SLUG_REQUIRE_BAKED(anims);
    {
        Sim sim { MakeFlatConfig() };
        sim.Step(2);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Shot_DG_Right"));
        sim.Step(sim.Tune().weapons.pistol.shotOverlayTicks + 1);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Idle_DG_Right"));
    }
    {
        Sim sim { MakeFlatConfig() };
        sim.Step(2);
        sim.Hold(eAction::Up);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Shot_Up_DG_Right"));
    }
    {
        Sim sim { MakeFlatConfig() };
        sim.Step(2);
        sim.Tap(eAction::Bomb);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Throw_DG_Right"));
    }
    {
        Sim sim { MakeFlatConfig() };
        sim.Step(2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Slash2_DG_Right"));
        sim.Step(sim.Tune().knife.durationTicks + 2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Slash1_DG_Right"));
    }
}

TEST(PlayerRigPose, CrouchAndAirVariantsOfTheOverlays)
{
    SLUG_REQUIRE_BAKED(anims);
    {
        Sim sim { MakeFlatConfig() };
        sim.Step(2);
        sim.Hold(eAction::Down);
        sim.Step(2);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Crouch_Shot_DG_Right"));
        sim.Release(eAction::Down);
    }
    {
        Sim sim { MakeFlatConfig() };
        sim.Step(2);
        sim.Tap(eAction::Jump);
        sim.Step(8);
        sim.Hold(eAction::Down);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Jump_Shot_Down_DG_Right"));
    }
}

TEST(PlayerRigPose, HmgUsesMachineGunShotClipAndShotgunItsOwn)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Tap(eAction::Shot);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Shot_MG_Right"));
    sim.Step(sim.Tune().weapons.hmg.burstShotsMax * sim.Tune().weapons.hmg.burstIntervalTicks + 2);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Idle_SG_Right"));

    sim.Player().PickUpWeapon(eWeapon::Shotgun, sim.Tune());
    sim.Step(sim.Tune().weapons.shotgun.fireIntervalTicks);
    sim.Tap(eAction::Shot);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Shot_SG_Right"));
}

TEST(PlayerRigPose, HmgRunAndShootCombinesShotUpperWithRunLower)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Hold(eAction::Right);
    sim.Step(5);
    sim.Tap(eAction::Shot);
    // 버스트 최소 길이(3발 x 4틱 = 12틱) 안에서 달리는 동안: 상체는 사격, 하체는 달리기
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Stand_Shot_MG_Right")) << "tick " << i;
        EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id_(anims, "Player_Stand_Move_Lower_Right")) << "tick " << i;
        sim.Step(4);
    }
}

TEST(PlayerRigPose, EveryNewShotRestartsTheShotAnimation)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Tap(eAction::Shot);
    sim.Step(sim.Tune().weapons.hmg.burstIntervalTicks * 2);
    const int beforeFrame = sim.State().GetRig().GetUpperFrame();
    sim.Step(sim.Tune().weapons.hmg.burstShotsMax * sim.Tune().weapons.hmg.burstIntervalTicks);
    sim.Tap(eAction::Shot);
    const int afterFrame = sim.State().GetRig().GetUpperFrame();
    EXPECT_EQ(afterFrame, 0);
    EXPECT_GE(beforeFrame, 0);
}

TEST(PlayerRigPose, DeathPlaysTheDeathClipAndFacingPicksTheSide)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(2);
    sim.Hold(eAction::Left);
    sim.Step(3);
    sim.ReleaseAll();
    sim.Player().Kill();
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id_(anims, "Player_Death_byBullet_DG_Left"));
}

TEST(PlayerRigFeet, IdleAndRunLowerBodyFeetSitOnTheGround)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Step(3);

    const PlayerRig& rig = sim.State().GetRig();
    {
        const AnimFrame& frame  = LowerFrame_(sim, anims);
        const float      bottom = rig.GetOrigin(sim.Player().GetFeet()).y + static_cast<float>(frame.h - frame.pivotY);
        EXPECT_FLOAT_EQ(bottom, sim.Y());
    }

    sim.Hold(eAction::Right);
    int groundedFrames = 0;
    for (int i = 0; i < 24; ++i)
    {
        sim.Step();
        const AnimFrame& frame  = LowerFrame_(sim, anims);
        const float      bottom = rig.GetOrigin(sim.Player().GetFeet()).y + static_cast<float>(frame.h - frame.pivotY);
        EXPECT_LE(bottom, sim.Y() + 0.01f) << "발이 땅을 파고들었다 (tick " << i << ")";
        if (bottom >= sim.Y() - 0.01f)
        {
            ++groundedFrames;
        }
    }
    EXPECT_GE(groundedFrames, 16);
}

TEST(PlayerRigFeet, RunCycleOriginNeverMovesVerticallyAndAnchorDriftIsAtMostTwoPixels)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Hold(eAction::Right);
    sim.Step(2);

    const float originY = sim.State().GetRig().GetOrigin(sim.Player().GetFeet()).y;
    for (int i = 0; i < 60; ++i)
    {
        sim.Step();
        EXPECT_FLOAT_EQ(sim.State().GetRig().GetOrigin(sim.Player().GetFeet()).y, originY);
    }

    for (const char* name: { "Player_Stand_Move_DG_Right", "Player_Stand_Move_DG_Left" })
    {
        const AnimClip* pClip = anims.FindClip(name);
        ASSERT_NE(pClip, nullptr) << name;
        for (size_t i = 0; i < pClip->frames.size(); ++i)
        {
            const AnimFrame& a  = pClip->frames[i];
            const AnimFrame& b  = pClip->frames[(i + 1) % pClip->frames.size()];
            const int        dy = Abs((a.h - a.pivotY) - (b.h - b.pivotY));
            EXPECT_LE(dy, 2) << name << " frame " << i;
        }
    }

    for (const char* name: { "Player_Stand_Move_Lower_Right", "Player_Stand_Move_Lower_Left" })
    {
        const AnimClip* pClip = anims.FindClip(name);
        ASSERT_NE(pClip, nullptr) << name;
        const int baseline = pClip->GetFootBaseline();
        int       airborne = 0;
        for (const AnimFrame& frame: pClip->frames)
        {
            const int bottom = frame.h - frame.pivotY;
            if (bottom < baseline - 2)
            {
                ++airborne;
            }
        }
        EXPECT_LE(airborne, 2) << name;
    }
}

TEST(PlayerRigFeet, CrouchAndIdleBodiesAlsoStandOnTheGround)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig() };
    sim.Hold(eAction::Down);
    sim.Step(5);

    const PlayerRig& rig  = sim.State().GetRig();
    const AnimClip&  clip = anims.Get(rig.GetUpperClip());
    EXPECT_FLOAT_EQ(rig.GetOriginLift(), static_cast<float>(clip.GetFootBaseline()));
}

TEST(PlayerRigFeet, EveryPlayerClipHasSanePivots)
{
    SLUG_REQUIRE_BAKED(anims);
    int checked = 0;
    for (const AnimClip& clip: anims.GetClips())
    {
        if (clip.name.rfind("Player_", 0) != 0)
        {
            continue;
        }
        ++checked;
        for (const AnimFrame& frame: clip.frames)
        {
            if (frame.IsEmpty())
            {
                continue;
            }
            if (clip.name.find("Idle_Lookup_SG") != String::npos)
            {
                continue;
            }
            EXPECT_LE(Abs(frame.pivotY), frame.h + 160) << clip.name;
            EXPECT_LE(Abs(frame.pivotX), frame.w + 160) << clip.name;
        }
    }
    EXPECT_GT(checked, 100);
}

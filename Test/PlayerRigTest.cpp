#include "PlayerTestUtil.h"
#include "Base/Types.h"

#include "Game/Player/PlayerRig.h"

using namespace slugtest;

namespace
{
AnimId Id(
    const AnimationLibrary& _anims,
    const char*             _name)
{
    const AnimId id = _anims.Find(_name);
    EXPECT_FALSE(id.IsNull()) << "클립 없음: " << _name;
    return id;
}

const AnimFrame& LowerFrame(
    Sim&                    _sim,
    const AnimationLibrary& _anims)
{
    const PlayerRig& rig = _sim.State().GetRig();
    return _anims.Get(rig.GetLowerClip()).frames[static_cast<size_t>(rig.GetLowerFrame())];
}
}   // namespace

// ===========================================
//  이름 표
// ===========================================

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
                // 선택 슬롯도 폴백을 거쳐 반드시 어떤 클립으로든 해석된다
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

    // HMG: 사격 동작은 MG 전용 클립, 나머지는 SG 세트를 재사용
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Shot, eWeapon::Hmg, 1), "Player_Stand_Shot_MG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::Idle, eWeapon::Hmg, 1), "Player_Stand_Idle_SG_Right");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::CrouchShot, eWeapon::Hmg, -1), "Player_Crouch_Shot_MG_Left");
    EXPECT_EQ(PlayerRig::ResolveClipName(anims, eRigClip::JumpShotDown, eWeapon::Hmg, 1), "Player_Jump_Shot_Lookdown_MG_Right");

    // 샷건
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
    EXPECT_FALSE(PlayerRig::ResolveClipName(anims, eRigClip::ShotDiag3, eWeapon::Hmg, 1).empty());   // MG 에는 있다
}

// ===========================================
//  상태 → 클립 선택
// ===========================================

TEST(PlayerRigPose, IdleUsesIdleUpperAndLowerClips)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(3);
    const PlayerRig& rig = sim.State().GetRig();
    EXPECT_EQ(rig.GetUpperClip(), Id(anims, "Player_Stand_Idle_DG_Right"));
    EXPECT_EQ(rig.GetLowerClip(), Id(anims, "Player_Stand_Idle_Lower_Right"));
}

TEST(PlayerRigPose, RunSwitchesToRunClipsOnTheSameTickAndFacingPicksTheSide)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(3);

    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Move_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id(anims, "Player_Stand_Move_Lower_Right"));

    sim.Release(eAction::Right);
    sim.Hold(eAction::Left);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Move_DG_Left"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id(anims, "Player_Stand_Move_Lower_Left"));
}

TEST(PlayerRigPose, UpperAndLowerShareOnePhaseWhileRunning)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
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
    Sim sim { MakeFlatConfig(), &anims };
    sim.Hold(eAction::Right);
    sim.Step(10);
    sim.Tap(eAction::Shot);
    sim.Step(sim.Tune().weapons.pistol.shotOverlayTicks + 3);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Move_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetUpperFrame(), sim.State().GetRig().GetLowerFrame());   // 달리기 위상이 이어진다
}

TEST(PlayerRigPose, CrouchJumpAndAirClips)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(2);

    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Crouch_Idle_DG_Right"));
    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Crouch_Move_DG_Right"));

    sim.ReleaseAll();
    sim.Step(2);
    sim.Hold(eAction::Jump);
    sim.Step(3);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Jump_Idle_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id(anims, "Player_Jump_Idle_Lower_Right"));

    sim.Hold(eAction::Right);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Jump_Move_DG_Right"));
    EXPECT_EQ(sim.State().GetRig().GetLowerClip(), Id(anims, "Player_Jump_Move_Lower_Right"));

    sim.ReleaseAll();
    sim.Hold(eAction::Down);
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Jump_Lookdown_DG_Right"));
}

TEST(PlayerRigPose, ShotSlashAndThrowOverlaysPickTheirClips)
{
    SLUG_REQUIRE_BAKED(anims);
    {
        Sim sim { MakeFlatConfig(), &anims };
        sim.Step(2);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Shot_DG_Right"));
        sim.Step(sim.Tune().weapons.pistol.shotOverlayTicks + 1);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Idle_DG_Right"));   // 겹침이 끝나면 복귀
    }
    {
        Sim sim { MakeFlatConfig(), &anims };
        sim.Step(2);
        sim.Hold(eAction::Up);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Shot_Up_DG_Right"));
    }
    {
        Sim sim { MakeFlatConfig(), &anims };
        sim.Step(2);
        sim.Tap(eAction::Bomb);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Throw_DG_Right"));
    }
    {
        Sim sim { MakeFlatConfig(), &anims };
        sim.Step(2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Slash2_DG_Right"));   // 첫 베기: 번갈아 시작 (토글)
        sim.Step(sim.Tune().knife.durationTicks + 2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Slash1_DG_Right"));
    }
}

TEST(PlayerRigPose, CrouchAndAirVariantsOfTheOverlays)
{
    SLUG_REQUIRE_BAKED(anims);
    {
        Sim sim { MakeFlatConfig(), &anims };
        sim.Step(2);
        sim.Hold(eAction::Down);
        sim.Step(2);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Crouch_Shot_DG_Right"));
        sim.Release(eAction::Down);
    }
    {
        Sim sim { MakeFlatConfig(), &anims };
        sim.Step(2);
        sim.Tap(eAction::Jump);
        sim.Step(8);
        sim.Hold(eAction::Down);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Jump_Shot_Down_DG_Right"));
    }
}

TEST(PlayerRigPose, HmgUsesMachineGunShotClipAndShotgunItsOwn)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Tap(eAction::Shot);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Shot_MG_Right"));
    sim.Step(20);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Idle_SG_Right"));

    sim.Player().PickUpWeapon(eWeapon::Shotgun, sim.Tune());
    sim.Tap(eAction::Shot);
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Stand_Shot_SG_Right"));
}

TEST(PlayerRigPose, EveryNewShotRestartsTheShotAnimation)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Hold(eAction::Shot);
    sim.Step();
    sim.Step(sim.Tune().weapons.hmg.holdRepeatTicks - 1);   // 다음 발사 직전: 애니메이션이 진행되어 있다
    const int beforeFrame = sim.State().GetRig().GetUpperFrame();
    sim.Step();   // 다음 발사 틱: 처음부터 다시
    const int afterFrame = sim.State().GetRig().GetUpperFrame();
    EXPECT_EQ(afterFrame, 0);
    EXPECT_GE(beforeFrame, 0);
}

TEST(PlayerRigPose, DeathPlaysTheDeathClipAndFacingPicksTheSide)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(2);
    sim.Hold(eAction::Left);
    sim.Step(3);
    sim.ReleaseAll();
    sim.Player().Kill();
    sim.Step();
    EXPECT_EQ(sim.State().GetRig().GetUpperClip(), Id(anims, "Player_Death_byBullet_DG_Left"));
}

// ===========================================
//  발 정렬 / 떨림 (피벗 데이터 기반)
// ===========================================

TEST(PlayerRigFeet, IdleAndRunLowerBodyFeetSitOnTheGround)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Step(3);

    const PlayerRig& rig = sim.State().GetRig();
    {
        const AnimFrame& frame  = LowerFrame(sim, anims);
        const float      bottom = rig.GetOrigin(sim.Player().GetFeet()).y + static_cast<float>(frame.h - frame.pivotY);
        EXPECT_FLOAT_EQ(bottom, sim.Y());
    }

    sim.Hold(eAction::Right);
    int groundedFrames = 0;
    for (int i = 0; i < 24; ++i)
    {
        sim.Step();
        const AnimFrame& frame  = LowerFrame(sim, anims);
        const float      bottom = rig.GetOrigin(sim.Player().GetFeet()).y + static_cast<float>(frame.h - frame.pivotY);
        EXPECT_LE(bottom, sim.Y() + 0.01f) << "발이 땅을 파고들었다 (tick " << i << ")";
        if (bottom >= sim.Y() - 0.01f)
        {
            ++groundedFrames;   // 발이 정확히 닿는 프레임
        }
    }
    EXPECT_GE(groundedFrames, 16);   // 12 프레임 주기 중 10 프레임은 접지 (나머지 2 프레임은 공중 동작)
}

TEST(PlayerRigFeet, RunCycleOriginNeverMovesVerticallyAndAnchorDriftIsAtMostTwoPixels)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeFlatConfig(), &anims };
    sim.Hold(eAction::Right);
    sim.Step(2);

    const float originY = sim.State().GetRig().GetOrigin(sim.Player().GetFeet()).y;
    for (int i = 0; i < 60; ++i)
    {
        sim.Step();
        EXPECT_FLOAT_EQ(sim.State().GetRig().GetOrigin(sim.Player().GetFeet()).y, originY);   // 달리는 동안 몸통 높이는 프레임마다 출렁이지 않는다
    }

    // 상체 달리기 클립의 프레임 간 바닥 중앙 앵커 변동 (피벗 기반)
    for (const char* name: { "Player_Stand_Move_DG_Right", "Player_Stand_Move_DG_Left" })
    {
        const AnimClip* pClip = anims.FindClip(name);
        ASSERT_NE(pClip, nullptr) << name;
        for (size_t i = 0; i < pClip->frames.size(); ++i)
        {
            const AnimFrame& a  = pClip->frames[i];
            const AnimFrame& b  = pClip->frames[(i + 1) % pClip->frames.size()];
            const int        dy = std::abs((a.h - a.pivotY) - (b.h - b.pivotY));
            EXPECT_LE(dy, 2) << name << " frame " << i;
        }
    }

    // 하체: 접지 프레임들의 바닥은 ±2px 안에 있고, 그보다 높은 프레임은 달리기의 공중 동작(주기당 2프레임)뿐
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
    Sim sim { MakeFlatConfig(), &anims };
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
            // 피벗이 프레임 크기의 몇 배씩 벗어나지 않는다 (단, Player_Stand_Idle_Lookup_SG 의 Y 오프셋 오타가 알려진 예외)
            if (clip.name.find("Idle_Lookup_SG") != slug::String::npos)
            {
                continue;
            }
            EXPECT_LE(std::abs(frame.pivotY), frame.h + 160) << clip.name;
            EXPECT_LE(std::abs(frame.pivotX), frame.w + 160) << clip.name;
        }
    }
    EXPECT_GT(checked, 100);
}

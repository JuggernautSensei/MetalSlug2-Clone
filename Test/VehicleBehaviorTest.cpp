#include <cmath>
#include "Base/Math.h"
#include <vector>

#include "VehicleTestUtil.h"

using namespace slugtest;

namespace
{
    constexpr float kGround = 378.f;

    SpawnParams Facing(const int _facing)
    {
        SpawnParams params;
        params.facing = _facing;
        return params;
    }

    struct SpawnLog
    {
        std::vector<int>      ticks;
        std::vector<Shot>     shots;
    };

    void StepAndLog(VehicleRig& _rig, const EnemyHandle _owner, SpawnLog& _log, const int _count)
    {
        for (int i = 0; i < _count; ++i)
        {
            _rig.Step(1);
            for (const Shot& shot: _rig.Projectiles().GetShots())
            {
                if (shot.params.owner == _owner.id && shot.ageTicks == 1)
                {
                    _log.ticks.push_back(_rig.Ticks());
                    _log.shots.push_back(shot);
                }
            }
        }
    }

    EnemyHandle SpawnJeepFacingThePlayer(VehicleRig& _rig, const Vec2 _player, const float _x)
    {
        _rig.SetPlayer(_player);
        return _rig.Spawn("jeep", { _x, kGround }, Facing(-1));
    }
}

TEST(VehicleJeep, DrivesAtTwoHundredPixelsPerSecondTowardTheEdgeOnThePlayersSide)
{
    VehicleRig rig({ 100.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing(-1));
    rig.Step(90);   // 1 s 대기 후 좌측 가장자리로 출발
    const float x0 = rig.E(h)->body.position.x;
    rig.Step(30);
    const float x1 = rig.E(h)->body.position.x;
    EXPECT_NEAR(x0 - x1, 100.f, 3.f);   // 200 px/s * 0.5 s
    EXPECT_EQ(rig.E(h)->facing, -1);
    EXPECT_EQ(rig.E(h)->state, 2) << "Move 상태여야 한다";   // kMove
}

TEST(VehicleJeep, WaitsOneSecondBeforeItMovesAfterActivation)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing(-1));
    rig.Step(30);
    EXPECT_FLOAT_EQ(rig.E(h)->body.position.x, 400.f);
}

TEST(VehicleJeep, StaysInactiveUntilItIsNearTheScreenEdge)
{
    VehicleRig rig({ 100.f, kGround }, 0.f);
    const EnemyHandle h = rig.Spawn("jeep", { 1200.f, kGround }, Facing(-1));   // 640 + 150 밖
    rig.Step(200);
    EXPECT_FALSE(rig.E(h)->bActive);
    EXPECT_FLOAT_EQ(rig.E(h)->body.position.x, 1200.f);
}

TEST(VehicleJeep, RocketsLeaveFromTheRearTowardThePlayer)
{
    VehicleRig rig;
    const EnemyHandle h = SpawnJeepFacingThePlayer(rig, { 500.f, kGround }, 300.f);
    SpawnLog log;
    StepAndLog(rig, h, log, 900);
    ASSERT_FALSE(log.shots.empty());
    const Shot& rocket = log.shots.front();
    EXPECT_LT(rocket.velocity.x, 0.f) << "플레이어(왼쪽)를 향해 뒤로 나간다";
    EXPECT_NEAR(rocket.velocity.y, 29.85f, 1.5f);
    EXPECT_EQ(rocket.params.startDelayTicks, 24);   // 점화 0.4 s
}

TEST(VehicleJeep, VolleyIsFourRocketsInTwoPairsWithPauses)
{
    VehicleRig rig;
    const EnemyHandle h = SpawnJeepFacingThePlayer(rig, { 500.f, kGround }, 300.f);
    SpawnLog log;
    StepAndLog(rig, h, log, 1500);
    ASSERT_GE(log.ticks.size(), 4u);
    const int gap1 = log.ticks[1] - log.ticks[0];
    const int gap2 = log.ticks[2] - log.ticks[1];
    const int gap3 = log.ticks[3] - log.ticks[2];
    EXPECT_NEAR(gap1, 96, 8) << "같은 쌍의 두 발은 약 1.6 s 간격";
    EXPECT_NEAR(gap2, 216, 10) << "쌍 사이 2 s 휴지 + 1.6 s";
    EXPECT_NEAR(gap3, 96, 8);
}

TEST(VehicleJeep, DiesAtEightyHitPointsAndAwardsItsScore)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 26; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    }
    EXPECT_TRUE(rig.E(h)->IsAlive());
    (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    EXPECT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 500);
}

TEST(VehicleJeep, BombsKillItInFourHitsAndItLeavesAfterOnePointSevenSeconds)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 4; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Blast, 20, eTeam::Player);
    }
    EXPECT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    rig.Step(90);
    EXPECT_NE(rig.E(h), nullptr) << "1.7 s 전에는 남아 있다";
    rig.Step(20);
    EXPECT_EQ(rig.E(h), nullptr) << "1.7 s 뒤 제거";
}

TEST(VehicleJeep, ARocketHitsAPlayerAtItsFlightHeight)
{
    VehicleRig rig;
    (void)SpawnJeepFacingThePlayer(rig, { 500.f, 300.f }, 300.f);
    rig.Step(1200);
    EXPECT_GE(rig.PlayerHits(), 1);
}

TEST(VehicleBTank, StaysClosedUntilThePlayerIsWithinFiveHundredPixels)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 1000.f, kGround }, Facing(-1));
    rig.Step(200);
    EXPECT_FALSE(rig.E(h)->bActive);
    rig.SetPlayer({ 520.f, kGround });
    rig.Step(1);
    EXPECT_TRUE(rig.E(h)->bActive);
}

TEST(VehicleBTank, OpensThenFiresTwoShellsPerCycleEveryFourPointEightSeconds)
{
    VehicleRig rig({ 520.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 1000.f, kGround }, Facing(-1));
    SpawnLog log;
    StepAndLog(rig, h, log, 620);
    ASSERT_GE(log.ticks.size(), 4u);
    EXPECT_NEAR(log.ticks[1] - log.ticks[0], 53, 4) << "연발: 발사 클립 길이(0.88 s)만큼";
    EXPECT_NEAR(log.ticks[2] - log.ticks[0], 288, 10) << "사이클 4.8 s";
    EXPECT_NEAR(log.ticks[3] - log.ticks[2], 53, 4);
}

TEST(VehicleBTank, ShellFliesLeftwardAsALob)
{
    VehicleRig rig({ 520.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 1000.f, kGround }, Facing(-1));
    SpawnLog log;
    StepAndLog(rig, h, log, 300);
    ASSERT_FALSE(log.shots.empty());
    const Shot& shell = log.shots.front();
    EXPECT_NEAR(shell.velocity.x, -400.f, 1.f);
    EXPECT_LT(shell.velocity.y, -180.f);   // 발사 직후 위로
    EXPECT_NEAR(shell.position.x, 920.f - 400.f / 60.f, 1.5f) << "머즐은 전차 왼쪽 80px (첫 틱에 한 틱 전진)";
}

TEST(VehicleBTank, ShellThatPassesOverAStandingPlayerHitsIt)
{
    VehicleRig rig({ 520.f, kGround });
    (void)rig.Spawn("btank", { 1000.f, kGround }, Facing(-1));
    rig.Step(400);
    EXPECT_GE(rig.PlayerHits(), 1);
}

TEST(VehicleBTank, KnifeDoesNotHurtTheTankButBulletsKillItOnTheFifteenth)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 400.f, kGround }, Facing(-1));
    rig.Step(2);
    EXPECT_EQ(rig.Strike(h.id, eHitKind::Melee, 8), 0u);
    EXPECT_TRUE(rig.E(h)->IsAlive());
    for (int i = 0; i < 14; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 8);
    }
    EXPECT_TRUE(rig.E(h)->IsAlive());
    (void)rig.Strike(h.id, eHitKind::Bullet, 8);
    EXPECT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 1000);
}

TEST(VehicleKnife, NoVehicleAcceptsTheKnife)
{
    for (const char* pName: { "jeep", "btank", "missle_tank", "sarubia", "aparch" })
    {
        VehicleRig rig({ 100.f, kGround });
        const EnemyHandle h = rig.Spawn(pName, { 400.f, kGround }, Facing(-1));
        rig.Step(2);
        EXPECT_EQ(rig.Strike(h.id, eHitKind::Melee, 8), 0u) << pName;
        EXPECT_TRUE(rig.E(h)->IsAlive()) << pName;
    }
}

TEST(VehicleBTank, DestructionRemovesItAfterFourSeconds)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 400.f, kGround }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 40; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    }
    ASSERT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    rig.Step(200);
    EXPECT_NE(rig.E(h), nullptr);
    rig.Step(60);
    EXPECT_EQ(rig.E(h), nullptr);
}

TEST(VehicleMissleTank, FiresFiveMissilesPerVolleyWhenTheTargetIsNear)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 380.f, kGround }, Facing(-1));   // dx = 280 (<= 300: 직사)
    SpawnLog log;
    StepAndLog(rig, h, log, 400);
    EXPECT_EQ(log.ticks.size(), 5u);
    for (const Shot& shot: log.shots)
    {
        EXPECT_NEAR(shot.velocity.y, 6.7f, 8.f) << "직사: 수직 속도 0 (중력 1 틱분만)";
        EXPECT_NEAR(shot.velocity.x, -300.f, 1.f);
    }
}

TEST(VehicleMissleTank, AimsUpForFarTargetsAndTheVolleyArcs)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 500.f, kGround }, Facing(-1));   // dx = 400 (> 300: 곡사)
    SpawnLog log;
    StepAndLog(rig, h, log, 500);
    ASSERT_FALSE(log.shots.empty());
    EXPECT_LT(log.shots.front().velocity.y, -150.f);
    EXPECT_NEAR(log.shots.front().velocity.x, -300.f, 1.f);
}

TEST(VehicleMissleTank, OnlyTicksWhileThePlayerIsWithinSixHundredPixels)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 900.f, kGround }, Facing(-1));   // dx = 800
    SpawnLog log;
    StepAndLog(rig, h, log, 800);
    EXPECT_TRUE(log.ticks.empty());
}

TEST(VehicleMissleTank, DestructionIsSilentUntilTwoSecondsLater)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 380.f, kGround }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 34; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    }
    EXPECT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 1000);
    rig.Step(110);
    EXPECT_NE(rig.E(h), nullptr);
    rig.Step(20);
    EXPECT_EQ(rig.E(h), nullptr);
}

TEST(VehicleSarubia, WalksTowardAFarPlayerAtOneHundredPixelsPerSecond)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("sarubia", { 700.f, kGround }, Facing(-1));   // dx = -600
    rig.Step(200);
    const float x0 = rig.E(h)->body.position.x;
    rig.Step(30);
    const float x1 = rig.E(h)->body.position.x;
    EXPECT_NEAR(x0 - x1, 50.f, 4.f);
}

TEST(VehicleSarubia, GrenadeFallsStraightDownThenRollsAlongTheGround)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("sarubia", { 380.f, kGround }, Facing(-1));   // dx = -280: 사격 구간
    SpawnLog log;
    StepAndLog(rig, h, log, 260);
    ASSERT_EQ(log.shots.size(), 1u);
    const int spawnTick = log.ticks.front();

    int landedTick = -1;
    for (int i = 0; i < 120 && landedTick < 0; ++i)
    {
        rig.Step(1);
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.owner == h.id && shot.params.rollSpeed > 0.f)
            {
                if (!shot.bLanded)
                {
                    EXPECT_NEAR(shot.velocity.x, 0.f, 1e-3f);
                }
                else
                {
                    landedTick = rig.Ticks();
                }
            }
        }
    }
    ASSERT_GE(landedTick, 0);
    EXPECT_NEAR(landedTick - spawnTick, 60, 6) << "150 px 낙하 (중력 300) 약 1 s";

    const auto GrenadeX = [&]() {
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.owner == h.id)
            {
                return shot.position.x;
            }
        }
        return -1.f;
    };
    rig.Step(10);
    const float x0 = GrenadeX();
    rig.Step(60);
    const float x1 = GrenadeX();
    EXPECT_NEAR(x0 - x1, 100.f, 6.f);
}

TEST(VehicleSarubia, GrenadeRollsIntoAPlayerOnItsPath)
{
    VehicleRig rig({ 100.f, kGround });
    (void)rig.Spawn("sarubia", { 380.f, kGround }, Facing(-1));
    rig.Step(600);
    EXPECT_GE(rig.PlayerHits(), 1);
}

TEST(VehicleSarubia, DeathExplodesAtOnceAndLeavesAfterTwoSeconds)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("sarubia", { 700.f, kGround }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 50; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    }
    EXPECT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 2000);
    rig.Step(110);
    EXPECT_NE(rig.E(h), nullptr);
    rig.Step(20);
    EXPECT_EQ(rig.E(h), nullptr);
}

TEST(VehicleAparch, FliesToItsScreenPointWithoutFalling)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("aparch", { 1000.f, 125.f }, Facing(-1));
    rig.Step(200);
    const Enemy* pEnemy = rig.E(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_NEAR(pEnemy->body.position.x, 445.f, 1.f);
    EXPECT_NEAR(pEnemy->body.position.y, 125.f, 1.f);
    EXPECT_FALSE(pEnemy->bGrounded);
}

TEST(VehicleAparch, BombPatternDropsFollowTheLegacyRhythm)
{
    VehicleRig rig({ 300.f, kGround });
    const EnemyHandle h = rig.Spawn("aparch", { 445.f, 125.f }, Facing(-1));
    SpawnLog log;
    StepAndLog(rig, h, log, 1500);

    std::vector<int> drops;
    for (size_t i = 0; i < log.shots.size(); ++i)
    {
        if (log.shots[i].params.gravity > 0.f)
        {
            drops.push_back(log.ticks[i]);
        }
    }
    // 레거시: 패턴 진입 시 누적 타이머 2 s 가 이월되어 첫 3발은 연속 프레임에 떨어지고, 이후 0.5 s 간격 묶음이 이어진다
    const int kOffsets[] = { 0, 1, 2, 29, 59, 89, 149, 179, 209 };
    bool      bFound     = false;
    for (size_t i = 0; i + 9 <= drops.size() && !bFound; ++i)
    {
        bool bAll = true;
        for (size_t j = 0; j < 9; ++j)
        {
            bAll = bAll && std::abs((drops[i + j] - drops[i]) - kOffsets[j]) <= 1;
        }
        bFound = bAll;
    }
    EXPECT_TRUE(bFound) << "드롭 리듬 9발 (레거시 burst + 0.5 s 묶음)이 연속으로 나와야 한다";
}

namespace
{
    // 드롭 발사 시점의 스폰 위치가 헬기 기준 (_offsetX, +60) 인지 확인하고 드롭 발수를 누적한다
    void CheckDropSpawnOffsets_(VehicleRig& _rig, const EnemyHandle _h, const float _offsetX, const int _steps, int& _outDrops)
    {
        for (int i = 0; i < _steps; ++i)
        {
            _rig.Step(1);
            const Enemy* pEnemy = _rig.E(_h);
            ASSERT_NE(pEnemy, nullptr);
            for (const Shot& shot: _rig.Projectiles().GetShots())
            {
                if (shot.params.owner == _h.id && shot.ageTicks == 1 && shot.params.gravity > 0.f)
                {
                    EXPECT_NEAR(shot.params.position.x - pEnemy->body.position.x, _offsetX, 0.01f);
                    EXPECT_NEAR(shot.params.position.y - pEnemy->body.position.y, 60.f, 0.01f);
                    EXPECT_FLOAT_EQ(shot.params.velocity.x, 0.f);
                    EXPECT_FLOAT_EQ(shot.params.velocity.y, 0.f);
                    ++_outDrops;
                }
            }
        }
    }
}

TEST(VehicleAparch, BombsSpawnTenPixelsAheadAndSixtyBelowWhenFacingLeft)
{
    VehicleRig rig({ 300.f, kGround });
    const EnemyHandle h = rig.Spawn("aparch", { 445.f, 125.f }, Facing(-1));
    int drops = 0;
    CheckDropSpawnOffsets_(rig, h, 10.f, 400, drops);
    EXPECT_GT(drops, 0);
}

TEST(VehicleAparch, DropMuzzleMirrorsWithFacing)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const EnemyDef* pDef = lib.Find("aparch");
    ASSERT_NE(pDef, nullptr);
    const int drop = pDef->FindWeapon("drop");
    ASSERT_GE(drop, 0);

    // 스폰은 항상 좌향으로 초기화되므로 우향 케이스는 muzzle 계산 함수에 직접 넣어 검증한다
    Enemy left;
    left.facing        = -1;
    left.body.position = { 445.f, 125.f };
    Enemy right;
    right.facing        = 1;
    right.body.position = { 445.f, 125.f };

    const Vec2 l = GetMuzzleWorld(left, *pDef, drop);
    const Vec2 r = GetMuzzleWorld(right, *pDef, drop);
    EXPECT_FLOAT_EQ(l.x, 455.f);
    EXPECT_FLOAT_EQ(l.y, 185.f);
    EXPECT_FLOAT_EQ(r.x, 435.f);
    EXPECT_FLOAT_EQ(r.y, 185.f);
}

TEST(VehicleAparch, MissilePatternFiresThreeLeftwardMissilesOneSecondApart)
{
    VehicleRig rig({ 300.f, kGround });
    const EnemyHandle h = rig.Spawn("aparch", { 445.f, 125.f }, Facing(-1));
    SpawnLog log;
    StepAndLog(rig, h, log, 1500);

    std::vector<int> missiles;
    for (size_t i = 0; i < log.shots.size(); ++i)
    {
        if (log.shots[i].params.gravity == 0.f)
        {
            missiles.push_back(log.ticks[i]);
        }
    }
    ASSERT_GE(missiles.size(), 3u);
    EXPECT_NEAR(missiles[1] - missiles[0], 60, 2);
    EXPECT_NEAR(missiles[2] - missiles[1], 60, 2);
    for (const Shot& shot: log.shots)
    {
        if (shot.params.gravity == 0.f)
        {
            EXPECT_LT(shot.velocity.x, -290.f);
            EXPECT_NEAR(shot.velocity.y, 0.f, 1.f);
            break;
        }
    }
}

TEST(VehicleAparch, DeathDriftsAndChainsExplosionsBeforeItLeaves)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("aparch", { 445.f, 125.f }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 40; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    }
    ASSERT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 2000);
    const float x0 = rig.E(h)->body.position.x;
    const float y0 = rig.E(h)->body.position.y;
    rig.Step(30);
    EXPECT_NEAR(rig.E(h)->body.position.x - x0, 25.f, 2.f);   // 표류 (50, -70) px/s
    EXPECT_NEAR(rig.E(h)->body.position.y - y0, -35.f, 2.f);
    rig.Step(70);
    EXPECT_EQ(rig.E(h), nullptr) << "1.5 s 뒤 제거";
}

TEST(VehicleAparch, AttackingItRequiresTheFullHitPointsTotal)
{
    VehicleRig rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("aparch", { 445.f, 125.f }, Facing(-1));
    rig.Step(2);
    for (int i = 0; i < 39; ++i)
    {
        (void)rig.Strike(h.id, eHitKind::Bullet, 3);
    }
    EXPECT_TRUE(rig.E(h)->IsAlive());
}

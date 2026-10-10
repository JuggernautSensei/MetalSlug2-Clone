// 탈것 동작 테스트: Jeep, BTank, MissleTank, Sarubia 의 AI, 피격, 파괴 검증
#include <cmath>
#include <vector>

#include "Base/Math.h"
#include "VehicleTestUtil.h"

using namespace slugtest;

namespace
{
constexpr float kGround = 378.f;

[[nodiscard]] SpawnParams Facing_(
    const int _facing)
{
    SpawnParams params;
    params.facing = _facing;
    return params;
}

struct SpawnLog
{
    std::vector<int>  ticks = {};
    std::vector<Shot> shots = {};
};

void StepAndLog_(
    VehicleRig&       _rig,
    const EnemyHandle _owner,
    SpawnLog&         _log,
    const int         _count)
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

[[nodiscard]] EnemyHandle SpawnJeepFacingThePlayer_(
    VehicleRig& _rig,
    const Vec2  _player,
    const float _x)
{
    _rig.SetPlayer(_player);
    return _rig.Spawn("jeep", { _x, kGround }, Facing_(-1));
}

// _owner 가 쏜 첫 수류탄의 x. 없으면 -1
[[nodiscard]] float GrenadeX_(
    VehicleRig&       _rig,
    const EnemyHandle _owner)
{
    for (const Shot& shot: _rig.Projectiles().GetShots())
    {
        if (shot.params.owner == _owner.id)
        {
            return shot.position.x;
        }
    }
    return -1.f;
}
}   // namespace

TEST(VehicleJeep, DrivesAtTwoHundredPixelsPerSecondTowardTheEdgeOnThePlayersSide)
{
    VehicleRig rig({ 100.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing_(-1));
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
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing_(-1));
    rig.Step(30);
    EXPECT_FLOAT_EQ(rig.E(h)->body.position.x, 400.f);
}

TEST(VehicleJeep, StaysInactiveUntilItIsNearTheScreenEdge)
{
    VehicleRig        rig({ 100.f, kGround }, 0.f);
    const EnemyHandle h = rig.Spawn("jeep", { 1200.f, kGround }, Facing_(-1));   // 640 + 150 밖
    rig.Step(200);
    EXPECT_FALSE(rig.E(h)->bActive);
    EXPECT_FLOAT_EQ(rig.E(h)->body.position.x, 1200.f);
}

TEST(VehicleJeep, RocketsLeaveFromTheRearTowardThePlayer)
{
    VehicleRig        rig;
    const EnemyHandle h = SpawnJeepFacingThePlayer_(rig, { 500.f, kGround }, 300.f);
    SpawnLog          log;
    StepAndLog_(rig, h, log, 900);
    ASSERT_FALSE(log.shots.empty());
    const Shot& rocket = log.shots.front();
    EXPECT_LT(rocket.velocity.x, 0.f) << "플레이어(왼쪽)를 향해 뒤로 나간다";
    EXPECT_NEAR(rocket.velocity.y, 29.85f, 1.5f);
    EXPECT_EQ(rocket.params.startDelayTicks, 24);   // 점화 0.4 s
}

TEST(VehicleJeep, VolleyIsFourRocketsInTwoPairsWithPauses)
{
    VehicleRig        rig;
    const EnemyHandle h = SpawnJeepFacingThePlayer_(rig, { 500.f, kGround }, 300.f);
    SpawnLog          log;
    StepAndLog_(rig, h, log, 1500);
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
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing_(-1));
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
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing_(-1));
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
    (void)SpawnJeepFacingThePlayer_(rig, { 500.f, 300.f }, 300.f);
    rig.Step(1200);
    EXPECT_GE(rig.PlayerHits(), 1);
}

TEST(VehicleBTank, StaysClosedUntilThePlayerIsWithinFiveHundredPixels)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 1000.f, kGround }, Facing_(-1));
    rig.Step(200);
    EXPECT_FALSE(rig.E(h)->bActive);
    rig.SetPlayer({ 520.f, kGround });
    rig.Step(1);
    EXPECT_TRUE(rig.E(h)->bActive);
}

TEST(VehicleBTank, OpensThenFiresTwoShellsPerCycleEveryFourPointEightSeconds)
{
    VehicleRig        rig({ 520.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 1000.f, kGround }, Facing_(-1));
    SpawnLog          log;
    StepAndLog_(rig, h, log, 620);
    ASSERT_GE(log.ticks.size(), 4u);
    EXPECT_NEAR(log.ticks[1] - log.ticks[0], 53, 4) << "연발: 발사 클립 길이(0.88 s)만큼";
    EXPECT_NEAR(log.ticks[2] - log.ticks[0], 288, 10) << "사이클 4.8 s";
    EXPECT_NEAR(log.ticks[3] - log.ticks[2], 53, 4);
}

TEST(VehicleBTank, ShellFliesLeftwardAsALob)
{
    VehicleRig        rig({ 520.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 1000.f, kGround }, Facing_(-1));
    SpawnLog          log;
    StepAndLog_(rig, h, log, 300);
    ASSERT_FALSE(log.shots.empty());
    const Shot& shell = log.shots.front();
    EXPECT_NEAR(shell.velocity.x, -400.f, 1.f);
    EXPECT_LT(shell.velocity.y, -180.f);   // 발사 직후 위로
    EXPECT_NEAR(shell.position.x, 920.f - 400.f / 60.f, 1.5f) << "머즐은 전차 왼쪽 80px (첫 틱에 한 틱 전진)";
}

TEST(VehicleBTank, ShellThatPassesOverAStandingPlayerHitsIt)
{
    VehicleRig rig({ 520.f, kGround });
    (void)rig.Spawn("btank", { 1000.f, kGround }, Facing_(-1));
    rig.Step(400);
    EXPECT_GE(rig.PlayerHits(), 1);
}

TEST(VehicleBTank, KnifeDoesNotHurtTheTankButBulletsKillItOnTheFifteenth)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 400.f, kGround }, Facing_(-1));
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
    for (const char* pName: { "jeep", "btank", "missle_tank", "sarubia" })
    {
        VehicleRig        rig({ 100.f, kGround });
        const EnemyHandle h = rig.Spawn(pName, { 400.f, kGround }, Facing_(-1));
        rig.Step(2);
        EXPECT_EQ(rig.Strike(h.id, eHitKind::Melee, 8), 0u) << pName;
        EXPECT_TRUE(rig.E(h)->IsAlive()) << pName;
    }
}

TEST(VehicleBTank, DestructionRemovesItAfterFourSeconds)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("btank", { 400.f, kGround }, Facing_(-1));
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
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 380.f, kGround }, Facing_(-1));   // dx = 280 (<= 300: 직사)
    SpawnLog          log;
    StepAndLog_(rig, h, log, 400);
    EXPECT_EQ(log.ticks.size(), 5u);
    for (const Shot& shot: log.shots)
    {
        EXPECT_NEAR(shot.velocity.y, 6.7f, 8.f) << "직사: 수직 속도 0 (중력 1 틱분만)";
        EXPECT_NEAR(shot.velocity.x, -300.f, 1.f);
    }
}

TEST(VehicleMissleTank, AimsUpForFarTargetsAndTheVolleyArcs)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 500.f, kGround }, Facing_(-1));   // dx = 400 (> 300: 곡사)
    SpawnLog          log;
    StepAndLog_(rig, h, log, 500);
    ASSERT_FALSE(log.shots.empty());
    EXPECT_LT(log.shots.front().velocity.y, -150.f);
    EXPECT_NEAR(log.shots.front().velocity.x, -300.f, 1.f);
}

TEST(VehicleMissleTank, OnlyTicksWhileThePlayerIsWithinSixHundredPixels)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 900.f, kGround }, Facing_(-1));   // dx = 800
    SpawnLog          log;
    StepAndLog_(rig, h, log, 800);
    EXPECT_TRUE(log.ticks.empty());
}

TEST(VehicleMissleTank, DestructionIsSilentUntilTwoSecondsLater)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("missle_tank", { 380.f, kGround }, Facing_(-1));
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
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("sarubia", { 700.f, kGround }, Facing_(-1));   // dx = -600
    rig.Step(200);
    const float x0 = rig.E(h)->body.position.x;
    rig.Step(30);
    const float x1 = rig.E(h)->body.position.x;
    EXPECT_NEAR(x0 - x1, 50.f, 4.f);
}

TEST(VehicleSarubia, GrenadeFallsStraightDownThenRollsAlongTheGround)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("sarubia", { 380.f, kGround }, Facing_(-1));   // dx = -280: 사격 구간
    SpawnLog          log;
    StepAndLog_(rig, h, log, 260);
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

    rig.Step(10);
    const float x0 = GrenadeX_(rig, h);
    rig.Step(60);
    const float x1 = GrenadeX_(rig, h);
    EXPECT_NEAR(x0 - x1, 100.f, 6.f);
}

TEST(VehicleSarubia, GrenadeRollsIntoAPlayerOnItsPath)
{
    VehicleRig rig({ 100.f, kGround });
    (void)rig.Spawn("sarubia", { 380.f, kGround }, Facing_(-1));
    rig.Step(600);
    EXPECT_GE(rig.PlayerHits(), 1);
}

TEST(VehicleSarubia, DeathExplodesAtOnceAndLeavesAfterTwoSeconds)
{
    VehicleRig        rig({ 100.f, kGround });
    const EnemyHandle h = rig.Spawn("sarubia", { 700.f, kGround }, Facing_(-1));
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

namespace
{
// 드롭 발사 시점의 스폰 위치가 헬기 기준 (_offsetX, +60) 인지 확인하고 드롭 발수를 누적한다
void CheckDropSpawnOffsets_(
    VehicleRig&       _rig,
    const EnemyHandle _h,
    const float       _offsetX,
    const int         _steps,
    int&              _outDrops)
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
}   // namespace

TEST(VehicleHitFlash, JeepFlashesOnNonLethalHitBlinksUnderFireAndExpires)
{
    VehicleRig rig({ 100.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing_(-1));
    ASSERT_NE(rig.E(h), nullptr);
    EXPECT_TRUE(rig.Lib().Get(rig.E(h)->defIndex).bHitFlash) << "hp 80 차량은 기본 플래시 대상";
    rig.Step(2);
    ASSERT_EQ(rig.E(h)->hitFlashTicks, 0);

    ASSERT_GT(rig.Strike(h.id, eHitKind::Bullet, 5), 0u);
    EXPECT_EQ(rig.E(h)->hitFlashTicks, kHitFlashTicks);
    EXPECT_TRUE(rig.E(h)->IsAlive());

    rig.Step(1);
    EXPECT_TRUE(IsHitFlashOn(rig.E(h)->hitFlashTicks)) << "피격 다음 틱부터 켜져 있다";
    EXPECT_FLOAT_EQ(HitFlashAmount(*rig.E(h)), 1.f);

    rig.Step(kHitFlashOnTicks);
    EXPECT_FALSE(IsHitFlashOn(rig.E(h)->hitFlashTicks)) << "ON 구간이 끝나면 꺼진다 (계속 켜져 있으면 안 된다)";
    EXPECT_FLOAT_EQ(HitFlashAmount(*rig.E(h)), 0.f);

    // 연사: 4 틱 간격으로 쏘아도 반드시 꺼지는 틱이 나타난다 (버그: 계속 빨간 채로 유지)
    int onTicks  = 0;
    int offTicks = 0;
    for (int i = 0; i < 48; ++i)
    {
        if (i % 4 == 0)
        {
            ASSERT_GT(rig.Strike(h.id, eHitKind::Bullet, 1), 0u);
        }
        rig.Step(1);
        (HitFlashAmount(*rig.E(h)) > 0.f ? onTicks : offTicks) += 1;
    }
    EXPECT_GT(onTicks, 10);
    EXPECT_GT(offTicks, 5) << "연사 중에도 깜빡여야 한다";

    rig.Step(kHitFlashTicks);
    EXPECT_EQ(rig.E(h)->hitFlashTicks, 0);
}

TEST(VehicleHitFlash, LethalHitDoesNotFlash)
{
    VehicleRig rig({ 100.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("jeep", { 400.f, kGround }, Facing_(-1));
    rig.Step(2);
    ASSERT_GT(rig.Strike(h.id, eHitKind::Bullet, 100000), 0u);
    EXPECT_FALSE(rig.E(h)->IsAlive());
    EXPECT_EQ(rig.E(h)->hitFlashTicks, 0);
}

namespace
{
ShotSpawn MakeFallingShell_(
    const float _x,
    const float _groundProbe,
    const float _maxFall = 3000.f)
{
    ShotSpawn spawn;
    spawn.position        = { _x, 200.f };
    spawn.size            = { 30.f, 40.f };
    spawn.gravity         = 300.f;
    spawn.maxFallSpeed    = _maxFall;
    spawn.lifetimeTicks   = 600;
    spawn.bExplodeOnSolid = true;
    spawn.groundProbeY    = _groundProbe;
    spawn.visual          = 1;
    spawn.damage          = 1;
    return spawn;
}

// 착탄(FX) 이 나올 때까지 돌리고 그 지점을 돌려준다. 10 s 안에 없으면 false
bool RunUntilBurst_(
    ProjectileSystem&                _ps,
    CombatSystem&                    _combat,
    const GroundMap*                 _pGround,
    const slug::Span<const Platform> _platforms,
    Vec2&                            _outFx)
{
    for (int i = 0; i < 600; ++i)
    {
        _combat.BeginTick();
        _ps.Step(_pGround, _combat, kStepSec, _platforms);
        const slug::Vector<ShotFx> fx = _ps.TakeShotFx();
        if (!fx.empty())
        {
            _outFx = fx[0].position;
            return true;
        }
    }
    return false;
}
}   // namespace

TEST(VehicleSarubia, GrenadeRollsWithItsSpriteBottomOnTheGround)
{
    ProjectileSystem ps;
    CombatSystem     combat;
    const GroundMap  ground = MakeFlatGround(3000);
    ShotSpawn        spawn  = MakeFallingShell_(300.f, 14.f, 300.f);
    spawn.rollSpeed         = 100.f;
    spawn.rollDirX          = -1.f;
    spawn.rollTicks         = 300;
    ps.SpawnShot(spawn, combat);
    for (int i = 0; i < 120; ++i)
    {
        combat.BeginTick();
        ps.Step(&ground, combat, kStepSec);
    }
    ASSERT_EQ(ps.GetShots().size(), 1u);
    const Shot& shot = ps.GetShots()[0];
    EXPECT_TRUE(shot.bLanded);
    EXPECT_NEAR(shot.position.y, kVehicleGroundY - 14.f, 1.5f) << "구르는 수류탄은 스프라이트 바닥이 지면에 있다";
    EXPECT_LT(shot.position.x, 300.f) << "착지 후 바라보는 쪽(-x)으로 굴러간다";
}

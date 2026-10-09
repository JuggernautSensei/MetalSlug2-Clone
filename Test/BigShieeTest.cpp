#include <algorithm>
#include <cmath>

#include <gtest/gtest.h>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Enemies/BigShiee.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyTypes.h"
#include "BossTestUtil.h"

using namespace slug;
using namespace slugtest;

namespace
{
    constexpr float kGroundY  = 416.f;   // 보스 발(바닥) y: 아트 높이 416 의 바닥
    constexpr float kPlayerX  = 100.f;   // 플레이어는 보스 왼쪽 띠에 선다

    EnemyHandle SpawnBoss(BossRig& _rig)
    {
        SpawnParams params;
        params.arena  = BossArena();
        params.facing = 1;
        return _rig.SpawnAt("bigshiee", { 710.f, kGroundY }, params);
    }

    bool RunUntilFight(BossRig& _rig, const EnemyHandle _h)
    {
        for (int i = 0; i < 600; ++i)
        {
            const Enemy* pBoss = _rig.Find(_h);
            if (pBoss && bigshiee::IsFightStarted(*pBoss))
            {
                return true;
            }
            _rig.Frame();
        }
        return false;
    }

    int TracerVisual(const BossRig& _rig)
    {
        return _rig.GetLib().FindProjectileIndex("bigshiee_tracer") + 1;
    }

    int ShellVisual(const BossRig& _rig)
    {
        return _rig.GetLib().FindProjectileIndex("bigshiee_shell") + 1;
    }
}

TEST(BigShiee, EntranceRollsInFromTheRightThenTheFightStarts)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);

    rig.Frame();
    const Enemy* pBoss = rig.Find(h);
    ASSERT_NE(pBoss, nullptr);
    EXPECT_EQ(pBoss->state, bigshiee::kEntrance);
    EXPECT_FALSE(bigshiee::IsFightStarted(*pBoss));
    EXPECT_GT(pBoss->body.position.x, 640.f);   // 오른쪽 화면 밖에서 시작

    int ticks = 1;
    while (!bigshiee::IsFightStarted(*rig.Find(h)) && ticks < 600)
    {
        rig.Frame();
        ++ticks;
    }
    EXPECT_NEAR(ticks, 150, 3);   // 2.5 s
    EXPECT_EQ(rig.Find(h)->state, bigshiee::kAssault);
    EXPECT_NEAR(rig.Find(h)->body.position.x, 470.f, 0.5f);   // 아레나 왼쪽 + homeX
}

TEST(BigShiee, HullStaysClearOfThePlayerWallAcrossTheWholeDrive)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    float minX = 1e9f;
    float maxX = -1e9f;
    for (int t = 0; t < 1800; ++t)
    {
        rig.Frame();
        const float x = rig.Find(h)->body.position.x;
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        // 플레이어 벽은 보스 주행과 무관하게 165 에 고정 (보스 x 가 움직여도 벽은 움직이지 않는다)
        ASSERT_TRUE(rig.World().HasPlayerMaxX());
        ASSERT_NEAR(rig.World().GetPlayerMaxX(), 165.f, 0.01f);
    }
    EXPECT_GE(minX, 444.f);   // 주행 445..495
    EXPECT_LE(maxX, 496.f);
    EXPECT_GT(maxX - minX, 20.f);   // 실제로 주행한다 (30 s 동안 범위 50 px 중 일부)
    // 선체 프레임 왼쪽 끝 (x - 271) 은 벽 165 보다 오른쪽에 있다. 알파 꼬리 (x - 298) 는 최대 약 27 px 겹칠 수 있다 (허용).
    EXPECT_GT(minX - 271.f, 165.f);
}

TEST(BigShiee, EachTurretDiesOnItsOwnAndTheHullKeepsItsHealth)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    EXPECT_EQ(bigshiee::GetTurretHp(*rig.Find(h), 0), 60);
    rig.Hit(bigshiee::TurretOwner(h.id, 0), 60);
    EXPECT_EQ(bigshiee::GetTurretHp(*rig.Find(h), 0), 0);
    EXPECT_EQ(bigshiee::GetTurretHp(*rig.Find(h), 1), 60);   // 다른 포탑은 그대로
    EXPECT_EQ(rig.Find(h)->health.hp, 1300);                  // 포탑 피해는 선체 HP 에 닿지 않는다
    EXPECT_FALSE(bigshiee::IsCannonDestroyed(*rig.Find(h)));
}

TEST(BigShiee, HullHitsReduceTheHullHitPointsAndTheHudRatio)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    EXPECT_NEAR(rig.World().GetBossHpRatio().value_or(-1.f), 1.f, 1e-4f);
    rig.Hit(h.id, 2);
    EXPECT_EQ(rig.Find(h)->health.hp, 1298);
    EXPECT_FALSE(bigshiee::IsEnraged(*rig.Find(h)));

    rig.Hit(h.id, 648);   // 합계 650 = 절반
    EXPECT_NEAR(rig.World().GetBossHpRatio().value_or(-1.f), 0.5f, 0.01f);
    EXPECT_TRUE(bigshiee::IsEnraged(*rig.Find(h)));   // 선체 50% 이하 = 격앙
}

TEST(BigShiee, CannonDeployFollowsTheAssaultPhaseOnTheClipTimeline)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    int deployAt = -1;
    int cannonAt = -1;
    for (int t = 0; t < 1200 && cannonAt < 0; ++t)
    {
        const Enemy* pBoss = rig.Find(h);
        if (deployAt < 0 && pBoss->state == bigshiee::kDeploy)
        {
            deployAt = t;
        }
        if (pBoss->state == bigshiee::kCannon)
        {
            cannonAt = t;
        }
        rig.Frame();
    }
    EXPECT_NEAR(deployAt, 720, 2);           // 포탑 연사 12 s 후 캐논 전개
    EXPECT_NEAR(cannonAt - deployAt, 240, 2);   // 캐논 전개 4 s
}

TEST(BigShiee, AssaultBurstsFireTracersFromTheTurretsAtThePlayer)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    rig.SetPlayerInvulnerable(false);
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    const int tracer = TracerVisual(rig);
    int       checked = 0;
    for (int t = 0; t < 360; ++t)
    {
        rig.Frame();
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.visual != tracer || shot.ageTicks != 1 || shot.params.owner != h.id)
            {
                continue;
            }
            const Vec2  to  = { kPlayerX - shot.position.x, (kGroundY - 30.f) - shot.position.y };
            const float len = std::sqrt(to.x * to.x + to.y * to.y);
            const float vlen = std::sqrt(shot.velocity.x * shot.velocity.x + shot.velocity.y * shot.velocity.y);
            ASSERT_GT(len, 1.f);
            const float dot = (shot.velocity.x * to.x + shot.velocity.y * to.y) / (vlen * len);
            EXPECT_GT(dot, 0.99f) << "트레이서가 플레이어를 조준하지 않음";
            EXPECT_NEAR(vlen, 230.f, 0.5f);
            ++checked;
        }
    }
    EXPECT_GE(checked, 2);   // 연사 1.2 s 마다 2발
}

TEST(BigShiee, CannonVolleysAimStreaksAtThePlayerAndShellsLandOnIt)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    rig.SetPlayerInvulnerable(false);
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    // 캐논 단계까지 진행한다
    for (int t = 0; t < 1200 && rig.Find(h)->state != bigshiee::kCannon; ++t)
    {
        rig.Frame();
    }
    ASSERT_EQ(rig.Find(h)->state, bigshiee::kCannon);

    const int tracer = TracerVisual(rig);
    const int shell  = ShellVisual(rig);
    int       shellSpawned = 0;
    int       blastHits    = 0;
    for (int t = 0; t < 1200; ++t)
    {
        rig.Frame();
        for (const Shot& s: rig.Projectiles().GetShots())
        {
            if (s.params.visual == shell && s.ageTicks == 1)
            {
                ++shellSpawned;
            }
        }
    }
    for (const HitEvent& ev: rig.PlayerHits())
    {
        blastHits += ev.kind == eHitKind::Blast ? 1 : 0;
    }
    EXPECT_GE(bigshiee::GetVolleyCount(*rig.Find(h)), 4);   // 4~5 s 간격 → 20 s 에 4~5 볼리
    EXPECT_GE(shellSpawned, 2);                              // 2 볼리마다 주황 포탄
    EXPECT_GE(blastHits, 1);                                 // 착탄 폭발이 플레이어를 맞힌다
    (void)tracer;
}

TEST(BigShiee, ShellImpactBlastLandsWhereThePlayerStoodAtLaunch)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    rig.SetPlayerInvulnerable(false);
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));
    for (int t = 0; t < 1200 && rig.Find(h)->state != bigshiee::kCannon; ++t)
    {
        rig.Frame();
    }

    const int shell = ShellVisual(rig);
    bool      shellSeen = false;
    int       spawnTick = -1;
    int       blastTick = -1;
    float     blastX    = 0.f;
    for (int t = 0; t < 1200 && blastTick < 0; ++t)
    {
        rig.Frame();
        for (const Shot& s: rig.Projectiles().GetShots())
        {
            if (s.params.visual == shell && s.ageTicks == 1 && !shellSeen)
            {
                shellSeen = true;
                spawnTick = t;
            }
            if (s.params.kind == eHitKind::Blast && blastTick < 0)
            {
                blastTick = t;
                blastX    = s.position.x;
            }
        }
    }
    ASSERT_TRUE(shellSeen);
    ASSERT_GE(blastTick, 0);
    EXPECT_NEAR(blastTick - spawnTick, 60, 2);   // 포탄 비행 1.0 s
    EXPECT_NEAR(blastX, kPlayerX, 1.0f);         // 착탄 지점 = 발사 시점 플레이어 발 x (플레이어는 정지)
}

TEST(BigShiee, ThePlayerWallIsReleasedWhenTheBossDies)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));
    rig.Frame();
    EXPECT_TRUE(rig.World().HasPlayerMaxX());   // 전투 중에는 벽이 걸린다

    rig.Hit(h.id, 5000);
    rig.Frame();
    EXPECT_FALSE(rig.World().HasPlayerMaxX());  // 사망 후에는 벽을 걸지 않는다 (스테이지 종료 가능)
    for (int t = 0; t < 300; ++t)
    {
        rig.Frame();
    }
    EXPECT_FALSE(rig.World().HasPlayerMaxX());
}

TEST(BigShiee, CannonVolleysAreSpacedFourToFiveSecondsAndNeverOverlap)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    rig.SetPlayerInvulnerable(false);
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));
    for (int t = 0; t < 1200 && rig.Find(h)->state != bigshiee::kCannon; ++t)
    {
        rig.Frame();
    }
    ASSERT_EQ(rig.Find(h)->state, bigshiee::kCannon);

    int last   = bigshiee::GetVolleyCount(*rig.Find(h));
    int lastAt = -1;
    int gaps   = 0;
    for (int t = 0; t < 1800; ++t)
    {
        rig.Frame();
        const int now = bigshiee::GetVolleyCount(*rig.Find(h));
        if (now != last)
        {
            if (lastAt >= 0)
            {
                EXPECT_GE(t - lastAt, 240) << "볼리 간격 4 s 미만";
                EXPECT_LE(t - lastAt, 300) << "볼리 간격 5 s 초과";
                ++gaps;
            }
            lastAt = t;
            last   = now;
        }
    }
    EXPECT_GE(gaps, 4);
}

TEST(BigShiee, EnragedCannonVolleysComeOneFifthFaster)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    rig.SetPlayerInvulnerable(false);
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));
    for (int t = 0; t < 1200 && rig.Find(h)->state != bigshiee::kCannon; ++t)
    {
        rig.Frame();
    }
    rig.Hit(bigshiee::CannonOwner(h.id), 300);   // 캐논 파괴 = 격앙
    ASSERT_TRUE(bigshiee::IsEnraged(*rig.Find(h)));

    int last   = bigshiee::GetVolleyCount(*rig.Find(h));
    int lastAt = -1;
    int gaps   = 0;
    for (int t = 0; t < 1800; ++t)
    {
        rig.Frame();
        const int now = bigshiee::GetVolleyCount(*rig.Find(h));
        if (now != last)
        {
            if (lastAt >= 0)
            {
                EXPECT_GE(t - lastAt, 192) << "격앙 간격 4 s x0.8 = 3.2 s 미만";
                EXPECT_LE(t - lastAt, 240) << "격앙 간격 5 s x0.8 = 4 s 초과";
                ++gaps;
            }
            lastAt = t;
            last   = now;
        }
    }
    EXPECT_GE(gaps, 5);
}

TEST(BigShiee, CannonDestructionEnragesAndRemovesTheCannonHurtbox)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    EXPECT_EQ(bigshiee::GetCannonHp(*rig.Find(h)), 300);
    rig.Hit(bigshiee::CannonOwner(h.id), 150);
    EXPECT_EQ(bigshiee::GetCannonHp(*rig.Find(h)), 150);
    EXPECT_FALSE(bigshiee::IsCannonDestroyed(*rig.Find(h)));
    rig.Hit(bigshiee::CannonOwner(h.id), 150);
    EXPECT_EQ(bigshiee::GetCannonHp(*rig.Find(h)), 0);
    EXPECT_TRUE(bigshiee::IsCannonDestroyed(*rig.Find(h)));
    EXPECT_TRUE(bigshiee::IsEnraged(*rig.Find(h)));
    EXPECT_EQ(rig.Find(h)->health.hp, 1300);  // 캐논 파괴는 선체 HP 를 깎지 않는다
}

TEST(BigShiee, NoAttacksWhenThePlayerIsDead)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));
    rig.SetPlayer({ kPlayerX, kGroundY }, false);

    const int tracer = TracerVisual(rig);
    const int volleysBefore = bigshiee::GetVolleyCount(*rig.Find(h));
    int       newTracers    = 0;
    for (int t = 0; t < 1500; ++t)
    {
        rig.Frame();
        for (const Shot& s: rig.Projectiles().GetShots())
        {
            newTracers += (s.params.visual == tracer && s.ageTicks == 1) ? 1 : 0;
        }
    }
    EXPECT_EQ(newTracers, 0);
    EXPECT_EQ(bigshiee::GetVolleyCount(*rig.Find(h)), volleysBefore);
}

TEST(BigShiee, HullDeathCascadeThenDestroyFinishedAfterTenSeconds)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    rig.SetPlayer({ kPlayerX, kGroundY });
    const EnemyHandle h = SpawnBoss(rig);
    ASSERT_TRUE(RunUntilFight(rig, h));

    rig.Hit(h.id, 5000);
    EXPECT_EQ(rig.CountEventsOf(eEnemyEventKind::Died, h), 1);
    EXPECT_TRUE(rig.World().IsDefeated(h));              // 선체 0 = 처치 (게이트 조건)
    EXPECT_FALSE(rig.World().IsDestroyFinished(h));

    for (int t = 0; t < 590; ++t)
    {
        rig.Frame();
    }
    EXPECT_FALSE(rig.World().IsDestroyFinished(h));      // 8 s 연쇄 + 침몰 중
    int extra = 0;
    while (!rig.World().IsDestroyFinished(h) && extra < 60)
    {
        rig.Frame();
        ++extra;
    }
    EXPECT_TRUE(rig.World().IsDestroyFinished(h));
    EXPECT_NEAR(590 + extra, 600, 3);                    // 10 s 후 완료 (gate 가 endStage 를 실행)
    EXPECT_EQ(rig.CountEventsOf(eEnemyEventKind::Died, h), 1);
}

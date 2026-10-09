#include <algorithm>
#include <cmath>
#include <vector>

#include "EnemyTestUtil.h"

using namespace slugtest;

namespace
{
    constexpr float kGroundY = 378.f;   // MakeFlatGround 의 지면 윗면

    bool OnGround(const Enemy* _pEnemy)
    {
        return _pEnemy && _pEnemy->bGrounded && std::abs(_pEnemy->body.position.y - kGroundY) < 0.5f;
    }
}   // namespace

// ===========================================
//  스폰 / 활성 / 정리
// ===========================================

TEST(EnemyWorld, SpawningAnUnknownDefinitionReturnsANullHandle)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    EXPECT_TRUE(rig.Spawn("nope", { 300.f, kGroundY }).IsNull());
    EXPECT_EQ(rig.World().GetCount(), 0u);
}

TEST(EnemyWorld, IdsStartAtTheEnemyOwnerBaseAndIncrease)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle a = rig.Spawn("soldier_bomb", { 300.f, kGroundY });
    const EnemyHandle b = rig.Spawn("soldier_bomb", { 320.f, kGroundY });
    EXPECT_EQ(a.id, kEnemyOwnerBase);
    EXPECT_EQ(b.id, kEnemyOwnerBase + 1);
    EXPECT_TRUE(IsEnemyOwner(a.id));
    EXPECT_TRUE(IsEnemyOwner(b.id | kShieldOwnerBit));
    EXPECT_FALSE(IsEnemyOwner(1u));
}

TEST(EnemyWorld, SoldierFallsOntoTheGroundAndIdles)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, 300.f });
    rig.Step(60);
    EXPECT_TRUE(OnGround(rig.E(h)));
    EXPECT_TRUE(rig.E(h)->bActive);
}

TEST(EnemyWorld, SpawnSnapsOntoGroundWithinReach)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, kGroundY - 6.f });   // 16 px 이내 → 지면에 붙는다
    ASSERT_NE(rig.E(h), nullptr);
    EXPECT_NEAR(rig.E(h)->body.position.y, kGroundY, 0.01f);
}

TEST(EnemyWorld, ActivationWaitsForTheScreenThenLatches)
{
    EnemyRig rig({ 100.f, kGroundY }, 0.f);   // 화면 [0, 640]: 오른쪽 여유 150 → x ≤ 790 이면 활성
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 1000.f, kGroundY });
    rig.Step(30);
    EXPECT_FALSE(rig.E(h)->bActive);

    rig.SetView(300.f);   // 화면 [300, 940] → 1000 ≤ 1090
    rig.Step(1);
    EXPECT_TRUE(rig.E(h)->bActive);

    rig.SetView(0.f);     // 다시 멀어져도 활성은 유지된다 (래치)
    rig.Step(10);
    EXPECT_TRUE(rig.E(h)->bActive);
}

TEST(EnemyWorld, InactiveMissileSoldierDoesNotFireWhileOffscreen)
{
    EnemyRig rig({ 100.f, kGroundY }, 0.f);
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("soldier_missile", { 1000.f, kGroundY });
    rig.Step(400);
    EXPECT_FALSE(rig.E(h)->bActive);
    EXPECT_EQ(rig.Projectiles().CountShots(h.id), 0);
}

TEST(EnemyWorld, EnemyBehindTheCameraIsRemovedWithARemovedEvent)
{
    EnemyRig rig({ 100.f, kGroundY }, 400.f);   // 화면 [400, 1040]: 뒤쪽 한계 = 200
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 100.f, kGroundY });
    rig.Step(1);
    EXPECT_EQ(rig.E(h), nullptr);
    EXPECT_TRUE(rig.World().IsDefeated(h));
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Removed), 1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);
}

TEST(EnemyWorld, DelayedSpawnHasNoHurtboxUntilItAppears)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    SpawnParams params;
    params.delaySec = 0.5f;
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, kGroundY }, params);
    EXPECT_FALSE(rig.World().IsDefeated(h));   // 대기 중이라도 "처치됨" 은 아니다
    EXPECT_EQ(rig.Strike(h.id, eHitKind::Bullet, 3), 0u);   // 대기 중에는 맞지 않는다
    rig.Step(40);
    ASSERT_NE(rig.E(h), nullptr);
    EXPECT_EQ(rig.E(h)->life, eEnemyLife::Alive);
}

TEST(EnemyWorld, CountAliveAndTagDefeatFollowTheSpawns)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    SpawnParams tagged;
    tagged.userTag = 5;
    const EnemyHandle a = rig.Spawn("soldier_bomb", { 300.f, kGroundY }, tagged);
    const EnemyHandle b = rig.Spawn("soldier_bomb", { 340.f, kGroundY }, tagged);
    EXPECT_EQ(rig.World().CountAlive("soldier_bomb"), 2);
    EXPECT_FALSE(rig.World().IsTagDefeated(5));
    rig.Step(2);
    (void)rig.Strike(a.id, eHitKind::Bullet, 3);
    (void)rig.Strike(b.id, eHitKind::Melee, 8);
    rig.Step(1);
    EXPECT_EQ(rig.World().CountAlive("soldier_bomb"), 0);
    EXPECT_TRUE(rig.World().IsTagDefeated(5));
}

TEST(EnemyWorld, KillAllAwardsScoreOnlyWhenAsked)
{
    {
        EnemyRig rig;
        ASSERT_TRUE(rig.IsLoaded());
        (void)rig.Spawn("soldier_bomb", { 300.f, kGroundY });
        (void)rig.Spawn("soldier_sword", { 340.f, kGroundY });
        rig.World().KillAll(true);
        EXPECT_EQ(rig.World().GetCount(), 0u);
        rig.Step(1);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 2);
        EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 200);
    }
    {
        EnemyRig rig;
        ASSERT_TRUE(rig.IsLoaded());
        (void)rig.Spawn("soldier_bomb", { 300.f, kGroundY });
        rig.World().KillAll(false);
        rig.Step(1);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 1);
        EXPECT_EQ(rig.SumScore(eEnemyEventKind::Died), 0);
    }
}

TEST(EnemyWorld, ClearMonstersKeepsNeutralNpcsUnlessAsked)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle soldier = rig.Spawn("soldier_bomb", { 300.f, kGroundY });
    const EnemyHandle pow     = rig.Spawn("pow", { 340.f, kGroundY });
    rig.World().ClearMonsters(false);
    EXPECT_TRUE(rig.World().IsDefeated(soldier));
    EXPECT_FALSE(rig.World().IsDefeated(pow));
    rig.Step(1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Removed), 1);

    rig.World().ClearMonsters(true);
    EXPECT_EQ(rig.World().GetCount(), 0u);
}

TEST(EnemyWorld, CancelPendingRemovesOnlyDelayedSpawns)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    SpawnParams later;
    later.delaySec = 2.f;
    (void)rig.Spawn("soldier_bomb", { 300.f, kGroundY }, later);
    const EnemyHandle now = rig.Spawn("soldier_bomb", { 340.f, kGroundY });
    rig.World().CancelPending();
    EXPECT_EQ(rig.World().GetCount(), 1u);
    EXPECT_NE(rig.E(now), nullptr);
}

TEST(EnemyWorld, ResetClearsEverythingAndRestartsIds)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    (void)rig.Spawn("soldier_bomb", { 300.f, kGroundY });
    rig.World().Reset(1);
    EXPECT_EQ(rig.World().GetCount(), 0u);
    EXPECT_EQ(rig.Spawn("soldier_bomb", { 300.f, kGroundY }).id, kEnemyOwnerBase);
}

// ===========================================
//  피해 / 사망
// ===========================================

TEST(EnemyWorld, PistolAndKnifeEachKillASoldierInOneHit)
{
    {
        EnemyRig rig;
        ASSERT_TRUE(rig.IsLoaded());
        const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, kGroundY });
        rig.Step(2);
        EXPECT_GT(rig.Strike(h.id, eHitKind::Bullet, 3), 0u);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 1);
        EXPECT_TRUE(rig.World().IsDefeated(h));
        EXPECT_EQ(rig.E(h)->life, eEnemyLife::Dying);
    }
    {
        EnemyRig rig;
        ASSERT_TRUE(rig.IsLoaded());
        const EnemyHandle h = rig.Spawn("soldier_sword", { 300.f, kGroundY });
        rig.Step(2);
        (void)rig.Strike(h.id, eHitKind::Melee, 8);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 1);
        EXPECT_EQ(rig.E(h)->lastHitKind, eHitKind::Melee);
    }
}

TEST(EnemyWorld, HitCountFollowsDamageAgainstAHighHp)
{
    // HP 20 짜리 보병: 권총(3) 7발, 칼(8) 3회, 폭탄(20) 1발
    const auto HitsToKill = [](const eHitKind _kind, const int _damage) {
        EnemyRig rig;
        if (!rig.IsLoaded())
        {
            return -1;
        }
        SpawnParams params;
        params.hp           = 20;
        const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, kGroundY }, params);
        rig.Step(2);
        for (int hits = 1; hits <= 20; ++hits)
        {
            (void)rig.Strike(h.id, _kind, _damage);
            rig.Step(1);
            if (rig.World().IsDefeated(h))
            {
                return hits;
            }
        }
        return -2;
    };
    EXPECT_EQ(HitsToKill(eHitKind::Bullet, 3), 7);
    EXPECT_EQ(HitsToKill(eHitKind::Melee, 8), 3);
    EXPECT_EQ(HitsToKill(eHitKind::Blast, 20), 1);
}

TEST(EnemyWorld, ShieldBlocksBulletsButNotMelee)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("shielder", { 300.f, kGroundY });
    rig.Step(10);
    ASSERT_TRUE(rig.E(h)->bShieldUp);

    // 방패 상자 (소유자 = id | kShieldOwnerBit) 를 총알이 때린다 → 막힌다 (피해 없음)
    EXPECT_GT(rig.Strike(h.id | kShieldOwnerBit, eHitKind::Bullet, 3), 0u);
    EXPECT_EQ(rig.E(h)->health.hp, 1);
    EXPECT_FALSE(rig.World().IsDefeated(h));
    rig.Step(1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);

    // 칼은 방패 상자가 받지 않는다 → 몸통이 맞아 죽는다
    (void)rig.Strike(h.id, eHitKind::Melee, 8);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 1);
}

TEST(EnemyWorld, PowIsFreedByAnyPlayerHitWithoutDying)
{
    {
        EnemyRig rig;
        ASSERT_TRUE(rig.IsLoaded());
        const EnemyHandle pow = rig.Spawn("pow", { 300.f, kGroundY });
        rig.Step(10);
        ASSERT_EQ(rig.E(pow)->state, 0);   // kStuck
        (void)rig.Strike(pow.id, eHitKind::Melee, 8);
        rig.Step(1);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Freed), 1);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);
        EXPECT_FALSE(rig.World().IsDefeated(pow));
        EXPECT_NE(rig.E(pow)->state, 0);   // Escape 로 전환
    }
    {
        EnemyRig rig;
        ASSERT_TRUE(rig.IsLoaded());
        const EnemyHandle pow = rig.Spawn("pow", { 300.f, kGroundY });
        rig.Step(10);
        (void)rig.Strike(pow.id, eHitKind::Bullet, 3);
        rig.Step(1);
        EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Freed), 1);
    }
}

TEST(EnemyWorld, PowIgnoresEnemyBulletsAndHasNoHurtboxOnceFreed)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle pow = rig.Spawn("pow", { 300.f, kGroundY });
    rig.Step(10);
    EXPECT_EQ(rig.Strike(pow.id, eHitKind::Bullet, 1, eTeam::Enemy), 0u);   // 적 탄은 중립 포로에게 닿지 않는다

    (void)rig.Strike(pow.id, eHitKind::Melee, 8);   // 풀려난다
    EXPECT_EQ(rig.Strike(pow.id, eHitKind::Melee, 8), 0u);   // 풀린 뒤에는 Hurtbox 가 없다
    rig.Step(1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Freed), 1);
}

// ===========================================
//  변신 / 공격
// ===========================================

TEST(EnemyWorld, FlyingBzkLandsAndTransformsIntoSoldierBzk)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle para = rig.Spawn("flying_bzk", { 300.f, 150.f });
    rig.Step(600);

    const EnemyHandle heir = rig.World().ResolveHandle(para);
    ASSERT_NE(heir.id, para.id) << "낙하산 병이 착지하지 않았다";
    const Enemy* pHeir = rig.E(heir);
    ASSERT_NE(pHeir, nullptr);
    EXPECT_EQ(rig.Lib().Get(pHeir->defIndex).name, "soldier_bzk");
    EXPECT_EQ(rig.E(para), nullptr);                    // 원래 개체는 사라졌다
    EXPECT_FALSE(rig.World().IsDefeated(para));         // 후계자를 따라가면 아직 살아 있다
    EXPECT_FALSE(rig.World().IsDefeated(heir));
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Transformed), 1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Removed), 0);   // 변신은 Removed 를 내지 않는다
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);
}

TEST(EnemyWorld, FlyingBzkFiresOnlyWhileInsideTheScreen)
{
    EnemyRig rig({ 100.f, kGroundY }, 0.f);
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("flying_bzk", { 300.f, -200.f });   // 화면 위쪽 밖
    rig.Step(100);
    EXPECT_EQ(rig.Projectiles().CountShots(h.id), 0);
}

TEST(EnemyWorld, SwordSoldierHitsThePlayerTeamWhenAdjacent)
{
    EnemyRig rig({ 100.f, kGroundY }, 0.f);
    ASSERT_TRUE(rig.IsLoaded());
    (void)rig.Spawn("soldier_sword", { 145.f, kGroundY });   // 플레이어 바로 옆 (칼 판정이 닿는 거리)
    for (int i = 0; i < 900 && rig.PlayerHits() == 0; ++i)
    {
        rig.Step(1);
    }
    EXPECT_GT(rig.PlayerHits(), 0) << "칼 병사가 플레이어를 한 번도 베지 않았다";
}

// ===========================================
//  결정성
// ===========================================

TEST(EnemyWorld, OneThousandEightHundredTicksAreDeterministic)
{
    const auto Run = [](std::vector<uint64_t>& _hashes) {
        EnemyRig rig({ 100.f, kGroundY }, 0.f);
        if (!rig.IsLoaded())
        {
            return false;
        }
        float x = 200.f;
        for (const char* name: kInfantryNames)
        {
            (void)rig.Spawn(name, { x, kGroundY });
            x += 60.f;
        }
        for (int i = 0; i < 1800; ++i)
        {
            rig.Step(1);
            _hashes.push_back(rig.World().HashState());
        }
        return true;
    };
    std::vector<uint64_t> first;
    std::vector<uint64_t> second;
    ASSERT_TRUE(Run(first));
    ASSERT_TRUE(Run(second));
    EXPECT_EQ(first, second);

    std::vector<uint64_t> distinct = first;
    std::sort(distinct.begin(), distinct.end());
    distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());
    EXPECT_GT(distinct.size(), 100u);   // 실제로 상태가 변한다
}

TEST(EnemyWorld, PowSpawnedHighFallsStraightDownUntilLanding)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle pow = rig.Spawn("pow", { 300.f, 150.f });
    ASSERT_FALSE(pow.IsNull());
    int ticks = 0;
    while (!rig.E(pow)->bGrounded && ticks < 120)
    {
        rig.Step(1);
        EXPECT_FLOAT_EQ(rig.E(pow)->body.position.x, 300.f) << "tick " << ticks;
        ++ticks;
    }
    EXPECT_TRUE(rig.E(pow)->bGrounded);
    EXPECT_FLOAT_EQ(rig.E(pow)->body.position.y, kGroundY);
}

TEST(EnemyWorld, PowHitWhileFallingIsNotFreedAndStaysVertical)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle pow = rig.Spawn("pow", { 300.f, 150.f });
    rig.Step(5);
    ASSERT_FALSE(rig.E(pow)->bGrounded);
    EXPECT_EQ(rig.Strike(pow.id, eHitKind::Bullet, 1), 0u);
    EXPECT_EQ(rig.Strike(pow.id, eHitKind::Melee, 8), 0u);
    for (int i = 0; i < 120 && !rig.E(pow)->bGrounded; ++i)
    {
        rig.Step(1);
        EXPECT_FLOAT_EQ(rig.E(pow)->body.position.x, 300.f) << "tick " << i;
    }
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Freed), 0);
}

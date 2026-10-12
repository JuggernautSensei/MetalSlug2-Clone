// 적 월드의 스폰, 활성화, 피격, 처치 이벤트, 방패, POW, 피격 점멸과 1800틱 결정성을 검증한다.

#include <algorithm>
#include <cmath>
#include <vector>

#include "EnemyTestUtil.h"

constexpr float kGroundY = 378.f;   // MakeFlatGround 의 지면 윗면

namespace
{
[[nodiscard]] bool OnGround_(
    const Enemy* _pEnemy)
{
    return _pEnemy && _pEnemy->bGrounded && std::abs(_pEnemy->body.position.y - kGroundY) < 0.5f;
}

// 높은 체력(20)의 병사를 쓰러뜨리는 데 필요한 타격 횟수를 돌려준다 (로드 실패 -1, 미처치 -2)
[[nodiscard]] int HitsToKill_(
    const eHitKind _kind,
    const int      _damage)
{
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
}

// 보병 전 종류를 세워 두고 1800틱 동안 상태 해시를 기록한다 (로드 실패 시 false)
[[nodiscard]] bool RunHashes_(
    Vector<uint64_t>& _hashes)
{
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
}
}   // namespace

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
    EXPECT_TRUE(OnGround_(rig.E(h)));
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
    EnemyRig rig({ 100.f, kGroundY }, 0.f);
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 1000.f, kGroundY });
    rig.Step(30);
    EXPECT_FALSE(rig.E(h)->bActive);

    rig.SetView(300.f);   // 화면 [300, 940] → 1000 ≤ 1090
    rig.Step(1);
    EXPECT_TRUE(rig.E(h)->bActive);

    rig.SetView(0.f);   // 다시 멀어져도 활성은 유지된다 (래치)
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
    params.delaySec     = 0.5f;
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, kGroundY }, params);
    EXPECT_FALSE(rig.World().IsDefeated(h));
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
    tagged.userTag      = 5;
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
    EXPECT_EQ(HitsToKill_(eHitKind::Bullet, 3), 7);
    EXPECT_EQ(HitsToKill_(eHitKind::Melee, 8), 3);
    EXPECT_EQ(HitsToKill_(eHitKind::Blast, 20), 1);
}

TEST(EnemyWorld, ShieldBlocksBulletsButNotMelee)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("shielder", { 300.f, kGroundY });
    rig.Step(10);
    ASSERT_TRUE(rig.E(h)->bShieldUp);

    EXPECT_GT(rig.Strike(h.id | kShieldOwnerBit, eHitKind::Bullet, 3), 0u);
    EXPECT_EQ(rig.E(h)->health.hp, 1);
    EXPECT_FALSE(rig.World().IsDefeated(h));
    rig.Step(1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);

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

    (void)rig.Strike(pow.id, eHitKind::Melee, 8);            // 풀려난다
    EXPECT_EQ(rig.Strike(pow.id, eHitKind::Melee, 8), 0u);   // 풀린 뒤에는 Hurtbox 가 없다
    rig.Step(1);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Freed), 1);
}

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
    EXPECT_EQ(rig.E(para), nullptr);
    EXPECT_FALSE(rig.World().IsDefeated(para));
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

TEST(EnemyWorld, OneThousandEightHundredTicksAreDeterministic)
{
    Vector<uint64_t> first;
    Vector<uint64_t> second;
    ASSERT_TRUE(RunHashes_(first));
    ASSERT_TRUE(RunHashes_(second));
    EXPECT_EQ(first, second);

    Vector<uint64_t> distinct = first;
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

TEST(EnemyWorld, PowFallingIntoPlayerIsRescuedMidAirAndGivesItem)
{
    constexpr int kClose = 5;                 // ePowState::kClose (구출 후 아이템 연출)
    EnemyRig      rig({ 300.f, kGroundY });   // 플레이어가 POW 의 낙하 경로 아래에 서 있다
    ASSERT_TRUE(rig.IsLoaded());
    SpawnParams params;
    params.drop           = "HM";
    const EnemyHandle pow = rig.Spawn("pow", { 300.f, 150.f }, params);
    ASSERT_FALSE(pow.IsNull());
    ASSERT_FALSE(rig.E(pow)->bGrounded);

    int ticks = 0;
    while (rig.E(pow)->state != kClose && ticks < 120)
    {
        rig.Step(1);
        ++ticks;
    }
    ASSERT_EQ(rig.E(pow)->state, kClose) << "공중에서 플레이어와 겹쳤는데 구출되지 않았다";
    EXPECT_FALSE(rig.E(pow)->bGrounded) << "착지 전에 구출되어야 한다";
    EXPECT_LT(rig.E(pow)->body.position.y, kGroundY);

    for (int i = 0; i < 120 && rig.CountEvents(eEnemyEventKind::ItemDropped) == 0; ++i)
    {
        rig.Step(1);
    }
    EXPECT_GT(rig.CountEvents(eEnemyEventKind::ItemDropped), 0) << "구출 후 아이템이 나오지 않았다";
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Freed), 0);
}

TEST(EnemyHitFlash, SingleHitIsOneOnPulseThenOffUntilNextHit)
{
    int timer = 0;
    ASSERT_TRUE(StartHitFlash(timer));
    EXPECT_EQ(timer, kHitFlashTicks);

    int onTicks = 0;
    for (int i = 0; i < 20; ++i)
    {
        TickHitFlash(timer);
        if (IsHitFlashOn(timer))
        {
            ++onTicks;
        }
    }
    EXPECT_EQ(onTicks, kHitFlashOnTicks) << "피격 한 번 = 켜진 틱 kHitFlashOnTicks 개 (영상 약 2 프레임)";
    EXPECT_EQ(timer, 0);
    EXPECT_FALSE(IsHitFlashOn(0));
}

TEST(EnemyHitFlash, ContinuousFireKeepsBlinkingInsteadOfStayingOn)
{
    // 원작 HMG 연사 (영상 약 4~8 틱 간격). 이전 구현은 재피격마다 타이머를 채워 항상 ON 이었다.
    for (int interval: { 2, 3, 4, 6, 8 })
    {
        int  timer    = 0;
        int  onTicks  = 0;
        int  offTicks = 0;
        int  toggles  = 0;
        bool bPrevOn  = false;
        for (int tick = 0; tick < 120; ++tick)
        {
            if (tick % interval == 0)
            {
                (void)StartHitFlash(timer);
            }
            TickHitFlash(timer);
            const bool bOn = IsHitFlashOn(timer);
            (bOn ? onTicks : offTicks) += 1;
            if (tick > 0 && bOn != bPrevOn)
            {
                ++toggles;
            }
            bPrevOn = bOn;
        }
        EXPECT_GT(offTicks, 20) << "interval=" << interval << " 연사 중에도 꺼지는 틱이 있어야 한다";
        EXPECT_GT(onTicks, 20) << "interval=" << interval;
        EXPECT_GE(toggles, 10) << "interval=" << interval << " 깜빡임이 반복되어야 한다";
    }
}

TEST(EnemyHitFlash, HitDuringOnPulseIsIgnoredAndHitDuringGapRestarts)
{
    int timer = 0;
    ASSERT_TRUE(StartHitFlash(timer));
    TickHitFlash(timer);
    ASSERT_TRUE(IsHitFlashOn(timer));
    const int before = timer;
    EXPECT_FALSE(StartHitFlash(timer)) << "ON 중 재피격은 타이머를 늘리지 않는다";
    EXPECT_EQ(timer, before);

    while (IsHitFlashOn(timer))
    {
        TickHitFlash(timer);
    }
    EXPECT_GT(timer, 0) << "ON 이 끝나면 gap 구간";
    EXPECT_TRUE(StartHitFlash(timer)) << "gap 중 재피격은 새 펄스를 시작한다";
    EXPECT_EQ(timer, kHitFlashTicks);
}

TEST(EnemyHitFlash, OneHitDefinitionNeverFlashesEvenWhenSpawnedWithMoreHp)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    SpawnParams params;
    params.hp           = 5;
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 300.f, kGroundY }, params);
    rig.Step(2);
    ASSERT_GT(rig.Strike(h.id, eHitKind::Bullet, 1), 0u);
    EXPECT_FALSE(rig.Lib().Get(rig.World().Get(h)->defIndex).bHitFlash) << "hp 1 정의는 기본적으로 플래시하지 않는다";
    EXPECT_TRUE(rig.World().Get(h)->IsAlive());
    EXPECT_EQ(rig.World().Get(h)->hitFlashTicks, 0);
}

TEST(EnemyHitFlash, PartFlashIsPerPartAndIgnoresOutOfRangeIndices)
{
    Enemy e;
    TriggerPartHitFlash(e, 2);
    EXPECT_EQ(e.partFlashTicks[2], kHitFlashTicks);
    EXPECT_EQ(e.partFlashTicks[1], 0);
    TickHitFlash(e.partFlashTicks[2]);   // 피격은 Tick 앞에서 처리되므로 보이는 첫 값은 한 틱 뒤
    EXPECT_FLOAT_EQ(PartFlashAmount(e, 2), 1.f);
    EXPECT_FLOAT_EQ(PartFlashAmount(e, 1), 0.f);
    EXPECT_FLOAT_EQ(HitFlashAmount(e), 0.f);

    TriggerPartHitFlash(e, -1);
    TriggerPartHitFlash(e, kEnemyFlashPartCount);
    EXPECT_FLOAT_EQ(PartFlashAmount(e, kEnemyFlashPartCount), 0.f);
    EXPECT_FLOAT_EQ(PartFlashAmount(e, -1), 0.f);
}

TEST(EnemyWorld, PowVariantPicksItsOwnClipSetAndDefaultsToOlder)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyDefLibrary* pLib = rig.World().GetLibrary();
    ASSERT_NE(pLib, nullptr);

    SpawnParams variant;
    variant.aiVariant       = 2;   // 외형 변형 2 (Rumi 계열 클립)
    const EnemyHandle hBase = rig.Spawn("pow", { 300.f, kGroundY });
    const EnemyHandle hVar  = rig.Spawn("pow", { 500.f, kGroundY }, variant);
    rig.Step(1);

    const Enemy* pBase = rig.E(hBase);
    const Enemy* pVar  = rig.E(hVar);
    ASSERT_NE(pBase, nullptr);
    ASSERT_NE(pVar, nullptr);
    ASSERT_EQ(pBase->state, 0);   // kStuck
    ASSERT_EQ(pVar->state, 0);
    const EnemyDef& def = pLib->Get(pBase->defIndex);
    EXPECT_STREQ(def.clips[static_cast<size_t>(pBase->clip)].name.c_str(), "stuck");
    EXPECT_STREQ(def.clips[static_cast<size_t>(pVar->clip)].name.c_str(), "stuck_v2");
    EXPECT_EQ(pVar->aiVariant, 2);
}

// 낙하산 오버레이는 병사 몸(레이어 15) 뒤에 그려진다 (오버레이 레이어 14)
TEST(EnemyWorld, ParachuteOverlayIsDrawnBehindTheParachutist)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    (void)rig.Spawn("flying_bzk", { 300.f, 200.f });
    rig.Step(3);

    SpriteBatch sprites;
    rig.World().Submit(sprites, anims, rig.Projectiles(), 0.f);

    bool bBody    = false;   // 병사 몸 (kEnemyDepth = 15)
    bool bOverlay = false;   // 낙하산 (kOverlayDepth = 14)
    for (const SpriteCommand& command: sprites.GetPendingCommands())
    {
        bBody    = bBody || command.depth == 15;
        bOverlay = bOverlay || command.depth == 14;
        EXPECT_NE(command.depth, 16) << "낙하산이 병사 앞에 그려진다 (명령 " << sprites.GetPendingCommands().size() << "개)";
    }
    EXPECT_TRUE(bBody);
    EXPECT_TRUE(bOverlay);
}

// 낙하 중 병사가 죽으면 낙하산 오버레이는 그 순간부터 더 이상 제출되지 않는다 (착지 변신 FX 는 별도)
TEST(EnemyWorld, ParachuteOverlayDisappearsWhenTheParachutistDies)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    const EnemyHandle h = rig.Spawn("flying_bzk", { 300.f, 200.f });
    ASSERT_FALSE(h.IsNull());
    rig.Step(3);
    ASSERT_TRUE(rig.E(h)->IsAlive());

    EXPECT_GT(rig.Strike(h.id, eHitKind::Bullet, 999), 0u);
    rig.Step(1);
    ASSERT_FALSE(rig.E(h)->IsAlive());

    SpriteBatch sprites;
    rig.World().Submit(sprites, anims, rig.Projectiles(), 0.f);
    for (const SpriteCommand& command: sprites.GetPendingCommands())
    {
        EXPECT_NE(command.depth, 14) << "죽은 병사의 낙하산 오버레이가 아직 그려진다";
    }
}

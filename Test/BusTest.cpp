// 버스 중간보스 테스트: 사전 배치 (슬라이드 인 없음), 카메라 뷰 오른쪽 끝 기준 개방 트리거, 개방 전 무적, 계단 병사 스폰/오르기
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "Base/Math.h"
#include "Render/SpriteBatch.h"
#include "VehicleTestUtil.h"

constexpr float    kGround       = 378.f;
constexpr float    kAnchorX      = 2985.f;
constexpr float    kStairAnchorX = 2400.f;   // 평지 길이 (3000) 안에서 계단 끝까지 올라가도록
constexpr uint32_t kStairTag     = 0x42555300u;
constexpr int      kStateParked  = 0;   // Bus.cpp 의 eBusState 순서

namespace
{
[[nodiscard]] SpawnParams BusParams_()
{
    SpawnParams params;
    params.facing = -1;
    return params;
}

[[nodiscard]] Vector<const Enemy*> StairSoldiers_(
    VehicleRig& _rig)
{
    Vector<const Enemy*> out;
    for (const Enemy& enemy: _rig.World().GetEnemies())
    {
        if (enemy.userTag == kStairTag && enemy.life != eEnemyLife::Gone)
        {
            out.push_back(&enemy);
        }
    }
    return out;
}
}   // namespace

TEST(Bus, StaysAtItsSpawnAnchorWithoutSlidingIn)
{
    VehicleRig rig({ 2700.f, kGround }, 0.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("bus", { 3236.f, kGround }, BusParams_());
    rig.Step(300);
    EXPECT_FLOAT_EQ(rig.E(h)->body.position.x, 3236.f);

    rig.SetView(2496.f);
    rig.Step(300);
    EXPECT_FLOAT_EQ(rig.E(h)->body.position.x, 3236.f) << "개방 뒤에도 슬라이드하지 않는다";
}

TEST(Bus, OpensOnlyWhenTheViewRightEdgeReachesTheBusLeftEdge)
{
    VehicleRig rig({ 2500.f, kGround }, 2200.f);   // 뷰 오른쪽 끝 2840 < 버스 왼쪽 끝 2885
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("bus", { kAnchorX, kGround }, BusParams_());
    rig.Step(60);
    EXPECT_EQ(rig.E(h)->state, kStateParked);

    rig.SetView(2250.f);   // 뷰 오른쪽 끝 2890 >= 2885
    rig.Step(2);
    EXPECT_NE(rig.E(h)->state, kStateParked);
}

TEST(Bus, IsInvulnerableAndSilentBeforeItOpens)
{
    VehicleRig rig({ 2700.f, kGround }, 0.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h  = rig.Spawn("bus", { kAnchorX, kGround }, BusParams_());
    const int         hp = rig.E(h)->health.hp;
    rig.Step(2);
    (void)rig.Strike(h.id, eHitKind::Bullet, 40);
    rig.Step(2);
    EXPECT_EQ(rig.E(h)->health.hp, hp);
    rig.Step(240);
    EXPECT_TRUE(StairSoldiers_(rig).empty());
    for (const Shot& shot: rig.Projectiles().GetShots())
    {
        EXPECT_NE(shot.params.owner, h.id) << "개방 전에는 사격하지 않는다";
    }
}

TEST(Bus, SpawnsStairSoldiersOffScreenOnTheRightAndCapsTheCount)
{
    VehicleRig rig({ 2700.f, kGround }, 2496.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h        = rig.Spawn("bus", { kAnchorX, kGround }, BusParams_());
    size_t            maxCount = 0;
    float             firstX   = -1.f;
    for (int i = 0; i < 900; ++i)
    {
        rig.Step(1);
        const Vector<const Enemy*> soldiers = StairSoldiers_(rig);
        maxCount                            = std::max(maxCount, soldiers.size());
        if (firstX < 0.f && !soldiers.empty())
        {
            firstX = soldiers.front()->body.position.x;
        }
    }
    ASSERT_GT(firstX, 0.f) << "계단 병사가 나와야 한다";
    EXPECT_GT(firstX, 2496.f + 640.f) << "화면 오른쪽 밖에서 나온다";
    EXPECT_NEAR(firstX, kAnchorX + 230.f, 3.f);
    const EnemyDef& busDef = rig.Lib().Get(rig.Lib().FindIndex("bus"));
    EXPECT_LE(maxCount, static_cast<size_t>(busDef.bus.soldierMax));
    EXPECT_NE(rig.E(h), nullptr);
}

TEST(Bus, StairSoldiersWalkUpTheStairsToTheLanding)
{
    VehicleRig rig({ 2700.f, kGround }, 2496.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    (void)rig.Spawn("bus", { kAnchorX, kGround }, BusParams_());
    bool  bReachedTop = false;
    float minY        = kGround;
    for (int i = 0; i < 900 && !bReachedTop; ++i)
    {
        rig.Step(1);
        for (const Enemy* pSoldier: StairSoldiers_(rig))
        {
            minY = std::min(minY, pSoldier->body.position.y);
            if (std::abs(pSoldier->body.position.x - (kAnchorX + 100.f)) < 3.f && pSoldier->body.position.y < kGround - 100.f)
            {
                bReachedTop = true;
            }
        }
    }
    EXPECT_TRUE(bReachedTop) << "승강대 (앵커 + 100, 높이 106) 까지 올라야 한다. minY=" << minY;
}

TEST(Bus, NeverFiresItsOwnProjectiles)
{
    VehicleRig rig({ 2700.f, kGround }, 2496.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("bus", { kAnchorX, kGround }, BusParams_());
    for (int i = 0; i < 900; ++i)
    {
        rig.Step(1);
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            EXPECT_NE(shot.params.owner, h.id) << "버스 본체는 직접 공격하지 않는다 (병사만 공격)";
        }
    }
    EXPECT_NE(rig.E(h), nullptr);
}

TEST(Bus, BazookaSoldiersClimbTheStairsToTheRoofAndFireRockets)
{
    // 바주카병도 계단을 올라 승강대 (버스 위) 에 서서 로켓을 쏜다. 평지 길이 (3000) 안에서 검증하도록 버스 앵커를 2400 으로 둔다.
    bool bSpawnedBzk  = false;
    bool bReachedRoof = false;
    bool bFiredRocket = false;
    for (uint64_t seed = 1; seed <= 12 && !bFiredRocket; ++seed)
    {
        VehicleRig rig({ 2100.f, kGround }, 2000.f);
        ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
        rig.World().ReseedRng(seed);
        const EnemyHandle h        = rig.Spawn("bus", { 2400.f, kGround }, BusParams_());
        const int         bzkIndex = rig.World().GetLibrary()->FindIndex("soldier_bzk");
        ASSERT_GE(bzkIndex, 0);
        for (int i = 0; i < 900 && !bFiredRocket; ++i)
        {
            rig.Step(1);
            for (const Enemy& soldier: rig.World().GetEnemies())
            {
                if (soldier.userTag != kStairTag || soldier.defIndex != bzkIndex || soldier.life == eEnemyLife::Gone)
                {
                    continue;
                }
                bSpawnedBzk = true;
                if (std::abs(soldier.body.position.x - (2400.f + 100.f)) < 3.f && soldier.body.position.y < kGround - 100.f)
                {
                    bReachedRoof = true;
                }
                for (const Shot& shot: rig.Projectiles().GetShots())
                {
                    if (shot.params.owner == soldier.id)
                    {
                        bFiredRocket = true;
                    }
                }
            }
        }
        EXPECT_NE(rig.E(h), nullptr);
    }
    EXPECT_TRUE(bSpawnedBzk) << "12 개 시드 안에 바주카병이 한 번은 나와야 한다";
    EXPECT_TRUE(bReachedRoof) << "바주카병은 계단을 올라 승강대 (앵커 + 100, 높이 106) 에 서야 한다";
    EXPECT_TRUE(bFiredRocket) << "바주카병이 버스 위에서 플레이어를 향해 로켓을 쏴야 한다";
}

namespace
{
[[nodiscard]] float RoofY_(
    VehicleRig& _rig)
{
    return kGround + _rig.Lib().Get(_rig.Lib().FindIndex("bus")).bus.roofTop.y;
}
}   // namespace

TEST(Bus, StairSoldiersClimbToTheRoofWithoutGettingStuck)
{
    VehicleRig rig({ 2100.f, kGround }, 2000.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    (void)rig.Spawn("bus", { kStairAnchorX, kGround }, BusParams_());
    const float roofY = RoofY_(rig);

    Vector<uint32_t> ids;
    Vector<int>      born;
    Vector<int>      lastMove;
    Vector<Vec2>     last;
    int              arrived = 0;
    for (int tick = 0; tick < 700; ++tick)
    {
        rig.Step(1);
        for (const Enemy* pSoldier: StairSoldiers_(rig))
        {
            size_t k = 0;
            while (k < ids.size() && ids[k] != pSoldier->id)
            {
                ++k;
            }
            if (k == ids.size())
            {
                ids.push_back(pSoldier->id);
                born.push_back(tick);
                lastMove.push_back(tick);
                last.push_back(pSoldier->body.position);
                continue;
            }
            const Vec2 pos = pSoldier->body.position;
            if (std::abs(pos.x - last[k].x) + std::abs(pos.y - last[k].y) > 0.1f)
            {
                lastMove[k] = tick;
                last[k]     = pos;
            }
            if (born[k] < 0)
            {
                continue;
            }
            if (pos.y <= roofY + 3.f)
            {
                EXPECT_LT(tick - born[k], 400) << "지붕까지 6.6 초를 넘겼다 (id " << pSoldier->id << ")";
                ++arrived;
                born[k] = -1;
            }
            else
            {
                EXPECT_LT(tick - lastMove[k], 30) << "오르는 중 멈췄다: id " << pSoldier->id << " tick " << tick << " pos " << pos.x << "," << pos.y;
            }
        }
    }
    EXPECT_GE(arrived, 3) << "병사 3 명 이상이 지붕 높이에 도달해야 한다";
}

TEST(Bus, StairSoldiersDoNotAttackBeforeReachingTheRoof)
{
    for (uint64_t seed = 1; seed <= 6; ++seed)
    {
        VehicleRig rig({ 2100.f, kGround }, 2000.f);
        ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
        rig.World().ReseedRng(seed);
        (void)rig.Spawn("bus", { kStairAnchorX, kGround }, BusParams_());
        const float      roofY = RoofY_(rig);
        Vector<uint32_t> arrivedIds;
        for (int tick = 0; tick < 700; ++tick)
        {
            rig.Step(1);
            for (const Enemy* pSoldier: StairSoldiers_(rig))
            {
                if (pSoldier->body.position.y <= roofY + 3.f)
                {
                    arrivedIds.push_back(pSoldier->id);
                }
                if (std::find(arrivedIds.begin(), arrivedIds.end(), pSoldier->id) != arrivedIds.end())
                {
                    continue;
                }
                EXPECT_EQ(rig.CountShots({ pSoldier->id }), 0) << "지붕에 닿기 전 사격 id " << pSoldier->id << " tick " << tick;
                EXPECT_EQ(pSoldier->meleeTicksLeft, 0) << "지붕에 닿기 전 베기 id " << pSoldier->id;
            }
        }
    }
}

TEST(Bus, StairSoldiersOnTheLandingAreDrawnInFrontOfTheBodyAndBehindTheCoverInEveryStage)
{
    SLUG_REQUIRE_BAKED(anims);
    VehicleRig rig({ 2100.f, kGround }, 2000.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h     = rig.Spawn("bus", { kStairAnchorX, kGround }, BusParams_());
    const BusCfg&     c     = rig.Lib().Get(rig.Lib().FindIndex("bus")).bus;
    const float       roofY = kGround + c.stairsTop.y;   // 계단 끝 승강대 (지붕 없음)

    const ARRAY<int, 3> wantStages = { 0, 1, 4 };
    for (const int wantStage: wantStages)
    {
        for (int i = 0; i < 1500 && rig.E(h)->aux2 < wantStage; ++i)
        {
            rig.Step(1);
            if (rig.E(h)->aux > 400)
            {
                (void)rig.Strike(h.id, eHitKind::Bullet, 1);
            }
        }
        ASSERT_EQ(rig.E(h)->aux2, wantStage);
        int   onRoof   = 0;
        float soldierY = roofY;
        for (int i = 0; i < 900 && onRoof == 0; ++i)
        {
            rig.Step(1);
            for (const Enemy* pSoldier: StairSoldiers_(rig))
            {
                if (pSoldier->body.position.y < kGround - 90.f)   // 승강대 근처 (계단 윗부분 / 도착 직후 전방 점프)
                {
                    ++onRoof;
                    soldierY = pSoldier->body.position.y;
                }
            }
        }
        ASSERT_EQ(rig.E(h)->aux2, wantStage) << "지붕 병사를 기다리는 동안 단계가 바뀌었다";
        ASSERT_GT(onRoof, 0) << "단계 " << wantStage << " 에서 지붕 위 병사가 없다";

        SpriteBatch sprites;
        rig.World().Submit(sprites, anims, rig.Projectiles(), 0.f);
        Vector<SpriteCommand> commands = sprites.GetPendingCommands();
        SortSpriteCommands(commands);
        int body    = -1;
        int soldier = -1;
        int cover   = -1;
        for (size_t i = 0; i < commands.size(); ++i)
        {
            const SpriteCommand& cmd = commands[i];
            if (cmd.depth == c.partCover.depth && cmd.sortKey == c.partCover.sort)
            {
                cover = static_cast<int>(i);
                if (wantStage >= 1)
                {
                    EXPECT_LE(cmd.frame.w, cmd.frame.pivotX + static_cast<int>(c.coverMaxX)) << "파손 단계 앞면 덮개는 BackD 의 앞면 부분 (계단 제외) 만 덮어야 한다";
                }
            }
            else if (cmd.depth == c.partBody.depth && cmd.sortKey == c.partBody.sort)
            {
                body = static_cast<int>(i);
            }
            else if (cmd.depth == c.partBody.depth && cmd.sortKey >= static_cast<int>(soldierY) - 2 && cmd.sortKey <= static_cast<int>(soldierY) + 10)
            {
                soldier = static_cast<int>(i);
            }
        }
        EXPECT_GE(body, 0) << "단계 " << wantStage;
        EXPECT_GE(soldier, 0) << "단계 " << wantStage;
        EXPECT_GE(cover, 0) << "단계 " << wantStage << ": 병사 앞 덮개 제출 없음";
        EXPECT_LT(body, soldier) << "단계 " << wantStage;
        EXPECT_LT(soldier, cover) << "단계 " << wantStage;
    }
}

TEST(Bus, StairSoldiersNeverSinkIntoTheBusBodyAndStandOnItsTop)
{
    for (uint64_t seed = 1; seed <= 6; ++seed)
    {
        VehicleRig rig({ 2100.f, kGround }, 2000.f);
        ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
        rig.World().ReseedRng(seed);
        (void)rig.Spawn("bus", { kStairAnchorX, kGround }, BusParams_());
        const Aabb  body     = rig.Lib().Get(rig.Lib().FindIndex("bus")).solid.At({ kStairAnchorX, kGround }, -1);
        const float topY     = body.min.y;   // 버스 몸체 윗면: 전방 점프한 병사가 서는 곳
        int         topTicks = 0;
        for (int tick = 0; tick < 1500; ++tick)
        {
            rig.Step(1);
            for (const Enemy* pSoldier: StairSoldiers_(rig))
            {
                const float x = pSoldier->body.position.x;
                if (pSoldier->life != eEnemyLife::Alive || x <= body.min.x + 4.f || x >= body.max.x - 4.f)
                {
                    continue;
                }
                topTicks += (std::abs(pSoldier->body.position.y - topY) < 2.f && pSoldier->bGrounded) ? 1 : 0;
                EXPECT_LE(pSoldier->body.position.y, topY + 2.f) << "시드 " << seed << " tick " << tick << ": 병사가 버스 몸체 안으로 들어갔다 (" << x << "," << pSoldier->body.position.y - kGround << ")";
            }
        }
        EXPECT_GT(topTicks, 60) << "시드 " << seed << ": 전방 점프한 병사가 버스 위에 서 있어야 한다";
    }
}

TEST(Bus, StairSoldierTraceWritesCsvWhenRequested)
{
    // BUS_TRACE=<path> 이면 계단 병사의 틱별 위치/상태를 CSV 로 남긴다 (진단용)
    char*  pPath  = nullptr;
    size_t length = 0;
    if (_dupenv_s(&pPath, &length, "BUS_TRACE") != 0 || pPath == nullptr)
    {
        GTEST_SKIP() << "BUS_TRACE not set";
    }
    const String path = pPath;
    std::free(pPath);
    std::ofstream out(path);
    out << "tick,id,def,x,y,state,grounded,shots\n";
    VehicleRig rig({ 2100.f, kGround }, 2000.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    (void)rig.Spawn("bus", { kStairAnchorX, kGround }, BusParams_());
    for (int i = 0; i < 900; ++i)
    {
        rig.Step(1);
        for (const Enemy* pSoldier: StairSoldiers_(rig))
        {
            int shots = 0;
            for (const Shot& shot: rig.Projectiles().GetShots())
            {
                shots += shot.params.owner == pSoldier->id ? 1 : 0;
            }
            out << i << ',' << pSoldier->id << ',' << pSoldier->defIndex << ',' << pSoldier->body.position.x - kStairAnchorX << ',' << pSoldier->body.position.y - kGround << ',' << pSoldier->state
                << ',' << pSoldier->bGrounded << ',' << shots << '\n';
        }
    }
}

TEST(Bus, StairSoldiersLeapForwardOverTheLandingWallOntoTheBus)
{
    VehicleRig rig({ 2700.f, kGround }, 2496.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    (void)rig.Spawn("bus", { kAnchorX, kGround }, BusParams_());
    const BusCfg& c = rig.Lib().Get(rig.Lib().FindIndex("bus")).bus;

    bool bPastLanding = false;
    for (int i = 0; i < 1500 && !bPastLanding; ++i)
    {
        rig.Step(1);
        for (const Enemy* pSoldier: StairSoldiers_(rig))
        {
            // 승강대 벽 (landing x ± landingW/2) 안쪽 (버스 쪽) 으로 넘어와 땅(윗면) 에 서 있다
            if (pSoldier->body.position.x < kAnchorX + c.stairsTop.x - c.landingW * 0.5f - 6.f && pSoldier->body.position.y < kGround - 100.f && pSoldier->bGrounded)
            {
                bPastLanding = true;
            }
        }
    }
    EXPECT_TRUE(bPastLanding) << "계단 끝에서 전방 점프해 버스 위에 올라서야 한다 (승강대와 버스 사이에 끼면 안 된다)";
}

TEST(Bus, BlastKilledSoldierIsRemovedOnlyAfterFallingOutOfTheCamera)
{
    VehicleRig rig({ 2100.f, kGround }, 2000.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const EnemyHandle h = rig.Spawn("soldier_bomb", { 2300.f, kGround }, SpawnParams {});
    rig.Step(30);
    ASSERT_NE(rig.E(h), nullptr);
    (void)rig.Strike(h.id, eHitKind::Blast, 999);

    float lastY      = kGround;
    bool  bGone      = false;
    float viewBottom = 0.f;
    for (int tick = 0; tick < 900 && !bGone; ++tick)
    {
        rig.Step(1);
        const Enemy* pEnemy = rig.E(h);
        if (pEnemy == nullptr || pEnemy->life == eEnemyLife::Gone)
        {
            bGone = true;
            break;
        }
        lastY = pEnemy->body.position.y;
    }
    viewBottom = 448.f;   // VehicleRig 뷰: y 0..448
    ASSERT_TRUE(bGone) << "시체는 결국 지워져야 한다";
    EXPECT_GT(lastY - 90.f, viewBottom - 20.f) << "화면 아래로 거의 완전히 나간 뒤에 지워져야 한다 (lastY=" << lastY << ", viewBottom=" << viewBottom << ")";
}

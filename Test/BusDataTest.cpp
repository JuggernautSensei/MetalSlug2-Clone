#include <algorithm>
#include <cmath>

#include "Base/Math.h"
#include "Render/SpriteBatch.h"
#include "VehicleTestUtil.h"

struct OldBurst
{
    int         stage;
    const char* clip;
    float       u;
    float       v;
    int         delayTicks;
};

struct OldDebris
{
    const char* clip;
    float       u;
    float       v;
    float       vx;
    float       vy;
    float       fps;
};

struct OldSpot
{
    int   stage;
    float u;
    float v;
    int   phase;
};

constexpr float kTol = 1e-5f;

namespace
{
void ExpectBursts_(
    const Vector<BusBurstDef>& _got,
    const OldBurst*            _want,
    const size_t               _count)
{
    ASSERT_EQ(_got.size(), _count);
    for (size_t i = 0; i < _count; ++i)
    {
        EXPECT_EQ(_got[i].stage, _want[i].stage) << i;
        EXPECT_EQ(_got[i].clip, _want[i].clip) << i;
        EXPECT_NEAR(_got[i].at.x, _want[i].u, kTol) << i;
        EXPECT_NEAR(_got[i].at.y, _want[i].v, kTol) << i;
        EXPECT_NEAR(_got[i].delaySec, static_cast<float>(_want[i].delayTicks) / 60.f, kTol) << i;
    }
}
}   // namespace

TEST(BusData, LoadedTablesEqualTheOldHardCodedValues)
{
    VehicleRig rig;
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const int index = rig.Lib().FindIndex("bus");
    ASSERT_GE(index, 0);
    const BusCfg& c = rig.Lib().Get(index).bus;

    EXPECT_FLOAT_EQ(c.pxPerSheetPx, 2.f);
    EXPECT_FLOAT_EQ(c.anchor.x, 52.f);
    EXPECT_FLOAT_EQ(c.anchor.y, 130.f);
    EXPECT_FLOAT_EQ(c.debrisGroundDy, -8.f);
    EXPECT_EQ(c.fxDepth, 60);

    const ARRAY<OldBurst, 12> bursts = {
        OldBurst { 2, "Explosion_Large", 51.f,  -5.f, 0 },
        OldBurst { 2, "Explosion_Large", 42.f,  86.f, 0 },
        OldBurst { 2, "Explosion_Large", 87.f,  70.f, 0 },
        OldBurst { 3, "Explosion_Large", 21.f,  72.f, 0 },
        OldBurst { 3,   "Explosion_Mid", 91.f, 113.f, 0 },
        OldBurst { 4, "Explosion_Large", 50.f,  95.f, 0 },
        OldBurst { 4,   "Explosion_Mid", 80.f, 100.f, 6 },
        OldBurst { 5, "Explosion_Large", 42.f, 101.f, 0 },
        OldBurst { 6,   "Explosion_Mid", 45.f, 100.f, 0 },
        OldBurst { 7,   "Explosion_Mid",  9.f, 116.f, 0 },
        OldBurst { 7,   "Explosion_Mid", 78.f,  95.f, 3 },
        OldBurst { 6, "Explosion_Small", 80.f,  40.f, 4 }
    };
    ExpectBursts_(c.bursts, bursts.data(), bursts.size());

    const ARRAY<OldBurst, 10> death = {
        OldBurst { 0, "Explosion_Large", 20.f, -10.f,  0 },
        OldBurst { 0, "Explosion_Large", 75.f,   0.f,  0 },
        OldBurst { 0, "Explosion_Large", 48.f,  35.f,  2 },
        OldBurst { 0, "Explosion_Large", 10.f,  60.f,  3 },
        OldBurst { 0, "Explosion_Large", 90.f,  55.f,  3 },
        OldBurst { 0, "Explosion_Large", 30.f,  95.f,  5 },
        OldBurst { 0, "Explosion_Large", 70.f, 100.f,  6 },
        OldBurst { 0, "Explosion_Large", 48.f, 125.f,  8 },
        OldBurst { 0,   "Explosion_Mid", 40.f,  20.f, 14 },
        OldBurst { 0,   "Explosion_Mid", 60.f,  80.f, 18 }
    };
    ExpectBursts_(c.deathBursts, death.data(), death.size());

    const ARRAY<OldDebris, 9> debris = {
        OldDebris { "Bus_DebGrille", 52.f, 112.f,  -60.f, -520.f, 14.f },
        OldDebris { "Bus_DebBumper", 52.f, 124.f,   90.f, -430.f, 10.f },
        OldDebris { "Bus_DebFrameA", 30.f,  30.f, -170.f, -640.f, 12.f },
        OldDebris { "Bus_DebFrameB", 75.f,  28.f,  160.f, -600.f, 12.f },
        OldDebris {  "Bus_DebChunk", 20.f,  70.f, -220.f, -480.f, 16.f },
        OldDebris {  "Bus_DebChunk", 80.f,  60.f,  240.f, -520.f, 16.f },
        OldDebris {  "Bus_DebChunk", 50.f,  20.f,   30.f, -700.f, 16.f },
        OldDebris {    "Bus_DebBar", 15.f, 105.f, -140.f, -380.f, 10.f },
        OldDebris {    "Bus_DebBar", 90.f, 100.f,  130.f, -420.f, 10.f }
    };
    ASSERT_EQ(c.debris.size(), debris.size());
    for (size_t i = 0; i < debris.size(); ++i)
    {
        EXPECT_EQ(c.debris[i].clip, debris[i].clip) << i;
        EXPECT_FLOAT_EQ(c.debris[i].at.x, debris[i].u) << i;
        EXPECT_FLOAT_EQ(c.debris[i].at.y, debris[i].v) << i;
        EXPECT_FLOAT_EQ(c.debris[i].velocity.x, debris[i].vx) << i;
        EXPECT_FLOAT_EQ(c.debris[i].velocity.y, debris[i].vy) << i;
        EXPECT_FLOAT_EQ(c.debris[i].fps, debris[i].fps) << i;
    }

    EXPECT_TRUE(c.innerSoldiers.empty()) << "창 안 장식 병사는 쓰지 않는다 (반파 시 병사 수가 늘어난 것처럼 보였다)";

    ASSERT_EQ(c.windows.size(), 4u);
    EXPECT_EQ(c.windows[0].clip, "Bus_WinUL");
    EXPECT_EQ(c.windows[1].clip, "Bus_WinUR");
    EXPECT_EQ(c.windows[2].clip, "Bus_WinLL");
    EXPECT_EQ(c.windows[2].stage, 4);
    EXPECT_EQ(c.windows[3].clip, "Bus_WinLR");
    EXPECT_EQ(c.windows[3].stage, 6);

    EXPECT_EQ(c.partInnerSoldier.depth, 15);
    EXPECT_EQ(c.partInnerSoldier.sort, 2);
    EXPECT_EQ(c.partBack.sort, 1);
    EXPECT_EQ(c.partBody.sort, 3);
    EXPECT_EQ(c.partStairs.sort, 1);
    EXPECT_EQ(c.partWindow.sort, 5);
    EXPECT_EQ(c.partDebris.sort, 20);
    EXPECT_EQ(c.partBack.depth, 15);
    EXPECT_EQ(c.partBody.depth, 15);
    EXPECT_GT(c.partCover.depth, 15) << "덮개는 적 레이어 (병사) 보다 위";
}

TEST(BusData, SoldierIsDrawnBetweenTheBusBodyAndItsCover)
{
    SLUG_REQUIRE_BAKED(anims);
    VehicleRig rig({ 2700.f, 378.f }, 2496.f);
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    const int index = rig.Lib().FindIndex("bus");
    ASSERT_GE(index, 0);
    const BusCfg& c = rig.Lib().Get(index).bus;

    SpawnParams busParams;
    busParams.facing = -1;
    (void)rig.Spawn("bus", { 2985.f, 378.f }, busParams);
    (void)rig.Spawn("soldier_sword", { 2700.f, 378.f });
    rig.Step(90);

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
        }
        else if (cmd.depth == c.partBody.depth && cmd.sortKey == c.partBody.sort)
        {
            body = static_cast<int>(i);
        }
        else if (cmd.depth == c.partBody.depth && cmd.sortKey == 378)
        {
            soldier = static_cast<int>(i);
        }
    }
    ASSERT_GE(body, 0) << "버스 본체 제출 없음 (" << commands.size() << "개)";
    ASSERT_GE(soldier, 0) << "병사 제출 없음";
    ASSERT_GE(cover, 0) << "버스 덮개 제출 없음";
    EXPECT_LT(body, soldier);
    EXPECT_LT(soldier, cover);
}

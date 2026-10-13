
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/Enemies/EnemyDef.h"

struct MigrationRow
{
    String name   = {};
    float  actual = 0.f;
    float  old    = 0.f;
};

TEST(DataMigrationA, LoadedDefsKeepTheOldHardCodedValues)
{
    String                  message;
    Result<EnemyDefLibrary> lib = EnemyDefLibrary::LoadDirectory(FindAssetRoot() / "Defs", &message);
    ASSERT_TRUE(lib) << message.c_str();

    const EnemyDef* pSoldier  = lib->Find("soldier_bomb");
    const EnemyDef* pJeep     = lib->Find("jeep");
    const EnemyDef* pHeliBomb = lib->Find("heli_bomb");
    const EnemyDef* pBoss     = lib->Find("bigshiee");
    ASSERT_NE(pSoldier, nullptr);
    ASSERT_NE(pJeep, nullptr);
    ASSERT_NE(pHeliBomb, nullptr);
    ASSERT_NE(pBoss, nullptr);

    const BigShieeCfg& boss = pBoss->bigShiee;

    const Vector<MigrationRow> rows = {
        {           "death.bloodOffsetY",   pSoldier->death.bloodOffsetY, -27.5f },
        {         "death.bombVelocity.x", pSoldier->death.bombVelocity.x,  110.f },
        {         "death.bombVelocity.y", pSoldier->death.bombVelocity.y, -430.f },
        {            "jeep.rocketShiftX",       pJeep->jeep.rocketShiftX,   20.f },
        {          "heliBomb.groundFxDy", pHeliBomb->heliBomb.groundFxDy,   -6.f },
        {          "bigShiee.chuteEdgeX",                boss.chuteEdgeX,   30.f },
        { "bigShiee.turretBlastOffset.x",       boss.turretBlastOffset.x,    0.f },
        { "bigShiee.turretBlastOffset.y",       boss.turretBlastOffset.y,  -24.f },
        { "bigShiee.turretSmokeOffset.x",       boss.turretSmokeOffset.x,    0.f },
        { "bigShiee.turretSmokeOffset.y",       boss.turretSmokeOffset.y,  -10.f },
        {  "bigShiee.deathFlameOffset.x",        boss.deathFlameOffset.x,    0.f },
        {  "bigShiee.deathFlameOffset.y",        boss.deathFlameOffset.y,   40.f },
        {        "bigShiee.deathBlastDy",              boss.deathBlastDy,  120.f },
        {   "bigShiee.deathHugeOffset.x",         boss.deathHugeOffset.x,   20.f },
        {   "bigShiee.deathHugeOffset.y",         boss.deathHugeOffset.y, -150.f },
        {   "bigShiee.deathDiscOffset.x",         boss.deathDiscOffset.x,  -40.f },
        {   "bigShiee.deathDiscOffset.y",         boss.deathDiscOffset.y, -170.f },
    };
    for (const MigrationRow& row: rows)
    {
        EXPECT_FLOAT_EQ(row.actual, row.old) << row.name.c_str();
    }
}

// 최종보스 전함 (Docs/Spec_BigShiee.md): 입장, 포탑/주포 체력과 파괴, 주포 전개, 사망 연출, 자동 스크롤, 결정성, 낙하산 인질 보급을 검증한다.

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "EnemyTestUtil.h"
#include "Game/Enemies/BigShiee.h"
#include "Game/Player/SlugRender.h"
#include "Game/Props/DecorWorld.h"
#include "Game/Props/PropWorld.h"
#include "Render/Animation.h"

constexpr float kCamLeft = 1000.f;

namespace
{
void LoadBossLib_(
    EnemyRig& _rig)
{
    String                  message;
    Result<EnemyDefLibrary> lib = EnemyDefLibrary::LoadDirectory(FindAssetRoot() / "Defs", &message);
    ASSERT_TRUE(lib) << message.c_str();
    _rig.Lib() = lib.Take();
}

[[nodiscard]] EnemyHandle SpawnBoss_(
    EnemyRig& _rig)
{
    SpawnParams params;
    params.facing     = 1;
    params.bActivated = true;
    return _rig.Spawn("bigshiee", { kCamLeft + 300.f, 378.f }, params);
}

[[nodiscard]] uint32_t PartOwner_(
    const EnemyHandle _handle,
    const int         _part)
{
    return MakePartOwner(_handle.id, static_cast<uint32_t>(_part));
}

void KillAllTurrets_(
    EnemyRig&         _rig,
    const EnemyHandle _boss)
{
    for (int part = 0; part < kBigShieeTurretCount; ++part)
    {
        for (int hit = 0; hit < 40 && _rig.Strike(PartOwner_(_boss, part), eHitKind::Bullet, 10) > 0; ++hit)
        {
        }
    }
}
}   // namespace

TEST(BigShiee, LoadsDefinitionWithSevenTurretsFacingRight)
{
    EnemyRig rig;
    LoadBossLib_(rig);
    const int index = rig.Lib().FindIndex("bigshiee");
    ASSERT_GE(index, 0);
    const EnemyDef& def = rig.Lib().Get(index);
    EXPECT_EQ(def.behavior, eEnemyBehavior::BigShiee);
    EXPECT_GT(def.hp, 0);
    int twins = 0;
    for (const BigShieeTurretCfg& turret: def.bigShiee.turrets)
    {
        EXPECT_GT(turret.hp, 0);
        twins += turret.bTwin ? 1 : 0;
    }
    EXPECT_EQ(twins, 4) << "쌍발 4 (A-D) + 단발 3 (E-G)";
    EXPECT_GE(def.bigShiee.numEntranceKeys, 4);
    EXPECT_GE(def.FindWeapon("shell"), 0);
    EXPECT_GE(def.FindWeapon("missile"), 0);
}

TEST(BigShiee, EntersFromOffscreenLeftSlowlyWithoutPopping)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle boss = SpawnBoss_(rig);
    rig.Step(1);
    const Enemy* pE = rig.E(boss);
    ASSERT_NE(pE, nullptr);
    EXPECT_LT(pE->body.position.x + 271.f, kCamLeft) << "첫 틱에는 선체 전체가 화면 왼쪽 밖이다";

    float prev = pE->body.position.x;
    for (int t = 0; t < 600; ++t)
    {
        rig.Step(1);
        const float x = rig.E(boss)->body.position.x;
        EXPECT_LT(std::abs(x - prev), 4.f) << "한 틱에 4px 넘게 움직이면 뿅 나타나는 것이다 (tick " << t << ")";
        EXPECT_GE(x, prev - 0.001f);
        prev = x;
    }
    EXPECT_GT(prev, kCamLeft - 100.f) << "10 초 뒤에는 대부분 들어와 있다";
    EXPECT_EQ(static_cast<eBigShieePhase>(rig.E(boss)->state), eBigShieePhase::Entrance);
    rig.Step(600);
    EXPECT_EQ(static_cast<eBigShieePhase>(rig.E(boss)->state), eBigShieePhase::Battery);
}

TEST(BigShiee, RequestsAutomaticCameraScrollAfterTheLockGlide)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    (void)SpawnBoss_(rig);
    float total = 0.f;
    for (int t = 0; t < 300; ++t)
    {
        rig.Step(1);
        total += rig.World().TakeCameraScroll();
    }
    EXPECT_GT(total, 50.f) << "5 초 동안 수십 px 스크롤";
    EXPECT_LT(total, 120.f);
}

TEST(BigShiee, TurretsHaveOwnHealthAndBreakOneByOne)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle boss = SpawnBoss_(rig);
    rig.Step(30);

    EXPECT_GT(rig.Strike(PartOwner_(boss, 0), eHitKind::Bullet, 3), 0u);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::PartDestroyed), 0);
    for (int hit = 0; hit < 40 && rig.Strike(PartOwner_(boss, 0), eHitKind::Bullet, 10) > 0; ++hit)
    {
    }
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::PartDestroyed), 1);
    EXPECT_EQ(rig.Strike(PartOwner_(boss, 0), eHitKind::Bullet, 10), 0u) << "부서진 포탑은 판정이 없다";
    EXPECT_GT(rig.Strike(PartOwner_(boss, 1), eHitKind::Bullet, 1), 0u) << "다른 포탑은 멀쩡하다";
    EXPECT_EQ(rig.Strike(PartOwner_(boss, kBigShieeCannonPart), eHitKind::Bullet, 10), 0u) << "주포는 아직 없다";
    EXPECT_EQ(rig.E(boss)->life, eEnemyLife::Alive);
}

TEST(BigShiee, FiresLobbedShellsOneVolleyAtATime)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle boss     = SpawnBoss_(rig);
    int               maxShots = 0;
    for (int t = 0; t < 1200; ++t)
    {
        rig.Step(1);
        maxShots = std::max(maxShots, rig.Projectiles().CountShots(boss.id));
    }
    EXPECT_GT(rig.E(boss)->shotsFired, 4);
    EXPECT_LE(maxShots, 8);
}

TEST(BigShiee, DeploysCannonAfterAllTurretsThenDiesAndFinishesTheDestroyFlow)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle boss = SpawnBoss_(rig);
    rig.Step(30);
    KillAllTurrets_(rig, boss);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::PartDestroyed), kBigShieeTurretCount);

    rig.Step(150);
    EXPECT_EQ(static_cast<eBigShieePhase>(rig.E(boss)->state) == eBigShieePhase::Deploy || static_cast<eBigShieePhase>(rig.E(boss)->state) == eBigShieePhase::Cannon, true);
    rig.Step(120);
    EXPECT_EQ(static_cast<eBigShieePhase>(rig.E(boss)->state), eBigShieePhase::Cannon);
    EXPECT_GT(rig.Strike(PartOwner_(boss, kBigShieeCannonPart), eHitKind::Bullet, 5), 0u) << "주포가 나온 뒤에는 맞는다";
    EXPECT_FALSE(rig.World().IsDefeated(boss));

    for (int hit = 0; hit < 2000 && rig.E(boss) && rig.E(boss)->life == eEnemyLife::Alive; ++hit)
    {
        (void)rig.Strike(PartOwner_(boss, kBigShieeCannonPart), eHitKind::Bullet, 50);
        rig.Step(1);
    }
    ASSERT_NE(rig.E(boss), nullptr);
    EXPECT_EQ(rig.E(boss)->life, eEnemyLife::Dying);
    EXPECT_GE(rig.CountEvents(eEnemyEventKind::Died), 1);
    EXPECT_FALSE(rig.World().IsDestroyFinished(boss));
    rig.Step(300);
    EXPECT_FALSE(rig.World().IsDestroyFinished(boss)) << "파괴 연출 중";
    rig.Step(300);
    EXPECT_TRUE(rig.World().IsDestroyFinished(boss));
}

TEST(BigShiee, IsDeterministic)
{
    int a = 0;
    int b = 0;
    for (int run = 0; run < 2; ++run)
    {
        EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
        LoadBossLib_(rig);
        const EnemyHandle boss = SpawnBoss_(rig);
        rig.Step(900);
        const Enemy* pE = rig.E(boss);
        ASSERT_NE(pE, nullptr);
        (run == 0 ? a : b) = static_cast<int>(pE->body.position.x * 1000.f) + pE->shotsFired * 7 + pE->ext[kBigShieeExtNextVolley];
    }
    EXPECT_EQ(a, b);
}

TEST(BigShiee, MeleeDoesNotHitTurretsOrCannonButBulletsAndBlastsDo)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle boss = SpawnBoss_(rig);
    rig.Step(30);

    EXPECT_EQ(rig.Strike(PartOwner_(boss, 0), eHitKind::Melee, 10), 0u) << "칼질은 포탑 파츠에 먹히지 않는다";
    EXPECT_EQ(rig.Strike(PartOwner_(boss, 3), eHitKind::Melee, 10), 0u);
    EXPECT_GT(rig.Strike(PartOwner_(boss, 0), eHitKind::Bullet, 1), 0u);
    EXPECT_GT(rig.Strike(PartOwner_(boss, 1), eHitKind::Blast, 1), 0u) << "폭발은 맞는다";

    KillAllTurrets_(rig, boss);
    rig.Step(150);
    rig.Step(120);
    ASSERT_EQ(static_cast<eBigShieePhase>(rig.E(boss)->state), eBigShieePhase::Cannon);
    EXPECT_EQ(rig.Strike(PartOwner_(boss, kBigShieeCannonPart), eHitKind::Melee, 50), 0u) << "주포도 칼질이 안 먹는다";
    EXPECT_GT(rig.Strike(PartOwner_(boss, kBigShieeCannonPart), eHitKind::Bullet, 5), 0u);
}

TEST(BigShiee, NoPartHurtboxesBeforeTheFirstTick)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle boss = SpawnBoss_(rig);
    EXPECT_EQ(rig.Strike(PartOwner_(boss, 0), eHitKind::Bullet, 1), 0u) << "스폰 직후 첫 틱 전에는 판정이 없다 (스폰 x 에 머무는 글리치 방지)";
    rig.Step(1);
    EXPECT_GT(rig.Strike(PartOwner_(boss, 0), eHitKind::Bullet, 1), 0u) << "첫 틱 뒤에는 판정이 있다";
    const Enemy* pE = rig.E(boss);
    ASSERT_NE(pE, nullptr);
    EXPECT_LT(pE->body.position.x + 271.f, kCamLeft) << "첫 틱 위치는 화면 왼쪽 밖";
}

TEST(BigShiee, RendersBehindEveryStageSprite)
{
    constexpr int kEnemyBodyDepth = 15;
    const int     front           = std::min({ kDecorFarDepth, kDecorBackDepth, kPropDepthBack, kPropDepthFront, kEnemyBodyDepth, kSlugBodyDepth });
    EXPECT_LT(kBigShieeDepthHull, front);
    EXPECT_LT(kBigShieeDepthAntenna, front);
    EXPECT_LT(kBigShieeDepthCannon, front);
    EXPECT_LT(kBigShieeDepthTurret, front);
    EXPECT_LT(kBigShieeDepthDisc, front);
    EXPECT_LT(kBigShieeDepthHull, kBigShieeDepthAntenna);
    EXPECT_LE(kBigShieeDepthAntenna, kBigShieeDepthCannon);
    EXPECT_LE(kBigShieeDepthCannon, kBigShieeDepthTurret);
    EXPECT_LE(kBigShieeDepthDisc, 0) << "음수 레이어 (하늘/배경 텍스처 바로 앞)";
}

TEST(BigShiee, BlastHitboxesLastNoLongerThanTheExplosionAnim)
{
    EnemyRig rig;
    LoadBossLib_(rig);
    const EnemyDef& def = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    for (const char* weapon: { "shell", "missile" })
    {
        const int index = def.FindWeapon(weapon);
        ASSERT_GE(index, 0) << weapon;
        const int projIndex = def.weapons[static_cast<size_t>(index)].projectile;
        ASSERT_GE(projIndex, 0) << weapon;
        const ProjectileDef& proj  = rig.Lib().GetProjectile(projIndex);
        const int            ticks = SecToTicks(proj.blastSec);
        EXPECT_GT(proj.blastSize.x, 0.f) << weapon;
        EXPECT_GE(ticks, 6) << weapon;
        EXPECT_LE(ticks, 10) << weapon << ": 폭발 판정은 폭발 애니 시간 (약 6~10 틱) 이내";
    }
}

TEST(BigShiee, DropsParachuteHostagesAboveTheViewNearTheRightEdge)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    (void)SpawnBoss_(rig);
    const int chuteDef = rig.Lib().FindIndex("pow_chute");
    ASSERT_GE(chuteDef, 0);
    for (int t = 0; t < 60 * 10; ++t)
    {
        rig.Step(1);
        bool bSpawned = false;
        for (const Enemy& e: rig.World().GetEnemies())
        {
            bSpawned = bSpawned || e.defIndex == chuteDef;
        }
        if (bSpawned)
        {
            break;
        }
    }

    int found = 0;
    for (const Enemy& e: rig.World().GetEnemies())
    {
        if (e.defIndex != chuteDef)
        {
            continue;
        }
        ++found;
        EXPECT_LT(e.body.position.y, 0.f) << "카메라 위쪽 밖에서 떨어진다";
        EXPECT_GE(e.body.position.x, kCamLeft + 640.f * 0.6f) << "화면 오른쪽 쪽";
        EXPECT_FALSE(e.drop.empty()) << "구출 시 줄 무기";
    }
    EXPECT_EQ(found, 1) << "첫 낙하산 인질은 6 초 뒤 한 번";
}

TEST(BigShiee, ChuteHostageShowsTheTiedStuckPoseWithoutParachuteWhileDescending)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const int chuteDef = rig.Lib().FindIndex("pow_chute");
    const int powDef   = rig.Lib().FindIndex("pow");
    ASSERT_GE(chuteDef, 0);
    ASSERT_GE(powDef, 0);
    const EnemyDef& def   = rig.Lib().Get(chuteDef);
    const EnemyDef& pow   = rig.Lib().Get(powDef);
    const int       stuck = pow.FindClip("stuck");
    ASSERT_GE(stuck, 0);
    EXPECT_EQ(def.overlayClip, -1) << "낙하산 오버레이 없음";
    const ARRAY<const char*, 10> kLogical = { "ready0", "ready1", "ready2", "ready3", "ready4", "shot0", "shot1", "shot2", "shot3", "shot4" };
    for (const char* logical: kLogical)
    {
        const int index = def.FindClip(logical);
        ASSERT_GE(index, 0) << logical;
        EXPECT_EQ(def.GetClip(index).name, pow.GetClip(stuck).name) << logical << ": 낙하 중에는 묶인 자세 (POW stuck 클립)";
        EXPECT_EQ(def.GetClip(index).frames, pow.GetClip(stuck).frames) << logical;
    }
}

TEST(BigShiee, ChuteHostageFallsFreelyAcceleratingThenLandsAsPowWithItsDrop)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const int chuteDef = rig.Lib().FindIndex("pow_chute");
    const int powDef   = rig.Lib().FindIndex("pow");
    ASSERT_GE(chuteDef, 0);
    const EnemyDef& def = rig.Lib().Get(chuteDef);
    ASSERT_GT(def.flyingBzk.fallGravity, 0.f);
    const float maxStep = def.flyingBzk.fallMaxSpeed * (1.f / 60.f);

    SpawnParams params;
    params.facing            = 1;
    params.bActivated        = true;
    params.drop              = "gunitem_mg";
    const EnemyHandle handle = rig.Spawn("pow_chute", { kCamLeft + 300.f, -40.f }, params);

    float  prevY    = -40.f;
    float  prevStep = 0.f;
    bool   bLanded  = false;
    float  landedY  = 0.f;
    String landedDrop;
    for (int t = 0; t < 60 * 10 && !bLanded; ++t)
    {
        rig.Step(1);
        const Enemy* pChute = rig.E(handle);
        if (pChute && pChute->life == eEnemyLife::Alive && pChute->defIndex == chuteDef)
        {
            const float step = pChute->body.position.y - prevY;
            EXPECT_GE(step, prevStep - 0.001f) << "낙하 속도는 줄지 않는다 (tick " << t << ")";
            EXPECT_LE(step, maxStep + 0.001f) << "최대 속도 이하 (tick " << t << ")";
            prevStep = step;
            prevY    = pChute->body.position.y;
        }
        for (const Enemy& e: rig.World().GetEnemies())
        {
            if (e.defIndex == powDef && e.life == eEnemyLife::Alive)
            {
                bLanded    = true;
                landedY    = e.body.position.y;
                landedDrop = e.drop;
            }
        }
    }
    EXPECT_TRUE(bLanded);
    EXPECT_NEAR(prevStep, maxStep, 0.01f) << "최고 속도에 도달한 채로 착지";
    EXPECT_NEAR(landedY, 378.f, 0.6f) << "최고 속도에서도 지면을 뚫고 지나가지 않는다";
    EXPECT_EQ(landedDrop, "gunitem_mg");
}

TEST(BigShiee, ChuteHostageLandsAsStuckPowThatDropsAWeapon)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    (void)SpawnBoss_(rig);
    const int powDef = rig.Lib().FindIndex("pow");
    ASSERT_GE(powDef, 0);

    bool   landed = false;
    String drop;
    for (int t = 0; t < 60 * 20 && !landed; ++t)
    {
        rig.Step(1);
        for (const Enemy& e: rig.World().GetEnemies())
        {
            if (e.defIndex == powDef && e.life == eEnemyLife::Alive && e.bGrounded)
            {
                landed = true;
                drop   = e.drop;
            }
        }
    }
    EXPECT_TRUE(landed) << "낙하산 인질은 땅에 닿으면 묶인 POW 가 된다";
    EXPECT_TRUE(drop == "gunitem_mg" || drop == "gunitem_rl" || drop == "gunitem_sg" || drop == "gunitem_fs") << drop.c_str();
}

TEST(BigShiee, ChuteHostageSurvivesBulletsAndBlastsWhileDescending)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    (void)SpawnBoss_(rig);
    const int chuteDef = rig.Lib().FindIndex("pow_chute");
    ASSERT_GE(chuteDef, 0);
    rig.Step(static_cast<int>(6.5f * 60.f));

    uint32_t chuteId = 0;
    for (const Enemy& e: rig.World().GetEnemies())
    {
        if (e.defIndex == chuteDef)
        {
            chuteId = e.id;
        }
    }
    ASSERT_NE(chuteId, 0u) << "낙하산 인질이 떨어지지 않았다";
    EXPECT_EQ(rig.Strike(chuteId, eHitKind::Bullet, 5), 0u) << "낙하 중 플레이어 총알은 닿지 않는다";
    EXPECT_EQ(rig.Strike(chuteId, eHitKind::Blast, 5), 0u) << "낙하 중 플레이어 폭발도 닿지 않는다";
    rig.Step(60);

    bool bAlive = false;
    for (const Enemy& e: rig.World().GetEnemies())
    {
        if (e.id == chuteId && e.life == eEnemyLife::Alive)
        {
            bAlive = true;
        }
    }
    EXPECT_TRUE(bAlive) << "총알/폭발을 맞아도 살아서 착지해야 한다";
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);
}

TEST(BigShiee, BlastEndsInTheFirstHalfOfItsExplosionAnim)
{
    EnemyRig rig;
    LoadBossLib_(rig);
    Result<AnimationLibrary> lib = AnimationLibrary::LoadFromDirectory(FindAssetRoot());
    ASSERT_TRUE(lib) << "Assets 가 없다";
    AnimationLibrary anims = lib.Take();

    const EnemyDef& def = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    for (const char* weapon: { "shell", "missile" })
    {
        const int index = def.FindWeapon(weapon);
        ASSERT_GE(index, 0) << weapon;
        const ProjectileDef& proj = rig.Lib().GetProjectile(def.weapons[static_cast<size_t>(index)].projectile);
        ASSERT_GT(proj.blastSize.x, 0.f) << weapon;
        const AnimClip* pClip = anims.FindClip(proj.explosion.clip);
        ASSERT_NE(pClip, nullptr) << weapon << ": 폭발 클립 " << proj.explosion.clip.c_str();
        EXPECT_LT(proj.blastSec, pClip->GetTotalDurationSec()) << weapon;
        EXPECT_LE(proj.blastSec, 0.5f * pClip->GetTotalDurationSec()) << weapon << ": 판정은 폭발 애니 초반 절반 이내로 끝난다";
    }
}

TEST(BigShiee, ChuteDropsLandInsideTheScrollingViewNearTheLeftEdge)
{
    EnemyRig rig({ kCamLeft + 20.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    (void)SpawnBoss_(rig);
    const int chuteDef = rig.Lib().FindIndex("pow_chute");
    ASSERT_GE(chuteDef, 0);
    rig.Step(static_cast<int>(6.5f * 60.f));

    // 낙하 거리 (발 y 기준, 카메라 위 40px 에서 출발) / 낙하 속도 만큼 카메라가 오른쪽으로 간다 (scrollSpeed 20 px/s)
    constexpr float kFallSec = (378.f + 40.f) / 60.f;
    const float     drift    = 20.f * kFallSec;
    int             found    = 0;
    for (const Enemy& e: rig.World().GetEnemies())
    {
        if (e.defIndex == chuteDef)
        {
            ++found;
            EXPECT_GE(e.body.position.x, kCamLeft + drift) << "착지할 때 화면 왼쪽 밖으로 밀려나지 않는다";
        }
    }
    EXPECT_EQ(found, 1);
}

// ---- 주포 머리 (yaw 추적 / 발사 각도 == 사격 행 == 미사일 스프라이트) ----

namespace
{
// 포탑을 다 부수고 주포 전개가 시작되기 직전까지 진행한다 (플레이어는 _playerFeet)
[[nodiscard]] EnemyHandle StartCannon_(
    EnemyRig&  _rig,
    const Vec2 _playerFeet)
{
    LoadBossLib_(_rig);
    const EnemyHandle boss = SpawnBoss_(_rig);
    _rig.SetPlayer(_playerFeet);
    _rig.Step(30);
    KillAllTurrets_(_rig, boss);
    return boss;
}

// 플레이어를 보스 포구에서 _deg 방향 (수직 아래 0, 오른쪽 +) 의 지면 위 한 점으로 옮긴다
void PlacePlayerAtAngle_(
    EnemyRig&         _rig,
    const EnemyHandle _boss,
    const float       _deg)
{
    const Enemy*    pBoss  = _rig.E(_boss);
    const EnemyDef& def    = _rig.Lib().Get(_rig.Lib().FindIndex("bigshiee"));
    const Vec2      muzzle = { pBoss->body.position.x + def.bigShiee.cannonAimOrigin.x, pBoss->body.position.y + def.bigShiee.cannonAimOrigin.y };
    const float     feetY  = 378.f;
    const float     dy     = (feetY - 30.f) - muzzle.y;
    _rig.SetPlayer({ muzzle.x + std::tan(_deg * 3.14159265f / 180.f) * dy, feetY });
}

int CannonYaw_(
    const EnemyRig&   _rig,
    const EnemyHandle _boss)
{
    return _rig.E(_boss)->ext[kBigShieeExtCannonYaw];
}

eBigShieeCannonMode CannonMode_(
    const EnemyRig&   _rig,
    const EnemyHandle _boss)
{
    return static_cast<eBigShieeCannonMode>(_rig.E(_boss)->ext[kBigShieeExtCannonMode]);
}
}   // namespace

TEST(BigShiee, CannonHeadYawFollowsThePlayerSide)
{
    const ARRAY<float, 3> angles = { -40.f, 0.f, 40.f };
    ARRAY<int, 3>         yaws   = {};
    for (int i = 0; i < 3; ++i)
    {
        EnemyRig          rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
        const EnemyHandle boss = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
        for (int tick = 0; tick < 700 && CannonMode_(rig, boss) != eBigShieeCannonMode::Ready; ++tick)
        {
            PlacePlayerAtAngle_(rig, boss, angles[i]);
            rig.Step(1);
        }
        ASSERT_EQ(CannonMode_(rig, boss), eBigShieeCannonMode::Ready);
        rig.Step(30);
        yaws[i] = CannonYaw_(rig, boss);
    }
    EXPECT_LT(yaws[0], 4) << "플레이어가 왼쪽이면 머리가 왼쪽 (Aim 0..3) 을 본다";
    EXPECT_EQ(yaws[1], 4) << "바로 아래면 정면 (Aim 4)";
    EXPECT_GT(yaws[2], 4) << "오른쪽이면 오른쪽 (Aim 5..8)";
}

TEST(BigShiee, CannonTurnsOneFramePerTurnTicksAndOnlyFiresWhenAligned)
{
    EnemyRig          rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    const EnemyHandle boss = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
    const EnemyDef&   def  = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    const int         turn = def.bigShiee.cannonTurnTicks;

    int  aimStart  = -1;
    int  readyAt   = -1;
    int  lastYaw   = 0;
    int  lastStep  = -1;
    int  shots     = 0;
    bool bPrevShot = false;
    for (int tick = 0; tick < 900 && shots < 2; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, 0.f);
        rig.Step(1);
        const eBigShieeCannonMode mode = CannonMode_(rig, boss);
        const int                 yaw  = CannonYaw_(rig, boss);
        if (mode == eBigShieeCannonMode::Aim && aimStart < 0)
        {
            aimStart = tick;
            lastStep = tick;
            lastYaw  = yaw;
        }
        if (aimStart >= 0 && yaw != lastYaw)
        {
            EXPECT_EQ(std::abs(yaw - lastYaw), 1) << "한 번에 한 프레임씩만 돈다";
            EXPECT_GE(tick - lastStep, turn);
            lastYaw  = yaw;
            lastStep = tick;
        }
        if (mode == eBigShieeCannonMode::Ready && readyAt < 0)
        {
            readyAt = tick;
        }
        const bool bShot = mode == eBigShieeCannonMode::Shot;
        if (bShot)
        {
            EXPECT_EQ(yaw, rig.E(boss)->ext[kBigShieeExtCannonRow] * 2) << "발사 중 머리는 사격 행의 정면 프레임";
            EXPECT_EQ(yaw, 4);
        }
        if (bShot && !bPrevShot)
        {
            ++shots;
        }
        bPrevShot = bShot;
    }
    ASSERT_GE(aimStart, 0);
    ASSERT_GE(readyAt, 0);
    EXPECT_EQ(readyAt - aimStart, 4 * turn) << "Aim 0 -> 4 = 네 칸 x cannonTurnTicks 틱";
    EXPECT_EQ(shots, 2) << "Shot 뒤 Ready 로 돌아와 다시 쏜다";
}

TEST(BigShiee, MissileFliesAlongTheFiringRowAngleWithMatchingSprite)
{
    EnemyRig probe({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(probe);
    const EnemyDef& def    = probe.Lib().Get(probe.Lib().FindIndex("bigshiee"));
    const int       weapon = def.FindWeapon("missile");
    ASSERT_GE(weapon, 0);
    const int            projIndex = def.weapons[static_cast<size_t>(weapon)].projectile;
    const ProjectileDef& proj      = probe.Lib().GetProjectile(projIndex);
    EXPECT_TRUE(proj.bRotate) << "스프라이트는 속도 방향으로 돈다 (아래를 향한 Boss_Missile3 한 벌)";
    EXPECT_EQ(proj.fly.name, "Boss_Missile3");

    for (int row = 0; row < 5; ++row)
    {
        const float       deg = def.bigShiee.cannonAimDeg[static_cast<size_t>(row)];
        EnemyRig          rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
        const EnemyHandle boss  = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
        bool              bSeen = false;
        for (int tick = 0; tick < 1200 && !bSeen; ++tick)
        {
            PlacePlayerAtAngle_(rig, boss, deg);
            rig.Step(1);
            for (const Shot& shot: rig.Projectiles().GetShots())
            {
                if (shot.params.owner != boss.id || shot.params.visual != static_cast<uint16_t>(projIndex + 1) || shot.ageTicks > 1)
                {
                    continue;
                }
                bSeen                 = true;
                const float launchDeg = std::atan2(shot.velocity.x, shot.velocity.y) * (180.f / 3.14159265f);
                EXPECT_NEAR(launchDeg, deg, 0.05f) << "행 " << row << ": 발사 방향 == 행 각도";
                EXPECT_EQ(rig.E(boss)->ext[kBigShieeExtCannonRow], row) << "사격 행 == 발사한 행";
                EXPECT_EQ(CannonYaw_(rig, boss), row * 2) << "머리 프레임 == 사격 행 정면";
                const float spriteDeg = std::atan2(-shot.velocity.x, shot.velocity.y) * (180.f / 3.14159265f);
                EXPECT_NEAR(-spriteDeg, deg, 0.05f) << "스프라이트 회전 (아래 = 0, 시계 방향) 이 속도 방향과 같다";
            }
        }
        EXPECT_TRUE(bSeen) << "행 " << row << ": 고폭탄이 나가야 한다";
    }
}

// 사격 행 r 의 Aim 짝은 yaw 2r. 플레이어를 훑게 (-60 .. 60 도) 해서 쏘게 하고, 쏘기 직전 틱에 이미 그 프레임이 보였는지,
// 사격 중 머리가 안 도는지, 끝난 뒤 같은 프레임으로 돌아오는지 본다.
TEST(BigShiee, EachShotStartsAndEndsOnItsPairedAimFrame)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    rig.SetAnims(&anims);
    const EnemyHandle boss = StartCannon_(rig, { kCamLeft + 100.f, 378.f });

    int                 prevYaw  = -1;
    eBigShieeCannonMode prevMode = eBigShieeCannonMode::Hidden;
    int                 shots    = 0;
    int                 afterRow = -1;
    int                 shotLen  = 0;
    ARRAY<bool, 5>      rowSeen  = {};
    for (int tick = 0; tick < 3600 && shots < 14; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, 60.f * std::sin(static_cast<float>(tick) / 140.f));
        rig.Step(1);
        const eBigShieeCannonMode mode = CannonMode_(rig, boss);
        const int                 yaw  = CannonYaw_(rig, boss);
        const int                 row  = rig.E(boss)->ext[kBigShieeExtCannonRow];
        if (mode == eBigShieeCannonMode::Shot && prevMode != eBigShieeCannonMode::Shot)
        {
            ++shots;
            shotLen                           = 0;
            rowSeen[static_cast<size_t>(row)] = true;
            EXPECT_EQ(prevYaw, row * 2) << "쏘기 직전 틱에 이미 그 행의 Aim 프레임이 보였다 (샷 " << shots << ")";
            EXPECT_EQ(prevMode, eBigShieeCannonMode::Ready);
        }
        if (mode == eBigShieeCannonMode::Shot)
        {
            ++shotLen;
            EXPECT_EQ(yaw, row * 2) << "사격 중에는 머리가 돌지 않는다";
        }
        if (mode == eBigShieeCannonMode::Ready && prevMode == eBigShieeCannonMode::Shot)
        {
            afterRow = row;
            EXPECT_EQ(yaw, row * 2) << "사격이 끝나면 같은 Aim 프레임으로 돌아온다";
            const AnimClip* pClip = anims.FindClip("Boss_Cannon_Shot" + std::to_string(row + 1));
            ASSERT_NE(pClip, nullptr);
            const int total = static_cast<int>(pClip->GetTotalDurationSec() * 60.f);
            EXPECT_NEAR(shotLen, total * (pClip->GetFrameCount() - 1) / pClip->GetFrameCount(), 1) << "마지막 칸 (Aim 자세의 2 px 이동본) 은 보이지 않는다";
        }
        prevYaw  = yaw;
        prevMode = mode;
    }
    EXPECT_GE(shots, 10);
    EXPECT_GE(afterRow, 0);
    int distinct = 0;
    for (const bool b: rowSeen)
    {
        distinct += b ? 1 : 0;
    }
    EXPECT_GE(distinct, 4) << "훑는 동안 거의 모든 행이 쓰였다";
}

// 발사 각도: 포구에서 10 틱 날아간 변위의 방향이 사격 행 각도 (그림에서 잰 포신 방향) 와 5 도 안에서 같고,
// 플레이어가 있는 쪽의 행이 고르며 (왼쪽 = 음수 = Aim 0), 머리 프레임의 방향각도 플레이어 쪽이다.
TEST(BigShiee, MissileTravelDirectionMatchesTheBarrelArtWithinFiveDegrees)
{
    // 원작 영상 (V2 303 s, 카메라 정지 구간) 에서 잰 왼쪽 행 비행 방향 -47.3 도. Aim 프레임의 포구 중심 x (머리 중심 기준) 는
    // -62 / -29 / 0 / +28 / +61 px 로 좌우 대칭이고 선형 비례로 보면 -47 / -22 / 0 / +22 / +47 도.
    const ARRAY<float, 5> kBarrelDeg = { -47.f, -22.f, 0.f, 22.f, 47.f };
    EnemyRig              probe({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(probe);
    const EnemyDef& def       = probe.Lib().Get(probe.Lib().FindIndex("bigshiee"));
    const int       projIndex = def.weapons[static_cast<size_t>(def.FindWeapon("missile"))].projectile;
    for (int row = 0; row < 5; ++row)
    {
        EXPECT_NEAR(def.bigShiee.cannonAimDeg[static_cast<size_t>(row)], kBarrelDeg[static_cast<size_t>(row)], 5.f) << "행 " << row;

        EnemyRig          rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
        const EnemyHandle boss   = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
        Vec2              first  = {};
        Vec2              last   = {};
        bool              bFirst = false;
        bool              bLast  = false;
        for (int tick = 0; tick < 1500 && !bLast; ++tick)
        {
            PlacePlayerAtAngle_(rig, boss, kBarrelDeg[static_cast<size_t>(row)]);
            rig.Step(1);
            for (const Shot& shot: rig.Projectiles().GetShots())
            {
                if (shot.params.owner != boss.id || shot.params.visual != static_cast<uint16_t>(projIndex + 1))
                {
                    continue;
                }
                if (shot.ageTicks == 1 && !bFirst)
                {
                    first  = shot.position;
                    bFirst = true;
                }
                if (shot.ageTicks == 11 && bFirst && !bLast)
                {
                    last  = shot.position;
                    bLast = true;
                }
            }
        }
        ASSERT_TRUE(bLast) << "행 " << row;
        const float travelDeg = std::atan2(last.x - first.x, last.y - first.y) * (180.f / 3.14159265f);
        EXPECT_NEAR(travelDeg, kBarrelDeg[static_cast<size_t>(row)], 5.f) << "행 " << row << ": 날아간 방향 (아래 0, 오른쪽 +)";
        EXPECT_EQ(rig.E(boss)->ext[kBigShieeExtCannonRow], row);
    }
}

TEST(BigShiee, ReadyHeadFollowsThePlayerAngleWithoutMirroring)
{
    EnemyRig           rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    const EnemyHandle  boss = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
    const BigShieeCfg& c    = rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).bigShiee;
    for (int tick = 0; tick < 900 && CannonMode_(rig, boss) != eBigShieeCannonMode::Ready; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, 0.f);
        rig.Step(1);
    }
    ASSERT_EQ(CannonMode_(rig, boss), eBigShieeCannonMode::Ready);
    for (float deg = -50.f; deg <= 50.f; deg += 12.5f)
    {
        for (int tick = 0; tick < 100; ++tick)
        {
            PlacePlayerAtAngle_(rig, boss, deg);
            rig.Step(1);
            if (tick > 60 && CannonMode_(rig, boss) == eBigShieeCannonMode::Shot)
            {
                break;
            }
        }
        if (CannonMode_(rig, boss) == eBigShieeCannonMode::Shot)
        {
            continue;
        }
        const int   yaw   = CannonYaw_(rig, boss);
        const float low   = c.cannonAimDeg[static_cast<size_t>(yaw / 2)];
        const float shown = yaw % 2 == 0 ? low : (low + c.cannonAimDeg[static_cast<size_t>(yaw / 2 + 1)]) * 0.5f;
        EXPECT_NEAR(shown, deg, 14.f) << "플레이어 " << deg << " 도: 보이는 머리 yaw " << yaw << " (좌우가 뒤집히지 않는다)";
        EXPECT_EQ(deg < -6.f, shown < 0.f) << deg;
    }
}

// ---- 실제 애니 길이로 돌린 전개/발사 시퀀스 (원작 영상 V2 실측, Docs/Wayfinder/Boss_sync.md) ----

TEST(BigShieeSync, LidOpensFirstThenCannonRisesThenMissilesFireOnFootageCadence)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    rig.SetAnims(&anims);
    const EnemyHandle boss = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
    const EnemyDef&   def  = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    const int         proj = def.weapons[static_cast<size_t>(def.FindWeapon("missile"))].projectile;

    int           lidStart     = -1;
    int           deployStart  = -1;
    int           readyStart   = -1;
    ARRAY<int, 4> missileTicks = { -1, -1, -1, -1 };
    int           missiles     = 0;
    for (int tick = 0; tick < 1200 && missiles < 3; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, -50.f);
        rig.Step(1);
        const Enemy* pBoss = rig.E(boss);
        if (lidStart < 0 && pBoss->ext[kBigShieeExtLidStart] >= 0 && static_cast<eBigShieePhase>(pBoss->state) == eBigShieePhase::Deploy)
        {
            lidStart = tick;
        }
        if (deployStart < 0 && CannonMode_(rig, boss) == eBigShieeCannonMode::Deploy)
        {
            deployStart = tick;
        }
        if (readyStart < 0 && CannonMode_(rig, boss) == eBigShieeCannonMode::Ready)
        {
            readyStart = tick;
        }
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.owner != boss.id || shot.params.visual != static_cast<uint16_t>(proj + 1) || shot.ageTicks > 1)
            {
                continue;
            }
            missileTicks[static_cast<size_t>(missiles++)] = tick;
            const int  row                                = pBoss->ext[kBigShieeExtCannonRow];
            const Vec2 muzzle                             = def.weapons[static_cast<size_t>(def.FindWeapon("missile"))].muzzle;
            EXPECT_NEAR(shot.position.x, pBoss->body.position.x + muzzle.x + def.bigShiee.cannonMuzzleX[static_cast<size_t>(row)] - def.bigShiee.cannonMuzzleX[2], 6.f) << "고폭탄은 weapons.missile.muzzle + 행 변화량에서 나간다";
            EXPECT_NEAR(shot.position.y, pBoss->body.position.y + muzzle.y, 6.f);
        }
    }
    ASSERT_GE(lidStart, 0);
    ASSERT_GE(deployStart, 0);
    ASSERT_GE(readyStart, 0);
    ASSERT_GE(missiles, 3) << "주포가 Ready 가 된 뒤 고폭탄이 계속 나가야 한다";

    EXPECT_NEAR(deployStart - lidStart, SecToTicks(def.bigShiee.cannonDeployDelaySec), 3) << "뚜껑이 열린 뒤 주포가 올라온다 (원작: 약 1.2 s)";
    EXPECT_NEAR(readyStart - deployStart, def.bigShiee.cannonDeployTicks, 3) << "주포 전개 길이 (원작: 약 1.5 s)";
    EXPECT_NEAR(missileTicks[0] - readyStart, SecToTicks(def.bigShiee.firstMissileSec) + def.bigShiee.missileFireTick, 3) << "첫 발 (원작: 전개 끝 + 약 2.3 s)";
    EXPECT_NEAR(missileTicks[1] - missileTicks[0], SecToTicks(def.bigShiee.missileIntervalSec), 3) << "발사 간격 (원작: 약 3.04 s)";
    EXPECT_NEAR(missileTicks[2] - missileTicks[1], SecToTicks(def.bigShiee.missileIntervalSec), 3);
}

// ---- 카메라 예산 (아레나 끝) + 선체 왕복 ----

namespace
{
[[nodiscard]] float BossScreenX_(
    const EnemyRig&   _rig,
    const EnemyHandle _boss,
    const float       _camLeft)
{
    return _rig.E(_boss)->body.position.x - _camLeft;
}
}   // namespace

TEST(BigShieeScroll, StopsAfterBudgetThenShuttles)
{
    float    camLeft = kCamLeft;
    EnemyRig rig({ camLeft + 100.f, 378.f }, camLeft);
    LoadBossLib_(rig);
    const BigShieeCfg& c = rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).bigShiee;
    ASSERT_GT(c.scrollBudgetPx, 0.f);
    ASSERT_GT(c.shuttleDriftSec, 0.f);
    const EnemyHandle boss = SpawnBoss_(rig);

    float          scrolled   = 0.f;
    int            stopTick   = -1;
    int            prevDrive  = 0;
    float          minX       = 100000.f;
    float          maxX       = -100000.f;
    ARRAY<int, 16> surgeTicks = {};
    int            surges     = 0;
    for (int tick = 0; tick < 9000; ++tick)
    {
        rig.Step(1);
        const float dx = rig.World().TakeCameraScroll();
        if (stopTick >= 0)
        {
            EXPECT_EQ(dx, 0.f) << "예산을 다 쓴 뒤 카메라는 한 틱도 가지 않는다 (tick " << tick << ")";
        }
        else if (dx == 0.f && scrolled >= c.scrollBudgetPx - 1.f)
        {
            stopTick = tick;
        }
        camLeft += dx;
        scrolled += dx;
        rig.SetView(camLeft);
        rig.SetPlayer({ camLeft + 100.f, 378.f });
        if (stopTick >= 0 && tick > stopTick + 60)
        {
            const float screenX = BossScreenX_(rig, boss, camLeft);
            minX                = std::min(minX, screenX);
            maxX                = std::max(maxX, screenX);
            const int driving   = rig.E(boss)->ext[kBigShieeExtDriving];
            if (driving != 0 && prevDrive == 0 && surges < 16)
            {
                surgeTicks[static_cast<size_t>(surges++)] = tick;
            }
            prevDrive = driving;
        }
    }
    ASSERT_GE(stopTick, 0) << "누적 스크롤이 예산에 닿아야 한다";
    EXPECT_NEAR(scrolled, c.scrollBudgetPx, 1.f);
    EXPECT_GE(minX, c.driveTriggerX - 6.f) << "왕복 뒤쪽 끝";
    EXPECT_LE(maxX, c.driveTargetX + 1.f) << "왕복 앞쪽 끝";
    EXPECT_GT(maxX - minX, 60.f) << "선체가 실제로 오간다";
    ASSERT_GE(surges, 5) << "카메라가 멈춘 뒤에도 주기적으로 돌진한다";
    for (int i = 2; i < surges; ++i)
    {
        const float periodSec = static_cast<float>(surgeTicks[static_cast<size_t>(i)] - surgeTicks[static_cast<size_t>(i - 1)]) / 60.f;
        EXPECT_NEAR(periodSec, 9.f, 1.5f) << "돌진 주기는 약 9 s (원작 8.5~9 s)";
    }
}

TEST(BigShieeScroll, ReversesOnceAfterTheCannonDeploys)
{
    float    camLeft = kCamLeft;
    EnemyRig rig({ camLeft + 100.f, 378.f }, camLeft);
    LoadBossLib_(rig);
    const BigShieeCfg& c = rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).bigShiee;
    ASSERT_GT(c.reverseSec, 0.f);
    const EnemyHandle boss = SpawnBoss_(rig);
    rig.Step(30);
    KillAllTurrets_(rig, boss);

    int   backTicks = 0;
    float maxBack   = 0.f;
    float prevX     = rig.E(boss)->body.position.x;
    for (int tick = 0; tick < 60 * 40; ++tick)
    {
        rig.Step(1);
        const float x = rig.E(boss)->body.position.x;
        if (x < prevX - 0.01f)
        {
            ++backTicks;
            maxBack = std::max(maxBack, prevX - x);
        }
        prevX = x;
        camLeft += rig.World().TakeCameraScroll();
        rig.SetView(camLeft);
        rig.SetPlayer({ camLeft + 100.f, 378.f });
    }
    EXPECT_NEAR(static_cast<float>(backTicks), c.reverseSec * 60.f, 4.f) << "후진은 한 번, reverseSec 동안";
    EXPECT_NEAR(maxBack, c.reverseSpeed / 60.f, 0.2f);
}

// ---- 포탑 조준 (플레이어 쪽으로 머리를 돌려 Shot1/2/3) ----

namespace
{
[[nodiscard]] int ExpectedAimPos_(
    const Enemy&             _boss,
    const BigShieeCfg&       _c,
    const BigShieeTurretCfg& _turret,
    const float              _playerX)
{
    const float dx = _playerX - (_boss.body.position.x + _turret.anchor.x + _turret.muzzle.x);
    if (dx < -_c.turretAimBandPx)
    {
        return 0;
    }
    return dx > _c.turretAimBandPx ? 8 : 4;
}

// 플레이어 x = 선체 원점 + _anchorX. 포탑이 Shot 에 들어갈 때마다 머리 위치가 플레이어 쪽 구간과 맞는지 보고, 본 위치별 (0 / 4 / 8) 횟수를 돌려준다
[[nodiscard]] ARRAY<int, 3> RunTurretAim_(
    const float _anchorX,
    float&      _outShellVx)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyDef&    def  = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    const BigShieeCfg& c    = def.bigShiee;
    const int          proj = def.weapons[static_cast<size_t>(def.FindWeapon("shell"))].projectile;
    const EnemyHandle  boss = SpawnBoss_(rig);

    ARRAY<int, 3>                    seen     = {};
    ARRAY<int, kBigShieeTurretCount> prevMode = {};
    _outShellVx                               = 0.f;
    for (int tick = 0; tick < 1800; ++tick)
    {
        const Enemy* pBoss = rig.E(boss);
        rig.SetPlayer({ pBoss->body.position.x + _anchorX, 378.f });
        rig.Step(1);
        pBoss = rig.E(boss);
        for (int i = 0; i < kBigShieeTurretCount; ++i)
        {
            const int mode = pBoss->ext[static_cast<size_t>(kBigShieeExtMode + i)];
            if (mode == static_cast<int>(eBigShieeTurretMode::Shot) && prevMode[static_cast<size_t>(i)] != mode)
            {
                const int pos = pBoss->ext[static_cast<size_t>(kBigShieeExtPos + i)];
                EXPECT_EQ(pos, ExpectedAimPos_(*pBoss, c, c.turrets[static_cast<size_t>(i)], pBoss->body.position.x + _anchorX)) << "포탑 " << i;
                if (pos % 4 == 0)
                {
                    ++seen[static_cast<size_t>(pos / 4)];
                }
            }
            prevMode[static_cast<size_t>(i)] = mode;
        }
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.owner == boss.id && shot.params.visual == static_cast<uint16_t>(proj + 1) && shot.ageTicks <= 1 && std::abs(shot.velocity.x) > std::abs(_outShellVx))
            {
                _outShellVx = shot.velocity.x;
            }
        }
    }
    return seen;
}
}   // namespace

TEST(BigShieeTurretAim, FacesPlayerSideLeft)
{
    float               vx   = 0.f;
    const ARRAY<int, 3> seen = RunTurretAim_(-420.f, vx);
    EXPECT_GE(seen[0], 3) << "플레이어가 포탑 왼쪽이면 Shot1 (머리 위치 0)";
    EXPECT_EQ(seen[1], 0);
    EXPECT_EQ(seen[2], 0);
    EXPECT_LT(vx, -10.f) << "포탄은 왼쪽으로 난다";
}

TEST(BigShieeTurretAim, FacesPlayerSideRight)
{
    float               vx   = 0.f;
    const ARRAY<int, 3> seen = RunTurretAim_(420.f, vx);
    EXPECT_GE(seen[2], 3) << "플레이어가 포탑 오른쪽이면 Shot3 (머리 위치 8)";
    EXPECT_EQ(seen[0], 0);
    EXPECT_EQ(seen[1], 0);
    EXPECT_GT(vx, 10.f) << "포탄은 오른쪽으로 난다";
}

TEST(BigShieeTurretAim, FacesPlayerSideAbove)
{
    float               vx   = 0.f;
    const ARRAY<int, 3> seen = RunTurretAim_(80.f, vx);
    EXPECT_GE(seen[0] + seen[1] + seen[2], 3);
    EXPECT_GE(seen[1], 1) << "바로 아래 근처 포탑은 Shot2 (머리 위치 4)";
}

TEST(BigShieeTurretAim, IdleTurretsTrackThePlayerOneTurnFrameAtATime)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const BigShieeCfg& c    = rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).bigShiee;
    const EnemyHandle  boss = SpawnBoss_(rig);
    ASSERT_GT(c.turretTrackTicks, 0);
    rig.Step(1);
    int lastPos = -1;
    int maxStep = 0;
    for (int tick = 0; tick < c.turretTrackTicks * 12; ++tick)
    {
        rig.SetPlayer({ rig.E(boss)->body.position.x - 900.f, 378.f });
        rig.Step(1);
        const int pos = rig.E(boss)->ext[static_cast<size_t>(kBigShieeExtPos + 3)];
        if (lastPos >= 0)
        {
            maxStep = std::max(maxStep, std::abs(pos - lastPos));
        }
        lastPos = pos;
    }
    EXPECT_LE(maxStep, 1) << "머리는 한 번에 한 칸씩 (Turn 클립 프레임) 돈다";
    EXPECT_EQ(lastPos, 0) << "플레이어가 왼쪽이면 결국 왼쪽 (Turn 프레임 0)";
}

// ---- 고폭탄 가속 / 탄피 ----

TEST(BigShieeMissile, AcceleratesFromMuzzleToGround)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle    boss   = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
    const EnemyDef&      def    = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    const int            weapon = def.FindWeapon("missile");
    const int            proj   = def.weapons[static_cast<size_t>(weapon)].projectile;
    const ProjectileDef& pd     = rig.Lib().GetProjectile(proj);
    ASSERT_GT(pd.rampMax, pd.rampFrom);
    float first = -1.f;
    float last  = -1.f;
    for (int tick = 0; tick < 1200 && last < 0.f; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, 0.f);
        rig.Step(1);
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.owner != boss.id || shot.params.visual != static_cast<uint16_t>(proj + 1))
            {
                continue;
            }
            const float speed = std::sqrt(shot.velocity.x * shot.velocity.x + shot.velocity.y * shot.velocity.y);
            if (shot.ageTicks == 1)
            {
                first = speed;
            }
            if (shot.ageTicks == 25)
            {
                last = speed;
            }
        }
    }
    ASSERT_GT(first, 0.f);
    EXPECT_LT(first, last) << "포구에서는 느리고 낙하하며 빨라진다";
    EXPECT_LE(last, pd.rampMax + 0.5f);
}

TEST(BigShieeCasing, SpawnsOnEachShotAndFallsOffTheScreenWithin120Ticks)
{
    EnemyRig rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    LoadBossLib_(rig);
    const EnemyHandle  boss     = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
    const EnemyDef&    def      = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    const BigShieeCfg& c        = def.bigShiee;
    const Vec2         muzzle   = def.weapons[static_cast<size_t>(def.FindWeapon("missile"))].muzzle;
    int                lastTick = 0;
    int                spawns   = 0;
    for (int tick = 0; tick < 1200 && spawns < 2; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, 0.f);
        rig.Step(1);
        const int casing = rig.E(boss)->ext[kBigShieeExtCasingTick];
        if (casing != lastTick)
        {
            ++spawns;
            lastTick           = casing;
            const Enemy* pBoss = rig.E(boss);
            const int    row   = pBoss->ext[kBigShieeExtCannonRow];
            EXPECT_NEAR(static_cast<float>(pBoss->ext[kBigShieeExtCasingX]), pBoss->body.position.x + muzzle.x + c.cannonMuzzleX[static_cast<size_t>(row)] - c.cannonMuzzleX[2] + c.casingOffset.x, 2.f);
            EXPECT_NEAR(static_cast<float>(pBoss->ext[kBigShieeExtCasingY]), pBoss->body.position.y + muzzle.y + c.casingOffset.y, 2.f);
        }
    }
    EXPECT_EQ(spawns, 2) << "발마다 탄피 한 개";
    const float drop = -(muzzle.y + c.casingOffset.y) + c.casingGroundDy;
    const float land = (-c.casingVel.y + std::sqrt(c.casingVel.y * c.casingVel.y + 2.f * c.casingGravity * drop)) / c.casingGravity;
    EXPECT_LT((land + c.casingLingerSec) * 60.f, 120.f);
}

TEST(BigShieeCasing, HopsSlightlyThenFallsToTheGroundLineAtTwoPointFourScale)
{
    EnemyRig rig;
    LoadBossLib_(rig);
    const BigShieeCfg& c = rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).bigShiee;
    EXPECT_FLOAT_EQ(c.casingScale, 2.0f);
    EXPECT_LT(c.casingVel.y, 0.f) << "살짝 위로 튄다";
    const float hop = c.casingVel.y * c.casingVel.y / (2.f * c.casingGravity);
    EXPECT_GT(hop, 20.f) << "눈에 보이게 위로 튄다";
    EXPECT_LT(hop, 60.f) << "잔해보다 훨씬 작은 뜀";
    EXPECT_FLOAT_EQ(c.casingLingerSec, 0.f) << "지면에 닿으면 바로 사라진다";
    const ProjectileDef& missile = rig.Lib().GetProjectile(rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).weapons[static_cast<size_t>(rig.Lib().Get(rig.Lib().FindIndex("bigshiee")).FindWeapon("missile"))].projectile);
    EXPECT_FLOAT_EQ(missile.scale, 1.f) << "미사일은 원래 크기";
}

namespace
{
// 사용자가 모션 창에서 고치는 weapons.missile.muzzle 이 실제 발사 위치다: 값을 (dx, dy) 옮기면 모든 사격 행에서 첫 위치가 정확히 (dx*facing, dy) 만큼 옮겨 간다
[[nodiscard]] Vec2 FirstMissileAt_(
    const float _dx,
    const float _dy,
    const float _deg,
    int&        _outRow)
{
    EnemyRig          rig({ kCamLeft + 100.f, 378.f }, kCamLeft);
    const EnemyHandle boss   = StartCannon_(rig, { kCamLeft + 100.f, 378.f });
    EnemyDef*         pDef   = rig.Lib().FindMutable("bigshiee");
    const int         weapon = pDef->FindWeapon("missile");
    pDef->weapons[static_cast<size_t>(weapon)].muzzle.x += _dx;
    pDef->weapons[static_cast<size_t>(weapon)].muzzle.y += _dy;
    const int proj = pDef->weapons[static_cast<size_t>(weapon)].projectile;
    for (int tick = 0; tick < 1500; ++tick)
    {
        PlacePlayerAtAngle_(rig, boss, _deg);
        rig.Step(1);
        for (const Shot& shot: rig.Projectiles().GetShots())
        {
            if (shot.params.owner == boss.id && shot.params.visual == static_cast<uint16_t>(proj + 1) && shot.ageTicks <= 1)
            {
                _outRow = rig.E(boss)->ext[kBigShieeExtCannonRow];
                return { shot.position.x - rig.E(boss)->body.position.x, shot.position.y - rig.E(boss)->body.position.y };
            }
        }
    }
    return { -9999.f, -9999.f };
}
}   // namespace

TEST(BigShieeMissile, WeaponMuzzleIsTheRealLaunchOffsetForEveryRow)
{
    for (const float deg: { -47.f, -22.f, 0.f, 22.f, 47.f })
    {
        int        rowA = -1;
        int        rowB = -1;
        const Vec2 a    = FirstMissileAt_(0.f, 0.f, deg, rowA);
        const Vec2 b    = FirstMissileAt_(13.f, -9.f, deg, rowB);
        ASSERT_GE(rowA, 0);
        EXPECT_EQ(rowA, rowB);
        EXPECT_NEAR(b.x - a.x, 13.f, 0.01f) << "deg " << deg;
        EXPECT_NEAR(b.y - a.y, -9.f, 0.01f) << "deg " << deg;
    }
}

TEST(BigShieeMissile, FrontRowLaunchesAtTheWeaponMuzzleAndOtherRowsShiftByTheirDelta)
{
    EnemyRig rig;
    LoadBossLib_(rig);
    const EnemyDef& def    = rig.Lib().Get(rig.Lib().FindIndex("bigshiee"));
    const Vec2      muzzle = def.weapons[static_cast<size_t>(def.FindWeapon("missile"))].muzzle;
    int             row    = -1;
    const Vec2      front  = FirstMissileAt_(0.f, 0.f, 0.f, row);
    EXPECT_EQ(row, 2);
    EXPECT_NEAR(front.x, muzzle.x, 1.f);
    EXPECT_NEAR(front.y, muzzle.y, 6.f);
    const Vec2 left = FirstMissileAt_(0.f, 0.f, -47.f, row);
    EXPECT_EQ(row, 0);
    EXPECT_NEAR(left.x - front.x, def.bigShiee.cannonMuzzleX[0], 1.f);
}

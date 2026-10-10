// 플레이어 무기 테스트: 무기별 발사 속도, 탄약, 투사체 폭발/연기, 조준 방향, 건 아이템 정의, 결정성 검증
#include <algorithm>
#include <vector>

#include "Base/Math.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Props/PropDef.h"
#include "PlayerTestUtil.h"
#include "PropTestUtil.h"

using namespace slugtest;

namespace
{
[[nodiscard]] const TrainingDummy& DummyOf_(
    Sim&           _sim,
    const uint32_t _id)
{
    const TrainingDummy* pDummy = _sim.State().GetTargets().Find(_id);
    EXPECT_NE(pDummy, nullptr);
    static const TrainingDummy kEmpty;
    return pDummy ? *pDummy : kEmpty;
}

[[nodiscard]] size_t BulletsOf_(
    Sim& _sim)
{
    return _sim.State().GetProjectiles().GetBullets().size();
}

constexpr ARRAY<int, 4> kWeapons = {
    static_cast<int>(eWeapon::Hmg),
    static_cast<int>(eWeapon::Shotgun),
    static_cast<int>(eWeapon::Rocket),
    static_cast<int>(eWeapon::Flame),
};

// 무기를 집고 90틱 동안 쏜 뒤의 상태 해시
[[nodiscard]] uint64_t HashAfterShooting_(
    const eWeapon _weapon)
{
    Sim sim;
    sim.Player().PickUpWeapon(_weapon, sim.Tune());
    sim.Hold(eAction::Shot);
    sim.Step(90);
    return sim.State().HashState();
}

[[nodiscard]] int FireIntervalOf_(
    const Tuning& _tuning,
    const eWeapon _weapon)
{
    const WeaponTuning& weapon = _tuning.weapons.Get(_weapon);
    return slug::Max(weapon.fireIntervalTicks, weapon.holdRepeatTicks);
}
}   // namespace

TEST(PlayerWeapon, EveryWeaponFiresAtItsTunedIntervalWhenTapped)
{
    for (const int raw: kWeapons)
    {
        const eWeapon weapon = static_cast<eWeapon>(raw);
        if (weapon == eWeapon::Hmg)
        {
            continue;   // HMG 는 버스트 테스트가 따로 있다
        }
        Sim sim;
        sim.Player().PickUpWeapon(weapon, sim.Tune());
        const int interval = FireIntervalOf_(sim.Tune(), weapon);

        std::vector<int> shotTicks;
        int              lastShots = 0;
        for (int tick = 0; tick < 120; ++tick)
        {
            sim.Set(eAction::Shot, tick % 2 == 0);   // 매 틱 연타해도 간격 아래로는 못 쏜다
            sim.Step();
            const int shots = sim.Player().GetStats().shotsFired;
            if (shots > lastShots)
            {
                shotTicks.push_back(tick);
                lastShots = shots;
            }
        }
        ASSERT_GE(shotTicks.size(), 3u) << "weapon " << raw;
        for (size_t i = 1; i < shotTicks.size(); ++i)
        {
            EXPECT_GE(shotTicks[i] - shotTicks[i - 1], interval) << "weapon " << raw;
        }
    }
}

TEST(PlayerWeapon, TapOnlyWeaponsNeverRepeatOnHold)
{
    for (const eWeapon weapon: { eWeapon::Pistol, eWeapon::Shotgun, eWeapon::Rocket, eWeapon::Flame })
    {
        Sim sim;
        sim.Player().PickUpWeapon(weapon, sim.Tune());
        EXPECT_EQ(sim.Tune().weapons.Get(weapon).holdRepeatTicks, 0);
        sim.Hold(eAction::Shot);
        sim.Step(300);
        EXPECT_EQ(sim.Player().GetStats().shotsFired, 1) << "weapon " << static_cast<int>(weapon);
    }
}

TEST(PlayerWeapon, HmgBurstBulletsFollowTheAimAngleOfEachShot)
{
    // 버스트 도중 위를 누르면 조준각이 스윕하므로 뒤쪽 탄일수록 더 위를 향한다
    Sim sim;
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Tap(eAction::Shot);
    sim.Hold(eAction::Up);
    sim.Step(40);
    const Span<const Bullet> bullets = sim.State().GetProjectiles().GetBullets();
    ASSERT_GE(bullets.size(), 2u);
    EXPECT_LT(bullets[bullets.size() - 1].velocity.y, bullets[0].velocity.y);
}

TEST(PlayerWeapon, PickupAmmoComesFromTuningAndSameWeaponStacks)
{
    for (const int raw: kWeapons)
    {
        const eWeapon weapon = static_cast<eWeapon>(raw);
        Sim           sim;
        sim.Player().PickUpWeapon(weapon, sim.Tune());
        const int ammo = sim.Tune().weapons.GetPickupAmmo(weapon);
        EXPECT_GT(ammo, 0);
        EXPECT_EQ(sim.Player().GetWeapon().weapon, weapon);
        EXPECT_EQ(sim.Player().GetWeapon().ammo, ammo);

        sim.Tap(eAction::Shot);
        const int afterShot = sim.Player().GetWeapon().ammo;
        EXPECT_EQ(afterShot, ammo - 1) << "weapon " << raw;

        sim.Player().PickUpWeapon(weapon, sim.Tune());
        EXPECT_EQ(sim.Player().GetWeapon().ammo, afterShot + ammo) << "weapon " << raw;
    }
}

TEST(PlayerWeapon, PickingAnotherWeaponReplacesTheCurrentOneWithItsOwnAmmo)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Player().PickUpWeapon(eWeapon::Rocket, sim.Tune());
    EXPECT_EQ(sim.Player().GetWeapon().weapon, eWeapon::Rocket);
    EXPECT_EQ(sim.Player().GetWeapon().ammo, sim.Tune().weapons.rocketPickupAmmo);
}

TEST(PlayerWeapon, EmptyWeaponRevertsToPistolAfterExactlyItsAmmo)
{
    for (const int raw: kWeapons)
    {
        const eWeapon weapon = static_cast<eWeapon>(raw);
        Sim           sim;
        sim.Player().PickUpWeapon(weapon, sim.Tune());
        const int ammo = sim.Tune().weapons.GetPickupAmmo(weapon);

        int ticks = -1;
        for (int tick = 0; tick < 4000 && ticks < 0; ++tick)
        {
            sim.Set(eAction::Shot, tick % 40 == 0);   // 연타 (HMG 는 버스트가 끝난 뒤 다시 누른다)
            sim.Step();
            ticks = sim.Player().GetWeapon().weapon == eWeapon::Pistol ? tick : -1;
        }
        ASSERT_GE(ticks, 0) << "weapon " << raw;
        EXPECT_EQ(sim.Player().GetStats().shotsFired, ammo) << "weapon " << raw;
        EXPECT_EQ(sim.Player().GetWeapon().ammo, 0);
    }
}

TEST(PlayerWeapon, RocketRespectsItsMaxAliveLimit)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Rocket, sim.Tune());
    sim.Hold(eAction::Shot);
    int maxSeen = 0;
    for (int tick = 0; tick < 200; ++tick)
    {
        sim.Step();
        maxSeen = std::max(maxSeen, static_cast<int>(BulletsOf_(sim)));
    }
    EXPECT_LE(maxSeen, sim.Tune().weapons.rocket.maxAlive);
    EXPECT_GE(maxSeen, 1);
}

TEST(PlayerWeapon, HmgShotDamagesAJeepByItsTunedDamage)
{
    Sim            sim;
    const uint32_t id = sim.State().AddTarget(eDummyKind::Jeep, { 260.f, 378.f });
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    const int hp = DummyOf_(sim, id).health.hp;
    sim.Tap(eAction::Shot);
    sim.Step(30);
    EXPECT_EQ(DummyOf_(sim, id).health.hp, hp - sim.Tune().weapons.hmg.damage * sim.Tune().weapons.hmg.burstShots);   // 한 번 눌러 버스트 전체가 명중
}

TEST(PlayerWeapon, ShotgunAndFlamePierceBothJeeps)
{
    for (const eWeapon weapon: { eWeapon::Shotgun, eWeapon::Flame })
    {
        Sim sim;
        sim.State().AddTarget(eDummyKind::Jeep, { 220.f, 378.f });
        // 화염 사거리는 200px 이라 두 번째 지프는 그 안에 둔다 (속도 180px/s 라 도달까지 약 60틱)
        const uint32_t second = sim.State().AddTarget(eDummyKind::Jeep, { 290.f, 378.f });
        sim.Player().PickUpWeapon(weapon, sim.Tune());
        sim.Hold(eAction::Shot);
        sim.Step(70);
        sim.Release(eAction::Shot);
        EXPECT_GT(DummyOf_(sim, second).hitsTaken, 0) << "weapon " << static_cast<int>(weapon);
    }
}

TEST(PlayerWeapon, RocketBlastDamagesDirectTargetOnceAndSparesFarTargets)
{
    Sim            sim;
    const uint32_t direct = sim.State().AddTarget(eDummyKind::Jeep, { 300.f, 378.f });
    const uint32_t far    = sim.State().AddTarget(eDummyKind::Jeep, { 1100.f, 378.f });
    sim.Player().PickUpWeapon(eWeapon::Rocket, sim.Tune());
    const int directHp = DummyOf_(sim, direct).health.hp;
    const int farHp    = DummyOf_(sim, far).health.hp;

    sim.Tap(eAction::Shot);
    sim.Step(120);

    EXPECT_EQ(DummyOf_(sim, direct).health.hp, directHp - sim.Tune().weapons.rocket.blastDamage);
    EXPECT_EQ(DummyOf_(sim, far).health.hp, farHp);
}

TEST(PlayerWeapon, ProjectileBlastAppearsOnRangeEndOnlyWithBlastSize)
{
    for (const bool bBlast: { false, true })
    {
        ProjectileSystem projectiles;
        CombatSystem     combat;
        BulletSpawn      spawn;
        spawn.position    = { 100.f, 200.f };
        spawn.sweepFrom   = spawn.position;
        spawn.direction   = { 1.f, 0.f };
        spawn.speed       = 300.f;
        spawn.range       = 60.f;
        spawn.blastSize   = bBlast ? Vec2 { 96.f, 96.f } : Vec2 { 0.f, 0.f };
        spawn.blastDamage = 15;
        spawn.blastTicks  = 6;

        bool bSawBlast = false;
        combat.BeginTick();
        projectiles.SpawnBullet(spawn, combat);
        for (int tick = 0; tick < 20; ++tick)
        {
            combat.BeginTick();
            projectiles.Step(nullptr, combat, 1.f / 60.f);
            bSawBlast = bSawBlast || !projectiles.GetBlasts().empty();
        }
        EXPECT_EQ(bSawBlast, bBlast);
        EXPECT_EQ(projectiles.GetBullets().size(), 0u);
    }
}

TEST(PlayerWeapon, RocketGroundImpactSpawnsBlastAndExplosionFx)
{
    ProjectileSystem projectiles;
    CombatSystem     combat;
    const GroundMap  ground = MakeFlatGround();
    BulletSpawn      spawn;
    spawn.position    = { 100.f, 300.f };
    spawn.sweepFrom   = spawn.position;
    spawn.direction   = { 0.f, 1.f };
    spawn.speed       = 600.f;
    spawn.range       = 500.f;
    spawn.blastSize   = { 96.f, 96.f };
    spawn.blastDamage = 15;
    spawn.blastTicks  = 6;

    combat.BeginTick();
    projectiles.SpawnBullet(spawn, combat);
    bool bExplosionFx = false;
    for (int tick = 0; tick < 30 && projectiles.GetBullets().size() > 0; ++tick)
    {
        combat.BeginTick();
        projectiles.Step(&ground, combat, 1.f / 60.f);
        for (const FxRequest& fx: projectiles.TakeFx())
        {
            bExplosionFx = bExplosionFx || fx.kind == eFxKind::RocketExplosion;
        }
    }
    EXPECT_TRUE(bExplosionFx);
    EXPECT_FALSE(projectiles.GetBlasts().empty());
}

TEST(PlayerWeapon, RocketLeavesSmokeTrailFx)
{
    ProjectileSystem projectiles;
    CombatSystem     combat;
    BulletSpawn      spawn;
    spawn.position  = { 100.f, 200.f };
    spawn.sweepFrom = spawn.position;
    spawn.direction = { 1.f, 0.f };
    spawn.speed     = 300.f;
    spawn.range     = 400.f;
    spawn.bTrail    = true;

    combat.BeginTick();
    projectiles.SpawnBullet(spawn, combat);
    int smoke = 0;
    for (int tick = 0; tick < 27; ++tick)
    {
        combat.BeginTick();
        projectiles.Step(nullptr, combat, 1.f / 60.f);
        for (const FxRequest& fx: projectiles.TakeFx())
        {
            smoke += fx.kind == eFxKind::RocketSmoke ? 1 : 0;
        }
    }
    EXPECT_GE(smoke, 3);
}

TEST(PlayerWeapon, AimStatesFireEveryWeaponInTheExpectedDirection)
{
    struct Aim
    {
        const char* name   = "";
        bool        bUp    = false;
        bool        bRight = false;
        bool        bDown  = false;
        bool        bAir   = false;
        float       signX  = 0.f;
        float       signY  = 0.f;
    };
    // 위 + 방향(upRight)도 모든 무기가 똑바로 위 (HMG 는 정지 조준하면 각도 90도)
    const ARRAY<Aim, 5> aims = {
        {
         { "forward", false, true, false, false, 1.f, 0.f },
         { "up", true, false, false, false, 0.f, -1.f },
         { "upRight", true, true, false, false, 0.f, -1.f },
         { "crouch", false, true, true, false, 1.f, 0.f },
         { "airDown", false, false, true, true, 0.f, 1.f },
         }
    };

    for (const int raw: kWeapons)
    {
        const eWeapon weapon = static_cast<eWeapon>(raw);
        for (const Aim& aim: aims)
        {
            Sim sim;
            sim.Step(2);
            if (aim.bAir)
            {
                sim.Tap(eAction::Jump);
                sim.Step(10);
            }
            sim.Player().PickUpWeapon(weapon, sim.Tune());
            sim.Set(eAction::Up, aim.bUp);
            sim.Set(eAction::Right, aim.bRight);
            sim.Set(eAction::Down, aim.bDown);
            sim.Step(kAimSettleTicks);   // 조준 키를 누른 채 조준각이 목표에 닿을 때까지 기다린다
            sim.Hold(eAction::Shot);
            sim.Step();

            ASSERT_GE(BulletsOf_(sim), 1u) << aim.name << " weapon " << raw;
            const Vec2 v = sim.State().GetProjectiles().GetBullets()[0].velocity;
            if (aim.signX != 0.f)   // 똑바로 위/아래 탄은 cos(90도) 의 미세한 부호 오차가 있을 수 있어 크기만 본다
            {
                EXPECT_EQ(v.x > 0.f, aim.signX > 0.f) << aim.name << " weapon " << raw;
            }
            EXPECT_EQ(std::abs(v.x) < 0.01f, aim.signX == 0.f) << aim.name << " weapon " << raw;
            EXPECT_EQ(v.y < 0.f, aim.signY < 0.f) << aim.name << " weapon " << raw;
            EXPECT_EQ(std::abs(v.y) < 0.01f, aim.signY == 0.f) << aim.name << " weapon " << raw;
        }
    }
}

TEST(PlayerWeapon, GunItemDefsForAllFourWeaponsMapToTheirWeapon)
{
    struct Case
    {
        const char* id     = "";
        eWeapon     weapon = eWeapon::Pistol;
    };
    const ARRAY<Case, 4> cases = {
        {
         { "gunitem_mg", eWeapon::Hmg },
         { "gunitem_sg", eWeapon::Shotgun },
         { "gunitem_rl", eWeapon::Rocket },
         { "gunitem_fs", eWeapon::Flame },
         }
    };
    for (const Case& c: cases)
    {
        const PropItemDef* pDef = GetPropLib().FindItem(c.id);
        ASSERT_NE(pDef, nullptr) << c.id;
        EXPECT_EQ(pDef->kind, eItemKind::Gun) << c.id;
        EXPECT_EQ(pDef->weapon, c.weapon) << c.id;
    }
}

TEST(PlayerWeapon, DebugSlotsMapToTheFourPickupWeapons)
{
    EXPECT_EQ(WeaponFromDebugSlot(1), eWeapon::Hmg);
    EXPECT_EQ(WeaponFromDebugSlot(2), eWeapon::Shotgun);
    EXPECT_EQ(WeaponFromDebugSlot(3), eWeapon::Rocket);
    EXPECT_EQ(WeaponFromDebugSlot(4), eWeapon::Flame);
    EXPECT_EQ(WeaponFromDebugSlot(0), eWeapon::Pistol);
}

TEST(PlayerWeapon, DeterminismHashIsStableAndSeesTheWeapon)
{
    const uint64_t flame = HashAfterShooting_(eWeapon::Flame);
    EXPECT_EQ(flame, HashAfterShooting_(eWeapon::Flame));
    EXPECT_NE(flame, HashAfterShooting_(eWeapon::Rocket));
    EXPECT_NE(HashAfterShooting_(eWeapon::Hmg), HashAfterShooting_(eWeapon::Shotgun));
}

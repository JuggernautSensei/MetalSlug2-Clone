#include <algorithm>
#include <cmath>
#include <vector>

#include <SDL3/SDL.h>

#include "Base/Math.h"
#include "Game/Combat/BulletVisual.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Props/PropDef.h"
#include "PlayerTestUtil.h"
#include "PropTestUtil.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"

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
}   // namespace

constexpr ARRAY<int, 4> kWeapons = {
    static_cast<int>(eWeapon::Hmg),
    static_cast<int>(eWeapon::Shotgun),
    static_cast<int>(eWeapon::Rocket),
    static_cast<int>(eWeapon::Flame),
};

namespace
{
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
    return Max(weapon.fireIntervalTicks, weapon.holdRepeatTicks);
}
}   // namespace

TEST(PlayerWeapon, EveryWeaponFiresAtItsTunedIntervalWhenTapped)
{
    for (const int raw: kWeapons)
    {
        const eWeapon weapon = static_cast<eWeapon>(raw);
        if (weapon == eWeapon::Hmg)
        {
            continue;
        }
        Sim sim;
        sim.Player().PickUpWeapon(weapon, sim.Tune());
        const int interval = FireIntervalOf_(sim.Tune(), weapon);

        Vector<int> shotTicks;
        int         lastShots = 0;
        for (int tick = 0; tick < 120; ++tick)
        {
            sim.Set(eAction::Shot, tick % 2 == 0);
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

TEST(PlayerWeapon, EveryWeaponKeepsTheShotOverlayWhileRunning)
{
    for (const eWeapon weapon: { eWeapon::Pistol, eWeapon::Hmg, eWeapon::Shotgun, eWeapon::Rocket, eWeapon::Flame })
    {
        Sim sim;
        if (weapon != eWeapon::Pistol)
        {
            sim.Player().PickUpWeapon(weapon, sim.Tune());
        }
        sim.Hold(eAction::Right);
        sim.Step(5);
        sim.Tap(eAction::Shot);

        int  overlayTicks    = 0;
        int  movingShotTicks = 0;
        bool bShotEnded      = false;
        bool bGap            = false;
        for (int tick = 0; tick < 120; ++tick)
        {
            if (sim.Player().MakeRigState().overlay == eOverlay::Shot)
            {
                bGap = bGap || bShotEnded;
                ++overlayTicks;
                movingShotTicks += sim.Player().IsMoving() ? 1 : 0;
            }
            else
            {
                bShotEnded = true;
            }
            sim.Step();
        }

        const WeaponTuning& tuning   = sim.Tune().weapons.Get(weapon);
        const int           expected = weapon == eWeapon::Hmg ? sim.Player().GetStats().shotsFired * tuning.burstIntervalTicks : tuning.shotOverlayTicks;
        EXPECT_EQ(overlayTicks, expected) << "weapon " << static_cast<int>(weapon);
        EXPECT_FALSE(bGap) << "weapon " << static_cast<int>(weapon);
        EXPECT_EQ(movingShotTicks, overlayTicks) << "weapon " << static_cast<int>(weapon);
    }
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
            sim.Set(eAction::Shot, tick % 40 == 0);
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
        maxSeen = Max(maxSeen, static_cast<int>(BulletsOf_(sim)));
    }
    EXPECT_LE(maxSeen, sim.Tune().weapons.rocket.maxAlive);
    EXPECT_GE(maxSeen, 1);
}

TEST(PlayerWeapon, HmgShotDamagesAJeepByItsTunedDamage)
{
    Sim            sim;
    const uint32_t id = sim.State().AddTarget(eDummyKind::Jeep, { 260.f, 378.f });
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    const int hp                        = DummyOf_(sim, id).health.hp;
    sim.Tune().weapons.hmg.spreadDeg    = 0.f;
    sim.Tune().weapons.hmg.muzzleJitter = { 0.f, 0.f };
    sim.Tap(eAction::Shot);
    sim.Step(90);
    EXPECT_EQ(DummyOf_(sim, id).health.hp, hp - sim.Tune().weapons.hmg.damage * sim.Player().GetStats().shotsFired);
}

TEST(PlayerWeapon, ShotgunAndFlamePierceBothJeeps)
{
    for (const eWeapon weapon: { eWeapon::Shotgun, eWeapon::Flame })
    {
        Sim sim;
        sim.State().AddTarget(eDummyKind::Jeep, { 220.f, 378.f });
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
        spawn.blastSize   = bBlast ? VECTOR2 { 96.f, 96.f } : VECTOR2 { 0.f, 0.f };
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
            sim.Tune().weapons.hmg.spreadDeg      = 0.f;
            sim.Tune().weapons.hmg.muzzleJitter   = { 0.f, 0.f };
            sim.Tune().weapons.flame.spreadDeg    = 0.f;
            sim.Tune().weapons.flame.muzzleJitter = { 0.f, 0.f };
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
            sim.Step(kAimSettleTicks);
            sim.Hold(eAction::Shot);
            sim.Step();

            ASSERT_GE(BulletsOf_(sim), 1u) << aim.name << " weapon " << raw;
            const VECTOR2 v = sim.State().GetProjectiles().GetBullets()[0].velocity;
            if (aim.signX != 0.f)
            {
                EXPECT_EQ(v.x > 0.f, aim.signX > 0.f) << aim.name << " weapon " << raw;
            }
            EXPECT_EQ(Abs(v.x) < 0.01f, aim.signX == 0.f) << aim.name << " weapon " << raw;
            EXPECT_EQ(v.y < 0.f, aim.signY < 0.f) << aim.name << " weapon " << raw;
            EXPECT_EQ(Abs(v.y) < 0.01f, aim.signY == 0.f) << aim.name << " weapon " << raw;
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

namespace
{
[[nodiscard]] Bullet MakeVisualBullet_(
    const eWeapon _weapon,
    const VECTOR2 _velocity,
    const int     _ageTicks,
    const float   _travelled,
    const float   _range)
{
    Bullet bullet;
    bullet.tag        = static_cast<uint8_t>(_weapon);
    bullet.velocity   = _velocity;
    bullet.position   = { 100.f, 100.f };
    bullet.renderPrev = bullet.position;
    bullet.ageTicks   = _ageTicks;
    bullet.travelled  = _travelled;
    bullet.range      = _range;
    return bullet;
}
}   // namespace

TEST(PlayerWeaponVisual, FlameShotClipsExistInOutputAndEveryStageDraws)
{
    SLUG_REQUIRE_BAKED(anims);
    for (const char* name: { "FlameShot_Orig_Head", "FlameShot_Orig_Loop", "FlameShot_Orig_Fade" })
    {
        const AnimClip* pClip = anims.FindClip(name);
        ASSERT_NE(pClip, nullptr) << name;
        ASSERT_FALSE(pClip->frames.empty()) << name;
        for (const AnimFrame& frame: pClip->frames)
        {
            EXPECT_GT(frame.w, 0) << name;
            EXPECT_GT(frame.h, 0) << name;
        }
    }

    const SpriteLibrary sprites;
    SpriteBatch         batch;
    const VECTOR2       at = { 0.f, 0.f };
    EXPECT_TRUE(SubmitPlayerBullet(batch, &anims, sprites, MakeVisualBullet_(eWeapon::Flame, { 300.f, 0.f }, 0, 0.f, 220.f), at, 30));
    EXPECT_TRUE(SubmitPlayerBullet(batch, &anims, sprites, MakeVisualBullet_(eWeapon::Flame, { 300.f, 0.f }, 20, 100.f, 220.f), at, 30));
    EXPECT_TRUE(SubmitPlayerBullet(batch, &anims, sprites, MakeVisualBullet_(eWeapon::Flame, { 300.f, 0.f }, 60, 210.f, 220.f), at, 30));
    EXPECT_EQ(batch.GetPendingCommands().size(), 3u);
}

TEST(PlayerWeaponVisual, RocketSpriteRotatesToItsFlightDirection)
{
    SLUG_REQUIRE_BAKED(anims);
    const SpriteLibrary sprites;
    struct Case
    {
        VECTOR2 velocity = {};
        float   expectDeg;
    };
    const float kDiag = 640.f * 0.70710678f;
    for (const Case& c: {
             Case {     { 640.f, 0.f },   0.f },
             Case {    { -640.f, 0.f },   0.f },
             Case {    { 0.f, -640.f }, -90.f },
             Case {     { 0.f, 640.f },  90.f },
             Case {  { kDiag, -kDiag }, -45.f },
             Case { { -kDiag, -kDiag },  45.f },
             Case {  { -kDiag, kDiag }, -45.f },
    })
    {
        SpriteBatch batch;
        ASSERT_TRUE(SubmitPlayerBullet(batch, &anims, sprites, MakeVisualBullet_(eWeapon::Rocket, c.velocity, 0, 0.f, 420.f), { 0.f, 0.f }, 30));
        ASSERT_EQ(batch.GetPendingCommands().size(), 1u);
        EXPECT_NEAR(batch.GetPendingCommands().front().options.rotationDeg, c.expectDeg, 0.5f) << "vx=" << c.velocity.x << " vy=" << c.velocity.y;
    }
}

struct PixelBox_
{
    int minX = 1 << 20;
    int minY = 1 << 20;
    int maxX = -1;
    int maxY = -1;

    [[nodiscard]] int Count() const { return maxX < 0 ? 0 : (maxX - minX + 1) * (maxY - minY + 1); }
    [[nodiscard]] int Width() const { return maxX < 0 ? 0 : maxX - minX + 1; }
    [[nodiscard]] int Height() const { return maxY < 0 ? 0 : maxY - minY + 1; }
};

constexpr int kRasterSize_ = 256;

struct RasterFixture_
{
    SDL_Surface*  pSurface  = nullptr;
    SDL_Renderer* pRenderer = nullptr;
    SpriteBatch   batch     = {};
    bool          bReady    = false;

    RasterFixture_(const AnimationLibrary& _anims)
    {
        pSurface  = SDL_CreateSurface(kRasterSize_, kRasterSize_, SDL_PIXELFORMAT_RGBA32);
        pRenderer = pSurface ? SDL_CreateSoftwareRenderer(pSurface) : nullptr;
        if (!pRenderer)
        {
            return;
        }
        batch.Init(pRenderer);
        bReady = batch.LoadAtlasPagesParallel(_anims, 1);
    }

    ~RasterFixture_()
    {
        batch.Shutdown();
        if (pRenderer)
        {
            SDL_DestroyRenderer(pRenderer);
        }
        if (pSurface)
        {
            SDL_DestroySurface(pSurface);
        }
    }
};

namespace
{
[[nodiscard]] PixelBox_ DrawAndMeasure_(
    RasterFixture_&         _fx,
    const AnimationLibrary& _anims,
    const eWeapon           _weapon,
    const VECTOR2           _velocity,
    const int               _ageTicks,
    const float             _travelled,
    const float             _range)
{
    PixelBox_ box;
    if (!_fx.bReady)
    {
        return box;
    }
    SDL_Renderer*       pRenderer = _fx.pRenderer;
    SDL_Surface*        pSurface  = _fx.pSurface;
    SpriteBatch&        batch     = _fx.batch;
    const SpriteLibrary sprites;
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pRenderer);
    (void)SubmitPlayerBullet(batch, &_anims, sprites, MakeVisualBullet_(_weapon, _velocity, _ageTicks, _travelled, _range), { 128.f, 128.f }, 30);
    batch.Flush(VECTOR2 { 0.f, 0.f });
    SDL_FlushRenderer(pRenderer);

    for (int y = 0; y < kRasterSize_; ++y)
    {
        for (int x = 0; x < kRasterSize_; ++x)
        {
            Uint8 r = 0;
            Uint8 g = 0;
            Uint8 b = 0;
            Uint8 a = 0;
            SDL_ReadSurfacePixel(pSurface, x, y, &r, &g, &b, &a);
            if (r + g + b > 24)
            {
                box.minX = Min(box.minX, x);
                box.minY = Min(box.minY, y);
                box.maxX = Max(box.maxX, x);
                box.maxY = Max(box.maxY, y);
            }
        }
    }
    return box;
}
}   // namespace

TEST(PlayerWeaponVisual, FlameShotIsRasterisedAtEveryStage)
{
    SLUG_REQUIRE_BAKED(anims);
    RasterFixture_ fx(anims);
    if (!fx.bReady)
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 쓸 수 없는 환경";
    }
    const PixelBox_ head = DrawAndMeasure_(fx, anims, eWeapon::Flame, { 300.f, 0.f }, 0, 0.f, 220.f);
    const PixelBox_ loop = DrawAndMeasure_(fx, anims, eWeapon::Flame, { 300.f, 0.f }, 20, 100.f, 220.f);
    const PixelBox_ fade = DrawAndMeasure_(fx, anims, eWeapon::Flame, { 300.f, 0.f }, 60, 210.f, 220.f);
    EXPECT_GT(head.Count(), 0);
    EXPECT_GT(loop.Count(), 0);
    EXPECT_GT(fade.Count(), 0);
    EXPECT_GE(loop.Width(), 60);
}

TEST(PlayerWeaponVisual, RocketFiredUpIsDrawnTallNotWide)
{
    SLUG_REQUIRE_BAKED(anims);
    RasterFixture_ fx(anims);
    if (!fx.bReady)
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 쓸 수 없는 환경";
    }
    const PixelBox_ up  = DrawAndMeasure_(fx, anims, eWeapon::Rocket, { 0.f, -300.f }, 0, 0.f, 420.f);
    const PixelBox_ fwd = DrawAndMeasure_(fx, anims, eWeapon::Rocket, { 300.f, 0.f }, 0, 0.f, 420.f);
    ASSERT_GT(up.Count(), 0);
    ASSERT_GT(fwd.Count(), 0);
    EXPECT_GT(up.Height(), up.Width());
    EXPECT_GT(fwd.Width(), fwd.Height());
}

namespace
{
[[nodiscard]] Vector<int> FlameSpawnTicks_(
    Sim&      _sim,
    const int _ticks)
{
    Vector<int> spawnTicks;
    for (int tick = 0; tick < _ticks; ++tick)
    {
        _sim.Step();
        for (const Bullet& bullet: _sim.State().GetProjectiles().GetBullets())
        {
            if (bullet.tag == static_cast<uint8_t>(eWeapon::Flame) && bullet.ageTicks == 1)
            {
                spawnTicks.push_back(tick);
            }
        }
    }
    return spawnTicks;
}

[[nodiscard]] Vector<Bullet> FlameBulletsAfter_(
    const int _ticks)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Flame, sim.Tune());
    sim.Hold(eAction::Shot);
    sim.Step(_ticks);
    const Span<const Bullet> bullets = sim.State().GetProjectiles().GetBullets();
    return Vector<Bullet>(bullets.begin(), bullets.end());
}
}   // namespace

TEST(PlayerWeapon, ShotgunHitboxTurnsWithTheAimDirection)
{
    const Tuning        tuning   = {};
    const WeaponTuning& shotgun  = tuning.weapons.shotgun;
    const VECTOR2       side     = MakeBulletSpawn(eWeapon::Shotgun, shotgun, 0, { 0.f, 0.f }, { 0.f, 0.f }, { 1.f, 0.f }).size;
    const VECTOR2       up       = MakeBulletSpawn(eWeapon::Shotgun, shotgun, 0, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, -1.f }).size;
    const VECTOR2       down     = MakeBulletSpawn(eWeapon::Shotgun, shotgun, 0, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 1.f }).size;
    const VECTOR2       diagonal = MakeBulletSpawn(eWeapon::Shotgun, shotgun, 0, { 0.f, 0.f }, { 0.f, 0.f }, { 0.7071f, -0.7071f }).size;
    const float         longSide = Max(shotgun.hitSize.y, shotgun.hitSize.x);

    EXPECT_FLOAT_EQ(side.x, shotgun.hitSize.x);
    EXPECT_FLOAT_EQ(side.y, shotgun.hitSize.y);
    EXPECT_FLOAT_EQ(up.x, shotgun.hitSize.y);
    EXPECT_FLOAT_EQ(up.y, shotgun.hitSize.x);
    EXPECT_FLOAT_EQ(down.x, shotgun.hitSize.y);
    EXPECT_FLOAT_EQ(down.y, shotgun.hitSize.x);
    EXPECT_FLOAT_EQ(diagonal.x, longSide);
    EXPECT_FLOAT_EQ(diagonal.y, longSide);
}

TEST(PlayerWeapon, ShotgunFiredUpSpawnsATallHitboxInTheWorld)
{
    Sim sim;
    sim.Step(2);
    sim.Player().PickUpWeapon(eWeapon::Shotgun, sim.Tune());
    sim.Hold(eAction::Up);
    sim.Step(30);
    sim.Tap(eAction::Shot);
    const Span<const Bullet> bullets = sim.State().GetProjectiles().GetBullets();
    ASSERT_GE(bullets.size(), 1u);
    EXPECT_GT(bullets[0].size.y, bullets[0].size.x);
}

TEST(PlayerWeapon, FlameInputCostsOneAmmoNotOnePerParticle)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Flame, sim.Tune());
    const int ammoBefore = sim.Player().GetWeapon().ammo;
    sim.Tap(eAction::Shot);
    sim.Step(60);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
    EXPECT_EQ(sim.Player().GetWeapon().ammo, ammoBefore - 1);
}

TEST(PlayerWeapon, FlameSpraysParticlesInTimedPuffs)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Flame, sim.Tune());
    sim.Hold(eAction::Shot);
    const Vector<int>   spawnTicks = FlameSpawnTicks_(sim, 60);
    const WeaponTuning& flame      = sim.Tune().weapons.flame;
    ASSERT_EQ(spawnTicks.size(), static_cast<size_t>(flame.particleCount));
    for (size_t i = 1; i < spawnTicks.size(); ++i)
    {
        EXPECT_EQ(spawnTicks[i] - spawnTicks[i - 1], flame.particleIntervalTicks) << "particle " << i;
    }
}

TEST(PlayerWeapon, FlameKeepsSprayingWhileRunning)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Flame, sim.Tune());
    sim.Hold(eAction::Right);
    sim.Step(5);
    sim.Hold(eAction::Shot);
    const Vector<int> spawnTicks = FlameSpawnTicks_(sim, 60);
    EXPECT_EQ(spawnTicks.size(), static_cast<size_t>(sim.Tune().weapons.flame.particleCount));
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
}

TEST(PlayerWeapon, FlameTotalDamageToOneTargetIsCapped)
{
    Sim            sim;
    const uint32_t id = sim.State().AddTarget(eDummyKind::Jeep, { 200.f, 378.f });
    sim.Player().PickUpWeapon(eWeapon::Flame, sim.Tune());
    const int hpBefore = DummyOf_(sim, id).health.hp;
    sim.Tap(eAction::Shot);
    sim.Step(90);

    const WeaponTuning& flame      = sim.Tune().weapons.flame;
    const int           dealt      = hpBefore - DummyOf_(sim, id).health.hp;
    const int           maxWindows = flame.particleCount * flame.particleIntervalTicks / flame.hitReleaseTicks;
    EXPECT_GT(dealt, 0);
    EXPECT_LE(dealt, maxWindows * flame.damage);
}

TEST(PlayerWeapon, FlameParticlesAreDeterministicForTheSameInputs)
{
    const Vector<Bullet> first  = FlameBulletsAfter_(30);
    const Vector<Bullet> second = FlameBulletsAfter_(30);
    ASSERT_FALSE(first.empty());
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i)
    {
        EXPECT_EQ(first[i].position.x, second[i].position.x) << i;
        EXPECT_EQ(first[i].position.y, second[i].position.y) << i;
        EXPECT_EQ(first[i].velocity.x, second[i].velocity.x) << i;
        EXPECT_EQ(first[i].velocity.y, second[i].velocity.y) << i;
        EXPECT_EQ(first[i].range, second[i].range) << i;
    }
}

TEST(PlayerWeaponVisual, FlameAttackMotionIsBiggerThanTheOriginalLoop)
{
    SLUG_REQUIRE_BAKED(anims);
    const AnimClip* pNew = anims.FindClip("FlameShot_AttackMotion");
    const AnimClip* pOld = anims.FindClip("FlameShot_Orig_Loop");
    ASSERT_NE(pNew, nullptr);
    ASSERT_NE(pOld, nullptr);
    ASSERT_GE(pNew->GetFrameCount(), 12);

    int newArea = 0;
    int oldArea = 0;
    for (const AnimFrame& frame: pNew->frames)
    {
        newArea = Max(newArea, frame.w * frame.h);
    }
    for (const AnimFrame& frame: pOld->frames)
    {
        oldArea = Max(oldArea, frame.w * frame.h);
    }
    EXPECT_GT(newArea * 10, oldArea * 13);
}

// 플레이어 전투 테스트: 샷, 나이프, 폭탄, 사망/리스폰/게임오버와 투사체 판정 검증
#include <vector>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "Game/Player/PlayerHitboxes.h"
#include "PlayerTestUtil.h"

using namespace slugtest;

namespace
{
[[nodiscard]] const TrainingDummy& Dummy_(
    Sim&           _sim,
    const uint32_t _id)
{
    const TrainingDummy* pDummy = _sim.State().GetTargets().Find(_id);
    EXPECT_NE(pDummy, nullptr);
    static const TrainingDummy kEmpty;
    return pDummy ? *pDummy : kEmpty;
}

[[nodiscard]] size_t BulletCount_(
    Sim& _sim)
{
    return _sim.State().GetProjectiles().GetBullets().size();
}

[[nodiscard]] bool HasMeleeHitbox_(
    Sim& _sim)
{
    for (const Hitbox& hitbox: _sim.State().GetCombat().GetHitboxes())
    {
        if (hitbox.kind == eHitKind::Melee)
        {
            return true;
        }
    }
    return false;
}

void UnlimitedBullets_(
    Sim& _sim)
{
    _sim.Tune().weapons.pistol.maxAlive = 0;
    _sim.Tune().weapons.hmg.maxAlive    = 0;
}

// 무작위 입력 3000틱 스크립트 (시드 고정)
[[nodiscard]] slug::Vector<InputFrame> MakeNoisyScript_()
{
    slug::RngMt19937         rng { uint64_t { 7 } };
    slug::Vector<InputFrame> frames;
    InputFrame               cur;
    for (int i = 0; i < 3000; ++i)
    {
        if (i % 3 == 0)
        {
            const eAction a = static_cast<eAction>(slug::Random<int>(rng, 0, 6));
            cur.Set(a, !cur.IsHeld(a));
        }
        frames.push_back(cur);
    }
    return frames;
}

// 더미가 죽을 때까지 진행한다. 죽은 틱 수를, 끝내 안 죽으면 -1 을 돌려준다.
[[nodiscard]] int StepUntilDummyDead_(
    Sim&           _sim,
    const uint32_t _id,
    const int      _maxTicks)
{
    for (int i = 0; i < _maxTicks; ++i)
    {
        if (!Dummy_(_sim, _id).IsAlive())
        {
            return i;
        }
        _sim.Step();
    }
    return !Dummy_(_sim, _id).IsAlive() ? _maxTicks : -1;
}

// 다음 탄이 발사될 때까지 틱을 진행하고, 그 탄의 발사각(도, 위가 +)을 돌려준다. 발사가 없으면 NaN
[[nodiscard]] float StepUntilShotAngleDeg_(
    Sim&      _sim,
    const int _maxTicks)
{
    const int before = _sim.Player().GetStats().shotsFired;
    for (int i = 0; i < _maxTicks; ++i)
    {
        _sim.Step();
        if (_sim.Player().GetStats().shotsFired > before)
        {
            const slug::Span<const Bullet> bullets = _sim.State().GetProjectiles().GetBullets();
            const Vec2                     v       = bullets[bullets.size() - 1].velocity;
            return std::atan2(-v.y, std::abs(v.x)) * 180.f / 3.14159265f;
        }
    }
    return std::nan("");
}
}   // namespace

TEST(PlayerShot, FirstShotSpawnsABulletOnTheSameTick)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Shot);
    sim.Step();
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
    ASSERT_EQ(BulletCount_(sim), 1u);
    EXPECT_GT(sim.State().GetProjectiles().GetBullets()[0].position.x, sim.X());
    EXPECT_EQ(sim.Player().GetOverlay(), eOverlay::Shot);
}

TEST(PlayerShot, PistolNeverHasMoreThanMaxAliveBullets)
{
    Sim sim;
    const size_t cap  = static_cast<size_t>(sim.Tune().weapons.pistol.maxAlive);
    size_t       peak = 0;
    for (int i = 0; i < 200; ++i)
    {
        sim.Set(eAction::Shot, i % 2 == 0);   // 연타
        sim.Step();
        peak = std::max(peak, BulletCount_(sim));
        ASSERT_LE(BulletCount_(sim), cap) << "tick " << i;
    }
    EXPECT_EQ(peak, cap);
}

TEST(PlayerShot, HeldPistolFiresExactlyOneShot)
{
    Sim sim;
    UnlimitedBullets_(sim);
    EXPECT_EQ(sim.Tune().weapons.pistol.holdRepeatTicks, 0);
    sim.Hold(eAction::Shot);
    sim.Step(120);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);   // 홀드만으로는 자동 연사가 없다
    sim.Release(eAction::Shot);
    sim.Step(2);
    sim.Hold(eAction::Shot);
    sim.Step(120);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 2);   // 다시 눌러야 다음 발
}

TEST(PlayerShot, TappingCannotBeatTheFireInterval)
{
    Sim sim;
    UnlimitedBullets_(sim);
    const int interval = sim.Tune().weapons.pistol.fireIntervalTicks;
    for (int i = 0; i < 40; ++i)
    {
        if (i % 2 == 0)
        {
            sim.Hold(eAction::Shot);
        }
        else
        {
            sim.Release(eAction::Shot);
        }
        sim.Step();
    }
    EXPECT_EQ(sim.Player().GetStats().shotsFired, (39 / interval) + 1);
}

TEST(PlayerShot, TappingIsFasterThanHoldingButNeverFasterThanTheInterval)
{
    Sim tap;
    Sim hold;
    UnlimitedBullets_(tap);
    UnlimitedBullets_(hold);
    hold.Hold(eAction::Shot);
    for (int i = 0; i < 60; ++i)
    {
        tap.Set(eAction::Shot, i % 2 == 0);
        tap.Step();
        hold.Step();
    }
    EXPECT_GT(tap.Player().GetStats().shotsFired, hold.Player().GetStats().shotsFired);
}

TEST(PlayerShot, ShotPressedDuringCooldownIsBufferedAndFiresWhenReady)
{
    Sim sim;
    UnlimitedBullets_(sim);
    sim.Tune().weapons.pistol.fireIntervalTicks = 5;
    sim.Tap(eAction::Shot);
    sim.Step();
    sim.Tap(eAction::Shot);
    sim.Step(2);   // t3, t4
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
    sim.Step(2);   // t5 에 버퍼된 눌림이 발사된다
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 2);
}

TEST(PlayerShot, BufferedShotExpiresAfterBufferWindow)
{
    Sim sim;
    sim.Tune().weapons.pistol.fireIntervalTicks = 30;   // 쿨다운이 버퍼보다 훨씬 길다
    sim.Tap(eAction::Shot);
    sim.Step(2);
    sim.Tap(eAction::Shot);
    sim.Step(40);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);   // 6 틱 버퍼가 만료되어 두 번째는 사라진다
}

TEST(PlayerShot, HmgTapFiresABurstSpreadOverTheShotMotion)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    const WeaponTuning& hmg = sim.Tune().weapons.hmg;
    ASSERT_GE(hmg.burstShots, 3);
    ASSERT_LE(hmg.burstShots, 5);

    sim.Tap(eAction::Shot);   // 틱 0: 첫 발이 즉시 나간다
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
    std::vector<int> shotTicks = { 0 };
    int              last      = 1;
    for (int tick = 1; tick < 80; ++tick)
    {
        sim.Step();
        const int shots = sim.Player().GetStats().shotsFired;
        if (shots > last)
        {
            shotTicks.push_back(tick);
            last = shots;
        }
    }
    ASSERT_EQ(static_cast<int>(shotTicks.size()), hmg.burstShots);   // 한 번 눌러 버스트 하나
    for (size_t i = 1; i < shotTicks.size(); ++i)
    {
        EXPECT_EQ(shotTicks[i] - shotTicks[i - 1], hmg.burstIntervalTicks);
    }
    EXPECT_EQ(sim.Player().GetWeapon().ammo, hmg.ammo - hmg.burstShots);   // 탄 수는 발마다 줄어든다
}

TEST(PlayerShot, HmgHoldDoesNotAutoFireBeyondOneBurst)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Hold(eAction::Shot);
    sim.Step(180);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, sim.Tune().weapons.hmg.burstShots);
}

TEST(PlayerShot, MovingCancelsTheHmgBurst)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Tap(eAction::Shot);
    sim.Step(2);
    EXPECT_EQ(sim.Player().GetOverlay(), eOverlay::Shot);
    sim.Hold(eAction::Right);
    sim.Step(60);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);   // 이동하자 남은 탄이 취소된다
    EXPECT_EQ(sim.Player().GetOverlay(), eOverlay::None);
}

TEST(PlayerShot, FlameDoesNotAutoFireOnHold)
{
    Sim sim;
    sim.Player().PickUpWeapon(eWeapon::Flame, sim.Tune());
    sim.Hold(eAction::Shot);
    sim.Step(120);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
    EXPECT_GE(sim.Tune().weapons.flame.hitSize.x, 48.f);   // 확대된 불꽃
}

TEST(PlayerShot, FiniteAmmoDecreasesAndRevertsToPistol)
{
    Sim sim;
    sim.Tune().weapons.hmgPickupAmmo = 3;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    EXPECT_EQ(sim.Player().GetWeapon().weapon, eWeapon::Hmg);
    EXPECT_EQ(sim.Player().GetWeapon().ammo, 3);

    sim.Hold(eAction::Shot);
    const int reverted = sim.StepUntilWeapon(100, eWeapon::Pistol);
    ASSERT_GE(reverted, 0);                             // 소진되면 권총으로 복귀
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 3);   // 정확히 탄 수만큼 쏘고 복귀
}

TEST(PlayerShot, PickingUpSameWeaponAddsAmmo)
{
    Sim sim;
    sim.Tune().weapons.hmgPickupAmmo = 50;
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    EXPECT_EQ(sim.Player().GetWeapon().ammo, 100);
}

TEST(PlayerShot, ShotgunPelletPiercesBothTargetsButPistolHitsOnlyTheFirst)
{
    for (const eWeapon weapon: { eWeapon::Pistol, eWeapon::Shotgun })
    {
        Sim sim;
        sim.State().AddTarget(eDummyKind::Jeep, { 220.f, 378.f });
        const uint32_t second = sim.State().AddTarget(eDummyKind::Jeep, { 360.f, 378.f });
        if (weapon != eWeapon::Pistol)
        {
            sim.Player().PickUpWeapon(weapon, sim.Tune());
        }
        sim.Tap(eAction::Shot);
        sim.Step(40);

        const bool bSecondHit = Dummy_(sim, second).hitsTaken > 0;
        EXPECT_EQ(bSecondHit, weapon == eWeapon::Shotgun) << "weapon " << static_cast<int>(weapon);
    }
}

TEST(PlayerShot, AimDirectionsShootInThoseDirections)
{
    struct Case
    {
        bool  bHmg   = false;
        bool  bUp    = false;
        bool  bRight = false;
        float signX  = 0.f;
        float signY  = 0.f;
    };
    // 앞 / 위 / 위 + 방향(권총은 대각 없이 똑바로 위) / 위 + 방향(HMG 도 정지 조준하면 똑바로 위, 각도 90도)
    const ARRAY<Case, 4> cases = {
        {
         { false, false, true, 1.f, 0.f },
         { false, true, false, 0.f, -1.f },
         { false, true, true, 0.f, -1.f },
         { true, true, true, 0.f, -1.f },
         }
    };

    for (const Case& c: cases)
    {
        Sim sim;
        sim.Step(2);
        if (c.bHmg)
        {
            sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
        }
        sim.Set(eAction::Up, c.bUp);
        sim.Set(eAction::Right, c.bRight);
        sim.Step(kAimSettleTicks);
        sim.Hold(eAction::Shot);
        sim.Step();
        ASSERT_EQ(BulletCount_(sim), 1u) << "hmg " << c.bHmg << " up " << c.bUp << " right " << c.bRight;
        const Vec2 v = sim.State().GetProjectiles().GetBullets()[0].velocity;
        if (c.signX != 0.f)
        {
            EXPECT_EQ(v.x > 0.f, c.signX > 0.f);
        }
        EXPECT_EQ(std::abs(v.x) < 0.01f, c.signX == 0.f);
        EXPECT_EQ(v.y < 0.f, c.signY < 0.f);
        EXPECT_EQ(std::abs(v.y) < 0.01f, c.signY == 0.f);
    }
}

TEST(PlayerShot, HmgUpPlusDirectionSweepsBulletAngleFromZeroToNinety)
{
    Sim sim;
    UnlimitedBullets_(sim);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Hold(eAction::Up);
    sim.Tap(eAction::Shot);   // 한 번 눌러 나가는 버스트 동안 조준각이 쓸고 지나간다

    std::vector<float> angles;
    int                shots = 1;
    {
        const slug::Span<const Bullet> first = sim.State().GetProjectiles().GetBullets();
        const Vec2                     v     = first[first.size() - 1].velocity;
        angles.push_back(std::atan2(-v.y, std::abs(v.x)) * 180.f / 3.14159265f);
    }
    for (int tick = 0; tick < 40; ++tick)
    {
        sim.Step();
        const int fired = sim.Player().GetStats().shotsFired;
        if (fired > shots)
        {
            shots                                  = fired;
            const slug::Span<const Bullet> bullets = sim.State().GetProjectiles().GetBullets();
            const Vec2                     v       = bullets[bullets.size() - 1].velocity;
            angles.push_back(std::atan2(-v.y, std::abs(v.x)) * 180.f / 3.14159265f);
        }
    }

    ASSERT_GE(angles.size(), 4u);
    EXPECT_LT(angles.front(), 10.f);          // 처음엔 거의 정면
    EXPECT_NEAR(angles.back(), 90.f, 0.5f);   // 끝에는 똑바로 위 (대각에서 멈추지 않는다)
    size_t increases = 0;
    size_t diagonal  = 0;
    for (size_t i = 0; i < angles.size(); ++i)
    {
        EXPECT_GE(angles[i], -0.5f) << "shot " << i;
        EXPECT_LE(angles[i], 90.5f) << "shot " << i;
        diagonal += (angles[i] >= 22.5f && angles[i] <= 67.5f) ? 1 : 0;
        if (i > 0)
        {
            EXPECT_GE(angles[i], angles[i - 1]) << "shot " << i;
            if (angles[i - 1] < 89.f)   // 쓸고 가는 구간에서는 탄마다 각도가 엄격히 증가한다
            {
                EXPECT_GT(angles[i], angles[i - 1]) << "shot " << i;
            }
            increases += angles[i] > angles[i - 1] + 0.5f ? 1 : 0;
        }
    }
    EXPECT_GE(diagonal, 1u);    // 조준각이 대각 구간을 지나며 쓸고 간다
    EXPECT_GE(increases, 2u);   // 모든 탄이 같은 각도가 아니다
}

TEST(PlayerShot, HmgHoldingUpWhileFiringEndsStraightUpThenReturnsToForwardOnRelease)
{
    Sim sim;
    UnlimitedBullets_(sim);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    sim.Hold(eAction::Up);
    sim.Tap(eAction::Shot);
    sim.Step(kAimSettleTicks + 10);   // 버스트가 끝나고 조준각이 90도까지 쓸고 간 뒤
    EXPECT_EQ(sim.Player().GetAim(), eAim::Up);
    sim.Hold(eAction::Shot);   // 다시 눌러 새 버스트: 첫 탄이 똑바로 위로 나간다
    EXPECT_NEAR(StepUntilShotAngleDeg_(sim, 10), 90.f, 0.01f);
    sim.Release(eAction::Shot);

    sim.Release(eAction::Up);   // 놓으면 정면으로 되돌아온다
    sim.Step(60);
    EXPECT_EQ(sim.Player().GetAim(), eAim::Forward);
    sim.Hold(eAction::Shot);
    EXPECT_NEAR(StepUntilShotAngleDeg_(sim, 10), 0.f, 0.01f);
}

TEST(PlayerShot, AirAimDownShootsDownAndBulletDiesOnGround)
{
    Sim sim;
    sim.Step(2);
    sim.Tap(eAction::Jump);
    sim.Step(10);
    sim.Hold(eAction::Down);
    sim.Hold(eAction::Shot);
    sim.Step();
    ASSERT_EQ(BulletCount_(sim), 1u);
    EXPECT_GT(sim.State().GetProjectiles().GetBullets()[0].velocity.y, 0.f);
    EXPECT_NEAR(sim.State().GetProjectiles().GetBullets()[0].velocity.x, 0.f, 0.01f);

    sim.Release(eAction::Shot);
    sim.Step(10);   // 사거리(42틱)에 닿기 전에 지면에서 사라진다
    EXPECT_EQ(BulletCount_(sim), 0u);
}

TEST(PlayerShot, LeftFacingShootsLeftFromTheMuzzleOnTheLeft)
{
    Sim sim;
    sim.Hold(eAction::Left);
    sim.Step(5);
    sim.Release(eAction::Left);
    sim.Hold(eAction::Shot);
    sim.Step();
    ASSERT_EQ(BulletCount_(sim), 1u);
    const Bullet& bullet = sim.State().GetProjectiles().GetBullets()[0];
    EXPECT_LT(bullet.velocity.x, 0.f);
    EXPECT_LT(bullet.position.x, sim.X());
}

TEST(PlayerShot, BulletsExpireAtTheirRange)
{
    Sim sim;
    sim.Tap(eAction::Shot);
    sim.Step(25);   // 333 px < 사거리 420 px
    EXPECT_EQ(BulletCount_(sim), 1u);
    sim.Step(15);   // 420 px / 800 px/s = 31.5 틱 이후
    EXPECT_EQ(BulletCount_(sim), 0u);
}

TEST(PlayerShot, PistolDamagesGruntJeepAndBusThroughTheSamePath)
{
    Sim            sim;
    const uint32_t grunt = sim.State().AddTarget(eDummyKind::Grunt, { 220.f, 378.f });
    sim.Tap(eAction::Shot);
    sim.Step(30);
    EXPECT_FALSE(Dummy_(sim, grunt).IsAlive());

    for (const eDummyKind kind: { eDummyKind::Jeep, eDummyKind::Bus })
    {
        Sim            other;
        const uint32_t id = other.State().AddTarget(kind, { 260.f, 378.f });
        const int      hp = Dummy_(other, id).health.hp;
        other.Tap(eAction::Shot);
        other.Step(30);
        EXPECT_EQ(Dummy_(other, id).health.hp, hp - other.Tune().weapons.pistol.damage);
    }
}

TEST(PlayerShot, EveryShotIsOneHitIdThatIsReleasedAfterwards)
{
    Sim sim;
    sim.State().AddTarget(eDummyKind::Bus, { 400.f, 378.f });
    sim.Hold(eAction::Shot);
    sim.Step(120);
    sim.ReleaseAll();
    sim.Step(120);
    EXPECT_EQ(sim.State().GetCombat().GetResolver().GetTrackedHitCount(), 0u);   // HitId 누수 없음
}

TEST(PlayerKnife, AdjacentGruntDiesFromOneSlashAndNoBulletIsFired)
{
    Sim sim;
    sim.Step(2);
    const uint32_t id = sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });

    sim.Hold(eAction::Shot);
    sim.Step();
    sim.Release(eAction::Shot);
    EXPECT_EQ(sim.Player().GetOverlay(), eOverlay::Slash);

    bool bAnyBullet = false;
    for (int i = 0; i < 30; ++i)
    {
        sim.Step();
        bAnyBullet = bAnyBullet || BulletCount_(sim) > 0;
    }
    EXPECT_FALSE(Dummy_(sim, id).IsAlive());
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 0);
    EXPECT_FALSE(bAnyBullet);
}

TEST(PlayerKnife, GruntDiesAtTheFirstActiveTick)
{
    Sim sim;
    sim.Step(2);
    const uint32_t id = sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
    sim.Tap(eAction::Shot);
    const int first = sim.Tune().knife.activeFirstTick;
    sim.Step(first - 1);
    EXPECT_TRUE(Dummy_(sim, id).IsAlive());
    sim.Step();
    EXPECT_FALSE(Dummy_(sim, id).IsAlive());
}

TEST(PlayerKnife, VehicleDummiesAreNotKnifedAndTakeThePistolInstead)
{
    for (const eDummyKind kind: { eDummyKind::Jeep, eDummyKind::Bus })
    {
        Sim sim;
        sim.Step(2);
        const uint32_t id = sim.State().AddTarget(kind, { sim.X() + 60.f, 378.f });
        const int      hp = Dummy_(sim, id).health.hp;
        sim.Tap(eAction::Shot);
        sim.Step(25);
        EXPECT_EQ(Dummy_(sim, id).health.hp, hp - sim.Tune().weapons.pistol.damage) << GetDummyName(kind);
        EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 0) << GetDummyName(kind);
        EXPECT_EQ(sim.Player().GetStats().shotsFired, 1) << GetDummyName(kind);
    }
}

TEST(PlayerKnife, OneSwingHitsATargetOnlyOnceEvenThoughTheWindowIsLong)
{
    Sim sim;
    sim.Step(2);
    const uint32_t id = sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
    sim.Tap(eAction::Shot);

    int activeTicks = 0;
    for (int i = 0; i < 20; ++i)
    {
        sim.Step();
        activeTicks += HasMeleeHitbox_(sim) ? 1 : 0;
    }
    const KnifeTuning& knife = sim.Tune().knife;
    EXPECT_EQ(activeTicks, knife.activeLastTick - knife.activeFirstTick + 1);   // 판정은 활성 구간 동안만 켜진다
    EXPECT_EQ(Dummy_(sim, id).hitsTaken, 1);
}

TEST(PlayerKnife, TappedShotOnAVehicleKeepsFiringThePistol)
{
    Sim sim;
    sim.Step(2);
    const uint32_t id = sim.State().AddTarget(eDummyKind::Jeep, { sim.X() + 60.f, 378.f });
    int killed = -1;
    for (int tick = 0; tick < 200 && killed < 0; ++tick)
    {
        sim.Set(eAction::Shot, tick % 6 == 0);   // 권총은 연타해야 계속 나간다
        sim.Step();
        killed = Dummy_(sim, id).health.hp <= 0 ? tick : -1;
    }
    ASSERT_GE(killed, 0);
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 0);
    EXPECT_GE(sim.Player().GetStats().shotsFired, 7);   // 권총 3 x 7 = 21 >= 지프 체력 20
}

TEST(PlayerKnife, TriggerRangeBoundary)
{
    const float reachFromCenter = 10.f + Tuning {}.knife.range;   // 몸 반폭 + range
    {
        Sim sim;
        sim.Step(2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + reachFromCenter + 10.f - 1.f, 378.f });   // 표적 왼쪽 가장자리가 영역 안
        sim.Step();
        EXPECT_TRUE(sim.Player().IsKnifeTargetInRange());
    }
    {
        Sim sim;
        sim.Step(2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + reachFromCenter + 10.f + 2.f, 378.f });   // 영역 밖
        sim.Step();
        EXPECT_FALSE(sim.Player().IsKnifeTargetInRange());
    }
}

TEST(PlayerKnife, FarTargetGetsBulletsNotKnife)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Bus, { sim.X() + 200.f, 378.f });
    sim.Tap(eAction::Shot);
    sim.Step(5);
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 0);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
}

TEST(PlayerKnife, TargetBehindThePlayerDoesNotTriggerKnife)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Grunt, { sim.X() - 25.f, 378.f });
    sim.Tap(eAction::Shot);
    sim.Step(5);
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 0);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 1);
}

TEST(PlayerKnife, WorksWhileCrouchedRunningAndInTheAir)
{
    {
        Sim sim;
        sim.Step(2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
        sim.Hold(eAction::Down);
        sim.Step(2);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
    }
    {
        Sim sim;
        sim.State().AddTarget(eDummyKind::Grunt, { 200.f, 378.f });
        sim.Hold(eAction::Right);
        sim.Step(25);
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
    }
    {
        Sim sim;
        sim.Step(2);
        sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
        sim.Tap(eAction::Jump);
        sim.Step(3);
        ASSERT_FALSE(sim.Player().IsGrounded());
        sim.Tap(eAction::Shot);
        EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
    }
}

TEST(PlayerKnife, BombAndKnifeSameTickKnifeWinsAndNoBombIsLost)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
    sim.Hold(eAction::Shot);
    sim.Hold(eAction::Bomb);
    sim.Step();
    sim.ReleaseAll();
    sim.Step(30);
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
    EXPECT_EQ(sim.Player().GetWeapon().bombs, 10);   // 취소된 던지기는 폭탄을 소모하지 않는다
}

TEST(PlayerKnife, NeverGetsStuckUnderRandomInput)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Grunt, { 140.f, 378.f });   // 계속 근접 상태
    const slug::Vector<InputFrame> script = MakeNoisyScript_();

    for (const InputFrame& frame: script)
    {
        sim.StepWith(frame);
    }
    sim.ReleaseAll();
    sim.Step(60);

    EXPECT_FALSE(sim.Player().IsKnifeInProgress());
    EXPECT_EQ(sim.Player().GetOverlay(), eOverlay::None);
    EXPECT_GT(sim.Player().GetStats().knifeSlashes, 0);
    sim.Step(5);
    EXPECT_EQ(sim.State().GetCombat().GetResolver().GetTrackedHitCount(), 0u);
}

TEST(PlayerKnife, KnifeIsMeleeAndHitsEveryTeamItCanHit)
{
    const KnifeTuning knife = Tuning {}.knife;
    Hitbox            swing;
    swing.box     = MakeKnifeSwingBox({ 100.f, 378.f }, 1, knife);
    swing.team    = eTeam::Player;
    swing.kind    = eHitKind::Melee;
    swing.damage  = knife.damage;
    swing.bPierce = true;
    swing.owner   = kPlayerOwnerId;
    swing.id      = 1;

    Hurtbox enemy;
    enemy.box          = Aabb::FromBottomCenter({ 130.f, 378.f }, { 20.f, 56.f });
    enemy.team         = eTeam::Enemy;
    enemy.owner        = 2000;
    Hurtbox neutral    = enemy;
    neutral.team       = eTeam::Neutral;
    neutral.owner      = 2001;
    Hurtbox bulletOnly = enemy;
    bulletOnly.owner   = 2002;
    bulletOnly.accepts = HitKindMask { eHitKind::Bullet };

    HitResolver                  resolver;
    const slug::Vector<Hitbox>   hits   = { swing };
    const slug::Vector<Hurtbox>  hurts  = { enemy, neutral, bulletOnly };
    const slug::Vector<HitEvent> events = resolver.Resolve(hits, hurts);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].target, 2000u);
    EXPECT_EQ(events[1].target, 2001u);
}

TEST(PlayerKnife, KnifeRangeInvariantReachCoversTheTrigger)
{
    const Tuning t;
    EXPECT_GE(t.knife.reach, t.body.standSize.x * 0.5f + t.knife.range);   // 자동 발동한 표적은 항상 베기 범위 안
    EXPECT_GT(t.knife.activeLastTick, t.knife.activeFirstTick);
    EXPECT_LT(t.knife.activeLastTick, t.knife.durationTicks);
    EXPECT_LT(t.knife.chainWindowTicks, t.knife.durationTicks);
}

TEST(PlayerKnife, KillingWithKnifeAwardsScore)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, 378.f });
    sim.Tap(eAction::Shot);
    sim.Step(10);
    EXPECT_EQ(sim.Player().GetScore(), 100);
}

TEST(PlayerBomb, ThrowReleasesTheBombAtTheConfiguredTickAndSpendsOne)
{
    Sim sim;
    sim.Step(2);
    sim.Hold(eAction::Bomb);
    sim.Step();
    sim.Release(eAction::Bomb);
    EXPECT_EQ(sim.Player().GetOverlay(), eOverlay::Throw);
    EXPECT_EQ(sim.Player().GetWeapon().bombs, 10);

    const int release = sim.Tune().bomb.releaseTick;
    sim.Step(release);
    EXPECT_EQ(sim.Player().GetWeapon().bombs, 9);
    EXPECT_EQ(sim.Player().GetStats().bombsThrown, 1);
    EXPECT_EQ(sim.State().GetProjectiles().GetBombs().size(), 1u);
}

TEST(PlayerBomb, BombExplodesOnTheGroundAndBlastHitsEveryDummyKind)
{
    for (const eDummyKind kind: { eDummyKind::Grunt, eDummyKind::Jeep, eDummyKind::Bus })
    {
        Sim sim;
        sim.Step(2);
        const uint32_t id = sim.State().AddTarget(kind, { 270.f, 378.f });
        const int      hp = Dummy_(sim, id).health.hp;
        sim.Tap(eAction::Bomb);
        sim.Step(80);
        const TrainingDummy& dummy = Dummy_(sim, id);
        EXPECT_EQ(dummy.hitsTaken, 1) << GetDummyName(kind);   // 폭발이 여러 틱 유지돼도 표적당 한 번
        EXPECT_EQ(hp - dummy.health.hp, std::min(hp, sim.Tune().bomb.damage)) << GetDummyName(kind);
        EXPECT_EQ(dummy.lastKind, eHitKind::Blast);
    }
}

TEST(PlayerBomb, BlastHitsAllTargetsInRadiusOnce)
{
    Sim sim;
    sim.Step(2);
    // 수류탄은 새 튜닝(260/-330, 중력 900)에서 약 x 330 근처 지면에 떨어진다.
    // 지프(90x50)는 그 지점 바로 앞, 버스(170x90)는 공중 경로와 겹치지 않게 뒤에 둬서
    // 폭발 범위(110x130) 안에서 둘 다 한 번씩만 맞는지 본다.
    const uint32_t a = sim.State().AddTarget(eDummyKind::Jeep, { 320.f, 378.f });
    const uint32_t b = sim.State().AddTarget(eDummyKind::Bus, { 420.f, 378.f });
    sim.Tap(eAction::Bomb);
    sim.Step(80);
    EXPECT_GT(Dummy_(sim, a).hitsTaken, 0);
    EXPECT_GT(Dummy_(sim, b).hitsTaken, 0);
    EXPECT_LE(Dummy_(sim, a).hitsTaken, 1);
    EXPECT_LE(Dummy_(sim, b).hitsTaken, 1);
}

TEST(PlayerBomb, BombExplodesOnContactWithATargetInMidAir)
{
    Sim sim;
    sim.Step(2);
    const uint32_t id = sim.State().AddTarget(eDummyKind::Bus, { sim.X() + 130.f, 378.f });
    sim.Tap(eAction::Bomb);
    sim.Step(25);   // 지면에 닿기 한참 전 (착탄은 ~46 틱)
    EXPECT_EQ(Dummy_(sim, id).health.hp, Dummy_(sim, id).health.maxHp - sim.Tune().bomb.damage);
    EXPECT_EQ(sim.State().GetProjectiles().GetBombs().size(), 0u);
}

TEST(PlayerBomb, NoMoreThanMaxInFlight)
{
    Sim sim;
    sim.Tune().bomb.throwTicks = 4;   // 던지기 동작을 짧게 해서 비행 상한이 막는지 본다
    size_t peak                = 0;
    sim.Hold(eAction::Bomb);
    for (int i = 0; i < 120; ++i)
    {
        sim.Set(eAction::Bomb, i % 6 == 0);
        sim.Step();
        const size_t inFlight = sim.State().GetProjectiles().GetBombs().size();
        peak                  = std::max(peak, inFlight);
        ASSERT_LE(inFlight, static_cast<size_t>(sim.Tune().bomb.maxInFlight));
    }
    EXPECT_EQ(peak, static_cast<size_t>(sim.Tune().bomb.maxInFlight));
}

TEST(PlayerBomb, CannotThrowWithZeroBombs)
{
    Sim sim;
    sim.Player().AddBombs(-10);
    sim.Tap(eAction::Bomb);
    sim.Step(20);
    EXPECT_EQ(sim.Player().GetStats().bombsThrown, 0);
    EXPECT_EQ(sim.Player().GetWeapon().bombs, 0);
}

TEST(PlayerBomb, ThrowArcLandsAheadOfThePlayer)
{
    Sim sim;
    sim.Step(2);
    sim.Tap(eAction::Bomb);
    float maxX = 0.f;
    for (int i = 0; i < 60; ++i)
    {
        sim.Step();
        for (const Bomb& bomb: sim.State().GetProjectiles().GetBombs())
        {
            maxX = std::max(maxX, bomb.position.x);
        }
    }
    EXPECT_GT(maxX, sim.X() + 100.f);   // 약 160 px 앞까지 포물선
    EXPECT_LT(maxX, sim.X() + 260.f);
}

namespace
{
struct Harness
{
    Tuning           tuning      = {};
    GroundMap        ground      = MakeFlatGround();
    PlayerController player      = {};
    CombatSystem     combat      = {};
    ProjectileSystem projectiles = {};
    InputHistory     history     = {};
    Vec2             respawnFeet = { 200.f, 200.f };
    int              ticks       = 0;

    Harness()
    {
        player.Reset(tuning, { 100.f, 378.f });
        player.SnapToGround(World(), 16.f);
    }

    [[nodiscard]] PhysicsWorld World() const
    {
        PhysicsWorld world;
        world.pGround = &ground;
        return world;
    }

    bool Tick(
        const InputFrame            _input  = {},
        const slug::Vector<Hitbox>& _inject = {})
    {
        history.Push(_input);
        combat.BeginTick();

        PlayerContext ctx;
        ctx.pTuning      = &tuning;
        ctx.physics      = World();
        ctx.pCombat      = &combat;
        ctx.pProjectiles = &projectiles;
        ctx.respawnFeet  = respawnFeet;
        player.Tick(ctx, history);

        for (const Hitbox& hitbox: _inject)
        {
            combat.AddHitbox(hitbox);
        }
        projectiles.Step(&ground, combat, kStepSec);
        const slug::Vector<HitEvent>& events = combat.Resolve();
        const bool                    bDied  = player.OnHitEvents(events);
        projectiles.OnHitEvents(events, combat);
        ++ticks;
        return bDied;
    }

    void Run(
        const int _count)
    {
        for (int i = 0; i < _count; ++i)
        {
            (void)Tick();
        }
    }

    [[nodiscard]] Hitbox EnemyHit(
        const eHitKind _kind,
        const eTeam    _team = eTeam::Enemy) const
    {
        Hitbox hit;
        hit.box    = Aabb::FromBottomCenter(player.GetFeet(), { 40.f, 60.f });
        hit.team   = _team;
        hit.kind   = _kind;
        hit.damage = 1;
        hit.owner  = 5000;
        hit.id     = combat.GetHitboxes().size() + 100 + static_cast<uint32_t>(ticks);
        return hit;
    }
};
}   // namespace

TEST(PlayerDeath, AnyEnemyHitboxKindKillsThePlayer)
{
    for (const eHitKind kind: { eHitKind::Bullet, eHitKind::Melee, eHitKind::Blast, eHitKind::Hazard })
    {
        Harness h;
        h.Run(3);
        const bool bDied = h.Tick({}, { h.EnemyHit(kind) });
        EXPECT_TRUE(bDied) << static_cast<int>(kind);
        EXPECT_EQ(h.player.GetState(), ePlayerState::Dead);
        EXPECT_EQ(h.player.GetLives(), 2);
    }
}

TEST(PlayerDeath, NeutralHazardKillsButOwnTeamDoesNot)
{
    Harness h;
    h.Run(3);
    (void)h.Tick({}, { h.EnemyHit(eHitKind::Blast, eTeam::Player) });
    EXPECT_NE(h.player.GetState(), ePlayerState::Dead);
    (void)h.Tick({}, { h.EnemyHit(eHitKind::Hazard, eTeam::Neutral) });
    EXPECT_EQ(h.player.GetState(), ePlayerState::Dead);
}

TEST(PlayerDeath, TouchingAnEnemyBodyIsHarmless)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Bus, { sim.X(), 378.f });   // 같은 자리
    sim.Step(60);
    EXPECT_TRUE(sim.Player().IsAlive());
    EXPECT_EQ(sim.Player().GetLives(), 3);
}

TEST(PlayerDeath, EnemyShooterBulletKillsThePlayerInTheSandbox)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Grunt, { 380.f, 378.f }, true);   // 사수: 280 px 앞
    const int died = sim.StepUntilDead(600);
    ASSERT_GE(died, 0);
    EXPECT_EQ(sim.Player().GetLives(), 2);
    EXPECT_EQ(sim.Player().GetStats().deaths, 1);
}

TEST(PlayerDeath, JumpingOverTheBulletSurvives)
{
    Sim sim;
    sim.Step(2);
    sim.State().AddTarget(eDummyKind::Grunt, { 380.f, 378.f }, true);
    int guard = 0;
    while (sim.Player().IsAlive() && guard++ < 400)
    {
        bool bClose = false;
        for (const Bullet& bullet: sim.State().GetProjectiles().GetBullets())
        {
            if (bullet.team == eTeam::Enemy && bullet.position.x - sim.X() < 70.f && bullet.position.x > sim.X())
            {
                bClose = true;
            }
        }
        sim.Set(eAction::Jump, bClose);
        sim.Step();
        if (sim.State().GetTargets().GetKillCount() > 0)
        {
            break;
        }
    }
    EXPECT_TRUE(sim.Player().IsAlive());
    EXPECT_GT(sim.Player().GetStats().jumps, 0);
}

TEST(PlayerDeath, DeadPlayerIgnoresInput)
{
    Sim sim;
    sim.Step(2);
    sim.Player().Kill();
    const float x = sim.X();
    sim.Hold(eAction::Right);
    sim.Hold(eAction::Shot);
    sim.Hold(eAction::Jump);
    sim.Step(30);
    EXPECT_FLOAT_EQ(sim.X(), x);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 0);
    EXPECT_EQ(sim.Player().GetStats().jumps, 0);
    EXPECT_EQ(sim.Player().GetState(), ePlayerState::Dead);
}

TEST(PlayerRespawn, RespawnsAfterConfiguredDelayWithInvincibilityAndPistol)
{
    Harness h;
    h.Run(3);
    h.player.PickUpWeapon(eWeapon::Hmg, h.tuning);
    h.player.AddBombs(-4);
    (void)h.Tick({}, { h.EnemyHit(eHitKind::Bullet) });
    ASSERT_EQ(h.player.GetState(), ePlayerState::Dead);

    h.Run(h.tuning.player.deathTicks - 2);
    EXPECT_EQ(h.player.GetState(), ePlayerState::Dead);
    h.Run(3);
    EXPECT_EQ(h.player.GetState(), ePlayerState::Respawn);
    EXPECT_NEAR(h.player.GetFeet().x, h.respawnFeet.x, 0.01f);
    EXPECT_EQ(h.player.GetWeapon().weapon, eWeapon::Pistol);
    EXPECT_EQ(h.player.GetWeapon().bombs, 10);
    EXPECT_TRUE(h.player.IsInvincible());
    EXPECT_GT(h.player.GetInvincibleTicks(), h.tuning.player.respawnInvincible - 5);
    EXPECT_TRUE(h.player.MakeHurtbox(h.tuning).bInvulnerable);
}

TEST(PlayerRespawn, DropsFromAboveLandsAndKeepsInvincibilityUntilItExpires)
{
    Harness h;
    h.respawnFeet = { 300.f, -40.f };
    h.Run(2);
    h.player.Kill();
    h.Run(h.tuning.player.deathTicks + 1);
    ASSERT_EQ(h.player.GetState(), ePlayerState::Respawn);
    EXPECT_LT(h.player.GetFeet().y, 0.f);   // 화면 위에서 시작

    h.Run(90);
    EXPECT_TRUE(h.player.IsGrounded());
    EXPECT_EQ(h.player.GetState(), ePlayerState::Idle);
    EXPECT_TRUE(h.player.IsInvincible());

    h.Run(h.tuning.player.respawnInvincible);
    EXPECT_FALSE(h.player.IsInvincible());
}

TEST(PlayerRespawn, InvincibleRespawnIgnoresEnemyHitboxesUntilItEnds)
{
    Harness h;
    h.Run(2);
    h.player.Kill();
    h.Run(h.tuning.player.deathTicks + 40);   // 리스폰 후 착지까지
    ASSERT_TRUE(h.player.IsInvincible());

    for (int i = 0; i < 20; ++i)
    {
        (void)h.Tick({}, { h.EnemyHit(eHitKind::Bullet) });
    }
    EXPECT_NE(h.player.GetState(), ePlayerState::Dead);
    EXPECT_EQ(h.player.GetLives(), 2);

    h.Run(h.tuning.player.respawnInvincible);
    ASSERT_FALSE(h.player.IsInvincible());
    (void)h.Tick({}, { h.EnemyHit(eHitKind::Bullet) });
    EXPECT_EQ(h.player.GetState(), ePlayerState::Dead);
    EXPECT_EQ(h.player.GetLives(), 1);
}

TEST(PlayerRespawn, BlinkAlternatesVisibilityWhileInvincible)
{
    Harness h;
    h.Run(2);
    h.player.Kill();
    h.Run(h.tuning.player.deathTicks + 5);
    ASSERT_TRUE(h.player.IsInvincible());

    bool bSawVisible = false;
    bool bSawHidden  = false;
    for (int i = 0; i < 16; ++i)
    {
        h.Run(1);
        (h.player.MakeRigState().bVisible ? bSawVisible : bSawHidden) = true;
    }
    EXPECT_TRUE(bSawVisible);
    EXPECT_TRUE(bSawHidden);

    h.Run(h.tuning.player.respawnInvincible);
    EXPECT_TRUE(h.player.MakeRigState().bVisible);
}

TEST(PlayerRespawn, ControlWorksWhileDroppingIn)
{
    Harness h;
    h.respawnFeet = { 300.f, -40.f };
    h.Run(2);
    h.player.Kill();
    h.Run(h.tuning.player.deathTicks + 2);
    ASSERT_EQ(h.player.GetState(), ePlayerState::Respawn);

    InputFrame right;
    right.Set(eAction::Right, true);
    const float x = h.player.GetFeet().x;
    (void)h.Tick(right);
    EXPECT_GT(h.player.GetFeet().x, x);
}

TEST(PlayerGameOver, LastLifeLostEndsTheGameWithoutRespawn)
{
    Harness h;
    for (int life = 0; life < 3; ++life)
    {
        h.Run(2);
        ASSERT_TRUE(h.player.IsAlive()) << life;
        h.player.Kill();
        if (life < 2)
        {
            h.Run(h.tuning.player.deathTicks + 200);   // 리스폰 + 무적 종료까지
        }
    }
    EXPECT_TRUE(h.player.IsGameOver());
    EXPECT_EQ(h.player.GetLives(), 0);

    h.Run(h.tuning.player.deathTicks + 200);
    EXPECT_EQ(h.player.GetState(), ePlayerState::Dead);
    EXPECT_GT(h.player.GetGameOverTicks(), h.tuning.player.deathTicks);
}

TEST(PlayerGameOver, SandboxRestartsAfterTheGameOverTimer)
{
    Sim sim;
    sim.Step(2);
    for (int life = 0; life < 3; ++life)
    {
        sim.Player().Kill();
        sim.Step(sim.Tune().player.deathTicks + 200);
        if (sim.Player().IsGameOver())
        {
            break;
        }
    }
    EXPECT_EQ(sim.Player().GetLives(), 3);
    EXPECT_FALSE(sim.Player().IsGameOver());
    EXPECT_TRUE(sim.Player().IsAlive());
}

TEST(PlayerVictory, VictoryStateIsInvulnerableAndIgnoresInput)
{
    Harness h;
    h.Run(3);
    h.player.SetVictory();
    const float x = h.player.GetFeet().x;
    InputFrame  input;
    input.Set(eAction::Right, true);
    input.Set(eAction::Shot, true);
    for (int i = 0; i < 10; ++i)
    {
        (void)h.Tick(input, { h.EnemyHit(eHitKind::Bullet) });
    }
    EXPECT_EQ(h.player.GetState(), ePlayerState::Victory);
    EXPECT_FLOAT_EQ(h.player.GetFeet().x, x);
    EXPECT_EQ(h.player.GetStats().shotsFired, 0);
    EXPECT_EQ(h.player.GetLives(), 3);
}

TEST(CombatSystem, HasTargetInFollowsTheResolverRules)
{
    CombatSystem combat;
    combat.BeginTick();

    Hurtbox enemy;
    enemy.box   = Aabb::FromMinMax({ 100.f, 0.f }, { 120.f, 40.f });
    enemy.team  = eTeam::Enemy;
    enemy.owner = 10;
    combat.AddHurtbox(enemy);

    const Aabb inside  = Aabb::FromMinMax({ 110.f, 10.f }, { 130.f, 30.f });
    const Aabb outside = Aabb::FromMinMax({ 130.f, 10.f }, { 150.f, 30.f });
    EXPECT_TRUE(combat.HasTargetIn(eTeam::Player, eHitKind::Melee, inside));
    EXPECT_FALSE(combat.HasTargetIn(eTeam::Player, eHitKind::Melee, outside));
    EXPECT_FALSE(combat.HasTargetIn(eTeam::Enemy, eHitKind::Melee, inside));
    EXPECT_FALSE(combat.HasTargetIn(eTeam::Player, eHitKind::Melee, inside, 10));   // 자기 자신 제외

    combat.BeginTick();
    enemy.bInvulnerable = true;
    combat.AddHurtbox(enemy);
    EXPECT_FALSE(combat.HasTargetIn(eTeam::Player, eHitKind::Melee, inside));

    combat.BeginTick();
    enemy.bInvulnerable = false;
    enemy.accepts       = HitKindMask { eHitKind::Bullet };
    combat.AddHurtbox(enemy);
    EXPECT_FALSE(combat.HasTargetIn(eTeam::Player, eHitKind::Melee, inside));
    EXPECT_TRUE(combat.HasTargetIn(eTeam::Player, eHitKind::Bullet, inside));
}

TEST(Projectiles, FastBulletDoesNotTunnelThroughAThinTarget)
{
    CombatSystem     combat;
    ProjectileSystem projectiles;

    BulletSpawn spawn;
    spawn.owner     = 1;
    spawn.position  = { 0.f, 50.f };
    spawn.sweepFrom = { 0.f, 50.f };
    spawn.direction = { 1.f, 0.f };
    spawn.speed     = 3000.f;   // 틱당 50 px
    spawn.size      = { 4.f, 4.f };
    spawn.range     = 1000.f;
    spawn.damage    = 1;

    Hurtbox thin;
    thin.box   = Aabb::FromMinMax({ 104.f, 40.f }, { 106.f, 60.f });
    thin.team  = eTeam::Enemy;
    thin.owner = 99;

    projectiles.SpawnBullet(spawn, combat);
    int hits = 0;
    for (int i = 0; i < 10; ++i)
    {
        combat.BeginTick();
        combat.AddHurtbox(thin);
        projectiles.Step(nullptr, combat, kStepSec);
        const slug::Vector<HitEvent>& events = combat.Resolve();
        hits += static_cast<int>(events.size());
        projectiles.OnHitEvents(events, combat);
    }
    EXPECT_EQ(hits, 1);
    EXPECT_TRUE(projectiles.GetBullets().empty());
}

TEST(Projectiles, FirstTickSweepCoversTheSpanFromTheBodyToTheMuzzle)
{
    CombatSystem     combat;
    ProjectileSystem projectiles;

    BulletSpawn spawn;
    spawn.owner     = 1;
    spawn.position  = { 60.f, 50.f };
    spawn.sweepFrom = { 0.f, 50.f };
    spawn.speed     = 600.f;
    spawn.size      = { 6.f, 6.f };
    projectiles.SpawnBullet(spawn, combat);

    Hurtbox between;
    between.box   = Aabb::FromMinMax({ 20.f, 40.f }, { 30.f, 60.f });
    between.team  = eTeam::Enemy;
    between.owner = 77;

    combat.BeginTick();
    combat.AddHurtbox(between);
    projectiles.Step(nullptr, combat, kStepSec);
    EXPECT_EQ(combat.Resolve().size(), 1u);
}

TEST(Projectiles, PierceHitsEachTargetOnceAndKeepsFlying)
{
    CombatSystem     combat;
    ProjectileSystem projectiles;

    BulletSpawn spawn;
    spawn.owner     = 1;
    spawn.position  = { 0.f, 50.f };
    spawn.sweepFrom = { 0.f, 50.f };
    spawn.speed     = 600.f;
    spawn.size      = { 10.f, 10.f };
    spawn.range     = 500.f;
    spawn.bPierce   = true;
    projectiles.SpawnBullet(spawn, combat);

    int hitsA = 0;
    int hitsB = 0;
    for (int i = 0; i < 60; ++i)
    {
        combat.BeginTick();
        for (uint32_t id = 10; id <= 11; ++id)
        {
            Hurtbox hurt;
            hurt.box   = Aabb::FromMinMax({ 100.f + 100.f * static_cast<float>(id - 10), 30.f }, { 140.f + 100.f * static_cast<float>(id - 10), 70.f });
            hurt.team  = eTeam::Enemy;
            hurt.owner = id;
            combat.AddHurtbox(hurt);
        }
        projectiles.Step(nullptr, combat, kStepSec);
        const slug::Vector<HitEvent>& events = combat.Resolve();
        for (const HitEvent& event: events)
        {
            (event.target == 10 ? hitsA : hitsB) += 1;
        }
        projectiles.OnHitEvents(events, combat);
    }
    EXPECT_EQ(hitsA, 1);
    EXPECT_EQ(hitsB, 1);
}

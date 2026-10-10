#include "Core/AssetPath.h"
#include <gtest/gtest.h>
#include "Base/Types.h"
#include "Base/Result.h"

#include "Game/Enemies/BossShiee.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "BossTestUtil.h"
#include "PlayerTestUtil.h"

#include <cmath>

using namespace slug;
using namespace slugtest;

// Boss Shiee (Docs/BossLayout.md, Docs/Spec_Boss.md). 시간 틱 = 1/60 s.
// 등장 240 -> 포탑 사격 (첫 발 전투 120 틱, 예고 Turning 45 + Shooting 발사 30) -> 포탑 4기 파괴 또는 1800 틱 -> 캐논 전개 160 -> 볼리.

namespace
{
    constexpr float kFeetX        = 1000.f;
    constexpr float kFeetY        = 1286.f;
    constexpr float kCamLeft      = 12700.f;
    constexpr int   kEntranceTick = 240;
    constexpr int   kFirstTurret  = 120;
    constexpr int   kTurnTicks    = 45;
    constexpr int   kFireTicks    = 30;
    constexpr int   kDeployTicks  = 160;
    constexpr int   kCannonHp     = 1000;
    constexpr int   kTurretHp     = 120;
    constexpr int   kTurretScore  = 500;

    EnemyHandle SpawnBoss(BossRig& _rig, const bool _bArena = true)
    {
        SpawnParams params;
        params.facing = 1;
        if (_bArena)
        {
            params.arena = Aabb::FromMinMax({ kCamLeft, 864.f }, { 14094.f, 1312.f });
        }
        _rig.SetCameraView(Aabb::FromMinSize({ kCamLeft, 864.f }, { 640.f, 448.f }));
        return _rig.SpawnAt("boss_shiee", { _bArena ? kCamLeft + 300.f : kFeetX, kFeetY }, params);
    }

    const Enemy* Boss(BossRig& _rig, const EnemyHandle _h)
    {
        return _rig.Find(_h);
    }

    // 전투 시작 (등장 종료) 까지 진행
    void ToFight(BossRig& _rig, const EnemyHandle _h)
    {
        _rig.Run(kEntranceTick + 2);
        ASSERT_EQ(Boss(_rig, _h)->state, bossshiee::kAssault);
    }

    void HitPart(BossRig& _rig, const EnemyHandle _h, const int _part, const int _damage)
    {
        _rig.Hit(bossshiee::TurretOwner(_h.id, _part), _damage);
        _rig.Frame();
    }

    void HitHull(BossRig& _rig, const EnemyHandle _h, const int _damage)
    {
        _rig.Hit(_h.id, _damage);
        _rig.Frame();
    }

    // 포탑 4기를 파괴해 캐논 전개 -> 캐논 (노출) 상태까지 진행
    void ToCannon(BossRig& _rig, const EnemyHandle _h)
    {
        ToFight(_rig, _h);
        for (int i = 0; i < 4; ++i)
        {
            HitPart(_rig, _h, i, kTurretHp);
        }
        ASSERT_EQ(Boss(_rig, _h)->state, bossshiee::kDeploy);
        _rig.Run(kDeployTicks + 2);
        ASSERT_EQ(Boss(_rig, _h)->state, bossshiee::kCannon);
    }

    int CountShotsOf(BossRig& _rig, const slug::StringView _proj)
    {
        const int projIndex = _rig.GetLib().FindProjectileIndex(_proj);
        if (projIndex < 0)
        {
            return -1;
        }
        int n = 0;
        for (const Shot& shot: _rig.Projectiles().GetShots())
        {
            if (shot.bAlive && shot.params.visual == static_cast<uint16_t>(projIndex + 1))
            {
                ++n;
            }
        }
        return n;
    }

    int CountDiedOf(BossRig& _rig, const EnemyHandle _h)
    {
        return _rig.CountEventsOf(eEnemyEventKind::Died, _h);
    }

    int CountPartScores(BossRig& _rig)
    {
        return _rig.CountEvents(eEnemyEventKind::PartDestroyed);
    }
}

TEST(BossShiee, EntranceComesInFromTheLeftOfTheCameraAndSettles)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    rig.Frame();
    const Enemy* e = Boss(rig, h);
    ASSERT_NE(e, nullptr);
    EXPECT_EQ(e->state, bossshiee::kEntrance);
    EXPECT_LT(e->body.position.x, kCamLeft + 0.f) << "등장은 카메라 왼쪽 밖에서 시작한다";
    EXPECT_FALSE(bossshiee::IsFightStarted(*e));
    EXPECT_FALSE(rig.World().GetBossHpRatio().has_value()) << "등장 중에는 체력 바가 없다";

    rig.Run(kEntranceTick / 2);
    const float mid = Boss(rig, h)->body.position.x;
    rig.Run(kEntranceTick / 2 + 2);
    e = Boss(rig, h);
    EXPECT_EQ(e->state, bossshiee::kAssault);
    EXPECT_GT(mid, kCamLeft + 0.f - 300.f) << "중간 지점은 화면 쪽으로 들어왔다";
    EXPECT_NEAR(e->body.position.x, kCamLeft + 330.f, 2.f) << "정지 위치 = 카메라 왼쪽 + stationX";
    EXPECT_TRUE(bossshiee::IsFightStarted(*e));
    ASSERT_TRUE(rig.World().GetBossHpRatio().has_value());
    EXPECT_NEAR(*rig.World().GetBossHpRatio(), 1.f, 1e-4f);
}

TEST(BossShiee, TurretsTelegraphBeforeTheyFire)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    ToFight(rig, h);

    // 첫 포탑 사격 전 (전투 시작 후 kFirstTurret - 10 틱) 까지는 사격도 예고도 없다
    rig.Run(kFirstTurret - 12);
    EXPECT_EQ(CountShotsOf(rig, "boss_tracer"), 0);
    for (int i = 0; i < kBossTurretCount; ++i)
    {
        EXPECT_EQ(bossshiee::GetTurretPhase(*Boss(rig, h), i), bossshiee::kTurretIdle);
    }

    // 예고 시작 (Turning) 이 보이고, 발사는 예고 합계 (Turning 45 + 발사 30) 뒤
    bool bTurning = false;
    for (int t = 0; t < 40 && !bTurning; ++t)
    {
        rig.Frame();
        for (int i = 0; i < kBossTurretCount; ++i)
        {
            bTurning = bTurning || bossshiee::GetTurretPhase(*Boss(rig, h), i) == bossshiee::kTurretTurning;
        }
    }
    EXPECT_TRUE(bTurning);
    EXPECT_EQ(CountShotsOf(rig, "boss_tracer"), 0) << "예고 중에는 아직 쏘지 않는다";

    rig.Run(kTurnTicks + kFireTicks + 4);
    EXPECT_GE(CountShotsOf(rig, "boss_tracer"), 1);
}

TEST(BossShiee, TracerTravelsAtTheSpecSpeedAndAimsAtThePlayer)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    rig.SetPlayer({ kCamLeft + 500.f, kFeetY });
    ToFight(rig, h);
    const int projIndex = rig.GetLib().FindProjectileIndex("boss_tracer");
    ASSERT_GE(projIndex, 0);

    for (int t = 0; t < 400 && CountShotsOf(rig, "boss_tracer") == 0; ++t)
    {
        rig.Frame();
    }
    const Shot* pShot = nullptr;
    for (const Shot& shot: rig.Projectiles().GetShots())
    {
        if (shot.bAlive && shot.params.visual == static_cast<uint16_t>(projIndex + 1))
        {
            pShot = &shot;
            break;
        }
    }
    ASSERT_NE(pShot, nullptr);
    const float speed = std::sqrt(pShot->velocity.x * pShot->velocity.x + pShot->velocity.y * pShot->velocity.y);
    EXPECT_NEAR(speed, 200.f, 1.f) << "트레이서 200 px/s (<= 260)";
    EXPECT_LE(speed, 260.f);
    EXPECT_GT(pShot->velocity.x, 0.f) << "플레이어 (보스 오른쪽) 쪽으로 쏜다";
}

TEST(BossShiee, DestroyingOneTurretScoresOnceAndLeavesTheHullAndOthersIntact)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    ToFight(rig, h);
    const int hullBefore = Boss(rig, h)->health.hp;

    HitPart(rig, h, 0, kTurretHp - 1);
    EXPECT_EQ(bossshiee::GetTurretHp(*Boss(rig, h), 0), 1);
    EXPECT_EQ(CountPartScores(rig), 0) << "피해만으로는 점수가 없다";

    HitPart(rig, h, 0, 5);
    EXPECT_EQ(bossshiee::GetTurretHp(*Boss(rig, h), 0), 0);
    EXPECT_EQ(bossshiee::GetTurretsDestroyed(*Boss(rig, h)), 1);
    EXPECT_EQ(CountPartScores(rig), 1);
    int score = 0;
    for (const EnemyEvent& ev: rig.Events())
    {
        if (ev.kind == eEnemyEventKind::PartDestroyed)
        {
            score = ev.score;
        }
    }
    EXPECT_EQ(score, kTurretScore);
    EXPECT_EQ(CountDiedOf(rig, h), 0) << "포탑 파괴는 보스 사망이 아니다";

    // 선체는 피해를 받지 않는다 (탄은 흡수만)
    HitHull(rig, h, 9999);
    EXPECT_EQ(Boss(rig, h)->health.hp, hullBefore);
    EXPECT_TRUE(Boss(rig, h)->IsAlive());
    EXPECT_EQ(CountDiedOf(rig, h), 0);
    // 파괴된 포탑은 다시 맞아도 점수가 없다
    HitPart(rig, h, 0, 100);
    EXPECT_EQ(CountPartScores(rig), 1);
}

TEST(BossShiee, HullAbsorbsBulletsWithoutHealthOrDeath)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    ToFight(rig, h);
    for (int i = 0; i < 20; ++i)
    {
        HitHull(rig, h, 50);
    }
    EXPECT_EQ(Boss(rig, h)->health.hp, kCannonHp);
    EXPECT_TRUE(Boss(rig, h)->IsAlive());
    EXPECT_NEAR(*rig.World().GetBossHpRatio(), 1.f, 1e-4f);
}

TEST(BossShiee, FourTurretsDownOpenTheDeployAndTheCannonVolleysStart)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    ToFight(rig, h);

    for (int i = 0; i < 3; ++i)
    {
        HitPart(rig, h, i, kTurretHp);
    }
    EXPECT_EQ(Boss(rig, h)->state, bossshiee::kAssault) << "3 기만 부서졌다";
    HitPart(rig, h, 3, kTurretHp);
    EXPECT_EQ(Boss(rig, h)->state, bossshiee::kDeploy);
    EXPECT_EQ(bossshiee::GetTurretsDestroyed(*Boss(rig, h)), 4);

    rig.Run(kDeployTicks + 2);
    EXPECT_EQ(Boss(rig, h)->state, bossshiee::kCannon);
    EXPECT_NEAR(*rig.World().GetBossHpRatio(), (3 * kTurretHp + kCannonHp) / static_cast<float>(7 * kTurretHp + kCannonHp), 0.01f);

    // 캐논 전개 후 첫 볼리: 트레이서 3발 (0.25 s 간격, 예고 0.75 s 뒤)
    rig.Run(120);
    EXPECT_GE(bossshiee::GetVolleyCount(*Boss(rig, h)), 1);
    EXPECT_GE(CountShotsOf(rig, "boss_tracer"), 2);
}

TEST(BossShiee, EverySecondVolleyLobsAnOrangeShellThatLandsOnTheStandingPlayer)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    rig.SetPlayer({ kCamLeft + 400.f, kFeetY });
    ToCannon(rig, h);
    const int projIndex = rig.GetLib().FindProjectileIndex("boss_shell");
    ASSERT_GE(projIndex, 0);

    int shellsSeen = 0;
    for (int t = 0; t < 60 * 20; ++t)
    {
        rig.Frame();
        const int n = CountShotsOf(rig, "boss_shell");
        shellsSeen = std::max(shellsSeen, n);
    }
    EXPECT_GE(shellsSeen, 1) << "두 볼리마다 주황 포탄 1발 (20 s 안에 있어야 한다)";
    EXPECT_GE(bossshiee::GetVolleyCount(*Boss(rig, h)), 4);
}

TEST(BossShiee, CannonHitsFlashAndTheCannonDeathStartsTheFinale)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    ToCannon(rig, h);
    const uint32_t cannonId = bossshiee::CannonOwner(h.id);

    rig.Hit(cannonId, 100);
    rig.Frame();
    EXPECT_EQ(bossshiee::GetCannonHp(*Boss(rig, h)), kCannonHp - 100);
    EXPECT_GT(Boss(rig, h)->partFlashTicks[bossshiee::kCannonPart], 0) << "캐논 피격 시 부위 플래시";
    EXPECT_EQ(CountPartScores(rig), 4) << "포탑 4기 점수";

    rig.Hit(cannonId, kCannonHp);   // 나머지 전부
    rig.Frame();                    // 체력 0 -> 다음 틱에 사망 처리
    rig.Frame();
    const Enemy* e = Boss(rig, h);
    EXPECT_EQ(e->life, eEnemyLife::Dying);
    EXPECT_TRUE(bossshiee::IsCannonDestroyed(*e));
    EXPECT_EQ(CountDiedOf(rig, h), 1);
    int bossScore = 0;
    for (const EnemyEvent& ev: rig.Events())
    {
        if (ev.kind == eEnemyEventKind::Died && ev.handle == h)
        {
            bossScore = ev.score;
        }
    }
    EXPECT_EQ(bossScore, 10000);

    // 사망 연출 10 s 뒤 IsDestroyFinished (게이트 boss_dead 가 여기서 열린다)
    rig.Run(600 - 40);
    EXPECT_FALSE(rig.World().IsDestroyFinished(h)) << "9.7 s 에는 아직 끝나지 않았다";
    rig.Run(60);
    EXPECT_TRUE(rig.World().IsDestroyFinished(h));
    EXPECT_EQ(CountDiedOf(rig, h), 1) << "사망 이벤트는 한 번";
    EXPECT_FALSE(rig.World().GetBossHpRatio().has_value()) << "사망 후 체력 바는 숨긴다";
}

TEST(BossShiee, NoAttacksOnceThePlayerIsDead)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    rig.SetPlayer({ kCamLeft + 300.f, kFeetY }, false);
    ToFight(rig, h);
    rig.Run(60 * 15);
    EXPECT_EQ(CountShotsOf(rig, "boss_tracer"), 0);
    EXPECT_EQ(CountShotsOf(rig, "boss_shell"), 0);
}

TEST(BossShiee, ScrollsOnlyInBurstsAndNeverPastItsBudget)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    rig.SetPlayer({ kCamLeft + 100.f, kFeetY });

    float total = 0.f;
    bool  bScrolledDuringEntrance = false;
    // 등장 중에는 카메라를 밀지 않는다 (스크롤 요청은 전투 중에만)
    for (int t = 0; t < kEntranceTick; ++t)
    {
        rig.Frame();
        bScrolledDuringEntrance = bScrolledDuringEntrance || rig.World().TakeCameraScroll() != 0.f;
    }
    EXPECT_FALSE(bScrolledDuringEntrance);

    int onTicks = 0;
    for (int t = 0; t < 60 * 30; ++t)
    {
        rig.Frame();
        const float dx = rig.World().TakeCameraScroll();
        total += dx;
        onTicks += dx > 0.f ? 1 : 0;
    }
    EXPECT_NEAR(total, 18.f * 18.f, 30.f) << "30 s 중 버스트 3 s / 휴지 2 s 6 주기 = 18 s 전진 (18 px/s)";
    EXPECT_GT(onTicks, 0);
    EXPECT_LT(onTicks, 60 * 30) << "휴지가 있다";
    EXPECT_LE(total, 700.f + 0.5f);
    EXPECT_NEAR(static_cast<float>(bossshiee::GetScrollLeftPx(*Boss(rig, h))), 700.f - total, 2.f) << "남은 예산 = budget - 사용분";
}

TEST(BossShiee, HullMovesWithTheCameraScroll)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    rig.SetPlayer({ kCamLeft + 100.f, kFeetY });
    ToFight(rig, h);
    (void)rig.World().TakeCameraScroll();   // 등장/전투 시작 때 쌓인 요청은 버린다
    const float x0 = Boss(rig, h)->body.position.x;
    float scrolled = 0.f;
    for (int t = 0; t < 600; ++t)
    {
        rig.Frame();
        scrolled += rig.World().TakeCameraScroll();
    }
    EXPECT_GT(scrolled, 0.f);
    EXPECT_NEAR(Boss(rig, h)->body.position.x - x0, scrolled, 0.5f) << "선체는 요청한 카메라 전진량만큼 같이 간다 (화면상 정지)";
}

TEST(BossShiee, BossBoundsAndPartsStayInsideTheHurtboxTableWhileTheCannonIsOpen)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle h = SpawnBoss(rig);
    ToCannon(rig, h);
    // 캐논 열림 상태에서 캐논 상자가 등록되어 있어야 맞는다 (아니면 피해 없음)
    rig.Hit(bossshiee::CannonOwner(h.id), 10);
    rig.Frame();
    EXPECT_EQ(bossshiee::GetCannonHp(*Boss(rig, h)), kCannonHp - 10);
}

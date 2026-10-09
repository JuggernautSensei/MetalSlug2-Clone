#include <gtest/gtest.h>
#include "Base/Types.h"

#include "World/Combat.h"

using namespace slug;

namespace
{
Hitbox MakeHit(
    const eTeam    _team,
    const eHitKind _kind,
    const Aabb&    _box,
    const uint32_t _id      = 1,
    const bool     _bPierce = false,
    const int      _damage  = 2,
    const uint32_t _owner   = 100)
{
    Hitbox hit;
    hit.box     = _box;
    hit.team    = _team;
    hit.kind    = _kind;
    hit.damage  = _damage;
    hit.bPierce = _bPierce;
    hit.owner   = _owner;
    hit.id      = _id;
    return hit;
}

Hurtbox MakeHurt(
    const eTeam    _team,
    const Aabb&    _box,
    const uint32_t _owner)
{
    Hurtbox hurt;
    hurt.box   = _box;
    hurt.team  = _team;
    hurt.owner = _owner;
    return hurt;
}

Aabb Box(
    const float _x0,
    const float _y0,
    const float _x1,
    const float _y1)
{
    return Aabb::FromMinMax({ _x0, _y0 }, { _x1, _y1 });
}

slug::Vector<HitEvent> Resolve(
    HitResolver&                _resolver,
    const slug::Vector<Hitbox>&  _hits,
    const slug::Vector<Hurtbox>& _hurts)
{
    return _resolver.Resolve(_hits, _hurts);
}
}   // namespace

// ===========================================
//  팀 규칙
// ===========================================

TEST(Combat, TeamMatrix)
{
    EXPECT_TRUE(CanTeamHit(eTeam::Player, eTeam::Enemy));
    EXPECT_TRUE(CanTeamHit(eTeam::Player, eTeam::Neutral));
    EXPECT_FALSE(CanTeamHit(eTeam::Player, eTeam::Player));

    EXPECT_TRUE(CanTeamHit(eTeam::Enemy, eTeam::Player));
    EXPECT_FALSE(CanTeamHit(eTeam::Enemy, eTeam::Enemy));
    EXPECT_FALSE(CanTeamHit(eTeam::Enemy, eTeam::Neutral));

    EXPECT_TRUE(CanTeamHit(eTeam::Neutral, eTeam::Player));
    EXPECT_TRUE(CanTeamHit(eTeam::Neutral, eTeam::Enemy));
    EXPECT_FALSE(CanTeamHit(eTeam::Neutral, eTeam::Neutral));
}

TEST(Combat, PlayerBulletHitsEnemyAndReportsDamage)
{
    HitResolver resolver;
    const auto  events = Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 1, false, 3) }, { MakeHurt(eTeam::Enemy, Box(5, 5, 20, 20), 7) });

    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].hitboxIndex, 0u);
    EXPECT_EQ(events[0].hurtboxIndex, 0u);
    EXPECT_EQ(events[0].damage, 3);
    EXPECT_EQ(events[0].kind, eHitKind::Bullet);
    EXPECT_EQ(events[0].attacker, 100u);
    EXPECT_EQ(events[0].target, 7u);
}

TEST(Combat, SameTeamIsIgnored)
{
    HitResolver resolver;
    EXPECT_TRUE(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 1) }, { MakeHurt(eTeam::Player, Box(0, 0, 10, 10), 7) }).empty());
    EXPECT_TRUE(Resolve(resolver, { MakeHit(eTeam::Enemy, eHitKind::Bullet, Box(0, 0, 10, 10), 2) }, { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 7) }).empty());
}

TEST(Combat, EnemyBulletHitsPlayer)
{
    HitResolver resolver;
    const auto  events = Resolve(resolver, { MakeHit(eTeam::Enemy, eHitKind::Bullet, Box(0, 0, 10, 10), 1) }, { MakeHurt(eTeam::Player, Box(5, 0, 15, 10), 1) });
    EXPECT_EQ(events.size(), 1u);
}

TEST(Combat, NeutralPropsAcceptPlayerHitsButNotEnemyFire)
{
    HitResolver                resolver;
    const slug::Vector<Hurtbox> prop = { MakeHurt(eTeam::Neutral, Box(0, 0, 10, 10), 50) };

    EXPECT_EQ(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 1) }, prop).size(), 1u);
    EXPECT_EQ(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Melee, Box(0, 0, 10, 10), 2) }, prop).size(), 1u);
    EXPECT_TRUE(Resolve(resolver, { MakeHit(eTeam::Enemy, eHitKind::Bullet, Box(0, 0, 10, 10), 3) }, prop).empty());
}

TEST(Combat, NeutralHazardsHitBothSidesButNotTheirOwner)
{
    HitResolver                resolver;
    const slug::Vector<Hurtbox> hurts = {
        MakeHurt(eTeam::Player, Box(0, 0, 10, 10), 1),
        MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 2),
        MakeHurt(eTeam::Neutral, Box(0, 0, 10, 10), 3),
    };

    // 위험물이 pierce 로 모두 훑는다 (Neutral 끼리는 무시)
    auto       hit    = MakeHit(eTeam::Neutral, eHitKind::Hazard, Box(0, 0, 10, 10), 9, true, 1, 3);
    const auto events = Resolve(resolver, { hit }, hurts);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].target, 1u);
    EXPECT_EQ(events[1].target, 2u);
}

TEST(Combat, OwnerNeverHitsItself)
{
    HitResolver resolver;
    // 폭발이 터진 본인 Hurtbox 는 Neutral 위험물이어도 무시
    Hurtbox self   = MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 100);
    auto    hazard = MakeHit(eTeam::Neutral, eHitKind::Blast, Box(0, 0, 10, 10), 1, true, 5, 100);
    EXPECT_TRUE(Resolve(resolver, { hazard }, { self }).empty());
}

// ===========================================
//  칼은 모든 적을 때린다 (클래스별 예외 없음)
// ===========================================

TEST(Combat, KnifeHitsEveryEnemyRegardlessOfKind)
{
    // 다양한 "적 종류" 를 팀/소유자만 다르게 만들어 본다. 종류에 따른 분기는 존재하지 않아야 한다.
    HitResolver          resolver;
    slug::Vector<Hurtbox> enemies;
    for (uint32_t i = 0; i < 20; ++i)
    {
        // 서로 겹치지 않게 일렬로 배치 (병사, 차량, 전차, 헬기, 보스 부위 …)
        const float x = static_cast<float>(i) * 30.f;
        enemies.push_back(MakeHurt(eTeam::Enemy, Box(x, 0, x + 24.f, 40), 200 + i));
    }

    for (uint32_t i = 0; i < enemies.size(); ++i)
    {
        const float x      = static_cast<float>(i) * 30.f;
        const auto  knife  = MakeHit(eTeam::Player, eHitKind::Melee, Box(x + 10.f, 10, x + 30.f, 30), 1000 + i, false, 4);
        const auto  events = Resolve(resolver, { knife }, enemies);
        ASSERT_EQ(events.size(), 1u) << "enemy " << i;
        EXPECT_EQ(events[0].target, 200 + i);
        EXPECT_EQ(events[0].damage, 4);
        EXPECT_EQ(events[0].kind, eHitKind::Melee);
    }
}

TEST(Combat, HurtboxMayRestrictAcceptedKinds)
{
    HitResolver resolver;
    Hurtbox     armored = MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 5);
    armored.accepts     = HitKindMask { eHitKind::Blast, eHitKind::Melee };   // 총알은 튕긴다

    EXPECT_TRUE(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 1) }, { armored }).empty());
    EXPECT_EQ(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Blast, Box(0, 0, 10, 10), 2) }, { armored }).size(), 1u);
    EXPECT_EQ(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Melee, Box(0, 0, 10, 10), 3) }, { armored }).size(), 1u);
}

TEST(Combat, InvulnerableHurtboxIgnoresHitsWithoutConsumingThem)
{
    HitResolver resolver;
    Hurtbox     shielded   = MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 5);
    shielded.bInvulnerable = true;
    const Hurtbox open     = MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 6);

    const auto events = Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 1) }, { shielded, open });
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].target, 6u);
}

// ===========================================
//  소모 / 관통 / 중복 방지
// ===========================================

TEST(Combat, NonPierceHitboxIsConsumedByFirstHitInIndexOrder)
{
    HitResolver resolver;
    const auto  events = Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 100, 10), 1) }, { MakeHurt(eTeam::Enemy, Box(40, 0, 50, 10), 1), MakeHurt(eTeam::Enemy, Box(10, 0, 20, 10), 2) });

    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].hurtboxIndex, 0u);   // 결정적: 가장 낮은 hurtbox 인덱스가 먼저
}

TEST(Combat, ConsumedBulletDoesNotHitAgainOnLaterFrames)
{
    HitResolver                resolver;
    const slug::Vector<Hitbox>  bullet = { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 77) };
    const slug::Vector<Hurtbox> target = { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 1) };

    EXPECT_EQ(Resolve(resolver, bullet, target).size(), 1u);
    EXPECT_TRUE(Resolve(resolver, bullet, target).empty());                                             // 같은 id 는 소모됨
    EXPECT_TRUE(Resolve(resolver, bullet, { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 2) }).empty());   // 다른 대상에게도 안 감

    resolver.Release(77);   // 새 공격이 같은 id 를 재사용
    EXPECT_EQ(Resolve(resolver, bullet, target).size(), 1u);
}

TEST(Combat, PiercingHitboxHitsEachTargetExactlyOnce)
{
    HitResolver               resolver;
    const slug::Vector<Hitbox> laser   = { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 200, 10), 5, true, 2) };
    slug::Vector<Hurtbox>      targets = { MakeHurt(eTeam::Enemy, Box(10, 0, 20, 10), 1), MakeHurt(eTeam::Enemy, Box(50, 0, 60, 10), 2) };

    auto events = Resolve(resolver, laser, targets);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].target, 1u);
    EXPECT_EQ(events[1].target, 2u);

    // 다음 프레임에도 계속 겹쳐 있지만 이미 맞은 대상은 제외
    EXPECT_TRUE(Resolve(resolver, laser, targets).empty());

    // 새 대상이 나타나면 그 대상만 맞는다
    targets.push_back(MakeHurt(eTeam::Enemy, Box(100, 0, 110, 10), 3));
    events = Resolve(resolver, laser, targets);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].target, 3u);
}

TEST(Combat, MeleeSwingSharingAnIdHitsEachTargetOncePerSwing)
{
    HitResolver                resolver;
    const slug::Vector<Hurtbox> enemy = { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 1) };

    // 칼 휘두르기: 여러 프레임 활성, 비관통이지만 한 번 맞으면 소모
    const slug::Vector<Hitbox> swing = { MakeHit(eTeam::Player, eHitKind::Melee, Box(0, 0, 10, 10), 31, false, 4) };
    int                       total = 0;
    for (int frame = 0; frame < 6; ++frame)
    {
        total += static_cast<int>(Resolve(resolver, swing, enemy).size());
    }
    EXPECT_EQ(total, 1);

    // 다음 휘두르기(새 id) 는 다시 맞는다
    const slug::Vector<Hitbox> nextSwing = { MakeHit(eTeam::Player, eHitKind::Melee, Box(0, 0, 10, 10), 32, false, 4) };
    EXPECT_EQ(Resolve(resolver, nextSwing, enemy).size(), 1u);
}

TEST(Combat, PierceHitsOnlyOncePerOwnerEvenWithManyHurtboxes)
{
    HitResolver                resolver;
    const slug::Vector<Hurtbox> boss   = { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 9), MakeHurt(eTeam::Enemy, Box(5, 0, 15, 10), 9) };   // 같은 owner 의 두 부위
    const auto                 events = Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Blast, Box(0, 0, 20, 10), 4, true, 20) }, boss);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].hurtboxIndex, 0u);
}

TEST(Combat, UntrackedHitboxHitsEveryCall)
{
    // id 0 = 추적 안 함 (지속 피해 구역 등)
    HitResolver                resolver;
    const slug::Vector<Hitbox>  zone   = { MakeHit(eTeam::Neutral, eHitKind::Hazard, Box(0, 0, 10, 10), kNoHitId, true, 1, 0) };
    const slug::Vector<Hurtbox> victim = { MakeHurt(eTeam::Player, Box(0, 0, 10, 10), 1) };
    EXPECT_EQ(Resolve(resolver, zone, victim).size(), 1u);
    EXPECT_EQ(Resolve(resolver, zone, victim).size(), 1u);
    EXPECT_EQ(resolver.GetTrackedHitCount(), 0u);
}

TEST(Combat, EventsAreOrderedByHitboxThenHurtbox)
{
    HitResolver               resolver;
    const slug::Vector<Hitbox> hits = {
        MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 100, 10), 1, true),
        MakeHit(eTeam::Player, eHitKind::Melee, Box(0, 0, 100, 10), 2, true),
    };
    const slug::Vector<Hurtbox> hurts = { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 1), MakeHurt(eTeam::Enemy, Box(20, 0, 30, 10), 2) };

    const auto events = Resolve(resolver, hits, hurts);
    ASSERT_EQ(events.size(), 4u);
    EXPECT_EQ(events[0].hitboxIndex, 0u);
    EXPECT_EQ(events[0].hurtboxIndex, 0u);
    EXPECT_EQ(events[1].hitboxIndex, 0u);
    EXPECT_EQ(events[1].hurtboxIndex, 1u);
    EXPECT_EQ(events[2].hitboxIndex, 1u);
    EXPECT_EQ(events[2].hurtboxIndex, 0u);
    EXPECT_EQ(events[3].hitboxIndex, 1u);
    EXPECT_EQ(events[3].hurtboxIndex, 1u);
}

TEST(Combat, TouchingEdgesAndEmptyBoxesDoNotHit)
{
    HitResolver resolver;
    EXPECT_TRUE(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(0, 0, 10, 10), 1) }, { MakeHurt(eTeam::Enemy, Box(10, 0, 20, 10), 1) }).empty());
    EXPECT_TRUE(Resolve(resolver, { MakeHit(eTeam::Player, eHitKind::Bullet, Box(5, 5, 5, 5), 2) }, { MakeHurt(eTeam::Enemy, Box(0, 0, 10, 10), 1) }).empty());
}

TEST(Combat, ResolverIsDeterministicAcrossIdenticalRuns)
{
    auto Run = []() {
        HitResolver           resolver;
        slug::Vector<uint32_t> log;
        for (int frame = 0; frame < 30; ++frame)
        {
            slug::Vector<Hitbox>  hits;
            slug::Vector<Hurtbox> hurts;
            for (uint32_t i = 0; i < 8; ++i)
            {
                const float x = static_cast<float>(frame * 3 + static_cast<int>(i) * 11);
                hits.push_back(MakeHit(i % 2 == 0 ? eTeam::Player : eTeam::Enemy, i % 3 == 0 ? eHitKind::Melee : eHitKind::Bullet, Box(x, 0, x + 14, 10), 1 + i, i % 4 == 0, 1));
                hurts.push_back(MakeHurt(i % 2 == 0 ? eTeam::Enemy : eTeam::Player, Box(x + 6, 0, x + 18, 10), 50 + i));
            }
            for (const HitEvent& event: resolver.Resolve(hits, hurts))
            {
                log.push_back(event.hitboxIndex * 1000 + event.hurtboxIndex);
            }
        }
        return log;
    };
    const auto a = Run();
    const auto b = Run();
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(a, b);
}

// ===========================================
//  Health
// ===========================================

TEST(Health, ApplyDamageClampsAndReportsApplied)
{
    Health health = Health::Make(5);
    EXPECT_EQ(health.ApplyDamage(2), 2);
    EXPECT_EQ(health.hp, 3);
    EXPECT_FALSE(health.IsDead());
    EXPECT_EQ(health.ApplyDamage(10), 3);   // 남은 체력까지만
    EXPECT_EQ(health.hp, 0);
    EXPECT_TRUE(health.IsDead());
    EXPECT_EQ(health.ApplyDamage(1), 0);   // 죽은 뒤에는 무시
}

TEST(Health, NonPositiveDamageAndHealing)
{
    Health health = Health::Make(10);
    EXPECT_EQ(health.ApplyDamage(0), 0);
    EXPECT_EQ(health.ApplyDamage(-5), 0);
    EXPECT_EQ(health.hp, 10);

    (void)health.ApplyDamage(6);
    EXPECT_EQ(health.Heal(3), 3);
    EXPECT_EQ(health.hp, 7);
    EXPECT_EQ(health.Heal(100), 3);   // 최대 체력 초과 불가
    EXPECT_EQ(health.hp, 10);
    EXPECT_FLOAT_EQ(health.GetRatio(), 1.f);
}

TEST(Health, DeadEntitiesCannotBeHealed)
{
    Health health = Health::Make(3);
    (void)health.ApplyDamage(3);
    EXPECT_EQ(health.Heal(2), 0);
    EXPECT_TRUE(health.IsDead());
}

#include <gtest/gtest.h>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Handle.h"

#include "World/World.h"

using namespace slug;

namespace
{
slug::Vector<Entity> Collect(
    World& _world)
{
    slug::Vector<Entity> result;
    _world.ForEachEntity([&](const Entity _entity) {
        result.push_back(_entity);
    });
    return result;
}
}   // namespace

// ===========================================
//  수명 / 지연 파괴
// ===========================================

TEST(World, CreateGivesDistinctLiveEntities)
{
    World        world;
    const Entity a = world.Create();
    const Entity b = world.Create();

    EXPECT_NE(a, b);
    EXPECT_TRUE(world.IsAlive(a));
    EXPECT_TRUE(world.IsAlive(b));
    EXPECT_EQ(world.GetEntityCount(), 2u);
    EXPECT_FALSE(world.IsAlive(Entity {}));   // null 핸들
    EXPECT_FALSE(world.IsAlive(slug::kNullHandle));
}

TEST(World, DestroyIsDeferredUntilFlush)
{
    World        world;
    const Entity e = world.Create();
    (void)world.Add<Health>(e, Health::Make(5));

    world.Destroy(e);
    EXPECT_FALSE(world.IsAlive(e));   // 즉시 죽은 것으로 취급
    EXPECT_TRUE(world.IsValid(e));    // 하지만 데이터는 아직 남아 있다
    ASSERT_NE(world.TryGet<Health>(e), nullptr);
    EXPECT_EQ(world.TryGet<Health>(e)->hp, 5);
    EXPECT_EQ(world.GetEntityCount(), 0u);

    world.FlushDestroyed();
    EXPECT_FALSE(world.IsValid(e));
    EXPECT_EQ(world.TryGet<Health>(e), nullptr);
    EXPECT_EQ(world.GetEntityCount(), 0u);
}

TEST(World, DoubleDestroyAndDeadHandlesAreIgnored)
{
    World        world;
    const Entity a = world.Create();
    const Entity b = world.Create();

    world.Destroy(a);
    world.Destroy(a);   // 중복
    world.FlushDestroyed();
    world.Destroy(a);   // 이미 사라진 핸들
    world.Destroy(Entity {});
    world.FlushDestroyed();

    EXPECT_TRUE(world.IsAlive(b));
    EXPECT_EQ(world.GetEntityCount(), 1u);
}

TEST(World, StaleHandleStaysDeadAfterSlotReuse)
{
    World        world;
    const Entity old = world.Create();
    (void)world.Add<Health>(old, Health::Make(3));
    world.Destroy(old);
    world.FlushDestroyed();

    // 슬롯을 재사용하는 새 엔티티들
    slug::Vector<Entity> fresh;
    for (int i = 0; i < 8; ++i)
    {
        fresh.push_back(world.Create());
    }

    bool bSlotReused = false;
    for (const Entity e: fresh)
    {
        EXPECT_NE(e, old);
        bSlotReused = bSlotReused || e.GetIndex() == old.GetIndex();
    }
    EXPECT_TRUE(bSlotReused);   // 재사용이 실제로 일어났다는 전제 확인

    EXPECT_FALSE(world.IsAlive(old));
    EXPECT_FALSE(world.IsValid(old));
    EXPECT_EQ(world.TryGet<Health>(old), nullptr);

    // 낡은 핸들로 Destroy 해도 새 엔티티는 영향받지 않는다
    world.Destroy(old);
    world.FlushDestroyed();
    for (const Entity e: fresh)
    {
        EXPECT_TRUE(world.IsAlive(e));
    }
}

TEST(World, ComponentsOfDestroyedEntityDoNotLeakToReusedSlot)
{
    World        world;
    const Entity a = world.Create();
    (void)world.Add<Transform>(a, Transform {
                                      { 5.f, 6.f },
                                      { 5.f, 6.f },
                                      -1
    });
    (void)world.Add<Health>(a, Health::Make(9));
    world.Destroy(a);
    world.FlushDestroyed();

    const Entity b = world.Create();
    EXPECT_FALSE(world.Has<Transform>(b));
    EXPECT_FALSE(world.Has<Health>(b));
}

TEST(World, ClearInvalidatesEveryOldHandleEvenAfterRecreating)
{
    World               world;
    slug::Vector<Entity> old;
    for (int i = 0; i < 5; ++i)
    {
        old.push_back(world.Create());
    }
    world.Clear();
    EXPECT_EQ(world.GetEntityCount(), 0u);

    for (int i = 0; i < 5; ++i)
    {
        (void)world.Create();
    }
    for (const Entity e: old)
    {
        EXPECT_FALSE(world.IsAlive(e));
    }
}

// ===========================================
//  순회 순서
// ===========================================

TEST(World, IterationIsCreationOrderAndSurvivesDestroys)
{
    World               world;
    slug::Vector<Entity> created;
    for (int i = 0; i < 6; ++i)
    {
        created.push_back(world.Create());
    }

    world.Destroy(created[1]);
    world.Destroy(created[4]);
    world.FlushDestroyed();

    // 슬롯이 재사용되어도 순서는 "생성 순서" (남은 것 → 새로 만든 것)
    const Entity n1 = world.Create();
    const Entity n2 = world.Create();

    const slug::Vector<Entity> order    = Collect(world);
    const slug::Vector<Entity> expected = { created[0], created[2], created[3], created[5], n1, n2 };
    EXPECT_EQ(order, expected);
}

TEST(World, DestroyDuringIterationIsSafeAndDeferred)
{
    World               world;
    slug::Vector<Entity> created;
    for (int i = 0; i < 5; ++i)
    {
        const Entity e = world.Create();
        (void)world.Add<Health>(e, Health::Make(i + 1));
        created.push_back(e);
    }

    int visited = 0;
    world.ForEach<Health>([&](const Entity _entity, Health& _health) {
        ++visited;
        if (_health.hp == 2)
        {
            world.Destroy(_entity);
            world.Destroy(created[3]);   // 아직 방문 전인 엔티티를 파괴 → 이후 순회에서 건너뛴다
        }
    });
    EXPECT_EQ(visited, 4);   // 5개 중 파괴 요청된 created[3] 은 방문하지 않음
    world.FlushDestroyed();
    EXPECT_EQ(world.GetEntityCount(), 3u);
}

TEST(World, EntitiesCreatedDuringIterationAreNotVisitedThisPass)
{
    World world;
    (void)world.Create();
    (void)world.Create();

    int visited = 0;
    world.ForEachEntity([&](const Entity) {
        ++visited;
        (void)world.Create();
    });
    EXPECT_EQ(visited, 2);
    EXPECT_EQ(world.GetEntityCount(), 4u);
}

TEST(World, ForEachVisitsOnlyEntitiesWithAllRequestedComponents)
{
    World        world;
    const Entity both  = world.Create();
    const Entity onlyT = world.Create();
    const Entity onlyH = world.Create();
    (void)world.Add<Transform>(both);
    (void)world.Add<Health>(both, Health::Make(1));
    (void)world.Add<Transform>(onlyT);
    (void)world.Add<Health>(onlyH, Health::Make(1));

    slug::Vector<Entity> seen;
    world.ForEach<Transform, Health>([&](const Entity _entity, Transform&, Health&) {
        seen.push_back(_entity);
    });
    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0], both);

    int transforms = 0;
    world.ForEach<Transform>([&](const Entity, Transform&) {
        ++transforms;
    });
    EXPECT_EQ(transforms, 2);
}

TEST(World, ForEachAllowsMutatingComponentsInPlace)
{
    World world;
    for (int i = 0; i < 4; ++i)
    {
        const Entity e = world.Create();
        (void)world.Add<Transform>(e, Transform {
                                          { static_cast<float>(i), 0.f },
                                          {                   0.f, 0.f },
                                          1
        });
    }

    world.ForEach<Transform>([](const Entity, Transform& _t) {
        _t.prevPosition = _t.position;
        _t.position.x += 10.f;
    });

    float sum = 0.f;
    world.ForEach<Transform>([&](const Entity, Transform& _t) {
        sum += _t.position.x - _t.prevPosition.x;
    });
    EXPECT_FLOAT_EQ(sum, 40.f);
}

// ===========================================
//  컴포넌트
// ===========================================

TEST(World, ComponentAddGetHasRemove)
{
    World        world;
    const Entity e = world.Create();

    EXPECT_FALSE(world.Has<Body>(e));
    Body& body    = world.Add<Body>(e);
    body.position = { 10.f, 20.f };
    EXPECT_TRUE(world.Has<Body>(e));
    EXPECT_EQ(world.Get<Body>(e).position, (Vec2 { 10.f, 20.f }));

    world.Remove<Body>(e);
    EXPECT_FALSE(world.Has<Body>(e));
    world.Remove<Body>(e);   // 없는 컴포넌트 제거는 무시
    EXPECT_EQ(world.TryGet<Body>(e), nullptr);
}

TEST(World, AddingTwiceReplacesTheComponent)
{
    World        world;
    const Entity e = world.Create();
    (void)world.Add<Health>(e, Health::Make(3));
    (void)world.Add<Health>(e, Health::Make(8));
    EXPECT_EQ(world.Get<Health>(e).hp, 8);
}

TEST(World, SpriteAndTeamComponentsHoldTheirData)
{
    World        world;
    const Entity e = world.Create();

    Sprite& sprite = world.Add<Sprite>(e);
    sprite.layer   = 5;
    sprite.bFlipX  = true;
    EXPECT_EQ(sprite.anim.GetFrame(), 0);
    EXPECT_TRUE(sprite.anim.GetClip().IsNull());

    (void)world.Add<Team>(e, Team { eTeam::Enemy });
    EXPECT_EQ(world.Get<Team>(e).team, eTeam::Enemy);
    EXPECT_EQ(world.Get<Sprite>(e).layer, 5);
}

// ===========================================
//  결정성
// ===========================================

TEST(World, RandomCreateDestroySequencesProduceIdenticalOrders)
{
    auto Run = []() {
        World               world;
        slug::Vector<Entity> live;
        uint32_t            state = 99u;
        auto                Next  = [&]() {
            state = state * 1664525u + 1013904223u;
            return state >> 8;
        };

        slug::Vector<uint32_t> trace;
        for (int step = 0; step < 400; ++step)
        {
            const uint32_t roll = Next() % 10;
            if (roll < 6 || live.empty())
            {
                const Entity e = world.Create();
                (void)world.Add<Health>(e, Health::Make(static_cast<int>(Next() % 9) + 1));
                live.push_back(e);
            }
            else
            {
                const size_t index = Next() % live.size();
                world.Destroy(live[index]);
                live.erase(live.begin() + static_cast<ptrdiff_t>(index));
            }
            if (step % 7 == 0)
            {
                world.FlushDestroyed();
            }

            world.ForEach<Health>([&](const Entity _entity, Health& _health) {
                trace.push_back(_entity.GetValue() ^ static_cast<uint32_t>(_health.hp));
            });
        }
        return trace;
    };

    const auto a = Run();
    const auto b = Run();
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(a, b);
}

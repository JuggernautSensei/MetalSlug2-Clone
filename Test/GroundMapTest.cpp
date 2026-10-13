
#include <gtest/gtest.h>

#include <cstdint>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "World/Geometry.h"
#include "World/GroundMap.h"

struct Lcg
{
    uint32_t state = 12345u;

    uint32_t Next()
    {
        state = state * 1664525u + 1013904223u;
        return state >> 8;
    }
};

struct Circle
{
    VECTOR2 center = { 0.f, 0.f };
    float   radius = 0.f;

    [[nodiscard]] bool Overlaps(
        const Circle& _other) const
    {
        const float dx  = _other.center.x - center.x;
        const float dy  = _other.center.y - center.y;
        const float sum = radius + _other.radius;
        return dx * dx + dy * dy < sum * sum;
    }

    [[nodiscard]] bool Overlaps(
        const Aabb& _box) const
    {
        const float cx = Clamp(center.x, _box.min.x, _box.max.x);
        const float cy = Clamp(center.y, _box.min.y, _box.max.y);
        const float dx = center.x - cx;
        const float dy = center.y - cy;
        return dx * dx + dy * dy < radius * radius;
    }
};

namespace
{
[[nodiscard]] Circle MakeCircle_(
    const float _x,
    const float _y,
    const float _radius)
{
    Circle circle;
    circle.center = { _x, _y };
    circle.radius = _radius;
    return circle;
}
}   // namespace

TEST(GroundMap, MakeFromRowsBuildsAsciiMap)
{
    const ARRAY<StringView, 3> rows = {
        "....",
        ".##.",
        "####"
    };
    const GroundMap map = GroundMap::MakeFromRows(rows);
    EXPECT_EQ(map.GetWidth(), 4);
    EXPECT_EQ(map.GetHeight(), 3);
    EXPECT_FALSE(map.IsSolid(1, 0));
    EXPECT_TRUE(map.IsSolid(1, 1));
    EXPECT_FALSE(map.IsSolid(3, 1));
    EXPECT_TRUE(map.IsSolid(3, 2));
}

TEST(GroundMap, OutOfBoundsPolicyDefaultsAndOverrides)
{
    GroundMap map = GroundMap::MakeEmpty(10, 10);
    EXPECT_FALSE(map.IsSolid(-1, 5));
    EXPECT_FALSE(map.IsSolid(10, 5));
    EXPECT_FALSE(map.IsSolid(5, -1));
    EXPECT_TRUE(map.IsSolid(5, 10));
    EXPECT_TRUE(map.IsSolid(-3, 12));

    GroundBoundsPolicy policy;
    policy.bSolidLeft   = true;
    policy.bSolidBottom = false;
    map.SetBoundsPolicy(policy);
    EXPECT_TRUE(map.IsSolid(-1, 5));
    EXPECT_FALSE(map.IsSolid(5, 10));
}

TEST(GroundMap, FindGroundYScansDownWithinRange)
{
    GroundMap map = GroundMap::MakeEmpty(20, 50);
    map.FillRect(0, 30, 19, 49, true);

    EXPECT_EQ(map.FindGroundY(5, 0, 100), 30);
    EXPECT_EQ(map.FindGroundY(5, 30, 0), 30);
    EXPECT_EQ(map.FindGroundY(5, 25, 5), 30);
    EXPECT_FALSE(map.FindGroundY(5, 10, 10).has_value());
    EXPECT_FALSE(map.FindGroundY(5, 10, -1).has_value());
}

TEST(GroundMap, FindGroundYSeesMapBottomAsSolid)
{
    const GroundMap map = GroundMap::MakeEmpty(20, 50);
    EXPECT_EQ(map.FindGroundY(5, 45, 20), 50);
    EXPECT_FALSE(map.FindGroundY(5, 10, 20).has_value());
}

TEST(GroundMap, IsRectSolidMatchesBruteForce)
{
    Lcg       rng;
    GroundMap map = GroundMap::MakeEmpty(70, 40);
    for (int i = 0; i < 160; ++i)
    {
        map.SetSolid(static_cast<int>(rng.Next() % 70), static_cast<int>(rng.Next() % 40), true);
    }

    for (int trial = 0; trial < 600; ++trial)
    {
        const int x0 = static_cast<int>(rng.Next() % 100) - 15;
        const int y0 = static_cast<int>(rng.Next() % 60) - 10;
        const int x1 = x0 + static_cast<int>(rng.Next() % 25);
        const int y1 = y0 + static_cast<int>(rng.Next() % 12);

        bool bExpected = false;
        for (int y = y0; y <= y1 && !bExpected; ++y)
        {
            for (int x = x0; x <= x1 && !bExpected; ++x)
            {
                bExpected = map.IsSolid(x, y);
            }
        }
        ASSERT_EQ(map.IsRectSolid(x0, y0, x1, y1), bExpected) << "rect " << x0 << "," << y0 << " " << x1 << "," << y1;
    }
}

TEST(GroundMap, IsRectSolidHandlesReversedBoundsAndSingleByteRanges)
{
    GroundMap map = GroundMap::MakeEmpty(32, 8);
    map.SetSolid(11, 3, true);
    EXPECT_TRUE(map.IsRectSolid(11, 3, 11, 3));
    EXPECT_TRUE(map.IsRectSolid(14, 5, 9, 1));
    EXPECT_FALSE(map.IsRectSolid(8, 0, 10, 7));
    EXPECT_TRUE(map.IsRectSolid(8, 0, 11, 7));
}

TEST(GroundMap, IsRectSolidRespectsOutOfBoundsPolicy)
{
    const GroundMap map = GroundMap::MakeEmpty(20, 20);
    EXPECT_FALSE(map.IsRectSolid(-10, 2, -1, 5));
    EXPECT_FALSE(map.IsRectSolid(5, -9, 8, -1));
    EXPECT_TRUE(map.IsRectSolid(5, 15, 8, 25));
    EXPECT_TRUE(map.IsRectSolid(-5, 18, 3, 22));
}

TEST(GroundMap, EditingPrimitives)
{
    GroundMap map = GroundMap::MakeEmpty(16, 16);
    map.FillRect(2, 2, 5, 4, true);
    EXPECT_EQ(map.CountSolidPixels(), 12u);
    map.FillRect(3, 3, 4, 3, false);
    EXPECT_EQ(map.CountSolidPixels(), 10u);
    map.SetSolid(100, 100, true);
    EXPECT_EQ(map.CountSolidPixels(), 10u);
    map.FillRect(-5, -5, 100, 100, true);
    EXPECT_EQ(map.CountSolidPixels(), 256u);
}

TEST(GroundMap, SlopeEstimateOnRampAndFlat)
{
    GroundMap map = GroundMap::MakeEmpty(120, 120);
    for (int x = 0; x < 100; ++x)
    {
        map.FillRect(x, 100 - x, x, 119, true);
    }
    map.FillRect(100, 100, 119, 119, true);

    EXPECT_NEAR(map.EstimateSlope(50, 50, 4, 8), -1.f, 1e-4f);
    const VECTOR2 normal = map.EstimateNormal(50, 50, 4, 8);
    EXPECT_NEAR(normal.x, -0.70710678f, 1e-4f);
    EXPECT_NEAR(normal.y, -0.70710678f, 1e-4f);

    EXPECT_FLOAT_EQ(map.EstimateSlope(110, 100, 4, 8), 0.f);
    const VECTOR2 flat = map.EstimateNormal(110, 100, 4, 8);
    EXPECT_FLOAT_EQ(flat.x, 0.f);
    EXPECT_FLOAT_EQ(flat.y, -1.f);
}

TEST(Geometry, AabbFactoriesAndAccessors)
{
    const Aabb a = Aabb::FromBottomCenter({ 100.f, 200.f }, { 24.f, 40.f });
    EXPECT_FLOAT_EQ(a.min.x, 88.f);
    EXPECT_FLOAT_EQ(a.max.x, 112.f);
    EXPECT_FLOAT_EQ(a.min.y, 160.f);
    EXPECT_FLOAT_EQ(a.max.y, 200.f);
    EXPECT_FLOAT_EQ(a.GetWidth(), 24.f);
    EXPECT_FLOAT_EQ(a.GetHeight(), 40.f);
    EXPECT_EQ(a.GetCenter(), (VECTOR2 { 100.f, 180.f }));

    const Aabb b = Aabb::FromCenterSize({ 10.f, 10.f }, { 4.f, 6.f });
    EXPECT_EQ(b.min, (VECTOR2 { 8.f, 7.f }));
    EXPECT_EQ(b.max, (VECTOR2 { 12.f, 13.f }));
    EXPECT_EQ(b.GetSize(), (VECTOR2 { 4.f, 6.f }));
}

TEST(Geometry, AabbOverlapIsStrictSoTouchingEdgesDoNotOverlap)
{
    const Aabb a = Aabb::FromMinMax({ 0.f, 0.f }, { 10.f, 10.f });
    EXPECT_TRUE(a.Overlaps(Aabb::FromMinMax({ 9.f, 9.f }, { 20.f, 20.f })));
    EXPECT_FALSE(a.Overlaps(Aabb::FromMinMax({ 10.f, 0.f }, { 20.f, 10.f })));
    EXPECT_FALSE(a.Overlaps(Aabb::FromMinMax({ 0.f, 10.f }, { 10.f, 20.f })));
    EXPECT_FALSE(a.Overlaps(Aabb::FromMinMax({ 11.f, 11.f }, { 20.f, 20.f })));
}

TEST(Geometry, AabbContainsPointAndBox)
{
    const Aabb a = Aabb::FromMinMax({ 0.f, 0.f }, { 10.f, 10.f });
    EXPECT_TRUE(a.Contains(VECTOR2 { 0.f, 0.f }));
    EXPECT_TRUE(a.Contains(VECTOR2 { 9.99f, 5.f }));
    EXPECT_FALSE(a.Contains(VECTOR2 { 10.f, 5.f }));
    EXPECT_TRUE(a.Contains(Aabb::FromMinMax({ 2.f, 2.f }, { 8.f, 8.f })));
    EXPECT_TRUE(a.Contains(a));
    EXPECT_FALSE(a.Contains(Aabb::FromMinMax({ 2.f, 2.f }, { 12.f, 8.f })));
}

TEST(Geometry, CircleOverlaps)
{
    const Circle c      = MakeCircle_(0.f, 0.f, 5.f);
    const Circle nearby = MakeCircle_(6.f, 0.f, 2.f);
    const Circle distantCircle    = MakeCircle_(7.f, 0.f, 2.f);
    EXPECT_TRUE(c.Overlaps(nearby));
    EXPECT_FALSE(c.Overlaps(distantCircle));
    EXPECT_TRUE(c.Overlaps(Aabb::FromMinMax({ 4.f, -1.f }, { 10.f, 1.f })));
    EXPECT_FALSE(c.Overlaps(Aabb::FromMinMax({ 4.f, 4.f }, { 10.f, 10.f })));
}

TEST(GroundMap, PeriodicTailRepeatsTheTileBeyondTheWidth)
{
    GroundMap map = GroundMap::MakeEmpty(16, 10);
    for (int y = 0; y < 10; ++y)
    {
        map.SetSolid(12, y, true);
        map.SetSolid(13, y, true);
    }
    map.SetPeriodicTail(16, 12, 4);
    EXPECT_TRUE(map.HasPeriodicTail());
    EXPECT_TRUE(map.IsSolid(16, 3));
    EXPECT_TRUE(map.IsSolid(17, 3));
    EXPECT_FALSE(map.IsSolid(18, 3));
    EXPECT_FALSE(map.IsSolid(19, 3));
    EXPECT_TRUE(map.IsSolid(20, 3));
    EXPECT_FALSE(map.IsSolid(16 + 4 * 100 + 2, 3));
    EXPECT_TRUE(map.IsSolid(16 + 4 * 100, 3));
}

#include <optional>
#include <set>
#include <utility>

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/GroundEditor.h"
#include "Game/Stage/StageMapping.h"

namespace
{
[[nodiscard]] RbSet<std::pair<int, int>> GroundTestSolidSet_(
    const GroundMaskDocument& _doc)
{
    RbSet<std::pair<int, int>> out;
    for (int y = 0; y < _doc.GetHeight(); ++y)
    {
        for (int x = 0; x < _doc.GetWidth(); ++x)
        {
            if (_doc.IsSolid(x, y))
            {
                out.insert({ x, y });
            }
        }
    }
    return out;
}

[[nodiscard]] FilePath GroundTestMaskPath_()
{
    return FindAssetRoot() / "Ground" / "Stage0_0.png";
}
}   // namespace

TEST(GroundEditorTest, RealMaskSavesAndReloadsWithIdenticalPixels)
{
    if (!std::filesystem::exists(GroundTestMaskPath_()))
    {
        GTEST_SKIP() << "Stage0_0.png 없음";
    }
    GroundMaskDocument doc;
    ASSERT_TRUE(doc.Load(GroundTestMaskPath_()));
    EXPECT_EQ(doc.GetWidth(), 14094);
    EXPECT_EQ(doc.GetHeight(), 1328);

    const FilePath tmp = std::filesystem::temp_directory_path() / "slug_groundeditor_roundtrip.png";
    ASSERT_TRUE(doc.Save(tmp));

    GroundMaskDocument reloaded;
    ASSERT_TRUE(reloaded.Load(tmp));
    EXPECT_EQ(reloaded.GetWidth(), doc.GetWidth());
    EXPECT_EQ(reloaded.GetHeight(), doc.GetHeight());
    EXPECT_TRUE(reloaded.GetPixels() == doc.GetPixels());

    const std::optional<GroundMap> gameMask = LoadGroundMask(tmp);
    ASSERT_TRUE(gameMask.has_value());
    const std::optional<GroundMap> original = LoadGroundMask(GroundTestMaskPath_());
    ASSERT_TRUE(original.has_value());
    EXPECT_EQ(gameMask->CountSolidPixels(), original->CountSolidPixels());
    EXPECT_TRUE(gameMask->IsSolid(9830, 1300));
    std::error_code ec;
    std::filesystem::remove(tmp, ec);
}

TEST(GroundEditorTest, PencilEraserRectAndFillProduceExpectedPixels)
{
    GroundMaskDocument doc;
    doc.Create(20, 10);

    doc.Stamp(5, 5, 1, true);
    EXPECT_EQ(GroundTestSolidSet_(doc), (RbSet<std::pair<int, int>> {
                                            { 5, 5 }
    }));

    doc.Stamp(5, 5, 1, false);
    EXPECT_TRUE(GroundTestSolidSet_(doc).empty());

    doc.FillRect(2, 2, 4, 3, true);
    EXPECT_EQ(GroundTestSolidSet_(doc).size(), 6u);
    EXPECT_TRUE(doc.IsSolid(2, 2));
    EXPECT_TRUE(doc.IsSolid(4, 3));
    EXPECT_FALSE(doc.IsSolid(5, 3));

    doc.FillRect(3, 2, 3, 3, false);
    EXPECT_EQ(GroundTestSolidSet_(doc).size(), 4u);
    EXPECT_FALSE(doc.IsSolid(3, 2));

    doc.DrawLine(10, 1, 14, 1, 1, true);
    for (int x = 10; x <= 14; ++x)
    {
        EXPECT_TRUE(doc.IsSolid(x, 1));
    }
    EXPECT_FALSE(doc.IsSolid(15, 1));

    doc.Stamp(17, 7, 2, true);
    EXPECT_TRUE(doc.IsSolid(16, 6));
    EXPECT_TRUE(doc.IsSolid(17, 7));
    EXPECT_FALSE(doc.IsSolid(18, 7));
}

TEST(GroundEditorTest, FloodFillStopsAtSolidWalls)
{
    GroundMaskDocument doc;
    doc.Create(10, 10);
    doc.FillRect(3, 3, 6, 3, true);
    doc.FillRect(3, 6, 6, 6, true);
    doc.FillRect(3, 3, 3, 6, true);
    doc.FillRect(6, 3, 6, 6, true);
    const size_t wallCount = GroundTestSolidSet_(doc).size();
    EXPECT_EQ(wallCount, 12u);

    doc.FloodFill(4, 4, true);
    EXPECT_EQ(GroundTestSolidSet_(doc).size(), 16u);
    EXPECT_FALSE(doc.IsSolid(0, 0));
    EXPECT_TRUE(doc.IsSolid(5, 5));

    doc.FloodFill(0, 0, true);
    EXPECT_EQ(GroundTestSolidSet_(doc).size(), 100u);
}

TEST(GroundEditorTest, UndoRedoRestoreExactlyAndStrokesGroupEdits)
{
    GroundMaskDocument doc;
    doc.Create(16, 16);
    EXPECT_FALSE(doc.IsDirty());

    const RbSet<std::pair<int, int>> empty = GroundTestSolidSet_(doc);
    doc.BeginStroke();
    doc.DrawLine(0, 0, 15, 15, 3, true);
    doc.Stamp(8, 0, 1, true);
    doc.EndStroke();
    const RbSet<std::pair<int, int>> afterStroke = GroundTestSolidSet_(doc);
    EXPECT_FALSE(afterStroke.empty());
    EXPECT_TRUE(doc.IsDirty());

    doc.FloodFill(15, 0, true);
    const RbSet<std::pair<int, int>> afterFill = GroundTestSolidSet_(doc);
    EXPECT_GT(afterFill.size(), afterStroke.size());

    doc.Undo();
    EXPECT_EQ(GroundTestSolidSet_(doc), afterStroke);
    doc.Undo();
    EXPECT_EQ(GroundTestSolidSet_(doc), empty);
    EXPECT_FALSE(doc.CanUndo());
    EXPECT_FALSE(doc.IsDirty());

    doc.Redo();
    EXPECT_EQ(GroundTestSolidSet_(doc), afterStroke);
    doc.Redo();
    EXPECT_EQ(GroundTestSolidSet_(doc), afterFill);
    EXPECT_FALSE(doc.CanRedo());

    doc.Undo();
    doc.Stamp(1, 14, 1, true);
    EXPECT_FALSE(doc.CanRedo());
}

TEST(GroundEditorTest, SavedSyntheticMaskLoadsThroughGameLoader)
{
    GroundMaskDocument doc;
    doc.Create(64, 32);
    doc.FillRect(10, 20, 40, 25, true);
    doc.Stamp(50, 5, 3, true);

    const FilePath tmp = std::filesystem::temp_directory_path() / "slug_groundeditor_synthetic.png";
    ASSERT_TRUE(doc.Save(tmp));
    EXPECT_FALSE(doc.IsDirty());

    const std::optional<GroundMap> game = LoadGroundMask(tmp);
    ASSERT_TRUE(game.has_value());
    EXPECT_EQ(game->GetWidth(), 64);
    EXPECT_EQ(game->GetHeight(), 32);
    for (int y = 0; y < 32; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            EXPECT_EQ(game->IsSolid(x, y), doc.IsSolid(x, y)) << x << "," << y;
        }
    }
    std::error_code ec;
    std::filesystem::remove(tmp, ec);
}

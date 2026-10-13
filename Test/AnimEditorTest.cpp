// 애니 오프셋 에디터 데이터 계층 (AnimDocument) 테스트. UI 없음. Assets/anim.json 이 없으면 건너뛴다.
#include <chrono>
#include <filesystem>

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/AnimEditor.h"

namespace
{
[[nodiscard]] String ReadAll_(
    const FilePath& _path)
{
    const Result<String> text = ReadFileText(_path);
    return text ? text.GetValue() : String {};
}
}   // namespace

TEST(AnimEditorTest, LoadEditSaveReloadOnlyPivotDiffers)
{
    const FilePath source = FindAssetRoot() / "anim.json";
    if (!std::filesystem::exists(source))
    {
        GTEST_SKIP() << "anim.json 없음";
    }
    const FilePath tmp1 = std::filesystem::temp_directory_path() / "slug_animeditor_1.json";
    const FilePath tmp2 = std::filesystem::temp_directory_path() / "slug_animeditor_2.json";
    const FilePath tmp3 = std::filesystem::temp_directory_path() / "slug_animeditor_3.json";

    AnimDocument doc;
    ASSERT_TRUE(doc.Load(source));
    ASSERT_GT(doc.GetClips().size(), 1u);

    // 정규화 저장 -> 재로드 -> 재저장이 바이트 동일해야 한다. 원본과도 같다면 서식이 완전 보존된 것.
    ASSERT_TRUE(doc.Save(tmp1));
    AnimDocument reloaded;
    ASSERT_TRUE(reloaded.Load(tmp1));
    ASSERT_TRUE(reloaded.Save(tmp2));
    const String text1 = ReadAll_(tmp1);
    EXPECT_EQ(text1, ReadAll_(tmp2));
    EXPECT_EQ(text1, ReadAll_(source));

    const int       clipIndex = 0;
    const int       frame     = 0;
    const AnimFrame before    = reloaded.GetClips()[clipIndex].frames[frame];
    reloaded.SetPivot(clipIndex, frame, before.pivotX + 3, before.pivotY - 2);
    EXPECT_TRUE(reloaded.IsDirty());
    ASSERT_TRUE(reloaded.Save(tmp3));

    AnimDocument edited;
    ASSERT_TRUE(edited.Load(tmp3));
    ASSERT_EQ(edited.GetClips().size(), doc.GetClips().size());
    for (size_t c = 0; c < doc.GetClips().size(); ++c)
    {
        const AnimEditClip& a = doc.GetClips()[c];
        const AnimEditClip& b = edited.GetClips()[c];
        EXPECT_EQ(a.name, b.name);
        EXPECT_EQ(a.bLoop, b.bLoop);
        EXPECT_EQ(a.loopStart, b.loopStart);
        EXPECT_EQ(a.cancelFrame, b.cancelFrame);
        EXPECT_EQ(a.frameDuration, b.frameDuration);
        ASSERT_EQ(a.frames.size(), b.frames.size());
        for (size_t f = 0; f < a.frames.size(); ++f)
        {
            const AnimFrame& fa      = a.frames[f];
            const AnimFrame& fb      = b.frames[f];
            const bool       bEdited = (c == static_cast<size_t>(clipIndex) && f == static_cast<size_t>(frame));
            EXPECT_EQ(fa.atlas, fb.atlas);
            EXPECT_EQ(fa.x, fb.x);
            EXPECT_EQ(fa.y, fb.y);
            EXPECT_EQ(fa.w, fb.w);
            EXPECT_EQ(fa.h, fb.h);
            EXPECT_EQ(fa.pivotX + (bEdited ? 3 : 0), fb.pivotX);
            EXPECT_EQ(fa.pivotY - (bEdited ? 2 : 0), fb.pivotY);
        }
    }

    const FilePath tmp4 = std::filesystem::temp_directory_path() / "slug_animeditor_4.json";
    ASSERT_TRUE(edited.Save(tmp4));
    EXPECT_EQ(ReadAll_(tmp3), ReadAll_(tmp4));

    ASSERT_TRUE(reloaded.CanUndo());
    reloaded.Undo();
    EXPECT_EQ(reloaded.Serialize(), text1);

    for (const FilePath& p: { tmp1, tmp2, tmp3, tmp4 })
    {
        std::error_code ec;
        std::filesystem::remove(p, ec);
    }
}

namespace
{
constexpr const char* kHotJson = R"({
  "version": 1,
  "animations": {
    "Hot_A": {
      "loop": false,
      "loopStart": 0,
      "frameDuration": 0.1,
      "cellW": 8,
      "cellH": 8,
      "frames": [
        {"atlas": 0, "x": 0, "y": 0, "w": 8, "h": 8, "pivotX": 4, "pivotY": 8},
        {"atlas": 0, "x": 8, "y": 0, "w": 8, "h": 8, "pivotX": 4, "pivotY": 8}
      ]
    }
  }
}
)";

void WriteText_(
    const FilePath& _path,
    const String&   _text)
{
    (void)WriteFileBytes(_path, _text);
}
}   // namespace

TEST(AnimEditorHotReload, PivotEditAppliesToTheLibraryOnTheNextPoll)
{
    const FilePath path = std::filesystem::temp_directory_path() / "slug_hot_1.json";
    WriteText_(path, kHotJson);

    Result<AnimationLibrary> loaded = AnimationLibrary::LoadFromJson(kHotJson);
    ASSERT_TRUE(loaded);
    AnimationLibrary lib = loaded.Take();

    AnimEditor editor;
    editor.Open(path);
    editor.SetLibrary(&lib);
    ASSERT_NE(lib.FindClip("Hot_A"), nullptr);
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[1].pivotX, 4);

    editor.GetDocument().SetPivot(0, 1, 9, 3);
    editor.Poll();
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[1].pivotX, 9);
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[1].pivotY, 3);
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[0].pivotX, 4);

    editor.GetDocument().Undo();
    editor.Poll();
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[1].pivotX, 4);

    editor.GetDocument().SetFrameDuration(0, 0.25);
    editor.GetDocument().SetLoop(0, true);
    editor.Poll();
    EXPECT_FLOAT_EQ(lib.FindClip("Hot_A")->frameDuration, 0.25f);
    EXPECT_TRUE(lib.FindClip("Hot_A")->bLoop);
    std::filesystem::remove(path);
}

TEST(AnimEditorHotReload, ExternalChangeReloadsOnlyWhenThereAreNoUnsavedEdits)
{
    const FilePath path = std::filesystem::temp_directory_path() / "slug_hot_2.json";
    WriteText_(path, kHotJson);
    Result<AnimationLibrary> loaded = AnimationLibrary::LoadFromJson(kHotJson);
    ASSERT_TRUE(loaded);
    AnimationLibrary lib = loaded.Take();

    AnimEditor editor;
    editor.Open(path);
    editor.SetLibrary(&lib);
    EXPECT_FALSE(editor.CheckExternal());   // 변화 없음

    String edited = kHotJson;
    edited.replace(edited.find("\"pivotX\": 4"), 11, "\"pivotX\": 7");
    WriteText_(path, edited);
    std::filesystem::last_write_time(path, std::filesystem::file_time_type::clock::now() + std::chrono::seconds(10));
    EXPECT_TRUE(editor.CheckExternal());
    EXPECT_EQ(editor.GetStatus(), "reloaded anim.json");
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[0].pivotX, 7);
    EXPECT_EQ(editor.GetDocument().GetClips()[0].frames[0].pivotX, 7);

    editor.GetDocument().SetPivot(0, 1, 20, 20);
    editor.Poll();
    WriteText_(path, kHotJson);
    std::filesystem::last_write_time(path, std::filesystem::file_time_type::clock::now() + std::chrono::seconds(20));
    EXPECT_FALSE(editor.CheckExternal());
    EXPECT_NE(editor.GetStatus().find("unsaved"), String::npos);
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[1].pivotX, 20);

    editor.Reload();
    EXPECT_EQ(lib.FindClip("Hot_A")->frames[1].pivotX, 4);
    EXPECT_FALSE(editor.GetDocument().IsDirty());
    std::filesystem::remove(path);
}

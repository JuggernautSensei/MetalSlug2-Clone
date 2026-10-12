#pragma once
#include <cstdint>
#include <filesystem>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Render/Animation.h"

struct SDL_Renderer;
struct SDL_Texture;

// anim.json 의 클립 하나 (파일에 있는 모든 필드를 보존한다).
struct AnimEditClip
{
    String            name          = {};
    bool              bLoop         = false;
    int               loopStart     = 0;
    bool              bHasCancel    = false;   // "cancelFrame" 필드가 있는 클립
    int               cancelFrame   = 0;
    double            frameDuration = 0.1;
    int               cellW         = 0;
    int               cellH         = 0;
    Vector<AnimFrame> frames        = {};
};

// anim.json 문서 (UI 없음). 편집은 Set*/ApplyPivotToAll 로만 하고, 매번 Undo 스택에 쌓는다.
// Serialize 는 원본 파일과 같은 서식(키 순서, 프레임 한 줄)으로 쓴다.
class AnimDocument
{
public:
    [[nodiscard]] Result<void> Load(const FilePath& _path);
    [[nodiscard]] Result<void> Save(const FilePath& _path) const;
    [[nodiscard]] Result<void> LoadFromText(StringView _json);
    [[nodiscard]] String       Serialize() const;

    [[nodiscard]] Span<const AnimEditClip> GetClips() const;
    [[nodiscard]] int                      FindClip(StringView _name) const;   // 없으면 -1
    [[nodiscard]] bool                     IsDirty() const;
    [[nodiscard]] bool                     CanUndo() const;

    void SetPivot(int _clip, int _frame, int _pivotX, int _pivotY);
    void ApplyPivotToAll(int _clip, int _pivotX, int _pivotY);
    void SetFrameDuration(int _clip, double _duration);
    void SetLoop(int _clip, bool _bLoop);
    void SetLoopStart(int _clip, int _loopStart);
    void Undo();
    void ClearDirty();

    // 문서의 클립을 라이브러리에 즉시 반영한다 (같은 이름이면 제자리 교체).
    void ApplyClipTo(int _clip, AnimationLibrary& _library) const;
    void ApplyTo(AnimationLibrary& _library) const;

    [[nodiscard]] int GetRevision() const;          // 편집/Undo/Load 마다 증가
    [[nodiscard]] int GetLastChangedClip() const;   // Load 직후는 -1 (전체 변경)

private:
    struct UndoEntry
    {
        int          clip     = 0;
        AnimEditClip snapshot = {};
    };

    void PushUndo_(int _clip);

    Vector<AnimEditClip> m_clips = {};
    Vector<UndoEntry>    m_undo  = {};
    int                  m_version     = 1;
    bool                 m_bDirty      = false;
    int                  m_revision    = 0;
    int                  m_lastChanged = -1;
};

// Animation 탭 UI. 아틀라스 페이지는 처음 쓸 때 텍스처로 읽는다.
class AnimEditor final
{
public:
    void Init(SDL_Renderer* _pRenderer);
    void Shutdown();
    void SelectClip(StringView _name);

    // 실행 중 게임의 애니 라이브러리. 편집/리로드가 여기에 즉시 반영된다.
    void SetLibrary(AnimationLibrary* _pLibrary);
    void Open(const FilePath& _path);

    // 에디터 프레임마다 호출: 문서 편집을 라이브러리에 반영하고 약 1초마다 파일 변경을 확인한다.
    void Poll();

    // 파일 mtime 이 바뀌었으면 (미편집이면 리로드, 편집 중이면 경고). 리로드했으면 true.
    [[nodiscard]] bool CheckExternal();
    void               Reload();

    [[nodiscard]] AnimDocument& GetDocument();
    [[nodiscard]] const String& GetStatus() const;

    // 에디터 프레임 안(ImGui::NewFrame 이후)에서 호출한다.
    void Draw();

private:
    void           DrawList_();
    void           DrawDetail_();
    void           DrawCanvas_(const AnimEditClip& _clip);
    void           HandleKeys_(int _clipIndex);
    void           Save_();
    void           SyncLibrary_();
    void           RememberMtime_();
    [[nodiscard]] SDL_Texture* GetPage_(int _atlas);

    SDL_Renderer*        m_pRenderer = nullptr;
    AnimDocument         m_doc       = {};
    FilePath             m_path      = {};
    Vector<SDL_Texture*> m_pages     = {};
    Vector<int>          m_pageW     = {};
    Vector<int>          m_pageH     = {};
    String               m_status    = {};
    ARRAY<char, 64>      m_filter    = {};
    int                  m_selected  = -1;
    int                  m_frame     = 0;
    int                  m_zoom      = 3;
    bool                 m_bPlaying  = false;
    bool                 m_bOnion    = true;
    float                m_playTimer = 0.f;
    bool                 m_bLoaded   = false;
    AnimationLibrary*    m_pLibrary  = nullptr;
    int                  m_syncedRevision = 0;
    int                  m_pollFrames     = 0;
    std::filesystem::file_time_type m_mtime = {};
};

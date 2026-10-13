#pragma once
#include <cstdint>
#include <filesystem>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Render/Animation.h"

struct SDL_Renderer;
struct SDL_Texture;

struct AnimEditClip
{
    String            name          = {};
    bool              bLoop         = false;
    int               loopStart     = 0;
    bool              bHasCancel    = false;
    int               cancelFrame   = 0;
    double            frameDuration = 0.1;
    int               cellW         = 0;
    int               cellH         = 0;
    Vector<AnimFrame> frames        = {};
};

// anim.json 문서
class AnimDocument
{
public:
    [[nodiscard]] Result<void> Load(const FilePath& _path);
    [[nodiscard]] Result<void> Save(const FilePath& _path) const;
    [[nodiscard]] Result<void> LoadFromText(StringView _json);
    [[nodiscard]] String       Serialize() const;

    [[nodiscard]] Span<const AnimEditClip> GetClips() const;
    [[nodiscard]] int                      FindClip(StringView _name) const;
    [[nodiscard]] bool                     IsDirty() const;
    [[nodiscard]] bool                     CanUndo() const;

    void SetPivot(int _clip, int _frame, int _pivotX, int _pivotY);
    void ApplyPivotToAll(int _clip, int _pivotX, int _pivotY);
    void SetFrameDuration(int _clip, double _duration);
    void SetLoop(int _clip, bool _bLoop);
    void SetLoopStart(int _clip, int _loopStart);
    void Undo();
    void ClearDirty();

    void ApplyClipTo(int _clip, AnimationLibrary& _library) const;
    void ApplyTo(AnimationLibrary& _library) const;

    [[nodiscard]] int GetRevision() const;
    [[nodiscard]] int GetLastChangedClip() const;

private:
    struct UndoEntry
    {
        int          clip     = 0;
        AnimEditClip snapshot = {};
    };

    void PushUndo_(int _clip);

    Vector<AnimEditClip> m_clips       = {};
    Vector<UndoEntry>    m_undo        = {};
    int                  m_version     = 1;
    bool                 m_bDirty      = false;
    int                  m_revision    = 0;
    int                  m_lastChanged = -1;
};

// Animation 탭 UI
class AnimEditor final
{
public:
    void Init(SDL_Renderer* _pRenderer);
    void Shutdown();
    void SelectClip(StringView _name);

    void SetLibrary(AnimationLibrary* _pLibrary);
    void Open(const FilePath& _path);

    void Poll();

    [[nodiscard]] bool CheckExternal();
    void               Reload();

    [[nodiscard]] AnimDocument& GetDocument();
    [[nodiscard]] const String& GetStatus() const;

    // 에디터 프레임 안에서 호출
    void Draw();

private:
    void                       DrawList_();
    void                       DrawDetail_();
    void                       DrawCanvas_(const AnimEditClip& _clip);
    void                       HandleKeys_(int _clipIndex);
    void                       Save_();
    void                       SyncLibrary_();
    void                       RememberMtime_();
    [[nodiscard]] SDL_Texture* GetPage_(int _atlas);

    SDL_Renderer*                   m_pRenderer      = nullptr;
    AnimDocument                    m_doc            = {};
    FilePath                        m_path           = {};
    Vector<SDL_Texture*>            m_pages          = {};
    Vector<int>                     m_pageW          = {};
    Vector<int>                     m_pageH          = {};
    String                          m_status         = {};
    ARRAY<char, 64>                 m_filter         = {};
    int                             m_selected       = -1;
    int                             m_frame          = 0;
    int                             m_zoom           = 3;
    bool                            m_bPlaying       = false;
    bool                            m_bOnion         = true;
    float                           m_playTimer      = 0.f;
    bool                            m_bLoaded        = false;
    AnimationLibrary*               m_pLibrary       = nullptr;
    int                             m_syncedRevision = 0;
    int                             m_pollFrames     = 0;
    std::filesystem::file_time_type m_mtime          = {};
};

#pragma once
#include "Base/SlugError.h"
#include "Render/Animation.h"

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
    [[nodiscard]] Result<void>             Load(const FilePath& _path);
    [[nodiscard]] Result<void>             Save(const FilePath& _path) const;
    [[nodiscard]] Result<void>             LoadFromText(StringView _json);
    [[nodiscard]] String                   Serialize() const;
    [[nodiscard]] Span<const AnimEditClip> GetClips() const;
    [[nodiscard]] int                      FindClip(StringView _name) const;
    [[nodiscard]] bool                     IsDirty() const;
    [[nodiscard]] bool                     CanUndo() const;
    void                                   SetPivot(int _clip, int _frame, int _pivotX, int _pivotY);
    void                                   ApplyPivotToAll(int _clip, int _pivotX, int _pivotY);
    void                                   SetFrameDuration(int _clip, double _duration);
    void                                   SetLoop(int _clip, bool _bLoop);
    void                                   SetLoopStart(int _clip, int _loopStart);
    void                                   Undo();
    void                                   ClearDirty();
    void                                   ApplyClipTo(int _clip, AnimationLibrary& _library) const;
    void                                   ApplyTo(AnimationLibrary& _library) const;
    [[nodiscard]] int                      GetRevision() const;
    [[nodiscard]] int                      GetLastChangedClip() const;

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

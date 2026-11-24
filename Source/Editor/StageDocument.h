#pragma once

#include "Base/SlugError.h"

struct Replacement;

struct StageEntity
{
    String             def         = {};
    String             kind        = {};
    String             id          = {};
    int                section     = -1;
    String             where       = {};
    double             x           = 0.0;
    double             y           = 0.0;
    double             fromY       = 0.0;
    bool               bHasX       = false;
    bool               bHasY       = false;
    bool               bHasZ       = false;
    int                z           = 0;
    int                zBegin      = -1;
    int                zEnd        = -1;
    bool               bGroundSnap = false;
    int                snapBegin   = -1;
    int                snapEnd     = -1;
    bool               bCanCopy    = true;
    bool               bFaceLeft   = false;
    int                begin       = 0;
    int                end         = 0;
    int                xBegin      = -1;
    int                xEnd        = -1;
    int                yBegin      = -1;
    int                yEnd        = -1;
    int                idBegin     = -1;
    int                idEnd       = -1;
    [[nodiscard]] bool IsPlaceable() const;
};

struct StageEditRect
{
    String           type       = {};
    ARRAY<double, 4> v          = {};
    ARRAY<int, 4>    begin      = {};
    ARRAY<int, 4>    end        = {};
    double           glide      = 0.0;
    int              glideBegin = -1;
    int              glideEnd   = -1;
};

struct StageSpriteFrame
{
    String file   = {};
    int    x      = 0;
    int    y      = 0;
    int    w      = 0;
    int    h      = 0;
    int    pivotX = 0;
    int    pivotY = 0;
};

struct StageEditSection
{
    String                id           = {};
    double                triggerX     = 0.0;
    int                   triggerBegin = -1;
    int                   triggerEnd   = -1;
    Vector<StageEditRect> rects        = {};
    Vector<String>        actionTypes  = {};
    int                   arrayBegin   = -1;
    int                   lastBegin    = -1;
    int                   lastEnd      = -1;
};
class StageDocument
{
public:
    [[nodiscard]] Result<void>                 Load(const FilePath& _path);
    [[nodiscard]] Result<void>                 LoadFromText(StringView _json);
    [[nodiscard]] Result<void>                 Save(const FilePath& _path);
    [[nodiscard]] const String&                GetText() const;
    [[nodiscard]] Span<const StageEntity>      GetEntities() const;
    [[nodiscard]] Span<const StageEditSection> GetSections() const;
    [[nodiscard]] int                          GetWorldWidth() const;
    [[nodiscard]] int                          GetWorldHeight() const;
    [[nodiscard]] const String&                GetBackground() const;
    [[nodiscard]] const String&                GetGroundInitial() const;
    [[nodiscard]] Span<const String>           GetGroundMaps() const;
    [[nodiscard]] bool                         IsDirty() const;
    [[nodiscard]] bool                         CanUndo() const;
    bool                                       SetPosition(int _entity, double _x, double _y, bool _bUndo = true);
    bool                                       SetZ(int _entity, int _z, int _defaultZ, bool _bUndo = true);
    // groundSnap 을 끄고 y 를 박는다
    bool UnlockGround(int _entity, double _y, bool _bUndo = true);
    int  Add(StringView _def, StringView _kind, int _section, double _x, double _y);
    int  Duplicate(int _entity, double _dx, double _dy);
    bool Delete(int _entity);
    void Undo();
    bool MoveSection(int _section, double _dx, bool _bTrigger, bool _bRects, bool _bSpawns, bool _bUndo = true);
    bool SetTriggerX(int _section, double _x, bool _bUndo = true);
    bool SetRect(int _section, int _rect, const ARRAY<double, 4>& _v, bool _bUndo = true);
    bool SetGlide(int _section, int _rect, double _glide, bool _bUndo = true);

private:
    [[nodiscard]] bool Reparse_();
    bool               Patch_(Vector<Replacement>& _list, bool _bUndo);
    void               PushUndo_();
    [[nodiscard]] int  FindEntityAt_(int _begin) const;
    [[nodiscard]] bool IsIdUsed_(StringView _id) const;

    String                   m_text          = {};
    String                   m_savedText     = {};
    Vector<String>           m_undo          = {};
    Vector<StageEntity>      m_entities      = {};
    Vector<StageEditSection> m_sections      = {};
    Vector<String>           m_groundMaps    = {};
    String                   m_background    = {};
    String                   m_groundInitial = {};
    int                      m_worldW        = 0;
    int                      m_worldH        = 0;
};

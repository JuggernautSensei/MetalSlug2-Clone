#pragma once
#include <array>
#include <cstdint>

#include "Base/SlugError.h"
#include "Base/Types.h"

struct SDL_Renderer;
struct SDL_Texture;
struct ImDrawList;
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
    [[nodiscard]] Result<void> Load(const FilePath& _path);
    [[nodiscard]] Result<void> LoadFromText(StringView _json);
    [[nodiscard]] Result<void> Save(const FilePath& _path);

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
    bool SetZ(int _entity, int _z, int _defaultZ, bool _bUndo = true);
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

    [[nodiscard]] static String FormatNumber(double _value);

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
class StageEditor final
{
public:
    void Init(SDL_Renderer* _pRenderer);
    void Shutdown();
    void SelectStage(StringView _name);
    void Draw();

private:
    struct Strip
    {
        SDL_Texture* pTex = nullptr;
        int          x    = 0;
        int          w    = 0;
    };
    struct Page
    {
        SDL_Texture* pTex = nullptr;
        int          w    = 0;
        int          h    = 0;
    };
    struct DefInfo
    {
        String name = {};
        String kind = {};
        int    z    = 0;
    };

    void                      LoadStage_(const String& _name);
    void                      LoadGround_(const String& _name);
    void                      LoadDefs_();
    void                      LoadFrames_();
    [[nodiscard]] String      SpriteName_(StringView _kind, StringView _def) const;
    [[nodiscard]] const Page* GetPage_(const String& _file);
    [[nodiscard]] int         DefaultZ_(const StageEntity& _e) const;
    [[nodiscard]] int         EffectiveZ_(const StageEntity& _e) const;
    [[nodiscard]] bool        PickSection_(float _mx, float _my, float _originX, float _originY, int& _section, int& _rect, int& _mask) const;
    [[nodiscard]] bool        DrawSprite_(ImDrawList* _pDraw, const StageEntity& _e, float _px, float _py, bool _bSelected);
    void                      ClearTextures_();
    void                      Save_();
    void                      DrawPanel_();
    void                      DrawView_();
    void                      Select_(int _entity, bool _bCenter);
    [[nodiscard]] int         GroundY_(int _x, int _fromY) const;
    [[nodiscard]] double      MarkerY_(const StageEntity& _e) const;
    void                      LoadStrips_(const FilePath& _path, bool _bGroundMask, Vector<Strip>& _out, int& _w, int& _h);
    [[nodiscard]] const char* DefaultKind_(StringView _def) const;

    SDL_Renderer*                     m_pRenderer    = nullptr;
    StageDocument                     m_doc          = {};
    FilePath                          m_path         = {};
    String                            m_stage        = {};
    String                            m_status       = {};
    Vector<String>                    m_stages       = {};
    Vector<DefInfo>                   m_defs         = {};
    Vector<Strip>                     m_bgStrips     = {};
    Vector<Strip>                     m_groundStrips = {};
    Vector<uint8_t>                   m_solid        = {};
    HashMap<String, StageSpriteFrame> m_frames       = {};
    HashMap<String, String>           m_defSprite    = {};
    HashMap<String, Page>             m_pages        = {};
    String                            m_ground       = {};
    int                               m_bgW          = 0;
    int                               m_bgH          = 0;
    int                               m_groundW      = 0;
    int                               m_groundH      = 0;
    int                               m_selected     = -1;
    int                               m_section      = 0;
    int                               m_defIndex     = 0;
    int                               m_kindIndex    = 0;
    float                             m_zoom         = 0.5f;
    float                             m_scrollX      = 0.f;
    float                             m_scrollY      = 0.f;
    bool                              m_bShowBg      = true;
    bool                              m_bShowGround  = true;
    bool                              m_bShowGrid    = true;
    bool                              m_bShowLabels  = true;
    bool                              m_bOnlySection = false;
    bool                              m_bPlace       = false;
    bool                              m_bDragging    = false;
    bool                              m_bNeedCenter  = false;
    bool                              m_bDragUndone  = false;
    bool                              m_bShowSprites = true;
    int                               m_secDrag      = -1;
    int                               m_rectDrag     = -1;
    int                               m_edgeMask     = 0;
    int                               m_secMode      = 0;
    bool                              m_bSecUndone   = false;
    double                            m_secPos       = 0.0;
    double                            m_secGrabX     = 0.0;
    double                            m_secGrabY     = 0.0;
    double                            m_grabDx       = 0.0;
    double                            m_grabDy       = 0.0;
};

#pragma once
#include <array>
#include <cstdint>

#include "Base/SlugError.h"
#include "Base/Types.h"

struct SDL_Renderer;
struct SDL_Texture;
struct ImDrawList;

#include "Editor/StageDocument.h"

class StageEditor
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

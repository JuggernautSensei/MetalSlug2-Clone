#pragma once
#include <cstdint>

#include "Base/SlugError.h"
#include "Base/Types.h"

struct SDL_Renderer;
struct SDL_Texture;

struct GroundPixelChange
{
    uint32_t index  = 0;
    uint8_t  before = 0;
    uint8_t  after  = 0;
};

class GroundMaskDocument
{
public:
    [[nodiscard]] Result<void>           Load(const FilePath& _path);
    [[nodiscard]] Result<void>           Save(const FilePath& _path);
    void                                 Create(int _width, int _height);
    [[nodiscard]] int                    GetWidth() const;
    [[nodiscard]] int                    GetHeight() const;
    [[nodiscard]] bool                   IsInside(int _x, int _y) const;
    [[nodiscard]] bool                   IsSolid(int _x, int _y) const;
    [[nodiscard]] bool                   IsDirty() const;
    [[nodiscard]] bool                   CanUndo() const;
    [[nodiscard]] bool                   CanRedo() const;
    [[nodiscard]] const Vector<uint8_t>& GetPixels() const;

    // 연속 호출을 하나의 undo 단위로 묶는다
    void               BeginStroke();
    void               EndStroke();
    void               Stamp(int _x, int _y, int _brush, bool _bSolid);
    void               DrawLine(int _x0, int _y0, int _x1, int _y1, int _brush, bool _bSolid);
    void               FillRect(int _x0, int _y0, int _x1, int _y1, bool _bSolid);
    void               FloodFill(int _x, int _y, bool _bSolid);
    void               Undo();
    void               Redo();
    [[nodiscard]] bool TakeDirtyColumns(int& _x0, int& _x1);

private:
    void SetPixel_(int _x, int _y, uint8_t _value);
    void MarkColumn_(int _x);

    Vector<uint8_t>                   m_pixels       = {};
    Vector<Vector<GroundPixelChange>> m_history      = {};
    Vector<GroundPixelChange>         m_stroke       = {};
    size_t                            m_applied      = 0;
    size_t                            m_savedApplied = 0;
    int                               m_width        = 0;
    int                               m_height       = 0;
    int                               m_dirtyX0      = 0;
    int                               m_dirtyX1      = -1;
    bool                              m_bInStroke    = false;
    bool                              m_bSavedLost   = false;
};

enum class eGroundTool
{
    Pencil,
    Eraser,
    Rect,
    Line,
    Fill
};

class GroundEditor
{
public:
    void Init(SDL_Renderer* _pRenderer);
    void Shutdown();
    void Draw();

private:
    struct MaskStrip
    {
        SDL_Texture* pTex = nullptr;
        int          x    = 0;
        int          w    = 0;
    };

    void LoadMask_(const String& _name);
    void LoadBackground_(const String& _maskName);
    void ClearTextures_();
    void UploadMaskColumns_(int _x0, int _x1);
    void Save_();
    void DrawPanel_();
    void DrawView_();
    void HandleKeys_();
    void HandleCanvasInput_(int _pixelX, int _pixelY, bool _bHovered);
    void CommitDrag_(int _pixelX, int _pixelY);

    SDL_Renderer*      m_pRenderer  = nullptr;
    GroundMaskDocument m_doc        = {};
    String             m_name       = {};
    String             m_status     = {};
    Vector<String>     m_names      = {};
    Vector<MaskStrip>  m_maskStrips = {};
    Vector<MaskStrip>  m_bgStrips   = {};
    eGroundTool        m_tool       = eGroundTool::Pencil;
    int                m_brush      = 1;
    int                m_bgH        = 0;
    int                m_startX     = 0;
    int                m_startY     = 0;
    int                m_lastX      = 0;
    int                m_lastY      = 0;
    float              m_zoom       = 1.f;
    float              m_scrollX    = 0.f;
    float              m_scrollY    = 0.f;
    bool               m_bShowBg    = true;
    bool               m_bShowMask  = true;
    bool               m_bDragging  = false;
    bool               m_bDragSolid = true;
    bool               m_bHasLast   = false;
    bool               m_bBgLoaded  = false;
};

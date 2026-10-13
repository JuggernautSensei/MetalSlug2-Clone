#include "Editor/GroundEditor.h"

#include <algorithm>
#include <cmath>
#include <format>

#include <SDL3/SDL.h>
#include <imgui.h>
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4996)
#include <stb_image_write.h>
#pragma warning(pop)

#include "Base/Json.h"
#include "Core/AssetPath.h"

constexpr int   kGroundStripWidth = 2048;
constexpr float kGroundMinZoom    = 0.125f;
constexpr float kGroundMaxZoom    = 16.f;
constexpr float kGroundGridZoom   = 8.f;
constexpr int   kGroundMaxBrush   = 16;

namespace
{
void AppendBigEndian32_(
    Vector<uint8_t>& _out,
    uint32_t         _value)
{
    _out.push_back(static_cast<uint8_t>(_value >> 24));
    _out.push_back(static_cast<uint8_t>(_value >> 16));
    _out.push_back(static_cast<uint8_t>(_value >> 8));
    _out.push_back(static_cast<uint8_t>(_value));
}

void AppendPngChunk_(
    Vector<uint8_t>&       _out,
    const char*            _type,
    const Vector<uint8_t>& _body)
{
    Vector<uint8_t> typeAndBody;
    for (int i = 0; i < 4; ++i)
    {
        typeAndBody.push_back(static_cast<uint8_t>(_type[i]));
    }
    typeAndBody.insert(typeAndBody.end(), _body.begin(), _body.end());
    AppendBigEndian32_(_out, static_cast<uint32_t>(_body.size()));
    _out.insert(_out.end(), typeAndBody.begin(), typeAndBody.end());
    AppendBigEndian32_(_out, stbiw__crc32(typeAndBody.data(), static_cast<int>(typeAndBody.size())));
}

[[nodiscard]] bool EncodePalettePng_(
    const Vector<uint8_t>& _pixels,
    int                    _width,
    int                    _height,
    Vector<uint8_t>&       _out)
{
    Vector<uint8_t> raw;
    raw.reserve((static_cast<size_t>(_width) + 1) * static_cast<size_t>(_height));
    for (int y = 0; y < _height; ++y)
    {
        raw.push_back(0);
        const uint8_t* pRow = &_pixels[static_cast<size_t>(y) * static_cast<size_t>(_width)];
        raw.insert(raw.end(), pRow, pRow + _width);
    }
    int            zlibSize = 0;
    unsigned char* pZlib    = stbi_zlib_compress(raw.data(), static_cast<int>(raw.size()), &zlibSize, 8);
    if (pZlib == nullptr)
    {
        return false;
    }
    const Vector<uint8_t> signature = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    _out                            = signature;
    Vector<uint8_t> header;
    AppendBigEndian32_(header, static_cast<uint32_t>(_width));
    AppendBigEndian32_(header, static_cast<uint32_t>(_height));
    header.push_back(8);
    header.push_back(3);
    header.push_back(0);
    header.push_back(0);
    header.push_back(0);
    AppendPngChunk_(_out, "IHDR", header);
    const Vector<uint8_t> palette = { 255, 255, 255, 0, 0, 255 };
    AppendPngChunk_(_out, "PLTE", palette);
    const Vector<uint8_t> data(pZlib, pZlib + zlibSize);
    STBIW_FREE(pZlib);
    AppendPngChunk_(_out, "IDAT", data);
    AppendPngChunk_(_out, "IEND", {});
    return true;
}

}   // namespace

Result<void> GroundMaskDocument::Load(
    const FilePath& _path)
{
    const Result<Vector<uint8_t>> file = ReadFileBytes(_path);
    if (!file || file.GetValue().empty())
    {
        return Error { eError::FileOpen };
    }
    const Vector<uint8_t>& encoded = file.GetValue();

    int      width    = 0;
    int      height   = 0;
    int      channels = 0;
    stbi_uc* pPixels  = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()), &width, &height, &channels, 4);
    if (pPixels == nullptr || width <= 0 || height <= 0)
    {
        return Error { eError::Parse };
    }
    Create(width, height);
    for (size_t i = 0; i < m_pixels.size(); ++i)
    {
        const stbi_uc* p = pPixels + i * 4;
        m_pixels[i]      = (p[0] < 80 && p[1] < 80 && p[2] >= 200 && p[3] >= 128) ? 1 : 0;
    }
    stbi_image_free(pPixels);
    return Error { kOK };
}

Result<void> GroundMaskDocument::Save(
    const FilePath& _path)
{
    Vector<uint8_t> png;
    if (m_width <= 0 || !EncodePalettePng_(m_pixels, m_width, m_height, png))
    {
        return Error { eError::InvalidData };
    }
    const Result<void> result = WriteFileBytes(_path, png);
    if (!result)
    {
        return result;
    }
    m_savedApplied = m_applied;
    m_bSavedLost   = false;
    return Error { kOK };
}

void GroundMaskDocument::Create(
    int _width,
    int _height)
{
    m_width  = Max(_width, 0);
    m_height = Max(_height, 0);
    m_pixels.assign(static_cast<size_t>(m_width) * static_cast<size_t>(m_height), 0);
    m_history.clear();
    m_stroke.clear();
    m_applied      = 0;
    m_savedApplied = 0;
    m_bSavedLost   = false;
    m_bInStroke    = false;
    m_dirtyX0      = 0;
    m_dirtyX1      = m_width - 1;
}

int GroundMaskDocument::GetWidth() const
{
    return m_width;
}

int GroundMaskDocument::GetHeight() const
{
    return m_height;
}

bool GroundMaskDocument::IsInside(
    int _x,
    int _y) const
{
    return _x >= 0 && _y >= 0 && _x < m_width && _y < m_height;
}

bool GroundMaskDocument::IsSolid(
    int _x,
    int _y) const
{
    return IsInside(_x, _y) && m_pixels[static_cast<size_t>(_y) * static_cast<size_t>(m_width) + static_cast<size_t>(_x)] != 0;
}

bool GroundMaskDocument::IsDirty() const
{
    return m_bSavedLost || m_applied != m_savedApplied;
}

bool GroundMaskDocument::CanUndo() const
{
    return m_applied > 0;
}

bool GroundMaskDocument::CanRedo() const
{
    return m_applied < m_history.size();
}

const Vector<uint8_t>& GroundMaskDocument::GetPixels() const
{
    return m_pixels;
}

void GroundMaskDocument::BeginStroke()
{
    m_bInStroke = true;
    m_stroke.clear();
}

void GroundMaskDocument::EndStroke()
{
    m_bInStroke = false;
    if (m_stroke.empty())
    {
        return;
    }
    if (m_savedApplied > m_applied)
    {
        m_bSavedLost = true;
    }
    m_history.resize(m_applied);
    m_history.push_back(m_stroke);
    m_stroke.clear();
    m_applied = m_history.size();
}

void GroundMaskDocument::Stamp(
    int  _x,
    int  _y,
    int  _brush,
    bool _bSolid)
{
    const bool bOwnStroke = !m_bInStroke;
    if (bOwnStroke)
    {
        BeginStroke();
    }
    const int size = Max(_brush, 1);
    const int left = _x - size / 2;
    const int top  = _y - size / 2;
    for (int y = top; y < top + size; ++y)
    {
        for (int x = left; x < left + size; ++x)
        {
            SetPixel_(x, y, _bSolid ? 1 : 0);
        }
    }
    if (bOwnStroke)
    {
        EndStroke();
    }
}

void GroundMaskDocument::DrawLine(
    int  _x0,
    int  _y0,
    int  _x1,
    int  _y1,
    int  _brush,
    bool _bSolid)
{
    const bool bOwnStroke = !m_bInStroke;
    if (bOwnStroke)
    {
        BeginStroke();
    }
    const int dx    = Abs(_x1 - _x0);
    const int dy    = -Abs(_y1 - _y0);
    const int stepX = _x0 < _x1 ? 1 : -1;
    const int stepY = _y0 < _y1 ? 1 : -1;
    int       error = dx + dy;
    int       x     = _x0;
    int       y     = _y0;
    while (true)
    {
        Stamp(x, y, _brush, _bSolid);
        if (x == _x1 && y == _y1)
        {
            break;
        }
        const int doubled = error * 2;
        if (doubled >= dy)
        {
            error += dy;
            x += stepX;
        }
        if (doubled <= dx)
        {
            error += dx;
            y += stepY;
        }
    }
    if (bOwnStroke)
    {
        EndStroke();
    }
}

void GroundMaskDocument::FillRect(
    int  _x0,
    int  _y0,
    int  _x1,
    int  _y1,
    bool _bSolid)
{
    const bool bOwnStroke = !m_bInStroke;
    if (bOwnStroke)
    {
        BeginStroke();
    }
    for (int y = Max(Min(_y0, _y1), 0); y <= Min(Max(_y0, _y1), m_height - 1); ++y)
    {
        for (int x = Max(Min(_x0, _x1), 0); x <= Min(Max(_x0, _x1), m_width - 1); ++x)
        {
            SetPixel_(x, y, _bSolid ? 1 : 0);
        }
    }
    if (bOwnStroke)
    {
        EndStroke();
    }
}

void GroundMaskDocument::FloodFill(
    int  _x,
    int  _y,
    bool _bSolid)
{
    if (!IsInside(_x, _y))
    {
        return;
    }
    const uint8_t target = IsSolid(_x, _y) ? 1 : 0;
    const uint8_t value  = _bSolid ? 1 : 0;
    if (target == value)
    {
        return;
    }
    const bool bOwnStroke = !m_bInStroke;
    if (bOwnStroke)
    {
        BeginStroke();
    }
    Vector<int> pending;
    pending.push_back(_x);
    pending.push_back(_y);
    while (!pending.empty())
    {
        const int y = pending.back();
        pending.pop_back();
        const int x = pending.back();
        pending.pop_back();
        if (!IsInside(x, y) || m_pixels[static_cast<size_t>(y) * static_cast<size_t>(m_width) + static_cast<size_t>(x)] != target)
        {
            continue;
        }
        SetPixel_(x, y, value);
        pending.push_back(x + 1);
        pending.push_back(y);
        pending.push_back(x - 1);
        pending.push_back(y);
        pending.push_back(x);
        pending.push_back(y + 1);
        pending.push_back(x);
        pending.push_back(y - 1);
    }
    if (bOwnStroke)
    {
        EndStroke();
    }
}

void GroundMaskDocument::Undo()
{
    if (m_applied == 0)
    {
        return;
    }
    --m_applied;
    for (const GroundPixelChange& c: m_history[m_applied])
    {
        m_pixels[c.index] = c.before;
        MarkColumn_(static_cast<int>(c.index % static_cast<uint32_t>(m_width)));
    }
}

void GroundMaskDocument::Redo()
{
    if (m_applied >= m_history.size())
    {
        return;
    }
    for (const GroundPixelChange& c: m_history[m_applied])
    {
        m_pixels[c.index] = c.after;
        MarkColumn_(static_cast<int>(c.index % static_cast<uint32_t>(m_width)));
    }
    ++m_applied;
}

bool GroundMaskDocument::TakeDirtyColumns(
    int& _x0,
    int& _x1)
{
    if (m_dirtyX1 < m_dirtyX0)
    {
        return false;
    }
    _x0       = m_dirtyX0;
    _x1       = m_dirtyX1;
    m_dirtyX0 = 0;
    m_dirtyX1 = -1;
    return true;
}

void GroundMaskDocument::SetPixel_(
    int     _x,
    int     _y,
    uint8_t _value)
{
    if (!IsInside(_x, _y))
    {
        return;
    }
    const uint32_t index = static_cast<uint32_t>(_y) * static_cast<uint32_t>(m_width) + static_cast<uint32_t>(_x);
    if (m_pixels[index] == _value)
    {
        return;
    }
    m_stroke.push_back({ index, m_pixels[index], _value });
    m_pixels[index] = _value;
    MarkColumn_(_x);
}

void GroundMaskDocument::MarkColumn_(
    int _x)
{
    if (m_dirtyX1 < m_dirtyX0)
    {
        m_dirtyX0 = _x;
        m_dirtyX1 = _x;
        return;
    }
    m_dirtyX0 = Min(m_dirtyX0, _x);
    m_dirtyX1 = Max(m_dirtyX1, _x);
}

void GroundEditor::Init(
    SDL_Renderer* _pRenderer)
{
    m_pRenderer = _pRenderer;
    m_names.clear();
    std::error_code ec;
    for (const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator(FindAssetRoot() / "Ground", ec))
    {
        if (entry.path().extension() == ".png")
        {
            m_names.push_back(entry.path().stem().string());
        }
    }
    std::sort(m_names.begin(), m_names.end());
}

void GroundEditor::Shutdown()
{
    ClearTextures_();
    m_pRenderer = nullptr;
}

void GroundEditor::ClearTextures_()
{
    for (MaskStrip& s: m_maskStrips)
    {
        SDL_DestroyTexture(s.pTex);
    }
    for (MaskStrip& s: m_bgStrips)
    {
        SDL_DestroyTexture(s.pTex);
    }
    m_maskStrips.clear();
    m_bgStrips.clear();
    m_bBgLoaded = false;
}

void GroundEditor::LoadMask_(
    const String& _name)
{
    const FilePath path = FindAssetRoot() / "Ground" / (_name + ".png");
    ClearTextures_();
    const Result<void> result = m_doc.Load(path);
    if (!result)
    {
        m_status = "load FAILED: " + path.string();
        m_name.clear();
        return;
    }
    m_name    = _name;
    m_scrollX = 0.f;
    m_scrollY = 0.f;
    for (int x = 0; x < m_doc.GetWidth(); x += kGroundStripWidth)
    {
        const int    sw   = Min(kGroundStripWidth, m_doc.GetWidth() - x);
        SDL_Texture* pTex = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, sw, m_doc.GetHeight());
        if (pTex == nullptr)
        {
            continue;
        }
        SDL_SetTextureBlendMode(pTex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(pTex, SDL_SCALEMODE_NEAREST);
        m_maskStrips.push_back({ pTex, x, sw });
    }
    int x0 = 0;
    int x1 = 0;
    if (m_doc.TakeDirtyColumns(x0, x1))
    {
        UploadMaskColumns_(x0, x1);
    }
    m_status = "loaded " + path.string();
    if (m_bShowBg)
    {
        LoadBackground_(_name);
    }
}

void GroundEditor::LoadBackground_(
    const String& _maskName)
{
    m_bBgLoaded                              = true;
    const FilePath                 stagePath = FindAssetRoot() / "Stage" / (_maskName.substr(0, _maskName.find('_')) + ".json");
    const Result<JsonDeserializer> stage     = LoadJsonFile(stagePath);
    if (!stage)
    {
        return;
    }
    const Result<JsonReader> world = stage->GetReader().FindField("world");
    if (!world)
    {
        return;
    }
    const Result<String>           background = world->ReadField<String>("background");
    const Result<JsonDeserializer> sprites    = LoadJsonFile(FindAssetRoot() / "sprites.json");
    if (!background || !sprites)
    {
        return;
    }
    const Result<JsonReader> all = sprites->GetReader().FindField("sprites");
    if (!all)
    {
        return;
    }
    const Result<JsonReader> sprite = all->FindField(background.GetValue());
    if (!sprite)
    {
        return;
    }
    const Result<String> file = sprite->ReadField<String>("file");
    if (!file)
    {
        return;
    }
    int      w      = 0;
    int      h      = 0;
    int      ch     = 0;
    stbi_uc* pPixel = stbi_load((FindAssetRoot() / file.GetValue()).string().c_str(), &w, &h, &ch, 4);
    if (pPixel == nullptr)
    {
        return;
    }
    m_bgH = h;
    for (int x = 0; x < w; x += kGroundStripWidth)
    {
        const int    sw   = Min(kGroundStripWidth, w - x);
        SDL_Texture* pTex = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, sw, h);
        if (pTex == nullptr)
        {
            continue;
        }
        SDL_UpdateTexture(pTex, nullptr, pPixel + static_cast<size_t>(x) * 4, w * 4);
        SDL_SetTextureBlendMode(pTex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(pTex, SDL_SCALEMODE_NEAREST);
        m_bgStrips.push_back({ pTex, x, sw });
    }
    stbi_image_free(pPixel);
}

void GroundEditor::UploadMaskColumns_(
    int _x0,
    int _x1)
{
    const int       w = m_doc.GetWidth();
    const int       h = m_doc.GetHeight();
    Vector<uint8_t> rgba;
    for (const MaskStrip& s: m_maskStrips)
    {
        if (s.x + s.w <= _x0 || s.x > _x1)
        {
            continue;
        }
        rgba.assign(static_cast<size_t>(s.w) * static_cast<size_t>(h) * 4, 0);
        for (int y = 0; y < h; ++y)
        {
            for (int col = 0; col < s.w; ++col)
            {
                if (m_doc.GetPixels()[static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(s.x + col)] != 0)
                {
                    uint8_t* p = &rgba[(static_cast<size_t>(y) * static_cast<size_t>(s.w) + static_cast<size_t>(col)) * 4];
                    p[0]       = 255;
                    p[1]       = 40;
                    p[2]       = 90;
                    p[3]       = 255;
                }
            }
        }
        SDL_UpdateTexture(s.pTex, nullptr, rgba.data(), s.w * 4);
    }
}

void GroundEditor::Save_()
{
    if (m_name.empty())
    {
        return;
    }
    const FilePath     path   = FindAssetRoot() / "Ground" / (m_name + ".png");
    const Result<void> result = m_doc.Save(path);
    m_status                  = result ? "saved " + path.string() : "save FAILED: " + path.string();
}

void GroundEditor::Draw()
{
    if (m_name.empty() && m_status.empty() && !m_names.empty())
    {
        LoadMask_(m_names.front());
    }
    ImGui::TextUnformatted("Collision mask editor: see the \"Ground Mask\" window.");
    ImGui::TextWrapped("%s%s", m_doc.IsDirty() ? "[modified] " : "", m_status.c_str());

    const ImGuiIO&       io    = ImGui::GetIO();
    const ImGuiViewport* pHost = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2 { pHost->Pos.x + 344.f, pHost->Pos.y + 40.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2 { Max(io.DisplaySize.x - 360.f, 480.f), Max(io.DisplaySize.y - 56.f, 320.f) }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Ground Mask"))
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput)
        {
            HandleKeys_();
        }
        ImGui::BeginChild("Panel", ImVec2 { 220.f, 0.f }, ImGuiChildFlags_Borders);
        DrawPanel_();
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("View", ImVec2 { 0.f, 0.f }, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        DrawView_();
        ImGui::EndChild();
    }
    ImGui::End();
}

void GroundEditor::HandleKeys_()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_S, false))
        {
            Save_();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Z, true))
        {
            m_doc.Undo();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Y, true))
        {
            m_doc.Redo();
        }
        return;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_B, false))
    {
        m_tool = eGroundTool::Pencil;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_E, false))
    {
        m_tool = eGroundTool::Eraser;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_R, false))
    {
        m_tool = eGroundTool::Rect;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_L, false))
    {
        m_tool = eGroundTool::Line;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_G, false))
    {
        m_tool = eGroundTool::Fill;
    }
}

void GroundEditor::DrawPanel_()
{
    int index = 0;
    for (int i = 0; i < static_cast<int>(m_names.size()); ++i)
    {
        if (m_names[static_cast<size_t>(i)] == m_name)
        {
            index = i;
        }
    }
    ImGui::BeginDisabled(m_doc.IsDirty());
    const char* preview = m_names.empty() ? "-" : m_names[static_cast<size_t>(index)].c_str();
    if (ImGui::BeginCombo("Mask", preview))
    {
        for (int i = 0; i < static_cast<int>(m_names.size()); ++i)
        {
            if (ImGui::Selectable(m_names[static_cast<size_t>(i)].c_str(), i == index))
            {
                LoadMask_(m_names[static_cast<size_t>(i)]);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
    if (m_doc.IsDirty())
    {
        ImGui::TextColored(ImVec4 { 1.f, 0.8f, 0.3f, 1.f }, "modified: save to switch");
    }
    if (ImGui::Button("Save (Ctrl+S)"))
    {
        Save_();
    }
    ImGui::BeginDisabled(!m_doc.CanUndo());
    if (ImGui::Button("Undo (Ctrl+Z)"))
    {
        m_doc.Undo();
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!m_doc.CanRedo());
    if (ImGui::Button("Redo (Ctrl+Y)"))
    {
        m_doc.Redo();
    }
    ImGui::EndDisabled();
    ImGui::Separator();

    ImGui::TextUnformatted("Tool");
    ImGui::RadioButton("Pencil (B)", reinterpret_cast<int*>(&m_tool), static_cast<int>(eGroundTool::Pencil));
    ImGui::RadioButton("Eraser (E)", reinterpret_cast<int*>(&m_tool), static_cast<int>(eGroundTool::Eraser));
    ImGui::RadioButton("Rect (R)", reinterpret_cast<int*>(&m_tool), static_cast<int>(eGroundTool::Rect));
    ImGui::RadioButton("Line (L)", reinterpret_cast<int*>(&m_tool), static_cast<int>(eGroundTool::Line));
    ImGui::RadioButton("Fill (G)", reinterpret_cast<int*>(&m_tool), static_cast<int>(eGroundTool::Fill));
    ImGui::SliderInt("Brush", &m_brush, 1, kGroundMaxBrush);
    ImGui::TextWrapped("Left: draw. Right: erase. Shift+click: line from last point. Mid-drag: pan. Wheel: zoom. Shift+wheel: scroll.");
    ImGui::Separator();

    ImGui::Checkbox("Mask", &m_bShowMask);
    if (ImGui::Checkbox("Background", &m_bShowBg) && m_bShowBg && !m_bBgLoaded && !m_name.empty())
    {
        LoadBackground_(m_name);
    }
    ImGui::SliderFloat("Zoom", &m_zoom, kGroundMinZoom, kGroundMaxZoom, "%.2f", ImGuiSliderFlags_Logarithmic);
}

void GroundEditor::DrawView_()
{
    const ImVec2 canvasPos  = ImGui::GetCursorScreenPos();
    const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 32.f || canvasSize.y < 32.f || m_doc.GetWidth() <= 0)
    {
        return;
    }
    const float readoutHeight = ImGui::GetTextLineHeightWithSpacing();
    ImGui::InvisibleButton("canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
    const bool     bHovered = ImGui::IsItemHovered();
    const ImGuiIO& io       = ImGui::GetIO();
    const float    originX  = canvasPos.x;
    const float    originY  = canvasPos.y + readoutHeight;
    if (bHovered && io.MouseWheel != 0.f)
    {
        if (io.KeyShift)
        {
            m_scrollX -= io.MouseWheel * 200.f / m_zoom;
        }
        else
        {
            const float worldX = m_scrollX + (io.MousePos.x - originX) / m_zoom;
            const float worldY = m_scrollY + (io.MousePos.y - originY) / m_zoom;
            m_zoom             = Clamp(m_zoom * (io.MouseWheel > 0.f ? 1.25f : 0.8f), kGroundMinZoom, kGroundMaxZoom);
            m_scrollX          = worldX - (io.MousePos.x - originX) / m_zoom;
            m_scrollY          = worldY - (io.MousePos.y - originY) / m_zoom;
        }
    }
    if (bHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f))
    {
        m_scrollX -= io.MouseDelta.x / m_zoom;
        m_scrollY -= io.MouseDelta.y / m_zoom;
    }
    const float worldW = static_cast<float>(m_doc.GetWidth());
    const float worldH = static_cast<float>(m_doc.GetHeight());
    const float viewW  = canvasSize.x / m_zoom;
    const float viewH  = (canvasSize.y - readoutHeight) / m_zoom;
    m_scrollX          = Clamp(m_scrollX, -64.f / m_zoom, Max(worldW - viewW, 0.f) + 64.f / m_zoom);
    m_scrollY          = Clamp(m_scrollY, -64.f / m_zoom, Max(worldH - viewH, 0.f) + 64.f / m_zoom);
    const float zoom   = m_zoom;

    const int pixelX = static_cast<int>(Floor(m_scrollX + (io.MousePos.x - originX) / zoom));
    const int pixelY = static_cast<int>(Floor(m_scrollY + (io.MousePos.y - originY) / zoom));
    HandleCanvasInput_(pixelX, pixelY, bHovered);

    int dirtyX0 = 0;
    int dirtyX1 = 0;
    if (m_doc.TakeDirtyColumns(dirtyX0, dirtyX1))
    {
        UploadMaskColumns_(dirtyX0, dirtyX1);
    }

    const float sx    = m_scrollX;
    const float sy    = m_scrollY;
    ImDrawList* pDraw = ImGui::GetWindowDrawList();
    pDraw->PushClipRect(canvasPos, ImVec2 { canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y }, true);
    pDraw->AddRectFilled(canvasPos, ImVec2 { canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y }, IM_COL32(24, 26, 32, 255));
    const ImVec2 w0 { std::round(originX - sx * zoom), std::round(originY - sy * zoom) };
    const ImVec2 w1 { std::round(originX + (worldW - sx) * zoom), std::round(originY + (worldH - sy) * zoom) };
    pDraw->AddRectFilled(w0, w1, IM_COL32(48, 52, 62, 255));
    if (m_bShowBg)
    {
        for (const MaskStrip& s: m_bgStrips)
        {
            const ImVec2 a { std::round(originX + (static_cast<float>(s.x) - sx) * zoom), w0.y };
            const ImVec2 b { std::round(originX + (static_cast<float>(s.x + s.w) - sx) * zoom), std::round(originY + (static_cast<float>(m_bgH) - sy) * zoom) };
            pDraw->AddImage(reinterpret_cast<ImTextureID>(s.pTex), a, b);
        }
    }
    if (m_bShowMask)
    {
        for (const MaskStrip& s: m_maskStrips)
        {
            const ImVec2 a { std::round(originX + (static_cast<float>(s.x) - sx) * zoom), w0.y };
            const ImVec2 b { std::round(originX + (static_cast<float>(s.x + s.w) - sx) * zoom), w1.y };
            pDraw->AddImage(reinterpret_cast<ImTextureID>(s.pTex), a, b, ImVec2 { 0.f, 0.f }, ImVec2 { 1.f, 1.f }, IM_COL32(255, 255, 255, 150));
        }
    }
    pDraw->AddRect(w0, w1, IM_COL32(200, 200, 90, 255));
    if (zoom >= kGroundGridZoom)
    {
        const int firstX = Max(0, static_cast<int>(Floor(sx)));
        const int lastX  = Min(m_doc.GetWidth(), static_cast<int>(Ceil(sx + viewW)));
        for (int x = firstX; x <= lastX; ++x)
        {
            const float px = std::round(originX + (static_cast<float>(x) - sx) * zoom);
            pDraw->AddLine(ImVec2 { px, canvasPos.y }, ImVec2 { px, canvasPos.y + canvasSize.y }, IM_COL32(255, 255, 255, 40));
        }
        const int firstY = Max(0, static_cast<int>(Floor(sy)));
        const int lastY  = Min(m_doc.GetHeight(), static_cast<int>(Ceil(sy + viewH)));
        for (int y = firstY; y <= lastY; ++y)
        {
            const float py = std::round(originY + (static_cast<float>(y) - sy) * zoom);
            pDraw->AddLine(ImVec2 { canvasPos.x, py }, ImVec2 { canvasPos.x + canvasSize.x, py }, IM_COL32(255, 255, 255, 40));
        }
    }

    if (bHovered || m_bDragging)
    {
        const bool bBrushTool = m_tool == eGroundTool::Pencil || m_tool == eGroundTool::Eraser || m_tool == eGroundTool::Line;
        const int  brush      = bBrushTool ? m_brush : 1;
        const int  left       = pixelX - brush / 2;
        const int  top        = pixelY - brush / 2;
        pDraw->AddRect(ImVec2 { originX + (static_cast<float>(left) - sx) * zoom, originY + (static_cast<float>(top) - sy) * zoom },
                       ImVec2 { originX + (static_cast<float>(left + brush) - sx) * zoom, originY + (static_cast<float>(top + brush) - sy) * zoom },
                       IM_COL32(255, 255, 0, 255));
        if (m_bDragging && m_tool == eGroundTool::Rect)
        {
            const int x0 = Min(m_startX, pixelX);
            const int x1 = Max(m_startX, pixelX);
            const int y0 = Min(m_startY, pixelY);
            const int y1 = Max(m_startY, pixelY);
            pDraw->AddRect(ImVec2 { originX + (static_cast<float>(x0) - sx) * zoom, originY + (static_cast<float>(y0) - sy) * zoom },
                           ImVec2 { originX + (static_cast<float>(x1 + 1) - sx) * zoom, originY + (static_cast<float>(y1 + 1) - sy) * zoom },
                           IM_COL32(255, 255, 0, 255),
                           0.f,
                           0,
                           2.f);
        }
        if (m_bDragging && m_tool == eGroundTool::Line)
        {
            pDraw->AddLine(ImVec2 { originX + (static_cast<float>(m_startX) + 0.5f - sx) * zoom, originY + (static_cast<float>(m_startY) + 0.5f - sy) * zoom },
                           ImVec2 { originX + (static_cast<float>(pixelX) + 0.5f - sx) * zoom, originY + (static_cast<float>(pixelY) + 0.5f - sy) * zoom },
                           IM_COL32(255, 255, 0, 255),
                           Max(static_cast<float>(m_brush) * zoom, 1.f));
        }
    }

    const bool   bInside = m_doc.IsInside(pixelX, pixelY);
    const String readout = bInside ? std::format("x={} y={} {}", pixelX, pixelY, m_doc.IsSolid(pixelX, pixelY) ? "solid" : "air")
                                   : String { "x=- y=- -" };
    const String text    = std::format("{}   zoom {:.2f}   {}x{}", readout, static_cast<double>(zoom), m_doc.GetWidth(), m_doc.GetHeight());
    pDraw->AddRectFilled(canvasPos, ImVec2 { canvasPos.x + canvasSize.x, canvasPos.y + readoutHeight }, IM_COL32(16, 16, 20, 255));
    pDraw->AddText(ImVec2 { canvasPos.x + 4.f, canvasPos.y + 1.f }, IM_COL32(230, 230, 230, 255), text.c_str());
    pDraw->PopClipRect();
}

void GroundEditor::HandleCanvasInput_(
    int  _pixelX,
    int  _pixelY,
    bool _bHovered)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (!m_bDragging)
    {
        if (!_bHovered)
        {
            return;
        }
        const bool bLeft  = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        const bool bRight = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
        if (!bLeft && !bRight)
        {
            return;
        }
        m_bDragSolid = bLeft && m_tool != eGroundTool::Eraser;
        m_startX     = _pixelX;
        m_startY     = _pixelY;
        if (m_tool == eGroundTool::Fill)
        {
            m_doc.FloodFill(_pixelX, _pixelY, m_bDragSolid);
            return;
        }
        m_bDragging = true;
        if (m_tool == eGroundTool::Pencil || m_tool == eGroundTool::Eraser)
        {
            m_doc.BeginStroke();
            if (io.KeyShift && m_bHasLast)
            {
                m_doc.DrawLine(m_lastX, m_lastY, _pixelX, _pixelY, m_brush, m_bDragSolid);
            }
            else
            {
                m_doc.Stamp(_pixelX, _pixelY, m_brush, m_bDragSolid);
            }
            m_lastX    = _pixelX;
            m_lastY    = _pixelY;
            m_bHasLast = true;
        }
        return;
    }
    const bool bHeld = ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right);
    if (m_tool == eGroundTool::Pencil || m_tool == eGroundTool::Eraser)
    {
        if (_pixelX != m_lastX || _pixelY != m_lastY)
        {
            m_doc.DrawLine(m_lastX, m_lastY, _pixelX, _pixelY, m_brush, m_bDragSolid);
            m_lastX = _pixelX;
            m_lastY = _pixelY;
        }
        if (!bHeld)
        {
            m_doc.EndStroke();
            m_bDragging = false;
        }
        return;
    }
    if (!bHeld)
    {
        CommitDrag_(_pixelX, _pixelY);
        m_bDragging = false;
    }
}

void GroundEditor::CommitDrag_(
    int _pixelX,
    int _pixelY)
{
    if (m_tool == eGroundTool::Rect)
    {
        m_doc.FillRect(m_startX, m_startY, _pixelX, _pixelY, m_bDragSolid);
    }
    else if (m_tool == eGroundTool::Line)
    {
        m_doc.DrawLine(m_startX, m_startY, _pixelX, _pixelY, m_brush, m_bDragSolid);
        m_lastX    = _pixelX;
        m_lastY    = _pixelY;
        m_bHasLast = true;
    }
}

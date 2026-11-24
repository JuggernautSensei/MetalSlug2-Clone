#include "Editor/AnimEditor.h"

#include <algorithm>
#include <charconv>
#include <format>

#include <SDL3/SDL.h>
#include <imgui.h>
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Base/JsonFields.h"
#include "Core/AssetPath.h"

void AnimEditor::Init(
    SDL_Renderer* _pRenderer)
{
    m_pRenderer = _pRenderer;
    Open(FindAssetRoot() / "anim.json");
}

void AnimEditor::Open(
    const FilePath& _path)
{
    m_path                    = _path;
    const Result<void> result = m_doc.Load(m_path);
    m_bLoaded                 = static_cast<bool>(result);
    m_status                  = m_bLoaded ? "loaded " + m_path.string() : "anim.json load failed";
    m_syncedRevision          = m_doc.GetRevision();
    RememberMtime_();
}

void AnimEditor::SetLibrary(
    AnimationLibrary* _pLibrary)
{
    m_pLibrary = _pLibrary;
}

AnimDocument& AnimEditor::GetDocument()
{
    return m_doc;
}

const String& AnimEditor::GetStatus() const
{
    return m_status;
}

void AnimEditor::RememberMtime_()
{
    std::error_code ec;
    m_mtime = std::filesystem::last_write_time(m_path, ec);
}

void AnimEditor::SyncLibrary_()
{
    if (m_doc.GetRevision() == m_syncedRevision)
    {
        return;
    }
    m_syncedRevision = m_doc.GetRevision();
    if (m_pLibrary == nullptr)
    {
        return;
    }
    const int clip = m_doc.GetLastChangedClip();
    if (clip >= 0)
    {
        m_doc.ApplyClipTo(clip, *m_pLibrary);
    }
    else
    {
        m_doc.ApplyTo(*m_pLibrary);
    }
}

void AnimEditor::Poll()
{
    constexpr int kPollFrames = 60;

    SyncLibrary_();
    if (++m_pollFrames >= kPollFrames)
    {
        m_pollFrames = 0;
        (void)CheckExternal();
    }
}

bool AnimEditor::CheckExternal()
{
    std::error_code                       ec;
    const std::filesystem::file_time_type mtime = std::filesystem::last_write_time(m_path, ec);
    if (ec || mtime == m_mtime)
    {
        return false;
    }
    if (m_doc.IsDirty())
    {
        m_mtime  = mtime;
        m_status = "anim.json changed on disk, unsaved edits kept (Reload discards them)";
        return false;
    }
    Reload();
    return m_bLoaded;
}

void AnimEditor::Reload()
{
    String name;
    if (m_selected >= 0 && m_selected < static_cast<int>(m_doc.GetClips().size()))
    {
        name = m_doc.GetClips()[static_cast<size_t>(m_selected)].name;
    }
    RememberMtime_();
    const Result<void> result = m_doc.Load(m_path);
    m_syncedRevision          = m_doc.GetRevision();
    if (!result)
    {
        m_status = "reload FAILED: " + m_path.string();
        return;
    }
    m_bLoaded = true;
    if (m_pLibrary != nullptr)
    {
        m_doc.ApplyTo(*m_pLibrary);
    }
    m_selected = name.empty() ? -1 : m_doc.FindClip(name);
    m_status   = "reloaded anim.json";
}

void AnimEditor::Shutdown()
{
    for (SDL_Texture* pPage: m_pages)
    {
        if (pPage != nullptr)
        {
            SDL_DestroyTexture(pPage);
        }
    }
    m_pages.clear();
    m_pageW.clear();
    m_pageH.clear();
    m_pRenderer = nullptr;
}

void AnimEditor::SelectClip(
    const StringView _name)
{
    const int index = m_doc.FindClip(_name);
    if (index >= 0)
    {
        m_selected = index;
        m_frame    = 0;
    }
}

SDL_Texture* AnimEditor::GetPage_(
    const int _atlas)
{
    if (_atlas < 0 || _atlas > 255 || m_pRenderer == nullptr)
    {
        return nullptr;
    }
    const size_t index = static_cast<size_t>(_atlas);
    if (m_pages.size() <= index)
    {
        m_pages.resize(index + 1, nullptr);
        m_pageW.resize(index + 1, 0);
        m_pageH.resize(index + 1, 0);
    }
    if (m_pages[index] != nullptr)
    {
        return m_pages[index];
    }

    const FilePath path   = FindAssetRoot() / "Atlas" / std::format("atlas_{:03}.png", _atlas);
    int            w      = 0;
    int            h      = 0;
    int            ch     = 0;
    stbi_uc*       pPixel = stbi_load(path.string().c_str(), &w, &h, &ch, 4);
    if (pPixel == nullptr)
    {
        return nullptr;
    }
    SDL_Texture* pTex = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (pTex != nullptr)
    {
        SDL_UpdateTexture(pTex, nullptr, pPixel, w * 4);
        SDL_SetTextureBlendMode(pTex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(pTex, SDL_SCALEMODE_NEAREST);
        m_pages[index] = pTex;
        m_pageW[index] = w;
        m_pageH[index] = h;
    }
    stbi_image_free(pPixel);
    return pTex;
}

void AnimEditor::Save_()
{
    const Result<void> result = m_doc.Save(m_path);
    if (result)
    {
        m_doc.ClearDirty();
        RememberMtime_();
        m_status = "saved " + m_path.string();
    }
    else
    {
        m_status = "save FAILED: " + m_path.string();
    }
}

void AnimEditor::Draw()
{
    const ImGuiViewport* pViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2 { pViewport->Pos.x + 350.f, pViewport->Pos.y + 40.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2 { pViewport->Size.x - 366.f, pViewport->Size.y - 56.f }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Animation Offsets"))
    {
        if (!m_bLoaded)
        {
            ImGui::TextUnformatted("anim.json load failed");
        }
        else
        {
            ImGui::BeginChild("AnimList", ImVec2 { 260.f, 0.f }, ImGuiChildFlags_Borders);
            DrawList_();
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("AnimDetail", ImVec2 { 0.f, 0.f });
            DrawDetail_();
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void AnimEditor::DrawList_()
{
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##filter", "filter", m_filter.data(), m_filter.size());
    ImGui::Separator();
    ImGui::BeginChild("AnimNames");
    const Span<const AnimEditClip> clips = m_doc.GetClips();
    for (size_t i = 0; i < clips.size(); ++i)
    {
        if (m_filter[0] != '\0' && clips[i].name.find(m_filter.data()) == String::npos)
        {
            continue;
        }
        if (ImGui::Selectable(clips[i].name.c_str(), static_cast<int>(i) == m_selected))
        {
            m_selected  = static_cast<int>(i);
            m_frame     = 0;
            m_bPlaying  = false;
            m_playTimer = 0.f;
        }
    }
    ImGui::EndChild();
}

void AnimEditor::DrawDetail_()
{
    if (ImGui::Button("Save (Ctrl+S)"))
    {
        Save_();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
    {
        Reload();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_doc.CanUndo());
    if (ImGui::Button("Undo (Ctrl+Z)"))
    {
        m_doc.Undo();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s%s", m_doc.IsDirty() ? "[modified] " : "", m_status.c_str());

    if (m_selected < 0 || m_selected >= static_cast<int>(m_doc.GetClips().size()))
    {
        ImGui::TextUnformatted("select an animation on the left");
        return;
    }
    const int           clipIndex  = m_selected;
    const AnimEditClip& clip       = m_doc.GetClips()[static_cast<size_t>(clipIndex)];
    const int           frameCount = static_cast<int>(clip.frames.size());
    m_frame                        = Clamp(m_frame, 0, Max(frameCount - 1, 0));

    ImGui::Separator();
    ImGui::TextUnformatted(clip.name.c_str());

    bool bLoop = clip.bLoop;
    if (ImGui::Checkbox("loop", &bLoop))
    {
        m_doc.SetLoop(clipIndex, bLoop);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.f);
    int loopStart = clip.loopStart;
    if (ImGui::InputInt("loopStart", &loopStart) && loopStart >= 0 && loopStart < Max(frameCount, 1))
    {
        m_doc.SetLoopStart(clipIndex, loopStart);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(130.f);
    float duration = static_cast<float>(clip.frameDuration);
    if (ImGui::InputFloat("frameDuration", &duration, 0.005f, 0.01f, "%.4f") && duration > 0.f)
    {
        m_doc.SetFrameDuration(clipIndex, static_cast<double>(duration));
    }

    if (ImGui::Button(m_bPlaying ? "Pause" : "Play"))
    {
        m_bPlaying  = !m_bPlaying;
        m_playTimer = 0.f;
    }
    ImGui::SameLine();
    if (ImGui::Button("<") && frameCount > 0)
    {
        m_bPlaying = false;
        m_frame    = (m_frame + frameCount - 1) % frameCount;
    }
    ImGui::SameLine();
    if (ImGui::Button(">") && frameCount > 0)
    {
        m_bPlaying = false;
        m_frame    = (m_frame + 1) % frameCount;
    }
    ImGui::SameLine();
    ImGui::Text("frame %d / %d", m_frame, frameCount);
    ImGui::SameLine();
    ImGui::Checkbox("onion", &m_bOnion);
    ImGui::SameLine();
    ImGui::Text("zoom %dx (wheel)", m_zoom);

    if (m_bPlaying && frameCount > 0)
    {
        m_playTimer += ImGui::GetIO().DeltaTime;
        const float step = static_cast<float>(Max(0.001, clip.frameDuration));
        while (m_playTimer >= step)
        {
            m_playTimer -= step;
            if (m_frame + 1 < frameCount)
            {
                ++m_frame;
            }
            else if (clip.bLoop)
            {
                m_frame = Clamp(clip.loopStart, 0, frameCount - 1);
            }
            else
            {
                m_bPlaying = false;
            }
        }
    }

    if (frameCount > 0)
    {
        const AnimFrame& cur    = clip.frames[static_cast<size_t>(m_frame)];
        int              pivotX = cur.pivotX;
        int              pivotY = cur.pivotY;
        bool             bEdit  = false;
        ImGui::SetNextItemWidth(110.f);
        bEdit |= ImGui::InputInt("pivotX", &pivotX);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.f);
        bEdit |= ImGui::InputInt("pivotY", &pivotY);
        if (bEdit)
        {
            m_doc.SetPivot(clipIndex, m_frame, pivotX, pivotY);
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply to all frames"))
        {
            m_doc.ApplyPivotToAll(clipIndex, cur.pivotX, cur.pivotY);
        }
        ImGui::TextUnformatted("Arrow keys: move sprite 1px (Left = pivotX+1, Up = pivotY+1)");
    }

    const AnimEditClip& view = m_doc.GetClips()[static_cast<size_t>(clipIndex)];
    ImGui::BeginChild("FrameList", ImVec2 { 0.f, 130.f }, ImGuiChildFlags_Borders);
    if (ImGui::BeginTable("frames", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("#");
        ImGui::TableSetupColumn("w");
        ImGui::TableSetupColumn("h");
        ImGui::TableSetupColumn("pivotX");
        ImGui::TableSetupColumn("pivotY");
        ImGui::TableHeadersRow();
        for (int i = 0; i < static_cast<int>(view.frames.size()); ++i)
        {
            const AnimFrame& fr = view.frames[static_cast<size_t>(i)];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const String label = std::format("{}", i);
            if (ImGui::Selectable(label.c_str(), i == m_frame, ImGuiSelectableFlags_SpanAllColumns))
            {
                m_frame    = i;
                m_bPlaying = false;
            }
            ImGui::TableNextColumn();
            ImGui::Text("%d", fr.w);
            ImGui::TableNextColumn();
            ImGui::Text("%d", fr.h);
            ImGui::TableNextColumn();
            ImGui::Text("%d", fr.pivotX);
            ImGui::TableNextColumn();
            ImGui::Text("%d", fr.pivotY);
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    DrawCanvas_(view);
    HandleKeys_(clipIndex);
}

void AnimEditor::DrawCanvas_(
    const AnimEditClip& _clip)
{
    const ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x < 16.f || size.y < 16.f)
    {
        return;
    }
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max = ImVec2 { min.x + size.x, min.y + size.y };
    ImGui::InvisibleButton("canvas", size);
    if (ImGui::IsItemHovered())
    {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel > 0.f)
        {
            m_zoom = Min(m_zoom + 1, 8);
        }
        else if (wheel < 0.f)
        {
            m_zoom = Max(m_zoom - 1, 1);
        }
    }

    ImDrawList* pDraw = ImGui::GetWindowDrawList();
    pDraw->PushClipRect(min, max, true);
    pDraw->AddRectFilled(min, max, IM_COL32(40, 40, 48, 255));

    const float originX = static_cast<float>(static_cast<int>(min.x + size.x * 0.5f));
    const float originY = static_cast<float>(static_cast<int>(min.y + size.y * 0.7f));
    const float zoom    = static_cast<float>(m_zoom);

    const int frameCount = static_cast<int>(_clip.frames.size());
    for (int pass = 0; pass < 2 && frameCount > 0; ++pass)
    {
        if (pass == 0 && (!m_bOnion || frameCount < 2))
        {
            continue;
        }
        const int        index = pass == 0 ? (m_frame + frameCount - 1) % frameCount : m_frame;
        const AnimFrame& fr    = _clip.frames[static_cast<size_t>(index)];
        if (fr.IsEmpty())
        {
            continue;
        }
        SDL_Texture* pPage = GetPage_(fr.atlas);
        if (pPage == nullptr)
        {
            continue;
        }
        const float  pw   = static_cast<float>(m_pageW[static_cast<size_t>(fr.atlas)]);
        const float  ph   = static_cast<float>(m_pageH[static_cast<size_t>(fr.atlas)]);
        const ImVec2 p0   = ImVec2 { originX - static_cast<float>(fr.pivotX) * zoom, originY - static_cast<float>(fr.pivotY) * zoom };
        const ImVec2 p1   = ImVec2 { p0.x + static_cast<float>(fr.w) * zoom, p0.y + static_cast<float>(fr.h) * zoom };
        const ImVec2 uv0  = ImVec2 { static_cast<float>(fr.x) / pw, static_cast<float>(fr.y) / ph };
        const ImVec2 uv1  = ImVec2 { static_cast<float>(fr.x + fr.w) / pw, static_cast<float>(fr.y + fr.h) / ph };
        const ImU32  tint = pass == 0 ? IM_COL32(255, 255, 255, 90) : IM_COL32(255, 255, 255, 255);
        pDraw->AddImage(reinterpret_cast<ImTextureID>(pPage), p0, p1, uv0, uv1, tint);
        if (pass == 1)
        {
            pDraw->AddRect(p0, p1, IM_COL32(80, 200, 255, 160));
        }
    }

    pDraw->AddLine(ImVec2 { min.x, originY }, ImVec2 { max.x, originY }, IM_COL32(255, 60, 60, 200));
    pDraw->AddLine(ImVec2 { originX, min.y }, ImVec2 { originX, max.y }, IM_COL32(255, 60, 60, 200));
    pDraw->AddCircle(ImVec2 { originX, originY }, 4.f, IM_COL32(255, 255, 0, 255));
    pDraw->PopClipRect();
}

void AnimEditor::HandleKeys_(
    const int _clipIndex)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
    {
        Save_();
        return;
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
    {
        m_doc.Undo();
        return;
    }
    if (io.WantTextInput || io.KeyCtrl || !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        return;
    }
    const AnimEditClip& clip = m_doc.GetClips()[static_cast<size_t>(_clipIndex)];
    if (m_frame < 0 || m_frame >= static_cast<int>(clip.frames.size()))
    {
        return;
    }
    const AnimFrame& cur = clip.frames[static_cast<size_t>(m_frame)];
    int              dx  = 0;
    int              dy  = 0;
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true))
    {
        dx = 1;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true))
    {
        dx = -1;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true))
    {
        dy = 1;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true))
    {
        dy = -1;
    }
    if (dx != 0 || dy != 0)
    {
        m_doc.SetPivot(_clipIndex, m_frame, cur.pivotX + dx, cur.pivotY + dy);
    }
}

Span<const AnimEditClip> AnimDocument::GetClips() const
{
    return m_clips;
}

bool AnimDocument::IsDirty() const
{
    return m_bDirty;
}

bool AnimDocument::CanUndo() const
{
    return !m_undo.empty();
}

void AnimDocument::ClearDirty()
{
    m_bDirty = false;
}

int AnimDocument::GetRevision() const
{
    return m_revision;
}

int AnimDocument::GetLastChangedClip() const
{
    return m_lastChanged;
}

void AnimDocument::ApplyClipTo(
    const int         _clip,
    AnimationLibrary& _library) const
{
    if (_clip < 0 || _clip >= static_cast<int>(m_clips.size()))
    {
        return;
    }
    const AnimEditClip& src = m_clips[static_cast<size_t>(_clip)];
    if (src.frames.empty() || !(src.frameDuration > 0.0))
    {
        return;
    }
    AnimClip clip;
    clip.name          = src.name;
    clip.bLoop         = src.bLoop;
    clip.loopStart     = Clamp(src.loopStart, 0, static_cast<int>(src.frames.size()) - 1);
    clip.frameDuration = static_cast<float>(src.frameDuration);
    clip.cellW         = src.cellW;
    clip.cellH         = src.cellH;
    clip.frames        = src.frames;
    (void)_library.AddClip(std::move(clip));
}

void AnimDocument::ApplyTo(
    AnimationLibrary& _library) const
{
    for (int i = 0; i < static_cast<int>(m_clips.size()); ++i)
    {
        ApplyClipTo(i, _library);
    }
}

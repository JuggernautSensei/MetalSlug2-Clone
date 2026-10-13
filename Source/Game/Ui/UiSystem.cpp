#include "Game/Ui/UiSystem.h"

#include "Core/AssetPath.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"
#include <algorithm>
#include <cstdio>
#include <utility>

#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Base/Types.h"

void UiSystem::Init()
{
    m_tick = 0;
    m_banner.Clear();
    m_fade.SetClear();
    m_bSprites = false;

    if (FindAssetRoot().empty())
    {
        return;
    }

    if (Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(FindAssetRoot()))
    {
        m_sprites  = library.Take();
        m_bSprites = true;
        m_font.Init(m_sprites);
    }
    else
    {
        SLUG_LOG_WARN("UI: sprites.json 을 읽지 못해 디버그 폰트로 대체합니다");
    }
}

void UiSystem::Tick()
{
    ++m_tick;
    m_banner.Tick();
    m_fade.Tick();
}

void UiSystem::DrawText(
    const int        _x,
    const int        _y,
    const StringView _text,
    const TextStyle& _style) const
{
    m_font.DrawText(Renderer::GetSingleton(), SpriteBatch::GetSingleton(), _x, _y, _text, _style);
}

bool UiSystem::DrawSprite(
    const StringView _name,
    const int        _centerX,
    const int        _centerY,
    const int        _scale,
    const uint8_t    _alpha) const
{
    const SpriteInfo* pInfo = m_sprites.Find(_name);
    if (!pInfo || pInfo->bStandalone)
    {
        return false;
    }

    const int scale = Max(1, _scale);
    const int w     = pInfo->frame.w * scale;
    const int h     = pInfo->frame.h * scale;

    SpriteDrawOptions options;
    options.tint.a = _alpha;
    SpriteBatch::GetSingleton().DrawFrameRect(pInfo->frame, { _centerX - w / 2, _centerY - h / 2, w, h }, options);
    return true;
}

bool UiSystem::DrawStandalone(
    const StringView _name,
    const int        _x,
    const int        _y,
    const uint8_t    _alpha) const
{
    const SpriteInfo* pInfo = m_sprites.Find(_name);
    if (!pInfo || !pInfo->bStandalone)
    {
        return false;
    }

    const int handle = SpriteBatch::GetSingleton().FindStandalone(pInfo->file);
    if (handle < 0)
    {
        return false;
    }
    SpriteBatch::GetSingleton().DrawTexture(handle, { static_cast<float>(_x), static_cast<float>(_y) }, { 0.f, 0.f }, Renderer::kLogicalWidth, Renderer::kLogicalHeight, { 255, 255, 255, _alpha });
    return true;
}

void UiSystem::DrawHud(
    const HudModel& _model)
{

    HudModel model = _model;
    model.uiTick   = m_tick;

    Renderer&    renderer = Renderer::GetSingleton();
    SpriteBatch& sprites  = SpriteBatch::GetSingleton();

    for (const HudItem& item: BuildHudItems(model, m_layout))
    {
        switch (item.kind)
        {
            case eHudItemKind::Text:
                m_font.DrawText(renderer, sprites, item.x, item.y, item.text, item.style);
                break;
            case eHudItemKind::Sprite:
                (void)DrawSprite(item.text, item.x, item.y, item.scale);
                break;
            case eHudItemKind::TimeDigits:
                m_font.DrawTimeDigits(sprites, item.x, item.y, item.text);
                break;
            case eHudItemKind::Rect:
                renderer.FillRect({ static_cast<float>(item.x), static_cast<float>(item.y) }, { static_cast<float>(item.w), static_cast<float>(item.h) }, item.color);
                break;
        }
    }
}

void UiSystem::DrawBanner()
{
    if (!m_banner.IsActive())
    {
        return;
    }

    const BannerSpec& spec    = m_banner.GetCurrent();
    const int         elapsed = m_banner.GetElapsedTicks();
    const int         lineY0  = spec.centerY >= 0 ? spec.centerY : m_layout.missionLine1Y;

    Vector<String> lines;
    String         current;
    for (const char c: spec.text)
    {
        if (c == '\n')
        {
            lines.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(c);
        }
    }
    lines.push_back(current);

    const int num  = Max(1, m_layout.missionGlyphNum);
    const int den  = Max(1, m_layout.missionGlyphDen);
    RGBA      tint = { 255, 255, 255, 255 };
    if (spec.color == eFontColor::Red)
    {
        tint = { 255, 110, 90, 255 };
    }

    int globalIndex = 0;
    for (size_t line = 0; line < lines.size(); ++line)
    {
        const String& text  = lines[line];
        const int     lineY = lineY0 + static_cast<int>(line) * m_layout.missionLineStride;

        int width = 0;
        for (const char c: text)
        {
            const String      name  = MakeMissionGlyphName(c);
            const SpriteInfo* pInfo = name.empty() ? nullptr : m_sprites.Find(name);
            width += pInfo ? (pInfo->frame.w * num / den + m_layout.missionGlyphGap) : (c == ' ' ? m_layout.missionSpaceW : 0);
        }
        int x = m_layout.screenW / 2 - width / 2;

        for (const char c: text)
        {
            const GlyphPose   pose  = ComputeMissionGlyphPose(elapsed, spec.fadeInTicks, spec.holdTicks, spec.fadeOutTicks, globalIndex++);
            const String      name  = MakeMissionGlyphName(c);
            const SpriteInfo* pInfo = name.empty() ? nullptr : m_sprites.Find(name);
            if (!pInfo)
            {
                x += (c == ' ' ? m_layout.missionSpaceW : 0);
                continue;
            }
            if (pose.alpha > 0)
            {
                const int         w = pInfo->frame.w * num / den;
                const int         h = pInfo->frame.h * num / den;
                SpriteDrawOptions options;
                options.tint   = tint;
                options.tint.a = pose.alpha;
                SpriteBatch::GetSingleton().DrawFrameRect(pInfo->frame, { x + pose.dx, lineY - h / 2 + pose.dy, w, h }, options);
            }
            x += pInfo->frame.w * num / den + m_layout.missionGlyphGap;
        }
    }
}

void UiSystem::DrawMissionResults() const
{
    if (!m_results.bBoxVisible)
    {
        return;
    }

    const uint8_t a   = m_results.boxAlpha;
    const IntRect box = { m_layout.resultsBoxX, m_layout.resultsBoxY, m_layout.resultsBoxW, m_layout.resultsBoxH };
    const int     cx  = box.x + box.w / 2;

    // 상자: 어두운 남색 바탕 + 연보라 테두리 (원본 영상 색)
    Renderer::GetSingleton().FillRect({ static_cast<float>(box.x), static_cast<float>(box.y) }, { static_cast<float>(box.w), static_cast<float>(box.h) }, { 12, 10, 60, static_cast<uint8_t>(a * 225 / 255) });
    const RGBA border = { 180, 170, 240, a };
    Renderer::GetSingleton().FillRect({ static_cast<float>(box.x), static_cast<float>(box.y) }, { static_cast<float>(box.w), 2.f }, border);
    Renderer::GetSingleton().FillRect({ static_cast<float>(box.x), static_cast<float>(box.y + box.h - 2) }, { static_cast<float>(box.w), 2.f }, border);
    Renderer::GetSingleton().FillRect({ static_cast<float>(box.x), static_cast<float>(box.y) }, { 2.f, static_cast<float>(box.h) }, border);
    Renderer::GetSingleton().FillRect({ static_cast<float>(box.x + box.w - 2), static_cast<float>(box.y) }, { 2.f, static_cast<float>(box.h) }, border);

    TextStyle white;
    white.color = eFontColor::White;
    white.align = eTextAlign::Center;
    white.alpha = a;
    m_font.DrawText(Renderer::GetSingleton(), SpriteBatch::GetSingleton(), m_layout.resultsTitleX, m_layout.resultsTitleY - kGlyphSize / 2, "RECAPTURED PRISONER", white);

    TextStyle yellow = white;
    yellow.color     = eFontColor::Yellow;
    m_font.DrawText(Renderer::GetSingleton(), SpriteBatch::GetSingleton(), cx, m_layout.resultsPlayerY - kGlyphSize / 2, "1P", yellow);

    ARRAY<char, 8> countBuf = {};
    std::snprintf(countBuf.data(), countBuf.size(), "%02d", Clamp(m_results.rescued, 0, 99));
    m_font.DrawTimeDigits(SpriteBatch::GetSingleton(), cx, m_layout.resultsCountTopY, countBuf.data(), a);
    if (m_results.bCounting)
    {
        TextStyle label = white;
        label.align     = eTextAlign::Left;
        m_font.DrawText(Renderer::GetSingleton(), SpriteBatch::GetSingleton(), cx + 36, m_layout.resultsCountTopY + 12, "X10000", label);
    }

    ARRAY<char, 16> scoreBuf = {};
    std::snprintf(scoreBuf.data(), scoreBuf.size(), "%d", Clamp(m_results.bonusScore, 0, 9'999'999));
    m_font.DrawTimeDigits(SpriteBatch::GetSingleton(), cx, m_layout.resultsScoreTopY, scoreBuf.data(), a);

    if (m_results.bRankVisible)
    {
        m_font.DrawText(Renderer::GetSingleton(), SpriteBatch::GetSingleton(), cx, m_layout.resultsRankY - kGlyphSize / 2, m_results.rankText, yellow);
    }
}

void UiSystem::DrawFade()
{
    const uint8_t alpha = m_fade.GetAlpha255();
    if (alpha == 0)
    {
        return;
    }
    Renderer::GetSingleton().FillRect({ 0.f, 0.f }, { static_cast<float>(m_layout.screenW), static_cast<float>(m_layout.screenH) }, { 0, 0, 0, alpha });
}

void UiSystem::DrawGameOverArt_() const
{
    if (!m_art.bVisible || m_art.alpha == 0)
    {
        return;
    }
    const SpriteInfo* pInfo = m_sprites.Find(m_art.sprite);
    if (!pInfo || pInfo->bStandalone || pInfo->frame.w <= 0 || pInfo->frame.h <= 0)
    {
        return;
    }

    // 화면을 덮도록 비율을 유지해 확대하고 가운데를 기준으로 잘라 그린다
    const float screenW = static_cast<float>(m_layout.screenW);
    const float screenH = static_cast<float>(m_layout.screenH);
    const float scale   = Max(screenH / static_cast<float>(pInfo->frame.h), screenW / static_cast<float>(pInfo->frame.w));
    const int   w       = static_cast<int>(static_cast<float>(pInfo->frame.w) * scale);
    const int   h       = static_cast<int>(static_cast<float>(pInfo->frame.h) * scale);

    SpriteDrawOptions options;
    options.tint.a = m_art.alpha;
    SpriteBatch::GetSingleton().DrawFrameRect(pInfo->frame, { (m_layout.screenW - w) / 2, (m_layout.screenH - h) / 2, w, h }, options);
}

void UiSystem::DrawOverlays()
{
    DrawMissionResults();
    DrawGameOverArt_();
    DrawFade();
    DrawBanner();
}

BannerSystem& UiSystem::GetBanner()
{
    return m_banner;
}

const BannerSystem& UiSystem::GetBanner() const
{
    return m_banner;
}

FadeController& UiSystem::GetFade()
{
    return m_fade;
}

const FadeController& UiSystem::GetFade() const
{
    return m_fade;
}

const UiLayout& UiSystem::GetLayout() const
{
    return m_layout;
}

int UiSystem::GetTick() const
{
    return m_tick;
}

bool UiSystem::HasSprites() const
{
    return m_bSprites;
}

void UiSystem::ShowBanner(
    const StringView _text,
    const float      _seconds)
{
    m_banner.Show(MakeBannerSpec(_text, _seconds));
}

void UiSystem::SetMissionResults(
    const MissionResultsView& _view)
{
    m_results = _view;
}

void UiSystem::SetGameOverArt(
    const GameOverArtView& _view)
{
    m_art = _view;
}

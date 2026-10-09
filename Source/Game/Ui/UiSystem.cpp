#include "Game/Ui/UiSystem.h"
#include "Base/Types.h"
#include "Base/Result.h"

#include <algorithm>
#include <map>
#include <utility>

#include "Base/Log.h"

namespace slug
{

namespace
{
    [[nodiscard]] int LoadStandaloneCached_(
        GameContext&          _ctx,
        const slug::StringView _name,
        const slug::StringView _file)
    {
        static std::map<std::pair<const SpriteBatch*, slug::String>, int> s_cache;

        const std::pair<const SpriteBatch*, slug::String> key { _ctx.pSprites, slug::String { _name } };
        if (const auto it = s_cache.find(key); it != s_cache.end())
        {
            return it->second;
        }

        int handle = -1;
        if (const slug::Result<int> loaded = _ctx.pSprites->LoadStandaloneTexture(_ctx.assetRoot / "Baked" / slug::FilePath { _file }))
        {
            handle = loaded.GetValue();
        }
        else
        {
            SLUG_LOG_WARN("UI 텍스처를 읽지 못했습니다: {}", _file);
        }
        s_cache.emplace(key, handle);
        return handle;
    }
}

void UiSystem::Init(
    const GameContext& _ctx)
{
    m_tick = 0;
    m_banner.Clear();
    m_fade.SetClear();
    m_bSprites = false;

    if (_ctx.IsHeadless() || _ctx.assetRoot.empty())
    {
        return;
    }

    if (slug::Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(_ctx.assetRoot / "Baked"))
    {
        m_sprites  = library.Take();
        m_bSprites = true;
        m_font.Init(m_sprites);
    }
    else
    {
        SLUG_LOG_WARN("UI: sprites.json 을 읽지 못해 디버그 폰트로 대체합니다 (Scripts/Bake.ps1 필요)");
    }
}

void UiSystem::Tick()
{
    ++m_tick;
    m_banner.Tick();
    m_fade.Tick();
}

void UiSystem::DrawText(
    GameContext&          _ctx,
    const int             _x,
    const int             _y,
    const slug::StringView _text,
    const TextStyle&      _style) const
{
    if (_ctx.IsHeadless() || !_ctx.pSprites)
    {
        return;
    }
    m_font.DrawText(*_ctx.pRenderer, *_ctx.pSprites, _x, _y, _text, _style);
}

bool UiSystem::DrawSprite(
    GameContext&          _ctx,
    const slug::StringView _name,
    const int             _centerX,
    const int             _centerY,
    const int             _scale,
    const uint8_t         _alpha) const
{
    if (_ctx.IsHeadless() || !_ctx.pSprites)
    {
        return false;
    }
    const SpriteInfo* pInfo = m_sprites.Find(_name);
    if (!pInfo || pInfo->bStandalone)
    {
        return false;
    }

    const int scale = std::max(1, _scale);
    const int w     = pInfo->frame.w * scale;
    const int h     = pInfo->frame.h * scale;

    SpriteDrawOptions options;
    options.tint.a = _alpha;
    _ctx.pSprites->DrawFrameRect(pInfo->frame, { _centerX - w / 2, _centerY - h / 2, w, h }, options);
    return true;
}

bool UiSystem::DrawStandalone(
    GameContext&          _ctx,
    const slug::StringView _name,
    const int             _x,
    const int             _y,
    const uint8_t         _alpha) const
{
    if (_ctx.IsHeadless() || !_ctx.pSprites)
    {
        return false;
    }
    const SpriteInfo* pInfo = m_sprites.Find(_name);
    if (!pInfo || !pInfo->bStandalone)
    {
        return false;
    }

    const int handle = LoadStandaloneCached_(_ctx, _name, pInfo->file);
    if (handle < 0)
    {
        return false;
    }
    _ctx.pSprites->DrawTexture(handle, { static_cast<float>(_x), static_cast<float>(_y) }, { 0.f, 0.f }, Renderer::kLogicalWidth, Renderer::kLogicalHeight, { 255, 255, 255, _alpha });
    return true;
}

void UiSystem::DrawHud(
    GameContext&    _ctx,
    const HudModel& _model)
{
    if (_ctx.IsHeadless() || !_ctx.pSprites)
    {
        return;
    }

    HudModel model = _model;
    model.uiTick   = m_tick;

    Renderer&    renderer = *_ctx.pRenderer;
    SpriteBatch& sprites  = *_ctx.pSprites;

    for (const HudItem& item: BuildHudItems(model, m_layout))
    {
        switch (item.kind)
        {
            case eHudItemKind::Text:
                m_font.DrawText(renderer, sprites, item.x, item.y, item.text, item.style);
                break;
            case eHudItemKind::Sprite:
                (void)DrawSprite(_ctx, item.text, item.x, item.y, item.scale);
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

void UiSystem::DrawBanner(
    GameContext& _ctx)
{
    if (_ctx.IsHeadless() || !_ctx.pSprites || !m_banner.IsActive())
    {
        return;
    }

    const float alpha = m_banner.GetAlpha();
    if (alpha <= 0.f)
    {
        return;
    }

    const BannerSpec& spec = m_banner.GetCurrent();

    TextStyle style;
    style.color   = spec.color;
    style.bBold   = spec.bBold;
    style.align   = eTextAlign::Center;
    style.lineGap = m_layout.bannerLineGap;
    style.alpha   = static_cast<uint8_t>(std::clamp(alpha, 0.f, 1.f) * 255.f + 0.5f);
    style.scale   = FitTextScale(spec.text, m_layout.screenW - 2 * m_layout.bannerMargin, std::min(spec.maxScale, m_layout.bannerMaxScale));

    const int centerY = spec.centerY >= 0 ? spec.centerY : m_layout.bannerCenterY;
    const int height  = MeasureTextHeight(spec.text, style);
    m_font.DrawText(*_ctx.pRenderer, *_ctx.pSprites, m_layout.screenW / 2, centerY - height / 2, spec.text, style);
}

void UiSystem::DrawFade(
    GameContext& _ctx)
{
    if (_ctx.IsHeadless())
    {
        return;
    }
    const uint8_t alpha = m_fade.GetAlpha255();
    if (alpha == 0)
    {
        return;
    }
    _ctx.pRenderer->FillRect({ 0.f, 0.f }, { static_cast<float>(m_layout.screenW), static_cast<float>(m_layout.screenH) }, { 0, 0, 0, alpha });
}

void UiSystem::DrawOverlays(
    GameContext& _ctx)
{
    DrawFade(_ctx);
    DrawBanner(_ctx);
}
}

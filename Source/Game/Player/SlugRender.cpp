#include "Game/Player/SlugRender.h"

#include <cmath>
#include <format>

#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Game/Combat/HitFlash.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace
{
[[nodiscard]] const AnimFrame* GetFrame_(
    const AnimationLibrary& _anims,
    const StringView        _name,
    const int               _frame)
{
    const AnimClip* pClip = _anims.FindClip(_name);
    if (pClip == nullptr || pClip->frames.empty())
    {
        return nullptr;
    }
    const int index = Clamp(_frame, 0, pClip->GetFrameCount() - 1);
    return &pClip->frames[static_cast<size_t>(index)];
}

[[nodiscard]] const AnimFrame* GetClipFrame_(
    const AnimationLibrary& _anims,
    const eSlugClip         _clip,
    const int               _frame)
{
    return GetFrame_(_anims, GetSlugClipInfo(_clip).pName, _frame);
}

[[nodiscard]] const AnimFrame* GetTimedFrame_(
    const AnimationLibrary& _anims,
    const StringView        _name,
    const int               _ageTicks)
{
    const AnimClip* pClip = _anims.FindClip(_name);
    if (pClip == nullptr || pClip->frames.empty())
    {
        return nullptr;
    }
    const float ticksPerFrame = Max(1.f, pClip->frameDuration * 60.f);
    const int   index         = Clamp(static_cast<int>(static_cast<float>(_ageTicks) / ticksPerFrame), 0, pClip->GetFrameCount() - 1);
    return &pClip->frames[static_cast<size_t>(index)];
}

void SubmitFx_(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const SlugFx&           _fx,
    const int               _depth)
{
    SpriteDrawOptions options;
    options.bFlipX = _fx.facing < 0;

    switch (_fx.kind)
    {
        case eSlugFxKind::CannonMuzzle:
        {
            String name = String { GetSlugClipInfo(eSlugClip::CannonFx).pName };
            if (_fx.variant == 5)
            {
                name = GetSlugClipInfo(eSlugClip::CannonFxAir).pName;
            }
            else if (_fx.variant == 6)
            {
                name = GetSlugClipInfo(eSlugClip::CannonFxCrouch).pName;
            }
            else if (_fx.variant > 0)
            {
                name = std::format("Slug_CannonFx_{}", _fx.variant);
            }
            if (const AnimFrame* pFrame = GetTimedFrame_(_anims, name, _fx.ageTicks))
            {
                _sprites.Submit(_depth, 0, *pFrame, _fx.position, options);
            }
            break;
        }
        case eSlugFxKind::Smoke:
        {
            if (const AnimFrame* pFrame = GetTimedFrame_(_anims, "Rocket_Smoke", _fx.ageTicks))
            {
                _sprites.Submit(_depth, 0, *pFrame, { _fx.position.x, _fx.position.y - static_cast<float>(_fx.ageTicks) * 0.5f }, options);
            }
            break;
        }
        case eSlugFxKind::Fire:
        {
            if (const AnimFrame* pFrame = GetTimedFrame_(_anims, "Fx_FireBomb_Air", _fx.ageTicks))
            {
                _sprites.Submit(_depth, 1, *pFrame, _fx.position, options);
            }
            break;
        }
        case eSlugFxKind::Explosion:
        {
            if (const AnimFrame* pFrame = GetTimedFrame_(_anims, GetSlugClipInfo(eSlugClip::Explosion).pName, _fx.ageTicks))
            {
                _sprites.Submit(_depth, 2, *pFrame, _fx.position, options);
            }
            break;
        }
        case eSlugFxKind::Debris:
        {
            const float   age = static_cast<float>(_fx.ageTicks);
            const VECTOR2 pos = { _fx.position.x + age * 1.5f * static_cast<float>(_fx.facing), _fx.position.y - 5.5f * age + 0.16f * age * age };
            if (const AnimFrame* pFrame = GetTimedFrame_(_anims, GetSlugClipInfo(eSlugClip::VulcanDebris).pName, _fx.ageTicks))
            {
                _sprites.Submit(_depth, 3, *pFrame, pos, options);
            }
            break;
        }
    }
}
}   // namespace

eSlugClip GetSlugBodyClip(
    const eSlugPose _pose)
{
    switch (_pose)
    {
        case eSlugPose::ParkedOpen:
        case eSlugPose::BoardingIn: return eSlugClip::Enter;
        case eSlugPose::Idle: return eSlugClip::Idle;
        case eSlugPose::Drive: return eSlugClip::Drive;
        case eSlugPose::AccelStart: return eSlugClip::AccelFwd;
        case eSlugPose::AccelStop: return eSlugClip::AccelBack;
        case eSlugPose::Crouching: return eSlugClip::Crouch;
        case eSlugPose::LowIdle: return eSlugClip::LowIdle;
        case eSlugPose::LowDrive: return eSlugClip::LowDrive;
        case eSlugPose::Jump: return eSlugClip::Jump;
        case eSlugPose::Land: return eSlugClip::Land;
        case eSlugPose::HitFront: return eSlugClip::HitFront;
        case eSlugPose::HitBack: return eSlugClip::HitBack;
        case eSlugPose::AirHitA: return eSlugClip::AirHitA;
        case eSlugPose::AirHitB: return eSlugClip::AirHitB;
    }
    return eSlugClip::Idle;
}

bool IsSlugOwner(
    const uint32_t _owner)
{
    return _owner >= kSlugOwnerBase && _owner < kSlugOwnerBase + static_cast<uint32_t>(kMaxSlugs);
}

void SubmitSlug(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const SlugVehicle&      _slug,
    const float             _alpha)
{
    if (_slug.GetMode() == eSlugMode::Gone)
    {
        return;
    }

    const VECTOR2 feet    = Lerp(_slug.GetPrevFeet(), _slug.GetFeet(), _alpha);
    const VECTOR2 delta   = feet - _slug.GetFeet();
    const int     sortKey = static_cast<int>(feet.y);
    const int     bodyZ   = ResolveZ(_slug.GetZ(), kSlugBodyDepth);
    const int     fxZ     = bodyZ + kSlugFxZOffset;
    const int     arrowZ  = bodyZ + kSlugArrowZOffset;

    if (_slug.IsBodyVisible())
    {
        const eSlugClip clip = GetSlugBodyClip(_slug.GetPose());
        if (const AnimFrame* pFrame = GetClipFrame_(_anims, clip, _slug.GetPoseFrame()))
        {
            SpriteDrawOptions options;
            options.bFlipX      = _slug.GetFacing() < 0;
            options.flashAmount = (IsHitFlashOn(_slug.GetFlashTicks()) || _slug.IsBlinkWhite()) ? 1.f : 0.f;
            _sprites.Submit(bodyZ, sortKey, *pFrame, feet, options);
        }
    }

    if (_slug.IsVulcanVisible())
    {
        const int        idx    = VulcanIndexFromAngle(_slug.GetVulcanAngle());
        const int        age    = _slug.GetVulcanShotAge();
        const AnimFrame* pFrame = nullptr;
        if (age < GetSlugClipInfo(eSlugClip::VulcanIdle).frames && age < 8)
        {
            pFrame = GetFrame_(_anims, GetVulcanShotClipName(idx), age / 2);
        }
        if (pFrame == nullptr)
        {
            pFrame = GetClipFrame_(_anims, eSlugClip::VulcanIdle, idx);
        }
        if (pFrame != nullptr)
        {
            SpriteDrawOptions options;
            options.flashAmount = IsHitFlashOn(_slug.GetFlashTicks()) ? 1.f : 0.f;
            _sprites.Submit(bodyZ, sortKey, *pFrame, _slug.GetVulcanMount() + delta, options);
        }
    }

    for (const SlugFx& fx: _slug.GetFx())
    {
        SubmitFx_(_sprites, _anims, fx, fxZ);
    }

    if (_slug.ShouldShowInArrow())
    {
        const int frameCount = GetSlugClipInfo(eSlugClip::ArrowIn).frames;
        const int frame      = (_slug.GetTick() * GetSlugClipInfo(eSlugClip::ArrowIn).fps / 60) % frameCount;
        if (const AnimFrame* pFrame = GetClipFrame_(_anims, eSlugClip::ArrowIn, frame))
        {
            _sprites.Submit(arrowZ, 0, *pFrame, { feet.x, feet.y - 84.f - 8.f });
        }
    }
    if (_slug.ShouldShowOutArrow())
    {
        const int frameCount = GetSlugClipInfo(eSlugClip::ArrowOut).frames;
        const int frame      = (_slug.GetTick() * GetSlugClipInfo(eSlugClip::ArrowOut).fps / 60) % frameCount;
        if (const AnimFrame* pFrame = GetClipFrame_(_anims, eSlugClip::ArrowOut, frame))
        {
            _sprites.Submit(arrowZ, 0, *pFrame, { feet.x, feet.y + 6.f });
        }
    }
}

bool SubmitSlugBullet(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const Bullet&           _bullet,
    const VECTOR2           _position,
    const int               _depth)
{
    if (!IsSlugOwner(_bullet.owner))
    {
        return false;
    }
    const float dx    = Abs(_bullet.velocity.x);
    const float dy    = Abs(_bullet.velocity.y);
    const float angle = ATan2(dy, dx) * kRad2Deg;
    const int   frame = Clamp(static_cast<int>(std::lround(angle / kBulletFrameStep)), 0, kBulletFrameCount - 1);

    const AnimFrame* pFrame = GetClipFrame_(_anims, eSlugClip::Bullet, frame);
    if (pFrame == nullptr)
    {
        return false;
    }
    SpriteDrawOptions options;
    options.bFlipX = _bullet.velocity.x < 0.f;
    options.bFlipY = _bullet.velocity.y < 0.f;
    _sprites.Submit(_depth, 0, *pFrame, _position, options);
    return true;
}

bool SubmitSlugShell(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const Bomb&             _bomb,
    const VECTOR2           _position,
    const int               _depth)
{
    if (!IsSlugOwner(_bomb.params.owner))
    {
        return false;
    }
    const float angle = ATan2(-_bomb.velocity.y, Abs(_bomb.velocity.x)) * kRad2Deg;
    const int   row   = Clamp(static_cast<int>(std::lround((90.f - Abs(angle)) / kVulcanAngleStep)), 0, kShellRowCount - 1);
    const int   col   = Clamp(3 + _bomb.ageTicks / 3, 3, kShellColCount - 1);

    const AnimFrame* pFrame = GetFrame_(_anims, GetShellClipName(row), col);
    if (pFrame == nullptr)
    {
        return false;
    }
    SpriteDrawOptions options;
    options.bFlipX = _bomb.velocity.x < 0.f;
    options.bFlipY = angle < 0.f;
    _sprites.Submit(_depth, 0, *pFrame, _position, options);
    return true;
}

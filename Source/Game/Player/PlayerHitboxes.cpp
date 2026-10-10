#include "Game/Player/PlayerHitboxes.h"

#include "Base/Math.h"

namespace slug
{

namespace
{
    constexpr float kInvSqrt2 = 0.70710678f;

    [[nodiscard]] const MuzzleSet& GetSet_(
        const ePose         _pose,
        const MuzzleTuning& _muzzle)
    {
        switch (_pose)
        {
            case ePose::Crouch: return _muzzle.crouch;
            case ePose::Air: return _muzzle.air;
            default: return _muzzle.stand;
        }
    }
}   // namespace

Aabb MakeKnifeTriggerBox(
    const Vec2         _feet,
    const int          _facing,
    const float        _bodyHalfWidth,
    const KnifeTuning& _knife)
{
    const float front = _bodyHalfWidth + _knife.range;
    const float x0    = _facing > 0 ? _feet.x : _feet.x - front;
    const float x1    = _facing > 0 ? _feet.x + front : _feet.x;
    return Aabb::FromMinMax({ x0, _feet.y - _knife.top }, { x1, _feet.y - _knife.bottom });
}

Aabb MakeKnifeSwingBox(
    const Vec2         _feet,
    const int          _facing,
    const KnifeTuning& _knife)
{
    const float x0 = _facing > 0 ? _feet.x - _knife.back : _feet.x - _knife.reach;
    const float x1 = _facing > 0 ? _feet.x + _knife.reach : _feet.x + _knife.back;
    return Aabb::FromMinMax({ x0, _feet.y - _knife.top }, { x1, _feet.y - _knife.bottom });
}

Vec2 GetMuzzlePosition(
    const Vec2          _feet,
    const int           _facing,
    const ePose         _pose,
    const eAim          _aim,
    const MuzzleTuning& _muzzle)
{
    const MuzzleSet& set = GetSet_(_pose, _muzzle);

    Vec2 local = set.forward;
    switch (_aim)
    {
        case eAim::Up: local = set.up; break;
        case eAim::DiagUp: local = set.diagUp; break;
        case eAim::Down: local = set.down; break;
        case eAim::DiagDown: local = set.diagDown; break;
        case eAim::Forward: break;
    }
    return { _feet.x + local.x * static_cast<float>(_facing), _feet.y + local.y };
}

Vec2 GetAimDirection(
    const eAim _aim,
    const int  _facing)
{
    const float f = static_cast<float>(_facing);
    switch (_aim)
    {
        case eAim::Up: return { 0.f, -1.f };
        case eAim::DiagUp: return { f * kInvSqrt2, -kInvSqrt2 };
        case eAim::Down: return { 0.f, 1.f };
        case eAim::DiagDown: return { f * kInvSqrt2, kInvSqrt2 };
        case eAim::Forward: break;
    }
    return { f, 0.f };
}

Vec2 GetBombReleasePosition(
    const Vec2        _feet,
    const int         _facing,
    const ePose       _pose,
    const BombTuning& _bomb)
{
    const float lowering = _pose == ePose::Crouch ? 14.f : 0.f;
    return { _feet.x + _bomb.releaseOffset.x * static_cast<float>(_facing), _feet.y + _bomb.releaseOffset.y + lowering };
}

Aabb MakePlayerHurtboxBox(
    const Vec2        _feet,
    const ePose       _pose,
    const BodyTuning& _body)
{
    const Vec2 size = _pose == ePose::Crouch ? _body.hurtCrouch : _body.hurtStand;
    return Aabb::FromBottomCenter(_feet, size);
}

}   // namespace slug

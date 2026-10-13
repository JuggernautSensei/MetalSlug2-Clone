#include "Game/Combat/BulletVisual.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Audio/AudioSystem.h"
#include "Base/Math.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"

constexpr float kTickSec = 1.f / 60.f;

// 파이어샷 입자: 베이크된 큰 불꽃 클립(FlameShot_AttackMotion, 26프레임, 최대 94x94) 을 Head -> Loop(반복) -> Fade 로 쓴다
constexpr const char* kFlameClip         = "FlameShot_AttackMotion";
constexpr int         kFlameHeadFrames   = 4;   // 머리: 앞 4프레임 (입자 생성 직후)
constexpr int         kFlameFadeFrames   = 6;   // 꼬리: 마지막 6프레임 (사거리 끝)
constexpr int         kFlameHeadTicks    = 2;
constexpr float       kFlameFadeTicks    = 8.f;
constexpr float       kFlameForwardShift = 24.f;   // 불꽃 중심을 총구보다 앞으로 (큰 프레임이 총을 가리지 않게)

namespace
{
[[nodiscard]] const AnimFrame* PickFlameFrame_(
    const AnimationLibrary* _pAnims,
    const Bullet&           _bullet)
{
    const AnimClip* pClip = _pAnims ? _pAnims->FindClip(kFlameClip) : nullptr;
    if (!pClip || pClip->frames.empty())
    {
        return nullptr;
    }
    const int count = pClip->GetFrameCount();
    if (count <= kFlameHeadFrames + kFlameFadeFrames)
    {
        return &pClip->frames[static_cast<size_t>(Clamp(_bullet.ageTicks / kFlameHeadTicks, 0, count - 1))];
    }

    const float speed      = Max(Abs(_bullet.velocity.x) + Abs(_bullet.velocity.y), 1.f);
    const float fadeLength = speed * kFlameFadeTicks / 60.f;
    const float remaining  = _bullet.range - _bullet.travelled;
    if (remaining < fadeLength)
    {
        const float progress = Clamp(1.f - remaining / fadeLength, 0.f, 1.f);
        const int   fadeStep = Min(kFlameFadeFrames - 1, static_cast<int>(progress * static_cast<float>(kFlameFadeFrames)));
        return &pClip->frames[static_cast<size_t>(count - kFlameFadeFrames + fadeStep)];
    }

    const int headTicks = kFlameHeadFrames * kFlameHeadTicks;
    if (_bullet.ageTicks < headTicks)
    {
        return &pClip->frames[static_cast<size_t>(_bullet.ageTicks / kFlameHeadTicks)];
    }
    const int loopCount = count - kFlameHeadFrames - kFlameFadeFrames;
    const int loopStep  = ((_bullet.ageTicks - headTicks) / kFlameHeadTicks) % loopCount;
    return &pClip->frames[static_cast<size_t>(kFlameHeadFrames + loopStep)];
}

[[nodiscard]] const AnimFrame* PickClipFrame_(
    const AnimationLibrary* _pAnims,
    const String&           _name,
    const int               _ageTicks,
    const bool              _bLoop)
{
    const AnimClip* pClip = _pAnims ? _pAnims->FindClip(_name) : nullptr;
    if (!pClip || pClip->frames.empty())
    {
        return nullptr;
    }
    const int count = pClip->GetFrameCount();
    const int index = static_cast<int>(static_cast<float>(_ageTicks) * kTickSec / pClip->frameDuration);
    return &pClip->frames[static_cast<size_t>(_bLoop ? index % count : Min(index, count - 1))];
}
}   // namespace

bool SubmitPlayerBullet(
    SpriteBatch&            _sprites,
    const AnimationLibrary* _pAnims,
    const SpriteLibrary&    _spriteLib,
    const Bullet&           _bullet,
    const VECTOR2           _pos,
    const int               _depth)
{
    const eWeapon weapon    = static_cast<eWeapon>(_bullet.tag);
    const float   vx        = _bullet.velocity.x;
    const float   vy        = _bullet.velocity.y;
    const bool    bVertical = Abs(vy) > Abs(vx);
    const bool    bRight    = vx >= 0.f;
    const char*   pDir      = bVertical ? (vy < 0.f ? "Up" : "Down") : (bRight ? "Right" : "Left");

    switch (weapon)
    {
        case eWeapon::Hmg:
        {
            float degrees = std::atan2(-vy, vx) * kRad2Deg;
            if (degrees < 0.f)
            {
                degrees += 360.f;
            }
            const int         index = static_cast<int>(std::lround(degrees / 6.f)) % 60;
            const String      name  = "HM_Bullet_" + std::to_string(index / 15) + "_" + std::to_string(index % 15);
            const SpriteInfo* pInfo = _spriteLib.Find(name);
            if (!pInfo)
            {
                return false;
            }
            _sprites.Submit(_depth, 0, pInfo->frame, _pos);
            return true;
        }
        case eWeapon::Shotgun:
        {
            const AnimFrame* pFrame = PickClipFrame_(_pAnims, String("ShotGun_Bullet_") + pDir, _bullet.ageTicks, false);
            if (!pFrame)
            {
                return false;
            }
            _sprites.Submit(_depth, 0, *pFrame, _pos);
            return true;
        }
        case eWeapon::Rocket:
        {
            // 로켓 스프라이트는 가로(오른쪽 향함/왼쪽 향함) 한 방향뿐이다. 진행 방향 각도만큼 피벗 기준으로 돌려 그린다
            const AnimFrame* pFrame = PickClipFrame_(_pAnims, bRight ? "Aparch_Missle_Right" : "Aparch_Missle_Left", _bullet.ageTicks, true);
            if (!pFrame)
            {
                return false;
            }
            float degrees = std::atan2(vy, vx) * kRad2Deg - (bRight ? 0.f : 180.f);
            if (degrees > 180.f)
            {
                degrees -= 360.f;
            }
            else if (degrees < -180.f)
            {
                degrees += 360.f;
            }
            _sprites.Submit(_depth, 0, *pFrame, _pos, { .rotationDeg = degrees });
            return true;
        }
        case eWeapon::Flame:
        {
            const AnimFrame* pFrame = PickFlameFrame_(_pAnims, _bullet);
            if (!pFrame)
            {
                return false;
            }
            // 수명에 따라 표시 배율이 커진다 (판정 성장과 같은 진행도: travelled / range)
            const float   t       = _bullet.range > 0.f ? Clamp(_bullet.travelled / _bullet.range, 0.f, 1.f) : 1.f;
            const float   scale   = Lerp(_bullet.scaleStart, _bullet.scaleEnd, t);
            const VECTOR2 shifted = { _pos.x + (bRight ? kFlameForwardShift : -kFlameForwardShift), _pos.y };
            _sprites.Submit(_depth, 0, *pFrame, shifted, { .bFlipX = !bRight, .scale = scale });
            return true;
        }
        default:
        {
            const SpriteInfo* pInfo = _spriteLib.Find(bVertical ? "Player_Bullet_Basic_V" : "Player_Bullet_Basic_H");
            if (!pInfo)
            {
                return false;
            }
            _sprites.Submit(_depth, 0, pInfo->frame, _pos);
            return true;
        }
    }
}

void PlayShotSfx(
    AudioSystem&  _audio,
    const eWeapon _weapon,
    const int     _shotIndex)
{
    switch (_weapon)
    {
        case eWeapon::Hmg: _audio.Play(kPistolShot, 0.55f, 0.12f); break;
        case eWeapon::Shotgun: _audio.Play(kPistolShot, 1.f, 0.06f); break;
        case eWeapon::Rocket: _audio.Play(kPistolShot, 0.9f, 0.02f); break;
        case eWeapon::Flame:
            if (_shotIndex % 6 == 0)
            {
                _audio.Play(kFlameShot, 0.7f, 0.05f);
            }
            break;
        default: _audio.Play(kPistolShot, 0.8f, 0.04f); break;
    }
}

void PlayWeaponPickupSfx(
    AudioSystem&  _audio,
    const eWeapon _weapon)
{
    if (_weapon == eWeapon::Pistol)
    {
        return;
    }
    _audio.Play(kPowerUp);
    switch (_weapon)
    {
        case eWeapon::Hmg: _audio.Play(kVoiceHmg); break;
        case eWeapon::Shotgun: _audio.Play(kVoiceShotgun); break;
        case eWeapon::Rocket: _audio.Play(kVoiceRocket); break;
        case eWeapon::Flame: _audio.Play(kVoiceFlame); break;
        default: break;
    }
}

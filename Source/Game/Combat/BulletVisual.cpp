#include "Game/Combat/BulletVisual.h"
#include "Base/Math.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Audio/AudioSink.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"

namespace slug
{

namespace
{
    constexpr float kTickSec = 1.f / 60.f;

    const AnimFrame* PickClipFrame_(
        const AnimationLibrary* _pAnims,
        const std::string&      _name,
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
        return &pClip->frames[static_cast<size_t>(_bLoop ? index % count : std::min(index, count - 1))];
    }
}

bool SubmitPlayerBullet(
    SpriteBatch&        _sprites,
    const AnimationLibrary* _pAnims,
    const SpriteLibrary& _spriteLib,
    const Bullet&       _bullet,
    const Vec2  _pos,
    const int           _layer)
{
    const eWeapon weapon    = static_cast<eWeapon>(_bullet.tag);
    const float   vx        = _bullet.velocity.x;
    const float   vy        = _bullet.velocity.y;
    const bool    bVertical = std::abs(vy) > std::abs(vx);
    const bool    bRight    = vx >= 0.f;
    const char*   dir       = bVertical ? (vy < 0.f ? "Up" : "Down") : (bRight ? "Right" : "Left");

    switch (weapon)
    {
        case eWeapon::Hmg:
        {
            float degrees = std::atan2(-vy, vx) * (180.f / 3.14159265f);
            if (degrees < 0.f)
            {
                degrees += 360.f;
            }
            const int         index = static_cast<int>(std::lround(degrees / 6.f)) % 60;
            const std::string name  = "HM_Bullet_" + std::to_string(index / 15) + "_" + std::to_string(index % 15);
            const SpriteInfo* pInfo  = _spriteLib.Find(name);
            if (!pInfo)
            {
                return false;
            }
            _sprites.Submit(_layer, 0, pInfo->frame, _pos);
            return true;
        }
        case eWeapon::Shotgun:
        {
            const AnimFrame* pFrame = PickClipFrame_(_pAnims, std::string("ShotGun_Bullet_") + dir, _bullet.ageTicks, false);
            if (!pFrame)
            {
                return false;
            }
            _sprites.Submit(_layer, 0, *pFrame, _pos);
            return true;
        }
        case eWeapon::Rocket:
        {
            const AnimFrame* pFrame = PickClipFrame_(_pAnims, bRight ? "Aparch_Missle_Right" : "Aparch_Missle_Left", _bullet.ageTicks, true);
            if (!pFrame)
            {
                return false;
            }
            _sprites.Submit(_layer, 0, *pFrame, _pos);
            return true;
        }
        case eWeapon::Flame:
        {
            const AnimFrame* pFrame = PickClipFrame_(_pAnims, "FlameShot_AttackMotion", _bullet.ageTicks, true);
            if (!pFrame)
            {
                return false;
            }
            _sprites.Submit(_layer, 0, *pFrame, _pos, { .bFlipX = !bRight });
            return true;
        }
        default:
        {
            const SpriteInfo* pInfo = _spriteLib.Find(bVertical ? "Player_Bullet_Basic_V" : "Player_Bullet_Basic_H");
            if (!pInfo)
            {
                return false;
            }
            _sprites.Submit(_layer, 0, pInfo->frame, _pos);
            return true;
        }
    }
}

void PlayShotSfx(
    IAudioSink& _audio,
    const eWeapon _weapon,
    const int   _shotIndex)
{
    switch (_weapon)
    {
        case eWeapon::Hmg: _audio.Play(sfx::kPistolShot, 0.55f, 0.12f); break;
        case eWeapon::Shotgun: _audio.Play(sfx::kPistolShot, 1.f, 0.06f); break;
        case eWeapon::Rocket: _audio.Play(sfx::kPistolShot, 0.9f, 0.02f); break;
        case eWeapon::Flame:
            if (_shotIndex % 6 == 0)
            {
                _audio.Play(sfx::kFlameShot, 0.7f, 0.05f);
            }
            break;
        default: _audio.Play(sfx::kPistolShot, 0.8f, 0.04f); break;
    }
}

void PlayWeaponPickupSfx(
    IAudioSink& _audio,
    const eWeapon _weapon)
{
    if (_weapon == eWeapon::Pistol)
    {
        return;
    }
    _audio.Play(sfx::kPowerUp);
    switch (_weapon)
    {
        case eWeapon::Hmg: _audio.Play(sfx::kVoiceHmg); break;
        case eWeapon::Shotgun: _audio.Play(sfx::kVoiceShotgun); break;
        case eWeapon::Rocket: _audio.Play(sfx::kVoiceRocket); break;
        case eWeapon::Flame: _audio.Play(sfx::kVoiceFlame); break;
        default: break;
    }
}
}

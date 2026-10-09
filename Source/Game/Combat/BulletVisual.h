#pragma once
#include "Base/Math.h"
#include "Core/Tuning.h"
#include "Game/Combat/Projectiles.h"

namespace slug
{

class AnimationLibrary;
class IAudioSink;
class SpriteBatch;
class SpriteLibrary;

bool SubmitPlayerBullet(SpriteBatch& _sprites, const AnimationLibrary* _pAnims, const SpriteLibrary& _spriteLib, const Bullet& _bullet, Vec2 _pos, int _layer);

void PlayShotSfx(IAudioSink& _audio, eWeapon _weapon, int _shotIndex);

void PlayWeaponPickupSfx(IAudioSink& _audio, eWeapon _weapon);
}

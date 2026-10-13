#pragma once
#include "Base/Math.h"
#include "Core/Tuning.h"
#include "Game/Combat/Projectiles.h"

class AnimationLibrary;
class AudioSystem;
class SpriteBatch;
class SpriteLibrary;

bool SubmitPlayerBullet(SpriteBatch& _sprites, const AnimationLibrary* _pAnims, const SpriteLibrary& _spriteLib, const Bullet& _bullet, VECTOR2 _pos, int _depth);

void PlayShotSfx(AudioSystem& _audio, eWeapon _weapon, int _shotIndex);

void PlayWeaponPickupSfx(AudioSystem& _audio, eWeapon _weapon);

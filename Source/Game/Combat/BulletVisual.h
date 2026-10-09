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

// 플레이어 탄 연출 (판정과 무관). 무기(Bullet.tag)와 방향/나이로 스프라이트·클립 프레임을 고른다.
// 그릴 것이 없으면 false 를 돌려준다 (호출자가 폴백 사각형을 그린다).
bool SubmitPlayerBullet(SpriteBatch& _sprites, const AnimationLibrary* _pAnims, const SpriteLibrary& _spriteLib, const Bullet& _bullet, Vec2 _pos, int _layer);

// 플레이어 발사음 (_shotIndex: 같은 틱의 몇 번째 발사인지. 화염처럼 빠른 무기는 일부만 울린다)
void PlayShotSfx(IAudioSink& _audio, eWeapon _weapon, int _shotIndex);

// 무기 획득 연출: 파워업 징글 + 무기 음성 (권총 제외)
void PlayWeaponPickupSfx(IAudioSink& _audio, eWeapon _weapon);

}   // namespace slug

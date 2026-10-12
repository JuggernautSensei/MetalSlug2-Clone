#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/SlugClips.h"
#include "Game/Player/SlugVehicle.h"

class AnimationLibrary;
class SpriteBatch;

// 렌더 레이어 (SandboxState/StageState 의 기존 레이어: 적 10, 플레이어 20, 총알 30, 폭탄 31)
inline constexpr int kSlugBodyDepth  = 19;
inline constexpr int kSlugFxDepth    = 32;
inline constexpr int kSlugArrowDepth = 34;

// 부품 오프셋: SlugVehicle z (몸통/발칸) 기준 FX, 화살표의 상대 depth
inline constexpr int kSlugFxZOffset    = kSlugFxDepth - kSlugBodyDepth;
inline constexpr int kSlugArrowZOffset = kSlugArrowDepth - kSlugBodyDepth;

[[nodiscard]] eSlugClip GetSlugBodyClip(eSlugPose _pose);

// 차량 몸통 + 발칸 + 효과 + IN/OUT 화살표를 스프라이트 배치에 제출한다 (보간 위치 사용).
void SubmitSlug(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const SlugVehicle&      _slug,
    float                   _alpha);

// 발칸탄 / 캐논 포탄 스프라이트. 해당 소유자(SV-001)의 투사체일 때만 true.
[[nodiscard]] bool IsSlugOwner(uint32_t _owner);

bool SubmitSlugBullet(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const Bullet&           _bullet,
    Vec2                    _position,
    int                     _depth);

bool SubmitSlugShell(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const Bomb&             _bomb,
    Vec2                    _position,
    int                     _depth);

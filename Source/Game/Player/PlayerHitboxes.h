#pragma once
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Player/PlayerTypes.h"
#include "World/Combat.h"
#include "World/Geometry.h"

namespace slug
{

// ===========================================
//  플레이어 판정 기하 (순수 함수)
//   발 중앙(feet) 과 바라보는 방향(facing: +1/-1) 에서 상자/점을 만든다.
//   칼은 eHitKind::Melee, 폭탄은 eHitKind::Blast, 총알은 eHitKind::Bullet — 대상별 특수 처리는 없다.
// ===========================================

// 칼 자동 발동 검사 영역: 몸 앞 가장자리 + knife.range 까지. 이 안에 다른 팀 Hurtbox 가 있으면 사격이 칼로 바뀐다.
[[nodiscard]] Aabb MakeKnifeTriggerBox(Vec2 _feet, int _facing, float _bodyHalfWidth, const KnifeTuning& _knife);

// 칼 베기 판정 상자 (발동 후 activeFirstTick..activeLastTick 동안 유효)
[[nodiscard]] Aabb MakeKnifeSwingBox(Vec2 _feet, int _facing, const KnifeTuning& _knife);

// 총구 위치 (월드)
[[nodiscard]] Vec2 GetMuzzlePosition(Vec2 _feet, int _facing, ePose _pose, eAim _aim, const MuzzleTuning& _muzzle);

// 조준 방향 단위 벡터 (월드, +y 아래)
[[nodiscard]] Vec2 GetAimDirection(eAim _aim, int _facing);

// 폭탄이 손을 떠나는 위치
[[nodiscard]] Vec2 GetBombReleasePosition(Vec2 _feet, int _facing, ePose _pose, const BombTuning& _bomb);

// 몸통(물리) 크기 / 피격 상자
[[nodiscard]] Vec2 GetBodySize(ePose _pose, const BodyTuning& _body);
[[nodiscard]] Aabb         MakePlayerHurtboxBox(Vec2 _feet, ePose _pose, const BodyTuning& _body);

// 칼 판정이 켜져 있는 틱인가 (발동 후 경과 틱 기준)
[[nodiscard]] constexpr bool IsKnifeSwingActive(
    const int          _ticksSinceStart,
    const KnifeTuning& _knife)
{
    return _ticksSinceStart >= _knife.activeFirstTick && _ticksSinceStart <= _knife.activeLastTick;
}

}   // namespace slug

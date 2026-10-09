#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Enemies/EnemyCommon.h"

namespace slug
{

// ===========================================
//  차량 / 전차 / 헬기 공용 도우미 (Vehicle*.cpp 가 함께 쓴다)
//   사망 연출은 데이터(def.death.fx, 지연 포함)로 선언하고 여기서는 재생/정리만 한다.
//   차량 원점 = 발 (지면 차량) 또는 기체 중심 (헬기). 연출 오프셋은 JSON 에 발 기준으로 적는다.
// ===========================================

// 사망 시작: 파괴 클립(_destroyClip, -1 이면 유지) + def.death.fx 요청 (fx 의 delay 는 월드가 처리한다)
void BeginVehicleDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _destroyClip);

// 사망 연출 중 매 틱: death.removeAtSec 에 도달하면 제거(Gone)한다. 깜빡임은 없다.
void TickVehicleDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

// 월드 좌표 _at 에 일회성 연출을 요청한다 (오프셋 없음, {side} 는 _facing 으로 치환)
void RequestFxAt(EnemyEnv& _env, jug::StringView _clip, jug::VECTOR2 _at, int _facing, int _layer = 60, float _delaySec = 0.f);

// 화면 좌표(카메라 왼쪽 위 기준, 픽셀) → 월드 좌표
[[nodiscard]] jug::VECTOR2 ScreenToWorld(const EnemyEnv& _env, jug::VECTOR2 _screen);

}   // namespace slug

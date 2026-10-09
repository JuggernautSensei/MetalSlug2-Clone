#pragma once
#include <JugX/Random.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "World/Physics.h"

namespace slug
{

class EnemyWorld;

// ===========================================
//  EnemyFxRequest — 이펙트 요청 (연출 전용, 시뮬레이션 상태에 영향 없음)
// ===========================================

struct EnemyFxRequest
{
    jug::String  clip       = {};   // {side} 는 요청 시점에 치환되어 있다
    jug::VECTOR2 position   = { 0.f, 0.f };
    int          layer      = 60;
    int          delayTicks = 0;
};

// ===========================================
//  EnemyEnv — 한 틱 동안 행동 함수들이 쓰는 서비스 묶음 (EnemyWorld 가 만든다)
// ===========================================

struct EnemyEnv
{
    EnemyWorld&             world;
    const EnemyContext&     ctx;
    const EnemyDefLibrary&  lib;
    PhysicsTuning           phys = {};   // 적 한 마리 분량 (중력/낙하 상한은 def 에서)
};

// ===========================================
//  BehaviorOps — 행동 종류별 함수 테이블 (EnemyBehaviors.cpp 의 GetBehaviorOps)
//   새 적 종류를 만들려면: eEnemyBehavior 값 추가 → <Kind>.cpp 에 Ops 함수들 구현 → 이 테이블에 한 줄 추가.
//   null 인 항목은 "아무것도 하지 않음".
// ===========================================

struct BehaviorOps
{
    // 생성 직후 한 번 (초기 상태/클립). 생성 시점에는 지면 정보가 없을 수 있다.
    void (*init)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    // 살아 있는 동안 매 틱 (활성 여부는 _e.bActive). 이동/물리/공격 판정 등록까지 여기서 한다.
    void (*tick)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    // HitEvent 를 받았을 때 (기본 피해 적용 전). true 를 돌려주면 처리 완료 — 기본 피해를 적용하지 않는다.
    // 방패병의 방패 판정 상자 피격, 포로 구출이 여기서 처리된다.
    bool (*onHit)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) = nullptr;

    // HP 가 0 이 된 순간 (사망 연출 시작). 점수/Died 이벤트는 월드가 이미 처리했다.
    void (*onDeath)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    // 사망 연출 중 매 틱. 끝나면 _e.life = Gone 으로 표시한다.
    void (*tickDying)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    // Hurtbox 등록 (null 이면 기본: def.hurtbox 하나). 여러 개를 낼 수 있다 (방패 판정 상자는 몸통보다 먼저).
    void (*registerHurtboxes)(const Enemy& _e, const EnemyDef& _def, CombatSystem& _combat) = nullptr;
};

[[nodiscard]] const BehaviorOps& GetBehaviorOps(eEnemyBehavior _behavior);

// 행동 파일별 Ops (각 .cpp 가 정의)
extern const BehaviorOps kSoldierOps;
extern const BehaviorOps kBzkOps;
extern const BehaviorOps kHangOps;
extern const BehaviorOps kFlyingBzkOps;
extern const BehaviorOps kShielderOps;
extern const BehaviorOps kPowOps;

}   // namespace slug

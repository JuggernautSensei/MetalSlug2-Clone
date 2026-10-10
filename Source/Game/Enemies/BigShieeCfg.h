#pragma once
#include "Base/Math.h"
#include "Base/Types.h"

namespace slug
{

inline constexpr int kBigShieeTurretCount   = 7;   // 포탑 A..G
inline constexpr int kBigShieeEntranceKeys  = 10;
inline constexpr int kBigShieeDeployStages  = 5;   // D1, D3, D6, D9, Open
inline constexpr int kBigShieePartCannon    = 7;   // 부위 번호: 0..6 포탑, 7 캐논, 8 선체
inline constexpr int kBigShieePartHull      = 8;
inline constexpr int kBigShieeHullBoxCount  = 2;

// Docs/Spec_BigShiee.md 의 수치 (게임 px = 원작 px x 2, 선체 로컬 좌표 = 선체 셀 좌상단 기준).
struct BigShieeCfg
{
    // 등장 (선체가 왼쪽에서 오른쪽으로 감속하며 들어온다)
    ARRAY<float, kBigShieeEntranceKeys> entranceT = {};   // 초
    ARRAY<float, kBigShieeEntranceKeys> entranceV = {};   // px/s
    int                                 fireStartTicks = 0;   // 등장 시작 후 포탑이 쏘기 시작하는 시점

    // 포탑 (앵커 = 스프라이트 아래 중앙)
    ARRAY<float, kBigShieeTurretCount> turretX     = {};
    ARRAY<float, kBigShieeTurretCount> turretY     = {};
    ARRAY<float, kBigShieeTurretCount> turretRow   = {};   // Idle/Shot 줄 번호 1..3
    ARRAY<float, kBigShieeTurretCount> turretFrame = {};   // 대기 자세 프레임
    ARRAY<float, kBigShieeTurretCount> turretFirst = {};   // 첫 발 시각 (등장 시작 기준, 초)
    int                                turretHp         = 20;
    int                                turretScore      = 100;
    Vec2                               turretBox        = { 76.f, 68.f };
    float                              turretBoxLift    = 34.f;   // 앵커에서 상자 중심까지 위로
    ARRAY<float, 2>                    turretIntervalSec = { 3.4f, 4.8f };
    int                                turretFireFrame  = 5;
    Vec2                               muzzleOffset     = { -14.f, -46.f };

    // 포탑 탄 (색이 도는 쌍탄, 느린 곡사)
    float           shellVy0         = -66.f;
    float           shellFlightSec   = 3.6f;
    ARRAY<float, 2> shellVxRange     = { -130.f, -30.f };
    float           shellSize        = 20.f;
    Vec2            shellPairOffset  = { 30.f, 6.f };
    int             shellColorTicks  = 4;
    int             shellGrowTicks   = 4;
    int             shellLifeTicks   = 480;

    // 불기둥 (착탄 지점에서 솟는다)
    float pillarW          = 72.f;
    float pillarH          = 230.f;
    int   pillarRiseTicks  = 22;
    int   pillarHoldTicks  = 60;
    int   pillarPuffs      = 6;
    float pillarPuffStep   = 40.f;
    int   pillarPuffDelay  = 3;

    // 전개
    ARRAY<float, kBigShieeDeployStages> deployT = {};   // 선체 단계 전환 시각 (초)
    float                               deployCannonSec = 1.3f;
    float                               deployEndSec    = 2.5f;

    // 캐논 (받침 좌하단 = 선체 로컬 좌표)
    Vec2  cannonBase      = { 293.f, 280.f };
    Vec2  cannonBox       = { 190.f, 250.f };   // 받침 좌하단에서 위로/오른쪽으로
    int   cannonHpA       = 140;
    int   cannonHpB       = 280;
    int   turnTicks       = 70;
    int   cannonScore     = 2000;
    Vec2  muzzleA         = { 20.f, -136.f };   // 받침 기준, 형태 A (왼쪽 포구)
    Vec2  muzzleB         = { 100.f, -156.f };  // 받침 기준, 형태 B (해골 입)
    ARRAY<float, 2> cannonIntervalSec = { 1.2f, 2.2f };
    ARRAY<float, 2> missileIntervalSec = { 2.4f, 3.6f };
    int   cannonFireFrame = 3;
    float shellSpeed      = 210.f;
    float missileSpeed    = 190.f;
    Vec2  projBox         = { 26.f, 26.f };
    int   smokeStepTicks  = 3;
    int   blastHoldTicks  = 40;
    Vec2  blastBox        = { 150.f, 110.f };
    Vec2  casingVel       = { 70.f, -150.f };

    // 선체 피격 흡수 상자 (로컬 좌표: minX, minY, maxX, maxY)
    ARRAY<float, 4 * kBigShieeHullBoxCount> hullBoxes = {};
    float                                   playerWallX = 220.f;   // 플레이어가 넘지 못하는 선 (선체 왼쪽 + 이만큼)

    // 파괴
    int   dyingBlastTicks   = 220;   // 연쇄 폭발 구간
    int   dyingCollapseTick = 252;   // 잔해로 바뀌는 시점
    int   dyingDoneTick     = 450;   // 연출 종료
    Vec2  wreckOffset       = { 297.f, 0.f };
    Vec2  wreckCannonOffset = { 200.f, 0.f };
};

}   // namespace slug

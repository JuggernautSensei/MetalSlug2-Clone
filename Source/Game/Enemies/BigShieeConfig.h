#pragma once
#include "Base/Types.h"

inline constexpr int kBigShieeTurretCount = 7;
inline constexpr int kBigShieeCannonPart  = 7;

// 렌더 레이어
inline constexpr int kBigShieeDepthHull    = -100;
inline constexpr int kBigShieeDepthAntenna = -99;
inline constexpr int kBigShieeDepthCannon  = -98;
inline constexpr int kBigShieeDepthTurret  = -97;
inline constexpr int kBigShieeDepthDisc    = -96;

// 최종보스 전함 '빅 셰이' 설정
struct BigShieeTurretConfig
{
    char    letter    = 'A';
    int     side      = 0;
    bool    bTwin     = true;
    VECTOR2 anchor    = { 0.f, 0.f };
    VECTOR2 boxSize   = { 70.f, 56.f };
    VECTOR2 boxOffset = { 0.f, -30.f };
    VECTOR2 muzzle    = { -22.f, -40.f };
    float   muzzleGap = 16.f;
    int     hp        = 40;
};

struct BigShieeConfig
{
    // 부품 깊이
    EnemyPartDepth                              partHull    = { kBigShieeDepthHull, 0 };
    EnemyPartDepth                              partAntenna = { kBigShieeDepthAntenna, 0 };
    EnemyPartDepth                              partCannon  = { kBigShieeDepthCannon, 0 };
    EnemyPartDepth                              partDisc    = { kBigShieeDepthDisc, 0 };
    EnemyPartDepth                              partWreck   = { kBigShieeDepthHull, 0 };
    ARRAY<EnemyPartDepth, kBigShieeTurretCount> partTurrets = {
        { { kBigShieeDepthTurret, 0 }, { kBigShieeDepthTurret, 1 }, { kBigShieeDepthTurret, 2 }, { kBigShieeDepthTurret, 3 }, { kBigShieeDepthTurret, 4 }, { kBigShieeDepthTurret, 5 }, { kBigShieeDepthTurret, 6 } }
    };

    ARRAY<BigShieeTurretConfig, kBigShieeTurretCount> turrets     = {};
    int                                               turretScore = 1000;
    VECTOR2                                           antennaP    = { 0.f, 0.f };   // 선체 위 리브 패널
    VECTOR2                                           antennaQ    = { 0.f, 0.f };

    float              startScreenX    = -230.f;
    ARRAY<VECTOR2, 24> entranceKeys    = {};
    int                numEntranceKeys = 0;

    float             scrollSpeed         = 20.f;
    float             scrollStartSec      = 1.4f;
    float             scrollBudgetPx      = 0.f;   // 보스 생성 뒤 누적 스크롤이 이만큼이면 카메라가 멈춘다
    ARRAY<VECTOR2, 8> scrollPauses        = {};
    int               numScrollPauses     = 0;
    float             deployPauseDelaySec = 0.8f;
    float             deployPauseSec      = 9.5f;
    float             latePauseFirstSec   = 12.5f;
    float             latePausePeriodSec  = 19.f;
    float             latePauseSec        = 1.2f;

    // driveTargetX 까지 돌진해 선수 를 화면 폭의 약 90% 에 되돌린다
    float driveTriggerX = 200.f;
    float driveTargetX  = 385.f;
    float driveSpeed    = 65.f;
    float driveAccel    = 160.f;

    // shuttleDriftSec 뒤 driveTargetX 로 돌진
    float shuttleDriftSec = 0.f;
    float reverseAtSec    = 0.f;
    float reverseSec      = 0.f;
    float reverseSpeed    = 0.f;

    ARRAY<float, 2> volleyIntervalSec = { 1.0f, 2.8f };
    float           firstVolleySec    = 4.5f;
    int             prepTicks         = 18;
    int             turnFrameTicks    = 3;
    int             shotTicks         = 60;
    int             fireTick          = 16;
    int             pairGapTicks      = 10;
    float           turretAimBandPx   = 72.f;
    int             turretTrackTicks  = 0;   // 쉬는 포탑이 조준 위치로 한 칸 도는 최소 간격

    VECTOR2         cannonAnchor         = { 30.f, -170.f };   // 주포 받침
    VECTOR2         cannonBoxSize        = { 120.f, 190.f };
    VECTOR2         cannonBoxOffset      = { 32.f, -265.f };
    VECTOR2         cannonAimOrigin      = { 110.5f, -301.f };
    float           deployDelaySec       = 0.5f;
    float           firstMissileSec      = 1.2f;
    float           missileIntervalSec   = 3.0f;
    int             missileFireTick      = 20;
    int             hullStepTicks        = 15;    // 전개 중 선체 셀 이 바뀌는 간격
    float           cannonDeployDelaySec = 0.f;   // 선수가 열리기 시작한 뒤 주포 전개 가 시작되기까지
    int             cannonDeployTicks    = 0;     // 주포 전개 길이
    int             cannonTurnTicks      = 4;
    ARRAY<float, 5> cannonAimDeg         = { -47.f, -22.f, 0.f, 22.f, 47.f };
    ARRAY<float, 5> cannonMuzzleX        = { -42.f, -24.f, 0.f, 24.f, 42.f };

    VECTOR2 casingOffset    = { 60.f, -20.f };
    float   casingScale     = 2.f;
    VECTOR2 casingVel       = { 36.f, -60.f };
    float   casingGravity   = 900.f;
    float   casingGroundDy  = 0.f;
    float   casingLingerSec = 0.2f;
    float   casingFps       = 12.f;

    float chuteFirstSec     = 6.f;
    float chuteIntervalSec  = 15.f;
    float chuteSpreadX      = 60.f;
    float chuteRightMarginX = 120.f;
    float chuteStartAboveY  = 40.f;

    // 체력이 줄수록 선체 위에 아주 작은 폭발이 무작위로 터진다
    float           lowHpFxBelow          = 0.6f;
    ARRAY<float, 2> lowHpFxSec            = { 1.2f, 0.25f };
    ARRAY<float, 4> lowHpFxArea           = { -210.f, -330.f, 210.f, -60.f };
    String          lowHpFxClip           = "Explosion_Small";
    float           dyingBlastIntervalSec = 0.12f;
    float           dyingWreckSec         = 6.3f;
    float           dyingDoneSec          = 8.4f;
    float           wreckSmokeIntervalSec = 0.35f;
    VECTOR2         wreckOffset           = { 0.f, 0.f };
    VECTOR2         cannonWreckOffset     = { 150.f, -60.f };

    // 파괴 연출 위치
    float   chuteEdgeX        = 30.f;
    VECTOR2 turretBlastOffset = { 0.f, -24.f };
    VECTOR2 turretSmokeOffset = { 0.f, -10.f };
    VECTOR2 deathFlameOffset  = { 0.f, 40.f };
    float   deathBlastDy      = 120.f;
    VECTOR2 deathHugeOffset   = { 20.f, -150.f };
    VECTOR2 deathDiscOffset   = { -40.f, -170.f };
};

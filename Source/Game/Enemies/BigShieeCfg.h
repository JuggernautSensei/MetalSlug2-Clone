#pragma once
#include "Base/Math.h"
#include "Base/Types.h"

inline constexpr int kBigShieeTurretCount = 7;
inline constexpr int kBigShieeCannonPart  = 7;   // 판정 파트 번호: 0..6 포탑, 7 주포

// 렌더 레이어 (SpriteBatch 는 레이어 오름차순으로 그린다). 선체/포탑/주포/잔해는 모든 스프라이트 중 가장 뒤:
// 하늘/배경 텍스처 바로 앞, 원경 (4), 소품 (background 11/12, front 38), 적 (15), 플레이어 (19/20), 이펙트 (29+) 보다 모두 뒤다.
inline constexpr int kBigShieeDepthHull    = -100;
inline constexpr int kBigShieeDepthAntenna = -99;
inline constexpr int kBigShieeDepthCannon  = -98;
inline constexpr int kBigShieeDepthTurret  = -97;
inline constexpr int kBigShieeDepthDisc    = -96;

// 최종보스 전함 '빅 셰이' 설정 (Docs/Spec_BigShiee.md). 길이는 전부 게임 px, 시간은 초 (틱으로 바꿔 쓴다).
//  - 원점(body.position) = 선체 셀 바닥 가운데 (스프라이트 피벗 = 선미 쪽 셀 폭 271 의 가운데 아래). 선수(X 문양)는 +x (오른쪽) 이다.
//  - 포탑/주포 좌표는 선체 원점 기준 (x 오른쪽 +, y 아래 +). 포탑 좌표는 정답 이미지 (Docs/img/BigShieeReference.png) 에서 실측한 값이다.
struct BigShieeTurretCfg
{
    char    letter    = 'A';
    int     side      = 0;   // 초기 방향: 0 = 왼쪽 (Idle1 계열), 1 = 오른쪽 (Idle3 계열). 쏘기 전에 플레이어 쪽으로 돈다
    bool    bTwin     = true;
    VECTOR2 anchor    = { 0.f, 0.f };
    VECTOR2 boxSize   = { 70.f, 56.f };
    VECTOR2 boxOffset = { 0.f, -30.f };
    VECTOR2 muzzle    = { -22.f, -40.f };
    float   muzzleGap = 16.f;
    int     hp        = 40;
};

struct BigShieeCfg
{
    // 부품 깊이 (params.parts.<name>). 기본값 = 위 kBigShieeDepth* 상수, 포탑 sort = 포탑 번호
    EnemyPartDepth                              partHull    = { kBigShieeDepthHull, 0 };
    EnemyPartDepth                              partAntenna = { kBigShieeDepthAntenna, 0 };
    EnemyPartDepth                              partCannon  = { kBigShieeDepthCannon, 0 };
    EnemyPartDepth                              partDisc    = { kBigShieeDepthDisc, 0 };
    EnemyPartDepth                              partWreck   = { kBigShieeDepthHull, 0 };
    ARRAY<EnemyPartDepth, kBigShieeTurretCount> partTurrets = {
        { { kBigShieeDepthTurret, 0 }, { kBigShieeDepthTurret, 1 }, { kBigShieeDepthTurret, 2 }, { kBigShieeDepthTurret, 3 }, { kBigShieeDepthTurret, 4 }, { kBigShieeDepthTurret, 5 }, { kBigShieeDepthTurret, 6 } }
    };

    ARRAY<BigShieeTurretCfg, kBigShieeTurretCount> turrets     = {};
    int                                            turretScore = 1000;
    VECTOR2                                        antennaP    = { 0.f, 0.f };   // 선체 위 리브 패널 (정답 이미지에서 실측)
    VECTOR2                                        antennaQ    = { 0.f, 0.f };

    // 입장: 선체 가운데가 카메라 왼쪽 + startScreenX 에서 출발해 화면 기준 위치 표 (초, 화면 x) 를 따라 들어온다. 표가 끝나면 월드에 고정된다
    float              startScreenX    = -230.f;
    ARRAY<VECTOR2, 24> entranceKeys    = {};
    int                numEntranceKeys = 0;

    // 자동 스크롤 (카메라): 보스가 살아 있는 동안 px/s. 멈춤 구간은 보스 생성 기준 (시작, 길이) 초
    float             scrollSpeed         = 20.f;
    float             scrollStartSec      = 1.4f;
    float             scrollBudgetPx      = 0.f;   // 보스 생성 뒤 누적 스크롤이 이만큼이면 카메라가 멈춘다 (아레나 끝). 0 = 끝없이
    ARRAY<VECTOR2, 8> scrollPauses        = {};
    int               numScrollPauses     = 0;
    float             deployPauseDelaySec = 0.8f;
    float             deployPauseSec      = 9.5f;
    float             latePauseFirstSec   = 12.5f;
    float             latePausePeriodSec  = 19.f;
    float             latePauseSec        = 1.2f;

    // 선체 주행: 입장이 끝난 뒤 월드에 고정되어 카메라 스크롤에 밀려 왼쪽으로 흐른다. 화면 x 가 driveTriggerX 아래로 가면 (원작: 약 9 초마다)
    // driveTargetX 까지 돌진해 선수 (X 문양) 를 화면 폭의 약 90% 에 되돌린다. 주포 전개 중에는 하지 않는다
    float driveTriggerX = 200.f;
    float driveTargetX  = 385.f;
    float driveSpeed    = 65.f;
    float driveAccel    = 160.f;

    // 아레나 끝 왕복 (와리가리): 카메라가 멈춘 뒤에는 선체가 스스로 scrollSpeed 로 왼쪽으로 흐르다가 (카메라가 가던 것과 같은 상대 운동)
    // shuttleDriftSec 뒤 driveTargetX 로 돌진한다. 0 = 끔. 주포 전개 시작 뒤 reverseAtSec 에 reverseSec 동안 한 번 후진 (reverseSec 0 = 없음)
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
    int             turretTrackTicks  = 0;   // 쉬는 포탑이 조준 위치로 한 칸 도는 최소 간격 (틱). 0 = 쉬는 동안 안 돈다

    VECTOR2         cannonAnchor         = { 30.f, -170.f };   // 주포 받침 (선체 원점 기준). 선체 가운데 상부 (X 문양 쪽 오른쪽) 에서 올라온다
    VECTOR2         cannonBoxSize        = { 120.f, 190.f };
    VECTOR2         cannonBoxOffset      = { 32.f, -265.f };
    VECTOR2         cannonAimOrigin      = { 110.5f, -301.f };
    float           deployDelaySec       = 0.5f;
    float           firstMissileSec      = 1.2f;
    float           missileIntervalSec   = 3.0f;
    int             missileFireTick      = 20;
    int             hullStepTicks        = 15;    // 전개 중 선체 셀 (D1 -> D3 -> D6 -> D9 -> Open) 이 바뀌는 간격 (틱)
    float           cannonDeployDelaySec = 0.f;   // 선수가 열리기 시작한 뒤 주포 전개 (Boss_Cannon_Deploy) 가 시작되기까지 (0 = 동시)
    int             cannonDeployTicks    = 0;     // 주포 전개 길이 (틱). 0 = Boss_Cannon_Deploy 클립 길이
    int             cannonTurnTicks      = 4;
    ARRAY<float, 5> cannonAimDeg         = { -47.f, -22.f, 0.f, 22.f, 47.f };   // 사격 행 (Shot1..5) 별 발사 각도: 수직 아래 0, 오른쪽 +. 홀수 yaw 는 이웃 두 행의 가운데
    ARRAY<float, 5> cannonMuzzleX        = { -42.f, -24.f, 0.f, 24.f, 42.f };

    // 탄피: 고폭탄을 쏠 때 포구 + casingOffset 에서 튀어 나와 해석적으로 떨어진다 (판정 없음, 슬롯 1 개)
    VECTOR2 casingOffset    = { 60.f, -20.f };
    float   casingScale     = 2.f;
    VECTOR2 casingVel       = { 36.f, -60.f };   // px/s (y 는 아래 +, 음수 = 살짝 위로 튄다)
    float   casingGravity   = 900.f;
    float   casingGroundDy  = 0.f;
    float   casingLingerSec = 0.2f;
    float   casingFps       = 12.f;

    // 낙하산 인질 보급 (pow_chute): 보스가 살아 있는 동안 chuteFirstSec 뒤부터 chuteIntervalSec 마다 카메라 위쪽 밖에서 떨어진다.
    float chuteFirstSec     = 6.f;
    float chuteIntervalSec  = 15.f;
    float chuteSpreadX      = 60.f;
    float chuteRightMarginX = 120.f;
    float chuteStartAboveY  = 40.f;

    // 파괴 (사망 시점 기준)
    // 체력이 줄수록 선체 위에 아주 작은 폭발이 무작위로 터진다 (남은 체력 비율 < lowHpFxBelow 부터, 간격은 lowHpFxSec[0] -> [1] 로 짧아진다)
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

    // 파괴 연출 위치 (선체/포탑 원점 기준)
    float   chuteEdgeX        = 30.f;
    VECTOR2 turretBlastOffset = { 0.f, -24.f };
    VECTOR2 turretSmokeOffset = { 0.f, -10.f };
    VECTOR2 deathFlameOffset  = { 0.f, 40.f };
    float   deathBlastDy      = 120.f;
    VECTOR2 deathHugeOffset   = { 20.f, -150.f };
    VECTOR2 deathDiscOffset   = { -40.f, -170.f };
};

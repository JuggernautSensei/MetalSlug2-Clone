#pragma once
#include "Base/Math.h"
#include "Base/Types.h"

inline constexpr int kBigShieeTurretCount = 7;   // 시트 포탑 A..G (쌍발 A-D, 단발 E-G)
inline constexpr int kBigShieeCannonPart  = 7;   // 판정 파트 번호: 0..6 포탑, 7 주포

// 렌더 레이어 (SpriteBatch 는 레이어 오름차순으로 그린다). 선체/포탑/주포/잔해는 모든 스프라이트 중 가장 뒤:
// 하늘/배경 텍스처 바로 앞, 원경 (4), 소품 (background 11/12, front 38), 적 (15), 플레이어 (19/20), 이펙트 (29+) 보다 모두 뒤다.
inline constexpr int kBigShieeDepthHull    = -100;
inline constexpr int kBigShieeDepthAntenna = -99;
inline constexpr int kBigShieeDepthCannon  = -98;
inline constexpr int kBigShieeDepthTurret  = -97;
inline constexpr int kBigShieeDepthDisc    = -96;   // 사망 연출의 검은 원: 선체 앞

// 최종보스 전함 '빅 셰이' 설정 (Docs/Spec_BigShiee.md). 길이는 전부 게임 px, 시간은 초 (틱으로 바꿔 쓴다).
//  - 원점(body.position) = 선체 셀 바닥 가운데 (스프라이트 피벗 = 선미 쪽 셀 폭 271 의 가운데 아래). 선수(X 문양)는 +x (오른쪽) 이다.
//  - 포탑/주포 좌표는 선체 원점 기준 (x 오른쪽 +, y 아래 +). 포탑 좌표는 정답 이미지 (Docs/img/BigShieeReference.png) 에서 실측한 값이다.
struct BigShieeTurretCfg
{
    char  letter    = 'A';                // 시트 포탑 문자 (클립 Boss_Turret_<letter>_*)
    int   side      = 0;                  // 초기 방향: 0 = 왼쪽 (Idle1 계열), 1 = 오른쪽 (Idle3 계열). 쏘기 전에 플레이어 쪽으로 돈다
    bool  bTwin     = true;               // 쌍발 (포탄 2발)
    Vec2  anchor    = { 0.f, 0.f };       // 포탑 받침 앵커
    Vec2  boxSize   = { 70.f, 56.f };     // 피격 상자 크기
    Vec2  boxOffset = { 0.f, -30.f };     // 피격 상자 중심 (앵커 기준)
    Vec2  muzzle    = { -22.f, -40.f };   // 왼쪽을 본 자세의 포구 중심 (앵커 기준)
    float muzzleGap = 16.f;               // 쌍발 포구 간격 (x)
    int   hp        = 40;
};

struct BigShieeCfg
{
    // 부품 깊이 (params.parts.<name>). 기본값 = 위 kBigShieeDepth* 상수, 포탑 sort = 포탑 번호
    EnemyPartDepth partHull    = { kBigShieeDepthHull, 0 };
    EnemyPartDepth partAntenna = { kBigShieeDepthAntenna, 0 };
    EnemyPartDepth partCannon  = { kBigShieeDepthCannon, 0 };
    EnemyPartDepth partDisc    = { kBigShieeDepthDisc, 0 };
    EnemyPartDepth partWreck   = { kBigShieeDepthHull, 0 };
    ARRAY<EnemyPartDepth, kBigShieeTurretCount> partTurrets = { { { kBigShieeDepthTurret, 0 }, { kBigShieeDepthTurret, 1 }, { kBigShieeDepthTurret, 2 }, { kBigShieeDepthTurret, 3 }, { kBigShieeDepthTurret, 4 }, { kBigShieeDepthTurret, 5 }, { kBigShieeDepthTurret, 6 } } };

    ARRAY<BigShieeTurretCfg, kBigShieeTurretCount> turrets = {};
    int                                            turretScore = 1000;
    Vec2                                           antennaP    = { 0.f, 0.f };   // 선체 위 리브 패널 (정답 이미지에서 실측)
    Vec2                                           antennaQ    = { 0.f, 0.f };

    // 입장: 선체 가운데가 카메라 왼쪽 + startScreenX 에서 출발해 화면 기준 위치 표 (초, 화면 x) 를 따라 들어온다. 표가 끝나면 월드에 고정된다
    float          startScreenX    = -230.f;
    ARRAY<Vec2, 24> entranceKeys   = {};
    int            numEntranceKeys = 0;

    // 자동 스크롤 (카메라): 보스가 살아 있는 동안 px/s. 멈춤 구간은 보스 생성 기준 (시작, 길이) 초
    float          scrollSpeed         = 20.f;
    float          scrollStartSec      = 1.4f;   // 카메라 락 활강이 끝난 뒤부터
    float          scrollBudgetPx      = 0.f;    // 보스 생성 뒤 누적 스크롤이 이만큼이면 카메라가 멈춘다 (아레나 끝). 0 = 끝없이
    ARRAY<Vec2, 8> scrollPauses        = {};
    int            numScrollPauses     = 0;
    float          deployPauseDelaySec = 0.8f;   // 주포 전개 시작 기준: 이 시점부터 deployPauseSec 동안 멈춘다
    float          deployPauseSec      = 9.5f;
    float          latePauseFirstSec   = 12.5f;  // 주포 전개 시작 기준: 이후 주기적인 짧은 멈춤
    float          latePausePeriodSec  = 19.f;
    float          latePauseSec        = 1.2f;

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

    // 포탑 포격 (한 번에 한 문)
    ARRAY<float, 2> volleyIntervalSec = { 1.0f, 2.8f };
    float           firstVolleySec    = 4.5f;   // 보스 생성 후 첫 포격
    int             prepTicks         = 18;
    int             turnFrameTicks    = 3;
    int             shotTicks         = 60;
    int             fireTick          = 16;   // Shot 클립 안에서 첫 포탄이 나가는 틱
    int             pairGapTicks      = 10;   // 쌍발 두 번째 포탄까지
    float           turretAimBandPx   = 72.f;  // 플레이어 발 x - 포구 x 가 이 안이면 위 (Idle2/Shot2), 밖이면 왼쪽 (Idle1/Shot1) / 오른쪽 (Idle3/Shot3)
    int             turretTrackTicks  = 0;     // 쉬는 포탑이 조준 위치로 한 칸 도는 최소 간격 (틱). 0 = 쉬는 동안 안 돈다

    // 주포
    Vec2  cannonAnchor     = { 30.f, -170.f };    // 주포 받침 (선체 원점 기준). 원작 V1 5:13~5:35 실측: 선체 가운데 상부 (X 문양 쪽 오른쪽) 에서 올라온다
    Vec2  cannonBoxSize    = { 120.f, 190.f };
    Vec2  cannonBoxOffset  = { 32.f, -265.f };    // 주포 피격 상자 중심 (선체 원점 기준)
    Vec2  cannonAimOrigin  = { 110.5f, -301.f };   // 플레이어 방향각 (고개 yaw) 기준점. 총구 위치와 분리
    float deployDelaySec   = 0.5f;                // 마지막 포탑 파괴 뒤 전개 시작
    float firstMissileSec  = 1.2f;                // 조준이 끝난 뒤 첫 발
    float missileIntervalSec = 3.0f;
    int   missileFireTick  = 20;                  // Shot 클립 안에서 고폭탄이 나가는 틱
    int   hullStepTicks    = 15;                  // 전개 중 선체 셀 (D1 -> D3 -> D6 -> D9 -> Open) 이 바뀌는 간격 (틱)
    float cannonDeployDelaySec = 0.f;             // 선수가 열리기 시작한 뒤 주포 전개 (Boss_Cannon_Deploy) 가 시작되기까지 (0 = 동시)
    int   cannonDeployTicks = 0;                  // 주포 전개 길이 (틱). 0 = Boss_Cannon_Deploy 클립 길이
    int   cannonTurnTicks  = 4;                   // 주포 머리가 Aim 프레임 한 칸 도는 틱
    ARRAY<float, 5> cannonAimDeg  = { -47.f, -22.f, 0.f, 22.f, 47.f };   // 사격 행 (Shot1..5) 별 발사 각도: 수직 아래 0, 오른쪽 +. 홀수 yaw 는 이웃 두 행의 가운데
    ARRAY<float, 5> cannonMuzzleX = { -42.f, -24.f, 0.f, 24.f, 42.f };   // 사격 행별 포구 x 변화 (가운데 행 대비): 머리가 도는 만큼 포신이 옮겨 간다. 포구 기준점 = weapons.missile.muzzle

    // 탄피 (고폭탄을 쏠 때 포구 + casingOffset 에서 튀어 나와 해석적으로 떨어진다. 판정 없음, 슬롯 1 개)
    Vec2  casingOffset    = { 60.f, -20.f };   // 포구 기준 출발 위치
    float casingScale     = 2.f;               // 탄피 그림 배율 (그림 가운데 기준)
    Vec2  casingVel       = { 36.f, -60.f };   // px/s (y 는 아래 +, 음수 = 살짝 위로 튄다)
    float casingGravity   = 900.f;
    float casingGroundDy  = 0.f;               // 지면 = 선체 원점 y + 이 값
    float casingLingerSec = 0.2f;              // 지면에 닿은 뒤 사라지기까지
    float casingFps       = 12.f;

    // 낙하산 인질 보급 (pow_chute): 보스가 살아 있는 동안 chuteFirstSec 뒤부터 chuteIntervalSec 마다 카메라 위쪽 밖에서 떨어진다.
    // x 는 화면 오른쪽 가장자리에서 chuteRightMarginX 안쪽 기준 +-chuteSpreadX 안 (결정적 난수). 땅에 닿으면 묶인 POW 로 변해 칼로 풀면 무기 (HMG/로켓/샷건/화염 중 난수)
    float chuteFirstSec    = 6.f;
    float chuteIntervalSec = 15.f;
    float chuteSpreadX     = 60.f;
    float chuteRightMarginX = 120.f;
    float chuteStartAboveY = 40.f;

    // 파괴 (사망 시점 기준)
    // 체력이 줄수록 선체 위에 아주 작은 폭발이 무작위로 터진다 (남은 체력 비율 < lowHpFxBelow 부터, 간격은 lowHpFxSec[0] -> [1] 로 짧아진다)
    float           lowHpFxBelow = 0.6f;
    ARRAY<float, 2> lowHpFxSec   = { 1.2f, 0.25f };
    ARRAY<float, 4> lowHpFxArea  = { -210.f, -330.f, 210.f, -60.f };   // 선체 기준 x0, y0, x1, y1
    String          lowHpFxClip  = "Explosion_Small";
    float dyingBlastIntervalSec = 0.12f;
    float dyingWreckSec         = 6.3f;   // 이 시점에 선체가 잔해로 바뀐다
    float dyingDoneSec          = 8.4f;   // 연출 종료 (destroyFinished)
    float wreckSmokeIntervalSec = 0.35f;
    Vec2  wreckOffset           = { 0.f, 0.f };
    Vec2  cannonWreckOffset     = { 150.f, -60.f };

    // 파괴 연출 위치 (선체/포탑 원점 기준)
    float chuteEdgeX         = 30.f;               // 낙하산 착지 화면 가장자리에서 이만큼 안쪽
    Vec2  turretBlastOffset  = { 0.f, -24.f };     // 포탑 파괴 폭발 (포탑 앵커 기준)
    Vec2  turretSmokeOffset  = { 0.f, -10.f };     // 포탑 파괴 연기 (폭발 위치 기준)
    Vec2  deathFlameOffset   = { 0.f, 40.f };      // 사망 주포 상단 화염 (주포 상단 기준)
    float deathBlastDy       = 120.f;              // 사망 연쇄 폭발 y 가산
    Vec2  deathHugeOffset    = { 20.f, -150.f };   // 사망 대폭발
    Vec2  deathDiscOffset    = { -40.f, -170.f };  // 사망 검은 원
};

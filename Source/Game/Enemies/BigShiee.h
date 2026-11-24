#pragma once
#include "Game/Enemies/BigShieeConfig.h"

// 전함 보스 단계
enum class eBigShieePhase
{
    Entrance,
    Battery,
    Deploy,
    Cannon
};

enum class eBigShieeTurretMode
{
    Idle,
    Prep,
    Turn,   // 머리를 플레이어 쪽 조준 위치 로 한 칸씩 돌린다
    Shot,
    Dead
};

enum class eBigShieeCannonMode
{
    Hidden,
    Deploy,
    Aim,
    Ready,
    Shot
};

struct BigShieeData
{
    ARRAY<int, kBigShieeTurretCount>                 turretHp       = {};
    ARRAY<eBigShieeTurretMode, kBigShieeTurretCount> turretMode     = {};
    ARRAY<int, kBigShieeTurretCount>                 turretTicks    = {};
    ARRAY<int, kBigShieeTurretCount>                 turretPosition = {};
    int                                              nextVolley     = 0;
    eBigShieeCannonMode                              cannonMode     = eBigShieeCannonMode::Hidden;
    int                                              cannonTicks    = 0;
    int                                              cannonRow      = 0;
    int                                              nextMissile    = 0;
    int                                              deployStart    = 0;
    int                                              moving         = 0;
    int                                              lastKill       = 0;
    int                                              scrollTicks    = 0;
    int                                              driving        = 0;
    int                                              speedX         = 0;
    int                                              nextChute      = 0;
    int                                              cannonYaw      = 0;
    int                                              cannonTurn     = 0;
    int                                              lidStart       = 0;
    int                                              casingTick     = 0;
    int                                              casingX        = 0;
    int                                              casingY        = 0;
    int                                              nextDamageFx   = 0;
    int                                              driftTicks     = 0;
};

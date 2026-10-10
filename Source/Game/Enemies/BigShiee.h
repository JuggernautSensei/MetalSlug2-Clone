#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/BigShieeCfg.h"

namespace slug
{

// 보스 단계 (Docs/Spec_BigShiee.md 3절)
enum class eBigShieePhase : int
{
    Entrance,   // 선체가 왼쪽에서 들어온다 (포탑은 일부 시간 뒤부터 쏜다)
    Turrets,    // 포탑 A..G 단계
    Deploy,     // 선체 전개 + 캐논 상승
    CannonA,    // 캐논 (왼쪽을 보고 곡사 포탄)
    Turn,       // 캐논이 정면으로 돈다
    CannonB,    // 해골 얼굴 (입에서 연기 꼬리 미사일)
    Dying       // 연쇄 폭발 -> 붕괴 -> 잔해
};

enum class eBigShieeTurretState : int
{
    Idle,
    Fire,
    Dead
};

struct BigShieeTurret
{
    eBigShieeTurretState state    = eBigShieeTurretState::Idle;
    int                  hp       = 0;
    int                  ticks    = 0;   // 현재 상태 경과
    int                  cooldown = 0;   // 다음 발사까지 (Idle)
    bool                 bFired   = false;
};

// 포탑 쌍탄: 위로 올라갔다가 느리게 떨어지는 곡사
struct BigShieeShell
{
    Vec2     pos      = { 0.f, 0.f };
    Vec2     vel      = { 0.f, 0.f };
    float    gravity  = 0.f;
    float    groundY  = 0.f;
    int      age      = 0;
    int      colorPhase = 0;
    uint32_t hitId    = 0;
    bool     bAlive   = true;
};

// 캐논 포탄/미사일: 플레이어 쪽으로 곧게 날며 연기 꼬리를 남긴다
struct BigShieeMissile
{
    Vec2     pos     = { 0.f, 0.f };
    Vec2     vel     = { 0.f, 0.f };
    float    groundY = 0.f;
    int      age     = 0;
    int      row     = 3;   // 스프라이트 줄 (1 기울어짐 .. 3 수직)
    bool     bFlipX  = false;
    uint32_t hitId   = 0;
    bool     bAlive  = true;
};

// 판정만 가진 폭발 영역 (불기둥, 포탄 착탄). 모양은 효과 클립이 그린다.
struct BigShieeBlast
{
    Vec2     base       = { 0.f, 0.f };   // 바닥 중심
    Vec2     size       = { 0.f, 0.f };   // 가득 찬 크기
    int      age        = 0;
    int      riseTicks  = 1;
    int      holdTicks  = 0;
    uint32_t hitId      = 0;
};

struct BigShieeCasing
{
    Vec2 pos = { 0.f, 0.f };
    Vec2 vel = { 0.f, 0.f };
    int  age = 0;
};

struct BigShieeState
{
    BigShieeCfg    cfg        = {};
    eBigShieePhase phase      = eBigShieePhase::Entrance;
    int            phaseTicks = 0;
    int            totalTicks = 0;

    float restX      = 0.f;   // 선체가 멈추는 발 x (스폰 위치)
    float travel     = 0.f;   // 등장 이동 총거리
    float curSpeed   = 0.f;
    int   hullStage  = 0;     // 전개 단계 (0 = 평상시 .. 5 = Open)
    int   cannonHpA  = 0;
    int   cannonHpB  = 0;
    bool  bCannonUp  = false;   // 캐논 전개 끝 (피격 가능)
    bool  bFormB     = false;

    int  cannonFireTimer  = 0;
    int  cannonShotTicks  = -1;
    bool bCannonFired     = false;
    int  turnTicks        = 0;

    ARRAY<BigShieeTurret, kBigShieeTurretCount> turrets = {};

    Vector<BigShieeShell>   shells   = {};
    Vector<BigShieeMissile> missiles = {};
    Vector<BigShieeBlast>   blasts   = {};
    Vector<BigShieeCasing>  casings  = {};

    int  dyingTicks   = 0;
    bool bWreck       = false;
    int  totalHp      = 0;
    bool bInit        = false;
};

}   // namespace slug

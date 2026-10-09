#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Render/Animation.h"
#include "World/Combat.h"
#include "World/Physics.h"

namespace slug
{

// ===========================================
//  소유자 ID 규약 (HitResolver 의 owner)
//   플레이어 = 1, 훈련 표적 = 1000+, 적 = 2000+ (스폰 순서대로 증가, 재사용 없음).
//   방패 판정 상자는 같은 적의 ID 에 kShieldOwnerBit 를 얹은 별도 소유자로 등록한다.
// ===========================================

inline constexpr uint32_t kEnemyOwnerBase = 2000;
inline constexpr uint32_t kShieldOwnerBit = 0x40000000u;

// 보스 부가 파츠(포탑 등) 소유자: 적 ID 의 하위 24비트에 (파트 번호 + 1) << 24 를 얹는다. EnemyWorld 는 kEnemyIdMask 로 적을 찾는다.
inline constexpr uint32_t kEnemyIdMask = 0x00FFFFFFu;
[[nodiscard]] constexpr uint32_t MakePartOwner(
    const uint32_t _enemyId,
    const uint32_t _partIndex)
{
    return (_enemyId & kEnemyIdMask) | ((_partIndex + 1u) << 24);
}
// 파트 번호 (파트가 아니면 -1)
[[nodiscard]] constexpr int GetPartIndex(
    const uint32_t _owner)
{
    const uint32_t part = (_owner >> 24) & 0x3Fu;
    return part == 0u ? -1 : static_cast<int>(part) - 1;
}

[[nodiscard]] constexpr bool IsEnemyOwner(
    const uint32_t _owner)
{
    const uint32_t id = _owner & ~kShieldOwnerBit;
    return id >= kEnemyOwnerBase && id < kShieldOwnerBit;
}

// ===========================================
//  EnemyHandle
//   ID 가 곧 핸들이다 (단조 증가, 재사용 없음 → 세대 검사가 필요 없고 낡은 핸들은 항상 "없음" 으로 조회된다).
//   0 = null.
// ===========================================

struct EnemyHandle
{
    uint32_t id = 0;

    [[nodiscard]] constexpr bool IsNull() const { return id == 0; }
    [[nodiscard]] constexpr bool operator==(const EnemyHandle&) const = default;
};

// ===========================================
//  SpawnParams
// ===========================================

struct SpawnParams
{
    int         facing     = -1;      // -1 왼쪽 / +1 오른쪽
    int         hp         = -1;      // -1: def 기본값
    slug::String drop       = {};      // 사망(포로는 구출) 시 줄 아이템 ID. 비면 없음
    int         aiVariant  = 0;       // 행동별 변종 (Soldier: 1 = 제자리 고정 — 걷지 않고 던지기/베기만)
    float       delaySec   = 0.f;     // 이 시간 뒤에 등장 (그 동안은 판정/렌더 없음)
    uint32_t    userTag    = 0;       // 호출자(스테이지)가 붙이는 값. 이벤트에 그대로 실려 나온다. 0 = 없음
    bool        bActivated = false;   // true: 화면 검사 없이 즉시 활성
    Aabb        arena      = {};      // 보스 전용 전투 구역 (비면 없음). 가로 기준은 arena.min.x / 바닥은 스폰 발 y
};

// ===========================================
//  EnemyEvent — 스테이지/HUD/아이템 쪽이 반응할 일들 (EnemyWorld::TakeEvents)
// ===========================================

enum class eEnemyEventKind : uint8_t
{
    Died,          // HP 가 0 이 됨 (score / drop / pos)
    ItemDropped,   // 포로가 아이템을 건넴 (drop / pos)
    Freed,         // 포로가 풀려남
    Transformed,   // 다른 정의로 변신 (낙하산 → 병사). other 가 새 핸들
    Removed        // 시체가 사라지거나 화면 밖으로 정리됨 (처치되지 않고 사라진 경우에도 발생)
};

struct EnemyEvent
{
    eEnemyEventKind kind     = eEnemyEventKind::Died;
    EnemyHandle     handle   = {};
    EnemyHandle     other    = {};
    int             defIndex = -1;
    slug::String     defName  = {};
    Vec2    pos      = { 0.f, 0.f };   // 발 위치
    int             score    = 0;
    slug::String     drop     = {};
    uint32_t        userTag  = 0;
    eHitKind        hitKind  = eHitKind::Bullet;   // Died: 마지막으로 맞은 공격 종류
};

// ===========================================
//  EnemyContext — 틱마다 호출자가 채워 넘기는 외부 서비스 (비소유)
// ===========================================

struct EnemyPlayerInfo
{
    Vec2 feet           = { 0.f, 0.f };
    Vec2 center         = { 0.f, 0.f };   // 피격 상자 중심 (조준 목표)
    Aabb         bodyBox        = {};             // 몸통 (포로 접촉 판정)
    bool         bAlive         = true;
    bool         bInvulnerable  = false;
};

struct EnemyContext
{
    const Tuning*           pTuning      = nullptr;   // 물리 수치(maxStepUp 등). null 이면 기본값
    PhysicsWorld            physics      = {};        // 지면/플랫폼/벽 (카메라 왼쪽 벽 같은 플레이어 전용 벽은 넣지 않는다)
    CombatSystem*           pCombat      = nullptr;
    ProjectileSystem*       pProjectiles = nullptr;
    EnemyPlayerInfo         player       = {};
    Aabb                    cameraView   = {};        // 현재 화면 영역 (월드). 활성화/정리 기준
    const AnimationLibrary* pAnims       = nullptr;   // 연출(이펙트) 용. null 이면 이펙트는 생략
};

// ===========================================
//  Enemy — 값 타입 한 마리 분량의 상태 (가상 함수 없음)
//   행동별 상태 번호는 각 행동 파일(Soldier.cpp 등)의 enum 이 정의한다.
// ===========================================

enum class eEnemyLife : uint8_t
{
    Pending,   // delay 대기
    Alive,
    Dying,     // 사망 연출 중 (판정 없음)
    Gone       // 제거 예정 (틱 끝에 정리됨)
};

struct Enemy
{
    uint32_t       id        = 0;
    int            defIndex  = -1;
    eEnemyBehavior behavior  = eEnemyBehavior::Soldier;
    eEnemyLife     life      = eEnemyLife::Alive;
    int            delayTicks = 0;

    Body         body      = {};            // body.position = 발
    Vec2 prevFeet  = { 0.f, 0.f };
    int          facing    = -1;
    Health       health    = {};
    bool         bActive   = false;
    bool         bInit     = false;         // 첫 틱에 행동 init 을 불렀는가 (init 은 EnemyContext 가 필요해서 Spawn 이 아니라 Tick 에서 한다)
    bool         bOnOneWay = false;         // 이번 틱 원웨이 플랫폼 위에 서 있었는가
    bool         bGrounded = false;

    // --- 상태기계 ---
    int state      = 0;
    int stateTicks = 0;   // 현재 상태에 들어온 뒤 경과 틱 (진입 틱 = 0)
    int counter    = 0;   // 남은 발수 등
    int aux        = 0;   // 행동별 임의 정수
    int aux2       = 0;
    int aux3       = 0;
    int aux4       = 0;
    bool bTransformed = false;   // 다른 정의로 변신해 사라짐 (Removed 이벤트 없음)
    int  ext[24]      = {};      // 행동별 확장 정수 (보스 파츠 상태 등: 포탑 HP/방향/타이머). 의미는 각 행동 파일이 정한다 (BattleShip.h 참조)
    Aabb arena        = {};      // 스폰 시 받은 보스 전투 구역 (비면 없음)

    // --- 표시 ---
    int  clip      = -1;   // EnemyDef::clips 인덱스
    int  clipTicks = 0;    // 클립 시작 후 경과 틱
    bool bVisible  = true;
    int  overlayTicks = 0;

    // --- 공격 판정 (근접) ---
    uint32_t meleeHitId    = kNoHitId;
    int      meleeTicksLeft = 0;
    int      meleeWeapon   = -1;
    Vec2 meleeOrigin = { 0.f, 0.f };

    // --- 사망 ---
    int      deathPhase = 0;
    int      deathTicks = 0;   // 최종 클립 시작 후 경과 틱 (깜빡임/제거 타이머)
    eHitKind lastHitKind = eHitKind::Bullet;
    int      shotsFired = 0;

    // --- 방패 ---
    bool bShieldUp  = false;
    int  shieldDir  = 0;   // 0 앞 / 1 위 / 2 아래
    int  defenceTicks = 0;

    // --- 인스턴스 정보 ---
    int         aiVariant = 0;
    uint32_t    userTag   = 0;
    slug::String drop      = {};

    [[nodiscard]] Vec2 GetFeet() const { return body.position; }
    [[nodiscard]] bool         IsAlive() const { return life == eEnemyLife::Alive; }
};

// 동시에 같은 종류를 센다거나 일부만 걸러 낼 때 쓰는 조건
struct EnemyFilter
{
    int      defIndex = -1;   // -1: 전부
    uint32_t userTag  = 0;    // 0: 전부
    bool     bHostileOnly = true;   // true: Enemy 팀만 (포로 제외)
};

}   // namespace slug

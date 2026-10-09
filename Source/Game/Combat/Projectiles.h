#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Combat/CombatSystem.h"
#include "World/Combat.h"
#include "World/GroundMap.h"

namespace slug
{

// ===========================================
//  투사체 (탄 / 폭탄 / 폭발)
//   플레이어와 적이 같은 시스템을 쓴다. 투사체는 매 틱 자기 Hitbox 를 CombatSystem 에 등록할 뿐이고,
//   누구를 때릴 수 있는지는 HitResolver 의 팀 규칙이 정한다.
// ===========================================

struct BulletSpawn
{
    eTeam        team      = eTeam::Player;
    uint32_t     owner     = kNoOwner;
    jug::VECTOR2 position  = { 0.f, 0.f };   // 총구
    jug::VECTOR2 sweepFrom = { 0.f, 0.f };   // 첫 틱 판정이 훑는 시작점 (몸통 중심): 총구가 표적 뒤로 넘어가도 맞는다
    jug::VECTOR2 direction = { 1.f, 0.f };   // 단위 벡터
    float        speed     = 600.f;          // px/s
    int          damage    = 1;
    jug::VECTOR2 size      = { 14.f, 10.f };
    float        range     = 400.f;
    bool         bPierce   = false;
    uint8_t      tag       = 0;   // 호출자가 정하는 분류 (예: 무기 종류) — CountBullets 용
    eHitKind     kind      = eHitKind::Bullet;
};

struct Bullet
{
    jug::VECTOR2 position   = { 0.f, 0.f };
    jug::VECTOR2 renderPrev = { 0.f, 0.f };
    jug::VECTOR2 hitFrom    = { 0.f, 0.f };
    jug::VECTOR2 velocity   = { 0.f, 0.f };   // px/s
    jug::VECTOR2 size       = { 14.f, 10.f };
    float        travelled  = 0.f;
    float        range      = 400.f;
    int          damage     = 1;
    eTeam        team       = eTeam::Player;
    eHitKind     kind       = eHitKind::Bullet;
    uint32_t     owner      = kNoOwner;
    uint32_t     hitId      = kNoHitId;
    bool         bPierce    = false;
    bool         bAlive     = true;
    uint8_t      tag        = 0;
};

struct BombSpawn
{
    eTeam        team       = eTeam::Player;
    uint32_t     owner      = kNoOwner;
    jug::VECTOR2 position   = { 0.f, 0.f };
    jug::VECTOR2 velocity   = { 0.f, 0.f };   // px/s
    float        gravity    = 900.f;
    jug::VECTOR2 hitSize    = { 12.f, 12.f };   // 비행 중 표적과 닿는 크기
    int          damage     = 20;
    jug::VECTOR2 blastSize  = { 104.f, 72.f };
    float        blastUp    = 28.f;
    int          blastTicks = 10;
};

struct Bomb
{
    jug::VECTOR2 position   = { 0.f, 0.f };
    jug::VECTOR2 renderPrev = { 0.f, 0.f };
    jug::VECTOR2 velocity   = { 0.f, 0.f };
    BombSpawn    params     = {};
    bool         bAlive     = true;
};

struct Blast
{
    Aabb     box       = {};
    int      ticksLeft = 0;
    int      damage    = 0;
    eTeam    team      = eTeam::Player;
    uint32_t owner     = kNoOwner;
    uint32_t hitId     = kNoHitId;
};

// 이펙트 요청 (렌더 쪽이 애니메이션으로 바꾼다)
enum class eFxKind : uint8_t
{
    BulletImpact,
    BombExplosion
};

struct FxRequest
{
    eFxKind      kind     = eFxKind::BulletImpact;
    jug::VECTOR2 position = { 0.f, 0.f };
};

class ProjectileSystem
{
public:
    void Clear();

    void SpawnBullet(const BulletSpawn& _spawn, CombatSystem& _combat);
    void SpawnBomb(const BombSpawn& _spawn);

    // 한 틱 진행: 이동 → 지면/표적 접촉 처리 → Hitbox 등록. _pGround 는 nullptr 가능.
    void Step(const GroundMap* _pGround, CombatSystem& _combat, float _dt);

    // Resolve 결과를 반영한다: 비관통 탄은 적중 시 소멸. (Step 에서 이미 등록한 hitId 기준)
    void OnHitEvents(jug::Span<const HitEvent> _events, CombatSystem& _combat);

    [[nodiscard]] int CountBullets(uint32_t _owner, uint8_t _tag) const;
    [[nodiscard]] int CountBombs(uint32_t _owner) const;

    [[nodiscard]] jug::Span<const Bullet> GetBullets() const { return m_bullets; }
    [[nodiscard]] jug::Span<const Bomb>   GetBombs() const { return m_bombs; }
    [[nodiscard]] jug::Span<const Blast>  GetBlasts() const { return m_blasts; }

    // 이번 틱에 쌓인 이펙트 요청을 비우며 가져간다.
    [[nodiscard]] jug::Vector<FxRequest> TakeFx();

private:
    void ExplodeBomb_(Bomb& _bomb, jug::VECTOR2 _at, CombatSystem& _combat);

    jug::Vector<Bullet>    m_bullets = {};
    jug::Vector<Bomb>      m_bombs   = {};
    jug::Vector<Blast>     m_blasts  = {};
    jug::Vector<FxRequest> m_fx      = {};
};

}   // namespace slug

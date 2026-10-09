#pragma once
#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Combat/CombatSystem.h"
#include "World/Combat.h"
#include "World/GroundMap.h"

namespace slug
{
struct BulletSpawn
{
    eTeam        team      = eTeam::Player;
    uint32_t     owner     = kNoOwner;
    Vec2 position  = { 0.f, 0.f };   // 총구
    Vec2 sweepFrom = { 0.f, 0.f };
    Vec2 direction = { 1.f, 0.f };   // 단위 벡터
    float        speed     = 600.f;          // px/s
    int          damage    = 1;
    Vec2 size      = { 14.f, 10.f };
    float        range     = 400.f;
    bool         bPierce   = false;
    uint8_t      tag       = 0;
    eHitKind     kind      = eHitKind::Bullet;
    Vec2 blastSize = { 0.f, 0.f };   // 착탄/사거리 끝 폭발 상자 (0 = 폭발 없음)
    int          blastDamage = 0;
    int          blastTicks  = 0;
    bool         bTrail      = false;
};

struct Bullet
{
    Vec2 position   = { 0.f, 0.f };
    Vec2 renderPrev = { 0.f, 0.f };
    Vec2 hitFrom    = { 0.f, 0.f };
    Vec2 velocity   = { 0.f, 0.f };   // px/s
    Vec2 size       = { 14.f, 10.f };
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
    Vec2 blastSize  = { 0.f, 0.f };
    int          blastDamage = 0;
    int          blastTicks  = 0;
    bool         bTrail     = false;
    int          ageTicks   = 0;
};

struct BombSpawn
{
    eTeam        team       = eTeam::Player;
    uint32_t     owner      = kNoOwner;
    Vec2 position   = { 0.f, 0.f };
    Vec2 velocity   = { 0.f, 0.f };   // px/s
    float        gravity    = 900.f;
    Vec2 hitSize    = { 12.f, 12.f };   // 비행 중 표적과 닿는 크기
    int          damage     = 20;
    Vec2 blastSize  = { 104.f, 72.f };
    float        blastUp    = 28.f;
    int          blastTicks = 10;
};

struct Bomb
{
    Vec2 position   = { 0.f, 0.f };
    Vec2 renderPrev = { 0.f, 0.f };
    Vec2 velocity   = { 0.f, 0.f };
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

struct ShotSpawn
{
    eTeam        team            = eTeam::Enemy;
    uint32_t     owner           = kNoOwner;
    Vec2 position        = { 0.f, 0.f };
    Vec2 velocity        = { 0.f, 0.f };   // 초기 속도 px/s
    Vec2 accel           = { 0.f, 0.f };
    float        maxSpeed        = 0.f;
    float        gravity         = 0.f;       // px/s²
    float        maxFallSpeed    = 3000.f;    // px/s
    Vec2 size            = { 20.f, 20.f };
    int          damage          = 1;
    eHitKind     kind            = eHitKind::Bullet;
    int          lifetimeTicks   = 300;
    int          startDelayTicks = 0;      // 점화 대기: 이 틱 동안 제자리 (판정은 유효)
    bool         bExplodeOnSolid = true;
    uint16_t     visual          = 0;
    uint8_t      tag             = 0;

    float        rollSpeed       = 0.f;
    float        rollDirX        = 1.f;    // 굴러가는 방향 (±1)
    int          rollTicks       = 0;
    bool         bExpireFx       = false;
};

struct Shot
{
    Vec2 position   = { 0.f, 0.f };
    Vec2 renderPrev = { 0.f, 0.f };
    Vec2 hitFrom    = { 0.f, 0.f };
    Vec2 velocity   = { 0.f, 0.f };
    ShotSpawn    params     = {};
    uint32_t     hitId      = kNoHitId;
    int          ageTicks   = 0;
    bool         bAlive     = true;
    bool         bLanded    = false;
    int          landedAge  = 0;       // 착지한 시점의 ageTicks
};

struct ShotFx
{
    Vec2 position = { 0.f, 0.f };
    uint16_t     visual   = 0;
};

enum class eFxKind : uint8_t
{
    BulletImpact,
    BombExplosion,
    RocketExplosion,
    RocketSmoke
};

struct FxRequest
{
    eFxKind      kind     = eFxKind::BulletImpact;
    Vec2 position = { 0.f, 0.f };
};

class ProjectileSystem
{
public:
    void Clear();

    void SpawnBullet(const BulletSpawn& _spawn, CombatSystem& _combat);
    void SpawnBomb(const BombSpawn& _spawn);
    void SpawnShot(const ShotSpawn& _spawn, CombatSystem& _combat);

    void Step(const GroundMap* _pGround, CombatSystem& _combat, float _dt);

    void OnHitEvents(slug::Span<const HitEvent> _events, CombatSystem& _combat);

    [[nodiscard]] int CountBullets(uint32_t _owner, uint8_t _tag) const;
    [[nodiscard]] int CountBombs(uint32_t _owner) const;
    [[nodiscard]] int CountShots(uint32_t _owner) const;


    [[nodiscard]] slug::Span<const Bullet> GetBullets() const { return m_bullets; }
    [[nodiscard]] slug::Span<const Bomb>   GetBombs() const { return m_bombs; }
    [[nodiscard]] slug::Span<const Blast>  GetBlasts() const { return m_blasts; }
    [[nodiscard]] slug::Span<const Shot>   GetShots() const { return m_shots; }

    [[nodiscard]] slug::Vector<FxRequest> TakeFx();

    [[nodiscard]] slug::Vector<ShotFx> TakeShotFx();

private:
    void ExplodeBomb_(Bomb& _bomb, Vec2 _at, CombatSystem& _combat);
    void SpawnBulletBlast_(const Bullet& _bullet, CombatSystem& _combat);

    slug::Vector<Bullet>    m_bullets = {};
    slug::Vector<Bomb>      m_bombs   = {};
    slug::Vector<Blast>     m_blasts  = {};
    slug::Vector<FxRequest> m_fx      = {};
    slug::Vector<Shot>      m_shots   = {};
    slug::Vector<ShotFx>    m_shotFx  = {};
};
}

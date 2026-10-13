#pragma once
#include "Base/Types.h"
#include "Game/Combat/CombatSystem.h"
#include "World/Combat.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

struct BulletSpawn
{
    eTeam    team        = eTeam::Player;
    uint32_t owner       = kNoOwner;
    VECTOR2  position    = { 0.f, 0.f };
    VECTOR2  sweepFrom   = { 0.f, 0.f };
    VECTOR2  direction   = { 1.f, 0.f };   // 단위 벡터
    float    speed       = 600.f;
    int      damage      = 1;
    VECTOR2  size        = { 14.f, 10.f };
    float    range       = 400.f;
    bool     bPierce     = false;
    uint8_t  tag         = 0;
    eHitKind kind        = eHitKind::Bullet;
    VECTOR2  blastSize   = { 0.f, 0.f };
    int      blastDamage = 0;
    int      blastTicks  = 0;
    bool     bTrail      = false;
    uint32_t hitId       = kNoHitId;
    VECTOR2  sizeEnd     = { 0.f, 0.f };   // 0 이면 판정 크기 고정
    float    scaleStart  = 1.f;            // 표시 배율
    float    scaleEnd    = 1.f;
    int      z           = kDefaultZ;
    float    accel       = 0.f;
    float    maxSpeed    = 0.f;   // accel 로 오르는 속도 상한
};

struct Bullet
{
    VECTOR2  position     = { 0.f, 0.f };
    VECTOR2  renderPrev   = { 0.f, 0.f };
    VECTOR2  hitFrom      = { 0.f, 0.f };
    VECTOR2  velocity     = { 0.f, 0.f };
    VECTOR2  size         = { 14.f, 10.f };
    float    travelled    = 0.f;
    float    range        = 400.f;
    int      damage       = 1;
    eTeam    team         = eTeam::Player;
    eHitKind kind         = eHitKind::Bullet;
    uint32_t owner        = kNoOwner;
    uint32_t hitId        = kNoHitId;
    bool     bPierce      = false;
    bool     bAlive       = true;
    uint8_t  tag          = 0;
    VECTOR2  blastSize    = { 0.f, 0.f };
    int      blastDamage  = 0;
    int      blastTicks   = 0;
    bool     bTrail       = false;
    int      ageTicks     = 0;
    bool     bSharedHitId = false;
    VECTOR2  sizeStart    = { 0.f, 0.f };
    VECTOR2  sizeEnd      = { 0.f, 0.f };
    float    scaleStart   = 1.f;
    float    scaleEnd     = 1.f;
    int      z            = kDefaultZ;
    float    accel        = 0.f;
    float    maxSpeed     = 0.f;
};

struct BombSpawn
{
    eTeam    team       = eTeam::Player;
    uint32_t owner      = kNoOwner;
    VECTOR2  position   = { 0.f, 0.f };
    VECTOR2  velocity   = { 0.f, 0.f };
    float    gravity    = 900.f;
    VECTOR2  hitSize    = { 12.f, 12.f };
    int      damage     = 20;
    VECTOR2  blastSize  = { 104.f, 72.f };
    float    blastUp    = 28.f;
    int      blastTicks = 10;
    bool     bBounce    = false;
    int      fuseTicks  = 0;
    int      z          = kDefaultZ;
};

struct Bomb
{
    VECTOR2   position   = { 0.f, 0.f };
    VECTOR2   renderPrev = { 0.f, 0.f };
    VECTOR2   velocity   = { 0.f, 0.f };
    BombSpawn params     = {};
    bool      bAlive     = true;
    int       ageTicks   = 0;
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
    eTeam    team            = eTeam::Enemy;
    uint32_t owner           = kNoOwner;
    VECTOR2  position        = { 0.f, 0.f };
    VECTOR2  velocity        = { 0.f, 0.f };
    VECTOR2  accel           = { 0.f, 0.f };
    float    maxSpeed        = 0.f;
    float    gravity         = 0.f;
    float    maxFallSpeed    = 3000.f;
    VECTOR2  size            = { 20.f, 20.f };
    int      damage          = 1;
    eHitKind kind            = eHitKind::Bullet;
    int      lifetimeTicks   = 300;
    int      startDelayTicks = 0;
    bool     bExplodeOnSolid = true;
    uint16_t visual          = 0;
    uint8_t  tag             = 0;

    float rollSpeed    = 0.f;
    float rollDirX     = 1.f;   // 굴러가는 방향
    int   rollTicks    = 0;
    bool  bExpireFx    = false;
    float groundProbeY = 0.f;

    VECTOR2 blastSize   = { 0.f, 0.f };   // 터질 때 남기는 폭발 판정
    float   blastUp     = 0.f;
    int     blastTicks  = 0;
    int     blastDamage = 1;
    int     z           = kDefaultZ;
};

struct Shot
{
    VECTOR2   position   = { 0.f, 0.f };
    VECTOR2   renderPrev = { 0.f, 0.f };
    VECTOR2   hitFrom    = { 0.f, 0.f };
    VECTOR2   velocity   = { 0.f, 0.f };
    ShotSpawn params     = {};
    uint32_t  hitId      = kNoHitId;
    int       ageTicks   = 0;
    bool      bAlive     = true;
    bool      bLanded    = false;
    int       landedAge  = 0;
};

struct ShotFx
{
    VECTOR2  position = { 0.f, 0.f };
    uint16_t visual   = 0;
    int      zOffset  = 0;
};

enum class eFxKind
{
    BulletImpact,
    BombExplosion,
    RocketExplosion,
    RocketSmoke
};

struct FxRequest
{
    eFxKind kind     = eFxKind::BulletImpact;
    VECTOR2 position = { 0.f, 0.f };
    int     zOffset  = 0;
};

class ProjectileSystem
{
public:
    void                             Clear();
    void                             SpawnBullet(const BulletSpawn& _spawn, CombatSystem& _combat);
    void                             SpawnBomb(const BombSpawn& _spawn);
    void                             SpawnShot(const ShotSpawn& _spawn, CombatSystem& _combat);
    void                             Step(const GroundMap* _pGround, CombatSystem& _combat, float _dt, Span<const Platform> _platforms = {});
    void                             OnHitEvents(Span<const HitEvent> _events, CombatSystem& _combat);
    [[nodiscard]] int                CountBullets(uint32_t _owner, uint8_t _tag) const;
    [[nodiscard]] int                CountBombs(uint32_t _owner) const;
    [[nodiscard]] int                CountShots(uint32_t _owner) const;
    [[nodiscard]] Span<const Bullet> GetBullets() const;
    [[nodiscard]] Span<const Bomb>   GetBombs() const;
    [[nodiscard]] Span<const Blast>  GetBlasts() const;
    [[nodiscard]] Span<const Shot>   GetShots() const;
    [[nodiscard]] Vector<FxRequest>  TakeFx();
    [[nodiscard]] Vector<ShotFx>     TakeShotFx();

private:
    void ExplodeBomb_(Bomb& _bomb, VECTOR2 _at, CombatSystem& _combat);
    void SpawnBulletBlast_(const Bullet& _bullet, CombatSystem& _combat);
    void SpawnShotBlast_(const ShotSpawn& _params, VECTOR2 _at, CombatSystem& _combat);

    Vector<Bullet>    m_bullets = {};
    Vector<Bomb>      m_bombs   = {};
    Vector<Blast>     m_blasts  = {};
    Vector<FxRequest> m_fx      = {};
    Vector<Shot>      m_shots   = {};
    Vector<ShotFx>    m_shotFx  = {};
};

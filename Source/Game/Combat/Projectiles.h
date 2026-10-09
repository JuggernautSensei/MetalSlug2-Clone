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

// ===========================================
//  범용 투사체 (Shot) — 적 공격용 (M3 추가)
//   직선 탄(Bullet) 과 달리 중력/가속/점화 대기를 지원한다 (포물선 폭탄, 가속 미사일, 점화 후 발사하는 로켓).
//   매 틱 자기 Hitbox 를 등록할 뿐 누구를 때릴 수 있는지는 HitResolver 가 정한다. 첫 적중 / 지형 접촉 시 소멸.
//   visual 은 호출자가 정하는 불투명 ID (렌더러가 클립으로 바꾼다). 0 = 없음.
// ===========================================

struct ShotSpawn
{
    eTeam        team            = eTeam::Enemy;
    uint32_t     owner           = kNoOwner;
    jug::VECTOR2 position        = { 0.f, 0.f };
    jug::VECTOR2 velocity        = { 0.f, 0.f };   // 초기 속도 px/s
    jug::VECTOR2 accel           = { 0.f, 0.f };   // 가속도 px/s² (진행 방향 가속 등). maxSpeed > 0 이면 |v| 상한
    float        maxSpeed        = 0.f;
    float        gravity         = 0.f;       // px/s²
    float        maxFallSpeed    = 3000.f;    // px/s
    jug::VECTOR2 size            = { 20.f, 20.f };
    int          damage          = 1;
    eHitKind     kind            = eHitKind::Bullet;
    int          lifetimeTicks   = 300;
    int          startDelayTicks = 0;      // 점화 대기: 이 틱 동안 제자리 (판정은 유효)
    bool         bExplodeOnSolid = true;   // 픽셀 지면에 닿으면 소멸
    uint16_t     visual          = 0;
    uint8_t      tag             = 0;

    // --- 구르는 탄 (사루비아 수류탄): 지면에 닿으면 착지하고 지면을 따라 구른다 ---
    float        rollSpeed       = 0.f;    // > 0 이면 착지 후 이 속도(px/s)로 굴러간다. 지형에 닿아도 소멸하지 않는다
    float        rollDirX        = 1.f;    // 굴러가는 방향 (±1)
    int          rollTicks       = 0;      // > 0 이면 착지 시점부터 이 틱 뒤에 소멸 (0 = lifetimeTicks 를 그대로 쓴다)
    bool         bExpireFx       = false;  // 수명 종료 시에도 연출 요청(ShotFx)을 낸다 (시각 전용 폭발). 기본은 조용히 사라진다
};

struct Shot
{
    jug::VECTOR2 position   = { 0.f, 0.f };
    jug::VECTOR2 renderPrev = { 0.f, 0.f };
    jug::VECTOR2 hitFrom    = { 0.f, 0.f };
    jug::VECTOR2 velocity   = { 0.f, 0.f };
    ShotSpawn    params     = {};
    uint32_t     hitId      = kNoHitId;
    int          ageTicks   = 0;
    bool         bAlive     = true;
    bool         bLanded    = false;   // 구르는 탄: 착지했는가
    int          landedAge  = 0;       // 착지한 시점의 ageTicks
};

// 샷이 적중/지형 접촉으로 사라질 때 남기는 연출 요청 (수명 종료/화면 밖 소멸에서는 만들지 않는다)
struct ShotFx
{
    jug::VECTOR2 position = { 0.f, 0.f };
    uint16_t     visual   = 0;
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
    void SpawnShot(const ShotSpawn& _spawn, CombatSystem& _combat);

    // 한 틱 진행: 이동 → 지면/표적 접촉 처리 → Hitbox 등록. _pGround 는 nullptr 가능.
    void Step(const GroundMap* _pGround, CombatSystem& _combat, float _dt);

    // Resolve 결과를 반영한다: 비관통 탄은 적중 시 소멸. (Step 에서 이미 등록한 hitId 기준)
    void OnHitEvents(jug::Span<const HitEvent> _events, CombatSystem& _combat);

    [[nodiscard]] int CountBullets(uint32_t _owner, uint8_t _tag) const;
    [[nodiscard]] int CountBombs(uint32_t _owner) const;
    [[nodiscard]] int CountShots(uint32_t _owner) const;

    // _keep 밖으로 나간 Shot 을 (연출 없이) 제거한다. 화면 밖 소멸용.
    void CullShots(const Aabb& _keep, CombatSystem& _combat);

    [[nodiscard]] jug::Span<const Bullet> GetBullets() const { return m_bullets; }
    [[nodiscard]] jug::Span<const Bomb>   GetBombs() const { return m_bombs; }
    [[nodiscard]] jug::Span<const Blast>  GetBlasts() const { return m_blasts; }
    [[nodiscard]] jug::Span<const Shot>   GetShots() const { return m_shots; }

    // 이번 틱에 쌓인 이펙트 요청을 비우며 가져간다.
    [[nodiscard]] jug::Vector<FxRequest> TakeFx();

    // Shot 의 연출 요청 (TakeFx 와 분리되어 있어 기존 호출자에게 영향이 없다)
    [[nodiscard]] jug::Vector<ShotFx> TakeShotFx();

private:
    void ExplodeBomb_(Bomb& _bomb, jug::VECTOR2 _at, CombatSystem& _combat);

    jug::Vector<Bullet>    m_bullets = {};
    jug::Vector<Bomb>      m_bombs   = {};
    jug::Vector<Blast>     m_blasts  = {};
    jug::Vector<FxRequest> m_fx      = {};
    jug::Vector<Shot>      m_shots   = {};
    jug::Vector<ShotFx>    m_shotFx  = {};
};

}   // namespace slug

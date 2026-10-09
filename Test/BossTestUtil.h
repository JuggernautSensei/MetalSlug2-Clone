#pragma once
#include <cmath>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/BattleShip.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Render/Animation.h"
#include "Render/SpriteLibrary.h"
#include "World/Constants.h"

// ===========================================
//  보스 테스트 도우미
//   Enemies.json + Boss.json 만 담은 임시 폴더를 만들어 읽는다 (다른 정의 파일의 진행 중 변경에 흔들리지 않게).
//   BossRig 는 SandboxState 없이 EnemyWorld / CombatSystem / ProjectileSystem 을 SandboxState::Tick 과 같은 순서로 돌린다.
// ===========================================

namespace slugtest
{

using namespace slug;

inline constexpr uint32_t kTestPlayerOwner = 1;

// Enemies.json + Boss.json 만 담은 임시 폴더를 만든다 (샌드박스 테스트의 enemyDefPath 로도 쓴다). 실패하면 빈 경로.
//   호출마다 새 폴더를 쓴다 (다른 테스트/프로세스가 잡고 있는 파일과 겹치지 않게). 복사는 몇 번 재시도한다.
inline std::filesystem::path BossDefsDir()
{
    static int sCounter = 0;
    const std::filesystem::path defs = std::filesystem::path { SLUG_ASSET_DIR } / "Defs";
    const std::filesystem::path dir  = std::filesystem::temp_directory_path() / ("slug_boss_test_defs_" + std::to_string(++sCounter));
    std::error_code             ec;
    std::filesystem::create_directories(dir, ec);
    for (const char* name: { "Enemies.json", "Boss.json" })
    {
        bool bCopied = false;
        for (int attempt = 0; attempt < 5 && !bCopied; ++attempt)
        {
            bCopied = std::filesystem::copy_file(defs / name, dir / name, std::filesystem::copy_options::overwrite_existing, ec) && !ec;
        }
        if (!bCopied)
        {
            return {};
        }
    }
    return dir;
}

// Enemies.json + Boss.json 만 읽는다. 실패하면 false.
inline bool LoadBossLibrary(
    EnemyDefLibrary& _out)
{
    const std::filesystem::path dir = BossDefsDir();
    if (dir.empty())
    {
        return false;
    }
    jug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::LoadDirectory(dir);
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

// 테스트용 틱 구동기. 플레이어는 가짜 몸통 상자 하나 (bInvulnerable 로 무적 표현).
class BossRig
{
public:
    BossRig()
    {
        m_ok = LoadBossLibrary(m_lib);
        m_world.SetLibrary(&m_lib);
        m_ctx.pCombat      = &m_combat;
        m_ctx.pProjectiles = &m_projectiles;
        m_ctx.cameraView   = Aabb::FromMinSize({ -20000.f, -20000.f }, { 40000.f, 40000.f });   // 화면 제약 없음
        m_ctx.pAnims       = nullptr;
    }

    [[nodiscard]] bool IsReady() const { return m_ok; }
    [[nodiscard]] const EnemyDefLibrary& GetLib() const { return m_lib; }
    [[nodiscard]] EnemyWorld&            World() { return m_world; }
    [[nodiscard]] ProjectileSystem&      Projectiles() { return m_projectiles; }
    [[nodiscard]] CombatSystem&          Combat() { return m_combat; }
    [[nodiscard]] const std::vector<EnemyEvent>& Events() const { return m_events; }
    [[nodiscard]] const std::vector<HitEvent>&   PlayerHits() const { return m_playerHits; }

    // 플레이어 위치/상태 (다음 Frame 부터 적용)
    void SetPlayer(const jug::VECTOR2 _feet, const bool _bAlive = true)
    {
        m_feet   = _feet;
        m_bAlive = _bAlive;
    }

    // 플레이어 몸통은 무적(true) 이면 피격 판정에서 빠진다. false 면 탄/포탄이 맞는다 (PlayerHits 로 기록)
    void SetPlayerInvulnerable(const bool _b) { m_bInvulnerable = _b; }

    // SandboxState::Tick 과 같은 순서: Hurtbox 등록 → 적 틱 → 투사체 → 판정 → 이벤트 적용
    void Frame()
    {
        m_ctx.player.feet          = m_feet;
        m_ctx.player.center        = { m_feet.x, m_feet.y - 30.f };
        m_ctx.player.bodyBox       = Aabb::FromCenterSize(m_ctx.player.center, { 24.f, 60.f });
        m_ctx.player.bAlive        = m_bAlive;
        m_ctx.player.bInvulnerable = m_bInvulnerable;

        m_combat.BeginTick();
        m_world.RegisterHurtboxes(m_combat);
        Hurtbox player;
        player.box           = m_ctx.player.bodyBox;
        player.team          = eTeam::Player;
        player.owner         = kTestPlayerOwner;
        player.bInvulnerable = m_bInvulnerable;
        m_combat.AddHurtbox(player);

        m_world.Tick(m_ctx);
        m_projectiles.Step(nullptr, m_combat, kStepSec);
        const jug::Vector<HitEvent>& events = m_combat.Resolve();
        m_world.OnHitEvents(events);
        m_projectiles.OnHitEvents(events, m_combat);
        for (const HitEvent& ev: events)
        {
            if (ev.target == kTestPlayerOwner)
            {
                m_playerHits.push_back(ev);
            }
        }
        jug::Vector<EnemyEvent> taken = m_world.TakeEvents();
        for (const EnemyEvent& ev: taken)
        {
            m_events.push_back(ev);
        }
        ++m_tick;
    }

    void Run(const int _ticks)
    {
        for (int i = 0; i < _ticks; ++i)
        {
            Frame();
        }
    }

    [[nodiscard]] int Tick() const { return m_tick; }

    // 이벤트 종류별 개수 (누적)
    [[nodiscard]] int CountEvents(const eEnemyEventKind _kind) const
    {
        int n = 0;
        for (const EnemyEvent& ev: m_events)
        {
            n += ev.kind == _kind ? 1 : 0;
        }
        return n;
    }

    // 소유자가 _id 인 적의 이벤트 개수 (누적)
    [[nodiscard]] int CountEventsOf(const eEnemyEventKind _kind, const EnemyHandle _h) const
    {
        int n = 0;
        for (const EnemyEvent& ev: m_events)
        {
            n += (ev.kind == _kind && ev.handle == _h) ? 1 : 0;
        }
        return n;
    }

    // 피해 한 번 (소유자 _target, 공격 종류 Bullet)
    void Hit(const uint32_t _target, const int _damage)
    {
        HitEvent ev;
        ev.damage       = _damage;
        ev.kind         = eHitKind::Bullet;
        ev.attackerTeam = eTeam::Player;
        ev.attacker     = kTestPlayerOwner;
        ev.target       = _target;
        ev.hitId        = 0;
        m_world.OnHitEvents(jug::Span<const HitEvent> { &ev, 1 });
        for (const EnemyEvent& e: m_world.TakeEvents())
        {
            m_events.push_back(e);
        }
    }

    // 여러 틱 동안 _count 개 이벤트 중 Enemy 종류 개수를 세는 대신 현재 적 목록에서 찾는다
    [[nodiscard]] const Enemy* Find(const EnemyHandle _h) const { return m_world.Get(_h); }

    [[nodiscard]] EnemyHandle SpawnAt(const jug::StringView _def, const jug::VECTOR2 _feet, const SpawnParams& _params = {})
    {
        return m_world.Spawn(_def, _feet, _params, nullptr);
    }

private:
    bool                      m_ok          = false;
    EnemyDefLibrary           m_lib         = {};
    EnemyWorld                m_world {};
    CombatSystem              m_combat      = {};
    ProjectileSystem          m_projectiles = {};
    EnemyContext              m_ctx         = {};
    jug::VECTOR2              m_feet        = { 0.f, 0.f };
    bool                      m_bAlive      = true;
    bool                      m_bInvulnerable = true;
    int                       m_tick        = 0;
    std::vector<EnemyEvent>   m_events      = {};
    std::vector<HitEvent>     m_playerHits  = {};
};

// 보스 아레나 (640x448, 바닥 378): 최종보스 테스트 기본값
inline Aabb BossArena()
{
    return Aabb::FromMinMax({ 0.f, 0.f }, { 640.f, 448.f });
}

}   // namespace slugtest

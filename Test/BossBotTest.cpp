#include "Core/AssetPath.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "Core/Input.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/Player/PlayerController.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropWorld.h"
#include "Game/SimRunner.h"
#include "Game/Stage/StageRunner.h"
#include "Game/Stage/StageState.h"

// 결정적 헤드리스 회피 봇 (보스 Big Shiee 전투). 봇은 게임 코드가 아니라 이 테스트 안에만 있다.
// 규칙: 오른쪽을 본다 (왼쪽 이동 = 뒤돌아보기, 그 동안 사격은 왼쪽으로 나간다), 사격 버튼을 계속 누른다,
// 선체가 사거리 안이면 폭탄을 던진다. 이동은 계획 기반: 적 탄 궤적을 1.5 s 앞까지 예측해
// 플레이어 몸 상자와 겹치지 않는 첫 계획 (목표로 이동 / 정지 / 짧은 좌우 이동 / 점프 / 점프+이동) 을 고른다.
namespace
{
using namespace slug;

constexpr float kDt          = 1.f / 60.f;
constexpr int   kHorizon     = 90;      // 계획 예측 상한 (1.5 s)
constexpr float kRunStep     = 250.f * kDt;   // 달리기 한 틱 이동 (runSpeed 250)
constexpr float kJumpSpeed   = 600.f;   // jumpSpeed
constexpr float kGravity     = 1400.f;  // gravity
constexpr float kHurtHalfW   = 12.f;    // 플레이어 몸 반폭 (피격 16 의 절반 8 + 여유 4: 경계 근처 탄을 놓치지 않게)
constexpr float kHurtH       = 62.f;    // 플레이어 몸 높이 (피격 54 + 여유 8)
constexpr float kStripMin    = 10.f;    // 아레나 왼쪽 기준 하한 (실제 하한은 화면 벽 margin 과 max)
constexpr float kStripMax    = 160.f;   // 플레이어 벽 165 보다 안쪽
constexpr float kPreferX     = 85.f;    // 대기 위치
constexpr float kBombMinX    = 60.f;    // 이 x 부터 선체 (190~) 에 닿는 폭탄 투척
constexpr float kJumpPenalty = 20.f;    // 점프 계획의 가산 비용 (같은 안전도면 제자리/이동을 택한다)
constexpr int   kBombGap     = 50;      // 폭탄 투척 간격 (틱)
constexpr float kDeadZone    = 2.f;
constexpr int   kStuckTicks  = 300;     // 5 s 동안 x 가 안 변하면 고착
constexpr int   kPowClose    = 5;       // ePowState::kClose (구출된 POW)

// 적 탄 한 발의 예측 궤적 (틱마다 위치, 판정 반폭/반높이)
struct ShotTrack
{
    std::vector<Vec2> pos;
    float             hx = 0.f;
    float             hy = 0.f;
};

ShotTrack BuildTrack(
    const Shot& _s)
{
    ShotTrack tr;
    tr.hx        = _s.params.size.x * 0.5f;
    tr.hy        = _s.params.size.y * 0.5f;
    const int n  = std::min(_s.params.lifetimeTicks - _s.ageTicks, kHorizon);
    Vec2      p  = _s.position;
    Vec2      v  = _s.velocity;
    for (int k = 0; k < n; ++k)
    {
        if (_s.params.gravity != 0.f)
        {
            v.y = std::min(v.y + _s.params.gravity * kDt, _s.params.maxFallSpeed);
        }
        p.x += v.x * kDt;
        p.y += v.y * kDt;
        tr.pos.push_back(p);
    }
    return tr;
}

// 계획: 목표 추격 (moveTicks < 0), 정지 (dir 0), 짧은 좌우 이동, 제자리/이동 점프
struct Plan
{
    int  dir        = 0;
    int  moveTicks  = 0;
    bool bJump      = false;
    bool bChase     = false;
};

struct Kinematics
{
    float x        = 0.f;
    float y        = 0.f;
    float vy       = 0.f;
    float groundY  = 0.f;
    bool  bGrounded = true;
};

// 계획을 kHorizon 틱 동안 시뮬레이션하고 첫 충돌 틱을 돌려준다 (충돌 없으면 kHorizon).
int SimulatePlan(
    const Plan&                  _plan,
    const Kinematics&            _k,
    const std::vector<ShotTrack>& _tracks,
    const float                  _target,
    const float                  _lo,
    const float                  _hi,
    const bool                   _bTurnNeeded,
    float&                       _xEnd)
{
    float x        = _k.x;
    float y        = _k.y;
    float vy       = _k.vy;
    bool  bGrounded = _k.bGrounded;
    if (_plan.bJump && bGrounded)
    {
        vy        = -kJumpSpeed;
        bGrounded = false;
    }
    for (int k = 0; k < kHorizon; ++k)
    {
        float step = 0.f;
        if (_plan.bChase)
        {
            step = std::clamp(_target - x, -kRunStep, kRunStep);
        }
        else if (_plan.dir != 0 && k < _plan.moveTicks)
        {
            step = static_cast<float>(_plan.dir) * kRunStep;
        }
        else if (_bTurnNeeded && k == 0 && !_plan.bJump)
        {
            step = kRunStep;   // 뒤돌아 보기 탭 (오른쪽 한 틱)
        }
        x = std::clamp(x + step, _lo, _hi);

        if (!bGrounded)
        {
            vy += kGravity * kDt;
            y  += vy * kDt;
            if (y >= _k.groundY)
            {
                y         = _k.groundY;
                vy        = 0.f;
                bGrounded = true;
            }
        }

        for (const ShotTrack& tr: _tracks)
        {
            if (k >= static_cast<int>(tr.pos.size()))
            {
                continue;
            }
            const Vec2& p = tr.pos[static_cast<size_t>(k)];
            if (std::fabs(p.x - x) < kHurtHalfW + tr.hx && p.y + tr.hy > y - kHurtH && p.y - tr.hy < y)
            {
                _xEnd = x;
                return k;
            }
        }
    }
    _xEnd = x;
    return kHorizon;
}

class DodgeBot
{
public:
    struct Options
    {
        bool bRescuePows = false;   // 보급 POW 를 구출하러 간다
        bool bTakeItems  = true;    // 떨어진 보급 아이템 (HMG/폭탄/탄약) 을 줍는다
        bool bThrowBombs = true;    // 폭탄 투척
        bool bAllowJump  = true;    // 점프 회피 허용 (false 면 좌우 이동만)
    };

    explicit DodgeBot(const Options& _options = {})
        : m_options(_options)
    {
    }

    [[nodiscard]] InputFrame Decide(const StageState& _s);
    [[nodiscard]] int        Deaths() const { return m_deaths; }
    [[nodiscard]] int        StuckEvents() const { return m_stuckEvents; }
    [[nodiscard]] int        BombsThrown() const { return m_bombsThrown; }
    [[nodiscard]] int        Jumps() const { return m_jumps; }
    [[nodiscard]] int        TrappedTicks() const { return m_trappedTicks; }

private:
    Options m_options;
    bool    m_bArena       = false;
    float   m_arenaLeft    = 0.f;
    float   m_groundY      = 0.f;
    float   m_lastX        = -1.e9f;
    int     m_lastLives    = -1;
    int     m_deaths       = 0;
    int     m_noMoveTicks  = 0;
    int     m_stuckEvents  = 0;
    int     m_bombCooldown = 0;
    int     m_bombsThrown  = 0;
    int     m_jumps        = 0;
    int     m_trappedTicks = 0;
};

InputFrame DodgeBot::Decide(
    const StageState& _s)
{
    InputFrame              frame  = {};
    const PlayerController& player = _s.GetPlayer();

    if (m_lastLives >= 0 && player.GetLives() < m_lastLives)
    {
        ++m_deaths;
    }
    m_lastLives = player.GetLives();
    if (!player.IsAlive())
    {
        m_noMoveTicks = 0;
        m_lastX       = -1.e9f;
        return frame;
    }

    const Vec2 feet     = player.GetFeet();
    const float x       = feet.x;
    const bool  bGround = player.IsGrounded();
    if (bGround)
    {
        m_groundY = feet.y;
    }
    if (!m_bArena)
    {
        _s.GetEnemies().ForEachAlive([&](const Enemy& _e) {
            if (!m_bArena && _e.behavior == eEnemyBehavior::BigShiee && !_e.arena.IsEmpty())
            {
                m_arenaLeft = _e.arena.min.x;
                m_bArena    = true;
            }
        });
    }
    const float arenaLeft = m_bArena ? m_arenaLeft : x - 100.f;

    // 실제 하한 = 화면 왼쪽 벽 (카메라 왼쪽 + playerMargin) 의 오른쪽 면 + 몸 반폭
    float             lo     = arenaLeft + kStripMin;
    const ScreenWalls screen = _s.GetRunner().MakeScreenWalls();
    if (screen.bHasLeft)
    {
        lo = std::max(lo, screen.left.max.x + kHurtHalfW);
    }
    const float hi         = arenaLeft + kStripMax;
    const bool  bHullAlive = _s.GetEnemies().CountAlive("bigshiee") > 0;

    // 목표 x: 기본은 대기 위치, 구출/아이템이 안전 구간에 있으면 가장 가까운 그쪽
    float desired  = std::clamp(arenaLeft + kPreferX, lo, hi);
    float bestDist = 1.e9f;
    const auto consider = [&](const float _tx) {
        if (_tx >= lo - 30.f && _tx <= hi + 30.f)
        {
            const float d = std::fabs(_tx - x);
            if (d < bestDist)
            {
                bestDist = d;
                desired  = std::clamp(_tx, lo, hi);
            }
        }
    };
    if (m_options.bRescuePows)
    {
        _s.GetEnemies().ForEachAlive([&](const Enemy& _e) {
            if (_e.behavior == eEnemyBehavior::Pow && _e.state != kPowClose)
            {
                consider(_e.body.position.x);
            }
        });
    }
    if (m_options.bTakeItems)
    {
        for (const PropItem& item: _s.GetProps().GetItems().GetItems())
        {
            if (!item.bPickedUp)
            {
                consider(item.feet.x);
            }
        }
    }

    // 적 탄 궤적 예측
    std::vector<ShotTrack> tracks;
    for (const Shot& shot: _s.GetProjectiles().GetShots())
    {
        if (shot.bAlive && shot.params.team == eTeam::Enemy)
        {
            tracks.push_back(BuildTrack(shot));
        }
    }

    // 계획 선택: 충돌 없는 첫 계획 (추격 > 정지 > 짧은 이동 > 점프), 전부 충돌이면 가장 오래 버티는 계획
    const std::vector<Plan> plans = {
        { 0, 0, false, true },
        { 0, 0, false, false },
        { -1, 8, false, false },
        { 1, 8, false, false },
        { -1, 16, false, false },
        { 1, 16, false, false },
        { -1, 30, false, false },
        { 1, 30, false, false },
        { -1, 50, false, false },
        { 1, 50, false, false },
        { 0, 0, true, false },
        { -1, 20, true, false },
        { 1, 20, true, false },
        { -1, 52, true, false },
        { 1, 52, true, false },
    };
    const Kinematics kin { x, feet.y, player.GetVelocity().y, m_groundY, bGround };
    // 충돌 없는 계획 중 끝 x 가 목표에 가장 가까운 것 (점프는 가산점). 전부 충돌이면 가장 오래 버티는 계획.
    int   chosen   = 1;
    int   fallback = 1;
    int   bestHit  = -1;
    float bestCost = 1.e9f;
    bool  bFound   = false;
    for (int i = 0; i < static_cast<int>(plans.size()); ++i)
    {
        const Plan& cand = plans[static_cast<size_t>(i)];
        if (cand.bJump && !m_options.bAllowJump)
        {
            continue;
        }
        float       xEnd = x;
        const int   hit  = SimulatePlan(cand, kin, tracks, desired, lo, hi, player.GetFacing() != 1, xEnd);
        if (hit >= kHorizon)
        {
            const float cost = std::fabs(xEnd - desired) + (cand.bJump ? kJumpPenalty : 0.f);
            if (cost < bestCost)
            {
                bestCost = cost;
                chosen   = i;
                bFound   = true;
            }
        }
        else if (hit > bestHit)
        {
            bestHit  = hit;
            fallback = i;
        }
    }
    if (!bFound)
    {
        chosen = fallback;
        if (bestHit < 30)
        {
            ++m_trappedTicks;   // 0.5 s 안에 맞는 것을 피할 계획이 없다 (갇힘)
        }
    }
    const Plan& plan = plans[static_cast<size_t>(chosen)];

    // 첫 틱 입력
    int dir = 0;
    if (plan.bChase)
    {
        // 왼쪽 데드존을 넓게 둔다: 왼쪽으로 가면 뒤돌아 보므로, 멈춘 채 오른쪽 탭 한 번으로 정렬되게 한다
        const float dx = desired - x;
        dir            = dx > kDeadZone ? 1 : (dx < -6.f ? -1 : 0);
    }
    else if (plan.moveTicks > 0)
    {
        dir = plan.dir;
    }
    // 멈춰 있는데 왼쪽을 보고 있으면 오른쪽을 한 틱 눌러 뒤돌아 본다 (사격 방향 회복)
    const bool bTurnTap = dir == 0 && !plan.bJump && player.GetFacing() != 1;
    frame.Set(eAction::Right, dir > 0 || bTurnTap);
    frame.Set(eAction::Left, dir < 0);
    frame.Set(eAction::Shot, true);
    if (plan.bJump && bGround)
    {
        frame.Set(eAction::Jump, true);
        ++m_jumps;
    }

    // 고착 감시: 가고 싶은데 x 가 5 s 동안 그대로 (벽 때문이 아닐 때만)
    const bool bWallReason = (dir > 0 && x >= std::min(hi, arenaLeft + 165.f) - 3.f) || (dir < 0 && x <= lo + 3.f);
    // 보스 사망 뒤에는 스테이지 흐름이 플레이어 입력을 잠근다 (x 고정) → 고착 감시는 보스가 살아 있는 동안만
    if (dir != 0 && bHullAlive && std::fabs(x - m_lastX) < 0.25f && !bWallReason)
    {
        if (++m_noMoveTicks >= kStuckTicks)
        {
            ++m_stuckEvents;
            m_noMoveTicks = 0;
        }
    }
    else
    {
        m_noMoveTicks = 0;
    }
    m_lastX = x;

    // 폭탄: 선체가 살아 있고 사거리 안이며 오른쪽을 볼 때 던진다 (왼쪽을 보면 먼저 돌아선다)
    if (m_bombCooldown > 0)
    {
        --m_bombCooldown;
    }
    const bool bWantBomb = m_options.bThrowBombs && bHullAlive && player.GetWeapon().bombs > 0 && m_bombCooldown == 0 && x >= arenaLeft + kBombMinX && _s.GetProjectiles().GetBombs().empty();
    if (bWantBomb)
    {
        if (player.GetFacing() == 1)
        {
            frame.Set(eAction::Bomb, true);
            m_bombCooldown = kBombGap;
            ++m_bombsThrown;
        }
        else if (dir == 0)
        {
            frame.Set(eAction::Right, true);   // 던지기 전에 오른쪽을 본다 (Left 와 동시에 누르면 제자리에 선다)
        }
    }
    return frame;
}

// 실제 플레이 입력 경로 (StageRunner::Step) 로 보스 아레나를 돌리는 시뮬.
class BossBotSim
{
public:
    explicit BossBotSim(const uint64_t _seed)
        : m_rng(_seed)
    {
        m_ctx.pRng       = &m_rng;
        m_ctx.assetRoot  = slug::FindAssetRoot();
        m_ctx.bDebugDraw = true;   // 디버그 워프 (F6 보스 아레나) 는 bDebugDraw 에서만 동작
        m_pState         = new StageState;
        m_runner         = std::unique_ptr<SimRunner> { new SimRunner { m_ctx, std::unique_ptr<IGameState> { m_pState } } };
    }

    void Warp(const int _code)
    {
        m_ctx.debugWarp = _code;
        m_runner->Step(InputFrame {});
    }

    void Step(const int _count)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_runner->Step(InputFrame {});
        }
    }

    void Tick(const InputFrame& _frame) { m_runner->Step(_frame); }

    [[nodiscard]] StageState&       State() { return *m_pState; }
    [[nodiscard]] const IGameState* StagePtr() const { return m_pState; }
    [[nodiscard]] const IGameState* Current() const { return m_runner->GetState(); }

private:
    slug::RngMt19937 m_rng;
    GameContext      m_ctx      = {};
    StageState*      m_pState   = nullptr;
    std::unique_ptr<SimRunner> m_runner = nullptr;
};

struct RunStats
{
    uint64_t seed        = 0;
    bool     bDefeated   = false;
    float    defeatSec   = -1.f;
    bool     bEnded      = false;
    float    endSec      = -1.f;
    bool     bGameOver   = false;
    int      deaths      = 0;
    int      stuck       = 0;
    int      bombs       = 0;
    int      jumps       = 0;
    int      trapped     = 0;   // 안전한 계획이 없던 틱 수
    int      rescued     = 0;
    float    hullRatio   = 1.f;   // 마지막으로 본 선체 HP 비율
};

RunStats RunBot(
    const uint64_t          _seed,
    const DodgeBot::Options _options)
{
    RunStats out;
    out.seed = _seed;
    BossBotSim sim(_seed);
    sim.Warp(2);   // F6: 보스 아레나 (DebugWarpToTheBoss 와 같은 워프)
    sim.Step(2);
    if (sim.State().GetEnemies().CountAlive("bigshiee") == 0)
    {
        return out;
    }
    sim.State().GetEnemies().ReseedRng(_seed);   // 시드마다 보스/POW 난수열을 바꾼다

    DodgeBot bot(_options);
    constexpr int kMaxTicks = 170 * 60;
    for (int tick = 0; tick < kMaxTicks; ++tick)
    {
        if (sim.Current() != sim.StagePtr())
        {
            out.bEnded = true;
            out.endSec = static_cast<float>(tick) / 60.f;
            break;
        }
        const StageState& state = sim.State();
        if (!out.bDefeated && state.GetEnemies().CountAlive("bigshiee") == 0)
        {
            out.bDefeated = true;
            out.defeatSec = static_cast<float>(tick) / 60.f;
        }
        if (const std::optional<float> ratio = state.GetEnemies().GetBossHpRatio())
        {
            out.hullRatio = *ratio;
        }
        if (state.GetPlayer().IsGameOver())
        {
            out.bGameOver = true;
            break;
        }
        out.rescued = state.GetRescuedPows();
        sim.Tick(bot.Decide(state));
    }
    out.deaths  = bot.Deaths();
    out.stuck   = bot.StuckEvents();
    out.bombs   = bot.BombsThrown();
    out.jumps   = bot.Jumps();
    out.trapped = bot.TrappedTicks();
    return out;
}

void ReportRuns(
    const char*             _variant,
    const DodgeBot::Options _options,
    int&                    _wins,
    int&                    _stuckTotal)
{
    constexpr uint64_t kSeeds = 5;
    for (uint64_t seed = 1; seed <= kSeeds; ++seed)
    {
        const RunStats r    = RunBot(seed, _options);
        const bool     bWin = r.bDefeated && r.defeatSec <= 150.f && r.deaths <= 2 && !r.bGameOver;
        std::printf("[BossBot] %-8s seed=%llu defeated=%d at=%.1fs ended=%d end=%.1fs deaths=%d stuck=%d bombs=%d jumps=%d trapped=%d rescued=%d hull=%.2f win=%d\n",
            _variant,
            static_cast<unsigned long long>(seed),
            r.bDefeated ? 1 : 0,
            r.defeatSec,
            r.bEnded ? 1 : 0,
            r.endSec,
            r.deaths,
            r.stuck,
            r.bombs,
            r.jumps,
            r.trapped,
            r.rescued,
            r.hullRatio,
            bWin ? 1 : 0);
        _wins += bWin ? 1 : 0;
        _stuckTotal += r.stuck;
        if (bWin)
        {
            EXPECT_TRUE(r.bEnded) << _variant << " seed " << seed << ": 보스 사망 뒤 스테이지가 끝나지 않았다";
        }
    }
}
}

TEST(BossBot, PistolAndTenBombsBeatBigShieeFromTheStripOnMostSeeds)
{
    int wins       = 0;
    int stuckTotal = 0;
    ReportRuns("pistol", DodgeBot::Options { false, true, true }, wins, stuckTotal);
    EXPECT_GE(wins, 4) << "5 seeds 중 150 s 안에 2 번 이하 사망으로 이긴 seed 가 4 개 미만";
    EXPECT_EQ(stuckTotal, 0) << "5 s 이상 x 가 멈춘 고착 이벤트";
}

TEST(BossBot, RescuingSupplyPowsKeepsTheFightWinnableOnMostSeeds)
{
    int wins       = 0;
    int stuckTotal = 0;
    ReportRuns("pow", DodgeBot::Options { true, true, true }, wins, stuckTotal);
    EXPECT_GE(wins, 2) << "보급 POW 구출 변형 (봇이 줍기 위해 안전 구역을 벗어나므로 기준이 낮다): 5 seeds 중 2 개 미만";
    EXPECT_EQ(stuckTotal, 0) << "5 s 이상 x 가 멈춘 고착 이벤트";
}

TEST(BossBot, PistolOnlyBalanceReportWithoutBombs)
{
    int wins       = 0;
    int stuckTotal = 0;
    ReportRuns("nobomb", DodgeBot::Options { false, true, false }, wins, stuckTotal);
    EXPECT_EQ(stuckTotal, 0) << "5 s 이상 x 가 멈춘 고착 이벤트";
}

TEST(BossBot, SidestepOnlyReportWithoutJumps)
{
    int wins       = 0;
    int stuckTotal = 0;
    ReportRuns("sidestep", DodgeBot::Options { false, true, true, false }, wins, stuckTotal);
    EXPECT_EQ(stuckTotal, 0) << "5 s 이상 x 가 멈춘 고착 이벤트";
}

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
#include "Game/Enemies/BossShiee.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/Player/PlayerController.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropWorld.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageState.h"
#include "Game/Stage/StageWorldHost.h"
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
constexpr float kStripMin    = 10.f;    // 화면 (카메라) 왼쪽 기준 하한 (실제 하한은 화면 벽 margin 과 max)
constexpr float kStripMax    = 600.f;   // 화면 오른쪽 안쪽 (보스 전 구간 자유 이동: 벽 없음, 카메라 기준 좌표)
constexpr float kPreferX     = 200.f;   // 대기 위치 (화면 기준)
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

// 포탑 예고 중인 탄 (Turning/Shooting): 발사 전까지 _delayTicks 틱 동안은 없고, 그 뒤 발사 시점 플레이어 중심을 향해 직선으로 간다.
// 원작 플레이처럼 포탑이 도는 것을 보고 미리 피한다 (발사 방향은 발사 시점 위치 근사).
ShotTrack BuildPendingTrack(
    const Vec2  _from,
    const Vec2  _target,
    const float _speed,
    const int   _delayTicks)
{
    ShotTrack tr;
    tr.hx = 7.f;
    tr.hy = 7.f;
    Vec2       d   = { _target.x - _from.x, _target.y - _from.y };
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    d = len < 0.0001f ? Vec2 { -1.f, 0.f } : Vec2 { d.x / len, d.y / len };
    for (int k = 0; k < kHorizon; ++k)
    {
        if (k < _delayTicks)
        {
            tr.pos.push_back({ -1.e6f, -1.e6f });
            continue;
        }
        const float t = static_cast<float>(k - _delayTicks + 1) * kDt;
        tr.pos.push_back({ _from.x + d.x * _speed * t, _from.y + d.y * _speed * t });
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
    const StageState* pStage = nullptr;   // 지형/소품 (지붕 플랫폼, 벽) 을 본다. null 이면 평지 가정
};

// 발 위치 (x, feetY) 에서 가장 가까운 발판 y (지면 비트맵 또는 소품 지붕 플랫폼). 없으면 groundY.
float SurfaceAt_(
    const Kinematics& _k,
    const float       _x,
    const float       _feetY)
{
    float best = _k.groundY;
    if (!_k.pStage)
    {
        return best;
    }
    float found = 1.e9f;
    if (const GroundMap* pGround = _k.pStage->GetHost().GetActiveGround())
    {
        if (const std::optional<int> g = pGround->FindGroundY(static_cast<int>(std::floor(_x)), static_cast<int>(std::floor(_feetY)) - 60, 320))
        {
            if (static_cast<float>(*g) >= _feetY - 12.f)
            {
                found = static_cast<float>(*g);
            }
        }
    }
    for (const Platform& p: _k.pStage->GetProps().GetPlatforms())
    {
        if (_x >= p.box.min.x && _x <= p.box.max.x && p.box.min.y >= _feetY - 12.f && p.box.min.y < found)
        {
            found = p.box.min.y;
        }
    }
    return found < 1.e8f ? found : best;
}

// 앞 46 px 안에 벽이 있는지 (발 위쪽 상자). 소품 벽과 충돌 벽 콜라이더를 본다.
bool WallAhead_(
    const StageState& _s,
    const Vec2        _feet)
{
    constexpr float kRange = 46.f;
    for (const Wall& wall: _s.GetProps().GetWalls())
    {
        if (wall.box.min.x > _feet.x && wall.box.min.x - _feet.x <= kRange && wall.box.min.y < _feet.y - 8.f)
        {
            return true;
        }
    }
    for (const StageCollider& collider: _s.GetHost().GetColliders())
    {
        if (collider.layer == eCollisionLayer::Wall && collider.box.min.x > _feet.x && collider.box.min.x - _feet.x <= kRange && collider.box.min.y < _feet.y - 8.f)
        {
            return true;
        }
    }
    return false;
}

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

        // 벽 (소품 벽 상자) 에 막힌다: 몸 상자가 벽 옆면을 넘지 못한다
        if (_k.pStage && step != 0.f)
        {
            for (const Wall& w: _k.pStage->GetProps().GetWalls())
            {
                if (y - kHurtH >= w.box.max.y || y <= w.box.min.y)
                {
                    continue;
                }
                if (step > 0.f && x + kHurtHalfW > w.box.min.x && x < w.box.min.x)
                {
                    x = w.box.min.x - kHurtHalfW;
                }
                else if (step < 0.f && x - kHurtHalfW < w.box.max.x && x > w.box.max.x)
                {
                    x = w.box.max.x + kHurtHalfW;
                }
            }
        }

        const float surface = SurfaceAt_(_k, x, y);
        if (bGrounded && surface > y + 1.f)
        {
            bGrounded = false;   // 발판 가장자리를 넘었다: 떨어진다
            vy        = 0.f;
        }
        else if (bGrounded && surface < y - 24.f)
        {
            bGrounded = false;   // 지붕 아래로 빠졌다 (계산상): 위에서 떨어지지 않게 발판으로 올린다
            y         = surface;
        }
        else if (bGrounded)
        {
            y = surface;         // 경사/지붕 높이를 따라간다
        }
        if (!bGrounded)
        {
            vy += kGravity * kDt;
            y  += vy * kDt;
            if (y >= surface)
            {
                y         = surface;
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

    DodgeBot()
        : DodgeBot(Options {})
    {
    }

    explicit DodgeBot(const Options& _options)
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
            if (!m_bArena && _e.behavior == eEnemyBehavior::BossShiee && !_e.arena.IsEmpty())
            {
                m_arenaLeft = _e.arena.min.x;
                m_bArena    = true;
            }
        });
    }
    // 전장 기준점 = 카메라 왼쪽 (보스 자동 전진으로 아레나 좌표는 움직인다. 화면 좌표로 고정하면 전략이 안 흔들린다)
    const float arenaLeft = m_bArena ? _s.GetCamera().GetPositionNoShake().x : x - 100.f;

    // 실제 하한 = 화면 왼쪽 벽 (카메라 왼쪽 + playerMargin) 의 오른쪽 면 + 몸 반폭
    float             lo     = arenaLeft + kStripMin;
    const ScreenWalls screen = _s.GetRunner().MakeScreenWalls();
    if (screen.bHasLeft)
    {
        lo = std::max(lo, screen.left.max.x + kHurtHalfW);
    }
    const float hi         = std::max(lo, arenaLeft + kStripMax);
    const bool  bHullAlive = _s.GetEnemies().CountAlive("boss_shiee") > 0;

    // 목표 x: 기본은 대기 위치, 구출/아이템이 안전 구간에 있으면 가장 가까운 그쪽
    float desired  = std::clamp(arenaLeft + kPreferX, lo, hi);

    // 사격 목표 (파츠): 노출된 캐논 우선, 없으면 가장 가까운 살아 있는 포탑. 그 아래로 가서 위로 쏜다 (피스톨은 전방/위만 쏜다)
    bool  bHasPart = false;
    float partX    = 0.f;
    {
        const EnemyDefLibrary* pLib = _s.GetEnemies().GetLibrary();
        _s.GetEnemies().ForEachAlive([&](const Enemy& _e) {
            if (_e.behavior != eEnemyBehavior::BossShiee || !pLib || !bossshiee::IsFightStarted(_e))
            {
                return;
            }
            const EnemyDef& def = pLib->Get(_e.defIndex);
            if (_e.state == bossshiee::kCannon && !bossshiee::IsCannonDestroyed(_e))
            {
                partX    = bossshiee::GetOrigin(_e, def).x + def.bossShiee.cannonOffset.x;
                bHasPart = true;
                return;
            }
            float bestD = 1.e9f;
            for (int i = 0; i < bossshiee::kTurretCount; ++i)
            {
                // 예고 중 (돌거나 쏘는 중) 인 포탑 아래에는 서지 않는다 (그 탄이 바로 머리 위로 떨어진다)
                if (bossshiee::GetTurretHp(_e, i) <= 0 || bossshiee::GetTurretPhase(_e, i) != bossshiee::kTurretIdle)
                {
                    continue;
                }
                const float cx = bossshiee::GetTurretCentre(_e, def, i).x;
                if (std::fabs(cx - x) < bestD)
                {
                    bestD    = std::fabs(cx - x);
                    partX    = cx;
                    bHasPart = true;
                }
            }
        });
    }
    if (bHasPart)
    {
        desired = std::clamp(partX, lo, hi);
    }
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

    // 포탑 예고 중인 탄 (아직 발사 전): 직선 예측에 넣는다
    {
        const EnemyDefLibrary* pLib = _s.GetEnemies().GetLibrary();
        const Vec2 center = player.GetBodyBox().GetCenter();
        _s.GetEnemies().ForEachAlive([&](const Enemy& _e) {
            if (_e.behavior != eEnemyBehavior::BossShiee || !pLib || !bossshiee::IsFightStarted(_e))
            {
                return;
            }
            const EnemyDef& def = pLib->Get(_e.defIndex);
            for (int i = 0; i < bossshiee::kTurretCount; ++i)
            {
                const int phase = bossshiee::GetTurretPhase(_e, i);
                if (bossshiee::GetTurretHp(_e, i) <= 0 || (phase != bossshiee::kTurretTurning && phase != bossshiee::kTurretShooting))
                {
                    continue;
                }
                const int ticks = bossshiee::GetTurretTicks(_e, i);
                const int delay = phase == bossshiee::kTurretTurning ? (def.bossShiee.turretTurnTicks - ticks) + def.bossShiee.turretFireTicks : def.bossShiee.turretFireTicks - ticks;
                const Vec2 from = { bossshiee::GetTurretCentre(_e, def, i).x + def.bossShiee.turretMuzzle.x, bossshiee::GetTurretCentre(_e, def, i).y + def.bossShiee.turretMuzzle.y };
                tracks.push_back(BuildPendingTrack(from, center, def.bossShiee.streakSpeed, std::max(0, delay)));
            }
            // 캐논 볼리 예고 (Gun_Aim): 트레이서 n 발이 stagger 간격으로 나온다 (부채꼴은 무시하고 중앙 방향으로 근사)
            const int aimStart = bossshiee::GetAimStartTick(_e);
            const int clock    = bossshiee::GetFightClock(_e);
            if (aimStart >= 0 && bossshiee::IsCannonDestroyed(_e) == false && _e.state == bossshiee::kCannon)
            {
                const int aimLen = def.bossShiee.volleyAimTicks + def.bossShiee.volleyStreaks * def.bossShiee.streakStaggerTicks;
                if (clock - aimStart < aimLen)
                {
                    const Vec2 muzzle = { bossshiee::GetOrigin(_e, def).x + def.bossShiee.cannonMuzzle.x, bossshiee::GetOrigin(_e, def).y + def.bossShiee.cannonMuzzle.y };
                    for (int k = 0; k < def.bossShiee.volleyStreaks; ++k)
                    {
                        const int fireAt = aimStart + def.bossShiee.volleyAimTicks + k * def.bossShiee.streakStaggerTicks;
                        tracks.push_back(BuildPendingTrack(muzzle, center, def.bossShiee.streakSpeed, std::max(0, fireAt - clock)));
                    }
                }
            }
        });
    }

    // 주황 포탄 착탄 폭발 (60x60, 0.3 s): 비행 중인 포탄의 착지 지점에 미리 넣는다 (폭발은 착지 틱에 나타난다)
    const EnemyDefLibrary* pShellLib = _s.GetEnemies().GetLibrary();
    if (const int shellIndex = pShellLib ? pShellLib->FindProjectileIndex("boss_shell") : -1; shellIndex >= 0)
    {
        for (const Shot& shot: _s.GetProjectiles().GetShots())
        {
            if (!shot.bAlive || shot.params.team != eTeam::Enemy || shot.params.visual != static_cast<uint16_t>(shellIndex + 1))
            {
                continue;
            }
            const ShotTrack shellTrack = BuildTrack(shot);
            for (size_t k = 0; k < shellTrack.pos.size(); ++k)
            {
                if (shellTrack.pos[k].y >= m_groundY - 2.f)
                {
                    ShotTrack blast;
                    blast.hx = 30.f;
                    blast.hy = 30.f;
                    for (size_t j = 0; j < shellTrack.pos.size(); ++j)
                    {
                        if (j < k)
                        {
                            blast.pos.push_back({ -1.e6f, -1.e6f });
                        }
                        else if (j < k + 18)
                        {
                            blast.pos.push_back({ shellTrack.pos[k].x, m_groundY - 30.f });
                        }
                        else
                        {
                            blast.pos.push_back({ -1.e6f, -1.e6f });
                        }
                    }
                    tracks.push_back(blast);
                    break;
                }
            }
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
    Kinematics kin { x, feet.y, player.GetVelocity().y, m_groundY, bGround };
    kin.pStage = &_s;
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
    // 파츠 바로 아래면 위로 쏜다 (Up = 조준 90 도)
    frame.Set(eAction::Up, bHasPart && std::fabs(x - partX) <= 14.f);
    // 오른쪽으로 갈 때 앞에 벽 (소품 벽/충돌 상자) 이 있으면 뛰어 넘는다 (StageIntegration 봇과 같은 규칙, 집 지붕 위로 올라간다)
    const bool bWallAhead = dir > 0 && bGround && WallAhead_(_s, player.GetFeet());
    if ((plan.bJump || bWallAhead) && bGround)
    {
        frame.Set(eAction::Jump, true);
        ++m_jumps;
    }

    // 고착 감시: 가고 싶은데 x 가 5 s 동안 그대로 (벽 때문이 아닐 때만)
    const bool bWallReason = (dir > 0 && x >= hi - 3.f) || (dir < 0 && x <= lo + 3.f);
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
    if (sim.State().GetEnemies().CountAlive("boss_shiee") == 0)
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
        if (!out.bDefeated && state.GetEnemies().CountAlive("boss_shiee") == 0)
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

TEST(BossBot, PistolAndTenBombsBeatBossShieeFromTheStripOnMostSeeds)
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

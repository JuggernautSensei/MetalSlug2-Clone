#include <algorithm>
#include <cmath>

#include "Base/Log.h"
#include "Base/Math.h"
#include "Game/Enemies/BigShiee.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 최종보스 전함 '빅 셰이' (원작 MS2 Mission 4). 설계/수치/측정 근거: Docs/Spec_BigShiee.md
// 모든 파츠는 음수 레이어로 그려 StageState 가 지면 텍스처보다 먼저 그린다 (하늘 < 선체 < 지붕/지면 < 플레이어).

namespace
{
// 보스 파츠(포탑/주포)는 총알과 폭발만 받는다. 칼질(근접)은 선체 보스의 장갑에 먹히지 않는다
const HitKindMask kBigShieeAccepts = HitKindMask { eHitKind::Bullet, eHitKind::Blast };
}   // namespace

// 낙하산 인질 정의 (Assets/Defs/BigShiee.json) 와 구출 시 드롭 무기 (헤비머신건 / 로켓 / 샷건 / 화염)
constexpr StringView kChuteDef = "pow_chute";
namespace
{
const ARRAY<StringView, 4> kChuteDrops = { "gunitem_mg", "gunitem_rl", "gunitem_sg", "gunitem_fs" };
}   // namespace

constexpr int kTurnMax      = 8;   // Turn 클립 프레임 수 - 1 (0 = 왼쪽, kTurnMid = 위, 8 = 오른쪽)
constexpr int kTurnMid      = 4;
constexpr int kAimRows      = 5;   // 주포 사격 행 (Boss_Cannon_Shot1..5, 왼쪽 .. 오른쪽)
constexpr int kCasingFrames = 6;
constexpr int kCasingDepth  = 54;
constexpr int kShotFrames   = 8;
constexpr int kYawMax       = 8;   // 주포 yaw 프레임 (Boss_Cannon_Aim 0..8, 사격 행 r = yaw 2r)

namespace
{
const char* const kLetters = "ABCDEFG";
}   // namespace

struct TurretNames
{
    ARRAY<String, 3> idle = {};
    ARRAY<String, 3> shot = {};
    String           turn = {};
    String           dead = {};
    String           puff = {};
};

namespace
{
[[nodiscard]] ARRAY<TurretNames, kBigShieeTurretCount> MakeNames_()
{
    ARRAY<TurretNames, kBigShieeTurretCount> names = {};
    for (size_t i = 0; i < names.size(); ++i)
    {
        const String letter = String(1, kLetters[i]);
        for (size_t a = 0; a < 3; ++a)
        {
            names[i].idle[a] = "Boss_Turret_" + letter + "_Idle" + std::to_string(a + 1);
            names[i].shot[a] = "Boss_Turret_" + letter + "_Shot" + std::to_string(a + 1);
        }
        names[i].turn = "Boss_Turret_" + letter + "_Turn";
        names[i].dead = "Boss_Turret_" + letter + "_Dead";
        names[i].puff = i < 4 ? "Boss_Blast_ABCD1" : "Boss_Blast_EFG1";
    }
    return names;
}

[[nodiscard]] const TurretNames& GetNames_(
    const char _letter)
{
    static const ARRAY<TurretNames, kBigShieeTurretCount> kNames = MakeNames_();
    const size_t                                          index  = static_cast<size_t>(std::clamp(_letter - 'A', 0, kBigShieeTurretCount - 1));
    return kNames[index];
}

[[nodiscard]] eBigShieePhase Phase_(
    const Enemy& _e)
{
    return static_cast<eBigShieePhase>(_e.state);
}

[[nodiscard]] int& Slot_(
    Enemy&    _e,
    const int _base,
    const int _index = 0)
{
    return _e.ext[static_cast<size_t>(_base + _index)];
}

[[nodiscard]] int GetSlot_(
    const Enemy& _e,
    const int    _base,
    const int    _index = 0)
{
    return _e.ext[static_cast<size_t>(_base + _index)];
}

[[nodiscard]] eBigShieeTurretMode GetTurretMode_(
    const Enemy& _e,
    const int    _i)
{
    return static_cast<eBigShieeTurretMode>(GetSlot_(_e, kBigShieeExtMode, _i));
}

void SetTurretMode_(
    Enemy&                    _e,
    const int                 _i,
    const eBigShieeTurretMode _mode)
{
    Slot_(_e, kBigShieeExtMode, _i)  = static_cast<int>(_mode);
    Slot_(_e, kBigShieeExtTicks, _i) = 0;
}

[[nodiscard]] eBigShieeCannonMode GetCannonMode_(
    const Enemy& _e)
{
    return static_cast<eBigShieeCannonMode>(GetSlot_(_e, kBigShieeExtCannonMode));
}

[[nodiscard]] const char* CannonModeName_(
    const eBigShieeCannonMode _mode)
{
    switch (_mode)
    {
        case eBigShieeCannonMode::Deploy: return "Deploy";
        case eBigShieeCannonMode::Aim: return "Aim";
        case eBigShieeCannonMode::Ready: return "Ready";
        case eBigShieeCannonMode::Shot: return "Shot";
        default: return "Hidden";
    }
}

void SetCannonMode_(
    Enemy&                    _e,
    const eBigShieeCannonMode _mode)
{
    EventLogFormat("bigshiee.cannon", "\"mode\":\"{}\",\"yaw\":{}", CannonModeName_(_mode), GetSlot_(_e, kBigShieeExtCannonYaw));
    Slot_(_e, kBigShieeExtCannonMode)  = static_cast<int>(_mode);
    Slot_(_e, kBigShieeExtCannonTicks) = 0;
}

[[nodiscard]] Vec2 World_(
    const Vec2 _origin,
    const Vec2 _rel)
{
    return { _origin.x + _rel.x, _origin.y + _rel.y };
}

// 키 표 (x = 초, y = 값) 를 선형 보간한다. 범위 밖은 양 끝 값.
[[nodiscard]] float Interp_(
    const Span<const Vec2> _keys,
    const float            _t)
{
    if (_keys.empty())
    {
        return 0.f;
    }
    if (_t <= _keys.front().x)
    {
        return _keys.front().y;
    }
    for (size_t i = 1; i < _keys.size(); ++i)
    {
        if (_t <= _keys[i].x)
        {
            const float span = _keys[i].x - _keys[i - 1].x;
            const float u    = span > 0.f ? (_t - _keys[i - 1].x) / span : 1.f;
            return _keys[i - 1].y + (_keys[i].y - _keys[i - 1].y) * u;
        }
    }
    return _keys.back().y;
}

[[nodiscard]] float LastKeyTime_(
    const Span<const Vec2> _keys)
{
    return _keys.empty() ? 0.f : _keys.back().x;
}

[[nodiscard]] int AliveTurrets_(
    const Enemy& _e)
{
    int count = 0;
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        if (GetSlot_(_e, kBigShieeExtHp, i) > 0)
        {
            ++count;
        }
    }
    return count;
}

[[nodiscard]] float AgeSec_(
    const Enemy& _e)
{
    return static_cast<float>(_e.aux) * kStepSec;
}

// 주포 단계 기준 시간 (초). 전개 전이면 음수.
[[nodiscard]] float DeploySec_(
    const Enemy& _e)
{
    const int start = GetSlot_(_e, kBigShieeExtDeployStart);
    return start < 0 ? -1.f : static_cast<float>(_e.aux - start) * kStepSec;
}

[[nodiscard]] bool IsScrollPaused_(
    const Enemy&       _e,
    const BigShieeCfg& _c)
{
    const float t = AgeSec_(_e);
    for (int i = 0; i < _c.numScrollPauses; ++i)
    {
        const Vec2 pause = _c.scrollPauses[static_cast<size_t>(i)];
        if (t >= pause.x && t < pause.x + pause.y)
        {
            return true;
        }
    }
    const float td = DeploySec_(_e);
    if (td < 0.f)
    {
        return false;
    }
    if (td >= _c.deployPauseDelaySec && td < _c.deployPauseDelaySec + _c.deployPauseSec)
    {
        return true;
    }
    const float late = td - _c.latePauseFirstSec;
    return late >= 0.f && std::fmod(late, _c.latePausePeriodSec) < _c.latePauseSec;
}

// 누적 스크롤이 예산에 닿았으면 카메라는 더 가지 않는다 (아레나 끝)
[[nodiscard]] bool IsScrollStopped_(
    const Enemy&       _e,
    const BigShieeCfg& _c)
{
    return _c.scrollBudgetPx > 0.f && static_cast<float>(GetSlot_(_e, kBigShieeExtScrollTicks)) * _c.scrollSpeed * kStepSec >= _c.scrollBudgetPx;
}

// 클립 길이 (틱). 애니메이션 라이브러리가 없는 환경 (테스트) 에서는 기본값
[[nodiscard]] int ClipTicks_(
    const EnemyEnv&  _env,
    const StringView _name,
    const int        _fallback)
{
    return _env.ctx.pAnims ? std::max(1, GetNamedPartTicks(*_env.ctx.pAnims, _name)) : _fallback;
}

// 사격 클립 길이 (틱). 마지막 칸은 Aim 자세의 복사본이라 보여 주지 않고 Aim 으로 돌아간다
[[nodiscard]] int ShotTicks_(
    const EnemyEnv&  _env,
    const StringView _name)
{
    const AnimClip* pClip = _env.ctx.pAnims ? _env.ctx.pAnims->FindClip(_name) : nullptr;
    const int       count = pClip ? pClip->GetFrameCount() : kShotFrames;
    return std::max(1, ClipTicks_(_env, _name, 34) * std::max(1, count - 1) / std::max(1, count));
}

[[nodiscard]] int RandTicks_(
    EnemyEnv&              _env,
    const ARRAY<float, 2>& _rangeSec)
{
    const int lo = SecToTicks(_rangeSec[0]);
    const int hi = SecToTicks(_rangeSec[1]);
    return lo + Roll(_env, std::max(1, hi - lo + 1));
}

bool SubmitFrame_(
    const EnemyRenderEnv& _render,
    const StringView      _name,
    const int             _frame,
    const Vec2            _origin,
    const int             _depth,
    const int             _sort,
    const float           _flash)
{
    const AnimClip* pClip = _render.anims.FindClip(_name);
    if (!pClip || pClip->frames.empty())
    {
        return false;
    }
    const AnimFrame& frame = pClip->frames[static_cast<size_t>(std::clamp(_frame, 0, pClip->GetFrameCount() - 1))];
    if (frame.IsEmpty())
    {
        return false;
    }
    SpriteDrawOptions options;
    options.flashAmount = _flash;
    _render.sprites.Submit(_depth + _render.zOffset, _sort, frame, _origin, options);
    return true;
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c = _def.bigShiee;
    PrepareBody(_e, _def);
    _e.facing     = 1;
    _e.aux        = 0;
    _e.counter    = 0;
    _e.deathTicks = 0;
    _e.ext.fill(0);

    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        const BigShieeTurretCfg& turret = c.turrets[static_cast<size_t>(i)];
        Slot_(_e, kBigShieeExtHp, i)    = turret.hp;
        Slot_(_e, kBigShieeExtMode, i)  = static_cast<int>(eBigShieeTurretMode::Idle);
        Slot_(_e, kBigShieeExtPos, i)   = turret.side * kTurnMax;
    }
    Slot_(_e, kBigShieeExtNextVolley)  = SecToTicks(c.firstVolleySec);
    Slot_(_e, kBigShieeExtNextChute)   = SecToTicks(c.chuteFirstSec);
    Slot_(_e, kBigShieeExtDeployStart) = -1;
    Slot_(_e, kBigShieeExtLidStart)    = -1;
    Slot_(_e, kBigShieeExtLastKill)    = -1;
    SetCannonMode_(_e, eBigShieeCannonMode::Hidden);

    // 선체는 카메라 왼쪽 밖에서 시작한다 (카메라 뷰가 없는 테스트 환경에서는 스폰 위치 그대로)
    const Aabb& view = _env.ctx.cameraView;
    if (view.max.x > view.min.x)
    {
        _e.body.position.x = view.min.x + c.startScreenX;
    }
    _e.prevFeet = _e.body.position;
    EnterState(_e, static_cast<int>(eBigShieePhase::Entrance));
}

// 포탄: 포구에서 플레이어 쪽으로 올라갔다 떨어지는 곡사 (weapon "shell" 의 arc)
[[nodiscard]] Vec2 ExtraFromWorld_(
    const Enemy&    _e,
    const EnemyDef& _def,
    const int       _weapon,
    const Vec2      _muzzle)
{
    if (_weapon < 0)
    {
        return { 0.f, 0.f };
    }
    const Vec2 base = GetMuzzleWorld(_e, _def, _weapon);
    return { _muzzle.x - base.x, _muzzle.y - base.y };
}

// 조준 위치 (Turn 프레임 0 / 4 / 8): 플레이어 발 x - 포구 x 가 turretAimBandPx 안이면 위, 밖이면 왼쪽/오른쪽. 플레이어가 없으면 지금 위치
[[nodiscard]] int AimPos_(
    const Enemy&             _e,
    const BigShieeCfg&       _c,
    const BigShieeTurretCfg& _turret,
    const EnemyEnv&          _env,
    const int                _pos)
{
    if (!_env.ctx.player.bAlive)
    {
        return _pos;
    }
    const float dx = _env.ctx.player.feet.x - (_e.body.position.x + _turret.anchor.x + _turret.muzzle.x);
    if (dx < -_c.turretAimBandPx)
    {
        return 0;
    }
    return dx > _c.turretAimBandPx ? kTurnMax : kTurnMid;
}

// 포구 x: 머리가 왼쪽이면 muzzle.x, 위면 0, 오른쪽이면 반대편 (muzzle.x 는 왼쪽을 본 자세 기준)
[[nodiscard]] float MuzzleX_(
    const BigShieeTurretCfg& _turret,
    const int                _pos)
{
    return _pos <= 0 ? _turret.muzzle.x : (_pos >= kTurnMax ? -_turret.muzzle.x : 0.f);
}

void FireShell_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _turret,
    const int       _barrel)
{
    const BigShieeCfg&       c      = _def.bigShiee;
    const BigShieeTurretCfg& turret = c.turrets[static_cast<size_t>(_turret)];
    const int                aim    = GetSlot_(_e, kBigShieeExtPos, _turret) / kTurnMid + 1;
    const float              gap    = turret.bTwin ? (_barrel == 0 ? -0.5f : 0.5f) * turret.muzzleGap : 0.f;
    const Vec2               muzzle = World_(_e.body.position, { turret.anchor.x + MuzzleX_(turret, GetSlot_(_e, kBigShieeExtPos, _turret)) + gap, turret.anchor.y + turret.muzzle.y });
    const int                weapon = _def.FindWeapon("shell");
    EventLogFormat("bigshiee.turret_fire", "\"turret\":{},\"barrel\":{},\"aim\":{},\"x\":{:.1f},\"y\":{:.1f}", _turret, _barrel, aim, muzzle.x, muzzle.y);
    (void)FireWeapon(_e, _def, _env, weapon, { 0.f, 0.f }, ExtraFromWorld_(_e, _def, weapon, muzzle));
}

void TickTurret_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _i)
{
    const BigShieeCfg&        c      = _def.bigShiee;
    const BigShieeTurretCfg&  turret = c.turrets[static_cast<size_t>(_i)];
    const eBigShieeTurretMode mode   = GetTurretMode_(_e, _i);
    if (mode == eBigShieeTurretMode::Dead)
    {
        return;
    }
    int& ticks = Slot_(_e, kBigShieeExtTicks, _i);
    int& pos   = Slot_(_e, kBigShieeExtPos, _i);
    ++ticks;

    switch (mode)
    {
        case eBigShieeTurretMode::Idle:
        {
            // 쉬는 동안 turretTrackTicks 마다 한 칸씩 플레이어 쪽으로 (Turn 프레임 정지 자세)
            const int target = AimPos_(_e, c, turret, _env, pos);
            if (c.turretTrackTicks > 0 && ticks >= c.turretTrackTicks && pos != target)
            {
                pos += target > pos ? 1 : -1;
                ticks = 0;
            }
            break;
        }
        case eBigShieeTurretMode::Prep:
            if (ticks >= c.prepTicks)
            {
                SetTurretMode_(_e, _i, pos != AimPos_(_e, c, turret, _env, pos) ? eBigShieeTurretMode::Turn : eBigShieeTurretMode::Shot);
            }
            break;
        case eBigShieeTurretMode::Turn:
            if (ticks % c.turnFrameTicks == 0)
            {
                const int target = AimPos_(_e, c, turret, _env, pos);
                pos += target > pos ? 1 : (target < pos ? -1 : 0);
                if (pos == target)
                {
                    SetTurretMode_(_e, _i, eBigShieeTurretMode::Shot);
                }
            }
            break;
        case eBigShieeTurretMode::Shot:
            if (ticks == c.fireTick)
            {
                FireShell_(_e, _def, _env, _i, 0);
                const Vec2 puff = World_(_e.body.position, { turret.anchor.x + MuzzleX_(turret, pos), turret.anchor.y + turret.muzzle.y });
                RequestFxAt(_env, GetNames_(turret.letter).puff, puff, _e.facing, 55);
            }
            else if (turret.bTwin && ticks == c.fireTick + c.pairGapTicks)
            {
                FireShell_(_e, _def, _env, _i, 1);
            }
            if (ticks >= c.shotTicks)
            {
                SetTurretMode_(_e, _i, eBigShieeTurretMode::Idle);
            }
            break;
        default:
            break;
    }
}

void TickVolley_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c = _def.bigShiee;
    if (_e.aux < GetSlot_(_e, kBigShieeExtNextVolley))
    {
        return;
    }
    ARRAY<int, kBigShieeTurretCount> idle  = {};
    int                              count = 0;
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        if (GetSlot_(_e, kBigShieeExtHp, i) > 0 && GetTurretMode_(_e, i) == eBigShieeTurretMode::Idle)
        {
            idle[static_cast<size_t>(count++)] = i;
        }
    }
    if (count <= 0)
    {
        Slot_(_e, kBigShieeExtNextVolley) = _e.aux + 30;
        return;
    }
    const int pick = idle[static_cast<size_t>(Roll(_env, count))];
    SetTurretMode_(_e, pick, eBigShieeTurretMode::Prep);
    Slot_(_e, kBigShieeExtNextVolley) = _e.aux + RandTicks_(_env, c.volleyIntervalSec);
}

// 낙하산 인질 보급: 카메라 위쪽 밖, 화면 오른쪽 가장자리 근처로 떨어뜨린다 (지연 스폰: 틱 루프 중 Enemy 벡터를 바꾸지 않는다)
void TickChute_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c = _def.bigShiee;
    if (_e.aux < GetSlot_(_e, kBigShieeExtNextChute))
    {
        return;
    }
    Slot_(_e, kBigShieeExtNextChute) = _e.aux + SecToTicks(c.chuteIntervalSec);

    const Aabb& view  = _env.ctx.cameraView;
    const int   index = _env.lib.FindIndex(kChuteDef);
    if (index < 0 || view.max.x <= view.min.x || !_env.ctx.player.bAlive)
    {
        return;
    }
    // 낙하 중에도 카메라가 자동 스크롤로 오른쪽으로 간다. 착지 때 화면 밖에 떨어지지 않도록 x 를 착지 시점 예상 화면 안으로 민다
    const float fallDist = std::max(0.f, _env.ctx.player.feet.y - (view.min.y - c.chuteStartAboveY));
    const float descend  = _env.lib.Get(index).flyingBzk.descendSpeed;
    const float fallSec  = descend > 0.f ? fallDist / descend : 0.f;
    const float scroll   = AgeSec_(_e) >= c.scrollStartSec && !IsScrollPaused_(_e, c) && !IsScrollStopped_(_e, c) ? c.scrollSpeed * fallSec : 0.f;

    const int   spread = std::max(1, static_cast<int>(c.chuteSpreadX));
    const float dx     = static_cast<float>(Roll(_env, 2 * spread + 1) - spread);
    float       x      = view.max.x + scroll - c.chuteRightMarginX + dx;
    const float lo     = view.min.x + scroll + c.chuteEdgeX;
    const float hi     = view.max.x + scroll - c.chuteEdgeX;
    if (lo <= hi)
    {
        x = std::clamp(x, lo, hi);
    }
    const Vec2  feet = { x, view.min.y - c.chuteStartAboveY };
    SpawnParams params;
    params.facing     = 1;
    params.bActivated = true;
    params.drop       = String(kChuteDrops[static_cast<size_t>(Roll(_env, static_cast<int>(kChuteDrops.size())))]);
    (void)_env.world.SpawnDeferred(index, feet, params);
}

[[nodiscard]] bool CannonVulnerable_(
    const Enemy& _e)
{
    const eBigShieeCannonMode mode = GetCannonMode_(_e);
    if (mode == eBigShieeCannonMode::Hidden)
    {
        return false;
    }
    return mode != eBigShieeCannonMode::Deploy || GetSlot_(_e, kBigShieeExtCannonTicks) >= 30;
}

[[nodiscard]] float PlayerAngleDeg_(
    const Enemy&       _e,
    const BigShieeCfg& _c,
    const EnemyEnv&    _env)
{
    const Vec2  muzzle = World_(_e.body.position, _c.cannonAimOrigin);
    const float dx     = _env.ctx.player.center.x - muzzle.x;
    const float dy     = std::max(40.f, _env.ctx.player.center.y - muzzle.y);
    return std::atan2(dx, dy) * kRad2Deg;
}

// Aim 프레임 yaw (0..8) 의 방향각: 짝수는 사격 행 각도, 홀수는 이웃 두 행의 가운데
[[nodiscard]] float YawDeg_(
    const BigShieeCfg& _c,
    const int          _yaw)
{
    const size_t low = static_cast<size_t>(_yaw / 2);
    if (_yaw % 2 == 0)
    {
        return _c.cannonAimDeg[low];
    }
    return (_c.cannonAimDeg[low] + _c.cannonAimDeg[low + 1]) * 0.5f;
}

[[nodiscard]] int NearestYaw_(
    const BigShieeCfg& _c,
    const float        _deg)
{
    int best = 0;
    for (int yaw = 1; yaw <= kYawMax; ++yaw)
    {
        if (std::abs(YawDeg_(_c, yaw) - _deg) < std::abs(YawDeg_(_c, best) - _deg))
        {
            best = yaw;
        }
    }
    return best;
}

// 이 틱에 맞출 yaw: 발사 차례면 플레이어에 가장 가까운 사격 행의 yaw (짝수), 아니면 플레이어를 바라보는 yaw
[[nodiscard]] int TargetYaw_(
    const Enemy&       _e,
    const BigShieeCfg& _c,
    const EnemyEnv&    _env,
    const bool         _bFire)
{
    if (!_env.ctx.player.bAlive)
    {
        return GetSlot_(_e, kBigShieeExtCannonYaw);
    }
    const float deg = PlayerAngleDeg_(_e, _c, _env);
    if (!_bFire)
    {
        return NearestYaw_(_c, deg);
    }
    int best = 0;
    for (int row = 1; row < kAimRows; ++row)
    {
        if (std::abs(_c.cannonAimDeg[static_cast<size_t>(row)] - deg) < std::abs(_c.cannonAimDeg[static_cast<size_t>(best)] - deg))
        {
            best = row;
        }
    }
    return best * 2;
}

void TurnCannon_(
    Enemy&             _e,
    const BigShieeCfg& _c,
    const int          _target)
{
    int& yaw  = Slot_(_e, kBigShieeExtCannonYaw);
    int& wait = Slot_(_e, kBigShieeExtCannonTurn);
    if (yaw == _target)
    {
        wait = 0;
        return;
    }
    if (++wait >= _c.cannonTurnTicks)
    {
        wait = 0;
        yaw += _target > yaw ? 1 : -1;
        EventLogFormat("bigshiee.yaw", "\"yaw\":{}", yaw);
    }
}

[[nodiscard]] float CasingLandSec_(
    const Enemy&       _e,
    const BigShieeCfg& _c)
{
    const float fromY = static_cast<float>(GetSlot_(_e, kBigShieeExtCasingY));
    const float drop  = std::max(0.f, _e.body.position.y + _c.casingGroundDy - fromY);
    const float disc  = _c.casingVel.y * _c.casingVel.y + 2.f * _c.casingGravity * drop;
    return (-_c.casingVel.y + std::sqrt(disc)) / _c.casingGravity;
}

bool CasingNow_(
    const Enemy&       _e,
    const BigShieeCfg& _c,
    Vec2&              _outAt,
    int&               _outFrame,
    float&             _outAlpha)
{
    const int start = GetSlot_(_e, kBigShieeExtCasingTick);
    if (start <= 0)
    {
        return false;
    }
    const float t     = static_cast<float>(_e.aux - start) * kStepSec;
    const Vec2  from  = { static_cast<float>(GetSlot_(_e, kBigShieeExtCasingX)), static_cast<float>(GetSlot_(_e, kBigShieeExtCasingY)) };
    const float tLand = CasingLandSec_(_e, _c);
    if (t < 0.f || t > tLand + _c.casingLingerSec)
    {
        return false;
    }
    const float tFly = std::min(t, tLand);
    _outAt           = { from.x + _c.casingVel.x * tFly, from.y + _c.casingVel.y * tFly + 0.5f * _c.casingGravity * tFly * tFly };
    _outFrame        = static_cast<int>(t * _c.casingFps) % kCasingFrames;
    _outAlpha        = t <= tLand || _c.casingLingerSec <= 0.f ? 1.f : std::clamp(1.f - (t - tLand) / _c.casingLingerSec, 0.f, 1.f);
    return true;
}

// 고폭탄: 사격 행의 각도로 나가고 (스프라이트도 같은 방향으로 돈다), 포구는 행의 포신 위치
void FireMissile_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c      = _def.bigShiee;
    const size_t       row    = static_cast<size_t>(std::clamp(GetSlot_(_e, kBigShieeExtCannonRow), 0, kAimRows - 1));
    const int          weapon = _def.FindWeapon("missile");
    const float        sign   = static_cast<float>(_e.facing < 0 ? -1 : 1);
    const Vec2         delta  = { (c.cannonMuzzleX[row] - c.cannonMuzzleX[kAimRows / 2]) * sign, 0.f };
    const Vec2         base   = weapon >= 0 ? GetMuzzleWorld(_e, _def, weapon) : _e.body.position;
    const Vec2         muzzle = { base.x + delta.x, base.y + delta.y };
    const float        rad    = c.cannonAimDeg[row] * kDeg2Rad;
    const Vec2         dir    = { std::sin(rad), std::cos(rad) };
    EventLogFormat("bigshiee.missile_fire", "\"row\":{},\"x\":{:.1f},\"y\":{:.1f},\"dx\":{:.3f},\"dy\":{:.3f}", row, muzzle.x, muzzle.y, dir.x, dir.y);
    (void)FireWeapon(_e, _def, _env, weapon, dir, delta);
    // 사격 순간 포구 섬광 (흰 원 -> 어두운 연기) 과 연기
    EventLogFormat("bigshiee.muzzle_fx", "\"x\":{:.1f},\"y\":{:.1f}", muzzle.x, muzzle.y);
    RequestFxAt(_env, "Bazuka_Shot_Effect", muzzle, _e.facing, 55);
    RequestFxAt(_env, "Boss_Smoke_Small", muzzle, _e.facing, 54);
    const Vec2 casing                 = { muzzle.x + c.casingOffset.x, muzzle.y + c.casingOffset.y };
    Slot_(_e, kBigShieeExtCasingTick) = _e.aux;
    Slot_(_e, kBigShieeExtCasingX)    = static_cast<int>(std::lround(casing.x));
    Slot_(_e, kBigShieeExtCasingY)    = static_cast<int>(std::lround(casing.y));
    EventLogFormat("bigshiee.casing", "\"x\":{:.1f},\"y\":{:.1f}", casing.x, casing.y);
}

void TickCannon_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg&        c    = _def.bigShiee;
    const eBigShieeCannonMode mode = GetCannonMode_(_e);
    if (mode == eBigShieeCannonMode::Hidden)
    {
        return;
    }
    int& ticks = Slot_(_e, kBigShieeExtCannonTicks);
    ++ticks;

    if (mode == eBigShieeCannonMode::Deploy)
    {
        const int deployTicks = c.cannonDeployTicks > 0 ? c.cannonDeployTicks : ClipTicks_(_env, "Boss_Cannon_Deploy", 70);
        if (ticks >= deployTicks)
        {
            SetCannonMode_(_e, eBigShieeCannonMode::Aim);
            Slot_(_e, kBigShieeExtCannonYaw)  = 0;
            Slot_(_e, kBigShieeExtCannonTurn) = 0;
        }
    }
    else if (mode == eBigShieeCannonMode::Aim)
    {
        const int target = TargetYaw_(_e, c, _env, false);
        TurnCannon_(_e, c, target);
        if (GetSlot_(_e, kBigShieeExtCannonYaw) == target)
        {
            SetCannonMode_(_e, eBigShieeCannonMode::Ready);
            Slot_(_e, kBigShieeExtNextMissile) = _e.aux + SecToTicks(c.firstMissileSec);
            EnterState(_e, static_cast<int>(eBigShieePhase::Cannon));
        }
    }
    else if (mode == eBigShieeCannonMode::Ready)
    {
        const bool bFire    = _e.aux >= GetSlot_(_e, kBigShieeExtNextMissile) && _env.ctx.player.bAlive;
        const int  target   = TargetYaw_(_e, c, _env, bFire);
        const bool bAligned = GetSlot_(_e, kBigShieeExtCannonYaw) == target;
        TurnCannon_(_e, c, target);
        if (bFire && bAligned)
        {
            Slot_(_e, kBigShieeExtCannonRow)   = target / 2;
            Slot_(_e, kBigShieeExtNextMissile) = _e.aux + SecToTicks(c.missileIntervalSec);
            SetCannonMode_(_e, eBigShieeCannonMode::Shot);
        }
    }
    else if (mode == eBigShieeCannonMode::Shot)
    {
        if (ticks == c.missileFireTick)
        {
            FireMissile_(_e, _def, _env);
        }
        const String shot = "Boss_Cannon_Shot" + std::to_string(GetSlot_(_e, kBigShieeExtCannonRow) + 1);
        if (ticks >= ShotTicks_(_env, shot))
        {
            SetCannonMode_(_e, eBigShieeCannonMode::Ready);
        }
    }
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c = _def.bigShiee;
    ++_e.aux;
    _e.body.velocity = { 0.f, 0.f };
    const float t    = AgeSec_(_e);

    // 자동 스크롤: 카메라가 플레이어와 무관하게 오른쪽으로 천천히 간다 (원작 실측 약 20 px/s, 가끔 정지)
    // 예산 (scrollBudgetPx) 을 다 쓰면 카메라는 멈추고 (아레나 끝) 대신 선체가 왕복한다
    const bool bScrolling = t >= c.scrollStartSec && !IsScrollPaused_(_e, c);
    const bool bStopped   = IsScrollStopped_(_e, c);
    if (bScrolling && !bStopped)
    {
        _env.world.RequestCameraScroll(c.scrollSpeed * kStepSec);
        ++Slot_(_e, kBigShieeExtScrollTicks);
    }

    const Aabb& view  = _env.ctx.cameraView;
    float       speed = 0.f;
    if (Phase_(_e) == eBigShieePhase::Entrance)
    {
        const Span<const Vec2> keys { c.entranceKeys.data(), static_cast<size_t>(c.numEntranceKeys) };
        const float            now  = Interp_(keys, t);
        const float            prev = Interp_(keys, t - kStepSec);
        speed                       = (now - prev) / kStepSec;
        if (view.max.x > view.min.x)
        {
            _e.body.position.x = view.min.x + now;
        }
        if (t >= LastKeyTime_(keys))
        {
            EventLogFormat("bigshiee.entrance_done", "\"boss\":{:.1f}", _e.body.position.x);
            EnterState(_e, static_cast<int>(eBigShieePhase::Battery));
        }
    }
    else if ((Phase_(_e) == eBigShieePhase::Battery || Phase_(_e) == eBigShieePhase::Cannon) && view.max.x > view.min.x)
    {
        const float screenX = _e.body.position.x - view.min.x;
        int&        driving = Slot_(_e, kBigShieeExtDriving);
        int&        drift   = Slot_(_e, kBigShieeExtDriftTicks);
        const float td      = DeploySec_(_e);
        const bool  bBack   = c.reverseSec > 0.f && driving == 0 && td >= c.reverseAtSec && td < c.reverseAtSec + c.reverseSec;
        if (bBack)
        {
            _e.body.position.x -= c.reverseSpeed * kStepSec;
            speed = -c.reverseSpeed;
        }
        else if (driving == 0 && bStopped && bScrolling && c.shuttleDriftSec > 0.f)
        {
            // 카메라가 멈춘 뒤: 카메라가 가던 것처럼 선체가 왼쪽으로 흐르다가 shuttleDriftSec 뒤 돌진
            _e.body.position.x -= c.scrollSpeed * kStepSec;
            if (++drift >= SecToTicks(c.shuttleDriftSec))
            {
                driving = 1;
            }
        }
        if (driving == 0 && screenX < c.driveTriggerX)
        {
            driving = 1;
        }
        if (driving != 0)
        {
            drift = 0;
        }
        if (driving != 0)
        {
            const float remaining = c.driveTargetX - screenX;
            const float prev      = static_cast<float>(GetSlot_(_e, kBigShieeExtSpeedX));
            speed                 = std::min({ c.driveSpeed, prev + c.driveAccel * kStepSec, std::sqrt(std::max(0.f, 2.f * c.driveAccel * remaining)) });
            speed                 = std::max(speed, 14.f);
            if (remaining <= speed * kStepSec)
            {
                _e.body.position.x = view.min.x + c.driveTargetX;
                speed              = 0.f;
                driving            = 0;
            }
            else
            {
                _e.body.position.x += speed * kStepSec;
            }
        }
    }
    Slot_(_e, kBigShieeExtMoving) = std::abs(speed) > 12.f ? 1 : 0;
    Slot_(_e, kBigShieeExtSpeedX) = static_cast<int>(std::lround(speed));

    if (_e.aux % 30 == 0)
    {
        EventLogFormat("bigshiee.cam", "\"cam\":{:.1f},\"boss\":{:.1f}", view.min.x, _e.body.position.x);
    }

    if (Phase_(_e) == eBigShieePhase::Entrance || Phase_(_e) == eBigShieePhase::Battery)
    {
        TickVolley_(_e, _def, _env);
    }
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        TickTurret_(_e, _def, _env, i);
    }

    // 딸피 연출: 남은 체력 (포탑 + 주포) 비율이 낮을수록 선체 위에 작은 폭발이 더 자주 터진다
    if (_e.life == eEnemyLife::Alive && c.lowHpFxBelow > 0.f)
    {
        int curHp = _e.health.hp;
        int maxHp = _e.health.maxHp;
        for (int i = 0; i < kBigShieeTurretCount; ++i)
        {
            curHp += GetSlot_(_e, kBigShieeExtHp, i);
            maxHp += c.turrets[static_cast<size_t>(i)].hp;
        }
        const float ratio = maxHp > 0 ? static_cast<float>(curHp) / static_cast<float>(maxHp) : 1.f;
        if (ratio < c.lowHpFxBelow && _e.aux >= GetSlot_(_e, kBigShieeExtNextDmgFx))
        {
            const float k                    = Clamp(1.f - ratio / c.lowHpFxBelow, 0.f, 1.f);
            const float sec                  = c.lowHpFxSec[0] + (c.lowHpFxSec[1] - c.lowHpFxSec[0]) * k;
            Slot_(_e, kBigShieeExtNextDmgFx) = _e.aux + std::max(1, SecToTicks(sec));
            const float w                    = c.lowHpFxArea[2] - c.lowHpFxArea[0];
            const float h                    = c.lowHpFxArea[3] - c.lowHpFxArea[1];
            const float rx                   = c.lowHpFxArea[0] + w * static_cast<float>(Roll(_env, 1000)) / 1000.f;
            const float ry                   = c.lowHpFxArea[1] + h * static_cast<float>(Roll(_env, 1000)) / 1000.f;
            RequestFxAt(_env, c.lowHpFxClip.c_str(), World_(_e.body.position, { rx, ry }), _e.facing, 57);
        }
    }

    if ((Phase_(_e) == eBigShieePhase::Battery || Phase_(_e) == eBigShieePhase::Entrance) && AliveTurrets_(_e) == 0)
    {
        const int lastKill = GetSlot_(_e, kBigShieeExtLastKill);
        if (lastKill >= 0 && _e.aux - lastKill >= SecToTicks(c.deployDelaySec))
        {
            EnterState(_e, static_cast<int>(eBigShieePhase::Deploy));
            Slot_(_e, kBigShieeExtLidStart) = _e.aux;
        }
    }
    if (Phase_(_e) == eBigShieePhase::Deploy && GetSlot_(_e, kBigShieeExtLidStart) >= 0)
    {
        const int sinceLid = _e.aux - GetSlot_(_e, kBigShieeExtLidStart);
        if (sinceLid % c.hullStepTicks == 0 && sinceLid / c.hullStepTicks <= 4)
        {
            EventLogFormat("bigshiee.lid", "\"step\":{}", sinceLid / c.hullStepTicks);
        }
    }
    // 선수 (뚜껑) 가 다 열리고 cannonDeployDelaySec 뒤에 주포가 올라온다 (원작: 뚜껑 -> 잠시 뒤 주포)
    if (Phase_(_e) == eBigShieePhase::Deploy && GetCannonMode_(_e) == eBigShieeCannonMode::Hidden
        && _e.aux - GetSlot_(_e, kBigShieeExtLidStart) >= SecToTicks(c.cannonDeployDelaySec))
    {
        Slot_(_e, kBigShieeExtDeployStart) = _e.aux;
        SetCannonMode_(_e, eBigShieeCannonMode::Deploy);
    }
    TickCannon_(_e, _def, _env);
    const int casingStart = GetSlot_(_e, kBigShieeExtCasingTick);
    if (casingStart > 0 && _e.aux - casingStart == static_cast<int>(std::ceil((CasingLandSec_(_e, c) + c.casingLingerSec) / kStepSec)))
    {
        EventLogFormat("bigshiee.casing_gone", "\"x\":{}", GetSlot_(_e, kBigShieeExtCasingX));
    }
    TickChute_(_e, _def, _env);
}

// 포탑/주포 피격. 포탑은 자체 체력, 주포는 보스 체력 (e.health). 소비한 피격은 true (부위 플래시는 EnemyWorld 가 건다)
bool OnHit_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const HitEvent& _event)
{
    const BigShieeCfg& c    = _def.bigShiee;
    const int          part = GetPartIndex(_event.target);

    if (part >= 0 && part < kBigShieeTurretCount)
    {
        int& hp = Slot_(_e, kBigShieeExtHp, part);
        if (hp <= 0)
        {
            return true;
        }
        hp -= _event.damage;
        if (hp <= 0)
        {
            hp = 0;
            SetTurretMode_(_e, part, eBigShieeTurretMode::Dead);
            Slot_(_e, kBigShieeExtPos, part) = c.turrets[static_cast<size_t>(part)].side * kTurnMax;
            Slot_(_e, kBigShieeExtLastKill)  = _e.aux;
            EventLogFormat("bigshiee.turret_dead", "\"turret\":{}", part);

            const BigShieeTurretCfg& turret    = c.turrets[static_cast<size_t>(part)];
            const Vec2               at        = World_(_e.body.position, { turret.anchor.x + c.turretBlastOffset.x, turret.anchor.y + c.turretBlastOffset.y });
            EnemyEvent               destroyed = _env.world.MakeEvent(_e, eEnemyEventKind::PartDestroyed);
            destroyed.score                    = c.turretScore;
            destroyed.pos                      = at;
            _env.world.PushEvent(destroyed);
            RequestFxAt(_env, turret.bTwin ? "Explosion_Large" : "Explosion_Mid", at, _e.facing, 56);
            RequestFxAt(_env, "Boss_Smoke_Big", { at.x + c.turretSmokeOffset.x, at.y + c.turretSmokeOffset.y }, _e.facing, 54, 0.25f);
        }
        return true;
    }

    if (part == kBigShieeCannonPart)
    {
        if (CannonVulnerable_(_e))
        {
            (void)_e.health.ApplyDamage(_event.damage);
        }
        return true;
    }
    return true;
}

void RegisterHurtboxes_(
    const Enemy&    _e,
    const EnemyDef& _def,
    CombatSystem&   _combat)
{
    // 첫 틱 (Init_) 전에는 위치가 스폰 지점 그대로라 판정을 등록하지 않는다
    if (!_e.bInit)
    {
        return;
    }
    const BigShieeCfg& c = _def.bigShiee;
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        if (GetSlot_(_e, kBigShieeExtHp, i) <= 0)
        {
            continue;
        }
        const BigShieeTurretCfg& turret = c.turrets[static_cast<size_t>(i)];
        const Vec2               center = World_(_e.body.position, { turret.anchor.x + turret.boxOffset.x, turret.anchor.y + turret.boxOffset.y });
        Hurtbox                  hurtbox;
        hurtbox.box     = Aabb::FromCenterSize(center, turret.boxSize);
        hurtbox.team    = _def.team;
        hurtbox.owner   = MakePartOwner(_e.id, static_cast<uint32_t>(i));
        hurtbox.accepts = kBigShieeAccepts;
        _combat.AddHurtbox(hurtbox);
    }
    if (CannonVulnerable_(_e))
    {
        const Vec2 center = World_(_e.body.position, c.cannonBoxOffset);
        Hurtbox    hurtbox;
        hurtbox.box     = Aabb::FromCenterSize(center, c.cannonBoxSize);
        hurtbox.team    = _def.team;
        hurtbox.owner   = MakePartOwner(_e.id, static_cast<uint32_t>(kBigShieeCannonPart));
        hurtbox.accepts = kBigShieeAccepts;
        _combat.AddHurtbox(hurtbox);
    }
}

// 사망 연출 (사망 시점 기준 초): 주포 상단 화염 -> 선체 연쇄 폭발 -> 주포 붕괴 + 잔해 -> 연기 구름
void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c = _def.bigShiee;
    _e.deathTicks        = 0;
    EventLogFormat("bigshiee.death", "\"x\":{:.1f}", _e.body.position.x);
    const Vec2 top = World_(_e.body.position, { c.cannonBoxOffset.x, c.cannonBoxOffset.y - c.cannonBoxSize.y * 0.4f });
    RequestFxAt(_env, "PlayerBombExplosion", { top.x + c.deathFlameOffset.x, top.y + c.deathFlameOffset.y }, _e.facing, 56);
    RequestFxAt(_env, "Explosion_Large", top, _e.facing, 57, 0.15f);
}

void TickDying_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BigShieeCfg& c = _def.bigShiee;
    _e.body.velocity     = { 0.f, 0.f };
    ++_e.deathTicks;
    const float t      = static_cast<float>(_e.deathTicks) * kStepSec;
    const Vec2  origin = _e.body.position;

    const bool bWreck = t >= c.dyingWreckSec;
    if (bWreck && static_cast<float>(_e.deathTicks - 1) * kStepSec < c.dyingWreckSec)
    {
        EventLogFormat("bigshiee.wreck", "\"x\":{:.1f}", origin.x);
    }
    if (t >= c.dyingDoneSec && static_cast<float>(_e.deathTicks - 1) * kStepSec < c.dyingDoneSec)
    {
        EventLogFormat("bigshiee.done", "\"x\":{:.1f}", origin.x);
    }
    if (bWreck)
    {
        const int interval = std::max(1, SecToTicks(c.wreckSmokeIntervalSec));
        if (_e.deathTicks % interval == 0 && t < c.dyingDoneSec + 1.f)
        {
            const float rx = static_cast<float>(Roll(_env, 440)) - 220.f;
            const float ry = static_cast<float>(Roll(_env, 260)) - 260.f;
            RequestFxAt(_env, "Boss_Smoke_Big", World_(origin, { rx, ry }), _e.facing, 54);
        }
        return;
    }

    const float progress = std::clamp(t / c.dyingWreckSec, 0.f, 1.f);
    const int   interval = std::max(1, static_cast<int>(static_cast<float>(SecToTicks(c.dyingBlastIntervalSec)) * (1.4f - progress)));
    if (_e.deathTicks % interval == 0)
    {
        const float rx   = static_cast<float>(Roll(_env, 480)) - 260.f;
        const float ry   = static_cast<float>(Roll(_env, 330)) - 340.f;
        const int   kind = Roll(_env, 3);
        RequestFxAt(_env, kind == 0 ? "Explosion_Large" : "Explosion_Mid", World_(origin, { rx, ry }), _e.facing, 57);
    }
    if (_e.deathTicks == SecToTicks(1.4f) || _e.deathTicks == SecToTicks(2.2f) || _e.deathTicks == SecToTicks(3.1f))
    {
        const float rx = static_cast<float>(Roll(_env, 300)) - 200.f;
        const float ry = static_cast<float>(Roll(_env, 160)) - 260.f;
        RequestFxAt(_env, "PlayerBombExplosion", World_(origin, { rx, ry + c.deathBlastDy }), _e.facing, 56);
    }
    if (_e.deathTicks == SecToTicks(c.dyingWreckSec) - 6)
    {
        RequestFxAt(_env, "Explosion_Huge", World_(origin, c.deathHugeOffset), _e.facing, 58);
    }
}

bool DeathDone_(
    const Enemy&    _e,
    const EnemyDef& _def)
{
    return static_cast<float>(_e.deathTicks) * kStepSec >= _def.bigShiee.dyingDoneSec;
}

// 선체 셀: 전개 중에는 선수가 열리는 순서 (D1 -> D3 -> D6 -> D9) 로 바뀌고 끝나면 열린 상태
void SubmitHull_(
    const Enemy&          _e,
    const BigShieeCfg&    _c,
    const EnemyRenderEnv& _render,
    const Vec2            _feet)
{
    const bool           bMoving = GetSlot_(_e, kBigShieeExtMoving) != 0;
    const eBigShieePhase phase   = Phase_(_e);
    const int            age     = _e.aux;
    const char*          clip    = bMoving ? "Boss_Hull_Drive" : "Boss_Hull_Idle";
    bool                 bLoop   = true;
    int                  ticks   = age;

    if (_e.life == eEnemyLife::Dying)
    {
        clip  = "Boss_Hull_Open";
        ticks = 0;
    }
    else if (phase == eBigShieePhase::Deploy)
    {
        const int step = (age - GetSlot_(_e, kBigShieeExtLidStart)) / std::max(1, _c.hullStepTicks);
        clip           = step <= 0 ? "Boss_Hull_D1" : (step == 1 ? "Boss_Hull_D3" : (step == 2 ? "Boss_Hull_D6" : (step == 3 ? "Boss_Hull_D9" : "Boss_Hull_Open")));
        ticks          = step >= 4 ? age : 0;
    }
    else if (phase == eBigShieePhase::Cannon)
    {
        clip = bMoving ? "Boss_Hull_OpenDrive" : "Boss_Hull_Open";
    }
    SubmitNamedPart(_render, clip, _feet, ticks, bLoop, _c.partHull.sort, _c.partHull.depth);
}

void SubmitTurret_(
    const Enemy&          _e,
    const BigShieeCfg&    _c,
    const EnemyRenderEnv& _render,
    const Vec2            _feet,
    const int             _i)
{
    const BigShieeTurretCfg&  turret = _c.turrets[static_cast<size_t>(_i)];
    const EnemyPartDepth&     part   = _c.partTurrets[static_cast<size_t>(_i)];
    const TurretNames&        names  = GetNames_(turret.letter);
    const eBigShieeTurretMode mode   = GetTurretMode_(_e, _i);
    const Vec2                origin = World_(_feet, turret.anchor);
    const int                 ticks  = GetSlot_(_e, kBigShieeExtTicks, _i);
    const int                 pos    = GetSlot_(_e, kBigShieeExtPos, _i);
    const float               flash  = PartFlashAmount(_e, _i);

    switch (mode)
    {
        case eBigShieeTurretMode::Dead:
            (void)SubmitNamedPart(_render, names.dead, origin, 0, false, part.sort, part.depth);
            break;
        case eBigShieeTurretMode::Turn:
            (void)SubmitFrame_(_render, names.turn, pos, origin, part.depth, part.sort - _i, flash);
            break;
        case eBigShieeTurretMode::Shot:
            (void)SubmitNamedPart(_render, names.shot[static_cast<size_t>(std::min(2, pos / kTurnMid))], origin, ticks, false, part.sort, part.depth, false, flash);
            break;
        default:
            if (pos % kTurnMid != 0)
            {
                (void)SubmitFrame_(_render, names.turn, pos, origin, part.depth, part.sort - _i, flash);
            }
            else
            {
                (void)SubmitNamedPart(_render, names.idle[static_cast<size_t>(pos / kTurnMid)], origin, _e.aux + _i * 7, true, part.sort, part.depth, false, flash);
            }
            break;
    }
}

// 전개 틱을 클립 틱으로 바꾼다 (cannonDeployTicks 가 클립 길이와 다르면 늘이거나 줄인다)
[[nodiscard]] int DeployClipTicks_(
    const BigShieeCfg&    _c,
    const EnemyRenderEnv& _render,
    const int             _ticks)
{
    const int clipTicks = GetNamedPartTicks(_render.anims, "Boss_Cannon_Deploy");
    if (_c.cannonDeployTicks <= 0 || clipTicks <= 0)
    {
        return _ticks;
    }
    return _ticks * clipTicks / _c.cannonDeployTicks;
}

void SubmitCannon_(
    const Enemy&          _e,
    const BigShieeCfg&    _c,
    const EnemyRenderEnv& _render,
    const Vec2            _feet,
    const float           _flash)
{
    const eBigShieeCannonMode mode   = GetCannonMode_(_e);
    const int                 ticks  = GetSlot_(_e, kBigShieeExtCannonTicks);
    const int                 row    = std::clamp(GetSlot_(_e, kBigShieeExtCannonRow), 0, kAimRows - 1);
    const int                 yaw    = std::clamp(GetSlot_(_e, kBigShieeExtCannonYaw), 0, kYawMax);
    const Vec2                origin = World_(_feet, _c.cannonAnchor);
    const String              shot   = "Boss_Cannon_Shot" + std::to_string(row + 1);
    switch (mode)
    {
        case eBigShieeCannonMode::Deploy:
            (void)SubmitNamedPart(_render, "Boss_Cannon_Deploy", origin, DeployClipTicks_(_c, _render, ticks), false, _c.partCannon.sort, _c.partCannon.depth, false, _flash);
            break;
        case eBigShieeCannonMode::Aim:
        case eBigShieeCannonMode::Ready:
            (void)SubmitFrame_(_render, "Boss_Cannon_Aim", yaw, origin, _c.partCannon.depth, _c.partCannon.sort, _flash);
            break;
        case eBigShieeCannonMode::Shot:
            (void)SubmitNamedPart(_render, shot, origin, ticks, false, _c.partCannon.sort, _c.partCannon.depth, false, _flash);
            break;
        default:
            break;
    }
}

void SubmitCasing_(
    const Enemy&          _e,
    const BigShieeCfg&    _c,
    const EnemyRenderEnv& _render)
{
    Vec2  at    = { 0.f, 0.f };
    int   frame = 0;
    float alpha = 1.f;
    if (!_render.pSprites || !CasingNow_(_e, _c, at, frame, alpha))
    {
        return;
    }
    const SpriteInfo* pInfo = _render.pSprites->Find("Boss_CasingStrip");
    if (!pInfo || !pInfo->bStandalone)
    {
        return;
    }
    const int handle = _render.sprites.FindStandalone(pInfo->file);
    if (handle < 0)
    {
        return;
    }
    const int         cellW = pInfo->width / kCasingFrames;
    SpriteDrawOptions options;
    options.tint.a = static_cast<uint8_t>(std::lround(alpha * 255.f));
    options.scale  = _c.casingScale;
    _render.sprites.SubmitStandalone(kCasingDepth + _render.zOffset, 0, handle, { frame * cellW, 0, cellW, pInfo->height }, at, options);
}

void SubmitParts_(
    const Enemy&          _e,
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render)
{
    // 첫 틱 (Init_) 전에는 스폰 지점에 그려지므로 숨긴다
    if (!_e.bInit)
    {
        return;
    }
    const BigShieeCfg& c    = _def.bigShiee;
    const Vec2         feet = _render.feet;

    if (_e.life == eEnemyLife::Dying)
    {
        const float t = static_cast<float>(_e.deathTicks) * kStepSec;
        if (t >= c.dyingWreckSec)
        {
            (void)SubmitNamedPart(_render, "Boss_Wreck", World_(feet, c.wreckOffset), 0, false, c.partWreck.sort, c.partWreck.depth);
            (void)SubmitNamedPart(_render, "Boss_CannonWreck", World_(feet, c.cannonWreckOffset), 0, false, c.partCannon.sort, c.partCannon.depth);
            return;
        }
        SubmitHull_(_e, c, _render, feet);
        (void)SubmitNamedPart(_render, "Boss_AntennaP", World_(feet, c.antennaP), 0, false, c.partAntenna.sort, c.partAntenna.depth);
        (void)SubmitNamedPart(_render, "Boss_AntennaQ", World_(feet, c.antennaQ), 0, false, c.partAntenna.sort, c.partAntenna.depth);
        for (int i = 0; i < kBigShieeTurretCount; ++i)
        {
            SubmitTurret_(_e, c, _render, feet, i);
        }
        SubmitCannon_(_e, c, _render, feet, (_e.deathTicks / 3) % 2 == 0 ? 1.f : 0.f);
        return;
    }

    SubmitHull_(_e, c, _render, feet);
    (void)SubmitNamedPart(_render, "Boss_AntennaP", World_(feet, c.antennaP), 0, false, c.partAntenna.sort, c.partAntenna.depth);
    (void)SubmitNamedPart(_render, "Boss_AntennaQ", World_(feet, c.antennaQ), 0, false, c.partAntenna.sort, c.partAntenna.depth);
    for (int i = 0; i < kBigShieeTurretCount; ++i)
    {
        SubmitTurret_(_e, c, _render, feet, i);
    }
    SubmitCannon_(_e, c, _render, feet, PartFlashAmount(_e, kBigShieeCannonPart));
    SubmitCasing_(_e, c, _render);
}
}   // namespace

const BehaviorOps kBigShieeOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = &RegisterHurtboxes_,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
};

namespace
{
// [t0, v0, t1, v1, ...] 키 표를 읽는다 (가변 길이, 최대 kCapacity 쌍). 읽은 쌍 수를 돌려준다 (키가 없으면 -1)
template<size_t kCapacity>
int ReadKeys_(
    DefReader&              _r,
    const StringView        _key,
    ARRAY<Vec2, kCapacity>& _out)
{
    ARRAY<float, kCapacity * 2> flat  = {};
    const int                   count = _r.NumList(_key, flat);
    if (count < 0 || count % 2 != 0)
    {
        return -1;
    }
    for (int i = 0; i < count / 2; ++i)
    {
        _out[static_cast<size_t>(i)] = { flat[static_cast<size_t>(i) * 2], flat[static_cast<size_t>(i) * 2 + 1] };
    }
    return count / 2;
}
}   // namespace

void ParseBigShieeCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BigShieeCfg& c = _def.bigShiee;

    ARRAY<float, kBigShieeTurretCount * 2> anchors  = {};
    ARRAY<float, kBigShieeTurretCount * 4> boxes    = {};
    ARRAY<float, kBigShieeTurretCount * 2> muzzles  = {};
    ARRAY<float, kBigShieeTurretCount>     sides    = {};
    ARRAY<float, kBigShieeTurretCount>     twins    = {};
    ARRAY<float, kBigShieeTurretCount>     hps      = {};
    const bool                             bAnchors = _r.NumArray("turretAnchor", anchors);
    const bool                             bBoxes   = _r.NumArray("turretBox", boxes);
    const bool                             bMuzzles = _r.NumArray("turretMuzzle", muzzles);
    const bool                             bSides   = _r.NumArray("turretSide", sides);
    const bool                             bTwins   = _r.NumArray("turretTwin", twins);
    const bool                             bHps     = _r.NumArray("turretHp", hps);
    const float                            gap      = _r.Num("muzzleGap", 16.f);
    for (size_t i = 0; i < static_cast<size_t>(kBigShieeTurretCount); ++i)
    {
        BigShieeTurretCfg& t = c.turrets[i];
        t.letter             = kLetters[i];
        t.muzzleGap          = gap;
        if (bAnchors)
        {
            t.anchor = { anchors[i * 2], anchors[i * 2 + 1] };
        }
        if (bBoxes)
        {
            t.boxSize   = { boxes[i * 4], boxes[i * 4 + 1] };
            t.boxOffset = { boxes[i * 4 + 2], boxes[i * 4 + 3] };
        }
        if (bMuzzles)
        {
            t.muzzle = { muzzles[i * 2], muzzles[i * 2 + 1] };
        }
        if (bSides)
        {
            t.side = static_cast<int>(sides[i]);
        }
        if (bTwins)
        {
            t.bTwin = twins[i] != 0.f;
        }
        if (bHps)
        {
            t.hp = static_cast<int>(hps[i]);
        }
    }
    c.turretScore = _r.Int("turretScore", c.turretScore);
    c.antennaP    = _r.ReadVec2("antennaP", c.antennaP);
    c.antennaQ    = _r.ReadVec2("antennaQ", c.antennaQ);

    c.startScreenX = _r.Num("startScreenX", c.startScreenX);
    if (const int n = ReadKeys_(_r, "entranceKeys", c.entranceKeys); n >= 0)
    {
        c.numEntranceKeys = n;
    }

    c.scrollSpeed         = _r.Num("scrollSpeed", c.scrollSpeed);
    c.scrollStartSec      = _r.Num("scrollStartSec", c.scrollStartSec);
    c.deployPauseDelaySec = _r.Num("deployPauseDelaySec", c.deployPauseDelaySec);
    c.deployPauseSec      = _r.Num("deployPauseSec", c.deployPauseSec);
    c.latePauseFirstSec   = _r.Num("latePauseFirstSec", c.latePauseFirstSec);
    c.latePausePeriodSec  = _r.Num("latePausePeriodSec", c.latePausePeriodSec);
    c.latePauseSec        = _r.Num("latePauseSec", c.latePauseSec);
    if (const int n = ReadKeys_(_r, "scrollPauses", c.scrollPauses); n >= 0)
    {
        c.numScrollPauses = n;
    }

    c.driveTriggerX = _r.Num("driveTriggerX", c.driveTriggerX);
    c.driveTargetX  = _r.Num("driveTargetX", c.driveTargetX);
    c.driveSpeed    = _r.Num("driveSpeed", c.driveSpeed);
    c.driveAccel    = _r.Num("driveAccel", c.driveAccel);

    c.scrollBudgetPx  = _r.Num("scrollBudgetPx", c.scrollBudgetPx);
    c.shuttleDriftSec = _r.Num("shuttleDriftSec", c.shuttleDriftSec);
    c.reverseAtSec    = _r.Num("reverseAtSec", c.reverseAtSec);
    c.reverseSec      = _r.Num("reverseSec", c.reverseSec);
    c.reverseSpeed    = _r.Num("reverseSpeed", c.reverseSpeed);

    (void)_r.NumArray("volleyIntervalSec", c.volleyIntervalSec);
    c.firstVolleySec   = _r.Num("firstVolleySec", c.firstVolleySec);
    c.prepTicks        = _r.Int("prepTicks", c.prepTicks);
    c.turnFrameTicks   = std::max(1, _r.Int("turnFrameTicks", c.turnFrameTicks));
    c.shotTicks        = _r.Int("shotTicks", c.shotTicks);
    c.fireTick         = _r.Int("fireTick", c.fireTick);
    c.pairGapTicks     = _r.Int("pairGapTicks", c.pairGapTicks);
    c.turretAimBandPx  = _r.Num("turretAimBandPx", c.turretAimBandPx);
    c.turretTrackTicks = _r.Int("turretTrackTicks", c.turretTrackTicks);

    c.cannonAnchor         = _r.ReadVec2("cannonAnchor", c.cannonAnchor);
    c.cannonBoxSize        = _r.ReadVec2("cannonBoxSize", c.cannonBoxSize);
    c.cannonBoxOffset      = _r.ReadVec2("cannonBoxOffset", c.cannonBoxOffset);
    c.cannonAimOrigin      = _r.ReadVec2("cannonAimOrigin", c.cannonAimOrigin);
    c.deployDelaySec       = _r.Num("deployDelaySec", c.deployDelaySec);
    c.firstMissileSec      = _r.Num("firstMissileSec", c.firstMissileSec);
    c.missileIntervalSec   = _r.Num("missileIntervalSec", c.missileIntervalSec);
    c.missileFireTick      = _r.Int("missileFireTick", c.missileFireTick);
    c.hullStepTicks        = std::max(1, _r.Int("hullStepTicks", c.hullStepTicks));
    c.cannonDeployDelaySec = _r.Num("cannonDeployDelaySec", c.cannonDeployDelaySec);
    c.cannonDeployTicks    = _r.Int("cannonDeployTicks", c.cannonDeployTicks);
    c.cannonTurnTicks      = std::max(1, _r.Int("cannonTurnTicks", c.cannonTurnTicks));
    (void)_r.NumArray("cannonAimDeg", c.cannonAimDeg);
    (void)_r.NumArray("cannonMuzzleX", c.cannonMuzzleX);

    c.casingOffset    = _r.ReadVec2("casingOffset", c.casingOffset);
    c.casingScale     = _r.Num("casingScale", c.casingScale);
    c.casingVel       = _r.ReadVec2("casingVel", c.casingVel);
    c.casingGravity   = _r.Num("casingGravity", c.casingGravity);
    c.casingGroundDy  = _r.Num("casingGroundDy", c.casingGroundDy);
    c.casingLingerSec = _r.Num("casingLingerSec", c.casingLingerSec);
    c.casingFps       = _r.Num("casingFps", c.casingFps);

    c.lowHpFxBelow = _r.Num("lowHpFxBelow", c.lowHpFxBelow);
    (void)_r.NumArray("lowHpFxSec", c.lowHpFxSec);
    (void)_r.NumArray("lowHpFxArea", c.lowHpFxArea);
    if (_r.FindRaw("lowHpFxClip") != nullptr)
    {
        c.lowHpFxClip = _r.Str("lowHpFxClip");
    }

    c.dyingBlastIntervalSec = _r.Num("dyingBlastIntervalSec", c.dyingBlastIntervalSec);
    c.dyingWreckSec         = _r.Num("dyingWreckSec", c.dyingWreckSec);
    c.dyingDoneSec          = _r.Num("dyingDoneSec", c.dyingDoneSec);
    c.wreckSmokeIntervalSec = _r.Num("wreckSmokeIntervalSec", c.wreckSmokeIntervalSec);
    c.wreckOffset           = _r.ReadVec2("wreckOffset", c.wreckOffset);
    c.cannonWreckOffset     = _r.ReadVec2("cannonWreckOffset", c.cannonWreckOffset);

    c.chuteFirstSec     = _r.Num("chuteFirstSec", c.chuteFirstSec);
    c.chuteIntervalSec  = _r.Num("chuteIntervalSec", c.chuteIntervalSec);
    c.chuteSpreadX      = _r.Num("chuteSpreadX", c.chuteSpreadX);
    c.chuteRightMarginX = _r.Num("chuteRightMarginX", c.chuteRightMarginX);
    c.chuteStartAboveY  = _r.Num("chuteStartAboveY", c.chuteStartAboveY);
    c.chuteEdgeX        = _r.Num("chuteEdgeX", c.chuteEdgeX);

    c.turretBlastOffset = _r.ReadVec2("turretBlastOffset", c.turretBlastOffset);
    c.turretSmokeOffset = _r.ReadVec2("turretSmokeOffset", c.turretSmokeOffset);
    c.deathFlameOffset  = _r.ReadVec2("deathFlameOffset", c.deathFlameOffset);
    c.deathBlastDy      = _r.Num("deathBlastDy", c.deathBlastDy);
    c.deathHugeOffset   = _r.ReadVec2("deathHugeOffset", c.deathHugeOffset);
    c.deathDiscOffset   = _r.ReadVec2("deathDiscOffset", c.deathDiscOffset);
}

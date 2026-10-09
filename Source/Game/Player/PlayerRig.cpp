#include "Game/Player/PlayerRig.h"
#include "Base/Types.h"
#include "Base/RGBA.h"

#include <format>

#include "Base/Log.h"
#include "Base/Math.h"

#include "World/Constants.h"

namespace slug
{

namespace
{
    // ===========================================
    //  클립 이름 표
    //   {W} = 무기 토큰(DG/SG/MG), {S} = Left/Right. 패턴을 앞에서부터 시도하고, 무기 토큰도 폴백 순서로 시도한다.
    // ===========================================

    struct SlotDef
    {
        const char* label;
        const char* patterns[3];
        bool        bRequired;
        eRigClip    fallback;   // 자기 자신이면 폴백 없음
    };

    // eRigClip 의 선언 순서와 같아야 한다 (아래 static_assert 로 개수만 확인).
    constexpr SlotDef kSlots[] = {
        {              "Idle",                                 { "Player_Stand_Idle_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {              "Move",                                 { "Player_Stand_Move_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {            "Lookup",                               { "Player_Stand_Lookup_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {       "LookUpDiag3",                          { "Player_Stand_Lookingup_3_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {       "LookUpDiag4",                          { "Player_Stand_Lookingup_4_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {              "Shot",                                 { "Player_Stand_Shot_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {            "ShotUp",                              { "Player_Stand_Shot_Up_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Shot },
        {         "ShotDiag3",                  { "Player_Stand_Shot_Lookingup_3_BT_{W}_{S}", nullptr, nullptr }, false,   eRigClip::LookUpDiag3 },
        {         "ShotDiag4",                  { "Player_Stand_Shot_Lookingup_4_BT_{W}_{S}", nullptr, nullptr }, false,   eRigClip::LookUpDiag4 },
        {            "Slash1",                               { "Player_Stand_Slash1_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {            "Slash2",                               { "Player_Stand_Slash2_{W}_{S}", nullptr, nullptr },  true,        eRigClip::Slash1 },
        {             "Throw",                                { "Player_Stand_Throw_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {        "IdleAction",                          { "Player_Stand_Idle_Action_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {        "CrouchIdle",                                { "Player_Crouch_Idle_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {        "CrouchMove",                                { "Player_Crouch_Move_{W}_{S}", nullptr, nullptr },  true,    eRigClip::CrouchIdle },
        {        "CrouchShot",                                { "Player_Crouch_Shot_{W}_{S}", nullptr, nullptr },  true,    eRigClip::CrouchIdle },
        {      "CrouchSlash1",                              { "Player_Crouch_Slash1_{W}_{S}", nullptr, nullptr },  true,    eRigClip::CrouchIdle },
        {      "CrouchSlash2",                              { "Player_Crouch_Slash2_{W}_{S}", nullptr, nullptr },  true,  eRigClip::CrouchSlash1 },
        {       "CrouchThrow",                               { "Player_Crouch_Throw_{W}_{S}", nullptr, nullptr },  true,    eRigClip::CrouchIdle },
        {          "JumpIdle",                                  { "Player_Jump_Idle_{W}_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {          "JumpMove",                                  { "Player_Jump_Move_{W}_{S}", nullptr, nullptr },  true,      eRigClip::JumpIdle },
        {      "JumpLookDown",                              { "Player_Jump_Lookdown_{W}_{S}", nullptr, nullptr },  true,      eRigClip::JumpIdle },
        { "JumpLookDownDiag1",                         { "Player_Jump_Lookingdown_1_{W}_{S}", nullptr, nullptr },  true,  eRigClip::JumpLookDown },
        { "JumpLookDownDiag2",                         { "Player_Jump_Lookingdown_2_{W}_{S}", nullptr, nullptr },  true,  eRigClip::JumpLookDown },
        {      "JumpShotDown", { "Player_Jump_Shot_Down_{W}_{S}", "Player_Jump_Shot_Lookdown_{W}_{S}", nullptr },  true,  eRigClip::JumpLookDown },
        {         "LowerIdle",                               { "Player_Stand_Idle_Lower_{S}", nullptr, nullptr },  true,     eRigClip::LowerIdle },
        {         "LowerMove",                               { "Player_Stand_Move_Lower_{S}", nullptr, nullptr },  true,     eRigClip::LowerIdle },
        {     "LowerJumpIdle",                                { "Player_Jump_Idle_Lower_{S}", nullptr, nullptr },  true,     eRigClip::LowerIdle },
        {     "LowerJumpMove",                                { "Player_Jump_Move_Lower_{S}", nullptr, nullptr },  true, eRigClip::LowerJumpIdle },
        {             "Death",                              { "Player_Death_byBullet_DG_{S}", nullptr, nullptr },  true,          eRigClip::Idle },
        {           "Victory",                                        { "Player_Victory_{W}", nullptr, nullptr },  true,          eRigClip::Idle },
    };
    static_assert(sizeof(kSlots) / sizeof(kSlots[0]) == kRigClipCount, "kSlots 는 eRigClip 과 같은 개수여야 한다.");

    // 무기별 토큰 폴백 순서
    constexpr const char* kPistolTokens[]  = { "DG", nullptr, nullptr };
    constexpr const char* kHmgTokens[]     = { "MG", "SG", "DG" };
    constexpr const char* kShotgunTokens[] = { "SG", "DG", nullptr };
    constexpr const char* kRocketTokens[]  = { "MG", "SG", "DG" };
    constexpr const char* kFlameTokens[]   = { "SG", "MG", "DG" };

    [[nodiscard]] const char* const* GetTokens_(
        const eWeapon _weapon)
    {
        switch (_weapon)
        {
            case eWeapon::Hmg: return kHmgTokens;
            case eWeapon::Shotgun: return kShotgunTokens;
            case eWeapon::Rocket: return kRocketTokens;
            case eWeapon::Flame: return kFlameTokens;
            default: return kPistolTokens;
        }
    }

    [[nodiscard]] slug::String Expand_(
        const char* _pattern,
        const char* _weaponToken,
        const bool  _bRight)
    {
        slug::String result  = _pattern;
        const auto  Replace = [&](const slug::StringView _key, const slug::StringView _value) {
            const size_t pos = result.find(_key);
            if (pos != slug::String::npos)
            {
                result.replace(pos, _key.size(), _value);
            }
        };
        Replace("{W}", _weaponToken);
        Replace("{S}", _bRight ? "Right" : "Left");
        return result;
    }

    [[nodiscard]] bool IsAirTimelineSlot_(
        const eRigClip _slot)
    {
        return _slot == eRigClip::JumpIdle || _slot == eRigClip::JumpMove || _slot == eRigClip::LowerJumpIdle || _slot == eRigClip::LowerJumpMove;
    }

    [[nodiscard]] int FacingIndex_(
        const int _facing)
    {
        return _facing < 0 ? 0 : 1;
    }
}   // namespace

const char* GetRigClipName(
    const eRigClip _clip)
{
    const size_t index = static_cast<size_t>(_clip);
    return index < kRigClipCount ? kSlots[index].label : "?";
}

bool PlayerRig::IsSlotRequired(
    const eRigClip _slot)
{
    return kSlots[static_cast<size_t>(_slot)].bRequired;
}

slug::String PlayerRig::ResolveClipName(
    const AnimationLibrary& _library,
    const eRigClip          _slot,
    const eWeapon           _weapon,
    const int               _facing)
{
    const SlotDef&     def    = kSlots[static_cast<size_t>(_slot)];
    const char* const* tokens = GetTokens_(_weapon);

    for (int t = 0; t < 3; ++t)
    {
        if (!tokens[t])
        {
            continue;
        }
        for (const char* pattern: def.patterns)
        {
            if (!pattern)
            {
                continue;
            }
            slug::String name = Expand_(pattern, tokens[t], _facing > 0);
            if (!_library.Find(name).IsNull())
            {
                return name;
            }
        }
    }
    return {};
}

PlayerRig::BindReport PlayerRig::Bind(
    const AnimationLibrary& _library,
    const RigTuning&        _tuning)
{
    m_pLibrary = &_library;
    m_tuning   = _tuning;
    m_upper    = {};
    m_lower    = {};

    BindReport report;
    for (size_t w = 0; w < kWeaponCount; ++w)
    {
        for (int side = 0; side < 2; ++side)
        {
            const int facing = side == 0 ? -1 : 1;
            for (size_t s = 0; s < kRigClipCount; ++s)
            {
                const eRigClip    slot = static_cast<eRigClip>(s);
                const slug::String name = ResolveClipName(_library, slot, static_cast<eWeapon>(w), facing);

                Slot& entry = m_tables[w][static_cast<size_t>(side)][s];
                entry       = {};
                if (name.empty())
                {
                    if (kSlots[s].bRequired)
                    {
                        ++report.missingRequired;
                    }
                    report.missingNames.push_back(std::format("weapon={} facing={} slot={}", w, facing, kSlots[s].label));
                    continue;
                }

                entry.id = _library.Find(name);
                if (const AnimClip* pClip = _library.GetOrNull(entry.id))
                {
                    entry.baseline = pClip->GetFootBaseline();
                }
            }
        }
    }

    if (report.missingRequired > 0)
    {
        SLUG_LOG_WARN("PlayerRig: 필수 클립 {}개 없음 (폴백 사용). 예: {}", report.missingRequired, report.missingNames.empty() ? slug::String {} : report.missingNames.front());
    }
    return report;
}

const PlayerRig::SlotTable& PlayerRig::GetTable_(
    const eWeapon _weapon,
    const int     _facing) const
{
    return m_tables[static_cast<size_t>(_weapon)][static_cast<size_t>(FacingIndex_(_facing))];
}

PlayerRig::Slot PlayerRig::GetSlot_(
    const SlotTable& _table,
    const eRigClip   _slot) const
{
    eRigClip current = _slot;
    for (int guard = 0; guard < 4; ++guard)
    {
        const Slot& entry = _table[static_cast<size_t>(current)];
        if (!entry.id.IsNull())
        {
            return entry;
        }
        const eRigClip next = kSlots[static_cast<size_t>(current)].fallback;
        if (next == current)
        {
            break;
        }
        current = next;
    }
    return {};
}

AnimId PlayerRig::GetSlotClip(
    const eWeapon  _weapon,
    const int      _facing,
    const eRigClip _slot) const
{
    return GetSlot_(GetTable_(_weapon, _facing), _slot).id;
}

float PlayerRig::ClipTicks_(
    const AnimId _clip) const
{
    const AnimClip* pClip = m_pLibrary ? m_pLibrary->GetOrNull(_clip) : nullptr;
    return pClip ? pClip->GetTotalDurationSec() * 60.f : 1.f;
}

void PlayerRig::Update(
    const RigState& _state)
{
    if (!m_pLibrary)
    {
        return;
    }

    const SlotTable& table = GetTable_(_state.weapon, _state.facing);

    // ===========================================
    //  슬롯 선택
    // ===========================================

    eRigClip upperSlot = eRigClip::Idle;
    eRigClip lowerSlot = eRigClip::LowerIdle;
    bool     bLower    = false;

    const bool bDead    = _state.state == ePlayerState::Dead;
    const bool bVictory = _state.state == ePlayerState::Victory;
    const int  diag     = _state.aim == eAim::DiagUp && m_tuning.diagUpVariant == 4 ? 4 : 3;

    if (bDead)
    {
        upperSlot = eRigClip::Death;
    }
    else if (bVictory)
    {
        upperSlot = eRigClip::Victory;
    }
    else if (_state.pose == ePose::Crouch)
    {
        switch (_state.overlay)
        {
            case eOverlay::Shot: upperSlot = eRigClip::CrouchShot; break;
            case eOverlay::Slash: upperSlot = _state.bSlashSecond ? eRigClip::CrouchSlash2 : eRigClip::CrouchSlash1; break;
            case eOverlay::Throw: upperSlot = eRigClip::CrouchThrow; break;
            default: upperSlot = _state.bMoving ? eRigClip::CrouchMove : eRigClip::CrouchIdle; break;
        }
    }
    else
    {
        const bool bAir = _state.pose == ePose::Air;
        bLower          = true;
        lowerSlot       = bAir ? (_state.bMoving ? eRigClip::LowerJumpMove : eRigClip::LowerJumpIdle) : (_state.bMoving ? eRigClip::LowerMove : eRigClip::LowerIdle);

        // 기본 상체 (겹침 동작이 없을 때)
        eRigClip base = bAir ? (_state.bMoving ? eRigClip::JumpMove : eRigClip::JumpIdle) : (_state.bMoving ? eRigClip::Move : eRigClip::Idle);
        if (!bAir && _state.bIdleAction && !_state.bMoving && _state.aim == eAim::Forward)
        {
            base = eRigClip::IdleAction;
        }
        switch (_state.aim)
        {
            case eAim::Up: base = eRigClip::Lookup; break;
            case eAim::DiagUp: base = diag == 4 ? eRigClip::LookUpDiag4 : eRigClip::LookUpDiag3; break;
            case eAim::Down: base = bAir ? eRigClip::JumpLookDown : base; break;
            case eAim::DiagDown: base = bAir ? eRigClip::JumpLookDownDiag1 : base; break;
            case eAim::Forward: break;
        }

        switch (_state.overlay)
        {
            case eOverlay::Shot:
                switch (_state.aim)
                {
                    case eAim::Up: upperSlot = eRigClip::ShotUp; break;
                    case eAim::DiagUp: upperSlot = diag == 4 ? eRigClip::ShotDiag4 : eRigClip::ShotDiag3; break;
                    case eAim::Down:
                    case eAim::DiagDown: upperSlot = bAir ? eRigClip::JumpShotDown : eRigClip::Shot; break;
                    case eAim::Forward: upperSlot = eRigClip::Shot; break;
                }
                break;
            case eOverlay::Slash: upperSlot = _state.bSlashSecond ? eRigClip::Slash2 : eRigClip::Slash1; break;
            case eOverlay::Throw: upperSlot = eRigClip::Throw; break;
            case eOverlay::None: upperSlot = base; break;
        }
    }

    const Slot upper = GetSlot_(table, upperSlot);
    const Slot lower = bLower ? GetSlot_(table, lowerSlot) : Slot {};

    // ===========================================
    //  원점 높이 (발 기준선)
    // ===========================================

    if (bDead || bVictory || _state.pose == ePose::Crouch)
    {
        m_originLift = static_cast<float>(upper.baseline);
    }
    else if (_state.pose == ePose::Air)
    {
        m_originLift = m_tuning.airBaseline;
    }
    else
    {
        m_originLift = static_cast<float>(lower.baseline);
    }

    // ===========================================
    //  하체 (기준 타임라인)
    // ===========================================

    m_bHasLower = bLower && !lower.id.IsNull();
    if (m_bHasLower)
    {
        if (m_lower.GetClip() != lower.id)
        {
            m_lower.Play(lower.id, true);
        }
        if (IsAirTimelineSlot_(lowerSlot))
        {
            const AnimClip& clip   = m_pLibrary->Get(lower.id);
            const float     length = slug::Max(1.f, ClipTicks_(lower.id));
            const float     start  = _state.bAirFromJump ? 0.f : 0.4f * length;
            m_lower.SetNormalizedTime(clip, slug::Clamp((static_cast<float>(_state.airTicks) + start) / length, 0.f, 1.f));
        }
        else
        {
            m_lower.Update(*m_pLibrary, kStepSec);
        }
    }
    m_lowerSlot = lowerSlot;

    // ===========================================
    //  상체
    // ===========================================

    if (upper.id.IsNull())
    {
        return;
    }

    const bool bNewOverlay = _state.overlay != eOverlay::None && _state.overlaySerial != m_lastOverlaySerial;
    const bool bNewState   = _state.stateSerial != m_lastStateSerial;
    m_lastOverlaySerial    = _state.overlaySerial;
    m_lastStateSerial      = _state.stateSerial;

    if (m_upper.GetClip() != upper.id || bNewOverlay || bNewState)
    {
        m_upper.Play(upper.id, true);
    }
    m_upperSlot = upperSlot;

    const AnimClip& upperClip = m_pLibrary->Get(upper.id);
    if (IsAirTimelineSlot_(upperSlot))
    {
        const float length = slug::Max(1.f, ClipTicks_(upper.id));
        const float start  = _state.bAirFromJump ? 0.f : 0.4f * length;
        m_upper.SetNormalizedTime(upperClip, slug::Clamp((static_cast<float>(_state.airTicks) + start) / length, 0.f, 1.f));
    }
    else if (_state.overlay != eOverlay::None && !bDead && !bVictory)
    {
        // 겹침 동작은 정해진 틱 안에 클립 전체가 재생되도록 속도를 맞춘다.
        const float speed = ClipTicks_(upper.id) / static_cast<float>(slug::Max(1, _state.overlayDurationTicks));
        m_upper.Update(*m_pLibrary, kStepSec, speed);
    }
    else
    {
        m_upper.Update(*m_pLibrary, kStepSec);
        if (upperSlot == eRigClip::Move && m_bHasLower && lowerSlot == eRigClip::LowerMove)
        {
            m_upper.SyncPhase(upperClip, m_lower, m_pLibrary->Get(lower.id));
        }
    }
}

void PlayerRig::Submit(
    SpriteBatch&       _sprites,
    const Vec2 _feet,
    const int          _layer,
    const int          _sortKey,
    const slug::RGBA    _tint) const
{
    if (!m_pLibrary)
    {
        return;
    }

    const Vec2 origin = GetOrigin(_feet);
    SpriteDrawOptions  options;
    options.tint = _tint;

    if (m_bHasLower)
    {
        if (const AnimFrame* pFrame = m_lower.GetFrameData(*m_pLibrary))
        {
            _sprites.Submit(_layer, _sortKey, *pFrame, origin, options);
        }
    }
    if (const AnimFrame* pFrame = m_upper.GetFrameData(*m_pLibrary))
    {
        _sprites.Submit(_layer, _sortKey, *pFrame, origin, options);
    }
}

}   // namespace slug

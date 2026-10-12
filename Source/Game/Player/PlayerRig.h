#pragma once
#include "Base/Handle.h"
#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Player/PlayerTypes.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

enum class eRigClip : uint8_t
{
    Idle,
    Move,
    Lookup,
    LookUpDiag3,
    LookUpDiag4,
    Shot,
    ShotUp,
    ShotDiag3,
    ShotDiag4,
    Slash1,
    Slash2,
    Throw,
    IdleAction,
    CrouchIdle,
    CrouchMove,
    CrouchShot,
    CrouchSlash1,
    CrouchSlash2,
    CrouchThrow,
    JumpIdle,
    JumpMove,
    JumpLookDown,
    JumpLookDownDiag1,
    JumpLookDownDiag2,
    JumpShotDown,
    LowerIdle,
    LowerMove,
    LowerJumpIdle,
    LowerJumpMove,
    Death,
    Victory,
    Count
};

inline constexpr size_t kRigClipCount = static_cast<size_t>(eRigClip::Count);

[[nodiscard]] const char* GetRigClipName(eRigClip _clip);

struct RigState
{
    eWeapon      weapon               = eWeapon::Pistol;
    ePlayerState state                = ePlayerState::Idle;
    ePose        pose                 = ePose::Stand;
    bool         bMoving              = false;
    int          facing               = 1;
    eAim         aim                  = eAim::Forward;
    eOverlay     overlay              = eOverlay::None;
    bool         bSlashSecond         = false;
    uint32_t     overlaySerial        = 0;
    int          overlayDurationTicks = 0;   // 겹침 동작 클립이 이 틱 수에 맞춰 재생된다
    int          airTicks             = 0;   // 공중에 뜬 뒤 경과 틱
    bool         bAirFromJump         = true;
    uint32_t     stateSerial          = 0;   // 사망 등 큰 상태 전환마다 증가
    bool         bIdleAction          = false;
    bool         bVisible             = true;
};

class PlayerRig
{
public:
    struct BindReport
    {
        int            missingRequired = 0;
        Vector<String> missingNames    = {};
    };

    BindReport Bind(const AnimationLibrary& _library, const RigTuning& _tuning);

    void Update(const RigState& _state);

    void Submit(
        SpriteBatch& _sprites,
        Vec2         _feet,
        int          _depth,
        int          _sortKey,
        RGBA         _tint = { 255, 255, 255, 255 }) const;

    [[nodiscard]] AnimId GetUpperClip() const;
    [[nodiscard]] AnimId GetLowerClip() const;
    [[nodiscard]] int    GetUpperFrame() const;
    [[nodiscard]] int    GetLowerFrame() const;
    [[nodiscard]] float  GetOriginLift() const;
    [[nodiscard]] Vec2   GetOrigin(Vec2 _feet) const;

    [[nodiscard]] AnimId GetSlotClip(eWeapon _weapon, int _facing, eRigClip _slot) const;

    [[nodiscard]] static String ResolveClipName(
        const AnimationLibrary& _library,
        eRigClip                _slot,
        eWeapon                 _weapon,
        int                     _facing);

    [[nodiscard]] static bool IsSlotRequired(eRigClip _slot);

private:
    struct Slot
    {
        AnimId id       = kNullHandle;
        int    baseline = 0;   // 클립 프레임들 중 발 쪽으로 가장 낮은 바닥 (원점 아래 px)
    };
    using SlotTable = ARRAY<Slot, kRigClipCount>;

    [[nodiscard]] const SlotTable& GetTable_(eWeapon _weapon, int _facing) const;
    [[nodiscard]] Slot             GetSlot_(const SlotTable& _table, eRigClip _slot) const;
    [[nodiscard]] float            ClipTicks_(AnimId _clip) const;

    const AnimationLibrary* m_pLibrary = nullptr;
    RigTuning               m_tuning   = {};

    ARRAY<ARRAY<SlotTable, 2>, kWeaponCount> m_tables = {};   // [weapon][0 = Left, 1 = Right]

    AnimPlayer m_upper     = {};
    AnimPlayer m_lower     = {};
    bool       m_bHasLower = false;

    eRigClip m_upperSlot         = eRigClip::Idle;
    eRigClip m_lowerSlot         = eRigClip::LowerIdle;
    uint32_t m_lastOverlaySerial = 0;
    uint32_t m_lastStateSerial   = 0;
    float    m_originLift        = 0.f;
};

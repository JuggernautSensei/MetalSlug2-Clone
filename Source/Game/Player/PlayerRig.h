#pragma once
#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Player/PlayerTypes.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

enum class eRigClip
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
    int          overlayDurationTicks = 0;
    int          airTicks             = 0;
    bool         bAirFromJump         = true;
    uint32_t     stateSerial          = 0;
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

    BindReport                  Bind(const AnimationLibrary& _library, const RigTuning& _tuning);
    void                        Update(const RigState& _state);
    void                        Submit(SpriteBatch& _sprites, VECTOR2 _feet, int _depth, int _sortKey, RGBA _tint = { 255, 255, 255, 255 }) const;
    [[nodiscard]] AnimId        GetUpperClip() const;
    [[nodiscard]] AnimId        GetLowerClip() const;
    [[nodiscard]] int           GetUpperFrame() const;
    [[nodiscard]] int           GetLowerFrame() const;
    [[nodiscard]] float         GetOriginLift() const;
    [[nodiscard]] VECTOR2       GetOrigin(VECTOR2 _feet) const;
    [[nodiscard]] AnimId        GetSlotClip(eWeapon _weapon, int _facing, eRigClip _slot) const;
    [[nodiscard]] static String ResolveClipName(const AnimationLibrary& _library, eRigClip _slot, eWeapon _weapon, int _facing);
    [[nodiscard]] static bool   IsSlotRequired(eRigClip _slot);

private:
    struct Slot
    {
        AnimId id       = kNullHandle;
        int    baseline = 0;
    };
    using SlotTable = ARRAY<Slot, kRigClipCount>;

    [[nodiscard]] const SlotTable& GetTable_(eWeapon _weapon, int _facing) const;
    [[nodiscard]] Slot             GetSlot_(const SlotTable& _table, eRigClip _slot) const;
    [[nodiscard]] float            ClipTicks_(AnimId _clip) const;

    const AnimationLibrary* m_pLibrary = nullptr;
    RigTuning               m_tuning   = {};

    ARRAY<ARRAY<SlotTable, 2>, (CountOf<eWeapon>() - 1)> m_tables = {};

    AnimPlayer m_upper     = {};
    AnimPlayer m_lower     = {};
    bool       m_bHasLower = false;

    uint32_t m_lastOverlaySerial = 0;
    uint32_t m_lastStateSerial   = 0;
    float    m_originLift        = 0.f;
};

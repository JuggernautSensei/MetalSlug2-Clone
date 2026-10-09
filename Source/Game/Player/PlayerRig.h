#pragma once
#include "Base/Handle.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Player/PlayerTypes.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{

// ===========================================
//  eRigClip
//   리그가 쓰는 "자리(slot)". 실제 클립 이름은 (무기, 좌우) 로 결정되는 표에서 한 번만 해석한다.
// ===========================================

enum class eRigClip : uint8_t
{
    // --- 상체 (서 있기) ---
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
    // --- 앉기 (하체 포함 전신) ---
    CrouchIdle,
    CrouchMove,
    CrouchShot,
    CrouchSlash1,
    CrouchSlash2,
    CrouchThrow,
    // --- 공중 상체 ---
    JumpIdle,
    JumpMove,
    JumpLookDown,
    JumpLookDownDiag1,
    JumpLookDownDiag2,
    JumpShotDown,
    // --- 하체 ---
    LowerIdle,
    LowerMove,
    LowerJumpIdle,
    LowerJumpMove,
    // --- 전신 ---
    Death,
    Victory,
    Count
};

inline constexpr size_t kRigClipCount = static_cast<size_t>(eRigClip::Count);

[[nodiscard]] const char* GetRigClipName(eRigClip _clip);

// ===========================================
//  RigState
//   컨트롤러가 이번 틱 결과로 채워 주는 "무엇을 보여줄지" 정보. 리그는 게임 로직을 모른다.
// ===========================================

struct RigState
{
    eWeapon      weapon               = eWeapon::Pistol;
    ePlayerState state                = ePlayerState::Idle;
    ePose        pose                 = ePose::Stand;
    bool         bMoving              = false;
    int          facing               = 1;
    eAim         aim                  = eAim::Forward;
    eOverlay     overlay              = eOverlay::None;
    bool         bSlashSecond         = false;   // 두 번째 베기 동작
    uint32_t     overlaySerial        = 0;       // 겹침 동작이 새로 시작될 때마다 증가 (같은 동작도 다시 처음부터)
    int          overlayDurationTicks = 0;       // 겹침 동작 클립이 이 틱 수에 맞춰 재생된다
    int          airTicks             = 0;       // 공중에 뜬 뒤 경과 틱
    bool         bAirFromJump         = true;    // false: 낭떠러지에서 떨어짐 (낙하 자세부터 시작)
    uint32_t     stateSerial          = 0;       // 사망 등 큰 상태 전환마다 증가
    bool         bIdleAction          = false;
    bool         bVisible             = true;   // 무적 깜빡임
};

// ===========================================
//  PlayerRig
//   상체 + 하체를 하나의 리그로 합성한다.
//    - 하체 타임라인이 기준이고, 상체(달리기)는 같은 위상(SyncPhase)을 공유한다.
//    - 둘 다 같은 원점에 그린다 (원본 에셋이 같은 원점 기준으로 만들어졌고 피벗이 프레임별로 구워져 있음).
//    - 원점의 높이는 하체 클립의 발 기준선(프레임 중 가장 낮은 바닥)으로 정해 발이 지면에 정확히 닿게 한다.
//   클립이 없으면 폴백 체인을 따르고, 한 번만 경고를 남긴다.
// ===========================================

class PlayerRig
{
public:
    struct BindReport
    {
        int                      missingRequired = 0;
        slug::Vector<slug::String> missingNames    = {};   // "무기/좌우/자리" 형태의 설명
    };

    // 클립 이름 표를 해석한다. 라이브러리는 이후에도 계속 유효해야 한다 (비소유).
    BindReport Bind(const AnimationLibrary& _library, const RigTuning& _tuning);

    void Update(const RigState& _state);

    // 현재 프레임들을 SpriteBatch 에 제출한다 (_feet = 발 중앙).
    void Submit(SpriteBatch& _sprites, Vec2 _feet, int _layer, int _sortKey, slug::RGBA _tint = { 255, 255, 255, 255 }) const;

    // --- 조회 (테스트/디버그) ---
    [[nodiscard]] bool         IsBound() const { return m_pLibrary != nullptr; }
    [[nodiscard]] AnimId       GetUpperClip() const { return m_upper.GetClip(); }
    [[nodiscard]] AnimId       GetLowerClip() const { return m_lower.GetClip(); }
    [[nodiscard]] int          GetUpperFrame() const { return m_upper.GetFrame(); }
    [[nodiscard]] int          GetLowerFrame() const { return m_lower.GetFrame(); }
    [[nodiscard]] bool         IsUpperFinished() const { return m_upper.IsFinished(); }
    [[nodiscard]] float        GetOriginLift() const { return m_originLift; }
    [[nodiscard]] Vec2 GetOrigin(Vec2 _feet) const { return { _feet.x, _feet.y - m_originLift }; }

    // 슬롯 → AnimId (없으면 null 핸들). 테스트용.
    [[nodiscard]] AnimId GetSlotClip(eWeapon _weapon, int _facing, eRigClip _slot) const;

    // 라이브러리에서 해석한 실제 클립 이름 (없으면 빈 문자열)
    [[nodiscard]] static slug::String ResolveClipName(const AnimationLibrary& _library, eRigClip _slot, eWeapon _weapon, int _facing);

    // 슬롯이 필수인가 (선택 슬롯은 없어도 폴백이 있다)
    [[nodiscard]] static bool IsSlotRequired(eRigClip _slot);

private:
    struct Slot
    {
        AnimId id       = slug::kNullHandle;
        int    baseline = 0;   // 클립 프레임들 중 발 쪽으로 가장 낮은 바닥 (원점 아래 px)
    };
    using SlotTable = slug::ARRAY<Slot, kRigClipCount>;

    [[nodiscard]] const SlotTable& GetTable_(eWeapon _weapon, int _facing) const;
    [[nodiscard]] Slot             GetSlot_(const SlotTable& _table, eRigClip _slot) const;
    [[nodiscard]] float            ClipTicks_(AnimId _clip) const;

    const AnimationLibrary* m_pLibrary = nullptr;
    RigTuning               m_tuning   = {};

    slug::ARRAY<slug::ARRAY<SlotTable, 2>, kWeaponCount> m_tables = {};   // [weapon][0 = Left, 1 = Right]

    AnimPlayer m_upper     = {};
    AnimPlayer m_lower     = {};
    bool       m_bHasLower = false;

    eRigClip m_upperSlot         = eRigClip::Idle;
    eRigClip m_lowerSlot         = eRigClip::LowerIdle;
    uint32_t m_lastOverlaySerial = 0;
    uint32_t m_lastStateSerial   = 0;
    float    m_originLift        = 0.f;
};

}   // namespace slug

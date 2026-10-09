#pragma once
#include <cstdint>
#include <optional>

#include "Base/Random.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "Render/Camera.h"
#include "World/Geometry.h"

namespace slug
{

// 카메라가 플레이어를 따라갈 때 목표점이 발보다 얼마나 위인가 (SandboxState 와 같은 값)
inline constexpr float kStageCameraTargetLift = 60.f;

// ===========================================
//  ScreenWalls
//   플레이어를 화면 안(좌우 playerMargin)에 가두는 보이지 않는 벽. PhysicsWorld::walls 에 그대로 넣는다.
//   왼쪽: 전진 전용 카메라이므로 항상. 오른쪽: 카메라 락 중이거나 카메라가 경계 오른쪽 끝에 닿았을 때(또는 이동 중).
// ===========================================

struct ScreenWalls
{
    Aabb left         = {};
    Aabb right        = {};
    bool bHasLeft     = false;
    bool bHasRight    = false;
};

// ===========================================
//  StageRunner
//   StageData 를 고정 60Hz 틱으로 결정적으로 실행한다.
//   한 틱의 순서: 섹션 트리거 → 예약 큐(지연 액션/웨이브) → 경계 곡선 → 게이트 → 카메라 추적/갱신.
//   - 시간은 모두 틱 정수 (초 * 60 반올림). 예약 시각 = 실행 틱 + delayTicks, 같은 틱은 예약 순서대로.
//   - 외부(적/소품/UI/오디오)는 전부 IStageHost 를 통해서만 만진다. 카메라는 직접 조작한다.
//   - RNG(카메라 기준 스폰의 dx 범위)는 호출자가 준 시드 RNG 를 쓴다.
// ===========================================

class StageRunner
{
public:
    StageRunner(const StageData& _data, IStageHost& _host, Camera& _camera, slug::RngMt19937& _rng);

    // 초기 상태 적용: 지면/원경/BGM/카메라 경계·offset·전진 정책. 틱 카운터와 진행 상태는 초기화된다.
    void Begin();

    // 스크립트의 player 항목을 해소한 발 위치 (groundSnap 은 host.GetGroundFindY). 지면을 못 찾으면 nullopt.
    [[nodiscard]] std::optional<Vec2> ResolvePlayerStart() const;

    // 한 틱 진행. endStage 이후에는 아무 일도 하지 않는다.
    void Tick();

    // --- 디버그 워프 ---
    // 아직 발동하지 않은 섹션 중 _index 앞의 것들을 silent 로(when: silent/always 액션만, 지연 없이) 실행하고,
    // 플레이어를 해당 섹션 triggerX + 1 의 지면으로 옮긴 뒤 카메라를 맞춘다. 대상 섹션 자체는 다음 틱에 정상 발동한다.
    // 이미 지나간 섹션으로는 돌아갈 수 없다(false). 대기 중인 웨이브/게이트/비영구 락은 정리한다.
    bool WarpToSection(int _index);
    bool WarpToSection(slug::StringView _id);
    bool WarpToBoss();   // id "boss" 섹션, 없으면 마지막 섹션

    // --- 카메라 / 플레이어 제약 ---
    [[nodiscard]] Vec2 ComputeCameraTarget(Vec2 _playerFeet) const;
    [[nodiscard]] ScreenWalls  MakeScreenWalls() const;

    // --- 상태 조회 (UI/디버그/테스트) ---
    [[nodiscard]] bool     IsFinished() const { return m_bFinished; }
    [[nodiscard]] int64_t  GetTick() const { return m_tick; }
    [[nodiscard]] int      GetNextSectionIndex() const { return m_nextSection; }   // 아직 발동하지 않은 첫 섹션
    [[nodiscard]] std::optional<float> GetNextTriggerX() const;
    [[nodiscard]] int      GetPendingActionCount() const;   // 예약 큐의 지연 액션 수 (웨이브 제외)
    [[nodiscard]] int      GetPendingWaveCount() const;
    [[nodiscard]] int      GetOpenGateCount() const { return static_cast<int>(m_gates.size()); }
    [[nodiscard]] slug::Vector<slug::String> GetOpenGateIds() const;

    [[nodiscard]] const slug::String& GetGroundName() const { return m_groundName; }
    [[nodiscard]] const slug::String& GetBackdropMode() const { return m_backdropMode; }
    [[nodiscard]] Vec2       GetCameraOffset() const { return m_cameraOffset; }
    [[nodiscard]] const Aabb&        GetBaseBounds() const { return m_base; }
    [[nodiscard]] Aabb               GetEffectiveBounds() const;   // 최상단 락 사각형 또는 기본 경계
    [[nodiscard]] int                GetLockDepth() const { return static_cast<int>(m_locks.size()); }
    [[nodiscard]] bool               HasPermanentLock() const;
    [[nodiscard]] bool               IsCurveActive(size_t _curveIndex) const;

    // 스크립트 spawn id 로 만든 SpawnId 들 (repeat 면 여러 개). 아직 스폰 전이면 빈 span.
    [[nodiscard]] slug::Span<const SpawnId> FindSpawnIds(slug::StringView _stageId) const;
    [[nodiscard]] bool                     WasSpawnSkipped(slug::StringView _stageId) const;

    [[nodiscard]] int GetSpawnedCount() const { return m_spawnedCount; }
    [[nodiscard]] int GetSkippedCount() const { return m_skippedCount; }

    // 러너 상태 해시 (결정성 검증)
    [[nodiscard]] uint64_t HashState() const;

private:
    struct PendingItem
    {
        int64_t            dueTick          = 0;
        uint64_t           seq              = 0;
        const StageAction* pAction          = nullptr;   // 지연 액션
        const WaveEntry*   pWave            = nullptr;   // 지연 웨이브 엔트리
        int                sectionIndex     = -1;
        bool               bAbortOnGameOver = false;
    };

    struct GateState
    {
        const StageAction* pAction          = nullptr;
        int                sectionIndex     = -1;
        bool               bAbortOnGameOver = false;
    };

    struct LockEntry
    {
        Aabb rect       = {};
        bool bPermanent = false;
    };

    struct SpawnRecord
    {
        slug::String           stageId = {};
        slug::Vector<SpawnId>  ids     = {};   // 비어 있으면 건너뜀(예: groundSnap 실패)
    };

    struct ExecContext
    {
        int  sectionIndex = -1;
        bool bSilent      = false;
        bool bAbort       = false;
    };

    [[nodiscard]] static int64_t ToTicks_(float _seconds);

    void FireSection_(int _index, bool _bSilent);
    void RunActions_(const slug::Vector<StageAction>& _actions, const ExecContext& _ctx);
    void Execute_(const StageAction& _action, const ExecContext& _ctx);
    void ExecSpawn_(const SpawnSpec& _spec, const ExecContext& _ctx, bool _bWave);
    void ExecLock_(const StageAction& _action, const ExecContext& _ctx);
    void ExecUnlock_();
    void ExecClear_(const StageAction& _action);

    void ProcessPending_();
    void ProcessCurves_();
    void ProcessGates_();
    void CancelAbortables_();
    [[nodiscard]] bool IsGateSatisfied_(const GateState& _gate) const;

    void ApplyCameraBounds_();
    void ApplyCameraLock_();
    [[nodiscard]] Aabb CurrentViewRect_() const;

    SpawnRecord*       FindRecord_(slug::StringView _stageId);
    const SpawnRecord* FindRecord_(slug::StringView _stageId) const;

    const StageData*  m_pData;
    IStageHost*       m_pHost;
    Camera*           m_pCamera;
    slug::RngMt19937*  m_pRng;

    int64_t m_tick        = 0;
    int     m_nextSection = 0;
    bool    m_bFinished   = false;

    slug::Vector<PendingItem> m_pending       = {};
    uint64_t                 m_nextSeq       = 1;
    uint64_t                 m_waveCutoffSeq = 0;   // seq < cutoff 인 웨이브 엔트리는 취소됨
    slug::Vector<GateState>   m_gates         = {};

    slug::Vector<SpawnRecord> m_records      = {};
    int                      m_spawnedCount = 0;
    int                      m_skippedCount = 0;

    // 카메라 상태
    Aabb                   m_base         = {};
    slug::Vector<LockEntry> m_locks        = {};
    Vec2           m_cameraOffset = { 0.f, 0.f };
    slug::Vector<uint8_t>   m_curveActive  = {};

    slug::String m_groundName   = {};
    slug::String m_backdropMode = {};
};

}   // namespace slug

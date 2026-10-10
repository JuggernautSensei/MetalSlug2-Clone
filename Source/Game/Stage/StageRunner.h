#pragma once
#include <cstdint>
#include <optional>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "Render/Camera.h"
#include "World/Geometry.h"

namespace slug
{

inline constexpr float kStageCameraTargetLift = 60.f;

struct ScreenWalls
{
    Aabb left      = {};
    Aabb right     = {};
    bool bHasLeft  = false;
    bool bHasRight = false;
};

class StageRunner
{
public:
    StageRunner(
        const StageData& _data,
        IStageHost&      _host,
        Camera&          _camera,
        RngMt19937&      _rng);

    StageRunner(const StageRunner&)            = delete;
    StageRunner& operator=(const StageRunner&) = delete;
    StageRunner(StageRunner&&)                 = delete;
    StageRunner& operator=(StageRunner&&)      = delete;

    void Begin();

    [[nodiscard]] std::optional<Vec2> ResolvePlayerStart() const;

    void Tick();

    // 자동 전진 (보스 스크롤): 카메라 중심 x 하한을 _dx 만큼 올린다 (Camera::AddScrollFloor). StageState 가 적 틱 뒤에 호출한다.
    void AddCameraScroll(float _dx);

    bool WarpToSection(int _index);
    bool WarpToSection(StringView _id);
    bool WarpToBoss();
    bool WarpToX(float _x);   // 디버그: 임의 월드 x 로 워프 (앞 구간은 조용히 적용)

    [[nodiscard]] Vec2        ComputeCameraTarget(Vec2 _playerFeet) const;
    [[nodiscard]] ScreenWalls MakeScreenWalls() const;

    [[nodiscard]] bool                 IsFinished() const;
    [[nodiscard]] int64_t              GetTick() const;
    [[nodiscard]] int                  GetNextSectionIndex() const;
    [[nodiscard]] std::optional<float> GetNextTriggerX() const;
    [[nodiscard]] int                  GetPendingActionCount() const;
    [[nodiscard]] int                  GetPendingWaveCount() const;
    [[nodiscard]] int                  GetOpenGateCount() const;
    [[nodiscard]] Vector<String>       GetOpenGateIds() const;

    [[nodiscard]] const String& GetGroundName() const;
    [[nodiscard]] const String& GetBackdropMode() const;
    [[nodiscard]] Vec2          GetCameraOffset() const;
    [[nodiscard]] const Aabb&   GetBaseBounds() const;
    [[nodiscard]] Aabb          GetEffectiveBounds() const;
    [[nodiscard]] int           GetLockDepth() const;
    [[nodiscard]] bool          HasPermanentLock() const;
    [[nodiscard]] bool          IsCurveActive(size_t _curveIndex) const;

    [[nodiscard]] Span<const SpawnId> FindSpawnIds(StringView _stageId) const;
    [[nodiscard]] bool                WasSpawnSkipped(StringView _stageId) const;

    [[nodiscard]] int GetSpawnedCount() const;
    [[nodiscard]] int GetSkippedCount() const;

    [[nodiscard]] uint64_t HashState() const;

private:
    struct PendingItem
    {
        int64_t            dueTick          = 0;
        uint64_t           seq              = 0;
        const StageAction* pAction          = nullptr;
        const WaveEntry*   pWave            = nullptr;
        int                sectionIndex     = -1;
        bool               bAbortOnGameOver = false;
        int                repeatsLeft      = 0;   // 웨이브 시간 반복: 남은 재발동 횟수 (seq 는 공유 = clear waves 로 함께 취소)
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
        String          stageId = {};
        Vector<SpawnId> ids     = {};
    };

    struct ExecContext
    {
        int  sectionIndex = -1;
        bool bSilent      = false;
        bool bAbort       = false;
    };

    [[nodiscard]] static int64_t ToTicks_(float _seconds);

    void FireSection_(int _index, bool _bSilent);
    bool WarpToSection_(int _index, float _targetX);
    void RunActions_(const Vector<StageAction>& _actions, const ExecContext& _ctx);
    void Execute_(const StageAction& _action, const ExecContext& _ctx);
    void ExecSpawn_(const SpawnSpec& _spec, const ExecContext& _ctx, bool _bWave);
    void ExecLock_(const StageAction& _action, const ExecContext& _ctx);
    void ExecUnlock_();
    void ExecClear_(const StageAction& _action);

    void               ProcessPending_();
    void               ProcessCurves_();
    void               ProcessGates_();
    void               CancelAbortables_();
    [[nodiscard]] bool IsGateSatisfied_(const GateState& _gate) const;

    void               ApplyCameraBounds_();
    void               ApplyCameraLock_();
    [[nodiscard]] Aabb CurrentViewRect_() const;

    SpawnRecord*       FindRecord_(StringView _stageId);
    const SpawnRecord* FindRecord_(StringView _stageId) const;

    const StageData* m_pData   = nullptr;
    IStageHost*      m_pHost   = nullptr;
    Camera*          m_pCamera = nullptr;
    RngMt19937*      m_pRng    = nullptr;

    int64_t m_tick        = 0;
    int     m_nextSection = 0;
    bool    m_bFinished   = false;

    Vector<PendingItem> m_pending       = {};
    uint64_t            m_nextSeq       = 1;
    uint64_t            m_waveCutoffSeq = 0;   // seq < cutoff 인 웨이브 엔트리는 취소됨
    Vector<GateState>   m_gates         = {};

    Vector<SpawnRecord> m_records      = {};
    int                 m_spawnedCount = 0;
    int                 m_skippedCount = 0;

    Aabb              m_base         = {};
    Vector<LockEntry> m_locks        = {};
    Vec2              m_cameraOffset = { 0.f, 0.f };
    Vector<uint8_t>   m_curveActive  = {};

    String m_groundName   = {};
    String m_backdropMode = {};
};

}   // namespace slug

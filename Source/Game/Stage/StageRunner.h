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
inline constexpr float kStageCameraTargetLift = 60.f;

struct ScreenWalls
{
    Aabb left         = {};
    Aabb right        = {};
    bool bHasLeft     = false;
    bool bHasRight    = false;
};

class StageRunner
{
public:
    StageRunner(const StageData& _data, IStageHost& _host, Camera& _camera, slug::RngMt19937& _rng);

    void Begin();

    [[nodiscard]] std::optional<Vec2> ResolvePlayerStart() const;

    void Tick();

    bool WarpToSection(int _index);
    bool WarpToSection(slug::StringView _id);
    bool WarpToBoss();

    [[nodiscard]] Vec2 ComputeCameraTarget(Vec2 _playerFeet) const;
    [[nodiscard]] ScreenWalls  MakeScreenWalls() const;

    [[nodiscard]] bool     IsFinished() const { return m_bFinished; }
    [[nodiscard]] int64_t  GetTick() const { return m_tick; }
    [[nodiscard]] int      GetNextSectionIndex() const { return m_nextSection; }
    [[nodiscard]] std::optional<float> GetNextTriggerX() const;
    [[nodiscard]] int      GetPendingActionCount() const;
    [[nodiscard]] int      GetPendingWaveCount() const;
    [[nodiscard]] int      GetOpenGateCount() const { return static_cast<int>(m_gates.size()); }
    [[nodiscard]] slug::Vector<slug::String> GetOpenGateIds() const;

    [[nodiscard]] const slug::String& GetGroundName() const { return m_groundName; }
    [[nodiscard]] const slug::String& GetBackdropMode() const { return m_backdropMode; }
    [[nodiscard]] Vec2       GetCameraOffset() const { return m_cameraOffset; }
    [[nodiscard]] const Aabb&        GetBaseBounds() const { return m_base; }
    [[nodiscard]] Aabb               GetEffectiveBounds() const;
    [[nodiscard]] int                GetLockDepth() const { return static_cast<int>(m_locks.size()); }
    [[nodiscard]] bool               HasPermanentLock() const;
    [[nodiscard]] bool               IsCurveActive(size_t _curveIndex) const;

    [[nodiscard]] slug::Span<const SpawnId> FindSpawnIds(slug::StringView _stageId) const;
    [[nodiscard]] bool                     WasSpawnSkipped(slug::StringView _stageId) const;

    [[nodiscard]] int GetSpawnedCount() const { return m_spawnedCount; }
    [[nodiscard]] int GetSkippedCount() const { return m_skippedCount; }

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
        slug::Vector<SpawnId>  ids     = {};
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

    Aabb                   m_base         = {};
    slug::Vector<LockEntry> m_locks        = {};
    Vec2           m_cameraOffset = { 0.f, 0.f };
    slug::Vector<uint8_t>   m_curveActive  = {};

    slug::String m_groundName   = {};
    slug::String m_backdropMode = {};
};
}

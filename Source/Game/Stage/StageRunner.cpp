#include "Game/Stage/StageRunner.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "Base/Assert.h"
#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "World/Constants.h"

constexpr double kTicksPerSec = 60.0;

constexpr float kWallDepth   = 64.f;
constexpr float kWallExtentY = 4000.f;

static void MixU64_(
    uint64_t&      _hash,
    const uint64_t _value)
{
    for (int i = 0; i < 8; ++i)
    {
        _hash ^= (_value >> (i * 8)) & 0xFFu;
        _hash *= 1099511628211ull;
    }
}

static void MixFloat_(
    uint64_t&   _hash,
    const float _value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &_value, sizeof(bits));
    MixU64_(_hash, bits);
}

StageRunner::StageRunner(
    const StageData& _data,
    IStageHost&      _host,
    Camera&          _camera,
    RngMt19937&      _rng)
    : m_pData(&_data)
    , m_pHost(&_host)
    , m_pCamera(&_camera)
    , m_pRng(&_rng)
{
}

int64_t StageRunner::ToTicks_(
    const float _seconds)
{
    return static_cast<int64_t>(std::llround(static_cast<double>(_seconds) * kTicksPerSec));
}

void StageRunner::Begin()
{
    m_tick        = 0;
    m_nextSection = 0;
    m_bFinished   = false;
    m_pending.clear();
    m_nextSeq       = 1;
    m_waveCutoffSeq = 0;
    m_gates.clear();
    m_records.clear();
    m_spawnedCount = 0;
    m_skippedCount = 0;
    m_locks.clear();

    const CameraConfig& config = m_pData->camera;
    m_base                     = config.initialBounds;
    m_cameraOffset             = config.initialOffset;
    m_curveActive.assign(config.boundsCurves.size(), 0);

    m_groundName   = m_pData->groundInitial;
    m_backdropMode = m_pData->backdropInitial;

    m_pCamera->CancelMove();
    m_pCamera->Unlock();
    m_pCamera->ClearScrollFloor();
    m_pCamera->SetForwardOnly(config.bForward);
    m_pCamera->SetForwardOnlyY(config.bForward);
    ApplyCameraBounds_();

    m_pHost->SetGround(m_groundName);
    if (!m_backdropMode.empty())
    {
        m_pHost->SetBackdrop(m_backdropMode);
    }
    if (!m_pData->bgmName.empty())
    {
        m_pHost->PlayBgm(m_pData->bgmName, m_pData->bBgmLoop);
    }
}

std::optional<Vec2> StageRunner::ResolvePlayerStart() const
{
    const PlayerStart& start = m_pData->player;
    float              y     = start.y;
    if (start.bGroundSnap)
    {
        const std::optional<float> ground = m_pHost->GetGroundFindY(start.x, start.fromY);
        if (!ground)
        {
            return std::nullopt;
        }
        y = *ground;
    }
    else if (start.origin == eOrigin::LegacyCenter)
    {
        y += start.halfHeight;
    }
    return Vec2 { start.x, y };
}

Aabb StageRunner::CurrentViewRect_() const
{
    return Aabb::FromMinSize(m_pCamera->GetPositionNoShake(), m_pCamera->GetTuning().viewSize);
}

const String& StageRunner::GetGroundName() const
{
    return m_groundName;
}

const String& StageRunner::GetBackdropMode() const
{
    return m_backdropMode;
}

Vec2 StageRunner::GetCameraOffset() const
{
    return m_cameraOffset;
}

const Aabb& StageRunner::GetBaseBounds() const
{
    return m_base;
}

Aabb StageRunner::GetEffectiveBounds() const
{
    return m_locks.empty() ? m_base : m_locks.back().rect;
}

int StageRunner::GetLockDepth() const
{
    return static_cast<int>(m_locks.size());
}

bool StageRunner::HasPermanentLock() const
{
    for (const LockEntry& lock: m_locks)
    {
        if (lock.bPermanent)
        {
            return true;
        }
    }
    return false;
}

bool StageRunner::IsCurveActive(
    const size_t _curveIndex) const
{
    return _curveIndex < m_curveActive.size() && m_curveActive[_curveIndex] != 0;
}

void StageRunner::ApplyCameraBounds_()
{
    m_pCamera->SetBounds(m_base);
}

void StageRunner::ApplyCameraLock_()
{
    if (m_locks.empty())
    {
        m_pCamera->Unlock();
    }
    else
    {
        m_pCamera->Lock(m_locks.back().rect);
    }
}

Vec2 StageRunner::ComputeCameraTarget(
    const Vec2 _playerFeet) const
{
    return { _playerFeet.x + m_cameraOffset.x, _playerFeet.y - kStageCameraTargetLift + m_cameraOffset.y };
}

ScreenWalls StageRunner::MakeScreenWalls() const
{
    ScreenWalls walls  = {};
    const float margin = m_pData->camera.playerMargin;
    const Aabb  view   = CurrentViewRect_();

    if (m_pData->camera.bForward)
    {
        walls.bHasLeft = true;
        walls.left     = Aabb::FromMinMax({ view.min.x - kWallDepth, -kWallExtentY }, { view.min.x + margin, kWallExtentY });
    }

    const Aabb bounds        = GetEffectiveBounds();
    const bool bAtRightLimit = view.max.x >= bounds.max.x - 0.5f;
    if (!m_locks.empty() || m_pCamera->IsMoving() || m_pCamera->HasPendingLock() || bAtRightLimit)
    {
        walls.bHasRight = true;
        walls.right     = Aabb::FromMinMax({ view.max.x - margin, -kWallExtentY }, { view.max.x + kWallDepth, kWallExtentY });
    }
    return walls;
}

void StageRunner::Tick()
{
    if (m_bFinished)
    {
        return;
    }

    CancelAbortables_();

    const Vec2 feet = m_pHost->PlayerFeet();
    while (m_nextSection < static_cast<int>(m_pData->sections.size()) && feet.x > m_pData->sections[static_cast<size_t>(m_nextSection)].triggerX && !m_bFinished)
    {
        const int index = m_nextSection++;
        FireSection_(index, false);
    }

    ProcessPending_();

    ProcessCurves_();

    ProcessGates_();

    m_pCamera->SetTarget(ComputeCameraTarget(m_pHost->PlayerFeet()));
    m_pCamera->Update(kStepSec);

    ++m_tick;
}

void StageRunner::AddCameraScroll(
    const float _dx)
{
    if (_dx == 0.f)
    {
        return;
    }
    if (!m_locks.empty())
    {
        // 잠금 중: 잠금 사각형을 카메라와 함께 옮긴다 (바닥만 올리면 잠금 오른쪽 끝에 막힌다). 플레이어 화면 가두기도 이 사각형을 따른다
        m_locks.back().rect.min.x += _dx;
        m_locks.back().rect.max.x += _dx;
        m_pCamera->ShiftLock(_dx);
        return;
    }
    m_pCamera->AddScrollFloor(_dx);
}

void StageRunner::CancelAbortables_()
{
    if (!m_pHost->IsGameOver())
    {
        return;
    }
    std::erase_if(m_gates, [](const GateState& _gate) { return _gate.bAbortOnGameOver; });
    std::erase_if(m_pending, [](const PendingItem& _item) { return _item.bAbortOnGameOver; });
}

void StageRunner::ProcessPending_()
{
    if (m_pending.empty())
    {
        return;
    }

    Vector<PendingItem> ready = {};
    for (size_t i = 0; i < m_pending.size();)
    {
        if (m_pending[i].dueTick <= m_tick)
        {
            ready.push_back(m_pending[i]);
            m_pending.erase(m_pending.begin() + static_cast<std::ptrdiff_t>(i));
        }
        else
        {
            ++i;
        }
    }
    std::ranges::stable_sort(ready, [](const PendingItem& _a, const PendingItem& _b) {
        return _a.dueTick != _b.dueTick ? _a.dueTick < _b.dueTick : _a.seq < _b.seq;
    });

    for (const PendingItem& item: ready)
    {
        if (m_bFinished)
        {
            break;
        }
        ExecContext ctx  = {};
        ctx.sectionIndex = item.sectionIndex;
        ctx.bSilent      = false;
        ctx.bAbort       = item.bAbortOnGameOver;

        if (item.pWave)
        {
            if (item.seq < m_waveCutoffSeq)
            {
                continue;
            }
            ExecSpawn_(item.pWave->spawn, ctx, true);
            if (item.repeatsLeft > 0 && !m_bFinished)
            {
                PendingItem next = item;
                next.dueTick     = item.dueTick + ToTicks_(item.pWave->every);
                next.repeatsLeft = item.repeatsLeft - 1;
                m_pending.push_back(next);
            }
        }
        else if (item.pAction)
        {
            if (item.bAbortOnGameOver && m_pHost->IsGameOver())
            {
                continue;
            }
            Execute_(*item.pAction, ctx);
        }
    }
}

void StageRunner::ProcessCurves_()
{
    if (HasPermanentLock())
    {
        return;
    }
    const float playerX  = m_pHost->PlayerFeet().x;
    bool        bChanged = false;
    for (size_t i = 0; i < m_pData->camera.boundsCurves.size(); ++i)
    {
        if (m_curveActive[i] == 0)
        {
            continue;
        }
        m_base   = m_pData->camera.boundsCurves[i].EvaluateBounds(playerX);
        bChanged = true;
    }
    if (bChanged)
    {
        ApplyCameraBounds_();
    }
}

bool StageRunner::IsGateSatisfied_(
    const GateState& _gate) const
{
    const StageAction& action = *_gate.pAction;
    for (const String& stageId: action.waitFor)
    {
        const SpawnRecord* pRecord = FindRecord_(stageId);
        if (!pRecord)
        {
            return false;
        }
        for (const SpawnId id: pRecord->ids)
        {
            const bool bDone = action.until == eGateUntil::DestroyFinished ? m_pHost->IsDestroyFinished(id) : m_pHost->IsDefeated(id);
            if (!bDone)
            {
                return false;
            }
        }
    }
    return true;
}

void StageRunner::ProcessGates_()
{
    for (size_t i = 0; i < m_gates.size() && !m_bFinished;)
    {
        if (!IsGateSatisfied_(m_gates[i]))
        {
            ++i;
            continue;
        }

        const GateState gate = m_gates[i];
        m_gates.erase(m_gates.begin() + static_cast<std::ptrdiff_t>(i));

        ExecContext ctx  = {};
        ctx.sectionIndex = gate.sectionIndex;
        ctx.bSilent      = false;
        ctx.bAbort       = gate.bAbortOnGameOver;
        RunActions_(gate.pAction->actions, ctx);
    }
}

void StageRunner::FireSection_(
    const int  _index,
    const bool _bSilent)
{
    const StageSection& section = m_pData->sections[static_cast<size_t>(_index)];

    for (size_t i = 0; i < m_pData->camera.boundsCurves.size(); ++i)
    {
        const BoundsCurve& curve = m_pData->camera.boundsCurves[i];
        if (curve.fromIndex == _index)
        {
            m_curveActive[i] = 1;
        }
        if (curve.untilIndex == _index)
        {
            m_curveActive[i] = 0;   // until 섹션의 액션이 실행되기 직전에 곡선을 끈다
        }
    }

    if (!_bSilent)
    {
        m_pHost->OnSectionEntered(_index, section.id);
    }

    ExecContext ctx  = {};
    ctx.sectionIndex = _index;
    ctx.bSilent      = _bSilent;
    RunActions_(section.actions, ctx);
}

void StageRunner::RunActions_(
    const Vector<StageAction>& _actions,
    const ExecContext&         _ctx)
{
    for (const StageAction& action: _actions)
    {
        if (m_bFinished)
        {
            return;
        }
        if (!ShouldRunAction(action.when, _ctx.bSilent))
        {
            continue;
        }

        const int64_t delayTicks = _ctx.bSilent ? 0 : ToTicks_(action.delay);
        if (delayTicks > 0)
        {
            PendingItem item      = {};
            item.dueTick          = m_tick + delayTicks;
            item.seq              = m_nextSeq++;
            item.pAction          = &action;
            item.sectionIndex     = _ctx.sectionIndex;
            item.bAbortOnGameOver = _ctx.bAbort;
            m_pending.push_back(item);
            continue;
        }
        Execute_(action, _ctx);
    }
}

void StageRunner::Execute_(
    const StageAction& _action,
    const ExecContext& _ctx)
{
    switch (_action.type)
    {
        case eActionType::Spawn:
            ExecSpawn_(_action.spawn, _ctx, false);
            break;

        case eActionType::SpawnWave:
            for (const WaveEntry& entry: _action.waveEntries)
            {
                const int64_t delayTicks = ToTicks_(entry.delay);
                if (delayTicks > 0 || entry.times > 1)
                {
                    PendingItem item      = {};
                    item.dueTick          = m_tick + delayTicks;
                    item.seq              = m_nextSeq++;
                    item.pWave            = &entry;
                    item.sectionIndex     = _ctx.sectionIndex;
                    item.bAbortOnGameOver = _ctx.bAbort;
                    item.repeatsLeft      = entry.times - 1;
                    m_pending.push_back(item);
                }
                else
                {
                    ExecSpawn_(entry.spawn, _ctx, true);
                }
            }
            break;

        case eActionType::SetBounds:
            if (!HasPermanentLock())
            {
                m_base = _action.rect.rect;
                ApplyCameraBounds_();
            }
            break;

        case eActionType::LockCamera:
            ExecLock_(_action, _ctx);
            break;

        case eActionType::UnlockCamera:
            ExecUnlock_();
            break;

        case eActionType::CameraMove:
            m_pCamera->BeginMove(_action.to, _ctx.bSilent ? 0.f : _action.seconds);
            break;

        case eActionType::SetCameraOffset:
            m_cameraOffset = _action.offset;
            break;

        case eActionType::SetBackdrop:
            m_backdropMode = _action.mode;
            m_pHost->SetBackdrop(_action.mode);
            break;

        case eActionType::SetGround:
            m_groundName = _action.name;
            m_pHost->SetGround(_action.name);
            break;

        case eActionType::Gate:
        {
            GateState gate        = {};
            gate.pAction          = &_action;
            gate.sectionIndex     = _ctx.sectionIndex;
            gate.bAbortOnGameOver = _action.bAbortOnGameOver || _ctx.bAbort;
            m_gates.push_back(gate);
            break;
        }

        case eActionType::Command:
            if (const SpawnRecord* pRecord = FindRecord_(_action.target))
            {
                for (const SpawnId id: pRecord->ids)
                {
                    m_pHost->SendCommand(id, _action.command);
                }
            }
            break;

        case eActionType::Clear:
            ExecClear_(_action);
            break;

        case eActionType::Banner:
            m_pHost->ShowBanner(_action.text, _action.seconds);
            if (!_action.name.empty())
            {
                m_pHost->PlaySfx(_action.name);
            }
            break;

        case eActionType::Fade:
            m_pHost->FadeTo(_action.fadeTo, _action.seconds);
            break;

        case eActionType::PlayBgm:
            if (_action.fadeOutSec > 0.f)
            {
                m_pHost->ChangeBgm(_action.name, _action.bLoop, _action.fadeOutSec);
            }
            else
            {
                m_pHost->PlayBgm(_action.name, _action.bLoop);
            }
            break;

        case eActionType::PlayerInvincible:
            m_pHost->SetPlayerInvincible(_action.seconds);
            break;

        case eActionType::EndStage:
            m_bFinished = true;
            m_pHost->EndStage(_action.next);
            break;
    }
}

void StageRunner::ExecLock_(
    const StageAction& _action,
    const ExecContext& _ctx)
{
    LockEntry entry  = {};
    entry.rect       = _action.rect.bCurrent ? CurrentViewRect_() : _action.rect.rect;
    entry.bPermanent = _action.bPermanent;
    m_locks.push_back(entry);

    if (_action.glide > 0.f && !_ctx.bSilent && !_action.rect.bCurrent)
    {
        m_pCamera->LockWithGlide(entry.rect, _action.glide);
    }
    else
    {
        m_pCamera->Lock(entry.rect);
    }
}

void StageRunner::ExecUnlock_()
{
    for (size_t i = m_locks.size(); i > 0; --i)
    {
        if (!m_locks[i - 1].bPermanent)
        {
            m_locks.erase(m_locks.begin() + static_cast<std::ptrdiff_t>(i - 1));
            m_pCamera->Unlock();
            ApplyCameraLock_();
            return;
        }
    }
}

void StageRunner::ExecClear_(
    const StageAction& _action)
{
    if (_action.bClearWaves)
    {
        m_waveCutoffSeq = m_nextSeq;
        std::erase_if(m_pending, [](const PendingItem& _item) { return _item.pWave != nullptr; });
        m_gates.clear();
    }
    if (_action.clearLayers != 0)
    {
        EntityFilter filter = {};
        filter.layerMask    = _action.clearLayers;
        m_pHost->ClearEntities(filter);
    }
}

StageRunner::SpawnRecord* StageRunner::FindRecord_(
    const StringView _stageId)
{
    for (SpawnRecord& record: m_records)
    {
        if (record.stageId == _stageId)
        {
            return &record;
        }
    }
    return nullptr;
}

const StageRunner::SpawnRecord* StageRunner::FindRecord_(
    const StringView _stageId) const
{
    for (const SpawnRecord& record: m_records)
    {
        if (record.stageId == _stageId)
        {
            return &record;
        }
    }
    return nullptr;
}

Span<const SpawnId> StageRunner::FindSpawnIds(
    const StringView _stageId) const
{
    const SpawnRecord* pRecord = FindRecord_(_stageId);
    return pRecord ? Span<const SpawnId> { pRecord->ids } : Span<const SpawnId> {};
}

bool StageRunner::WasSpawnSkipped(
    const StringView _stageId) const
{
    const SpawnRecord* pRecord = FindRecord_(_stageId);
    return pRecord && pRecord->ids.empty();
}

void StageRunner::ExecSpawn_(
    const SpawnSpec&   _spec,
    const ExecContext& _ctx,
    const bool         _bWave)
{
    const int count = _spec.GetCount();
    for (int i = 0; i < count; ++i)
    {
        SpawnRequest request   = {};
        request.kind           = _spec.kind;
        request.def            = _spec.def;
        request.stageId        = _spec.id;
        request.facing         = _spec.facing;
        request.bHasZ          = _spec.bHasZ;
        request.z              = _spec.z;
        request.bHasHp         = _spec.bHasHp;
        request.hp             = _spec.hp;
        request.drop           = _spec.drop;
        request.bSettle        = _spec.bSettle;
        request.bFromCamera    = _spec.bHasCamera;
        request.bWave          = _bWave;
        request.bSilent        = _ctx.bSilent;
        request.repeatIndex    = i;
        request.sectionIndex   = _ctx.sectionIndex;
        request.bHasLayer      = _spec.bHasLayer;
        request.layer          = _spec.layer;
        request.cullMargin     = _spec.cullMargin;
        request.bHasAmbush     = _spec.bHasAmbush;
        request.ambush         = _spec.ambush;
        request.size           = _spec.size;
        request.collisionLayer = _spec.collisionLayer;
        request.anim           = _spec.anim;
        request.sprite         = _spec.sprite;
        request.pParams        = &_spec.params;

        Vec2 position = { 0.f, 0.f };
        bool bSkip    = false;
        if (_spec.bHasCamera)
        {
            const Aabb          view = CurrentViewRect_();
            const CameraAnchor& a    = _spec.camera;
            int                 dx   = a.dxMin;
            if (a.IsRandomX())
            {
                dx = Random<int>(*m_pRng, a.dxMin, a.dxMax);
            }
            const float baseX = a.ax == eAnchorX::Left ? view.min.x : (a.ax == eAnchorX::Right ? view.max.x : (view.min.x + view.max.x) * 0.5f);
            const float baseY = a.ay == eAnchorY::Top ? view.min.y : (a.ay == eAnchorY::Bottom ? view.max.y : (view.min.y + view.max.y) * 0.5f);
            position          = { baseX + static_cast<float>(dx), baseY + a.dy };
        }
        else
        {
            position.x = _spec.x;
            position.y = _spec.y;
            if (_spec.bHasRepeat)
            {
                position.x += static_cast<float>(i) * _spec.repeat.stepX;
                position.y += static_cast<float>(i) * _spec.repeat.stepY;
            }
            if (_spec.bGroundSnap)
            {
                const std::optional<float> ground = m_pHost->GetGroundFindY(position.x, _spec.fromY);
                if (ground)
                {
                    position.y = *ground;
                }
                else
                {
                    bSkip = true;
                }
            }
        }
        if (_spec.bHasCamera && _spec.bHasRepeat)
        {
            position.x += static_cast<float>(i) * _spec.repeat.stepX;
            position.y += static_cast<float>(i) * _spec.repeat.stepY;
        }

        if (_spec.origin == eOrigin::LegacyCenter && (_spec.kind == eEntityKind::Enemy || _spec.kind == eEntityKind::Npc))
        {
            request.halfHeight   = _spec.halfHeight;
            request.position     = { position.x, position.y + _spec.halfHeight };
            request.positionMode = ePositionMode::Feet;
        }
        else
        {
            request.halfHeight   = _spec.bHasHalfHeight ? _spec.halfHeight : 0.f;
            request.position     = position;
            request.positionMode = _spec.origin == eOrigin::Feet ? ePositionMode::Feet : ePositionMode::LegacyCenter;
        }

        SpawnId id = kInvalidSpawnId;
        if (!bSkip)
        {
            id = m_pHost->SpawnEntity(request);
        }
        if (id == kInvalidSpawnId)
        {
            ++m_skippedCount;
        }
        else
        {
            ++m_spawnedCount;
        }

        if (!_spec.id.empty())
        {
            SpawnRecord* pRecord = FindRecord_(_spec.id);
            if (!pRecord)
            {
                m_records.push_back({ _spec.id, {} });
                pRecord = &m_records.back();
            }
            if (id != kInvalidSpawnId)
            {
                pRecord->ids.push_back(id);
            }
        }
    }
}

bool StageRunner::WarpToSection(
    const StringView _id)
{
    const int index = m_pData->FindSection(_id);
    return index >= 0 && WarpToSection(index);
}

bool StageRunner::WarpToBoss()
{
    int index = m_pData->FindSection("boss");
    if (index < 0)
    {
        index = static_cast<int>(m_pData->sections.size()) - 1;
    }
    return index >= 0 && WarpToSection(index);
}

bool StageRunner::WarpToX(
    const float _x)
{
    int index = m_nextSection;
    while (index < static_cast<int>(m_pData->sections.size()) && m_pData->sections[static_cast<size_t>(index)].triggerX < _x)
    {
        ++index;
    }
    if (index >= static_cast<int>(m_pData->sections.size()))
    {
        index = static_cast<int>(m_pData->sections.size()) - 1;
    }
    return WarpToSection_(index, _x);
}

bool StageRunner::WarpToSection(
    const int _index)
{
    if (_index < 0 || _index >= static_cast<int>(m_pData->sections.size()))
    {
        return false;
    }
    return WarpToSection_(_index, m_pData->sections[static_cast<size_t>(_index)].triggerX + 1.f);
}

bool StageRunner::WarpToSection_(
    const int   _index,
    const float _targetX)
{
    if (_index < m_nextSection || _index >= static_cast<int>(m_pData->sections.size()) || m_bFinished)
    {
        return false;
    }

    m_pending.clear();
    m_gates.clear();
    m_waveCutoffSeq = m_nextSeq;
    std::erase_if(m_locks, [](const LockEntry& _lock) { return !_lock.bPermanent; });
    m_pCamera->CancelMove();
    m_pCamera->Unlock();
    ApplyCameraLock_();

    while (m_nextSection < _index)
    {
        FireSection_(m_nextSection, true);
        ++m_nextSection;
    }

    const float targetX = _targetX;
    const Aabb  bounds  = GetEffectiveBounds();
    Vec2        feet    = m_pHost->PlayerFeet();
    feet.x              = targetX;
    if (const std::optional<float> ground = m_pHost->GetGroundFindY(targetX, bounds.min.y))
    {
        feet.y = *ground;
    }
    m_pHost->WarpPlayer(feet);

    ProcessCurves_();
    m_pCamera->SetTarget(ComputeCameraTarget(feet));
    m_pCamera->SnapToTarget();
    return true;
}

bool StageRunner::IsFinished() const
{
    return m_bFinished;
}

int64_t StageRunner::GetTick() const
{
    return m_tick;
}

int StageRunner::GetNextSectionIndex() const
{
    return m_nextSection;
}

std::optional<float> StageRunner::GetNextTriggerX() const
{
    if (m_nextSection >= static_cast<int>(m_pData->sections.size()))
    {
        return std::nullopt;
    }
    return m_pData->sections[static_cast<size_t>(m_nextSection)].triggerX;
}

int StageRunner::GetPendingActionCount() const
{
    int count = 0;
    for (const PendingItem& item: m_pending)
    {
        count += item.pAction ? 1 : 0;
    }
    return count;
}

int StageRunner::GetPendingWaveCount() const
{
    int count = 0;
    for (const PendingItem& item: m_pending)
    {
        count += item.pWave ? 1 : 0;
    }
    return count;
}

int StageRunner::GetOpenGateCount() const
{
    return static_cast<int>(m_gates.size());
}

Vector<String> StageRunner::GetOpenGateIds() const
{
    Vector<String> ids = {};
    for (const GateState& gate: m_gates)
    {
        ids.push_back(gate.pAction->gateId);
    }
    return ids;
}

int StageRunner::GetSpawnedCount() const
{
    return m_spawnedCount;
}

int StageRunner::GetSkippedCount() const
{
    return m_skippedCount;
}

uint64_t StageRunner::HashState() const
{
    uint64_t hash = 14695981039346656037ull;
    MixU64_(hash, static_cast<uint64_t>(m_tick));
    MixU64_(hash, static_cast<uint64_t>(m_nextSection));
    MixU64_(hash, m_bFinished ? 1u : 0u);
    for (const PendingItem& item: m_pending)
    {
        MixU64_(hash, static_cast<uint64_t>(item.dueTick));
        MixU64_(hash, item.seq);
    }
    MixU64_(hash, m_gates.size());
    for (const LockEntry& lock: m_locks)
    {
        MixFloat_(hash, lock.rect.min.x);
        MixFloat_(hash, lock.rect.min.y);
        MixFloat_(hash, lock.rect.max.x);
        MixFloat_(hash, lock.rect.max.y);
    }
    MixFloat_(hash, m_base.min.x);
    MixFloat_(hash, m_base.min.y);
    MixFloat_(hash, m_base.max.x);
    MixFloat_(hash, m_base.max.y);
    MixFloat_(hash, m_cameraOffset.x);
    MixFloat_(hash, m_cameraOffset.y);
    for (const SpawnRecord& record: m_records)
    {
        for (const SpawnId id: record.ids)
        {
            MixU64_(hash, id);
        }
    }
    MixU64_(hash, static_cast<uint64_t>(m_spawnedCount));
    MixU64_(hash, static_cast<uint64_t>(m_skippedCount));
    for (const char c: m_groundName)
    {
        MixU64_(hash, static_cast<uint8_t>(c));
    }
    for (const char c: m_backdropMode)
    {
        MixU64_(hash, static_cast<uint8_t>(c));
    }
    const Vec2 center = m_pCamera->GetViewCenter();
    MixFloat_(hash, center.x);
    MixFloat_(hash, center.y);
    return hash;
}

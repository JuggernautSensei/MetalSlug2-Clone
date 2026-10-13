#pragma once
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "Game/Stage/StageRunner.h"
#include "Render/Camera.h"

struct SpawnLogEntry
{
    int64_t      tick = 0;
    SpawnId      id   = kInvalidSpawnId;
    SpawnRequest request;
};

struct HostEvent
{
    int64_t tick = 0;
    String  text;
};

[[nodiscard]] std::optional<float> DefaultGround_(
    const float,
    float _fromY);

using GroundQueryFn = std::optional<float> (*)(float, float);

class FakeStageHost final : public IStageHost
{
public:
    int64_t       now           = 0;
    VECTOR2       playerFeet    = { 0.f, 378.f };
    bool          bGameOver     = false;
    GroundQueryFn groundFn      = DefaultGround_;
    bool          bAutoDefeat   = false;
    SpawnId       nextId        = 1;
    bool          bRefuseSpawns = false;

    Vector<SpawnLogEntry>              spawns;
    Vector<HostEvent>                  events;
    RbSet<SpawnId>                     defeated;
    RbSet<SpawnId>                     destroyed;
    bool                               bEnded = false;
    String                             endNext;
    String                             currentGround;
    String                             currentBackdrop;
    Vector<VECTOR2>                    warps;
    Vector<std::pair<SpawnId, String>> commands;
    Vector<uint32_t>                   clears;
    float                              invincibleSeconds = 0.f;

    mutable Vector<std::pair<float, float>> groundQueries;

    SpawnId SpawnEntity(
        const SpawnRequest& _request) override;

    bool IsDefeated(
        SpawnId _id) const override;

    bool IsDestroyFinished(
        SpawnId _id) const override;

    void ClearEntities(
        const EntityFilter& _filter) override;

    void SendCommand(
        SpawnId    _id,
        StringView _command) override;

    void SetGround(
        StringView _name) override;

    std::optional<float> GetGroundFindY(
        float _x,
        float _fromY) const override;

    VECTOR2 PlayerFeet() const override;
    bool    IsGameOver() const override;

    void SetPlayerInvincible(
        float _seconds) override;

    void WarpPlayer(
        VECTOR2 _feet) override;

    void SetBackdrop(
        StringView _modeId) override;

    void ShowBanner(
        const String& _text,
        float         _seconds) override;

    void FadeTo(
        eFadeTarget _target,
        float       _seconds) override;

    void PlayBgm(
        StringView _name,
        bool       _bLoop) override;

    void ChangeBgm(
        StringView _name,
        bool       _bLoop,
        float      _fadeOutSec) override;

    void PlaySfx(
        StringView _name) override;

    void EndStage(
        StringView _nextState) override;

    void OnSectionEntered(
        int        _index,
        StringView _id) override;

    [[nodiscard]] int CountEvents(
        const String& _prefix) const;

    [[nodiscard]] const HostEvent* FindEvent(
        const String& _prefix,
        int           _nth = 0) const;

    [[nodiscard]] int CountSpawnsOf(
        const String& _def) const;

    void DefeatAll();

private:
    void Log_(
        String _text);
};

[[nodiscard]] String WrapStage(
    const String& _sections,
    const String& _camera = R"J({"initialBounds":[0,0,4000,448]})J");

[[nodiscard]] StageData MustLoad(
    const String& _json);

[[nodiscard]] String LoadError(
    const String& _json);

class RunnerFixture
{
public:
    explicit RunnerFixture(
        const String& _json);

    void Step(
        int _count = 1);

    void StepAt(
        float _x,
        int   _count = 1);

    StageData     data;
    FakeStageHost host;
    Camera        camera;
    RngMt19937    rng = RngMt19937 { std::mt19937_64::default_seed };
    StageRunner   runner;
};

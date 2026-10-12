// 스테이지 테스트 공용 도우미: IStageHost 기록용 가짜 구현체, 스테이지 JSON 래퍼, StageRunner 픽스처.
#pragma once
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Result.h"
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
    int64_t     tick = 0;
    std::string text;
};

// 기본 바닥 질의: y=378 에 평평한 바닥이 있고, 그보다 아래에서 시작하면 바닥이 없다.
[[nodiscard]] std::optional<float> DefaultGround_(
    const float,
    const float _fromY);

// 바닥 질의 함수: (x, fromY) -> 바닥 y. 바닥이 없으면 nullopt
using GroundQueryFn = std::optional<float> (*)(float, float);

class FakeStageHost final : public IStageHost
{
public:
    int64_t       now           = 0;
    Vec2          playerFeet    = { 0.f, 378.f };
    bool          bPlayerAlive  = true;
    bool          bGameOver     = false;
    GroundQueryFn groundFn      = DefaultGround_;
    bool          bAutoDefeat   = false;
    SpawnId       nextId        = 1;
    bool          bRefuseSpawns = false;

    std::vector<SpawnLogEntry>                   spawns;
    std::vector<HostEvent>                       events;
    std::set<SpawnId>                            defeated;
    std::set<SpawnId>                            destroyed;
    bool                                         bEnded = false;
    std::string                                  endNext;
    std::string                                  currentGround;
    std::string                                  currentBackdrop;
    std::vector<Vec2>                            warps;
    std::vector<std::pair<SpawnId, std::string>> commands;
    std::vector<uint32_t>                        clears;
    float                                        invincibleSeconds = 0.f;

    // GetGroundFindY 가 받은 (x, fromY) 질의를 순서대로 기록한다 (const 질의라 mutable)
    mutable std::vector<std::pair<float, float>> groundQueries;

    SpawnId SpawnEntity(
        const SpawnRequest& _request) override;

    bool IsDefeated(
        const SpawnId _id) const override;

    bool IsDestroyFinished(
        const SpawnId _id) const override;

    void ClearEntities(
        const EntityFilter& _filter) override;

    void SendCommand(
        const SpawnId          _id,
        const StringView _command) override;

    void SetGround(
        const StringView _name) override;

    std::optional<float> GetGroundFindY(
        const float _x,
        const float _fromY) const override;

    Vec2 PlayerFeet() const override;
    bool PlayerAlive() const override;
    bool IsGameOver() const override;

    void SetPlayerInvincible(
        const float _seconds) override;

    void WarpPlayer(
        const Vec2 _feet) override;

    void SetBackdrop(
        const StringView _modeId) override;

    void ShowBanner(
        const String& _text,
        const float         _seconds) override;

    void FadeTo(
        const eFadeTarget _target,
        const float       _seconds) override;

    void PlayBgm(
        const StringView _name,
        const bool             _bLoop) override;

    void ChangeBgm(
        const StringView _name,
        const bool       _bLoop,
        const float      _fadeOutSec) override;

    void PlaySfx(
        const StringView _name) override;

    void EndStage(
        const StringView _nextState) override;

    void OnSectionEntered(
        const int              _index,
        const StringView _id) override;

    [[nodiscard]] int CountEvents(
        const std::string& _prefix) const;

    [[nodiscard]] const HostEvent* FindEvent(
        const std::string& _prefix,
        int                _nth = 0) const;

    [[nodiscard]] int CountSpawnsOf(
        const std::string& _def) const;

    void DefeatAll();

private:
    void Log_(
        std::string _text);
};

[[nodiscard]] std::string WrapStage(
    const std::string& _sections,
    const std::string& _camera = R"J({"initialBounds":[0,0,4000,448]})J");

[[nodiscard]] StageData MustLoad(
    const std::string& _json);

[[nodiscard]] std::string LoadError(
    const std::string& _json);

class RunnerFixture
{
public:
    explicit RunnerFixture(
        const std::string& _json);

    void Step(
        const int _count = 1);

    void StepAt(
        const float _x,
        const int   _count = 1);

    StageData        data;
    FakeStageHost    host;
    Camera           camera;
    RngMt19937 rng = RngMt19937 { std::mt19937_64::default_seed };
    StageRunner      runner;
};

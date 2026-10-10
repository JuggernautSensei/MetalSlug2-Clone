// 스테이지 테스트 공용 도우미: IStageHost 기록용 가짜 구현체, 스테이지 JSON 래퍼, StageRunner 픽스처.
#pragma once
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "Game/Stage/StageRunner.h"
#include "Render/Camera.h"

namespace slugtest
{

using namespace slug;

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

namespace stage_test_detail
{

    // 기본 바닥 질의: y=378 에 평평한 바닥이 있고, 그보다 아래에서 시작하면 바닥이 없다.
    [[nodiscard]] inline std::optional<float> DefaultGround_(
        const float,
        const float _fromY)
    {
        if (_fromY <= 378.f)
        {
            return 378.f;
        }
        return std::nullopt;
    }

}   // namespace stage_test_detail

// 바닥 질의 함수: (x, fromY) -> 바닥 y. 바닥이 없으면 nullopt
using GroundQueryFn = std::optional<float> (*)(float, float);

class FakeStageHost final : public IStageHost
{
public:
    int64_t       now           = 0;
    Vec2          playerFeet    = { 0.f, 378.f };
    bool          bPlayerAlive  = true;
    bool          bGameOver     = false;
    GroundQueryFn groundFn      = stage_test_detail::DefaultGround_;
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
        const SpawnRequest& _request) override
    {
        if (bRefuseSpawns)
        {
            return kInvalidSpawnId;
        }
        const SpawnId id = nextId++;
        spawns.push_back({ now, id, _request });
        if (bAutoDefeat)
        {
            defeated.insert(id);
            destroyed.insert(id);
        }
        return id;
    }

    bool IsDefeated(
        const SpawnId _id) const override
    {
        return defeated.contains(_id);
    }

    bool IsDestroyFinished(
        const SpawnId _id) const override
    {
        return destroyed.contains(_id);
    }

    void ClearEntities(
        const EntityFilter& _filter) override
    {
        clears.push_back(_filter.layerMask);
        Log_("clear:" + std::to_string(_filter.layerMask));
    }

    void SendCommand(
        const SpawnId          _id,
        const slug::StringView _command) override
    {
        commands.emplace_back(_id, std::string { _command });
        Log_("cmd:" + std::to_string(_id) + ":" + std::string { _command });
    }

    void SetGround(
        const slug::StringView _name) override
    {
        currentGround = std::string { _name };
        Log_("ground:" + currentGround);
    }

    std::optional<float> GetGroundFindY(
        const float _x,
        const float _fromY) const override
    {
        groundQueries.emplace_back(_x, _fromY);
        return groundFn(_x, _fromY);
    }

    Vec2 PlayerFeet() const override { return playerFeet; }
    bool PlayerAlive() const override { return bPlayerAlive; }
    bool IsGameOver() const override { return bGameOver; }

    void SetPlayerInvincible(
        const float _seconds) override
    {
        invincibleSeconds = _seconds;
        Log_("invincible:" + std::to_string(_seconds));
    }

    void WarpPlayer(
        const Vec2 _feet) override
    {
        playerFeet = _feet;
        warps.push_back(_feet);
        Log_("warp");
    }

    void SetBackdrop(
        const slug::StringView _modeId) override
    {
        currentBackdrop = std::string { _modeId };
        Log_("backdrop:" + currentBackdrop);
    }

    void ShowBanner(
        const slug::String& _text,
        const float         _seconds) override
    {
        Log_("banner:" + _text + ":" + std::to_string(_seconds));
    }

    void FadeTo(
        const eFadeTarget _target,
        const float       _seconds) override
    {
        Log_(std::string { "fade:" } + (_target == eFadeTarget::Black ? "black" : "clear") + ":" + std::to_string(_seconds));
    }

    void PlayBgm(
        const slug::StringView _name,
        const bool             _bLoop) override
    {
        Log_("bgm:" + std::string { _name } + (_bLoop ? ":loop" : ":once"));
    }

    void EndStage(
        const slug::StringView _nextState) override
    {
        bEnded  = true;
        endNext = std::string { _nextState };
        Log_("end:" + endNext);
    }

    void OnSectionEntered(
        const int              _index,
        const slug::StringView _id) override
    {
        Log_("section:" + std::to_string(_index) + ":" + std::string { _id });
    }

    [[nodiscard]] int CountEvents(
        const std::string& _prefix) const
    {
        int count = 0;
        for (const HostEvent& event: events)
        {
            count += event.text.rfind(_prefix, 0) == 0 ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] const HostEvent* FindEvent(
        const std::string& _prefix,
        int                _nth = 0) const
    {
        for (const HostEvent& event: events)
        {
            if (event.text.rfind(_prefix, 0) == 0 && _nth-- == 0)
            {
                return &event;
            }
        }
        return nullptr;
    }

    [[nodiscard]] int CountSpawnsOf(
        const std::string& _def) const
    {
        int count = 0;
        for (const SpawnLogEntry& entry: spawns)
        {
            count += entry.request.def == _def ? 1 : 0;
        }
        return count;
    }

    void DefeatAll()
    {
        for (const SpawnLogEntry& entry: spawns)
        {
            defeated.insert(entry.id);
            destroyed.insert(entry.id);
        }
    }

private:
    void Log_(
        std::string _text)
    {
        events.push_back({ now, std::move(_text) });
    }
};

[[nodiscard]] inline std::string WrapStage(
    const std::string& _sections,
    const std::string& _camera = R"J({"initialBounds":[0,0,4000,448]})J")
{
    return std::string { R"J({"version":1,"name":"T","world":{"size":[4000,1000],"background":"bg"},
"ground":{"initial":"G0","maps":["G0","G1"]},
"backdrop":{"initial":"a","modes":{"a":{"sprite":"s","offset":[0,0],"parallaxX":0.1},"b":{"sprite":"s2","offset":[1,1],"parallaxX":0.2}}},
"rects":{"arena":[100,0,740,448]},
"camera":)J" }
         + _camera + R"J(,"sections":)J" + _sections + "}";
}

[[nodiscard]] inline StageData MustLoad(
    const std::string& _json)
{
    StageLoadDiagnostics    diag;
    slug::Result<StageData> result = LoadStageFromJson(_json, &diag);
    if (!result)
    {
        ADD_FAILURE() << "stage load failed: " << diag.message;
        return {};
    }
    return result.Take();
}

[[nodiscard]] inline std::string LoadError(
    const std::string& _json)
{
    StageLoadDiagnostics          diag;
    const slug::Result<StageData> result = LoadStageFromJson(_json, &diag);
    return result ? std::string {} : (diag.message.empty() ? std::string { "<no message>" } : diag.message);
}

class RunnerFixture
{
public:
    explicit RunnerFixture(
        const std::string& _json)
        : data(MustLoad(_json))
        , rng(uint64_t { 1 })
        , runner(data, host, camera, rng)
    {
        camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
        runner.Begin();
    }

    void Step(
        const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            host.now = runner.GetTick();
            runner.Tick();
        }
    }

    void StepAt(
        const float _x,
        const int   _count = 1)
    {
        host.playerFeet.x = _x;
        Step(_count);
    }

    StageData        data;
    FakeStageHost    host;
    Camera           camera;
    slug::RngMt19937 rng;
    StageRunner      runner;
};

}   // namespace slugtest

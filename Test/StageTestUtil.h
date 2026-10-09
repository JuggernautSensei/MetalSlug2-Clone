#pragma once
#include <functional>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Result.h"
#include "Base/Random.h"
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "Game/Stage/StageRunner.h"
#include "Render/Camera.h"

// ===========================================
//  스테이지 테스트 도우미
//   FakeStageHost : IStageHost 의 완전한 가짜 구현 (호출 기록, 격파 제어, 지면 함수, 플레이어 위치).
//   RunnerFixture : StageData + FakeStageHost + Camera + StageRunner 묶음.
// ===========================================

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

class FakeStageHost final : public IStageHost
{
public:
    // --- 테스트가 조작하는 값 ---
    int64_t                                              now = 0;
    Vec2                                         playerFeet = { 0.f, 378.f };
    bool                                                 bPlayerAlive = true;
    bool                                                 bGameOver    = false;
    std::function<std::optional<float>(float, float)>    groundFn = [](float, float _fromY) -> std::optional<float> {
        if (_fromY <= 378.f)
        {
            return 378.f;
        }
        return std::nullopt;
    };
    bool                                                 bAutoDefeat = false;   // 스폰된 enemy/prop 이 즉시 격파된 것으로 본다
    SpawnId                                              nextId = 1;
    bool                                                 bRefuseSpawns = false;

    // --- 기록 ---
    std::vector<SpawnLogEntry> spawns;
    std::vector<HostEvent>     events;
    std::set<SpawnId>          defeated;
    std::set<SpawnId>          destroyed;
    bool                       bEnded = false;
    std::string                endNext;
    std::string                currentGround;
    std::string                currentBackdrop;
    std::vector<Vec2>  warps;
    std::vector<std::pair<SpawnId, std::string>> commands;
    std::vector<uint32_t>      clears;
    float                      invincibleSeconds = 0.f;

    SpawnId SpawnEntity(const SpawnRequest& _request) override
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

    bool IsDefeated(SpawnId _id) const override { return defeated.contains(_id); }
    bool IsDestroyFinished(SpawnId _id) const override { return destroyed.contains(_id); }

    void ClearEntities(const EntityFilter& _filter) override
    {
        clears.push_back(_filter.layerMask);
        Log_("clear:" + std::to_string(_filter.layerMask));
    }

    void SendCommand(SpawnId _id, slug::StringView _command) override
    {
        commands.emplace_back(_id, std::string { _command });
        Log_("cmd:" + std::to_string(_id) + ":" + std::string { _command });
    }

    void SetGround(slug::StringView _name) override
    {
        currentGround = std::string { _name };
        Log_("ground:" + currentGround);
    }

    std::optional<float> GetGroundFindY(float _x, float _fromY) const override { return groundFn(_x, _fromY); }

    Vec2 PlayerFeet() const override { return playerFeet; }
    bool         PlayerAlive() const override { return bPlayerAlive; }
    bool         IsGameOver() const override { return bGameOver; }

    void SetPlayerInvincible(float _seconds) override
    {
        invincibleSeconds = _seconds;
        Log_("invincible:" + std::to_string(_seconds));
    }

    void WarpPlayer(Vec2 _feet) override
    {
        playerFeet = _feet;
        warps.push_back(_feet);
        Log_("warp");
    }

    void SetBackdrop(slug::StringView _modeId) override
    {
        currentBackdrop = std::string { _modeId };
        Log_("backdrop:" + currentBackdrop);
    }

    void ShowBanner(const slug::String& _text, float _seconds) override { Log_("banner:" + _text + ":" + std::to_string(_seconds)); }

    void FadeTo(eFadeTarget _target, float _seconds) override { Log_(std::string { "fade:" } + (_target == eFadeTarget::Black ? "black" : "clear") + ":" + std::to_string(_seconds)); }

    void PlayBgm(slug::StringView _name, bool _bLoop) override { Log_("bgm:" + std::string { _name } + (_bLoop ? ":loop" : ":once")); }

    void EndStage(slug::StringView _nextState) override
    {
        bEnded  = true;
        endNext = std::string { _nextState };
        Log_("end:" + endNext);
    }

    void OnSectionEntered(int _index, slug::StringView _id) override
    {
        Log_("section:" + std::to_string(_index) + ":" + std::string { _id });
    }

    // --- 조회 도우미 ---
    [[nodiscard]] int CountEvents(const std::string& _prefix) const
    {
        int count = 0;
        for (const HostEvent& event: events)
        {
            count += event.text.rfind(_prefix, 0) == 0 ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] const HostEvent* FindEvent(const std::string& _prefix, int _nth = 0) const
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

    [[nodiscard]] int CountSpawnsOf(const std::string& _def) const
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
    void Log_(std::string _text) { events.push_back({ now, std::move(_text) }); }
};

// 작은 스테이지 JSON 을 만든다. _sections 는 "[...]" 배열 문자열, _camera 는 camera 객체 문자열.
inline std::string WrapStage(
    const std::string& _sections,
    const std::string& _camera = R"J({"initialBounds":[0,0,4000,448]})J")
{
    return std::string { R"J({"version":1,"name":"T","world":{"size":[4000,1000],"background":"bg"},
"ground":{"initial":"G0","maps":["G0","G1"]},
"backdrop":{"initial":"a","modes":{"a":{"sprite":"s","offset":[0,0],"parallaxX":0.1},"b":{"sprite":"s2","offset":[1,1],"parallaxX":0.2}}},
"rects":{"arena":[100,0,740,448]},
"camera":)J" } + _camera + R"J(,"sections":)J" + _sections + "}";
}

inline StageData MustLoad(const std::string& _json)
{
    StageLoadDiagnostics diag;
    slug::Result<StageData> result = LoadStageFromJson(_json, &diag);
    if (!result)
    {
        ADD_FAILURE() << "stage load failed: " << diag.message;
        return {};
    }
    return result.Take();
}

// 로드 실패를 기대하고 진단 메시지를 돌려준다. 성공하면 빈 문자열.
inline std::string LoadError(const std::string& _json)
{
    StageLoadDiagnostics diag;
    const slug::Result<StageData> result = LoadStageFromJson(_json, &diag);
    return result ? std::string {} : (diag.message.empty() ? std::string { "<no message>" } : diag.message);
}

class RunnerFixture
{
public:
    explicit RunnerFixture(const std::string& _json)
        : data(MustLoad(_json))
        , rng(uint64_t { 1 })
        , runner(data, host, camera, rng)
    {
        camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
        runner.Begin();
    }

    // 플레이어 x 를 _x 로 두고 _count 틱 진행
    void Step(int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            host.now = runner.GetTick();
            runner.Tick();
        }
    }

    void StepAt(float _x, int _count = 1)
    {
        host.playerFeet.x = _x;
        Step(_count);
    }

    StageData         data;
    FakeStageHost     host;
    Camera            camera;
    slug::RngMt19937   rng;
    StageRunner       runner;
};

}   // namespace slugtest

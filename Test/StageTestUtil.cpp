#include "StageTestUtil.h"

std::optional<float> DefaultGround_(
    const float,
    const float _fromY)
{
    if (_fromY <= 378.f)
    {
        return 378.f;
    }
    return std::nullopt;
}

SpawnId FakeStageHost::SpawnEntity(
    const SpawnRequest& _request)
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

bool FakeStageHost::IsDefeated(
    const SpawnId _id) const
{
    return defeated.contains(_id);
}

bool FakeStageHost::IsDestroyFinished(
    const SpawnId _id) const
{
    return destroyed.contains(_id);
}

void FakeStageHost::ClearEntities(
    const EntityFilter& _filter)
{
    clears.push_back(_filter.layerMask);
    Log_("clear:" + std::to_string(_filter.layerMask));
}

void FakeStageHost::SendCommand(
    const SpawnId    _id,
    const StringView _command)
{
    commands.emplace_back(_id, std::string { _command });
    Log_("cmd:" + std::to_string(_id) + ":" + std::string { _command });
}

void FakeStageHost::SetGround(
    const StringView _name)
{
    currentGround = std::string { _name };
    Log_("ground:" + currentGround);
}

std::optional<float> FakeStageHost::GetGroundFindY(
    const float _x,
    const float _fromY) const
{
    groundQueries.emplace_back(_x, _fromY);
    return groundFn(_x, _fromY);
}

Vec2 FakeStageHost::PlayerFeet() const
{
    return playerFeet;
}

bool FakeStageHost::PlayerAlive() const
{
    return bPlayerAlive;
}

bool FakeStageHost::IsGameOver() const
{
    return bGameOver;
}

void FakeStageHost::SetPlayerInvincible(
    const float _seconds)
{
    invincibleSeconds = _seconds;
    Log_("invincible:" + std::to_string(_seconds));
}

void FakeStageHost::WarpPlayer(
    const Vec2 _feet)
{
    playerFeet = _feet;
    warps.push_back(_feet);
    Log_("warp");
}

void FakeStageHost::SetBackdrop(
    const StringView _modeId)
{
    currentBackdrop = std::string { _modeId };
    Log_("backdrop:" + currentBackdrop);
}

void FakeStageHost::ShowBanner(
    const String& _text,
    const float   _seconds)
{
    Log_("banner:" + _text + ":" + std::to_string(_seconds));
}

void FakeStageHost::FadeTo(
    const eFadeTarget _target,
    const float       _seconds)
{
    Log_(std::string { "fade:" } + (_target == eFadeTarget::Black ? "black" : "clear") + ":" + std::to_string(_seconds));
}

void FakeStageHost::PlayBgm(
    const StringView _name,
    const bool       _bLoop)
{
    Log_("bgm:" + std::string { _name } + (_bLoop ? ":loop" : ":once"));
}

void FakeStageHost::ChangeBgm(
    const StringView _name,
    const bool       _bLoop,
    const float      _fadeOutSec)
{
    Log_("bgmChange:" + std::string { _name } + (_bLoop ? ":loop:" : ":once:") + std::to_string(_fadeOutSec));
}

void FakeStageHost::PlaySfx(
    const StringView _name)
{
    Log_("sfx:" + std::string { _name });
}

void FakeStageHost::EndStage(
    const StringView _nextState)
{
    bEnded  = true;
    endNext = std::string { _nextState };
    Log_("end:" + endNext);
}

void FakeStageHost::OnSectionEntered(
    const int        _index,
    const StringView _id)
{
    Log_("section:" + std::to_string(_index) + ":" + std::string { _id });
}

int FakeStageHost::CountEvents(
    const std::string& _prefix) const
{
    int count = 0;
    for (const HostEvent& event: events)
    {
        count += event.text.rfind(_prefix, 0) == 0 ? 1 : 0;
    }
    return count;
}

const HostEvent* FakeStageHost::FindEvent(
    const std::string& _prefix,
    int                _nth) const
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

int FakeStageHost::CountSpawnsOf(
    const std::string& _def) const
{
    int count = 0;
    for (const SpawnLogEntry& entry: spawns)
    {
        count += entry.request.def == _def ? 1 : 0;
    }
    return count;
}

void FakeStageHost::DefeatAll()
{
    for (const SpawnLogEntry& entry: spawns)
    {
        defeated.insert(entry.id);
        destroyed.insert(entry.id);
    }
}

void FakeStageHost::Log_(
    std::string _text)
{
    events.push_back({ now, std::move(_text) });
}

std::string WrapStage(
    const std::string& _sections,
    const std::string& _camera)
{
    return std::string { R"J({"version":1,"name":"T","world":{"size":[4000,1000],"background":"bg"},
"ground":{"initial":"G0","maps":["G0","G1"]},
"backdrop":{"initial":"a","modes":{"a":{"sprite":"s","offset":[0,0],"parallaxX":0.1},"b":{"sprite":"s2","offset":[1,1],"parallaxX":0.2}}},
"rects":{"arena":[100,0,740,448]},
"camera":)J" }
         + _camera + R"J(,"sections":)J" + _sections + "}";
}

StageData MustLoad(
    const std::string& _json)
{
    StageLoadDiagnostics diag;
    Result<StageData>    result = LoadStageFromJson(_json, &diag);
    if (!result)
    {
        ADD_FAILURE() << "stage load failed: " << diag.message;
        return {};
    }
    return result.Take();
}

std::string LoadError(
    const std::string& _json)
{
    StageLoadDiagnostics    diag;
    const Result<StageData> result = LoadStageFromJson(_json, &diag);
    return result ? std::string {} : (diag.message.empty() ? std::string { "<no message>" } : diag.message);
}

RunnerFixture::RunnerFixture(
    const std::string& _json)
    : data(MustLoad(_json))
    , rng(uint64_t { 1 })
    , runner(data, host, camera, rng)
{
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    runner.Begin();
}

void RunnerFixture::Step(
    const int _count)
{
    for (int i = 0; i < _count; ++i)
    {
        host.now = runner.GetTick();
        runner.Tick();
    }
}

void RunnerFixture::StepAt(
    const float _x,
    const int   _count)
{
    host.playerFeet.x = _x;
    Step(_count);
}

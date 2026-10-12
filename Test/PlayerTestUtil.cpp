#include "PlayerTestUtil.h"

GroundMap MakeFlatGround(
    const int _width,
    const int _height,
    const int _floorY)
{
    GroundMap map = GroundMap::MakeEmpty(_width, _height);
    map.FillRect(0, _floorY, _width - 1, _height - 1, true);
    return map;
}

SandboxConfig MakeConfig(
    GroundMap  _ground,
    const Vec2 _spawn)
{
    SandboxConfig config;
    config.bLoadGround     = false;
    config.cameraBounds    = Aabb::FromMinMax({ 0.f, 0.f }, { static_cast<float>(_ground.GetWidth()), 448.f });
    config.ground          = std::move(_ground);
    config.spawnFeet       = _spawn;
    config.bSpawnTargets   = false;
    config.bLoadTuningFile = false;
    config.bBackground     = false;
    return config;
}

SandboxConfig MakeFlatConfig()
{
    return MakeConfig(MakeFlatGround());
}

Sim::Sim(
    SandboxConfig     _config,
    AnimationLibrary* _pAnims)
    : m_rng(uint64_t { 1 })
{
    m_ctx.pRng   = &m_rng;
    m_ctx.pAnims = _pAnims;

    m_pState  = new SandboxState { std::move(_config) };
    m_pRunner = Scoped<SimRunner> {
        new SimRunner { m_ctx, Scoped<IGameState> { m_pState } }
    };
}

SandboxState& Sim::State()
{
    return *m_pState;
}

PlayerController& Sim::Player()
{
    return m_pState->GetPlayer();
}

Tuning& Sim::Tune()
{
    return m_pState->GetTuning();
}

SimRunner& Sim::Runner()
{
    return *m_pRunner;
}

void Sim::Hold(const eAction _action)
{
    m_frame.Set(_action, true);
}

void Sim::Release(const eAction _action)
{
    m_frame.Set(_action, false);
}

void Sim::ReleaseAll()
{
    m_frame = {};
}

void Sim::Set(const eAction _action, const bool _bHeld)
{
    m_frame.Set(_action, _bHeld);
}

void Sim::Step(
    const int _count)
{
    for (int i = 0; i < _count; ++i)
    {
        m_pRunner->Step(m_frame);
    }
}

void Sim::Tap(
    const eAction _action)
{
    Hold(_action);
    Step();
    Release(_action);
}

void Sim::StepWith(
    const InputFrame _frame)
{
    m_frame = _frame;
    m_pRunner->Step(_frame);
}

int Sim::StepUntilGrounded(
    const int _maxTicks)
{
    for (int i = 0; i < _maxTicks; ++i)
    {
        if (Player().IsGrounded())
        {
            return i;
        }
        Step();
    }
    return Player().IsGrounded() ? _maxTicks : -1;
}

int Sim::StepUntilAirborne(
    const int _maxTicks)
{
    for (int i = 0; i < _maxTicks; ++i)
    {
        if (!Player().IsGrounded())
        {
            return i;
        }
        Step();
    }
    return !Player().IsGrounded() ? _maxTicks : -1;
}

int Sim::StepUntilDead(
    const int _maxTicks)
{
    for (int i = 0; i < _maxTicks; ++i)
    {
        if (!Player().IsAlive())
        {
            return i;
        }
        Step();
    }
    return !Player().IsAlive() ? _maxTicks : -1;
}

int Sim::StepUntilWeapon(
    const int     _maxTicks,
    const eWeapon _weapon)
{
    for (int i = 0; i < _maxTicks; ++i)
    {
        if (Player().GetWeapon().weapon == _weapon)
        {
            return i;
        }
        Step();
    }
    return Player().GetWeapon().weapon == _weapon ? _maxTicks : -1;
}

float Sim::X()
{
    return Player().GetFeet().x;
}

float Sim::Y()
{
    return Player().GetFeet().y;
}

bool LoadAnims(
    AnimationLibrary& _out)
{
    Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(FindAssetRoot());
    if (!library)
    {
        return false;
    }
    _out = library.Take();
    return true;
}

#include "VehicleTestUtil.h"

FilePath VehicleDefsDir()
{
    return FindAssetRoot() / "Defs";
}

bool LoadVehicleLib(
    EnemyDefLibrary& _out,
    String*          _pMessage)
{
    Result<EnemyDefLibrary> lib = EnemyDefLibrary::LoadDirectory(VehicleDefsDir(), _pMessage);
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

VehicleRig::VehicleRig(
    const VECTOR2 _player,
    const float   _camLeft)
    : m_world(&m_lib, 7)
    , m_ground(MakeFlatGround(3000))
    , m_player(
          _player)
    , m_view(Aabb::FromMinSize({ _camLeft, 0.f }, { 640.f, 448.f }))
{
    m_bLoaded = LoadVehicleLib(m_lib, &m_message);
    m_world.SetLibrary(&m_lib);
}

bool VehicleRig::IsLoaded() const
{
    return m_bLoaded;
}

const String& VehicleRig::GetLoadMessage() const
{
    return m_message;
}

EnemyWorld& VehicleRig::World()
{
    return m_world;
}

EnemyDefLibrary& VehicleRig::Lib()
{
    return m_lib;
}

CombatSystem& VehicleRig::Combat()
{
    return m_combat;
}

ProjectileSystem& VehicleRig::Projectiles()
{
    return m_projectiles;
}

Vector<EnemyEvent>& VehicleRig::Events()
{
    return m_events;
}

void VehicleRig::SetPlayer(
    const VECTOR2 _feet)
{
    m_player = _feet;
}

void VehicleRig::SetView(
    const float _camLeft)
{
    m_view = Aabb::FromMinSize({ _camLeft, 0.f }, { 640.f, 448.f });
}

EnemyHandle VehicleRig::Spawn(
    const char*        _name,
    const VECTOR2      _feet,
    const SpawnParams& _params)
{
    const PhysicsWorld world = MakeWorld_();
    return m_world.Spawn(_name, _feet, _params, &world);
}

void VehicleRig::Step(
    const int _count)
{
    for (int i = 0; i < _count; ++i)
    {
        m_combat.BeginTick();
        RegisterPlayer_();
        m_world.RegisterHurtboxes(m_combat);
        const EnemyContext ctx = MakeCtx_();
        m_world.Tick(ctx);
        m_projectiles.Step(&m_ground, m_combat, kStepSec);
        const Vector<HitEvent>& events = m_combat.Resolve();
        CountPlayerHits_(events);
        m_world.OnHitEvents(events);
        m_projectiles.OnHitEvents(events, m_combat);
        CollectEvents_();
        ++m_ticks;
    }
}

int VehicleRig::Ticks() const
{
    return m_ticks;
}

size_t VehicleRig::Strike(
    const uint32_t _owner,
    const eHitKind _kind,
    const int      _damage,
    const eTeam    _team)
{
    m_combat.BeginTick();
    RegisterPlayer_();
    m_world.RegisterHurtboxes(m_combat);
    bool bFound = false;
    Aabb box    = {};
    for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
    {
        if (hurtbox.owner == _owner)
        {
            box    = hurtbox.box;
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        return 0;
    }
    Hitbox hitbox;
    hitbox.box    = box;
    hitbox.team   = _team;
    hitbox.kind   = _kind;
    hitbox.damage = _damage;
    hitbox.owner  = _team == eTeam::Player ? kTestPlayerOwner : 2000u;
    hitbox.id     = m_combat.NewHitId();
    m_combat.AddHitbox(hitbox);
    const Vector<HitEvent>& events = m_combat.Resolve();
    const size_t            count  = events.size();
    m_world.OnHitEvents(events);
    CollectEvents_();
    return count;
}

int VehicleRig::CountEvents(
    const eEnemyEventKind _kind) const
{
    int count = 0;
    for (const EnemyEvent& event: m_events)
    {
        count += event.kind == _kind ? 1 : 0;
    }
    return count;
}

int VehicleRig::SumScore(
    const eEnemyEventKind _kind) const
{
    int sum = 0;
    for (const EnemyEvent& event: m_events)
    {
        if (event.kind == _kind)
        {
            sum += event.score;
        }
    }
    return sum;
}

int VehicleRig::CountShots(
    const EnemyHandle _owner) const
{
    int count = 0;
    for (const Shot& shot: m_projectiles.GetShots())
    {
        count += (shot.bAlive && shot.params.owner == _owner.id) ? 1 : 0;
    }
    return count;
}

const EnemyState* VehicleRig::E(
    const EnemyHandle _handle) const
{
    return m_world.Get(_handle);
}

int VehicleRig::PlayerHits() const
{
    return m_playerHits;
}

PhysicsWorld VehicleRig::MakeWorld_() const
{
    PhysicsWorld world;
    world.pGround = &m_ground;
    return world;
}

EnemyContext VehicleRig::MakeCtx_()
{
    EnemyContext ctx;
    ctx.physics = MakeWorld_();
    m_walls.clear();
    for (const Aabb& box: m_world.GetSolidBoxes())
    {
        m_walls.push_back({ box });
    }
    ctx.physics.walls        = m_walls;
    ctx.pCombat              = &m_combat;
    ctx.pProjectiles         = &m_projectiles;
    ctx.player.feet          = m_player;
    ctx.player.center        = { m_player.x, m_player.y - 30.f };
    ctx.player.bodyBox       = Aabb::FromBottomCenter(m_player, { 20.f, 58.f });
    ctx.player.bAlive        = true;
    ctx.player.bInvulnerable = false;
    ctx.cameraView           = m_view;
    ctx.pAnims               = nullptr;
    return ctx;
}

void VehicleRig::RegisterPlayer_()
{
    Hurtbox hurtbox;
    hurtbox.box   = Aabb::FromBottomCenter(m_player, { 16.f, 54.f });
    hurtbox.team  = eTeam::Player;
    hurtbox.owner = kTestPlayerOwner;
    m_combat.AddHurtbox(hurtbox);
}

void VehicleRig::CollectEvents_()
{
    for (EnemyEvent& event: m_world.TakeEvents())
    {
        m_events.push_back(std::move(event));
    }
}

void VehicleRig::CountPlayerHits_(
    const Vector<HitEvent>& _events)
{
    for (const HitEvent& event: _events)
    {
        if (event.target == kTestPlayerOwner)
        {
            ++m_playerHits;
        }
    }
}

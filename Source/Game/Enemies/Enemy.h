#pragma once
#include <optional>

#include "Base/Types.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Physics.h"

class EnemyWorld;

struct EnemyFxRequest
{
    String  clip       = {};
    VECTOR2 position   = { 0.f, 0.f };
    int     depth      = 60;
    int     delayTicks = 0;
    bool    bFlipX     = false;
    bool    bDebris    = false;
    VECTOR2 velocity   = { 0.f, 0.f };
};

struct EnemyEnv
{
    EnemyWorld&            world;
    const EnemyContext&    ctx;
    const EnemyDefLibrary& lib;
    PhysicsTuning          phys    = {};
    int                    zOffset = 0;
};

struct EnemyRenderEnv
{
    SpriteBatch&            sprites;
    const AnimationLibrary& anims;
    const SpriteLibrary*    pSprites = nullptr;
    const ProjectileSystem& projectiles;
    VECTOR2                 feet    = { 0.f, 0.f };
    float                   alpha   = 0.f;
    int                     depth   = 0;
    float                   flash   = 0.f;
    int                     zOffset = 0;
};

class Enemy
{
    JUG_CLASS(Enemy, NO_COPY, NO_MOVE)
    friend class EnemyWorld;

public:
    Enemy()          = default;
    virtual ~Enemy() = default;
    [[nodiscard]] const EnemyState&        GetState() const noexcept { return m_state; }
    [[nodiscard]] virtual int              GetShotCounter() const noexcept { return 0; }
    virtual void                           Init(const EnemyDef& _def, EnemyEnv& _env) = 0;
    virtual void                           Tick(const EnemyDef& _def, EnemyEnv& _env) = 0;
    virtual bool                           OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event);
    virtual void                           OnDeath(const EnemyDef& _def, EnemyEnv& _env);
    virtual void                           TickDying(const EnemyDef& _def, EnemyEnv& _env);
    virtual void                           RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const;
    virtual void                           SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const;
    [[nodiscard]] virtual bool             DeathDone(const EnemyDef& _def) const;
    [[nodiscard]] virtual const AnimFrame* BodyFrame(const EnemyDef& _def, const AnimationLibrary& _anims) const;

protected:
    void                      HashActionState_(Fnv1a& _hash, int _shots, int _progress, int _secondary = 0, int _tertiary = 0, int _quaternary = 0) const;
    [[nodiscard]] EnemyState& GetMutableState() noexcept { return m_state; }

private:
    virtual void HashActionState(Fnv1a& _hash) const;

    EnemyState m_state = {};
};

[[nodiscard]] Scoped<Enemy> MakeEnemy(eEnemyBehavior _behavior);

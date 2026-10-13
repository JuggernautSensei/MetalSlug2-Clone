#pragma once
#include <optional>

#include "Base/Math.h"
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

struct BehaviorOps
{
    void (*init)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    void (*tick)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    bool (*onHit)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) = nullptr;

    void (*onDeath)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    void (*tickDying)(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env) = nullptr;

    void (*registerHurtboxes)(const Enemy& _e, const EnemyDef& _def, CombatSystem& _combat) = nullptr;

    void (*submitParts)(const Enemy& _e, const EnemyDef& _def, const EnemyRenderEnv& _render) = nullptr;

    bool (*deathDone)(const Enemy& _e, const EnemyDef& _def) = nullptr;

    void (*onCommand)(Enemy& _e, const EnemyDef& _def, StringView _command) = nullptr;

    const AnimFrame* (*bodyFrame)(const Enemy& _e, const EnemyDef& _def, const AnimationLibrary& _anims) = nullptr;
};

[[nodiscard]] const BehaviorOps& GetBehaviorOps(eEnemyBehavior _behavior);

extern const BehaviorOps kSoldierOps;
extern const BehaviorOps kBzkOps;
extern const BehaviorOps kHangOps;
extern const BehaviorOps kFlyingBzkOps;
extern const BehaviorOps kShielderOps;
extern const BehaviorOps kPowOps;
extern const BehaviorOps kJeepOps;
extern const BehaviorOps kBTankOps;
extern const BehaviorOps kMissleTankOps;
extern const BehaviorOps kSarubiaOps;
extern const BehaviorOps kBusOps;
extern const BehaviorOps kHeliOps;
extern const BehaviorOps kHeliBombOps;
extern const BehaviorOps kBigShieeOps;

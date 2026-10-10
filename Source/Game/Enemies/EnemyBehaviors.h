#pragma once
#include <optional>

#include "Base/Random.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Physics.h"

namespace slug
{

class EnemyWorld;

struct EnemyFxRequest
{
    slug::String  clip       = {};
    Vec2 position   = { 0.f, 0.f };
    int          layer      = 60;
    int          delayTicks = 0;
};

struct EnemyEnv
{
    EnemyWorld&             world;
    const EnemyContext&     ctx;
    const EnemyDefLibrary&  lib;
    PhysicsTuning           phys = {};
};

struct EnemyRenderEnv
{
    SpriteBatch&            sprites;
    const AnimationLibrary& anims;
    const SpriteLibrary*    pSprites;
    const ProjectileSystem& projectiles;
    Vec2            feet;
    float                   alpha;
    int                     layer;
    float                   flash = 0.f;   // 이 적의 전체 피격 플래시 양 (EnemyWorld::Submit 이 채움)
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
    void (*onCommand)(Enemy& _e, const EnemyDef& _def, slug::StringView _command) = nullptr;   // 스테이지 명령 (command 액션)
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
}

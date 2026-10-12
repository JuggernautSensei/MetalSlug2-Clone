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
    String clip       = {};
    Vec2   position   = { 0.f, 0.f };
    int    depth      = 60;
    int    delayTicks = 0;
    bool   bFlipX     = false;          // 그림 좌우 반전 (FxDef.mirror)
    bool   bDebris    = false;          // 잔해 연출 (velocity 로 날린다)
    Vec2   velocity   = { 0.f, 0.f };   // px/s
};

struct EnemyEnv
{
    EnemyWorld&            world;
    const EnemyContext&    ctx;
    const EnemyDefLibrary& lib;
    PhysicsTuning          phys    = {};
    int                    zOffset = 0;   // 인스턴스 z - 기본 z: 이 적이 요청하는 FX/투사체 depth 에 더한다
};

struct EnemyRenderEnv
{
    SpriteBatch&            sprites;
    const AnimationLibrary& anims;
    const SpriteLibrary*    pSprites = nullptr;
    const ProjectileSystem& projectiles;
    Vec2                    feet    = { 0.f, 0.f };
    float                   alpha   = 0.f;
    int                     depth   = 0;
    float                   flash   = 0.f;   // 이 적의 전체 피격 플래시 양 (EnemyWorld::Submit 이 채움)
    int                     zOffset = 0;     // 인스턴스 z - 기본 z: 모든 부품 depth 에 더한다
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

    void (*onCommand)(Enemy& _e, const EnemyDef& _def, StringView _command) = nullptr;   // 스테이지 명령 (command 액션)

    // 몸통 프레임 대체 (렌더 전용): 돌려주면 현재 클립 프레임 대신 그린다. nullptr 이면 클립 프레임
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

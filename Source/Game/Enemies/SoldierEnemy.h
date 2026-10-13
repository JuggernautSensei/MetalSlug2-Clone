#pragma once
#include "Game/Enemies/Enemy.h"

enum class eSoldierState
{
    Wait,
    Fall,
    Land,
    Idle,
    Move,
    MoveDown,
    HJump,
    Throw,
    Slash,
    JumpSlashReady,
    JumpSlash,
    JumpSlashLand,
    Climb
};

class SoldierEnemy : public Enemy
{
    JUG_CLASS(SoldierEnemy, NO_COPY, NO_MOVE)

public:
    SoldierEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void                HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] float AbsDx_(const EnemyEnv& _env) const;
    void                Reposition_(const EnemyDef& _def, EnemyEnv& _env, eSoldierState _fallback);
    void                Go_(const EnemyDef& _def, EnemyEnv& _env, eSoldierState _requestedState);
    void                TickClimb_(const EnemyDef& _def, EnemyEnv& _env);
    void                DecideBomb_(const EnemyDef& _def, EnemyEnv& _env);
    void                DecideSword_(const EnemyDef& _def, EnemyEnv& _env);
    void                DecideMissile_(const EnemyDef& _def, EnemyEnv& _env);
};

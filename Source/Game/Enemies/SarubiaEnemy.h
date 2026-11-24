#pragma once
#include "Game/Enemies/Enemy.h"

enum class eSarubiaState
{
    Idle,
    Turn,
    MoveStart,
    Move,
    MoveStop,
    ShotStart,
    Shot
};

class SarubiaEnemy : public Enemy
{
    JUG_CLASS(SarubiaEnemy, NO_COPY, NO_MOVE)

public:
    SarubiaEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    [[nodiscard]] int ClipOf_(const SarubiaConfig& _config) const;
    void              Go_(const EnemyDef& _def, eSarubiaState _state);
};

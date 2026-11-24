#pragma once
#include "Game/Enemies/Enemy.h"

enum class eHangState
{
    Idle,
    Throw
};

class HangEnemy : public Enemy
{
    JUG_CLASS(HangEnemy, NO_COPY, NO_MOVE)

public:
    HangEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void HashActionState(Fnv1a& _hash) const override;
    void Go_(const EnemyDef& _def, EnemyEnv& _env, eHangState _state);

    int m_throwTicks = 0;
};

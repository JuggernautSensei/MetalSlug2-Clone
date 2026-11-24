#pragma once
#include "Game/Enemies/Enemy.h"

class HeliBombEnemy : public Enemy
{
    JUG_CLASS(HeliBombEnemy, NO_COPY, NO_MOVE)

public:
    HeliBombEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void HashActionState(Fnv1a& _hash) const override;
    void AddBombHitbox_(VECTOR2 _size, int _damage, eHitKind _kind, EnemyEnv& _env) const;

    int m_blastTicks = 0;
};

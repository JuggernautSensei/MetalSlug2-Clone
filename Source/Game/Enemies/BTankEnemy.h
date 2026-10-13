#pragma once
#include "Game/Enemies/Enemy.h"

enum class eBTankState
{
    Closed,
    Open,
    Idle,
    Fire
};

class BTankEnemy : public Enemy
{
    JUG_CLASS(BTankEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetShotCounter() const noexcept override { return m_shotsRemaining; }
    BTankEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void              HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] int ClipOf_(const BTankConfig& _config) const;
    void              Go_(const EnemyDef& _def, eBTankState _state);

    int m_shotsRemaining = 0;
};

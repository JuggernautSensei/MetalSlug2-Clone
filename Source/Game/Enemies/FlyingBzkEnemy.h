#pragma once
#include "Game/Enemies/Enemy.h"

enum class eFlyingState
{
    Descend,
    Shot
};

class FlyingBzkEnemy : public Enemy
{
    JUG_CLASS(FlyingBzkEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetShotCounter() const noexcept override { return m_shotsFired; }
    FlyingBzkEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;
    void RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;

private:
    void               HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] bool IsOnScreen_(const EnemyEnv& _env) const;
    void               UpdatePosture_(const FlyingBzkConfig& _config, const EnemyEnv& _env);
    void               Go_(const EnemyDef& _def, eFlyingState _state);

    int m_fallTicks = 0;

    int m_shotsFired     = 0;
    int m_fireTicks      = 0;
    int m_aimIndex       = 0;
    int m_driftDirection = 0;
    int m_driftTicks     = 0;
};

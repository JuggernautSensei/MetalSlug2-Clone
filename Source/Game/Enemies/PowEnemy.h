#pragma once
#include "Game/Enemies/Enemy.h"

enum class ePowState
{
    Stuck,
    Escape,
    Walk,
    MoveDown,
    Glide,
    Close,
    Run
};

class PowEnemy : public Enemy
{
    JUG_CLASS(PowEnemy, NO_COPY, NO_MOVE)

public:
    PowEnemy() = default;
    void               Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    [[nodiscard]] bool OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) override;
    void               RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;

private:
    void              HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] int VariantClip_(const PowConfig& _config, int _slot, int _base) const;
    void              Go_(const EnemyDef& _def, EnemyEnv& _env, ePowState _state);
    bool              TouchingPlayer_(EnemyEnv& _env);

    int m_turnTicks      = 0;
    int m_turnDelayTicks = 0;
    int m_patrolTicks    = 0;
};

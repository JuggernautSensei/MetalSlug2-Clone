#pragma once
#include "Game/Enemies/Enemy.h"

enum class eShielderState
{
    Wait,
    Fall,
    Idle,
    Move,
    MoveStop,
    ShotStart,
    ShotReady,
    Shot,
    ShotEnd,
    LookStart,
    LookIdle,
    LookEnd
};

class ShielderEnemy : public Enemy
{
    JUG_CLASS(ShielderEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetShotCounter() const noexcept override { return m_shotsRemaining; }
    ShielderEnemy() = default;
    void               Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    [[nodiscard]] bool OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) override;
    void               OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void               TickDying(const EnemyDef& _def, EnemyEnv& _env) override;
    void               RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;

private:
    void               HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] int  StateClip_(const ShielderConfig& _config) const;
    [[nodiscard]] bool StateDone_(const EnemyDef& _def) const;
    void               Go_(const EnemyDef& _def, EnemyEnv& _env, eShielderState _state, int _dir = -1);

    int m_shotsRemaining = 0;
};

#pragma once
#include "Game/Enemies/Enemy.h"

enum class eMissileTankState
{
    Idle,
    Aim,
    Ready,
    Shot
};

class MissileTankEnemy : public Enemy
{
    JUG_CLASS(MissileTankEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetShotCounter() const noexcept override { return m_shotsRemaining; }
    MissileTankEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void               HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] bool IsUp_() const;
    [[nodiscard]] int  ClipOf_(const MissileTankConfig& _config) const;
    void               Go_(const EnemyDef& _def, eMissileTankState _state);

    int m_shotsRemaining   = 0;
    int m_launcherPosition = 0;
};

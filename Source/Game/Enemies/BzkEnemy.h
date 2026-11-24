#pragma once
#include "Game/Enemies/Enemy.h"

enum class eBzkState
{
    Wait,
    Fall,
    Idle,
    Move,
    HJump,
    Landing,
    Aim,
    ShotReady,
    Shot,
    Recover,
    Climb
};

class BzkEnemy : public Enemy
{
    JUG_CLASS(BzkEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetShotCounter() const noexcept override { return m_shotsRemaining; }
    BzkEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void              HashActionState(Fnv1a& _hash) const override;
    void              Go_(const EnemyDef& _def, EnemyEnv& _env, eBzkState _state);
    void              TickClimb_(const EnemyDef& _def, EnemyEnv& _env);
    [[nodiscard]] int ChoosePosture_(const BzkConfig& _config, const EnemyEnv& _env, bool& _bDown) const;

    int  m_shotsRemaining = 0;
    int  m_aimPosture     = 0;
    int  m_aimDown        = 0;
    int  m_jumpStarted    = 0;
    bool m_bHasAimPosture = false;
};

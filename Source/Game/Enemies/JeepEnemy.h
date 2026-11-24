#pragma once
#include "Game/Enemies/Enemy.h"

enum class eJeepState
{
    Wait,
    IdleAndMove,
    Move,
    Turn,
    TurnToMove,
    Idle,
    Aimming,
    ShotIdle,
    ShotReady,
    Shot,
    ReAim,
    TurnAfterShot
};

class JeepEnemy : public Enemy
{
    JUG_CLASS(JeepEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetShotCounter() const noexcept override { return m_shotsRemaining; }
    JeepEnemy() = default;
    void Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;

private:
    void                HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] int   ClipOf_(const JeepConfig& _config) const;
    void                Go_(const EnemyDef& _def, eJeepState _state);
    [[nodiscard]] float TargetX_(const JeepConfig& _config, const Aabb& _view) const;
    [[nodiscard]] int   ParkSide_(const JeepConfig& _config, const Aabb& _view) const;
    [[nodiscard]] bool  PlayerOnFireSide_(const EnemyEnv& _env) const;

    int m_shotsRemaining = 0;
    int m_parkSide       = 0;
    int m_hasTurned      = 0;
};

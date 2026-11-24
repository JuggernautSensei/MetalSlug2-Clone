#pragma once
#include "Game/Enemies/Enemy.h"

enum class eBusState
{
    Parked,
    Active
};

class BusEnemy : public Enemy
{
    JUG_CLASS(BusEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] int GetDamageStage() const noexcept { return m_damageStage; }
    [[nodiscard]] int GetElapsedTicks() const noexcept { return m_elapsedTicks; }
    BusEnemy() = default;
    void               Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void               OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void               TickDying(const EnemyDef& _def, EnemyEnv& _env) override;
    void               SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const override;
    [[nodiscard]] bool DeathDone(const EnemyDef& _def) const override;

private:
    void HashActionState(Fnv1a& _hash) const override;
    void EnterStage_(const EnemyDef& _def, EnemyEnv& _env, int _stage);
    void SpawnStairSoldier_(const EnemyDef& _def, EnemyEnv& _env);
    void TickSoldiers_(const EnemyDef& _def, EnemyEnv& _env);
    void UpdateStage_(const EnemyDef& _def, EnemyEnv& _env);
    void SubmitDebris_(const EnemyDef& _def, const EnemyRenderEnv& _render) const;
    void SubmitLive_(const EnemyDef& _def, const EnemyRenderEnv& _render) const;

    ARRAY<int, BusConfig::kStageCount + 1> m_stageStartTicks = {};
    int                                    m_nextSoldierTick = 0;

    int m_elapsedTicks = 0;
    int m_damageStage  = 0;
};

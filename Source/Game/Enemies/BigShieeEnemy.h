#pragma once
#include "Game/Enemies/Enemy.h"
#include "Game/Enemies/BigShiee.h"

class BigShieeEnemy : public Enemy
{
    JUG_CLASS(BigShieeEnemy, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] const BigShieeData& GetBossData() const noexcept { return m_boss; }
    BigShieeEnemy() = default;
    void               Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    [[nodiscard]] bool OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) override;
    void               OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void               TickDying(const EnemyDef& _def, EnemyEnv& _env) override;
    void               RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;
    void               SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const override;
    [[nodiscard]] bool DeathDone(const EnemyDef& _def) const override;

private:
    void                         HashActionState(Fnv1a& _hash) const override;
    [[nodiscard]] eBigShieePhase Phase_() const;

    void SetTurretMode_(int _i, eBigShieeTurretMode _mode);

    void                  SetCannonMode_(eBigShieeCannonMode _mode);
    [[nodiscard]] int     AliveTurrets_() const;
    [[nodiscard]] float   AgeSec_() const;
    [[nodiscard]] float   DeploySec_() const;
    [[nodiscard]] bool    IsScrollPaused_(const BigShieeConfig& _config) const;
    [[nodiscard]] bool    IsScrollStopped_(const BigShieeConfig& _config) const;
    [[nodiscard]] VECTOR2 ExtraFromWorld_(const EnemyDef& _def, int _weapon, VECTOR2 _muzzle) const;
    [[nodiscard]] int     AimPos_(const BigShieeConfig& _config, const BigShieeTurretConfig& _turret, const EnemyEnv& _env, int _pos) const;
    void                  FireShell_(const EnemyDef& _def, EnemyEnv& _env, int _turret, int _barrel);
    void                  TickTurret_(const EnemyDef& _def, EnemyEnv& _env, int _i);
    void                  TickVolley_(const EnemyDef& _def, EnemyEnv& _env);
    void                  TickChute_(const EnemyDef& _def, EnemyEnv& _env);
    [[nodiscard]] bool    CannonVulnerable_() const;
    [[nodiscard]] float   PlayerAngleDeg_(const BigShieeConfig& _config, const EnemyEnv& _env) const;
    [[nodiscard]] int     TargetYaw_(const BigShieeConfig& _config, const EnemyEnv& _env, bool _bFire) const;
    void                  TurnCannon_(const BigShieeConfig& _config, int _target);
    [[nodiscard]] float   CasingLandSec_(const BigShieeConfig& _config) const;
    bool                  CasingNow_(const BigShieeConfig& _config, VECTOR2& _outAt, int& _outFrame, float& _outAlpha) const;
    void                  FireMissile_(const EnemyDef& _def, EnemyEnv& _env);
    void                  TickCannon_(const EnemyDef& _def, EnemyEnv& _env);
    void                  SubmitHull_(const BigShieeConfig& _config, const EnemyRenderEnv& _render, VECTOR2 _feet) const;
    void                  SubmitTurret_(const BigShieeConfig& _config, const EnemyRenderEnv& _render, VECTOR2 _feet, int _i) const;
    void                  SubmitCannon_(const BigShieeConfig& _config, const EnemyRenderEnv& _render, VECTOR2 _feet, float _flash) const;
    void                  SubmitCasing_(const BigShieeConfig& _config, const EnemyRenderEnv& _render) const;

    BigShieeData m_boss = {};

    int m_ageTicks = 0;
};

#pragma once
#include "Game/Enemies/Enemy.h"

struct HeliFlight
{
    int startX        = 0;
    int startY        = 0;
    int targetX       = 0;
    int targetY       = 0;
    int bombTime      = 0;
    int turnFlip      = 0;
    int podTicks      = 0;
    int cycle         = 0;
    int passDirection = 0;
    int idleTicks     = 0;
};

enum class eHeliState
{
    Bomb,
    Low,
    PodOut,
    Move,
    Volley,
    MoveBack,
    Turn   // 방향 전환
};

class HeliEnemy : public Enemy
{
    JUG_CLASS(HeliEnemy, NO_COPY, NO_MOVE)

public:
    HeliEnemy() = default;
    void                           Init(const EnemyDef& _def, EnemyEnv& _env) override;
    void                           Tick(const EnemyDef& _def, EnemyEnv& _env) override;
    void                           OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;
    void                           TickDying(const EnemyDef& _def, EnemyEnv& _env) override;
    void                           SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const override;
    [[nodiscard]] const AnimFrame* BodyFrame(const EnemyDef& _def, const AnimationLibrary& _anims) const override;

private:
    void                HashActionState(Fnv1a& _hash) const override;
    void                SetPodTicks_(int _ticks);
    void                Go_(const EnemyDef& _def, eHeliState _state);
    void                BeginMove_(const EnemyDef& _def, const EnemyEnv& _env, eHeliState _state, VECTOR2 _target);
    [[nodiscard]] bool  StepMove_(const HeliConfig& _config);
    void                StepFollow_(const HeliConfig& _config, const EnemyEnv& _env);
    void                DropBomb_(const EnemyDef& _def, EnemyEnv& _env);
    [[nodiscard]] int   FirstBombTicks_(const HeliConfig& _config) const;
    void                StepBombing_(const EnemyDef& _def, EnemyEnv& _env, int _first, int _bursts);
    [[nodiscard]] float LowTargetX_(const HeliConfig& _config, const EnemyEnv& _env) const;
    void                StepLow_(const HeliConfig& _config, const EnemyEnv& _env);
    void                BeginLow_(const EnemyDef& _def, const EnemyEnv& _env);
    [[nodiscard]] bool  WantsTurn_(const HeliConfig& _config, const EnemyEnv& _env);
    [[nodiscard]] int   ShownPitchFrame_(const EnemyDef& _def, const AnimationLibrary& _anims) const;

    HeliFlight m_flight = {};

    int m_turnWaitTicks = 0;
};

#pragma once
#include "Base/Types.h"
#include "Core/Input.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/SlugTypes.h"
#include "Game/Player/SlugVehicle.h"
#include "World/Combat.h"
#include "World/Physics.h"

class SlugSystem
{
public:
    void Reset(const SlugTuning& _tuning);
    int  Spawn(VECTOR2 _feet, int _facing, int _hp = -1);

    // 플레이어 Tick 앞에서 호출
    void Tick(const SlugContext& _ctx, const InputHistory& _input, PlayerController& _player);
    void OnHitEvents(Span<const HitEvent> _events, Span<const Hitbox> _hitboxes);

    // 폭탄 보급 상자를 탑승 중에 먹으면 캐논탄이
    bool                                  OnBombSupply(int _amount);
    [[nodiscard]] int                     GetCount() const;
    [[nodiscard]] SlugVehicle&            Get(int _index);
    [[nodiscard]] const SlugVehicle&      Get(int _index) const;
    [[nodiscard]] Span<const SlugVehicle> GetAll() const;
    [[nodiscard]] const SlugVehicle*      GetRiddenOrNull() const;
    [[nodiscard]] SlugVehicle*            GetRiddenOrNull();
    [[nodiscard]] bool                    IsRiding() const;
    [[nodiscard]] Vector<Wall>            MakeEnemyWalls() const;
    [[nodiscard]] const SlugTuning&       GetTuning() const;
    [[nodiscard]] uint64_t                HashState() const;

private:
    SlugTuning          m_tuning      = {};
    Vector<SlugVehicle> m_slugs       = {};
    int                 m_riddenIndex = -1;
};

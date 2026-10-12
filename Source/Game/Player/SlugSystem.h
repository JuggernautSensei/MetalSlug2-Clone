#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/Input.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/SlugTypes.h"
#include "Game/Player/SlugVehicle.h"
#include "World/Combat.h"
#include "World/Physics.h"

// 호스트(SandboxState / StageState)가 가진 SV-001 묶음. 탑승/하차를 PlayerController 와 이어 주고,
// 적 이동을 막는 벽, 피격 이벤트 전달, HUD 값을 제공한다.
class SlugSystem
{
public:
    void Reset(const SlugTuning& _tuning);

    // 주차된 차량을 하나 놓는다. 돌려주는 값은 인덱스 (실패 시 -1)
    int Spawn(Vec2 _feet, int _facing, int _hp = -1);

    // 플레이어 Tick 앞에서 호출한다. 탑승 판정, 차량 Tick, 하차/파괴 처리, 플레이어 위치 동기화.
    void Tick(const SlugContext& _ctx, const InputHistory& _input, PlayerController& _player);

    void OnHitEvents(Span<const HitEvent> _events, Span<const Hitbox> _hitboxes);

    // 폭탄 보급 상자를 탑승 중에 먹으면 캐논탄이 된다. 처리했으면 true (플레이어 폭탄은 늘지 않는다).
    bool OnBombSupply(int _amount);

    [[nodiscard]] int           GetCount() const;
    [[nodiscard]] SlugVehicle&  Get(int _index);
    [[nodiscard]] const SlugVehicle& Get(int _index) const;
    [[nodiscard]] Span<const SlugVehicle> GetAll() const;
    [[nodiscard]] int           GetRiddenIndex() const;
    [[nodiscard]] const SlugVehicle* GetRiddenOrNull() const;
    [[nodiscard]] SlugVehicle*       GetRiddenOrNull();
    [[nodiscard]] bool          IsRiding() const;

    // 적 지형 충돌용: 차량 몸통을 벽으로 (보병이 올라타지 못하게)
    [[nodiscard]] Vector<Wall> MakeEnemyWalls() const;

    [[nodiscard]] const SlugTuning& GetTuning() const;

    [[nodiscard]] uint64_t HashState() const;

private:
    SlugTuning         m_tuning        = {};
    Vector<SlugVehicle> m_slugs        = {};
    int                m_riddenIndex   = -1;
};

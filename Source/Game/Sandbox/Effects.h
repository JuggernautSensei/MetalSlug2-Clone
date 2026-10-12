#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

// 잔해 공용 수치. 건물 붕괴 잔해(PropWorld)와 차량/병사 잔해(EffectList, EnemyCommon) 가 같이 쓴다.
inline constexpr float kDebrisGravity         = 1000.f;   // px/s^2
inline constexpr float kDebrisLifeSec         = 5.f;
inline constexpr float kDebrisVxMin           = -250.f;   // 초기 수평 속도 범위 (px/s)
inline constexpr float kDebrisVxMax           = 249.f;
inline constexpr float kDebrisVyMin           = -800.f;   // 초기 수직 속도 범위 (px/s, 위쪽은 음수)
inline constexpr float kDebrisVyMax           = -301.f;
inline constexpr float kDebrisCullBelowMargin = 100.f;

class GroundMap;

struct Effect
{
    AnimPlayer anim     = {};
    Vec2       position = { 0.f, 0.f };
    int        depth    = 0;
    bool       bFlipX   = false;

    bool  bDebris  = false;
    Vec2  velocity = { 0.f, 0.f };   // px/s, +y 아래
    float ageSec   = 0.f;
};

class EffectList
{
public:
    void Clear();

    void Spawn(
        const AnimationLibrary& _library,
        StringView              _clipName,
        Vec2                    _position,
        int                     _depth  = 50,
        bool                    _bFlipX = false);

    void SpawnDebris(
        const AnimationLibrary& _library,
        StringView              _clipName,
        Vec2                    _position,
        int                     _depth,
        Vec2                    _velocity,
        bool                    _bFlipX = false);

    // _cullBelowY: 잔해는 y 가 이 값 + kDebrisCullBelowMargin 을 넘으면 제거한다 (카메라 하단 y 를 넘긴다).
    // 잔해 클립은 루프이므로 IsFinished 로 제거하지 않고, 수명(kDebrisLifeSec) 은 안전망으로 둔다.
    void Update(
        const AnimationLibrary& _library,
        float                   _dt,
        float                   _cullBelowY = 1.0e9f);
    void Submit(const AnimationLibrary& _library, SpriteBatch& _sprites) const;

    [[nodiscard]] size_t             GetCount() const;
    [[nodiscard]] Span<const Effect> GetEffects() const;

private:
    Vector<Effect> m_effects = {};
};

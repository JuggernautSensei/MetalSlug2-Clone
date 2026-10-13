#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

// 잔해 공용 수치
inline constexpr float kDebrisGravity         = 1000.f;
inline constexpr float kDebrisLifeSec         = 5.f;
inline constexpr float kDebrisVxMin           = -250.f;   // 초기 수평 속도 범위
inline constexpr float kDebrisVxMax           = 249.f;
inline constexpr float kDebrisVyMin           = -800.f;   // 초기 수직 속도 범위
inline constexpr float kDebrisVyMax           = -301.f;
inline constexpr float kDebrisCullBelowMargin = 100.f;

class GroundMap;

struct Effect
{
    AnimPlayer anim     = {};
    VECTOR2    position = { 0.f, 0.f };
    int        depth    = 0;
    bool       bFlipX   = false;

    bool    bDebris  = false;
    VECTOR2 velocity = { 0.f, 0.f };
    float   ageSec   = 0.f;
};

class EffectList
{
public:
    void Clear();

    void Spawn(const AnimationLibrary& _library, StringView _clipName, VECTOR2 _position, int _depth = 50, bool _bFlipX = false);

    void SpawnDebris(const AnimationLibrary& _library, StringView _clipName, VECTOR2 _position, int _depth, VECTOR2 _velocity, bool _bFlipX = false);

    void Update(const AnimationLibrary& _library, float _dt, float _cullBelowY = 1.0e9f);
    void Submit(const AnimationLibrary& _library, SpriteBatch& _sprites) const;

    [[nodiscard]] size_t             GetCount() const;
    [[nodiscard]] Span<const Effect> GetEffects() const;

private:
    Vector<Effect> m_effects = {};
};

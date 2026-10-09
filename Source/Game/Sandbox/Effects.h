#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{

// ===========================================
//  EffectList
//   한 번 재생하고 사라지는 단발성 애니메이션 (폭발, 피격 불꽃). 판정과 무관한 연출 전용이다.
// ===========================================

struct Effect
{
    AnimPlayer   anim     = {};
    jug::VECTOR2 position = { 0.f, 0.f };
    int          layer    = 0;
};

class EffectList
{
public:
    void Clear() { m_effects.clear(); }

    // 클립 이름으로 생성한다. 라이브러리에 없으면 무시한다.
    void Spawn(const AnimationLibrary& _library, jug::StringView _clipName, jug::VECTOR2 _position, int _layer = 50);

    void Update(const AnimationLibrary& _library, float _dt);
    void Submit(const AnimationLibrary& _library, SpriteBatch& _sprites) const;

    [[nodiscard]] size_t GetCount() const { return m_effects.size(); }

private:
    jug::Vector<Effect> m_effects = {};
};

}   // namespace slug

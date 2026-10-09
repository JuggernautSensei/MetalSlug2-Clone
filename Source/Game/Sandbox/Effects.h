#pragma once
#include "Base/Types.h"
#include "Base/Math.h"

#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{
struct Effect
{
    AnimPlayer   anim     = {};
    Vec2 position = { 0.f, 0.f };
    int          layer    = 0;
};

class EffectList
{
public:
    void Clear() { m_effects.clear(); }

    void Spawn(const AnimationLibrary& _library, slug::StringView _clipName, Vec2 _position, int _layer = 50);

    void Update(const AnimationLibrary& _library, float _dt);
    void Submit(const AnimationLibrary& _library, SpriteBatch& _sprites) const;

    [[nodiscard]] size_t GetCount() const { return m_effects.size(); }

private:
    slug::Vector<Effect> m_effects = {};
};
}

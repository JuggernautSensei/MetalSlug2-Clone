#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{

struct Effect
{
    AnimPlayer anim     = {};
    Vec2       position = { 0.f, 0.f };
    int        layer    = 0;
};

class EffectList
{
public:
    void Clear();

    void Spawn(
        const AnimationLibrary& _library,
        StringView              _clipName,
        Vec2                    _position,
        int                     _layer = 50);

    void Update(const AnimationLibrary& _library, float _dt);
    void Submit(const AnimationLibrary& _library, SpriteBatch& _sprites) const;

    [[nodiscard]] size_t GetCount() const;

private:
    Vector<Effect> m_effects = {};
};

}   // namespace slug

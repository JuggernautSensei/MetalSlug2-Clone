#include "Game/Sandbox/Effects.h"

#include <algorithm>

namespace slug
{

void EffectList::Spawn(
    const AnimationLibrary& _library,
    const jug::StringView   _clipName,
    const jug::VECTOR2      _position,
    const int               _layer)
{
    const AnimId id = _library.Find(_clipName);
    if (id.IsNull())
    {
        return;
    }

    Effect effect;
    effect.anim.Play(id, true);
    effect.position = _position;
    effect.layer    = _layer;
    m_effects.push_back(effect);
}

void EffectList::Update(
    const AnimationLibrary& _library,
    const float             _dt)
{
    for (Effect& effect: m_effects)
    {
        effect.anim.Update(_library, _dt);
    }
    std::erase_if(m_effects, [](const Effect& _e) { return _e.anim.IsFinished(); });
}

void EffectList::Submit(
    const AnimationLibrary& _library,
    SpriteBatch&            _sprites) const
{
    for (const Effect& effect: m_effects)
    {
        if (const AnimFrame* pFrame = effect.anim.GetFrameData(_library))
        {
            _sprites.Submit(effect.layer, 0, *pFrame, effect.position);
        }
    }
}

}   // namespace slug

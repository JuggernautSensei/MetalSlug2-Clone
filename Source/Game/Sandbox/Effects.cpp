#include "Game/Sandbox/Effects.h"

#include <algorithm>

#include "Base/Math.h"
#include "Base/Types.h"

namespace
{
void StepDebris_(
    Effect& _effect,
    float   _dt)
{
    _effect.ageSec += _dt;
    _effect.velocity.y += kDebrisGravity * _dt;
    _effect.position.x += _effect.velocity.x * _dt;
    _effect.position.y += _effect.velocity.y * _dt;
}
}   // namespace

void EffectList::Clear()
{
    m_effects.clear();
}

void EffectList::Spawn(
    const AnimationLibrary& _library,
    StringView              _clipName,
    VECTOR2                 _position,
    int                     _depth,
    bool                    _bFlipX)
{
    const AnimId id = _library.Find(_clipName);
    if (id.IsNull())
    {
        return;
    }

    Effect effect;
    effect.anim.Play(id, true);
    effect.position = _position;
    effect.depth    = _depth;
    effect.bFlipX   = _bFlipX;
    m_effects.push_back(effect);
}

void EffectList::SpawnDebris(
    const AnimationLibrary& _library,
    StringView              _clipName,
    VECTOR2                 _position,
    int                     _depth,
    VECTOR2                 _velocity,
    bool                    _bFlipX)
{
    const size_t before = m_effects.size();
    Spawn(_library, _clipName, _position, _depth, _bFlipX);
    if (m_effects.size() > before)
    {
        Effect& effect  = m_effects.back();
        effect.bDebris  = true;
        effect.velocity = _velocity;
    }
}

void EffectList::Update(
    const AnimationLibrary& _library,
    float                   _dt,
    float                   _cullBelowY)
{
    for (Effect& effect: m_effects)
    {
        if (effect.bDebris)
        {
            StepDebris_(effect, _dt);
        }
        effect.anim.Update(_library, _dt);
    }
    const float cullY = _cullBelowY + kDebrisCullBelowMargin;
    std::erase_if(m_effects, [cullY](const Effect& _e) {
        if (_e.bDebris)
        {
            return _e.ageSec >= kDebrisLifeSec || _e.position.y > cullY;
        }
        return _e.anim.IsFinished();
    });
}

void EffectList::Submit(
    const AnimationLibrary& _library,
    SpriteBatch&            _sprites) const
{
    for (const Effect& effect: m_effects)
    {
        if (const AnimFrame* pFrame = effect.anim.GetFrameData(_library))
        {
            SpriteDrawOptions options;
            options.bFlipX = effect.bFlipX;
            _sprites.Submit(effect.depth, 0, *pFrame, effect.position, options);
        }
    }
}

size_t EffectList::GetCount() const
{
    return m_effects.size();
}

Span<const Effect> EffectList::GetEffects() const
{
    return m_effects;
}

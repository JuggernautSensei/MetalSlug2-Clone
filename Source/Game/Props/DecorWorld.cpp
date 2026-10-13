#include "Game/Props/DecorWorld.h"

#include <algorithm>

// 끝나지 않는 구간
constexpr float kDecorForeverSec = 1e9f;
// 구간 최소 길이
constexpr float kDecorMinSec = 0.01f;
// 한 틱에서 넘길 수 있는 구간 수 상한
constexpr int kDecorMaxAdvance = 64;

DecorSpawn MakeDecorSpawn(
    const StringView  _clip,
    const eStageLayer _layer,
    const VECTOR2     _pos,
    const ParamValue* _pParams)
{
    DecorSpawn spawn;
    spawn.clip  = String { _clip };
    spawn.layer = _layer;
    spawn.pos   = _pos;

    if (_pParams == nullptr)
    {
        return spawn;
    }

    if (const ParamValue* pVelocity = _pParams->Find("velocity"))
    {
        (void)pVelocity->TryGetVec2(spawn.velocity);
    }
    spawn.bFlipX = _pParams->GetBool("flip", false);
    spawn.bLoop  = _pParams->GetBool("loop", false);
    if (const ParamValue* pScale = _pParams->Find("scale"))
    {
        const float scale = pScale->AsFloat(1.f);
        spawn.scale       = scale > 0.f ? scale : 1.f;
    }

    if (const ParamValue* pSequence = _pParams->Find("sequence"); pSequence && pSequence->IsArray())
    {
        for (size_t i = 0; i < pSequence->GetSize(); ++i)
        {
            const ParamValue& item = pSequence->At(i);
            if (!item.IsObject())
            {
                continue;
            }
            DecorStep step;
            step.clip = item.GetString("clip");
            if (step.clip.empty())
            {
                continue;
            }
            if (const ParamValue* pSeconds = item.Find("seconds"))
            {
                step.seconds = Max(pSeconds->AsFloat(1.f), kDecorMinSec);
            }
            if (const ParamValue* pStepVelocity = item.Find("velocity"))
            {
                (void)pStepVelocity->TryGetVec2(step.velocity);
            }
            step.bFlipX = item.GetBool("flip", false);
            spawn.sequence.push_back(step);
        }
    }
    return spawn;
}

uint32_t DecorWorld::Spawn(
    const DecorSpawn& _spawn)
{
    Instance inst;
    inst.id    = m_nextId++;
    inst.layer = _spawn.layer;
    inst.pos   = _spawn.pos;
    inst.z     = _spawn.z;
    inst.bLoop = _spawn.bLoop;
    inst.scale = _spawn.scale;

    if (_spawn.sequence.empty())
    {
        DecorStep step;
        step.clip     = _spawn.clip;
        step.seconds  = kDecorForeverSec;
        step.velocity = _spawn.velocity;
        step.bFlipX   = _spawn.bFlipX;
        inst.steps.push_back(step);
    }
    else
    {
        inst.steps.assign(_spawn.sequence.begin(), _spawn.sequence.end());
    }

    m_items.push_back(inst);
    return inst.id;
}

void DecorWorld::EnterStep_(
    Instance&               _inst,
    const AnimationLibrary& _anims) const
{
    const DecorStep& step = _inst.steps[static_cast<size_t>(_inst.stepIndex)];
    _inst.velocity        = step.velocity;
    _inst.bFlipX          = step.bFlipX;
    _inst.stepLeft += step.seconds;
    _inst.bNeedEnter = false;

    const AnimId clip = _anims.Find(step.clip);
    if (_anims.IsValid(clip))
    {
        _inst.anim.Play(clip, true);
    }
}

void DecorWorld::Advance_(
    Instance&               _inst,
    const AnimationLibrary& _anims) const
{
    int next = _inst.stepIndex + 1;
    if (next >= static_cast<int>(_inst.steps.size()))
    {
        if (!_inst.bLoop)
        {
            _inst.bFinished = true;
            _inst.velocity  = { 0.f, 0.f };
            _inst.stepLeft  = kDecorForeverSec;
            return;
        }
        next = 0;
    }
    _inst.stepIndex = next;
    EnterStep_(_inst, _anims);
}

void DecorWorld::Tick(
    const AnimationLibrary& _anims,
    const float             _dt)
{
    for (Instance& inst: m_items)
    {
        if (inst.bNeedEnter)
        {
            EnterStep_(inst, _anims);
        }
        if (!inst.bFinished)
        {
            inst.stepLeft -= _dt;
            for (int guard = 0; !inst.bFinished && inst.stepLeft <= 0.f && guard < kDecorMaxAdvance; ++guard)
            {
                Advance_(inst, _anims);
            }
        }
        inst.pos.x += inst.velocity.x * _dt;
        inst.pos.y += inst.velocity.y * _dt;
        if (_anims.IsValid(inst.anim.GetClip()))
        {
            inst.anim.Update(_anims, _dt);
        }
    }
}

void DecorWorld::Submit(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims) const
{
    for (const Instance& inst: m_items)
    {
        if (!_anims.IsValid(inst.anim.GetClip()))
        {
            continue;
        }
        const AnimFrame* pFrame = inst.anim.GetFrameData(_anims);
        if (pFrame == nullptr || pFrame->IsEmpty())
        {
            continue;
        }
        const int         depth = ResolveZ(inst.z, inst.layer == eStageLayer::FarBackground ? kDecorFarDepth : kDecorBackDepth);
        SpriteDrawOptions options;
        options.bFlipX = inst.bFlipX;
        options.scale  = inst.scale;
        _sprites.Submit(depth, static_cast<int>(inst.pos.y), *pFrame, inst.pos, options);
    }
}

bool DecorWorld::SetZ(
    const uint32_t _id,
    const int      _z)
{
    for (Instance& inst: m_items)
    {
        if (inst.id == _id)
        {
            inst.z = _z;
            return true;
        }
    }
    return false;
}

void DecorWorld::ClearByLayerMask(
    const uint32_t _layerMask)
{
    std::erase_if(m_items, [_layerMask](const Instance& _inst) { return (_layerMask & LayerBit(_inst.layer)) != 0; });
}

void DecorWorld::Clear()
{
    m_items.clear();
    m_nextId = 1;
}

bool DecorWorld::Get(
    const uint32_t _id,
    VECTOR2&       _outPos,
    VECTOR2&       _outVelocity,
    StringView&    _outClip,
    bool&          _bOutFinished) const
{
    for (const Instance& inst: m_items)
    {
        if (inst.id != _id)
        {
            continue;
        }
        _outPos       = inst.pos;
        _outVelocity  = inst.velocity;
        _outClip      = inst.steps[static_cast<size_t>(inst.stepIndex)].clip;
        _bOutFinished = inst.bFinished;
        return true;
    }
    return false;
}

size_t DecorWorld::GetCount() const
{
    return m_items.size();
}

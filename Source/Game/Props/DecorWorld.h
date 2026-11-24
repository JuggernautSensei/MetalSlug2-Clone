#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Game/Stage/StageData.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

// 장식 레이어 정렬 번호
inline constexpr int kDecorFarDepth  = 4;
inline constexpr int kDecorBackDepth = 11;

struct DecorStep
{
    String  clip     = {};
    float   seconds  = 1.f;   // 구간 길이
    VECTOR2 velocity = { 0.f, 0.f };
    bool    bFlipX   = false;
};

struct DecorSpawn
{
    String            clip     = {};
    eStageLayer       layer    = eStageLayer::Background;
    VECTOR2           pos      = { 0.f, 0.f };
    VECTOR2           velocity = { 0.f, 0.f };
    bool              bFlipX   = false;
    bool              bLoop    = false;
    float             scale    = 1.f;
    int               z        = kDefaultZ;
    Vector<DecorStep> sequence = {};
};

// anim_decor 의 params 를 DecorSpawn 으로 읽는다
[[nodiscard]] DecorSpawn MakeDecorSpawn(StringView _clip, eStageLayer _layer, VECTOR2 _pos, const ParamValue* _pParams);

class DecorWorld
{
    struct Instance
    {
        uint32_t          id         = 0;
        eStageLayer       layer      = eStageLayer::Background;
        VECTOR2           pos        = { 0.f, 0.f };
        int               z          = kDefaultZ;
        VECTOR2           velocity   = { 0.f, 0.f };
        bool              bFlipX     = false;
        bool              bLoop      = false;
        bool              bFinished  = false;
        bool              bNeedEnter = true;
        float             scale      = 1.f;
        int               stepIndex  = 0;
        float             stepLeft   = 0.f;
        Vector<DecorStep> steps      = {};
        AnimPlayer        anim       = {};
    };

public:
    uint32_t             Spawn(const DecorSpawn& _spawn);
    bool                 SetZ(uint32_t _id, int _z);
    void                 Tick(const AnimationLibrary& _anims, float _dt);
    void                 Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims) const;
    void                 ClearByLayerMask(uint32_t _layerMask);
    void                 Clear();
    [[nodiscard]] size_t GetCount() const;
    [[nodiscard]] bool   Get(uint32_t _id, VECTOR2& _outPos, VECTOR2& _outVelocity, StringView& _outClip, bool& _bOutFinished) const;

private:
    void EnterStep_(Instance& _inst, const AnimationLibrary& _anims) const;
    void Advance_(Instance& _inst, const AnimationLibrary& _anims) const;

    Vector<Instance> m_items  = {};
    uint32_t         m_nextId = 1;
};

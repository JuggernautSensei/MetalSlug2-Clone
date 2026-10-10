#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Stage/StageData.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"

namespace slug
{

// 장식 레이어 정렬 번호 (소품 background 12 보다 뒤에 그려진다)
inline constexpr int kDecorFarLayer  = 4;    // farBackground (원경: 비행기 등)
inline constexpr int kDecorBackLayer = 11;   // background (지상 보행자 등, 플레이어/적 아래)

// 장식 연출 한 구간. 구간 동안 clip 을 재생하며 velocity 로 움직인다.
struct DecorStep
{
    String clip     = {};
    float  seconds  = 1.f;            // 구간 길이 (초)
    Vec2   velocity = { 0.f, 0.f };   // 픽셀/초
    bool   bFlipX   = false;
};

// 장식 한 개 생성 요청. sequence 가 비면 clip 하나를 velocity 로 계속 움직인다.
struct DecorSpawn
{
    String            clip     = {};
    eStageLayer       layer    = eStageLayer::Background;
    Vec2              pos      = { 0.f, 0.f };   // 엔티티 원점 (애니 pivot 기준)
    Vec2              velocity = { 0.f, 0.f };
    bool              bFlipX   = false;
    bool              bLoop    = false;   // sequence 끝나면 처음부터 반복
    Vector<DecorStep> sequence = {};
};

// anim_decor 의 params 를 DecorSpawn 으로 읽는다.
//  velocity: [vx, vy]
//  sequence: [{ "clip", "seconds", "velocity": [vx, vy], "flip" }, ...]
//  loop: bool (sequence 반복)
//  flip: bool (단일 clip 좌우 반전)
[[nodiscard]] DecorSpawn MakeDecorSpawn(
    StringView        _clip,
    eStageLayer       _layer,
    Vec2              _pos,
    const ParamValue* _pParams);

// 적/소품과 무관한 시각 전용 장식. 피격/판정이 없다.
class DecorWorld
{
    struct Instance
    {
        uint32_t          id         = 0;
        eStageLayer       layer      = eStageLayer::Background;
        Vec2              pos        = { 0.f, 0.f };
        Vec2              velocity   = { 0.f, 0.f };
        bool              bFlipX     = false;
        bool              bLoop      = false;
        bool              bFinished  = false;
        bool              bNeedEnter = true;   // 첫 구간 진입 전 (클립 조회 필요)
        int               stepIndex  = 0;
        float             stepLeft   = 0.f;   // 현재 구간 남은 초
        Vector<DecorStep> steps      = {};
        AnimPlayer        anim       = {};
    };

public:
    uint32_t Spawn(const DecorSpawn& _spawn);

    // _anims 는 클립 조회에 쓰고, 재생 시간은 _dt 로 진행한다.
    void Tick(const AnimationLibrary& _anims, float _dt);
    void Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims) const;

    void ClearByLayerMask(uint32_t _layerMask);
    void Clear();

    [[nodiscard]] size_t GetCount() const;

    // 테스트용 조회
    [[nodiscard]] bool Get(
        uint32_t    _id,
        Vec2&       _outPos,
        Vec2&       _outVelocity,
        StringView& _outClip,
        bool&       _bOutFinished) const;

private:
    void EnterStep_(Instance& _inst, const AnimationLibrary& _anims) const;
    void Advance_(Instance& _inst, const AnimationLibrary& _anims) const;

    Vector<Instance> m_items  = {};
    uint32_t         m_nextId = 1;
};

}   // namespace slug

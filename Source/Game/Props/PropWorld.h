#pragma once
#include <array>
#include <cstdint>

#include <JugX/Random.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Combat/CombatSystem.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropDef.h"
#include "Game/Sandbox/Effects.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Combat.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{

class BitmapFont;

// ===========================================
//  PropWorld
//   파괴 가능한 장면 소품 (상점/집/트럭/배/다리/탑/울타리/간판 낙하물) 과 그 아이템(ItemWorld) 을 소유한다.
//   적(EnemyWorld) 과 분리되어 있으며, 판정은 CombatSystem 의 Neutral 팀 Hurtbox 로만 한다 (owner id >= 5000).
//
//   스테이지 틱 순서 (호출자):
//     combat.BeginTick()
//     props.Tick(ctx)                  ← 타이머/낙하/연출/컬링. 이후 GetPlatforms()/GetWalls() 가 이번 틱 상태를 가리킨다
//     props.RegisterHurtboxes(combat)  ← 플레이어 Tick 전
//     player.Tick(physics = 지면 + props.GetPlatforms()/GetWalls() + 적 등)
//     enemies...; combat.Resolve()
//     props.OnHitEvents(events)        ← 피해 적용 (파괴/연출/드롭 큐)
//     items.Collect(player.GetBodyBox()) → ApplyItemPickup(player, pickup, tuning)
//     props.TakeEvents(), props.Submit(...)
//
//   값 타입 충돌 출력:
//     GetPlatforms()      원웨이 발판 (플레이어 + 적). 상점 발판, 탑/망루 발판, 가드 포스트.
//     GetEnemyPlatforms() 원웨이 발판 (적만). 집 지붕 (플레이어는 통과).
//     GetWalls()          고체 (플레이어 + 적). 트럭, 배, 다리, 블록, 미니하우스 벽.
//   플레이어/적 물리 세계에는 다른 발판/벽과 합쳐서 넘긴다 (스테이지가 조립).
// ===========================================

inline constexpr uint32_t kPropOwnerBase = 5000;
using PropHandle = uint32_t;
inline constexpr PropHandle kNoProp = 0;

inline constexpr int kPropLayerBack  = 12;   // 소품 (background)
inline constexpr int kPropLayerFront = 38;   // 소품 (front)
inline constexpr int kPropDebrisLayer = 39;  // 디브리
inline constexpr int kPropEffectLayer = 50;  // 폭발 연출 (EffectList 기본과 같음)

enum class ePropEventKind : uint8_t
{
    Damaged,       // 피해 적용됨 (value = 실제 깎인 양)
    Destroyed,     // HP 0 (파괴 시작)
    DropSpawned,   // 드롭 아이템 생성 (id = 아이템 id, name = 아이템/그룹 이름)
    GroundPatch,   // 픽셀맵 패치 요청 (name = 스프라이트, pos = 좌상단). 호스트가 ApplyGroundPatch 로 적용
    SecondaryBurst,// explodeOnDeath 2차 폭발 (이후 제거)
    Removed,       // 파괴 연출 종료/2차 폭발 후 제거
    Culled         // 카메라 뒤로 벗어나 제거
};

struct PropEvent
{
    ePropEventKind kind     = ePropEventKind::Damaged;
    PropHandle     id       = kNoProp;
    int            defIndex = -1;
    jug::VECTOR2   pos      = { 0.f, 0.f };
    int            value    = 0;
    jug::String    name     = {};
};

// 상태별 표시 슬롯 (PropInstance::visuals 인덱스)
enum class ePropVisual : uint8_t
{
    Idle,
    Damaged,
    Destroyed,
    Hit,
    Landed,
    Count
};

struct PropInstance
{
    PropHandle                 id                    = kNoProp;
    int                        defIndex              = -1;
    int                        variant               = -1;
    jug::VECTOR2               pos                   = { 0.f, 0.f };   // 스프라이트 중심 (legacyCenter, 변종 dy 반영)
    bool                       bFront                = false;
    float                      cullMargin            = 200.f;
    Health                     health                = {};
    std::array<PropVisual, static_cast<size_t>(ePropVisual::Count)> visuals = {};
    jug::Vector<PropPart>      parts                 = {};

    ePropVisual                visual                = ePropVisual::Idle;
    bool                       bVisualBound          = false;
    AnimPlayer                 anim                  = {};
    bool                       bHitPlaying           = false;
    float                      hitTimerSec           = 0.f;
    bool                       bDamagedBurstDone     = false;

    bool                       bDestroyed            = false;
    float                      destroyTimerSec       = 0.f;
    float                      destroyDurationSec    = 0.f;
    bool                       bRemoveAfterDestroy   = false;
    bool                       bExplodeOnDeath       = false;
    float                      secondaryTimerSec     = -1.f;   // >= 0: 2차 폭발 대기
    bool                       bRemoveAfterSecondary = false;

    bool                       bFalling              = false;   // 낙하체 (def.fall)
    float                      fallVy                = 0.f;
    float                      fallGravity           = 700.f;
    float                      fallHalfHeight        = 0.f;
    float                      landTimerSec          = -1.f;    // >= 0: 착지 후 경과 (afterLandSec 뒤 폭발)

    bool                       bRemove               = false;
};

// 아직 Tick 에서 재생되지 않은 폭발 (EffectList 가 AnimationLibrary 를 필요로 해서 Tick 까지 미룬다)
struct PropFxRequest
{
    jug::String  clip  = {};
    jug::VECTOR2 pos   = { 0.f, 0.f };
    int          layer = kPropEffectLayer;
};

// 파괴 디브리 조각 (중력 1000, 수명 5 s, 충돌 없음)
struct PropDebrisPiece
{
    jug::VECTOR2 pos    = { 0.f, 0.f };
    jug::VECTOR2 vel    = { 0.f, 0.f };
    float        ageSec = 0.f;
    jug::String  clip   = {};
    AnimPlayer   anim   = {};
    bool         bBound = false;
};

struct PropTickContext
{
    const GroundMap*        pGround = nullptr;   // 낙하체 착지/아이템 물리 (null 이면 지면 없음)
    const AnimationLibrary* pAnims  = nullptr;   // 클립 바인딩/재생 (null 이면 연출 시각 진행 없음)
    float                   cameraLeft = -1.0e9f;   // 이 x 에서 cullMargin 만큼 더 왼쪽 객체는 제거
    float                   dt         = kStepSec;
};

class PropWorld
{
public:
    explicit PropWorld(const PropDefLibrary* _pLibrary = nullptr, uint64_t _seed = 1);

    void SetLibrary(const PropDefLibrary* _pLibrary);
    [[nodiscard]] const PropDefLibrary* GetLibrary() const { return m_pLibrary; }

    // 소품/아이템/연출을 모두 지우고 RNG 를 다시 시드한다. ID 는 기본값으로 돌아간다.
    void Reset(uint64_t _seed = 1);

    // ===========================================
    //  스폰
    // ===========================================

    // _center = 스프라이트 중심 (legacyCenter). 알 수 없는 def 면 kNoProp.
    PropHandle Spawn(jug::StringView _defName, jug::VECTOR2 _center, const PropSpawnParams& _params = {});
    PropHandle SpawnByIndex(int _defIndex, jug::VECTOR2 _center, const PropSpawnParams& _params = {});

    // 아이템 (stage 의 item 스폰 / 드롭). _feet = 발(아이템 원점). 반환 0 = 알 수 없는 이름
    uint32_t SpawnItem(jug::StringView _name, jug::VECTOR2 _feet, bool _bBounce = false);

    // ===========================================
    //  틱 / 판정
    // ===========================================

    void Tick(const PropTickContext& _ctx);
    void RegisterHurtboxes(CombatSystem& _combat) const;
    void OnHitEvents(jug::Span<const HitEvent> _events);

    // ===========================================
    //  정적 충돌 출력 (마지막 Tick/OnHitEvents 기준)
    // ===========================================

    [[nodiscard]] jug::Span<const Platform> GetPlatforms() const      { return m_platforms; }
    [[nodiscard]] jug::Span<const Platform> GetEnemyPlatforms() const { return m_enemyPlatforms; }
    [[nodiscard]] jug::Span<const Wall>     GetWalls() const          { return m_walls; }

    // ===========================================
    //  조회
    // ===========================================

    // 없는 핸들(제거된 것 포함)은 "파괴됨/연출 끝" 으로 본다 (게이트 판정용)
    [[nodiscard]] bool IsDestroyed(PropHandle _handle) const;
    [[nodiscard]] bool IsDestroyFinished(PropHandle _handle) const;
    [[nodiscard]] bool IsAlive(PropHandle _handle) const;
    [[nodiscard]] int  GetHp(PropHandle _handle) const;
    [[nodiscard]] const PropInstance* Get(PropHandle _handle) const;
    [[nodiscard]] jug::Span<const PropInstance> GetProps() const { return m_props; }
    [[nodiscard]] size_t GetCount() const { return m_props.size(); }
    [[nodiscard]] jug::Span<const PropFxRequest>    GetPendingFx() const { return m_pendingFx; }
    [[nodiscard]] jug::Span<const PropDebrisPiece>  GetDebris() const { return m_debris; }

    // 이벤트를 비우며 가져간다
    [[nodiscard]] jug::Vector<PropEvent> TakeEvents();

    // 아이템 (습득/조회)
    [[nodiscard]] ItemWorld&       GetItems()       { return m_items; }
    [[nodiscard]] const ItemWorld& GetItems() const { return m_items; }

    // ===========================================
    //  정리 / 렌더 / 결정성
    // ===========================================

    // 한쪽 레이어(_bFront: front 소품 / false: background 소품)를 모두 제거한다 (이벤트 없음)
    void ClearByLayer(bool _bFront);
    void ClearAll();

    // 소품 + 디브리 + 연출 + 아이템을 그린다. _pFont 가 null 이면 점수 팝업 생략
    void Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont = nullptr);

    [[nodiscard]] uint64_t HashState() const;

private:
    struct PendingChild
    {
        int          defIndex = -1;
        jug::VECTOR2 pos      = { 0.f, 0.f };
    };

    [[nodiscard]] size_t FindIndex_(PropHandle _id) const;   // 없으면 size()
    [[nodiscard]] float  RollFloat_(float _lo, float _hi);
    [[nodiscard]] int    RollInt_(int _n);

    void Destroy_(PropInstance& _inst, const PropDef& _def, jug::VECTOR2 _center);
    void Detonate_(const PropBurst& _burst, jug::VECTOR2 _center);
    void QueueDrops_(const PropDef& _def, jug::VECTOR2 _center);
    void StartHit_(PropInstance& _inst, const PropDef& _def);
    void UpdateVisual_(PropInstance& _inst, const AnimationLibrary* _pAnims, float _dt);
    [[nodiscard]] ePropVisual ComputeVisual_(const PropInstance& _inst) const;
    void RebuildCollision_();
    [[nodiscard]] static bool PartActive_(const PropPart& _part, const PropInstance& _inst);

    const PropDefLibrary* m_pLibrary = nullptr;
    jug::Vector<PropInstance>      m_props          = {};
    jug::Vector<PropEvent>         m_events         = {};
    jug::Vector<PropFxRequest>     m_pendingFx      = {};
    jug::Vector<PropDebrisPiece>   m_debris         = {};
    jug::Vector<PendingChild>      m_pendingChildren = {};
    jug::Vector<Platform>          m_platforms      = {};
    jug::Vector<Platform>          m_enemyPlatforms = {};
    jug::Vector<Wall>              m_walls          = {};
    ItemWorld                      m_items          { nullptr, uint64_t { 1 } };
    EffectList                     m_effects        = {};
    jug::RngMt19937                m_rng            = jug::RngMt19937 { uint64_t { 1 } };
    uint32_t                       m_nextId         = kPropOwnerBase;
};

// 픽셀맵 패치 적용 (GroundPatch 이벤트를 받은 호스트가 호출). patchMask 의 고체 픽셀을 map 의 같은 위치(좌상단 _topLeft) 에 쓴다.
// 맵 밖은 무시한다. patchMask 가 공기인 칸도 map 을 공기로 만든다 (파괴 후 지형 교체).
void ApplyGroundPatch(GroundMap& _map, jug::VECTOR2 _topLeft, const GroundMap& _patchMask);

}   // namespace slug

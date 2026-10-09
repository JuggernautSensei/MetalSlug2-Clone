#pragma once
#include <cstdint>

#include <JugX/Random.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Core/Tuning.h"
#include "Game/Props/PropDef.h"
#include "Render/Animation.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"
#include "World/Geometry.h"
#include "World/Physics.h"

namespace slug
{

class PlayerController;
class BitmapFont;

// ===========================================
//  ItemWorld
//   바닥에 떨어지는 습득 아이템 (무기/보급/점수) 과 점수 팝업.
//   - 물리: Physics::Step (중력 1400, 지면/벽/원웨이 발판에 착지). 상자 32x32, 발 = 아이템 원점.
//   - 수명 15 s (70% = 10.5 s 부터 깜빡임), 화면 왼쪽 이탈 시 제거.
//   - 습득: Collect(플레이어 몸 상자) 가 픽업 목록을 돌려주고 ApplyItemPickup 이 플레이어에 반영한다.
//     (아이템 스스로는 플레이어를 바꾸지 않는다. 테스트/다른 호스트에서도 같은 규칙을 쓴다.)
//   - 결정성: 모든 난수는 생성자/Reset 의 시드 RNG 로만 뽑는다.
// ===========================================

inline constexpr uint32_t kItemIdBase        = 7000;
inline constexpr float    kItemLifetimeSec   = 15.f;
inline constexpr float    kItemBlinkStartSec = 10.5f;
inline constexpr float    kItemBlinkPeriod   = 0.2f;
inline constexpr float    kItemGravity       = 1400.f;   // px/s²
inline constexpr float    kItemPickupHideSec = 0.06f;
inline constexpr float    kScorePopupBlinkSec = 0.07f;
inline constexpr float    kScorePopupRiseDelaySec = 1.05f;
inline constexpr float    kScorePopupLifeSec = 2.8f;

// 한 번에 가져가는 습득 결과 (플레이어에 반영할 내용)
struct ItemPickup
{
    uint32_t     id     = 0;
    jug::String  name   = {};
    eItemKind    kind   = eItemKind::Score;
    eWeapon      weapon = eWeapon::Pistol;
    eSupplyKind  supply = eSupplyKind::Bomb;
    int          amount = 0;   // 폭탄 개수 (Bomb)
    int          score  = 0;
    jug::VECTOR2 pos    = { 0.f, 0.f };
};

// 습득/연출 요청 (PropWorld 가 Tick 에서 이펙트로 바꾼다)
struct ItemFxRequest
{
    jug::String  clip = {};
    jug::VECTOR2 pos  = { 0.f, 0.f };
};

struct PropItem
{
    uint32_t     id        = 0;
    int          defIndex  = -1;          // PropDefLibrary::GetItems() 인덱스
    jug::VECTOR2 feet      = { 0.f, 0.f };
    Body         body      = {};
    float        ageSec    = 0.f;
    bool         bBounce   = false;
    float        rollVx    = 0.f;         // 굴림 속도: 착지 후에만 적용 (떨어지는 동안은 수직으로 떨어진다)
    bool         bBegun    = false;       // begin 클립(Item_Mando_Begin) 재생 중
    bool         bPickedUp = false;       // 습득됨: pickupSec 동안 Disappear 연출 후 제거
    float        pickupSec = 0.f;         // 습득 후 남은 연출 시간 (bPickedUp 일 때만 의미)
    AnimPlayer   anim      = {};
    AnimPlayer   beginAnim = {};
    bool         bAnimBound = false;
};

struct ScorePopup
{
    jug::VECTOR2 pos     = { 0.f, 0.f };
    int          score   = 0;
    float        ageSec  = 0.f;
};

class ItemWorld
{
public:
    explicit ItemWorld(const PropDefLibrary* _pLibrary = nullptr, uint64_t _seed = 1);

    void SetLibrary(const PropDefLibrary* _pLibrary) { m_pLibrary = _pLibrary; }
    void Reset(uint64_t _seed = 1);

    // 이름은 아이템 id 또는 그룹(random_*) 이름. _feet = 발(아이템 원점). _bBounce: 공중에서 vy -400, 수평 ±75 (SuppleItem)
    // 알 수 없는 이름이면 0 을 돌려준다.
    uint32_t Spawn(jug::StringView _name, jug::VECTOR2 _feet, bool _bBounce = false);

    // _pAnims: 클립 바인딩/재생용 (없으면 애니 진행 없음). _cameraLeft: 이 왼쪽으로 벗어난 아이템은 제거
    void Tick(const PhysicsWorld& _world, float _dt, float _cameraLeft, const AnimationLibrary* _pAnims);

    // 플레이어 몸 상자와 겹친 아이템을 습득 처리한다. 반환: 이번 틱 픽업 목록 (ApplyItemPickup 으로 반영)
    [[nodiscard]] jug::Vector<ItemPickup> Collect(const Aabb& _playerBox);

    // 습득 이펙트 요청을 비우며 가져간다
    [[nodiscard]] jug::Vector<ItemFxRequest> TakeFx();

    // _pFont 가 null 이면 점수 팝업은 그리지 않는다
    void Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont) const;

    void ClearAll();

    [[nodiscard]] jug::Span<const PropItem>    GetItems() const { return m_items; }
    [[nodiscard]] jug::Span<const ScorePopup>  GetPopups() const { return m_popups; }
    [[nodiscard]] const PropItem*              Find(uint32_t _id) const;
    [[nodiscard]] size_t                       GetCount() const { return m_items.size(); }
    [[nodiscard]] uint64_t                     HashState() const;

private:
    void SpawnPopup_(jug::VECTOR2 _pos, int _score);
    void BindAnim_(PropItem& _item, const AnimationLibrary& _anims) const;

    const PropDefLibrary*          m_pLibrary = nullptr;
    jug::Vector<PropItem>          m_items    = {};
    jug::Vector<ScorePopup>        m_popups   = {};
    jug::Vector<ItemFxRequest>     m_fx       = {};
    jug::RngMt19937                m_rng      = jug::RngMt19937 { uint64_t { 1 } };
    uint32_t                       m_nextId   = kItemIdBase;
};

// 습득 결과를 플레이어에 반영한다.
//   Score  → AddScore
//   Gun    → PickUpWeapon (같은 무기면 탄 추가)
//   Supply → Bomb: AddBombs(amount). Bullet: 지금 든 무기가 유한 탄이면 PickUpWeapon(그 무기)
void ApplyItemPickup(PlayerController& _player, const ItemPickup& _pickup, const Tuning& _tuning);

}   // namespace slug

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Ui/BitmapFont.h"

namespace slug
{

namespace
{

// JugX 의 실수 범위 Random 은 내부 오버로드 해석 문제로 쓰지 않는다. [_lo, _hi] 균등 난수 (결정적).
template<typename RngT>
float RandRange(RngT& _rng, const float _lo, const float _hi)
{
    const double unit = static_cast<double>(_rng.Generate()) / 18446744073709551615.0;
    return _lo + static_cast<float>(unit) * (_hi - _lo);
}

// 점수 팝업/아이템 렌더 레이어 (PropWorld 의 소품 레이어 사이)
constexpr int kItemRenderLayer  = 34;
constexpr int kPopupRenderLayer = 46;
constexpr float kSupplyBounceVy = -400.f;
constexpr float kSupplyBounceVx = 75.f;
constexpr float kRollMinVx      = -50.f;
constexpr float kRollMaxVx      = 49.f;
constexpr float kRollStopSpeed  = 1.f;

// 발 기준 원점에서 프레임을 바닥 정렬로 그릴 때의 원점 (스프라이트 하단 = 발)
jug::VECTOR2 BottomAlignedOrigin(const AnimFrame& _frame, const jug::VECTOR2 _feet)
{
    return { _feet.x, _feet.y - static_cast<float>(_frame.h - _frame.pivotY) };
}

}   // namespace

ItemWorld::ItemWorld(
    const PropDefLibrary* _pLibrary,
    const uint64_t        _seed)
    : m_pLibrary(_pLibrary)
    , m_rng(_seed)
{
}

void ItemWorld::Reset(
    const uint64_t _seed)
{
    m_items.clear();
    m_popups.clear();
    m_fx.clear();
    m_rng  = jug::RngMt19937 { _seed };
    m_nextId = kItemIdBase;
}

void ItemWorld::ClearAll()
{
    m_items.clear();
    m_popups.clear();
    m_fx.clear();
}

uint32_t ItemWorld::Spawn(
    const jug::StringView _name,
    const jug::VECTOR2    _feet,
    const bool            _bBounce)
{
    if (m_pLibrary == nullptr)
    {
        return 0;
    }

    // 그룹이면 균등 추첨 (스테이지 시드 RNG)
    jug::String itemName { _name };
    if (const PropItemGroup* pGroup = m_pLibrary->FindGroup(_name))
    {
        const int pick = static_cast<int>(m_rng.Generate() % static_cast<uint64_t>(pGroup->names.size()));
        itemName       = pGroup->names[static_cast<size_t>(pick)];
    }
    const int defIndex = m_pLibrary->FindItemIndex(itemName);
    if (defIndex < 0)
    {
        return 0;
    }
    const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(defIndex)];

    PropItem item;
    item.id       = m_nextId++;
    item.defIndex = defIndex;
    item.feet     = _feet;
    item.bBounce  = _bBounce;
    item.body.position = _feet;
    item.body.size     = { 32.f, 32.f };
    item.body.gravityScale = kItemGravity / PhysicsTuning {}.gravity;

    if (def.kind == eItemKind::Score && def.bRoll)
    {
        item.rollVx = RandRange(m_rng, kRollMinVx, kRollMaxVx);
    }
    if (_bBounce)
    {
        item.body.velocity.y = kSupplyBounceVy;
        item.body.velocity.x = (m_rng.Generate() & 1u) ? kSupplyBounceVx : -kSupplyBounceVx;
    }
    m_items.push_back(item);
    return item.id;
}

void ItemWorld::BindAnim_(
    PropItem&               _item,
    const AnimationLibrary& _anims) const
{
    const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(_item.defIndex)];
    _item.bAnimBound = true;
    if (def.begin.ref == eVisualRef::Anim)
    {
        _item.bBegun = true;
        _item.beginAnim.Play(_anims.Find(def.begin.name));
    }
    else if (def.visual.ref == eVisualRef::Anim)
    {
        _item.anim.Play(_anims.Find(def.visual.name));
    }
}

void ItemWorld::Tick(
    const PhysicsWorld&     _world,
    const float             _dt,
    const float             _cameraLeft,
    const AnimationLibrary* _pAnims)
{
    // 점수 팝업
    for (ScorePopup& popup: m_popups)
    {
        popup.ageSec += _dt;
    }
    m_popups.erase(std::remove_if(m_popups.begin(), m_popups.end(), [](const ScorePopup& _p) { return _p.ageSec >= kScorePopupLifeSec; }), m_popups.end());

    for (PropItem& item: m_items)
    {
        item.ageSec += _dt;
        if (_pAnims != nullptr)
        {
            if (!item.bAnimBound)
            {
                BindAnim_(item, *_pAnims);
            }
            if (item.bBegun)
            {
                item.beginAnim.Update(*_pAnims, _dt);
                if (item.beginAnim.IsFinished())
                {
                    item.bBegun = false;
                    const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];
                    if (def.visual.ref == eVisualRef::Anim)
                    {
                        item.anim.Play(_pAnims->Find(def.visual.name));
                    }
                }
            }
            else
            {
                item.anim.Update(*_pAnims, _dt);
            }
        }
        if (item.bPickedUp)
        {
            item.pickupSec -= _dt;
            continue;   // 습득 연출 중에는 물리를 멈춘다
        }

        const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];
        if (def.kind == eItemKind::Score && def.bRoll && item.body.bGrounded)
        {
            if (item.rollVx != 0.f)
            {
                item.body.velocity.x = item.rollVx;   // 착지하는 순간 굴러간다
                item.rollVx          = 0.f;
            }
            // 지면에서 v -= v*dt 로 감속 (정지 임계 이하이면 0)
            item.body.velocity.x -= item.body.velocity.x * _dt;
            if (std::abs(item.body.velocity.x) < kRollStopSpeed)
            {
                item.body.velocity.x = 0.f;
            }
        }
        if (item.bBounce && item.body.bGrounded)
        {
            item.body.velocity.x = 0.f;   // 착지 후 수평 튐 종료
        }
        (void)Physics::Step(item.body, _world, PhysicsTuning {}, _dt);
        item.feet = item.body.position;
    }

    m_items.erase(
        std::remove_if(
            m_items.begin(),
            m_items.end(),
            [&](const PropItem& _item)
            {
                if (_item.bPickedUp)
                {
                    return _item.pickupSec <= 0.f;
                }
                return _item.ageSec >= kItemLifetimeSec || _item.feet.x < _cameraLeft - 50.f;
            }),
        m_items.end());
}

void ItemWorld::SpawnPopup_(
    const jug::VECTOR2 _pos,
    const int          _score)
{
    ScorePopup popup;
    popup.pos   = _pos;
    popup.score = _score;
    m_popups.push_back(popup);
}

jug::Vector<ItemPickup> ItemWorld::Collect(
    const Aabb& _playerBox)
{
    jug::Vector<ItemPickup> out;
    if (m_pLibrary == nullptr)
    {
        return out;
    }
    for (PropItem& item: m_items)
    {
        if (item.bPickedUp)
        {
            continue;
        }
        const Aabb box = Aabb::FromBottomCenter(item.feet, { 32.f, 32.f });
        if (!_playerBox.Overlaps(box))
        {
            continue;
        }
        const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];

        ItemPickup pickup;
        pickup.id     = item.id;
        pickup.name   = def.name;
        pickup.kind   = def.kind;
        pickup.weapon = def.weapon;
        pickup.supply = def.supply;
        pickup.amount = def.amount;
        pickup.score  = def.score;
        pickup.pos    = item.feet;
        out.push_back(pickup);

        if (def.kind == eItemKind::Score)
        {
            SpawnPopup_(item.feet, def.score);
            item.ageSec = kItemLifetimeSec;   // 다음 정리 때 제거
            continue;
        }
        // 무기/보급: 습득 연출(Disappear) 후 짧게 사라진다. Get_Item_Effect 는 PropWorld 가 재생한다.
        item.bPickedUp = true;
        item.pickupSec = kItemPickupHideSec;
        m_fx.push_back(ItemFxRequest { jug::String { "Get_Item_Effect" }, item.feet });
    }
    // 점수 아이템은 바로 제거 (수명 종료로 표시)
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [](const PropItem& _i) { return !_i.bPickedUp && _i.ageSec >= kItemLifetimeSec; }), m_items.end());
    return out;
}

jug::Vector<ItemFxRequest> ItemWorld::TakeFx()
{
    jug::Vector<ItemFxRequest> out = std::move(m_fx);
    m_fx.clear();
    return out;
}

const PropItem* ItemWorld::Find(
    const uint32_t _id) const
{
    for (const PropItem& item: m_items)
    {
        if (item.id == _id)
        {
            return &item;
        }
    }
    return nullptr;
}

void ItemWorld::Submit(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const SpriteLibrary&    _spriteLib,
    const BitmapFont*       _pFont) const
{
    if (m_pLibrary == nullptr)
    {
        return;
    }
    for (const PropItem& item: m_items)
    {
        const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];

        // 깜빡임: 수명 70% 이후 (습득 연출 중에는 항상 보임)
        if (!item.bPickedUp && item.ageSec >= kItemBlinkStartSec && (static_cast<int>(item.ageSec / kItemBlinkPeriod) % 2) != 0)
        {
            continue;
        }

        const PropVisual* pVisual = &def.visual;
        AnimFrame         frame   = {};
        if (item.bPickedUp)
        {
            pVisual = &def.pickup;
            if (pVisual->ref == eVisualRef::Sprite)
            {
                const SpriteInfo* pSprite = _spriteLib.Find(pVisual->name);
                if (pSprite != nullptr)
                {
                    frame = pSprite->frame;
                }
            }
            else if (pVisual->ref == eVisualRef::Anim)
            {
                const AnimId id = _anims.Find(pVisual->name);
                if (const AnimClip* pClip = _anims.GetOrNull(id))
                {
                    frame = pClip->frames.empty() ? AnimFrame {} : pClip->frames.front();
                }
            }
        }
        else if (item.bBegun && def.begin.ref == eVisualRef::Anim)
        {
            const AnimFrame* pFrame = item.beginAnim.GetFrameData(_anims);
            frame                   = pFrame ? *pFrame : AnimFrame {};
        }
        else if (pVisual->ref == eVisualRef::Sprite)
        {
            const SpriteInfo* pSprite = _spriteLib.Find(pVisual->name);
            if (pSprite != nullptr)
            {
                frame = pSprite->frame;
            }
        }
        else if (pVisual->ref == eVisualRef::Anim)
        {
            const AnimFrame* pFrame = item.anim.GetFrameData(_anims);
            frame                   = pFrame ? *pFrame : AnimFrame {};
        }
        if (frame.IsEmpty())
        {
            continue;
        }
        _sprites.Submit(kItemRenderLayer, static_cast<int>(item.feet.y), frame, BottomAlignedOrigin(frame, item.feet));
    }

    if (_pFont == nullptr)
    {
        return;
    }
    for (const ScorePopup& popup: m_popups)
    {
        const std::string text = std::to_string(popup.score);

        // 0.07 s 마다 빨강/노랑 교대. 1.05 s 뒤부터 가속 상승 (-500 px/s²)
        const int   phase  = static_cast<int>(popup.ageSec / kScorePopupBlinkSec);
        const eFontColor color = (phase % 2 == 0) ? eFontColor::Red : eFontColor::Yellow;
        float       rise   = 0.f;
        if (popup.ageSec > kScorePopupRiseDelaySec)
        {
            const float t = popup.ageSec - kScorePopupRiseDelaySec;
            rise          = 0.5f * 500.f * t * t;
        }

        // 자릿수 합계 폭으로 가운데 정렬
        jug::Vector<AnimFrame> glyphs;
        int                    width = 0;
        for (const char ch: text)
        {
            const int glyph = fontmap::GetGlyphIndex(ch);
            if (glyph < 0)
            {
                continue;
            }
            const AnimFrame frame = _pFont->GetGlyphFrame(color, false, glyph);
            glyphs.push_back(frame);
            width += frame.w;
        }
        int cursor = static_cast<int>(popup.pos.x) - width / 2;
        for (const AnimFrame& frame: glyphs)
        {
            const jug::VECTOR2 origin = { static_cast<float>(cursor + frame.pivotX), popup.pos.y - rise - 32.f + static_cast<float>(frame.pivotY) };
            _sprites.Submit(kPopupRenderLayer, static_cast<int>(popup.pos.y), frame, origin);
            cursor += frame.w;
        }
    }
}

uint64_t ItemWorld::HashState() const
{
    uint64_t h = 1469598103934665603ull;
    auto     mix = [&](uint64_t _v)
    {
        h ^= _v;
        h *= 1099511628211ull;
    };
    auto mixF = [&](float _f)
    {
        uint32_t bits = 0;
        static_assert(sizeof(bits) == sizeof(_f));
        std::memcpy(&bits, &_f, sizeof(bits));
        mix(bits);
    };
    for (const PropItem& item: m_items)
    {
        mix(item.id);
        mix(static_cast<uint64_t>(item.defIndex));
        mixF(item.feet.x);
        mixF(item.feet.y);
        mixF(item.body.velocity.x);
        mixF(item.body.velocity.y);
        mixF(item.ageSec);
        mix(item.bPickedUp ? 1u : 0u);
        mixF(item.pickupSec);
    }
    for (const ScorePopup& popup: m_popups)
    {
        mixF(popup.pos.x);
        mixF(popup.pos.y);
        mix(static_cast<uint64_t>(popup.score));
        mixF(popup.ageSec);
    }
    return h;
}

void ApplyItemPickup(
    PlayerController&   _player,
    const ItemPickup&   _pickup,
    const Tuning&       _tuning)
{
    switch (_pickup.kind)
    {
        case eItemKind::Score:
            _player.AddScore(_pickup.score);
            break;
        case eItemKind::Gun:
            _player.PickUpWeapon(_pickup.weapon, _tuning);
            break;
        case eItemKind::Supply:
            if (_pickup.supply == eSupplyKind::Bomb)
            {
                _player.AddBombs(_pickup.amount);
            }
            else
            {
                // 탄약 보급: 지금 든 무기가 유한 탄이면 그 무기의 탄을 채운다 (권총은 탄 개념 없음)
                const eWeapon current = _player.GetWeapon().weapon;
                if (current != eWeapon::Pistol)
                {
                    _player.PickUpWeapon(current, _tuning);
                }
            }
            break;
    }
}

}   // namespace slug

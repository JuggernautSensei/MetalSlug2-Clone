#include "Game/Props/ItemWorld.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Base/Types.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerWeapons.h"
#include "Game/Ui/BitmapFont.h"

namespace
{
void MixValue_(
    uint64_t&      _hash,
    const uint64_t _value)
{
    _hash ^= _value;
    _hash *= 1'099'511'628'211ull;
}

void MixFloat_(
    uint64_t&   _hash,
    const float _value)
{
    const uint32_t bits = std::bit_cast<uint32_t>(_value);
    MixValue_(_hash, bits);
}
}   // namespace

constexpr float kSupplyBounceVy = -400.f;
constexpr float kRollMinVx      = -50.f;
constexpr float kRollMaxVx      = 49.f;

namespace
{
[[nodiscard]] VECTOR2 BottomAlignedOrigin_(
    const AnimFrame& _frame,
    const VECTOR2    _feet)
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
    JUG_ASSERT(_pLibrary != nullptr, "ItemWorld 는 정의 라이브러리 없이 만들 수 없다.\n");
}

void ItemWorld::Reset(
    const uint64_t _seed)
{
    m_items.clear();
    m_popups.clear();
    m_fx.clear();
    m_rng    = RngMt19937 { _seed };
    m_nextId = kItemIdBase;
}

void ItemWorld::ClearAll()
{
    m_items.clear();
    m_popups.clear();
    m_fx.clear();
}

uint32_t ItemWorld::Spawn(
    const StringView _name,
    const VECTOR2    _feet,
    const bool       _bBounce,
    const int        _z)
{
    String itemName { _name };
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
    item.id                = m_nextId++;
    item.defIndex          = defIndex;
    item.feet              = _feet;
    item.z                 = _z;
    item.bBounce           = _bBounce;
    item.body.position     = _feet;
    item.body.size         = { 32.f, 32.f };
    item.body.gravityScale = kItemGravity / PhysicsTuning {}.gravity;

    if (def.kind == eItemKind::Score && def.bRoll)
    {
        item.rollVx = kRollMinVx + static_cast<float>(Random<double>(m_rng)) * (kRollMaxVx - kRollMinVx);
    }
    if (_bBounce)
    {
        // 수직으로만 튀어 오른다
        (void)m_rng.Generate();
        item.body.velocity.y = kSupplyBounceVy;
        item.body.velocity.x = 0.f;
    }
    m_items.push_back(item);
    return item.id;
}

void ItemWorld::BindAnim_(
    PropItem&               _item,
    const AnimationLibrary& _anims) const
{
    const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(_item.defIndex)];
    _item.bAnimBound       = true;
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
    const AnimationLibrary* _pAnims,
    const ItemTuning*       _pTuning)
{
    const ItemTuning tuning = _pTuning != nullptr ? *_pTuning : ItemTuning {};
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
                    item.bBegun            = false;
                    const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];
                    if (def.visual.ref == eVisualRef::Anim)
                    {
                        item.anim.Play(_pAnims->Find(def.visual.name));
                    }
                }
            }
            else
            {
                item.anim.Update(*_pAnims, _dt * item.spinScale);
            }
        }
        if (item.bPickedUp)
        {
            item.pickupSec -= _dt;
            continue;
        }

        const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];
        if (def.kind == eItemKind::Score && def.bRoll && item.body.bGrounded)
        {
            if (item.rollVx != 0.f)
            {
                item.body.velocity.x = item.rollVx;
                item.rollVx          = 0.f;
            }
            item.body.velocity.x -= item.body.velocity.x * tuning.rollFrictionPerSec * _dt;
            if (Abs(item.body.velocity.x) < tuning.rollStopSpeed)
            {
                item.body.velocity.x = 0.f;
            }
        }
        if (item.bBounce && item.body.bGrounded)
        {
            item.body.velocity.x = 0.f;
        }
        (void)Physics::Step(item.body, _world, PhysicsTuning {}, _dt);
        item.feet = item.body.position;

        if (def.kind == eItemKind::Score && def.bRoll)
        {
            // 공중에서는 일정하게 돈다
            const float speedX = item.rollVx != 0.f ? item.rollVx : item.body.velocity.x;
            item.spinScale     = item.body.bGrounded
                                   ? Clamp(Abs(speedX) / tuning.rollSpinRefSpeed, 0.f, 1.f)
                                   : 1.f;
        }
    }

    m_items.erase(
        std::remove_if(
            m_items.begin(),
            m_items.end(),
            [&](const PropItem& _item) {
                if (_item.bPickedUp)
                {
                    return _item.pickupSec <= 0.f;
                }
                return _item.ageSec >= kItemLifetimeSec || _item.feet.x < _cameraLeft - 50.f;
            }),
        m_items.end());
}

void ItemWorld::SpawnPopup_(
    const VECTOR2 _pos,
    const int     _score,
    const int     _z)
{
    ScorePopup popup;
    popup.pos   = _pos;
    popup.score = _score;
    popup.z     = _z;
    m_popups.push_back(popup);
}

Vector<ItemPickup> ItemWorld::Collect(
    const Aabb& _playerBox)
{
    Vector<ItemPickup> out;
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
            SpawnPopup_(item.feet, def.score, ZShift(item.z, kItemRenderDepth, kPopupRenderDepth));
            item.ageSec = kItemLifetimeSec;
            continue;
        }
        item.bPickedUp = true;
        item.pickupSec = kItemPickupHideSec;
        m_fx.push_back(ItemFxRequest { String { "Get_Item_Effect" }, item.feet, ZDelta(item.z, kItemRenderDepth) });
    }
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [](const PropItem& _i) { return !_i.bPickedUp && _i.ageSec >= kItemLifetimeSec; }), m_items.end());
    return out;
}

bool ItemWorld::SetZ(
    const uint32_t _id,
    const int      _z)
{
    for (PropItem& item: m_items)
    {
        if (item.id == _id)
        {
            item.z = _z;
            return true;
        }
    }
    return false;
}

Vector<ItemFxRequest> ItemWorld::TakeFx()
{
    Vector<ItemFxRequest> out = std::move(m_fx);
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
    for (const PropItem& item: m_items)
    {
        const PropItemDef& def = m_pLibrary->GetItems()[static_cast<size_t>(item.defIndex)];

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
        _sprites.Submit(ResolveZ(item.z, kItemRenderDepth), static_cast<int>(item.feet.y), frame, BottomAlignedOrigin_(frame, item.feet));
    }

    if (_pFont == nullptr)
    {
        return;
    }
    for (const ScorePopup& popup: m_popups)
    {
        const String text = std::to_string(popup.score);

        const int        phase = static_cast<int>(popup.ageSec / kScorePopupBlinkSec);
        const eFontColor color = (phase % 2 == 0) ? eFontColor::Red : eFontColor::Yellow;
        float            rise  = 0.f;
        if (popup.ageSec > kScorePopupRiseDelaySec)
        {
            const float t = popup.ageSec - kScorePopupRiseDelaySec;
            rise          = 0.5f * 500.f * t * t;
        }

        Vector<AnimFrame> glyphs;
        int               width = 0;
        for (const char ch: text)
        {
            const int glyph = GetGlyphIndex(ch);
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
            const VECTOR2 origin = { static_cast<float>(cursor + frame.pivotX), popup.pos.y - rise - 32.f + static_cast<float>(frame.pivotY) };
            _sprites.Submit(ResolveZ(popup.z, kPopupRenderDepth), static_cast<int>(popup.pos.y), frame, origin);
            cursor += frame.w;
        }
    }
}

uint64_t ItemWorld::HashState() const
{
    uint64_t h = 1'469'598'103'934'665'603ull;
    for (const PropItem& item: m_items)
    {
        MixValue_(h, item.id);
        MixValue_(h, static_cast<uint64_t>(item.defIndex));
        MixFloat_(h, item.feet.x);
        MixFloat_(h, item.feet.y);
        MixFloat_(h, item.body.velocity.x);
        MixFloat_(h, item.body.velocity.y);
        MixFloat_(h, item.ageSec);
        MixValue_(h, item.bPickedUp ? 1u : 0u);
        MixFloat_(h, item.pickupSec);
    }
    for (const ScorePopup& popup: m_popups)
    {
        MixFloat_(h, popup.pos.x);
        MixFloat_(h, popup.pos.y);
        MixValue_(h, static_cast<uint64_t>(popup.score));
        MixFloat_(h, popup.ageSec);
    }
    return h;
}

void ItemWorld::SetLibrary(
    const PropDefLibrary* _pLibrary)
{
    JUG_ASSERT(_pLibrary != nullptr, "정의 라이브러리는 null 일 수 없다.\n");
    m_pLibrary = _pLibrary;
}

Span<const PropItem> ItemWorld::GetItems() const
{
    return m_items;
}

Span<const ScorePopup> ItemWorld::GetPopups() const
{
    return m_popups;
}

size_t ItemWorld::GetCount() const
{
    return m_items.size();
}

void ApplyItemPickup(
    PlayerController& _player,
    const ItemPickup& _pickup,
    const Tuning&     _tuning)
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
                const eWeapon current = _player.GetWeapon().weapon;
                if (current != eWeapon::Pistol)
                {
                    _player.PickUpWeapon(current, _tuning);
                }
            }
            break;
    }
}

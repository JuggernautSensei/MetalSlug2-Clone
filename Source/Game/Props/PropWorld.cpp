#include "Game/Props/PropWorld.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Ui/BitmapFont.h"

namespace
{
template<typename RngT>
[[nodiscard]] float RandRange_(
    RngT&       _rng,
    const float _lo,
    const float _hi)
{
    const double unit = static_cast<double>(_rng.Generate()) / 18'446'744'073'709'551'615.0;
    return _lo + static_cast<float>(unit) * (_hi - _lo);
}
}   // namespace

constexpr float kHitFlashSec = 0.5f;   // 피격 연출 최대 길이 (Hit 클립이 없으면 연출 없음)

constexpr size_t kVisualCount = static_cast<size_t>(ePropVisual::Count);

namespace
{
[[nodiscard]] uint64_t MixHash_(
    const uint64_t _h,
    const uint64_t _v)
{
    return (_h ^ _v) * 1'099'511'628'211ull;
}

[[nodiscard]] uint64_t MixFloat_(
    const uint64_t _h,
    const float    _f)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &_f, sizeof(bits));
    return MixHash_(_h, bits);
}
}   // namespace

PropWorld::PropWorld(
    const PropDefLibrary* _pLibrary,
    const uint64_t        _seed)
    : m_pLibrary(_pLibrary)
    , m_items(_pLibrary, _seed)
    , m_rng(_seed)
{
    JUG_ASSERT(_pLibrary != nullptr, "PropWorld 는 정의 라이브러리 없이 만들 수 없다.\n");
}

void PropWorld::SetLibrary(
    const PropDefLibrary* _pLibrary)
{
    JUG_ASSERT(_pLibrary != nullptr, "정의 라이브러리는 null 일 수 없다.\n");
    m_pLibrary = _pLibrary;
    m_items.SetLibrary(_pLibrary);
}

void PropWorld::Reset(
    const uint64_t _seed)
{
    ClearAll();
    m_rng    = RngMt19937 { _seed };
    m_nextId = kPropOwnerBase;
    m_items.Reset(_seed);
}

void PropWorld::ClearAll()
{
    m_props.clear();
    m_events.clear();
    m_pendingFx.clear();
    m_debris.clear();
    m_pendingChildren.clear();
    m_items.ClearAll();
    m_effects.Clear();
    m_decor.Clear();
    RebuildCollision_();
}

void PropWorld::ClearByLayer(
    const bool _bFront)
{
    m_props.erase(std::remove_if(m_props.begin(), m_props.end(), [&](const PropInstance& _p) { return _p.bFront == _bFront; }), m_props.end());
    RebuildCollision_();
}

float PropWorld::RollFloat_(
    const float _lo,
    const float _hi)
{
    return RandRange_(m_rng, _lo, _hi);
}

int PropWorld::RollInt_(
    const int _n)
{
    if (_n <= 1)
    {
        return 0;
    }
    return static_cast<int>(m_rng.Generate() % static_cast<uint64_t>(_n));
}

size_t PropWorld::FindIndex_(
    const PropHandle _id) const
{
    for (size_t i = 0; i < m_props.size(); ++i)
    {
        if (m_props[i].id == _id)
        {
            return i;
        }
    }
    return m_props.size();
}

const PropInstance* PropWorld::Get(
    const PropHandle _handle) const
{
    const size_t idx = FindIndex_(_handle);
    return idx < m_props.size() ? &m_props[idx] : nullptr;
}

bool PropWorld::IsDestroyed(
    const PropHandle _handle) const
{
    const PropInstance* pInst = Get(_handle);
    return pInst == nullptr || pInst->bDestroyed;
}

bool PropWorld::IsDestroyFinished(
    const PropHandle _handle) const
{
    const PropInstance* pInst = Get(_handle);
    return pInst == nullptr || (pInst->bDestroyed && pInst->destroyTimerSec >= pInst->destroyDurationSec);
}

bool PropWorld::IsAlive(
    const PropHandle _handle) const
{
    const PropInstance* pInst = Get(_handle);
    return pInst != nullptr && !pInst->bDestroyed;
}

int PropWorld::GetHp(
    const PropHandle _handle) const
{
    const PropInstance* pInst = Get(_handle);
    return pInst != nullptr ? pInst->health.hp : 0;
}

Vector<PropEvent> PropWorld::TakeEvents()
{
    Vector<PropEvent> out = std::move(m_events);
    m_events.clear();
    return out;
}

PropHandle PropWorld::Spawn(
    const StringView       _defName,
    const Vec2             _center,
    const PropSpawnParams& _params)
{
    return SpawnByIndex(m_pLibrary->FindIndex(_defName), _center, _params);
}

PropHandle PropWorld::SpawnByIndex(
    const int              _defIndex,
    const Vec2             _center,
    const PropSpawnParams& _params)
{
    if (_defIndex < 0 || static_cast<size_t>(_defIndex) >= m_pLibrary->GetDefs().size())
    {
        return kNoProp;
    }
    const PropDef& def = m_pLibrary->GetDefs()[static_cast<size_t>(_defIndex)];

    PropInstance inst;
    inst.id              = m_nextId++;
    inst.defIndex        = _defIndex;
    inst.pos             = _center;
    inst.depth           = _params.z != kDefaultZ ? _params.z : def.depth;
    inst.bFront          = def.depth >= kPropDepthFront;
    inst.cullMargin      = def.cullMargin;
    inst.sortOffsetY     = def.sortOffsetY;
    inst.bSortByX        = def.bSortByX;
    inst.bExplodeOnDeath = _params.bExplodeOnDeath;

    const PropVariant* pVar = nullptr;
    if (!def.variants.empty())
    {
        const int v  = _params.variant >= 0 ? _params.variant : RollInt_(static_cast<int>(def.variants.size()));
        pVar         = &def.variants[static_cast<size_t>(v)];
        inst.variant = v;
        inst.pos.y += pVar->dy;
    }

    ARRAY<PropVisual, kVisualCount>& vis             = inst.visuals;
    vis[static_cast<size_t>(ePropVisual::Idle)]      = (pVar && pVar->idle.ref != eVisualRef::None) ? pVar->idle : def.idle;
    vis[static_cast<size_t>(ePropVisual::Damaged)]   = def.damaged;
    vis[static_cast<size_t>(ePropVisual::Destroyed)] = def.destroyed;
    vis[static_cast<size_t>(ePropVisual::Hit)]       = (pVar && pVar->hit.ref != eVisualRef::None) ? pVar->hit : def.hit;
    vis[static_cast<size_t>(ePropVisual::Landed)]    = def.fall.landed;
    if (!_params.idleSprite.empty())
    {
        vis[static_cast<size_t>(ePropVisual::Idle)] = PropVisual { eVisualRef::Sprite, _params.idleSprite };
    }
    if (!_params.destroySprite.empty())
    {
        vis[static_cast<size_t>(ePropVisual::Destroyed)] = PropVisual { eVisualRef::Sprite, _params.destroySprite };
    }

    inst.parts = (pVar && !pVar->parts.empty()) ? pVar->parts : def.parts;
    if (_params.hitbox)
    {
        for (PropPart& part: inst.parts)
        {
            if (part.role == ePropRole::Hurt)
            {
                part.size   = _params.hitbox->size;
                part.offset = _params.hitbox->offset;
                break;
            }
        }
    }
    if (_params.standBox)
    {
        for (PropPart& part: inst.parts)
        {
            if (part.role == ePropRole::Platform)
            {
                part.size   = _params.standBox->size;
                part.offset = _params.standBox->offset;
                break;
            }
        }
    }

    inst.health             = Health::Make(_params.hp >= 0 ? _params.hp : def.hp);
    inst.bFalling           = def.bHasFall;
    inst.fallGravity        = def.fall.gravity;
    inst.fallHalfHeight     = def.fall.halfHeight;
    inst.destroyDurationSec = def.destroyDurationSec;

    m_props.push_back(std::move(inst));
    RebuildCollision_();
    return m_props.back().id;
}

uint32_t PropWorld::SpawnItem(
    const StringView _name,
    const Vec2       _feet,
    const bool       _bBounce,
    const int        _z)
{
    return m_items.Spawn(_name, _feet, _bBounce, _z);
}

bool PropWorld::PartActive_(
    const PropPart&     _part,
    const PropInstance& _inst)
{
    switch (_part.when)
    {
        case ePropWhen::Intact: return !_inst.bDestroyed;
        case ePropWhen::Destroyed: return _inst.bDestroyed;
        case ePropWhen::Always: return true;
    }
    return false;
}

void PropWorld::RegisterHurtboxes(
    CombatSystem& _combat) const
{
    const HitKindMask accepts = AcceptsFor(false);
    for (const PropInstance& inst: m_props)
    {
        if (inst.bRemove)
        {
            continue;
        }
        for (const PropPart& part: inst.parts)
        {
            if (part.role != ePropRole::Hurt || !PartActive_(part, inst))
            {
                continue;
            }
            Hurtbox hurtbox;
            hurtbox.box     = Aabb::FromCenterSize(inst.pos + part.offset, part.size);
            hurtbox.team    = eTeam::Neutral;
            hurtbox.owner   = inst.id;
            hurtbox.accepts = accepts;
            _combat.AddHurtbox(hurtbox);
        }
    }
}

void PropWorld::OnHitEvents(
    const Span<const HitEvent> _events)
{
    for (const HitEvent& ev: _events)
    {
        const size_t idx = FindIndex_(ev.target);
        if (idx >= m_props.size())
        {
            continue;
        }
        PropInstance& inst = m_props[idx];
        if (inst.bDestroyed || inst.bRemove)
        {
            continue;
        }
        const PropDef& def = m_pLibrary->GetDefs()[static_cast<size_t>(inst.defIndex)];

        const int applied = inst.health.ApplyDamage(ev.damage);
        if (applied <= 0)
        {
            continue;
        }
        PropEvent damaged;
        damaged.kind     = ePropEventKind::Damaged;
        damaged.id       = inst.id;
        damaged.defIndex = inst.defIndex;
        damaged.pos      = inst.pos;
        damaged.value    = applied;
        m_events.push_back(damaged);

        if (inst.visuals[static_cast<size_t>(ePropVisual::Hit)].ref != eVisualRef::None)
        {
            inst.bHitPlaying  = true;
            inst.hitTimerSec  = kHitFlashSec;
            inst.bVisualBound = false;
        }

        if (def.damagedAtHp >= 0 && !inst.bDamagedBurstDone && inst.health.hp > 0 && inst.health.hp <= def.damagedAtHp)
        {
            inst.bDamagedBurstDone = true;
            Detonate_(def.onDamaged, inst.pos);
        }
        if (inst.health.IsDead())
        {
            Destroy_(inst, def, inst.pos);
        }
    }

    for (const PendingChild& child: m_pendingChildren)
    {
        SpawnByIndex(child.defIndex, child.pos);
    }
    m_pendingChildren.clear();
    RebuildCollision_();
}

void PropWorld::Destroy_(
    PropInstance&  _inst,
    const PropDef& _def,
    const Vec2     _center)
{
    _inst.bDestroyed          = true;
    _inst.destroyTimerSec     = 0.f;
    _inst.bHitPlaying         = false;
    _inst.bVisualBound        = false;
    _inst.bRemoveAfterDestroy = _def.bRemoveOnDestroy;

    PropEvent destroyed;
    destroyed.kind     = ePropEventKind::Destroyed;
    destroyed.id       = _inst.id;
    destroyed.defIndex = _inst.defIndex;
    destroyed.pos      = _center;
    m_events.push_back(destroyed);
    EventLogFormat("prop.destroyed", "\"def\":\"{}\",\"x\":{:.1f},\"y\":{:.1f}", _def.name, _center.x, _center.y);

    Detonate_(_def.onDestroy, _center);
    QueueDrops_(_def, _center);

    for (const PropChild& child: _def.spawns)
    {
        const int childIndex = m_pLibrary->FindIndex(child.def);
        if (childIndex >= 0)
        {
            m_pendingChildren.push_back(PendingChild { childIndex, _center + child.offset });
        }
    }
    if (_def.bHasSecondary && _inst.bExplodeOnDeath)
    {
        _inst.secondaryTimerSec     = _def.secondary.afterSec;
        _inst.bRemoveAfterSecondary = true;
    }
    if (_def.bHasGroundPatch)
    {
        PropEvent patch;
        patch.kind     = ePropEventKind::GroundPatch;
        patch.id       = _inst.id;
        patch.defIndex = _inst.defIndex;
        patch.name     = _def.groundPatch.sprite;
        patch.pos      = { _center.x - (_def.groundPatch.size.x * 0.5f - _def.groundPatch.inset.x), _center.y - (_def.groundPatch.size.y * 0.5f - _def.groundPatch.inset.y) };
        m_events.push_back(patch);
    }
}

void PropWorld::Detonate_(
    const PropBurst& _burst,
    const Vec2       _center)
{
    for (const PropExplosion& e: _burst.explosions)
    {
        for (int i = 0; i < e.count; ++i)
        {
            Vec2 p = { _center.x + e.offset.x, _center.y + e.offset.y };
            if (e.spread > 0.f)
            {
                p.x += RollFloat_(-e.spread, e.spread);
                p.y += RollFloat_(-e.spread, e.spread);
            }
            m_pendingFx.push_back(PropFxRequest { e.clip, p, kPropEffectDepth });
        }
    }
    for (int i = 0; i < _burst.debris.count; ++i)
    {
        PropDebrisPiece piece;
        const int       index = RollInt_(GetDebrisClipCount(_burst.debris.set));
        piece.clip            = GetDebrisClipName(_burst.debris.set, index);
        piece.pos             = _center;
        piece.vel             = { RollFloat_(kDebrisVxMin, kDebrisVxMax), RollFloat_(kDebrisVyMin, kDebrisVyMax) };
        m_debris.push_back(std::move(piece));
    }
}

void PropWorld::QueueDrops_(
    const PropDef& _def,
    const Vec2     _center)
{
    for (const PropDrop& drop: _def.drops)
    {
        for (int i = 0; i < drop.count; ++i)
        {
            const float    dx = drop.spreadX > 0.f ? RollFloat_(-drop.spreadX, drop.spreadX) : 0.f;
            const Vec2     feet { _center.x + dx, _center.y };
            const uint32_t itemId = m_items.Spawn(drop.item, feet, drop.bBounce);
            if (itemId != 0)
            {
                PropEvent ev;
                ev.kind  = ePropEventKind::DropSpawned;
                ev.id    = itemId;
                ev.pos   = feet;
                ev.name  = drop.item;
                ev.value = 1;
                m_events.push_back(ev);
            }
        }
    }
}

ePropVisual PropWorld::ComputeVisual_(
    const PropInstance& _inst) const
{
    if (_inst.bFalling)
    {
        return _inst.landTimerSec >= 0.f ? ePropVisual::Landed : ePropVisual::Idle;
    }
    if (_inst.bDestroyed && _inst.visuals[static_cast<size_t>(ePropVisual::Destroyed)].ref != eVisualRef::None)
    {
        return ePropVisual::Destroyed;
    }
    if (_inst.bHitPlaying && _inst.visuals[static_cast<size_t>(ePropVisual::Hit)].ref != eVisualRef::None)
    {
        return ePropVisual::Hit;
    }
    const PropDef& def = m_pLibrary->GetDefs()[static_cast<size_t>(_inst.defIndex)];
    if (def.damagedAtHp >= 0 && _inst.health.hp > 0 && _inst.health.hp <= def.damagedAtHp && _inst.visuals[static_cast<size_t>(ePropVisual::Damaged)].ref != eVisualRef::None)
    {
        return ePropVisual::Damaged;
    }
    return ePropVisual::Idle;
}

void PropWorld::UpdateVisual_(
    PropInstance&           _inst,
    const AnimationLibrary* _pAnims,
    const float             _dt)
{
    if (_inst.bHitPlaying)
    {
        _inst.hitTimerSec -= _dt;
        if (_inst.hitTimerSec <= 0.f)
        {
            _inst.bHitPlaying = false;
        }
    }
    const ePropVisual next    = ComputeVisual_(_inst);
    const bool        bChange = !_inst.bVisualBound || next != _inst.visual;
    _inst.visual              = next;
    if (_pAnims == nullptr)
    {
        return;
    }
    const PropVisual& pv = _inst.visuals[static_cast<size_t>(next)];
    if (bChange)
    {
        _inst.bVisualBound = true;
        if (pv.ref == eVisualRef::Anim)
        {
            _inst.anim.Play(_pAnims->Find(pv.name), true);
        }
    }
    if (pv.ref == eVisualRef::Anim)
    {
        _inst.anim.Update(*_pAnims, _dt);
    }
}

void PropWorld::Tick(
    const PropTickContext& _ctx)
{
    if (_ctx.pAnims != nullptr)
    {
        m_decor.Tick(*_ctx.pAnims, _ctx.dt);
    }
    const float dt = _ctx.dt;

    if (_ctx.pAnims != nullptr)
    {
        for (const PropFxRequest& fx: m_pendingFx)
        {
            m_effects.Spawn(*_ctx.pAnims, fx.clip, fx.pos, fx.depth);
        }
        m_effects.Update(*_ctx.pAnims, dt, _ctx.cameraBottom);
    }
    m_pendingFx.clear();

    for (PropDebrisPiece& piece: m_debris)
    {
        piece.ageSec += dt;
        piece.vel.y += kDebrisGravity * dt;
        piece.pos.x += piece.vel.x * dt;
        piece.pos.y += piece.vel.y * dt;
        if (_ctx.pAnims != nullptr)
        {
            if (!piece.bBound)
            {
                piece.bBound = true;
                piece.anim.Play(_ctx.pAnims->Find(piece.clip));
            }
            piece.anim.Update(*_ctx.pAnims, dt);
        }
    }
    m_debris.erase(
        std::remove_if(m_debris.begin(), m_debris.end(), [&](const PropDebrisPiece& _p) { return _p.ageSec >= kDebrisLifeSec || _p.pos.y > _ctx.cameraBottom + kDebrisCullBelowMargin; }),
        m_debris.end());

    for (PropInstance& inst: m_props)
    {
        const PropDef& def = m_pLibrary->GetDefs()[static_cast<size_t>(inst.defIndex)];

        if (inst.bFalling)
        {
            if (inst.landTimerSec < 0.f)
            {
                inst.fallVy += inst.fallGravity * dt;
                inst.pos.y += inst.fallVy * dt;
                if (_ctx.pGround != nullptr)
                {
                    const std::optional<int> surface = _ctx.pGround->FindGroundY(static_cast<int>(std::floor(inst.pos.x)), static_cast<int>(std::floor(inst.pos.y)), 2000);
                    if (surface && inst.pos.y + inst.fallHalfHeight >= static_cast<float>(*surface))
                    {
                        inst.pos.y        = static_cast<float>(*surface) - inst.fallHalfHeight;
                        inst.fallVy       = 0.f;
                        inst.landTimerSec = 0.f;
                    }
                }
            }
            else
            {
                inst.landTimerSec += dt;
                if (inst.landTimerSec >= def.fall.afterLandSec)
                {
                    Detonate_(def.onDestroy, inst.pos);
                    inst.bRemove = true;
                }
            }
        }

        if (inst.bDestroyed)
        {
            inst.destroyTimerSec += dt;
            if (inst.bRemoveAfterDestroy && inst.destroyTimerSec >= inst.destroyDurationSec)
            {
                inst.bRemove = true;
            }
        }
        if (inst.secondaryTimerSec >= 0.f)
        {
            inst.secondaryTimerSec -= dt;
            if (inst.secondaryTimerSec < 0.f)
            {
                inst.secondaryTimerSec = -1.f;
                Detonate_(def.secondary.burst, inst.pos);
                PropEvent ev;
                ev.kind     = ePropEventKind::SecondaryBurst;
                ev.id       = inst.id;
                ev.defIndex = inst.defIndex;
                ev.pos      = inst.pos;
                m_events.push_back(ev);
                if (inst.bRemoveAfterSecondary)
                {
                    inst.bRemove = true;
                }
            }
        }
        UpdateVisual_(inst, _ctx.pAnims, dt);
    }

    for (PropInstance& inst: m_props)
    {
        if (!inst.bRemove && inst.pos.x < _ctx.cameraLeft - inst.cullMargin)
        {
            // 폭이 큰 소품 (집) 은 앵커가 멀어져도 일부가 화면에 남을 수 있다: 스프라이트가 화면 왼쪽 밖으로 완전히 나간 뒤에만 지운다
            Aabb bounds = {};
            if (GetVisualBounds(inst, _ctx.pAnims, bounds) && bounds.max.x >= _ctx.cameraLeft)
            {
                continue;
            }
            inst.bRemove = true;
            PropEvent ev;
            ev.kind     = ePropEventKind::Culled;
            ev.id       = inst.id;
            ev.defIndex = inst.defIndex;
            ev.pos      = inst.pos;
            m_events.push_back(ev);
        }
    }
    m_props.erase(
        std::remove_if(
            m_props.begin(),
            m_props.end(),
            [&](const PropInstance& _p) {
                if (!_p.bRemove)
                {
                    return false;
                }
                PropEvent ev;
                ev.kind     = ePropEventKind::Removed;
                ev.id       = _p.id;
                ev.defIndex = _p.defIndex;
                ev.pos      = _p.pos;
                m_events.push_back(ev);
                return true;
            }),
        m_props.end());

    const PhysicsWorld world { _ctx.pGround, GetPlatforms(), GetWalls() };
    m_items.Tick(world, dt, _ctx.cameraLeft, _ctx.pAnims, _ctx.pItemTuning);
    for (const ItemFxRequest& fx: m_items.TakeFx())
    {
        if (_ctx.pAnims != nullptr)
        {
            m_effects.Spawn(*_ctx.pAnims, fx.clip, fx.pos, kPropEffectDepth + fx.zOffset);
        }
    }

    RebuildCollision_();
}

void PropWorld::RebuildCollision_()
{
    m_platforms.clear();
    m_enemyPlatforms.clear();
    m_walls.clear();
    for (const PropInstance& inst: m_props)
    {
        if (inst.bRemove)
        {
            continue;
        }
        for (const PropPart& part: inst.parts)
        {
            if (!PartActive_(part, inst))
            {
                continue;
            }
            const Aabb box = Aabb::FromCenterSize(inst.pos + part.offset, part.size);
            switch (part.role)
            {
                case ePropRole::Wall: m_walls.push_back(Wall { box }); break;
                case ePropRole::Platform: m_platforms.push_back(Platform { box, true }); break;
                case ePropRole::EnemyPlatform: m_enemyPlatforms.push_back(Platform { box, true }); break;
                case ePropRole::Hurt: break;
            }
        }
    }
}

int PropWorld::SortKey(
    const PropInstance& _inst)
{
    if (_inst.bSortByX)
    {
        // 건물: 작은 x (먼저 나오는 건물) 일수록 키가 커서 앞에 그려진다. 동률은 y 로 정한다.
        return -static_cast<int>(_inst.pos.x) * kBuildingDepthXScale + static_cast<int>(_inst.pos.y);
    }
    return static_cast<int>(_inst.pos.y + _inst.sortOffsetY);
}

namespace
{
[[nodiscard]] bool ResolveFrame_(
    const PropInstance&     _inst,
    const AnimationLibrary* _pAnims,
    const SpriteLibrary*    _pSprites,
    AnimFrame&              _out)
{
    const PropVisual& pv = _inst.visuals[static_cast<size_t>(_inst.visual)];
    if (pv.ref == eVisualRef::Sprite)
    {
        const SpriteInfo* pSprite = _pSprites != nullptr ? _pSprites->Find(pv.name) : nullptr;
        if (pSprite == nullptr)
        {
            return false;
        }
        _out = pSprite->frame;
    }
    else if (pv.ref == eVisualRef::Anim)
    {
        const AnimFrame* pFrame = _pAnims != nullptr ? _inst.anim.GetFrameData(*_pAnims) : nullptr;
        if (pFrame == nullptr)
        {
            return false;
        }
        _out = *pFrame;
    }
    else
    {
        return false;
    }
    return !_out.IsEmpty();
}
}   // namespace

bool PropWorld::GetVisualBounds(
    const PropInstance&     _inst,
    const AnimationLibrary* _pAnims,
    Aabb&                   _out) const
{
    AnimFrame frame = {};
    if (!ResolveFrame_(_inst, _pAnims, m_pSprites, frame))
    {
        return false;
    }
    const Vec2 topLeft = { _inst.pos.x - static_cast<float>(frame.pivotX), _inst.pos.y - static_cast<float>(frame.pivotY) };
    _out               = Aabb::FromMinSize(topLeft, { static_cast<float>(frame.w), static_cast<float>(frame.h) });
    return true;
}

void PropWorld::SetSpriteLibrary(
    const SpriteLibrary* _pSprites)
{
    m_pSprites = _pSprites;
}

void PropWorld::Submit(
    SpriteBatch&            _sprites,
    const AnimationLibrary& _anims,
    const SpriteLibrary&    _spriteLib,
    const BitmapFont*       _pFont)
{
    for (const PropInstance& inst: m_props)
    {
        if (inst.bRemove)
        {
            continue;
        }
        AnimFrame frame = {};
        if (!ResolveFrame_(inst, &_anims, &_spriteLib, frame))
        {
            continue;
        }
        _sprites.Submit(inst.depth, SortKey(inst), frame, inst.pos);
    }

    for (const PropDebrisPiece& piece: m_debris)
    {
        const AnimFrame* pFrame = piece.anim.GetFrameData(_anims);
        if (pFrame != nullptr && !pFrame->IsEmpty())
        {
            _sprites.Submit(kPropDebrisDepth, static_cast<int>(piece.pos.y), *pFrame, piece.pos);
        }
    }
    m_effects.Submit(_anims, _sprites);
    m_items.Submit(_sprites, _anims, _spriteLib, _pFont);
    m_decor.Submit(_sprites, _anims);
}

bool ApplyGroundPatch(
    GroundMap&       _map,
    const Vec2       _topLeft,
    const GroundMap& _footprint)
{
    // 발자국(마스크의 고체 픽셀)만 공기로 지운다. 발자국 밖 픽셀은 절대 건드리지 않는다.
    // 가드 (하나라도 걸리면 아무것도 바꾸지 않고 false):
    //  1) 발자국이 어떤 열의 바닥(발자국 사각형 안 가장 아래 고체 픽셀)을 지우면 거부 (도로가 뚫리는 경우)
    //  2) 발자국 사각형 안 고체 픽셀의 kMaxClearPercent % 를 넘게 지우면 거부
    constexpr uint64_t kMaxClearPercent = 50;

    const int x0 = static_cast<int>(std::floor(_topLeft.x));
    const int y0 = static_cast<int>(std::floor(_topLeft.y));
    const int w  = _footprint.GetWidth();
    const int h  = _footprint.GetHeight();

    uint64_t total   = 0;
    uint64_t cleared = 0;
    for (int x = 0; x < w; ++x)
    {
        const int mx = x0 + x;
        if (mx < 0 || mx >= _map.GetWidth())
        {
            continue;
        }
        // 이 열의 바닥 = 지면 맵 전체에서 가장 아래 고체 픽셀
        int floorY = -1;
        for (int my = _map.GetHeight() - 1; my >= 0; --my)
        {
            if (_map.IsSolid(mx, my))
            {
                floorY = my;
                break;
            }
        }
        const int fy = floorY - y0;
        if (fy >= 0 && fy < h && _footprint.IsSolid(x, fy))
        {
            return false;
        }
        for (int y = 0; y < h; ++y)
        {
            const int my = y0 + y;
            if (my < 0 || my >= _map.GetHeight() || !_map.IsSolid(mx, my))
            {
                continue;
            }
            ++total;
            if (_footprint.IsSolid(x, y))
            {
                ++cleared;
            }
        }
    }
    if (total == 0 || cleared * 100u > total * kMaxClearPercent)
    {
        return false;
    }

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            if (_footprint.IsSolid(x, y))
            {
                _map.SetSolid(x0 + x, y0 + y, false);
            }
        }
    }
    return true;
}

uint64_t PropWorld::HashState() const
{
    uint64_t h = 1469598103934665603ull;
    for (const PropInstance& inst: m_props)
    {
        h = MixHash_(h, inst.id);
        h = MixHash_(h, static_cast<uint64_t>(inst.defIndex));
        h = MixFloat_(h, inst.pos.x);
        h = MixFloat_(h, inst.pos.y);
        h = MixHash_(h, static_cast<uint64_t>(inst.health.hp));
        h = MixHash_(h, inst.bDestroyed ? 1u : 0u);
        h = MixFloat_(h, inst.destroyTimerSec);
        h = MixFloat_(h, inst.secondaryTimerSec);
        h = MixFloat_(h, inst.landTimerSec);
        h = MixHash_(h, static_cast<uint64_t>(inst.visual));
    }
    for (const PropDebrisPiece& piece: m_debris)
    {
        h = MixFloat_(h, piece.pos.x);
        h = MixFloat_(h, piece.pos.y);
        h = MixFloat_(h, piece.vel.x);
        h = MixFloat_(h, piece.vel.y);
    }
    return MixHash_(h, m_items.HashState());
}

const PropDefLibrary* PropWorld::GetLibrary() const
{
    return m_pLibrary;
}

Span<const Platform> PropWorld::GetPlatforms() const
{
    return m_platforms;
}

Span<const Platform> PropWorld::GetEnemyPlatforms() const
{
    return m_enemyPlatforms;
}

Span<const Wall> PropWorld::GetWalls() const
{
    return m_walls;
}

Span<const PropInstance> PropWorld::GetProps() const
{
    return m_props;
}

size_t PropWorld::GetCount() const
{
    return m_props.size();
}

Span<const PropDebrisPiece> PropWorld::GetDebris() const
{
    return m_debris;
}

ItemWorld& PropWorld::GetItems()
{
    return m_items;
}

const ItemWorld& PropWorld::GetItems() const
{
    return m_items;
}

uint32_t PropWorld::SpawnDecor(
    const DecorSpawn& _spawn)
{
    return m_decor.Spawn(_spawn);
}

void PropWorld::ClearDecorByLayerMask(
    const uint32_t _layerMask)
{
    m_decor.ClearByLayerMask(_layerMask);
}

const DecorWorld& PropWorld::GetDecor() const
{
    return m_decor;
}

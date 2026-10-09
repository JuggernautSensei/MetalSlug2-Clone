#include <algorithm>
#include <cmath>
#include <cstring>

#include "Game/Props/PropWorld.h"
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

constexpr float kDebrisGravity    = 1000.f;
constexpr float kDebrisLifeSec    = 5.f;
constexpr float kDebrisVxMin      = -250.f;
constexpr float kDebrisVxMax      = 249.f;
constexpr float kDebrisVyMin      = -800.f;
constexpr float kDebrisVyMax      = -301.f;
constexpr float kHitFlashSec      = 0.5f;   // 피격 연출 최대 길이 (Hit 클립이 없으면 연출 없음)

constexpr size_t kVisualCount = static_cast<size_t>(ePropVisual::Count);

uint64_t MixHash(uint64_t _h, uint64_t _v)
{
    return (_h ^ _v) * 1099511628211ull;
}

uint64_t MixFloat(uint64_t _h, float _f)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &_f, sizeof(bits));
    return MixHash(_h, bits);
}

}   // namespace

PropWorld::PropWorld(
    const PropDefLibrary* _pLibrary,
    const uint64_t        _seed)
    : m_pLibrary(_pLibrary)
    , m_items(_pLibrary, _seed)
    , m_rng(_seed)
{
}

void PropWorld::SetLibrary(
    const PropDefLibrary* _pLibrary)
{
    m_pLibrary = _pLibrary;
    m_items.SetLibrary(_pLibrary);
}

void PropWorld::Reset(
    const uint64_t _seed)
{
    ClearAll();
    m_rng    = jug::RngMt19937 { _seed };
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
    return RandRange(m_rng, _lo, _hi);
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

jug::Vector<PropEvent> PropWorld::TakeEvents()
{
    jug::Vector<PropEvent> out = std::move(m_events);
    m_events.clear();
    return out;
}

// ===========================================
//  스폰
// ===========================================

PropHandle PropWorld::Spawn(
    const jug::StringView  _defName,
    const jug::VECTOR2     _center,
    const PropSpawnParams& _params)
{
    if (m_pLibrary == nullptr)
    {
        return kNoProp;
    }
    return SpawnByIndex(m_pLibrary->FindIndex(_defName), _center, _params);
}

PropHandle PropWorld::SpawnByIndex(
    const int              _defIndex,
    const jug::VECTOR2     _center,
    const PropSpawnParams& _params)
{
    if (m_pLibrary == nullptr || _defIndex < 0 || static_cast<size_t>(_defIndex) >= m_pLibrary->GetDefs().size())
    {
        return kNoProp;
    }
    const PropDef& def = m_pLibrary->GetDefs()[static_cast<size_t>(_defIndex)];

    PropInstance inst;
    inst.id               = m_nextId++;
    inst.defIndex         = _defIndex;
    inst.pos              = _center;
    inst.bFront           = def.bFront;
    inst.cullMargin       = def.cullMargin;
    inst.bExplodeOnDeath  = _params.bExplodeOnDeath;

    // 변종 (Truck 3종, Pence 3종): 강제 번호가 없으면 시드 RNG 로 고른다. dy 는 레거시 FinalInit 보정
    const PropVariant* pVar = nullptr;
    if (!def.variants.empty())
    {
        const int v = _params.variant >= 0 ? _params.variant : RollInt_(static_cast<int>(def.variants.size()));
        pVar        = &def.variants[static_cast<size_t>(v)];
        inst.variant = v;
        inst.pos.y += pVar->dy;
    }

    auto& vis = inst.visuals;
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

    inst.health               = Health::Make(_params.hp >= 0 ? _params.hp : def.hp);
    inst.bFalling             = def.bHasFall;
    inst.fallGravity          = def.fall.gravity;
    inst.fallHalfHeight       = def.fall.halfHeight;
    inst.destroyDurationSec   = def.destroyDurationSec;

    m_props.push_back(std::move(inst));
    RebuildCollision_();
    return m_props.back().id;
}

uint32_t PropWorld::SpawnItem(
    const jug::StringView _name,
    const jug::VECTOR2    _feet,
    const bool            _bBounce)
{
    return m_items.Spawn(_name, _feet, _bBounce);
}

// ===========================================
//  판정
// ===========================================

bool PropWorld::PartActive_(
    const PropPart&    _part,
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
    const HitKindMask accepts = HitKindMask { eHitKind::Bullet, eHitKind::Melee, eHitKind::Blast };
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
    const jug::Span<const HitEvent> _events)
{
    if (m_pLibrary == nullptr)
    {
        return;
    }
    for (const HitEvent& ev: _events)
    {
        const size_t idx = FindIndex_(ev.target);
        if (idx >= m_props.size())
        {
            continue;   // 소품이 아닌 대상
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
            continue;   // 불사(hp 0) 또는 피해 0
        }
        PropEvent damaged;
        damaged.kind     = ePropEventKind::Damaged;
        damaged.id       = inst.id;
        damaged.defIndex = inst.defIndex;
        damaged.pos      = inst.pos;
        damaged.value    = applied;
        m_events.push_back(damaged);

        // 피격 연출 (Hit 클립이 있으면 다시 재생)
        if (inst.visuals[static_cast<size_t>(ePropVisual::Hit)].ref != eVisualRef::None)
        {
            inst.bHitPlaying  = true;
            inst.hitTimerSec  = kHitFlashSec;
            inst.bVisualBound = false;
        }

        // 손상 단계 진입 (1회): 예) 지면집 HP≤20 → 목재 부스러기
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

    // 파괴 중 생성 예약 (간판 등): 순회가 끝난 뒤 생성한다
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
    const jug::VECTOR2 _center)
{
    _inst.bDestroyed        = true;
    _inst.destroyTimerSec   = 0.f;
    _inst.bHitPlaying       = false;
    _inst.bVisualBound      = false;
    _inst.bRemoveAfterDestroy = _def.bRemoveOnDestroy;

    PropEvent destroyed;
    destroyed.kind     = ePropEventKind::Destroyed;
    destroyed.id       = _inst.id;
    destroyed.defIndex = _inst.defIndex;
    destroyed.pos      = _center;
    m_events.push_back(destroyed);

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
    const jug::VECTOR2 _center)
{
    for (const PropExplosion& e: _burst.explosions)
    {
        for (int i = 0; i < e.count; ++i)
        {
            jug::VECTOR2 p = { _center.x + e.offset.x, _center.y + e.offset.y };
            if (e.spread > 0.f)
            {
                p.x += RollFloat_(-e.spread, e.spread);
                p.y += RollFloat_(-e.spread, e.spread);
            }
            m_pendingFx.push_back(PropFxRequest { e.clip, p, kPropEffectLayer });
        }
    }
    for (int i = 0; i < _burst.debris.count; ++i)
    {
        PropDebrisPiece piece;
        const int       index = RollInt_(GetDebrisClipCount(_burst.debris.set));
        piece.clip   = GetDebrisClipName(_burst.debris.set, index);
        piece.pos    = _center;
        piece.vel    = { RollFloat_(kDebrisVxMin, kDebrisVxMax), RollFloat_(kDebrisVyMin, kDebrisVyMax) };
        m_debris.push_back(std::move(piece));
    }
}

void PropWorld::QueueDrops_(
    const PropDef&     _def,
    const jug::VECTOR2 _center)
{
    for (const PropDrop& drop: _def.drops)
    {
        for (int i = 0; i < drop.count; ++i)
        {
            const float dx = drop.spreadX > 0.f ? RollFloat_(-drop.spreadX, drop.spreadX) : 0.f;
            const jug::VECTOR2 feet { _center.x + dx, _center.y };
            const uint32_t     itemId = m_items.Spawn(drop.item, feet, drop.bBounce);
            if (itemId != 0)
            {
                PropEvent ev;
                ev.kind = ePropEventKind::DropSpawned;
                ev.id   = itemId;
                ev.pos  = feet;
                ev.name = drop.item;
                ev.value = 1;
                m_events.push_back(ev);
            }
        }
    }
}

// ===========================================
//  틱
// ===========================================

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
    _inst.visual = next;
    if (_pAnims == nullptr)
    {
        return;   // 라이브러리 없이는 클립 시각 진행이 없다 (헤드리스/테스트)
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
    if (m_pLibrary == nullptr)
    {
        return;
    }
    const float dt = _ctx.dt;

    // 1) 폭발 요청 → EffectList (라이브러리가 있어야 재생된다. 없으면 버린다)
    if (_ctx.pAnims != nullptr)
    {
        for (const PropFxRequest& fx: m_pendingFx)
        {
            m_effects.Spawn(*_ctx.pAnims, fx.clip, fx.pos, fx.layer);
        }
        m_effects.Update(*_ctx.pAnims, dt);
    }
    m_pendingFx.clear();

    // 2) 디브리 (중력 1000, 5 s, 화면 왼쪽 이탈 제거)
    for (PropDebrisPiece& piece: m_debris)
    {
        piece.ageSec += dt;
        piece.vel.y  += kDebrisGravity * dt;
        piece.pos.x  += piece.vel.x * dt;
        piece.pos.y  += piece.vel.y * dt;
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
        std::remove_if(m_debris.begin(), m_debris.end(), [&](const PropDebrisPiece& _p) { return _p.ageSec >= kDebrisLifeSec || _p.pos.x < _ctx.cameraLeft - 50.f; }),
        m_debris.end());

    // 3) 소품 시간 진행: 낙하체, 파괴 연출, 2차 폭발, 시각
    for (PropInstance& inst: m_props)
    {
        const PropDef& def = m_pLibrary->GetDefs()[static_cast<size_t>(inst.defIndex)];

        if (inst.bFalling)
        {
            if (inst.landTimerSec < 0.f)
            {
                inst.fallVy += inst.fallGravity * dt;
                inst.pos.y  += inst.fallVy * dt;
                if (_ctx.pGround != nullptr)
                {
                    const std::optional<int> surface = _ctx.pGround->FindGroundY(static_cast<int>(std::floor(inst.pos.x)), static_cast<int>(std::floor(inst.pos.y)), 2000);
                    if (surface && inst.pos.y + inst.fallHalfHeight >= static_cast<float>(*surface))
                    {
                        inst.pos.y      = static_cast<float>(*surface) - inst.fallHalfHeight;
                        inst.fallVy     = 0.f;
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

    // 4) 카메라 뒤로 벗어난 소품 제거 (레거시 RemoveObjectOutOfCamera). Culled 이벤트 뒤에 Removed 가 같이 나간다
    for (PropInstance& inst: m_props)
    {
        if (!inst.bRemove && inst.pos.x < _ctx.cameraLeft - inst.cullMargin)
        {
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
            [&](const PropInstance& _p)
            {
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

    // 5) 아이템 물리 (소품 발판/벽을 지면과 함께 쓴다). 습득 이펙트는 여기서 재생한다
    const PhysicsWorld world { _ctx.pGround, GetPlatforms(), GetWalls() };
    m_items.Tick(world, dt, _ctx.cameraLeft, _ctx.pAnims);
    for (const ItemFxRequest& fx: m_items.TakeFx())
    {
        if (_ctx.pAnims != nullptr)
        {
            m_effects.Spawn(*_ctx.pAnims, fx.clip, fx.pos, kPropEffectLayer);
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

// ===========================================
//  렌더
// ===========================================

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
        const PropVisual& pv = inst.visuals[static_cast<size_t>(inst.visual)];
        AnimFrame         frame = {};
        if (pv.ref == eVisualRef::Sprite)
        {
            const SpriteInfo* pSprite = _spriteLib.Find(pv.name);
            if (pSprite == nullptr)
            {
                continue;
            }
            frame = pSprite->frame;
        }
        else if (pv.ref == eVisualRef::Anim)
        {
            const AnimFrame* pFrame = inst.anim.GetFrameData(_anims);
            if (pFrame == nullptr)
            {
                continue;
            }
            frame = *pFrame;
        }
        else
        {
            continue;   // 보이지 않는 상태 (예: 교체 전 배경에 포함된 다리)
        }
        if (frame.IsEmpty())
        {
            continue;
        }
        _sprites.Submit(inst.bFront ? kPropLayerFront : kPropLayerBack, static_cast<int>(inst.pos.y), frame, inst.pos);
    }

    for (const PropDebrisPiece& piece: m_debris)
    {
        const AnimFrame* pFrame = piece.anim.GetFrameData(_anims);
        if (pFrame != nullptr && !pFrame->IsEmpty())
        {
            _sprites.Submit(kPropDebrisLayer, static_cast<int>(piece.pos.y), *pFrame, piece.pos);
        }
    }
    m_effects.Submit(_anims, _sprites);
    m_items.Submit(_sprites, _anims, _spriteLib, _pFont);
}

void ApplyGroundPatch(
    GroundMap&         _map,
    const jug::VECTOR2 _topLeft,
    const GroundMap&   _patchMask)
{
    const int x0 = static_cast<int>(std::floor(_topLeft.x));
    const int y0 = static_cast<int>(std::floor(_topLeft.y));
    for (int y = 0; y < _patchMask.GetHeight(); ++y)
    {
        for (int x = 0; x < _patchMask.GetWidth(); ++x)
        {
            _map.SetSolid(x0 + x, y0 + y, _patchMask.IsSolid(x, y));
        }
    }
}

uint64_t PropWorld::HashState() const
{
    uint64_t h = 1469598103934665603ull;
    for (const PropInstance& inst: m_props)
    {
        h = MixHash(h, inst.id);
        h = MixHash(h, static_cast<uint64_t>(inst.defIndex));
        h = MixFloat(h, inst.pos.x);
        h = MixFloat(h, inst.pos.y);
        h = MixHash(h, static_cast<uint64_t>(inst.health.hp));
        h = MixHash(h, inst.bDestroyed ? 1u : 0u);
        h = MixFloat(h, inst.destroyTimerSec);
        h = MixFloat(h, inst.secondaryTimerSec);
        h = MixFloat(h, inst.landTimerSec);
        h = MixHash(h, static_cast<uint64_t>(inst.visual));
    }
    for (const PropDebrisPiece& piece: m_debris)
    {
        h = MixFloat(h, piece.pos.x);
        h = MixFloat(h, piece.pos.y);
        h = MixFloat(h, piece.vel.x);
        h = MixFloat(h, piece.vel.y);
    }
    return MixHash(h, m_items.HashState());
}

}   // namespace slug

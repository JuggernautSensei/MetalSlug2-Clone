#include <format>
#include "Base/Types.h"
#include "Base/Math.h"
#include <initializer_list>
#include <string_view>

#include "Base/Log.h"
#include "Base/Json.h"

#include "Base/Error.h"
#include "Game/Props/PropDef.h"
#include "Render/Animation.h"
#include "Render/SpriteLibrary.h"

namespace slug
{

// ===========================================
//  디브리 클립 (Wood1..6 / Metal1..12 / Rock1..3)
// ===========================================

int GetDebrisClipCount(
    const eDebrisSet _set)
{
    switch (_set)
    {
        case eDebrisSet::Wood: return 6;
        case eDebrisSet::Metal: return 12;
        case eDebrisSet::Rock: return 3;
    }
    return 0;
}

slug::String GetDebrisClipName(
    const eDebrisSet _set,
    const int        _index)
{
    const char* prefix = _set == eDebrisSet::Wood ? "Wood" : (_set == eDebrisSet::Metal ? "Metal" : "Rock");
    return slug::String { std::format("{}{}", prefix, _index + 1) };
}

namespace
{

// ===========================================
//  PropDefParser
//   Props.json → PropDefLibrary. 알 수 없는 키/값은 오류이며, 메시지는 "<정의>: <필드>: 사유" 형식이다.
// ===========================================

using KeyList = std::initializer_list<std::string_view>;

eDebrisSet ParseDebrisSet(const std::string_view _s, bool& _bOk)
{
    _bOk = true;
    if (_s == "wood") return eDebrisSet::Wood;
    if (_s == "metal") return eDebrisSet::Metal;
    if (_s == "rock") return eDebrisSet::Rock;
    _bOk = false;
    return eDebrisSet::Wood;
}

}   // namespace

class PropDefParser
{
public:
    explicit PropDefParser(PropDefLibrary& _lib)
        : m_lib(_lib)
    {
    }

    [[nodiscard]] bool Run(const slug::JsonReader& _root);

    [[nodiscard]] bool ParseParams(const PropDef& _def, const slug::JsonReader& _params, PropSpawnParams& _out);

    slug::String error = {};

private:
    bool Fail_(std::string_view _where, std::string_view _field, std::string_view _msg);
    bool Keys_(const slug::JsonReader& _obj, std::string_view _where, KeyList _allowed);
    bool Num_(const slug::JsonReader& _obj, std::string_view _where, std::string_view _key, float& _out, bool _bRequired);
    bool Int_(const slug::JsonReader& _obj, std::string_view _where, std::string_view _key, int& _out, bool _bRequired);
    bool Bool_(const slug::JsonReader& _obj, std::string_view _where, std::string_view _key, bool& _out);
    bool Str_(const slug::JsonReader& _obj, std::string_view _where, std::string_view _key, slug::String& _out, bool _bRequired);
    bool Vec2_(const slug::JsonReader& _obj, std::string_view _where, std::string_view _key, Vec2& _out, bool _bRequired);
    bool Visual_(const slug::JsonReader& _obj, std::string_view _where, std::string_view _key, PropVisual& _out);
    bool Parts_(const slug::JsonReader& _arr, std::string_view _where, slug::Vector<PropPart>& _out);
    bool Burst_(const slug::JsonReader& _obj, std::string_view _where, PropBurst& _out, bool _bAllowAfterSec = false);
    bool Drops_(const slug::JsonReader& _arr, std::string_view _where, slug::Vector<PropDrop>& _out);
    bool Spawns_(const slug::JsonReader& _arr, std::string_view _where, slug::Vector<PropChild>& _out);
    bool Def_(std::string_view _name, const slug::JsonReader& _obj);
    bool Item_(std::string_view _name, const slug::JsonReader& _obj);
    bool CheckRefs_();

    PropDefLibrary& m_lib;
};

bool PropDefParser::Fail_(
    const std::string_view _where,
    const std::string_view _field,
    const std::string_view _msg)
{
    if (error.empty())
    {
        error = std::format("{}: {}: {}", std::string { _where }, std::string { _field }, std::string { _msg });
    }
    return false;
}

bool PropDefParser::Keys_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const KeyList          _allowed)
{
    if (!_obj.IsObject())
    {
        return Fail_(_where, "", "객체가 필요합니다");
    }
    for (const auto [key, value]: _obj.GetObject())
    {
        const std::string_view k { key };
        bool                   bFound = false;
        for (const std::string_view allowed: _allowed)
        {
            bFound = bFound || allowed == k;
        }
        if (!bFound)
        {
            return Fail_(_where, k, "알 수 없는 키입니다");
        }
    }
    return true;
}

bool PropDefParser::Num_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const std::string_view _key,
    float&                 _out,
    const bool             _bRequired)
{
    if (!_obj.HasField(_key))
    {
        return _bRequired ? Fail_(_where, _key, "필수 항목이 없습니다") : true;
    }
    const slug::Result<double> value = _obj.ReadField<double>(_key);
    if (!value)
    {
        return Fail_(_where, _key, "숫자가 필요합니다");
    }
    _out = static_cast<float>(value.GetValue());
    return true;
}

bool PropDefParser::Int_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const std::string_view _key,
    int&                   _out,
    const bool             _bRequired)
{
    float value = 0.f;
    if (!_obj.HasField(_key))
    {
        return _bRequired ? Fail_(_where, _key, "필수 항목이 없습니다") : true;
    }
    if (!Num_(_obj, _where, _key, value, true))
    {
        return false;
    }
    if (value != static_cast<float>(static_cast<int>(value)))
    {
        return Fail_(_where, _key, "정수가 필요합니다");
    }
    _out = static_cast<int>(value);
    return true;
}

bool PropDefParser::Bool_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const std::string_view _key,
    bool&                  _out)
{
    if (!_obj.HasField(_key))
    {
        return true;
    }
    const slug::Result<bool> value = _obj.ReadField<bool>(_key);
    if (!value)
    {
        return Fail_(_where, _key, "true/false 가 필요합니다");
    }
    _out = value.GetValue();
    return true;
}

bool PropDefParser::Str_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const std::string_view _key,
    slug::String&           _out,
    const bool             _bRequired)
{
    if (!_obj.HasField(_key))
    {
        return _bRequired ? Fail_(_where, _key, "필수 항목이 없습니다") : true;
    }
    const slug::Result<slug::String> value = _obj.ReadField<slug::String>(_key);
    if (!value)
    {
        return Fail_(_where, _key, "문자열이 필요합니다");
    }
    _out = value.GetValue();
    return true;
}

bool PropDefParser::Vec2_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const std::string_view _key,
    Vec2&          _out,
    const bool             _bRequired)
{
    if (!_obj.HasField(_key))
    {
        return _bRequired ? Fail_(_where, _key, "필수 항목이 없습니다") : true;
    }
    const slug::Result<slug::JsonReader> arr = _obj.FindField(_key);
    if (!arr || !arr->IsArray() || arr->GetSize() != 2)
    {
        return Fail_(_where, _key, "[x, y] 배열이 필요합니다");
    }
    float xy[2] = { 0.f, 0.f };
    int   i     = 0;
    for (const slug::JsonReader item: arr->GetArray())
    {
        const slug::Result<double> v = item.Read<double>();
        if (!v || i >= 2)
        {
            return Fail_(_where, _key, "숫자 배열이 필요합니다");
        }
        xy[i++] = static_cast<float>(v.GetValue());
    }
    _out = { xy[0], xy[1] };
    return true;
}

bool PropDefParser::Visual_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    const std::string_view _key,
    PropVisual&            _out)
{
    if (!_obj.HasField(_key))
    {
        return true;
    }
    const slug::Result<slug::JsonReader> v = _obj.FindField(_key);
    if (!v || !v->IsObject())
    {
        return Fail_(_where, _key, "{\"anim\": ...} 또는 {\"sprite\": ...} 객체가 필요합니다");
    }
    if (!Keys_(*v, std::format("{}.{}", _where, _key), { "note", "anim", "sprite" }))
    {
        return false;
    }
    const bool bAnim   = v->HasField("anim");
    const bool bSprite = v->HasField("sprite");
    if (bAnim == bSprite)
    {
        return Fail_(_where, _key, "anim 또는 sprite 중 하나만 지정해야 합니다");
    }
    _out.ref = bAnim ? eVisualRef::Anim : eVisualRef::Sprite;
    return Str_(*v, _where, bAnim ? "anim" : "sprite", _out.name, true);
}

bool PropDefParser::Parts_(
    const slug::JsonReader& _arr,
    const std::string_view _where,
    slug::Vector<PropPart>& _out)
{
    if (!_arr.IsArray())
    {
        return Fail_(_where, "parts", "배열이 필요합니다");
    }
    for (const slug::JsonReader item: _arr.GetArray())
    {
        const std::string where = std::format("{}.parts", _where);
        if (!Keys_(item, where, { "note", "role", "when", "size", "offset" }))
        {
            return false;
        }
        // role: 문자열 또는 문자열 배열 (예: ["hurt", "wall"] = 같은 상자를 피격과 벽으로 둘 다)
        slug::Vector<ePropRole> roles;
        if (!item.HasField("role"))
        {
            return Fail_(where, "role", "필수 항목이 없습니다");
        }
        const slug::Result<slug::JsonReader> roleField = item.FindField("role");
        auto                               parseRole = [&](const std::string_view _s) -> bool
        {
            if (_s == "hurt") roles.push_back(ePropRole::Hurt);
            else if (_s == "wall") roles.push_back(ePropRole::Wall);
            else if (_s == "platform") roles.push_back(ePropRole::Platform);
            else if (_s == "enemyPlatform") roles.push_back(ePropRole::EnemyPlatform);
            else return Fail_(where, "role", "hurt/wall/platform/enemyPlatform 중 하나여야 합니다");
            return true;
        };
        if (roleField->IsArray())
        {
            for (const slug::JsonReader r: roleField->GetArray())
            {
                const slug::Result<slug::String> s = r.Read<slug::String>();
                if (!s || !parseRole(s.GetValue()))
                {
                    return Fail_(where, "role", "문자열 배열이 필요합니다");
                }
            }
        }
        else
        {
            const slug::Result<slug::String> s = roleField->Read<slug::String>();
            if (!s || !parseRole(s.GetValue()))
            {
                return false;
            }
        }

        ePropWhen when = ePropWhen::Intact;
        if (item.HasField("when"))
        {
            const slug::Result<slug::String> w = item.ReadField<slug::String>("when");
            if (!w)
            {
                return Fail_(where, "when", "문자열이 필요합니다");
            }
            const std::string_view ws = w.GetValue();
            if (ws == "intact") when = ePropWhen::Intact;
            else if (ws == "destroyed") when = ePropWhen::Destroyed;
            else if (ws == "always") when = ePropWhen::Always;
            else return Fail_(where, "when", "intact/destroyed/always 중 하나여야 합니다");
        }

        Vec2 size   = { 0.f, 0.f };
        Vec2 offset = { 0.f, 0.f };
        if (!Vec2_(item, where, "size", size, true) || !Vec2_(item, where, "offset", offset, false))
        {
            return false;
        }
        for (const ePropRole role: roles)
        {
            _out.push_back(PropPart { role, when, size, offset });
        }
    }
    return true;
}

bool PropDefParser::Burst_(
    const slug::JsonReader& _obj,
    const std::string_view _where,
    PropBurst&             _out,
    const bool             _bAllowAfterSec)
{
    const bool bKeysOk = _bAllowAfterSec ? Keys_(_obj, _where, { "note", "explosions", "debris", "afterSec" }) : Keys_(_obj, _where, { "note", "explosions", "debris" });
    if (!bKeysOk)
    {
        return false;
    }
    if (_obj.HasField("explosions"))
    {
        const slug::Result<slug::JsonReader> arr = _obj.FindField("explosions");
        if (!arr || !arr->IsArray())
        {
            return Fail_(_where, "explosions", "배열이 필요합니다");
        }
        for (const slug::JsonReader item: arr->GetArray())
        {
            const std::string where = std::format("{}.explosions", _where);
            PropExplosion     e;
            if (!Keys_(item, where, { "note", "clip", "count", "spread", "offset" }) || !Str_(item, where, "clip", e.clip, true) || !Int_(item, where, "count", e.count, false) || !Num_(item, where, "spread", e.spread, false) || !Vec2_(item, where, "offset", e.offset, false))
            {
                return false;
            }
            _out.explosions.push_back(e);
        }
    }
    if (_obj.HasField("debris"))
    {
        const slug::Result<slug::JsonReader> d = _obj.FindField("debris");
        const std::string                  where = std::format("{}.debris", _where);
        if (!d || !Keys_(*d, where, { "note", "set", "count" }))
        {
            return false;
        }
        slug::String set;
        if (!Str_(*d, where, "set", set, true) || !Int_(*d, where, "count", _out.debris.count, true))
        {
            return false;
        }
        bool bOk = false;
        _out.debris.set = ParseDebrisSet(set, bOk);
        if (!bOk)
        {
            return Fail_(where, "set", "wood/metal/rock 중 하나여야 합니다");
        }
    }
    return true;
}

bool PropDefParser::Drops_(
    const slug::JsonReader& _arr,
    const std::string_view _where,
    slug::Vector<PropDrop>& _out)
{
    if (!_arr.IsArray())
    {
        return Fail_(_where, "drops", "배열이 필요합니다");
    }
    for (const slug::JsonReader item: _arr.GetArray())
    {
        const std::string where = std::format("{}.drops", _where);
        PropDrop          d;
        if (!Keys_(item, where, { "note", "item", "count", "spreadX", "bounce" }) || !Str_(item, where, "item", d.item, true) || !Int_(item, where, "count", d.count, false) || !Num_(item, where, "spreadX", d.spreadX, false) || !Bool_(item, where, "bounce", d.bBounce))
        {
            return false;
        }
        _out.push_back(std::move(d));
    }
    return true;
}

bool PropDefParser::Spawns_(
    const slug::JsonReader& _arr,
    const std::string_view _where,
    slug::Vector<PropChild>& _out)
{
    if (!_arr.IsArray())
    {
        return Fail_(_where, "spawns", "배열이 필요합니다");
    }
    for (const slug::JsonReader item: _arr.GetArray())
    {
        const std::string where = std::format("{}.spawns", _where);
        PropChild         c;
        if (!Keys_(item, where, { "note", "def", "offset" }) || !Str_(item, where, "def", c.def, true) || !Vec2_(item, where, "offset", c.offset, false))
        {
            return false;
        }
        _out.push_back(std::move(c));
    }
    return true;
}

bool PropDefParser::Def_(
    const std::string_view _name,
    const slug::JsonReader& _obj)
{
    const std::string where { _name };
    if (!Keys_(_obj, where, { "note", "hp", "cullMargin", "layer", "idle", "damaged", "damagedAtHp", "destroyed", "hit", "parts", "variants", "remove", "destroyDuration", "onDamaged", "onDestroy", "secondary", "drops", "spawns", "fall", "groundPatch" }))
    {
        return false;
    }
    PropDef def;
    def.name = slug::String { _name };
    if (!Int_(_obj, where, "hp", def.hp, true) || !Num_(_obj, where, "cullMargin", def.cullMargin, false) || !Int_(_obj, where, "damagedAtHp", def.damagedAtHp, false) || !Num_(_obj, where, "destroyDuration", def.destroyDurationSec, false) || !Bool_(_obj, where, "remove", def.bRemoveOnDestroy))
    {
        return false;
    }
    if (def.hp < 0)
    {
        return Fail_(where, "hp", "0 이상이어야 합니다");
    }
    if (_obj.HasField("layer"))
    {
        slug::String layer;
        if (!Str_(_obj, where, "layer", layer, true))
        {
            return false;
        }
        if (layer == "background") def.bFront = false;
        else if (layer == "front") def.bFront = true;
        else return Fail_(where, "layer", "background/front 중 하나여야 합니다");
    }
    if (!Visual_(_obj, where, "idle", def.idle) || !Visual_(_obj, where, "damaged", def.damaged) || !Visual_(_obj, where, "destroyed", def.destroyed) || !Visual_(_obj, where, "hit", def.hit))
    {
        return false;
    }
    if (_obj.HasField("parts") && !Parts_(_obj.FindField("parts").GetValue(), where, def.parts))
    {
        return false;
    }
    if (_obj.HasField("variants"))
    {
        const slug::Result<slug::JsonReader> arr = _obj.FindField("variants");
        if (!arr || !arr->IsArray() || arr->GetSize() == 0)
        {
            return Fail_(where, "variants", "1개 이상의 배열이 필요합니다");
        }
        for (const slug::JsonReader item: arr->GetArray())
        {
            const std::string where2 = std::format("{}.variants", where);
            PropVariant       v;
            if (!Keys_(item, where2, { "note", "name", "dy", "idle", "hit", "parts" }) || !Str_(item, where2, "name", v.name, true) || !Num_(item, where2, "dy", v.dy, false) || !Visual_(item, where2, "idle", v.idle) || !Visual_(item, where2, "hit", v.hit))
            {
                return false;
            }
            if (item.HasField("parts") && !Parts_(item.FindField("parts").GetValue(), where2, v.parts))
            {
                return false;
            }
            def.variants.push_back(std::move(v));
        }
    }
    if (_obj.HasField("onDamaged"))
    {
        if (!Burst_(_obj.FindField("onDamaged").GetValue(), where, def.onDamaged))
        {
            return false;
        }
    }
    if (_obj.HasField("onDestroy"))
    {
        if (!Burst_(_obj.FindField("onDestroy").GetValue(), where, def.onDestroy))
        {
            return false;
        }
    }
    if (_obj.HasField("secondary"))
    {
        const slug::Result<slug::JsonReader> s = _obj.FindField("secondary");
        const std::string                  where2 = where + ".secondary";
        if (!s || !Num_(*s, where2, "afterSec", def.secondary.afterSec, false) || !Burst_(*s, where2, def.secondary.burst, true))
        {
            return Fail_(where, "secondary", "잘못된 객체입니다");
        }
        def.bHasSecondary = true;
    }
    if (_obj.HasField("drops") && !Drops_(_obj.FindField("drops").GetValue(), where, def.drops))
    {
        return false;
    }
    if (_obj.HasField("spawns") && !Spawns_(_obj.FindField("spawns").GetValue(), where, def.spawns))
    {
        return false;
    }
    if (_obj.HasField("fall"))
    {
        const slug::Result<slug::JsonReader> f = _obj.FindField("fall");
        const std::string                  where2 = where + ".fall";
        if (!f || !Keys_(*f, where2, { "note", "gravity", "landed", "afterLandSec", "halfHeight" }) || !Num_(*f, where2, "gravity", def.fall.gravity, false) || !Num_(*f, where2, "afterLandSec", def.fall.afterLandSec, false) || !Num_(*f, where2, "halfHeight", def.fall.halfHeight, false) || !Visual_(*f, where2, "landed", def.fall.landed))
        {
            return Fail_(where, "fall", "잘못된 객체입니다");
        }
        def.bHasFall = true;
    }
    if (_obj.HasField("groundPatch"))
    {
        const slug::Result<slug::JsonReader> g = _obj.FindField("groundPatch");
        const std::string                  where2 = where + ".groundPatch";
        if (!g || !Keys_(*g, where2, { "note", "sprite", "size", "inset" }) || !Str_(*g, where2, "sprite", def.groundPatch.sprite, true) || !Vec2_(*g, where2, "size", def.groundPatch.size, true) || !Vec2_(*g, where2, "inset", def.groundPatch.inset, true))
        {
            return Fail_(where, "groundPatch", "잘못된 객체입니다");
        }
        def.bHasGroundPatch = true;
    }
    if (m_lib.m_defIndex.contains(def.name))
    {
        return Fail_(where, "", "정의 이름이 중복됩니다");
    }
    m_lib.m_defIndex.emplace(def.name, static_cast<uint32_t>(m_lib.m_defs.size()));
    m_lib.m_defs.push_back(std::move(def));
    return true;
}

bool PropDefParser::Item_(
    const std::string_view _name,
    const slug::JsonReader& _obj)
{
    const std::string where = std::format("items.{}", std::string { _name });
    if (!Keys_(_obj, where, { "note", "kind", "weapon", "supply", "amount", "score", "roll", "visual", "begin", "pickup" }))
    {
        return false;
    }
    PropItemDef item;
    item.name = slug::String { _name };

    slug::String kind;
    if (!Str_(_obj, where, "kind", kind, true))
    {
        return false;
    }
    if (kind == "gun") item.kind = eItemKind::Gun;
    else if (kind == "supply") item.kind = eItemKind::Supply;
    else if (kind == "score") item.kind = eItemKind::Score;
    else return Fail_(where, "kind", "gun/supply/score 중 하나여야 합니다");

    slug::String weapon;
    if (Str_(_obj, where, "weapon", weapon, false) && _obj.HasField("weapon"))
    {
        if (weapon == "hmg") item.weapon = eWeapon::Hmg;
        else if (weapon == "shotgun") item.weapon = eWeapon::Shotgun;
        else if (weapon == "rocket") item.weapon = eWeapon::Rocket;
        else if (weapon == "flame") item.weapon = eWeapon::Flame;
        else return Fail_(where, "weapon", "hmg/shotgun/rocket/flame 중 하나여야 합니다");
    }
    slug::String supply;
    if (Str_(_obj, where, "supply", supply, false) && _obj.HasField("supply"))
    {
        if (supply == "bomb") item.supply = eSupplyKind::Bomb;
        else if (supply == "bullet") item.supply = eSupplyKind::Bullet;
        else return Fail_(where, "supply", "bomb/bullet 중 하나여야 합니다");
    }
    if (!Int_(_obj, where, "amount", item.amount, false) || !Int_(_obj, where, "score", item.score, false) || !Bool_(_obj, where, "roll", item.bRoll))
    {
        return false;
    }
    if (!Visual_(_obj, where, "visual", item.visual) || !Visual_(_obj, where, "begin", item.begin) || !Visual_(_obj, where, "pickup", item.pickup))
    {
        return false;
    }
    if (item.kind == eItemKind::Gun && (item.weapon == eWeapon::Pistol || !_obj.HasField("weapon")))
    {
        return Fail_(where, "weapon", "gun 종류는 weapon 이 필요합니다");
    }
    if (item.kind == eItemKind::Score && item.score <= 0)
    {
        return Fail_(where, "score", "score 종류는 양수 score 가 필요합니다");
    }
    if (item.kind == eItemKind::Supply && item.supply == eSupplyKind::Bomb && item.amount <= 0)
    {
        return Fail_(where, "amount", "bomb 보급은 양수 amount 가 필요합니다");
    }
    if (item.visual.ref == eVisualRef::None)
    {
        return Fail_(where, "visual", "표시 스프라이트/클립이 필요합니다");
    }
    if (m_lib.m_itemIndex.contains(item.name))
    {
        return Fail_(where, "", "아이템 이름이 중복됩니다");
    }
    m_lib.m_itemIndex.emplace(item.name, static_cast<uint32_t>(m_lib.m_items.size()));
    m_lib.m_items.push_back(std::move(item));
    return true;
}

bool PropDefParser::CheckRefs_()
{
    for (const PropDef& def: m_lib.m_defs)
    {
        const std::string where { def.name.c_str() };
        for (const PropDrop& drop: def.drops)
        {
            if (m_lib.m_itemIndex.find(drop.item) == m_lib.m_itemIndex.end() && m_lib.m_groupIndex.find(drop.item) == m_lib.m_groupIndex.end())
            {
                return Fail_(where, "drops", std::format("'{}' 아이템/그룹이 없습니다", std::string { drop.item.c_str() }));
            }
        }
        for (const PropChild& child: def.spawns)
        {
            if (m_lib.m_defIndex.find(child.def) == m_lib.m_defIndex.end())
            {
                return Fail_(where, "spawns", std::format("'{}' 정의가 없습니다", std::string { child.def.c_str() }));
            }
        }
    }
    return true;
}

bool PropDefParser::Run(
    const slug::JsonReader& _root)
{
    if (!Keys_(_root, "Props.json", { "note", "version", "defs", "items", "groups" }))
    {
        return false;
    }
    int version = 1;
    if (!Int_(_root, "Props.json", "version", version, false))
    {
        return false;
    }
    if (version != 1)
    {
        return Fail_("Props.json", "version", "지원하지 않는 버전입니다 (1 만 지원)");
    }

    // 그룹은 아이템보다 먼저 이름만 등록한다 (드롭 검사는 뒤에서). 아이템 본체는 아래에서 읽는다.
    if (_root.HasField("defs"))
    {
        const slug::Result<slug::JsonReader> defs = _root.FindField("defs");
        if (!defs || !defs->IsObject())
        {
            return Fail_("Props.json", "defs", "객체가 필요합니다");
        }
        for (const auto [key, value]: defs->GetObject())
        {
            if (!Def_(std::string_view { key }, value))
            {
                return false;
            }
        }
    }
    if (_root.HasField("items"))
    {
        const slug::Result<slug::JsonReader> items = _root.FindField("items");
        if (!items || !items->IsObject())
        {
            return Fail_("Props.json", "items", "객체가 필요합니다");
        }
        for (const auto [key, value]: items->GetObject())
        {
            if (!Item_(std::string_view { key }, value))
            {
                return false;
            }
        }
    }
    if (_root.HasField("groups"))
    {
        const slug::Result<slug::JsonReader> groups = _root.FindField("groups");
        if (!groups || !groups->IsObject())
        {
            return Fail_("Props.json", "groups", "객체가 필요합니다");
        }
        for (const auto [key, value]: groups->GetObject())
        {
            PropItemGroup group;
            group.name = slug::String { std::string_view { key } };
            if (!value.IsArray())
            {
                return Fail_("groups", std::string_view { key }, "아이템 이름 배열이 필요합니다");
            }
            for (const slug::JsonReader item: value.GetArray())
            {
                const slug::Result<slug::String> name = item.Read<slug::String>();
                if (!name)
                {
                    return Fail_("groups", std::string_view { key }, "문자열 배열이 필요합니다");
                }
                if (m_lib.m_itemIndex.find(name.GetValue()) == m_lib.m_itemIndex.end())
                {
                    return Fail_("groups", std::string_view { key }, std::format("'{}' 아이템이 없습니다", std::string { name->c_str() }));
                }
                group.names.push_back(name.GetValue());
            }
            if (group.names.empty())
            {
                return Fail_("groups", std::string_view { key }, "빈 그룹입니다");
            }
            m_lib.m_groupIndex.emplace(group.name, static_cast<uint32_t>(m_lib.m_groups.size()));
            m_lib.m_groups.push_back(std::move(group));
        }
    }
    return CheckRefs_();
}

bool PropDefParser::ParseParams(
    const PropDef&         _def,
    const slug::JsonReader& _params,
    PropSpawnParams&       _out)
{
    const std::string where { _def.name.c_str() };
    if (!Keys_(_params, where, { "note", "hp", "sprites", "hitbox", "standBox", "explodeOnDeath", "variant" }))
    {
        return false;
    }
    if (!Int_(_params, where, "hp", _out.hp, false) || !Bool_(_params, where, "explodeOnDeath", _out.bExplodeOnDeath) || !Int_(_params, where, "variant", _out.variant, false))
    {
        return false;
    }
    if (_out.hp < -1)
    {
        return Fail_(where, "hp", "0 이상이어야 합니다");
    }
    if (_params.HasField("sprites"))
    {
        const slug::Result<slug::JsonReader> arr = _params.FindField("sprites");
        if (!arr || !arr->IsArray() || arr->GetSize() < 1 || arr->GetSize() > 2)
        {
            return Fail_(where, "sprites", "[idle] 또는 [idle, destroyed] 배열이 필요합니다");
        }
        int i = 0;
        for (const slug::JsonReader item: arr->GetArray())
        {
            const slug::Result<slug::String> s = item.Read<slug::String>();
            if (!s)
            {
                return Fail_(where, "sprites", "문자열 배열이 필요합니다");
            }
            (i == 0 ? _out.idleSprite : _out.destroySprite) = s.GetValue();
            ++i;
        }
    }
    for (const char* boxKey: { "hitbox", "standBox" })
    {
        if (!_params.HasField(boxKey))
        {
            continue;
        }
        const slug::Result<slug::JsonReader> box = _params.FindField(boxKey);
        if (!box || !Keys_(*box, std::format("{}.{}", where, boxKey), { "note", "size", "offset" }))
        {
            return false;
        }
        PropBoxOverride o;
        if (!Vec2_(*box, where, "size", o.size, true) || !Vec2_(*box, where, "offset", o.offset, false))
        {
            return false;
        }
        if (std::string_view { boxKey } == "hitbox")
        {
            _out.hitbox = o;
        }
        else
        {
            _out.standBox = o;
        }
    }
    if (_out.variant >= 0 && _def.variants.empty())
    {
        return Fail_(where, "variant", "이 정의에는 변종이 없습니다");
    }
    if (_out.variant >= static_cast<int>(_def.variants.size()))
    {
        return Fail_(where, "variant", "변종 번호가 범위를 벗어났습니다");
    }
    return true;
}

// ===========================================
//  PropDefLibrary
// ===========================================

slug::Result<PropDefLibrary> PropDefLibrary::Parse(
    const slug::StringView _json,
    slug::String*          _pMessage)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::Load(_json, slug::eJsonLoadOption::AllowComments);
    if (!doc)
    {
        if (_pMessage)
        {
            *_pMessage = "Props.json: JSON 구문 오류";
        }
        SLUG_LOG_ERROR("Props.json: JSON 구문 오류");
        return slug::Error { eError::Parse };
    }
    PropDefLibrary library;
    PropDefParser  parser { library };
    if (!parser.Run(doc->GetReader()))
    {
        if (_pMessage)
        {
            *_pMessage = parser.error;
        }
        SLUG_LOG_ERROR("소품 정의 오류: {}", parser.error);
        return slug::Error { eError::PropDefInvalid };
    }
    return library;
}

slug::Result<PropDefLibrary> PropDefLibrary::Load(
    const slug::FilePath& _path,
    slug::String*         _pMessage)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::LoadFromFile(_path, slug::eJsonLoadOption::AllowComments);
    if (!doc)
    {
        if (_pMessage)
        {
            *_pMessage = std::format("{}: 파일을 열 수 없거나 JSON 구문 오류", _path.string());
        }
        SLUG_LOG_ERROR("소품 정의 파일 로드 실패: {}", _path.string());
        return slug::Error { eError::FileOpen };
    }
    PropDefLibrary library;
    PropDefParser  parser { library };
    if (!parser.Run(doc->GetReader()))
    {
        if (_pMessage)
        {
            *_pMessage = parser.error;
        }
        SLUG_LOG_ERROR("소품 정의 오류 ({}): {}", _path.string(), parser.error);
        return slug::Error { eError::PropDefInvalid };
    }
    return library;
}

const PropDef* PropDefLibrary::Find(
    const slug::StringView _name) const
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_defs[static_cast<size_t>(index)];
}

int PropDefLibrary::FindIndex(
    const slug::StringView _name) const
{
    const auto it = m_defIndex.find(slug::String { _name });
    return it == m_defIndex.end() ? -1 : static_cast<int>(it->second);
}

const PropItemDef* PropDefLibrary::FindItem(
    const slug::StringView _name) const
{
    const int index = FindItemIndex(_name);
    return index < 0 ? nullptr : &m_items[static_cast<size_t>(index)];
}

int PropDefLibrary::FindItemIndex(
    const slug::StringView _name) const
{
    const auto it = m_itemIndex.find(slug::String { _name });
    return it == m_itemIndex.end() ? -1 : static_cast<int>(it->second);
}

const PropItemGroup* PropDefLibrary::FindGroup(
    const slug::StringView _name) const
{
    const auto it = m_groupIndex.find(slug::String { _name });
    return it == m_groupIndex.end() ? nullptr : &m_groups[it->second];
}

bool PropDefLibrary::ParseSpawnParams(
    const slug::StringView  _defName,
    const slug::JsonReader& _params,
    PropSpawnParams&       _out,
    slug::String&           _error) const
{
    const PropDef* pDef = Find(_defName);
    if (!pDef)
    {
        _error = slug::String { std::format("{}: 정의가 없습니다", std::string { _defName }) };
        return false;
    }
    PropDefLibrary scratch;   // 파서는 라이브러리를 변경하지 않는다 (파라미터 해석만 사용)
    PropDefParser  parser { scratch };
    _out = {};
    if (!parser.ParseParams(*pDef, _params, _out))
    {
        _error = parser.error;
        return false;
    }
    return true;
}

slug::String PropDefLibrary::FindMissingAsset(
    const AnimationLibrary& _anims,
    const SpriteLibrary&    _sprites) const
{
    auto missing = [&](const PropVisual& _v, const std::string& _where) -> slug::String
    {
        if (_v.ref == eVisualRef::Anim && !_anims.IsValid(_anims.Find(_v.name)))
        {
            return slug::String { std::format("{}: 클립 '{}'", _where, std::string { _v.name.c_str() }) };
        }
        if (_v.ref == eVisualRef::Sprite && _sprites.Find(_v.name) == nullptr)
        {
            return slug::String { std::format("{}: 스프라이트 '{}'", _where, std::string { _v.name.c_str() }) };
        }
        return {};
    };
    auto checkBurst = [&](const PropBurst& _b, const std::string& _where) -> slug::String
    {
        for (const PropExplosion& e: _b.explosions)
        {
            slug::String m = missing(PropVisual { eVisualRef::Anim, e.clip }, _where);
            if (!m.empty())
            {
                return m;
            }
        }
        for (int i = 0; _b.debris.count > 0 && i < GetDebrisClipCount(_b.debris.set); ++i)
        {
            slug::String m = missing(PropVisual { eVisualRef::Anim, GetDebrisClipName(_b.debris.set, i) }, _where);
            if (!m.empty())
            {
                return m;
            }
        }
        return {};
    };

    for (const PropDef& def: m_defs)
    {
        const std::string w { def.name.c_str() };
        slug::String       m;
        for (const PropVisual* v: { &def.idle, &def.damaged, &def.destroyed, &def.hit, &def.fall.landed })
        {
            if (!(m = missing(*v, w)).empty())
            {
                return m;
            }
        }
        for (const PropVariant& var: def.variants)
        {
            for (const PropVisual* v: { &var.idle, &var.hit })
            {
                if (!(m = missing(*v, w)).empty())
                {
                    return m;
                }
            }
        }
        if (!(m = checkBurst(def.onDamaged, w)).empty() || !(m = checkBurst(def.onDestroy, w)).empty() || !(m = checkBurst(def.secondary.burst, w)).empty())
        {
            return m;
        }
        if (def.bHasGroundPatch)
        {
            const SpriteInfo* pSprite = _sprites.Find(def.groundPatch.sprite);
            if (pSprite == nullptr)
            {
                return slug::String { std::format("{}: 스프라이트 '{}'", w, std::string { def.groundPatch.sprite.c_str() }) };
            }
            if (static_cast<float>(pSprite->width) != def.groundPatch.size.x || static_cast<float>(pSprite->height) != def.groundPatch.size.y)
            {
                return slug::String { std::format("{}: groundPatch 크기가 스프라이트와 다릅니다", w) };
            }
        }
    }
    for (const PropItemDef& item: m_items)
    {
        slug::String m;
        for (const PropVisual* v: { &item.visual, &item.begin, &item.pickup })
        {
            if (!(m = missing(*v, std::format("item {}", std::string { item.name.c_str() }))).empty())
            {
                return m;
            }
        }
    }
    return {};
}

}   // namespace slug

#include <algorithm>
#include "Base/Types.h"
#include "Base/Math.h"
#include <cmath>
#include <format>

#include "Base/Log.h"
#include "Base/Json.h"

#include "Base/Error.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Render/Animation.h"

namespace slug
{
class EnemyDefParser
{
public:
    explicit EnemyDefParser(EnemyDefLibrary& _library)
        : m_lib(_library)
    {
    }

    [[nodiscard]] bool Run(const slug::JsonReader& _root);
    [[nodiscard]] bool RunMany(slug::Span<const slug::JsonReader> _roots);
    [[nodiscard]] bool AddRoot_(const slug::JsonReader& _root);
    [[nodiscard]] bool Finish_();

    slug::String error = {};

private:
    using Chain   = slug::Vector<slug::JsonReader>;
    using Entries = slug::Vector<std::pair<slug::String, slug::JsonReader>>;

    struct RawDef
    {
        slug::String     name;
        slug::JsonReader reader;
    };

    [[nodiscard]] bool BuildChain_(slug::StringView _name, int _depth, Chain& _out);
    [[nodiscard]] bool CheckKeys_(const Chain& _chain, slug::StringView _defName, slug::Span<const char* const> _allowed);
    [[nodiscard]] bool CheckObjectKeys_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _object, slug::Span<const char* const> _allowed);

    [[nodiscard]] std::optional<slug::JsonReader> Find_(const Chain& _chain, slug::StringView _key) const;
    [[nodiscard]] Entries                        Merge_(const Chain& _chain, slug::StringView _key) const;

    void Fail_(slug::StringView _def, slug::StringView _field, slug::StringView _msg);
    [[nodiscard]] bool ReadNum_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, slug::StringView _key, float& _out, bool _bRequired);
    [[nodiscard]] bool ReadInt_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, slug::StringView _key, int& _out, bool _bRequired);
    [[nodiscard]] bool ReadBool_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, slug::StringView _key, bool& _out);
    [[nodiscard]] bool ReadStr_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, slug::StringView _key, slug::String& _out, bool _bRequired);
    [[nodiscard]] bool ReadVec2_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, slug::StringView _key, Vec2& _out);
    [[nodiscard]] bool ReadBox_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, BoxDef& _out);
    [[nodiscard]] bool ReadClip_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, ClipDef& _out);
    [[nodiscard]] bool ReadFx_(slug::StringView _def, slug::StringView _field, const slug::JsonReader& _reader, FxDef& _out);

    [[nodiscard]] bool ParseProjectile_(const RawDef& _raw, const Chain& _chain);
    [[nodiscard]] bool ParseEnemy_(const RawDef& _raw, const Chain& _chain);
    [[nodiscard]] bool ParseWeapon_(const slug::String& _def, const slug::String& _name, const slug::JsonReader& _reader, WeaponDef& _out);
    [[nodiscard]] bool ParseDeath_(EnemyDef& _def, const slug::JsonReader& _death);

    EnemyDefLibrary&                m_lib;
    slug::Vector<RawDef>             m_raw;
    slug::HashMap<slug::String, int>  m_rawIndex;
};

namespace
{
    constexpr const char* kRootKeys[] = { "note", "version", "globals", "defs" };
    constexpr const char* kGlobalKeys[] = { "note", "dpsRef", "activationMarginPx", "despawnBehindCameraPx", "despawnBelowCameraPx" };
    constexpr const char* kEnemyKeys[] = {
        "note", "extends", "abstract", "kind", "behavior", "team", "hp", "ttk", "score", "body", "hurtbox", "solid", "activation", "physics",
        "clips", "weapons", "params", "death", "drops", "overlay", "noDespawn", "human"
    };
    constexpr const char* kProjectileKeys[] = {
        "note", "extends", "abstract", "kind", "team", "lethal", "box", "damage", "speed", "speedRamp", "startDelay", "v0", "gravity", "maxFall",
        "lifetime", "onSolid", "anim", "explosion", "rollSpeed", "rollSec", "expireFx"
    };
    constexpr const char* kKnownKinds[] = { "walker", "vehicle", "turret", "flyer", "boss", "prop", "npc", "item", "projectile", "effect", "debris" };
    constexpr const char* kActivationKeys[] = { "note", "mode", "marginPx", "latch" };
    constexpr const char* kPhysicsKeys[] = { "note", "gravity", "maxFall", "speed", "jumpVy", "dropSpeed", "footHalfWidth", "useGravity" };
    constexpr const char* kWeaponKeys[] = { "note", "projectile", "muzzle", "aim", "tilt", "arc", "muzzleFx", "hitbox" };
    constexpr const char* kDeathKeys[] = {
        "note", "bomb", "slash", "stand1", "stand2", "fall", "land", "jump", "fallAt1", "fallAt2", "blinkAtSec", "blinkForSec", "blinkPeriodSec", "removeAtSec", "fx"
    };
    constexpr const char* kClipKeys[] = { "note", "name", "frames", "sec", "loop" };
    constexpr const char* kBoxKeys[] = { "note", "w", "h", "cx", "cy" };
    constexpr const char* kFxKeys[] = { "note", "clip", "offset", "delay", "layer" };
    constexpr const char* kSpeedRampKeys[] = { "note", "from", "accel", "max" };
    constexpr const char* kAnimKeys[] = { "note", "begin", "fly", "land" };
    constexpr const char* kExplosionKeys[] = { "note", "fx", "offset" };
    constexpr const char* kDropKeys[] = { "note", "item", "count" };
    constexpr const char* kOverlayKeys[] = { "note", "clip", "offset" };
    constexpr const char* kTiltKeys[] = { "note", "threshold", "dy" };
    constexpr const char* kArcKeys[] = { "note", "minVx", "maxVx" };
    constexpr const char* kHitboxKeys[] = { "note", "w", "h", "cx", "cy", "lifeSec", "damage", "follow" };

    [[nodiscard]] bool Contains_(
        const slug::Span<const char* const> _list,
        const slug::StringView              _key)
    {
        for (const char* pItem: _list)
        {
            if (_key == pItem)
            {
                return true;
            }
        }
        return false;
    }
}

void EnemyDefParser::Fail_(
    const slug::StringView _def,
    const slug::StringView _field,
    const slug::StringView _msg)
{
    if (error.empty())
    {
        error = _field.empty() ? std::format("{}: {}", _def, _msg) : std::format("{}: {}: {}", _def, _field, _msg);
    }
}

bool EnemyDefParser::ReadNum_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    const slug::StringView  _key,
    float&                 _out,
    const bool             _bRequired)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        if (_bRequired)
        {
            Fail_(_def, std::format("{}.{}", _field, _key), "필수 값이 없습니다");
        }
        return !_bRequired;
    }
    const slug::Result<double> value = _reader.ReadField<double>(_key);
    if (!value)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "숫자가 아닙니다");
        return false;
    }
    _out = static_cast<float>(value.GetValue());
    return true;
}

bool EnemyDefParser::ReadInt_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    const slug::StringView  _key,
    int&                   _out,
    const bool             _bRequired)
{
    float value = static_cast<float>(_out);
    if (!ReadNum_(_def, _field, _reader, _key, value, _bRequired))
    {
        return false;
    }
    _out = static_cast<int>(std::lround(static_cast<double>(value)));
    return true;
}

bool EnemyDefParser::ReadBool_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    const slug::StringView  _key,
    bool&                  _out)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        return true;
    }
    const slug::Result<bool> value = _reader.ReadField<bool>(_key);
    if (!value)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "true/false 가 아닙니다");
        return false;
    }
    _out = value.GetValue();
    return true;
}

bool EnemyDefParser::ReadStr_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    const slug::StringView  _key,
    slug::String&           _out,
    const bool             _bRequired)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        if (_bRequired)
        {
            Fail_(_def, std::format("{}.{}", _field, _key), "필수 값이 없습니다");
        }
        return !_bRequired;
    }
    const slug::Result<slug::String> value = _reader.ReadField<slug::String>(_key);
    if (!value)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "문자열이 아닙니다");
        return false;
    }
    _out = value.GetValue();
    return true;
}

bool EnemyDefParser::ReadVec2_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    const slug::StringView  _key,
    Vec2&          _out)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        return true;
    }
    const slug::Result<slug::JsonReader> array = _reader.FindField(_key);
    if (!array || !array->IsArray() || array->GetSize() != 2)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "[x, y] 배열이 아닙니다");
        return false;
    }
    float values[2] = {};
    int   i         = 0;
    for (const slug::JsonReader item: array->GetArray())
    {
        const slug::Result<double> value = item.Read<double>();
        if (!value)
        {
            Fail_(_def, std::format("{}.{}", _field, _key), "숫자가 아닌 원소가 있습니다");
            return false;
        }
        values[i++] = static_cast<float>(value.GetValue());
    }
    _out = { values[0], values[1] };
    return true;
}

bool EnemyDefParser::CheckObjectKeys_(
    const slug::StringView              _def,
    const slug::StringView              _field,
    const slug::JsonReader&             _object,
    const slug::Span<const char* const> _allowed)
{
    if (!_object.IsObject())
    {
        Fail_(_def, _field, "객체가 필요합니다");
        return false;
    }
    for (const auto [key, value]: _object.GetObject())
    {
        if (!Contains_(_allowed, key))
        {
            Fail_(_def, _field, std::format("알 수 없는 키 '{}'", key));
            return false;
        }
    }
    return true;
}

bool EnemyDefParser::ReadBox_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    BoxDef&                _out)
{
    if (!CheckObjectKeys_(_def, _field, _reader, kBoxKeys))
    {
        return false;
    }
    if (!ReadNum_(_def, _field, _reader, "w", _out.w, true) || !ReadNum_(_def, _field, _reader, "h", _out.h, true) || !ReadNum_(_def, _field, _reader, "cx", _out.cx, false) || !ReadNum_(_def, _field, _reader, "cy", _out.cy, false))
    {
        return false;
    }
    if (!(_out.w > 0.f) || !(_out.h > 0.f))
    {
        Fail_(_def, _field, "w, h 는 0 보다 커야 합니다");
        return false;
    }
    return true;
}

bool EnemyDefParser::ReadClip_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    ClipDef&               _out)
{
    if (!CheckObjectKeys_(_def, _field, _reader, kClipKeys))
    {
        return false;
    }
    if (!ReadStr_(_def, _field, _reader, "name", _out.name, true) || !ReadInt_(_def, _field, _reader, "frames", _out.frames, true) || !ReadNum_(_def, _field, _reader, "sec", _out.frameSec, true) || !ReadBool_(_def, _field, _reader, "loop", _out.bLoop))
    {
        return false;
    }
    if (_out.name.empty() || _out.frames < 1 || !(_out.frameSec > 0.f))
    {
        Fail_(_def, _field, "name 이 비었거나 frames < 1 또는 sec <= 0 입니다");
        return false;
    }
    _out.frameUnits = static_cast<int64_t>(std::llround(static_cast<double>(_out.frameSec) * static_cast<double>(kAnimTimeUnitsPerSec)));
    _out.bSided     = _out.name.find("{side}") != slug::String::npos;
    return true;
}

bool EnemyDefParser::ReadFx_(
    const slug::StringView  _def,
    const slug::StringView  _field,
    const slug::JsonReader& _reader,
    FxDef&                 _out)
{
    if (!CheckObjectKeys_(_def, _field, _reader, kFxKeys))
    {
        return false;
    }
    return ReadStr_(_def, _field, _reader, "clip", _out.clip, true) && ReadVec2_(_def, _field, _reader, "offset", _out.offset) && ReadNum_(_def, _field, _reader, "delay", _out.delaySec, false) && ReadInt_(_def, _field, _reader, "layer", _out.layer, false);
}

bool EnemyDefParser::BuildChain_(
    const slug::StringView _name,
    const int             _depth,
    Chain&                _out)
{
    if (_depth > 8)
    {
        Fail_(_name, "extends", "상속이 너무 깊거나 순환합니다");
        return false;
    }
    const auto it = m_rawIndex.find(slug::String { _name });
    if (it == m_rawIndex.end())
    {
        Fail_(_name, "extends", "존재하지 않는 정의입니다");
        return false;
    }
    const RawDef& raw = m_raw[static_cast<size_t>(it->second)];
    if (raw.reader.HasField("extends"))
    {
        const slug::Result<slug::String> parent = raw.reader.ReadField<slug::String>("extends");
        if (!parent)
        {
            Fail_(raw.name, "extends", "문자열이 아닙니다");
            return false;
        }
        if (!BuildChain_(parent.GetValue(), _depth + 1, _out))
        {
            return false;
        }
    }
    _out.push_back(raw.reader);
    return true;
}

bool EnemyDefParser::CheckKeys_(
    const Chain&                       _chain,
    const slug::StringView              _defName,
    const slug::Span<const char* const> _allowed)
{
    for (const slug::JsonReader& reader: _chain)
    {
        if (!CheckObjectKeys_(_defName, "", reader, _allowed))
        {
            return false;
        }
    }
    return true;
}

std::optional<slug::JsonReader> EnemyDefParser::Find_(
    const Chain&          _chain,
    const slug::StringView _key) const
{
    for (size_t i = _chain.size(); i > 0; --i)
    {
        if (_chain[i - 1].HasField(_key))
        {
            const slug::Result<slug::JsonReader> field = _chain[i - 1].FindField(_key);
            if (field)
            {
                return field.GetValue();
            }
        }
    }
    return std::nullopt;
}

EnemyDefParser::Entries EnemyDefParser::Merge_(
    const Chain&          _chain,
    const slug::StringView _key) const
{
    Entries entries;
    for (const slug::JsonReader& reader: _chain)
    {
        if (!reader.HasField(_key))
        {
            continue;
        }
        const slug::Result<slug::JsonReader> field = reader.FindField(_key);
        if (!field || !field->IsObject())
        {
            continue;
        }
        for (const auto [name, value]: field->GetObject())
        {
            const auto found = std::find_if(entries.begin(), entries.end(), [&](const auto& _e) { return _e.first == name; });
            if (found != entries.end())
            {
                found->second = value;
            }
            else
            {
                entries.emplace_back(slug::String { name }, value);
            }
        }
    }
    return entries;
}

bool EnemyDefParser::ParseProjectile_(
    const RawDef& _raw,
    const Chain&  _chain)
{
    const slug::String& name = _raw.name;
    if (!CheckKeys_(_chain, name, kProjectileKeys))
    {
        return false;
    }

    ProjectileDef projectile;
    projectile.name = name;

    const auto Obj = [&](const slug::StringView _key) { return Find_(_chain, _key); };

    const std::optional<slug::JsonReader> box = Obj("box");
    if (!box)
    {
        Fail_(name, "box", "필수 값이 없습니다");
        return false;
    }
    {
        BoxDef boxDef;
        if (!ReadBox_(name, "box", *box, boxDef))
        {
            return false;
        }
        projectile.size = { boxDef.w, boxDef.h };
    }

    float lifetime = projectile.lifetimeSec;
    float delay    = 0.f;
    for (const slug::JsonReader& reader: _chain)
    {
        if (!ReadInt_(name, "", reader, "damage", projectile.damage, false) || !ReadNum_(name, "", reader, "speed", projectile.speed, false) || !ReadNum_(name, "", reader, "gravity", projectile.gravity, false) || !ReadNum_(name, "", reader, "maxFall", projectile.maxFall, false) || !ReadNum_(name, "", reader, "lifetime", lifetime, false) || !ReadNum_(name, "", reader, "startDelay", delay, false) || !ReadVec2_(name, "", reader, "v0", projectile.v0))
        {
            return false;
        }
    }
    projectile.lifetimeSec   = lifetime;
    projectile.startDelaySec = delay;

    if (const std::optional<slug::JsonReader> ramp = Obj("speedRamp"))
    {
        if (!CheckObjectKeys_(name, "speedRamp", *ramp, kSpeedRampKeys) || !ReadNum_(name, "speedRamp", *ramp, "from", projectile.rampFrom, false) || !ReadNum_(name, "speedRamp", *ramp, "accel", projectile.rampAccel, true) || !ReadNum_(name, "speedRamp", *ramp, "max", projectile.rampMax, true))
        {
            return false;
        }
        projectile.bRamp = true;
    }

    if (const std::optional<slug::JsonReader> onSolid = Obj("onSolid"))
    {
        const slug::Result<slug::String> mode = onSolid->Read<slug::String>();
        if (!mode || (mode.GetValue() != "explode" && mode.GetValue() != "none"))
        {
            Fail_(name, "onSolid", "\"explode\" 또는 \"none\" 이어야 합니다");
            return false;
        }
        projectile.bExplodeOnSolid = mode.GetValue() == "explode";
    }

    const std::optional<slug::JsonReader> anim = Obj("anim");
    if (!anim || !CheckObjectKeys_(name, "anim", *anim, kAnimKeys))
    {
        Fail_(name, "anim", "{fly[, begin]} 가 필요합니다");
        return false;
    }
    if (!anim->HasField("fly"))
    {
        Fail_(name, "anim.fly", "필수 값이 없습니다");
        return false;
    }
    {
        const slug::Result<slug::JsonReader> fly = anim->FindField("fly");
        if (!fly || !ReadClip_(name, "anim.fly", fly.GetValue(), projectile.fly))
        {
            return false;
        }
        if (anim->HasField("begin"))
        {
            const slug::Result<slug::JsonReader> begin = anim->FindField("begin");
            if (!begin || !ReadClip_(name, "anim.begin", begin.GetValue(), projectile.begin))
            {
                return false;
            }
        }
        if (anim->HasField("land"))
        {
            const slug::Result<slug::JsonReader> land = anim->FindField("land");
            if (!land || !ReadClip_(name, "anim.land", land.GetValue(), projectile.land))
            {
                return false;
            }
        }
    }

    if (const std::optional<slug::JsonReader> explosion = Obj("explosion"))
    {
        if (!CheckObjectKeys_(name, "explosion", *explosion, kExplosionKeys) || !ReadStr_(name, "explosion", *explosion, "fx", projectile.explosion.clip, true) || !ReadVec2_(name, "explosion", *explosion, "offset", projectile.explosion.offset))
        {
            return false;
        }
    }

    for (const slug::JsonReader& reader: _chain)
    {
        if (!ReadNum_(name, "", reader, "rollSpeed", projectile.rollSpeed, false) || !ReadNum_(name, "", reader, "rollSec", projectile.rollSec, false) || !ReadBool_(name, "", reader, "expireFx", projectile.bExpireFx))
        {
            return false;
        }
    }

    const bool bMoves = projectile.speed > 0.f || projectile.bRamp || projectile.v0.x != 0.f || projectile.v0.y != 0.f || projectile.gravity != 0.f;
    if (!bMoves)
    {
        Fail_(name, "speed", "speed / speedRamp / v0 / gravity 중 하나는 있어야 합니다 (움직이지 않는 투사체)");
        return false;
    }

    m_lib.m_projIndex[name] = static_cast<int>(m_lib.m_projectiles.size());
    m_lib.m_projectiles.push_back(std::move(projectile));
    return true;
}

bool EnemyDefParser::ParseWeapon_(
    const slug::String&     _def,
    const slug::String&     _name,
    const slug::JsonReader& _reader,
    WeaponDef&             _out)
{
    const slug::String field = std::format("weapons.{}", _name);
    if (!CheckObjectKeys_(_def, field, _reader, kWeaponKeys))
    {
        return false;
    }
    _out.name = _name;

    slug::String projectileName;
    if (!ReadStr_(_def, field, _reader, "projectile", projectileName, false) || !ReadVec2_(_def, field, _reader, "muzzle", _out.muzzle))
    {
        return false;
    }
    if (!projectileName.empty())
    {
        _out.projectile = m_lib.FindProjectileIndex(projectileName);
        if (_out.projectile < 0)
        {
            Fail_(_def, field + ".projectile", std::format("'{}' 투사체 정의가 없습니다", projectileName));
            return false;
        }
    }

    slug::String aim;
    if (!ReadStr_(_def, field, _reader, "aim", aim, false))
    {
        return false;
    }
    if (!aim.empty())
    {
        if (aim == "facing")
        {
            _out.aim = eWeaponAim::Facing;
        }
        else if (aim == "given")
        {
            _out.aim = eWeaponAim::Given;
        }
        else
        {
            Fail_(_def, field + ".aim", "\"facing\" 또는 \"given\" 이어야 합니다");
            return false;
        }
    }

    if (_reader.HasField("tilt"))
    {
        const slug::Result<slug::JsonReader> tilt = _reader.FindField("tilt");
        if (!tilt || !CheckObjectKeys_(_def, field + ".tilt", tilt.GetValue(), kTiltKeys) || !ReadNum_(_def, field + ".tilt", tilt.GetValue(), "threshold", _out.tiltThreshold, true) || !ReadNum_(_def, field + ".tilt", tilt.GetValue(), "dy", _out.tiltDy, true))
        {
            return false;
        }
    }
    if (_reader.HasField("arc"))
    {
        const slug::Result<slug::JsonReader> arc = _reader.FindField("arc");
        if (!arc || !CheckObjectKeys_(_def, field + ".arc", arc.GetValue(), kArcKeys) || !ReadNum_(_def, field + ".arc", arc.GetValue(), "minVx", _out.arcMinVx, true) || !ReadNum_(_def, field + ".arc", arc.GetValue(), "maxVx", _out.arcMaxVx, true))
        {
            return false;
        }
        _out.bArc = true;
    }
    if (_reader.HasField("muzzleFx"))
    {
        const slug::Result<slug::JsonReader> fx = _reader.FindField("muzzleFx");
        if (!fx || !ReadFx_(_def, field + ".muzzleFx", fx.GetValue(), _out.muzzleFx))
        {
            return false;
        }
    }
    if (_reader.HasField("hitbox"))
    {
        const slug::Result<slug::JsonReader> hitbox = _reader.FindField("hitbox");
        if (!hitbox || !CheckObjectKeys_(_def, field + ".hitbox", hitbox.GetValue(), kHitboxKeys))
        {
            return false;
        }
        BoxDef box;
        if (!ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "w", box.w, true) || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "h", box.h, true) || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "cx", box.cx, false) || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "cy", box.cy, false) || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "lifeSec", _out.hitboxLifeSec, true) || !ReadInt_(_def, field + ".hitbox", hitbox.GetValue(), "damage", _out.hitboxDamage, false) || !ReadBool_(_def, field + ".hitbox", hitbox.GetValue(), "follow", _out.bHitboxFollows))
        {
            return false;
        }
        _out.hitbox   = box;
        _out.bHitbox  = true;
    }
    if (_out.projectile < 0 && !_out.bHitbox)
    {
        Fail_(_def, field, "projectile 또는 hitbox 가 필요합니다");
        return false;
    }
    return true;
}

bool EnemyDefParser::ParseDeath_(
    EnemyDef&              _def,
    const slug::JsonReader& _death)
{
    const slug::String& name = _def.name;
    if (!CheckObjectKeys_(name, "death", _death, kDeathKeys))
    {
        return false;
    }

    DeathDef& death = _def.death;

    const auto Clip = [&](const char* _key, int& _out) {
        if (!_death.HasField(_key))
        {
            return true;
        }
        slug::String logical;
        if (!ReadStr_(name, "death", _death, _key, logical, true))
        {
            return false;
        }
        _out = _def.FindClip(logical);
        if (_out < 0)
        {
            Fail_(name, std::format("death.{}", _key), std::format("'{}' 클립이 clips 에 없습니다", logical));
            return false;
        }
        return true;
    };

    if (!Clip("bomb", death.clipBomb) || !Clip("slash", death.clipSlash) || !Clip("stand1", death.clipStand1) || !Clip("stand2", death.clipStand2) || !Clip("fall", death.clipFall) || !Clip("land", death.clipLand) || !Clip("jump", death.clipJump))
    {
        return false;
    }
    if (!ReadInt_(name, "death", _death, "fallAt1", death.fallAt1, false) || !ReadInt_(name, "death", _death, "fallAt2", death.fallAt2, false) || !ReadNum_(name, "death", _death, "blinkAtSec", death.blinkAtSec, false) || !ReadNum_(name, "death", _death, "blinkForSec", death.blinkForSec, false) || !ReadNum_(name, "death", _death, "blinkPeriodSec", death.blinkPeriodSec, false) || !ReadNum_(name, "death", _death, "removeAtSec", death.removeAtSec, false))
    {
        return false;
    }

    if (_death.HasField("fx"))
    {
        const slug::Result<slug::JsonReader> fx = _death.FindField("fx");
        if (!fx || !fx->IsArray())
        {
            Fail_(name, "death.fx", "배열이 아닙니다");
            return false;
        }
        int index = 0;
        for (const slug::JsonReader item: fx->GetArray())
        {
            FxDef def;
            if (!ReadFx_(name, std::format("death.fx[{}]", index++), item, def))
            {
                return false;
            }
            death.fx.push_back(std::move(def));
        }
    }
    return true;
}

bool EnemyDefParser::ParseEnemy_(
    const RawDef& _raw,
    const Chain&  _chain)
{
    const slug::String& name = _raw.name;
    if (!CheckKeys_(_chain, name, kEnemyKeys))
    {
        return false;
    }

    EnemyDef def;
    def.name = name;

    slug::String kind;
    slug::String behaviorName;
    slug::String teamName;
    for (const slug::JsonReader& reader: _chain)
    {
        if (!ReadStr_(name, "", reader, "kind", kind, false) || !ReadStr_(name, "", reader, "behavior", behaviorName, false) || !ReadStr_(name, "", reader, "team", teamName, false))
        {
            return false;
        }
    }
    if (kind.empty())
    {
        Fail_(name, "kind", "필수 값이 없습니다");
        return false;
    }
    {
        bool bKnown = false;
        for (const char* pKind: kKnownKinds)
        {
            bKnown = bKnown || kind == pKind;
        }
        if (!bKnown)
        {
            Fail_(name, "kind", std::format("알 수 없는 종류 '{}'", kind));
            return false;
        }
    }
    if (behaviorName.empty())
    {
        Fail_(name, "behavior", "필수 값이 없습니다");
        return false;
    }
    const std::optional<eEnemyBehavior> behavior = FindBehavior(behaviorName);
    if (!behavior)
    {
        Fail_(name, "behavior", std::format("알 수 없는 행동 '{}'", behaviorName));
        return false;
    }
    def.behavior = *behavior;
    if (teamName.empty())
    {
        teamName = kind == "npc" ? "neutral" : "enemy";
    }
    if (teamName == "enemy")
    {
        def.team = eTeam::Enemy;
    }
    else if (teamName == "neutral")
    {
        def.team = eTeam::Neutral;
    }
    else
    {
        Fail_(name, "team", std::format("알 수 없는 팀 '{}' (enemy / neutral)", teamName));
        return false;
    }

    def.activation.marginPx = m_lib.m_globals.activationMarginPx;
    float ttk               = 0.f;
    for (const slug::JsonReader& reader: _chain)
    {
        if (!ReadInt_(name, "", reader, "hp", def.hp, false) || !ReadInt_(name, "", reader, "score", def.score, false) || !ReadNum_(name, "", reader, "ttk", ttk, false))
        {
            return false;
        }
    }
    if (!Find_(_chain, "hp"))
    {
        Fail_(name, "hp", "필수 값이 없습니다");
        return false;
    }
    if (def.hp < 1)
    {
        Fail_(name, "hp", "1 이상이어야 합니다");
        return false;
    }
    if (ttk > 0.f && std::abs(static_cast<float>(def.hp) - ttk * m_lib.m_globals.dpsRef) > 0.05f * static_cast<float>(def.hp))
    {
        SLUG_LOG_WARN("{}: hp({}) 가 ttk({}) x dpsRef({}) 와 5% 이상 다릅니다", name, def.hp, ttk, m_lib.m_globals.dpsRef);
    }

    const std::optional<slug::JsonReader> body = Find_(_chain, "body");
    if (!body || !ReadBox_(name, "body", *body, def.body))
    {
        if (!body)
        {
            Fail_(name, "body", "필수 값이 없습니다");
        }
        return false;
    }
    def.hurtbox = def.body;
    if (const std::optional<slug::JsonReader> hurtbox = Find_(_chain, "hurtbox"))
    {
        def.hurtbox = BoxDef {};
        if (!ReadBox_(name, "hurtbox", *hurtbox, def.hurtbox))
        {
            return false;
        }
    }
    if (const std::optional<slug::JsonReader> solid = Find_(_chain, "solid"))
    {
        if (!ReadBox_(name, "solid", *solid, def.solid))
        {
            return false;
        }
    }

    if (const std::optional<slug::JsonReader> activation = Find_(_chain, "activation"))
    {
        slug::String mode;
        if (!CheckObjectKeys_(name, "activation", *activation, kActivationKeys) || !ReadStr_(name, "activation", *activation, "mode", mode, false) || !ReadNum_(name, "activation", *activation, "marginPx", def.activation.marginPx, false) || !ReadBool_(name, "activation", *activation, "latch", def.activation.bLatch))
        {
            return false;
        }
        if (mode == "always")
        {
            def.activation.bAlways = true;
        }
        else if (!mode.empty() && mode != "screen")
        {
            Fail_(name, "activation.mode", "\"screen\" 또는 \"always\" 이어야 합니다");
            return false;
        }
    }

    if (const std::optional<slug::JsonReader> physics = Find_(_chain, "physics"))
    {
        PhysicsDef& p = def.physics;
        if (!CheckObjectKeys_(name, "physics", *physics, kPhysicsKeys) || !ReadNum_(name, "physics", *physics, "gravity", p.gravity, false) || !ReadNum_(name, "physics", *physics, "maxFall", p.maxFall, false) || !ReadNum_(name, "physics", *physics, "speed", p.speed, false) || !ReadNum_(name, "physics", *physics, "jumpVy", p.jumpVy, false) || !ReadNum_(name, "physics", *physics, "dropSpeed", p.dropSpeed, false) || !ReadNum_(name, "physics", *physics, "footHalfWidth", p.footHalfWidth, false) || !ReadBool_(name, "physics", *physics, "useGravity", p.bGravity))
        {
            return false;
        }
    }

    for (const auto& [logical, reader]: Merge_(_chain, "clips"))
    {
        NamedClip clip;
        clip.name = logical;
        if (!ReadClip_(name, std::format("clips.{}", logical), reader, clip.clip))
        {
            return false;
        }
        def.clips.push_back(std::move(clip));
    }
    for (const auto& [weaponName, reader]: Merge_(_chain, "weapons"))
    {
        WeaponDef weapon;
        if (!ParseWeapon_(name, weaponName, reader, weapon))
        {
            return false;
        }
        def.weapons.push_back(std::move(weapon));
    }

    if (const std::optional<slug::JsonReader> death = Find_(_chain, "death"))
    {
        if (!ParseDeath_(def, *death))
        {
            return false;
        }
    }

    if (const std::optional<slug::JsonReader> drops = Find_(_chain, "drops"))
    {
        if (!drops->IsArray())
        {
            Fail_(name, "drops", "배열이 아닙니다");
            return false;
        }
        int index = 0;
        for (const slug::JsonReader item: drops->GetArray())
        {
            DropDef drop;
            const slug::String field = std::format("drops[{}]", index++);
            if (!CheckObjectKeys_(name, field, item, kDropKeys) || !ReadStr_(name, field, item, "item", drop.item, true) || !ReadInt_(name, field, item, "count", drop.count, false))
            {
                return false;
            }
            def.drops.push_back(std::move(drop));
        }
    }

    if (const std::optional<slug::JsonReader> overlay = Find_(_chain, "overlay"))
    {
        slug::String clip;
        if (!CheckObjectKeys_(name, "overlay", *overlay, kOverlayKeys) || !ReadStr_(name, "overlay", *overlay, "clip", clip, true) || !ReadVec2_(name, "overlay", *overlay, "offset", def.overlayOffset))
        {
            return false;
        }
        def.overlayClip = def.FindClip(clip);
        if (def.overlayClip < 0)
        {
            Fail_(name, "overlay.clip", std::format("'{}' 클립이 clips 에 없습니다", clip));
            return false;
        }
    }

    for (const slug::JsonReader& reader: _chain)
    {
        if (!ReadBool_(name, "", reader, "noDespawn", def.bNoDespawn) || !ReadBool_(name, "", reader, "human", def.bHuman))
        {
            return false;
        }
    }

    slug::Vector<ParamEntry> params;
    for (auto& [key, reader]: Merge_(_chain, "params"))
    {
        params.emplace_back(key, reader);
    }
    {
        slug::String message;
        DefReader   reader { name, def, params, message };
        switch (def.behavior)
        {
            case eEnemyBehavior::Soldier: ParseSoldierCfg(reader, def); break;
            case eEnemyBehavior::Bzk: ParseBzkCfg(reader, def); break;
            case eEnemyBehavior::Hang: ParseHangCfg(reader, def); break;
            case eEnemyBehavior::FlyingBzk: ParseFlyingBzkCfg(reader, def); break;
            case eEnemyBehavior::Shielder: ParseShielderCfg(reader, def); break;
            case eEnemyBehavior::Pow: ParsePowCfg(reader, def); break;
            case eEnemyBehavior::Bus: ParseBusCfg(reader, def); break;
            case eEnemyBehavior::BigShiee: ParseBigShieeCfg(reader, def); break;
            case eEnemyBehavior::Jeep: ParseJeepCfg(reader, def); break;
            case eEnemyBehavior::BTank: ParseBTankCfg(reader, def); break;
            case eEnemyBehavior::MissleTank: ParseMissleTankCfg(reader, def); break;
            case eEnemyBehavior::Sarubia: ParseSarubiaCfg(reader, def); break;
            case eEnemyBehavior::Aparch: ParseAparchCfg(reader, def); break;
            case eEnemyBehavior::Count: break;
        }
        if (!reader.Finish())
        {
            if (error.empty())
            {
                error = message;
            }
            return false;
        }
    }

    m_lib.m_defIndex[name] = static_cast<int>(m_lib.m_defs.size());
    m_lib.m_defs.push_back(std::move(def));
    return true;
}

bool EnemyDefParser::Run(
    const slug::JsonReader& _root)
{
    return RunMany(slug::Span<const slug::JsonReader> { &_root, 1 });
}

bool EnemyDefParser::RunMany(
    const slug::Span<const slug::JsonReader> _roots)
{
    for (const slug::JsonReader& root: _roots)
    {
        if (!AddRoot_(root))
        {
            return false;
        }
    }
    return Finish_();
}

bool EnemyDefParser::AddRoot_(
    const slug::JsonReader& _root)
{
    constexpr const char* kRoot = "Enemies.json";
    if (!CheckObjectKeys_(kRoot, "", _root, kRootKeys))
    {
        return false;
    }

    float version = 1.f;
    if (!ReadNum_(kRoot, "", _root, "version", version, false))
    {
        return false;
    }
    if (version != 1.f)
    {
        Fail_(kRoot, "version", "지원하지 않는 버전입니다 (1 만 지원)");
        return false;
    }

    if (_root.HasField("globals"))
    {
        const slug::Result<slug::JsonReader> globals = _root.FindField("globals");
        EnemyGlobals&                      g       = m_lib.m_globals;
        if (!globals || !CheckObjectKeys_(kRoot, "globals", globals.GetValue(), kGlobalKeys) || !ReadNum_(kRoot, "globals", globals.GetValue(), "dpsRef", g.dpsRef, false) || !ReadNum_(kRoot, "globals", globals.GetValue(), "activationMarginPx", g.activationMarginPx, false) || !ReadNum_(kRoot, "globals", globals.GetValue(), "despawnBehindCameraPx", g.despawnBehindCameraPx, false) || !ReadNum_(kRoot, "globals", globals.GetValue(), "despawnBelowCameraPx", g.despawnBelowCameraPx, false))
        {
            return false;
        }
    }

    const slug::Result<slug::JsonReader> defs = _root.FindField("defs");
    if (!defs || !defs->IsObject())
    {
        Fail_(kRoot, "defs", "객체가 필요합니다");
        return false;
    }
    for (const auto [name, reader]: defs->GetObject())
    {
        if (m_rawIndex.contains(slug::String { name }))
        {
            Fail_(name, "", "정의 이름이 중복되었습니다");
            return false;
        }
        m_rawIndex[slug::String { name }] = static_cast<int>(m_raw.size());
        m_raw.push_back({ slug::String { name }, reader });
    }
    return true;
}

bool EnemyDefParser::Finish_()
{
    for (int pass = 0; pass < 2; ++pass)
    {
        for (const RawDef& raw: m_raw)
        {
            Chain chain;
            if (!BuildChain_(raw.name, 0, chain))
            {
                return false;
            }
            bool bAbstract = false;
            if (!ReadBool_(raw.name, "", raw.reader, "abstract", bAbstract))
            {
                return false;
            }
            if (bAbstract)
            {
                continue;
            }

            slug::String kind;
            for (const slug::JsonReader& reader: chain)
            {
                if (!ReadStr_(raw.name, "", reader, "kind", kind, false))
                {
                    return false;
                }
            }
            const bool bProjectile = kind == "projectile";
            if (bProjectile != (pass == 0))
            {
                continue;
            }
            if (!(bProjectile ? ParseProjectile_(raw, chain) : ParseEnemy_(raw, chain)))
            {
                return false;
            }
        }
    }

    for (const EnemyDef& def: m_lib.m_defs)
    {
        if (def.behavior == eEnemyBehavior::FlyingBzk && m_lib.FindIndex(def.flyingBzk.landedDef) < 0)
        {
            Fail_(def.name, "params.landedDef", std::format("'{}' 정의가 없습니다", def.flyingBzk.landedDef));
            return false;
        }
    }
    return true;
}

slug::Result<EnemyDefLibrary> EnemyDefLibrary::Parse(
    const slug::StringView _json,
    slug::String*          _pMessage)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::Load(_json, slug::eJsonLoadOption::AllowComments);
    if (!doc)
    {
        if (_pMessage)
        {
            *_pMessage = "Enemies.json: JSON 구문 오류";
        }
        SLUG_LOG_ERROR("Enemies.json: JSON 구문 오류");
        return slug::Error { eError::Parse };
    }

    EnemyDefLibrary library;
    EnemyDefParser  parser { library };
    if (!parser.Run(doc->GetReader()))
    {
        if (_pMessage)
        {
            *_pMessage = parser.error;
        }
        SLUG_LOG_ERROR("적 정의 오류: {}", parser.error);
        return slug::Error { eError::EnemyDef };
    }
    return library;
}

slug::Result<EnemyDefLibrary> EnemyDefLibrary::Load(
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
        SLUG_LOG_ERROR("적 정의 파일 로드 실패: {}", _path.string());
        return slug::Error { eError::FileOpen };
    }

    EnemyDefLibrary library;
    EnemyDefParser  parser { library };
    if (!parser.Run(doc->GetReader()))
    {
        if (_pMessage)
        {
            *_pMessage = parser.error;
        }
        SLUG_LOG_ERROR("적 정의 오류 ({}): {}", _path.string(), parser.error);
        return slug::Error { eError::EnemyDef };
    }
    return library;
}

slug::Result<EnemyDefLibrary> EnemyDefLibrary::LoadDirectory(
    const slug::FilePath& _dir,
    slug::String*         _pMessage)
{
    slug::Vector<slug::FilePath> files;
    std::error_code            ec;
    for (const auto& entry: std::filesystem::directory_iterator(_dir, ec))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            files.push_back(entry.path());
        }
    }
    if (ec || files.empty())
    {
        if (_pMessage)
        {
            *_pMessage = std::format("{}: 정의 파일(*.json)을 찾을 수 없습니다", _dir.string());
        }
        SLUG_LOG_ERROR("적 정의 폴더 로드 실패: {}", _dir.string());
        return slug::Error { eError::FileOpen };
    }
    std::sort(files.begin(), files.end());

    slug::Vector<slug::JsonDeserializer> docs;
    docs.reserve(files.size());
    slug::Vector<slug::JsonReader> roots;
    roots.reserve(files.size());
    for (const slug::FilePath& file: files)
    {
        slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::LoadFromFile(file, slug::eJsonLoadOption::AllowComments);
        if (!doc)
        {
            if (_pMessage)
            {
                *_pMessage = std::format("{}: 파일을 열 수 없거나 JSON 구문 오류", file.string());
            }
            SLUG_LOG_ERROR("적 정의 파일 로드 실패: {}", file.string());
            return slug::Error { eError::FileOpen };
        }
        docs.push_back(doc.Take());
        roots.push_back(docs.back().GetReader());
    }

    EnemyDefLibrary library;
    EnemyDefParser  parser { library };
    if (!parser.RunMany(roots))
    {
        if (_pMessage)
        {
            *_pMessage = parser.error;
        }
        SLUG_LOG_ERROR("적 정의 오류 ({}): {}", _dir.string(), parser.error);
        return slug::Error { eError::EnemyDef };
    }
    return library;
}

slug::FilePath EnemyDefLibrary::GetDefaultPath(
    const slug::FilePath& _assetRoot)
{
    return _assetRoot / "Defs" / "Enemies.json";
}

slug::FilePath EnemyDefLibrary::GetDefaultDirectory(
    const slug::FilePath& _assetRoot)
{
    return _assetRoot / "Defs";
}

const EnemyDef* EnemyDefLibrary::Find(
    const slug::StringView _name) const
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_defs[static_cast<size_t>(index)];
}

int EnemyDefLibrary::FindIndex(
    const slug::StringView _name) const
{
    const auto it = m_defIndex.find(slug::String { _name });
    return it == m_defIndex.end() ? -1 : it->second;
}

const ProjectileDef* EnemyDefLibrary::FindProjectile(
    const slug::StringView _name) const
{
    const int index = FindProjectileIndex(_name);
    return index < 0 ? nullptr : &m_projectiles[static_cast<size_t>(index)];
}

int EnemyDefLibrary::FindProjectileIndex(
    const slug::StringView _name) const
{
    const auto it = m_projIndex.find(slug::String { _name });
    return it == m_projIndex.end() ? -1 : it->second;
}
}

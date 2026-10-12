#include <algorithm>
#include <cmath>
#include <format>

#include "Base/SlugError.h"
#include "Base/Json.h"
#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Render/Animation.h"

class EnemyDefParser
{
public:
    explicit EnemyDefParser(EnemyDefLibrary& _library);

    [[nodiscard]] bool Run(const JsonReader& _root);
    [[nodiscard]] bool RunMany(Span<const JsonReader> _roots);

    String error = {};

private:
    using Chain   = Vector<JsonReader>;
    using Entry   = std::pair<String, JsonReader>;
    using Entries = Vector<Entry>;
    using Member  = std::pair<StringView, JsonReader>;

    struct RawDef
    {
        String     name = {};
        JsonReader reader;
    };

    [[nodiscard]] bool AddRoot_(const JsonReader& _root);
    [[nodiscard]] bool Finish_();

    [[nodiscard]] bool BuildChain_(StringView _name, int _depth, Chain& _out);
    [[nodiscard]] bool CheckKeys_(const Chain& _chain, StringView _defName, Span<const char* const> _allowed);

    [[nodiscard]] bool CheckObjectKeys_(
        StringView              _def,
        StringView              _field,
        const JsonReader&       _object,
        Span<const char* const> _allowed);

    [[nodiscard]] std::optional<JsonReader> Find_(const Chain& _chain, StringView _key) const;
    [[nodiscard]] Entries                   Merge_(const Chain& _chain, StringView _key) const;

    void Fail_(StringView _def, StringView _field, StringView _msg);

    [[nodiscard]] bool ReadNum_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        StringView        _key,
        float&            _out,
        bool              _bRequired);

    [[nodiscard]] bool ReadInt_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        StringView        _key,
        int&              _out,
        bool              _bRequired);

    [[nodiscard]] bool ReadBool_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        StringView        _key,
        bool&             _out);

    [[nodiscard]] bool ReadStr_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        StringView        _key,
        String&           _out,
        bool              _bRequired);

    [[nodiscard]] bool ReadVec2_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        StringView        _key,
        Vec2&             _out);

    [[nodiscard]] bool ReadBox_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        BoxDef&           _out);

    [[nodiscard]] bool ReadClip_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        ClipDef&          _out);

    [[nodiscard]] bool ReadFx_(
        StringView        _def,
        StringView        _field,
        const JsonReader& _reader,
        FxDef&            _out);

    [[nodiscard]] bool ParseProjectile_(const RawDef& _raw, const Chain& _chain);
    [[nodiscard]] bool ParseEnemy_(const RawDef& _raw, const Chain& _chain);

    [[nodiscard]] bool ParseWeapon_(
        const String&     _def,
        const String&     _name,
        const JsonReader& _reader,
        WeaponDef&        _out);

    [[nodiscard]] bool ParseDeath_(EnemyDef& _def, const JsonReader& _death);

    [[nodiscard]] bool ReadDeathClip_(
        const EnemyDef&   _def,
        const JsonReader& _death,
        StringView        _key,
        int&              _out);

    EnemyDefLibrary&     m_lib;
    Vector<RawDef>       m_raw      = {};
    HashMap<String, int> m_rawIndex = {};
};

constexpr ARRAY<const char*, 4>  kRootKeys   = { "note", "version", "globals", "defs" };
constexpr ARRAY<const char*, 5>  kGlobalKeys = { "note", "dpsRef", "activationMarginPx", "despawnBehindCameraPx", "despawnBelowCameraPx" };
constexpr ARRAY<const char*, 27> kEnemyKeys  = {
    "note",
    "extends",
    "abstract",
    "kind",
    "behavior",
    "team",
    "hp",
    "ttk",
    "score",
    "body",
    "hurtbox",
    "hitboxes",
    "solid",
    "activation",
    "physics",
    "clips",
    "weapons",
    "params",
    "death",
    "drops",
    "overlay",
    "noOverlay",
    "noDespawn",
    "human",
    "hitFlash",
    "centerAnchor",
    "artFacing"
};
constexpr ARRAY<const char*, 27> kProjectileKeys = {
    "note",
    "extends",
    "abstract",
    "kind",
    "team",
    "lethal",
    "box",
    "damage",
    "speed",
    "speedRamp",
    "startDelay",
    "v0",
    "gravity",
    "maxFall",
    "lifetime",
    "onSolid",
    "anim",
    "explosion",
    "rollSpeed",
    "rollSec",
    "expireFx",
    "groundProbe",
    "trail",
    "trailSec",
    "blast",
    "rotate",
    "scale"
};
constexpr ARRAY<const char*, 11> kKnownKinds       = { "walker", "vehicle", "turret", "flyer", "boss", "prop", "npc", "item", "projectile", "effect", "debris" };
constexpr ARRAY<const char*, 4>  kActivationKeys   = { "note", "mode", "marginPx", "latch" };
constexpr ARRAY<const char*, 8>  kPhysicsKeys      = { "note", "gravity", "maxFall", "speed", "jumpVy", "dropSpeed", "footHalfWidth", "useGravity" };
constexpr ARRAY<const char*, 10> kWeaponKeys       = { "note", "projectile", "muzzle", "aim", "tilt", "arc", "muzzleFx", "hitbox", "motionClip", "effects" };
constexpr ARRAY<const char*, 5>  kWeaponEffectKeys = { "note", "clip", "offset", "frame", "depth" };
constexpr ARRAY<const char*, 17> kDeathKeys        = {
    "note",
    "bomb",
    "slash",
    "stand1",
    "stand2",
    "fall",
    "land",
    "jump",
    "fallAt1",
    "fallAt2",
    "blinkAtSec",
    "blinkForSec",
    "blinkPeriodSec",
    "removeAtSec",
    "bloodOffsetY",
    "bombVelocity",
    "fx"
};
constexpr ARRAY<const char*, 5> kClipKeys      = { "note", "name", "frames", "sec", "loop" };
constexpr ARRAY<const char*, 5> kBoxKeys       = { "note", "w", "h", "cx", "cy" };
constexpr ARRAY<const char*, 7> kFxKeys        = { "note", "clip", "offset", "delay", "depth", "debris", "mirror" };
constexpr ARRAY<const char*, 4> kSpeedRampKeys = { "note", "from", "accel", "max" };
constexpr ARRAY<const char*, 4> kAnimKeys      = { "note", "begin", "fly", "land" };
constexpr ARRAY<const char*, 4> kExplosionKeys = { "note", "fx", "offset", "sfx" };
constexpr ARRAY<const char*, 6> kBlastKeys     = { "note", "w", "h", "up", "sec", "damage" };
constexpr ARRAY<const char*, 3> kDropKeys      = { "note", "item", "count" };
constexpr ARRAY<const char*, 3> kOverlayKeys   = { "note", "clip", "offset" };
constexpr ARRAY<const char*, 3> kTiltKeys      = { "note", "threshold", "dy" };
constexpr ARRAY<const char*, 3> kArcKeys       = { "note", "minVx", "maxVx" };
constexpr ARRAY<const char*, 8> kExtraBoxKeys  = { "note", "id", "kind", "w", "h", "offset", "frames", "damage" };
constexpr ARRAY<const char*, 8> kHitboxKeys    = { "note", "w", "h", "cx", "cy", "lifeSec", "damage", "follow" };

namespace
{
[[nodiscard]] bool Contains_(
    const Span<const char* const> _list,
    const StringView              _key)
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
}   // namespace

EnemyDefParser::EnemyDefParser(
    EnemyDefLibrary& _library)
    : m_lib(_library)
{
}

void EnemyDefParser::Fail_(
    const StringView _def,
    const StringView _field,
    const StringView _msg)
{
    if (error.empty())
    {
        error = _field.empty() ? std::format("{}: {}", _def, _msg) : std::format("{}: {}: {}", _def, _field, _msg);
    }
}

bool EnemyDefParser::ReadNum_(
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    const StringView  _key,
    float&            _out,
    const bool        _bRequired)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        if (_bRequired)
        {
            Fail_(_def, std::format("{}.{}", _field, _key), "필수 값이 없습니다");
        }
        return !_bRequired;
    }
    const Result<double> value = _reader.ReadField<double>(_key);
    if (!value)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "숫자가 아닙니다");
        return false;
    }
    _out = static_cast<float>(value.GetValue());
    return true;
}

bool EnemyDefParser::ReadInt_(
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    const StringView  _key,
    int&              _out,
    const bool        _bRequired)
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
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    const StringView  _key,
    bool&             _out)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        return true;
    }
    const Result<bool> value = _reader.ReadField<bool>(_key);
    if (!value)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "true/false 가 아닙니다");
        return false;
    }
    _out = value.GetValue();
    return true;
}

bool EnemyDefParser::ReadStr_(
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    const StringView  _key,
    String&           _out,
    const bool        _bRequired)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        if (_bRequired)
        {
            Fail_(_def, std::format("{}.{}", _field, _key), "필수 값이 없습니다");
        }
        return !_bRequired;
    }
    const Result<String> value = _reader.ReadField<String>(_key);
    if (!value)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "문자열이 아닙니다");
        return false;
    }
    _out = value.GetValue();
    return true;
}

bool EnemyDefParser::ReadVec2_(
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    const StringView  _key,
    Vec2&             _out)
{
    if (!_reader.IsObject() || !_reader.HasField(_key))
    {
        return true;
    }
    const Result<JsonReader> array = _reader.FindField(_key);
    if (!array || !array->IsArray() || array->GetSize() != 2)
    {
        Fail_(_def, std::format("{}.{}", _field, _key), "[x, y] 배열이 아닙니다");
        return false;
    }
    ARRAY<float, 2> values = {};
    int             i      = 0;
    for (const JsonReader item: array->GetArray())
    {
        const Result<double> value = item.Read<double>();
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
    const StringView              _def,
    const StringView              _field,
    const JsonReader&             _object,
    const Span<const char* const> _allowed)
{
    if (!_object.IsObject())
    {
        Fail_(_def, _field, "객체가 필요합니다");
        return false;
    }
    for (const Member& member: _object.GetObject())
    {
        if (!Contains_(_allowed, member.first))
        {
            Fail_(_def, _field, std::format("알 수 없는 키 '{}'", member.first));
            return false;
        }
    }
    return true;
}

bool EnemyDefParser::ReadBox_(
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    BoxDef&           _out)
{
    if (!CheckObjectKeys_(_def, _field, _reader, kBoxKeys))
    {
        return false;
    }
    if (!ReadNum_(_def, _field, _reader, "w", _out.w, true)
        || !ReadNum_(_def, _field, _reader, "h", _out.h, true)
        || !ReadNum_(_def, _field, _reader, "cx", _out.cx, false)
        || !ReadNum_(_def, _field, _reader, "cy", _out.cy, false))
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
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    ClipDef&          _out)
{
    if (!CheckObjectKeys_(_def, _field, _reader, kClipKeys))
    {
        return false;
    }
    if (!ReadStr_(_def, _field, _reader, "name", _out.name, true)
        || !ReadInt_(_def, _field, _reader, "frames", _out.frames, true)
        || !ReadNum_(_def, _field, _reader, "sec", _out.frameSec, true)
        || !ReadBool_(_def, _field, _reader, "loop", _out.bLoop))
    {
        return false;
    }
    if (_out.name.empty() || _out.frames < 1 || !(_out.frameSec > 0.f))
    {
        Fail_(_def, _field, "name 이 비었거나 frames < 1 또는 sec <= 0 입니다");
        return false;
    }
    _out.frameUnits = static_cast<int64_t>(std::llround(static_cast<double>(_out.frameSec) * static_cast<double>(kAnimTimeUnitsPerSec)));
    _out.bSided     = _out.name.find("{side}") != String::npos;
    return true;
}

bool EnemyDefParser::ReadFx_(
    const StringView  _def,
    const StringView  _field,
    const JsonReader& _reader,
    FxDef&            _out)
{
    if (!CheckObjectKeys_(_def, _field, _reader, kFxKeys))
    {
        return false;
    }
    return ReadStr_(_def, _field, _reader, "clip", _out.clip, true)
        && ReadVec2_(_def, _field, _reader, "offset", _out.offset)
        && ReadNum_(_def, _field, _reader, "delay", _out.delaySec, false)
        && ReadInt_(_def, _field, _reader, "depth", _out.depth, false)
        && ReadBool_(_def, _field, _reader, "debris", _out.bDebris)
        && ReadBool_(_def, _field, _reader, "mirror", _out.bMirror);
}

bool EnemyDefParser::BuildChain_(
    const StringView _name,
    const int        _depth,
    Chain&           _out)
{
    if (_depth > 8)
    {
        Fail_(_name, "extends", "상속이 너무 깊거나 순환합니다");
        return false;
    }
    const HashMap<String, int>::const_iterator it = m_rawIndex.find(String { _name });
    if (it == m_rawIndex.end())
    {
        Fail_(_name, "extends", "존재하지 않는 정의입니다");
        return false;
    }
    const RawDef& raw = m_raw[static_cast<size_t>(it->second)];
    if (raw.reader.HasField("extends"))
    {
        const Result<String> parent = raw.reader.ReadField<String>("extends");
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
    const Chain&                  _chain,
    const StringView              _defName,
    const Span<const char* const> _allowed)
{
    for (const JsonReader& reader: _chain)
    {
        if (!CheckObjectKeys_(_defName, "", reader, _allowed))
        {
            return false;
        }
    }
    return true;
}

std::optional<JsonReader> EnemyDefParser::Find_(
    const Chain&     _chain,
    const StringView _key) const
{
    for (size_t i = _chain.size(); i > 0; --i)
    {
        if (_chain[i - 1].HasField(_key))
        {
            const Result<JsonReader> field = _chain[i - 1].FindField(_key);
            if (field)
            {
                return field.GetValue();
            }
        }
    }
    return std::nullopt;
}

EnemyDefParser::Entries EnemyDefParser::Merge_(
    const Chain&     _chain,
    const StringView _key) const
{
    Entries entries;
    for (const JsonReader& reader: _chain)
    {
        if (!reader.HasField(_key))
        {
            continue;
        }
        const Result<JsonReader> field = reader.FindField(_key);
        if (!field || !field->IsObject())
        {
            continue;
        }
        for (const Member& member: field->GetObject())
        {
            const Entries::iterator found = std::find_if(entries.begin(), entries.end(), [&](const Entry& _e) { return _e.first == member.first; });
            if (found != entries.end())
            {
                found->second = member.second;
            }
            else
            {
                entries.emplace_back(String { member.first }, member.second);
            }
        }
    }
    return entries;
}

bool EnemyDefParser::ParseProjectile_(
    const RawDef& _raw,
    const Chain&  _chain)
{
    const String& name = _raw.name;
    if (!CheckKeys_(_chain, name, kProjectileKeys))
    {
        return false;
    }

    ProjectileDef projectile;
    projectile.name = name;

    const std::optional<JsonReader> box = Find_(_chain, "box");
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
    for (const JsonReader& reader: _chain)
    {
        if (!ReadInt_(name, "", reader, "damage", projectile.damage, false)
            || !ReadNum_(name, "", reader, "speed", projectile.speed, false)
            || !ReadNum_(name, "", reader, "gravity", projectile.gravity, false)
            || !ReadNum_(name, "", reader, "maxFall", projectile.maxFall, false)
            || !ReadNum_(name, "", reader, "lifetime", lifetime, false)
            || !ReadNum_(name, "", reader, "startDelay", delay, false)
            || !ReadVec2_(name, "", reader, "v0", projectile.v0))
        {
            return false;
        }
    }
    projectile.lifetimeSec   = lifetime;
    projectile.startDelaySec = delay;

    if (const std::optional<JsonReader> ramp = Find_(_chain, "speedRamp"))
    {
        if (!CheckObjectKeys_(name, "speedRamp", *ramp, kSpeedRampKeys)
            || !ReadNum_(name, "speedRamp", *ramp, "from", projectile.rampFrom, false)
            || !ReadNum_(name, "speedRamp", *ramp, "accel", projectile.rampAccel, true)
            || !ReadNum_(name, "speedRamp", *ramp, "max", projectile.rampMax, true))
        {
            return false;
        }
        projectile.bRamp = true;
    }

    if (const std::optional<JsonReader> onSolid = Find_(_chain, "onSolid"))
    {
        const Result<String> mode = onSolid->Read<String>();
        if (!mode || (mode.GetValue() != "explode" && mode.GetValue() != "none"))
        {
            Fail_(name, "onSolid", "\"explode\" 또는 \"none\" 이어야 합니다");
            return false;
        }
        projectile.bExplodeOnSolid = mode.GetValue() == "explode";
    }

    const std::optional<JsonReader> anim = Find_(_chain, "anim");
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
        const Result<JsonReader> fly = anim->FindField("fly");
        if (!fly || !ReadClip_(name, "anim.fly", fly.GetValue(), projectile.fly))
        {
            return false;
        }
        if (anim->HasField("begin"))
        {
            const Result<JsonReader> begin = anim->FindField("begin");
            if (!begin || !ReadClip_(name, "anim.begin", begin.GetValue(), projectile.begin))
            {
                return false;
            }
        }
        if (anim->HasField("land"))
        {
            const Result<JsonReader> land = anim->FindField("land");
            if (!land || !ReadClip_(name, "anim.land", land.GetValue(), projectile.land))
            {
                return false;
            }
        }
    }

    if (const std::optional<JsonReader> explosion = Find_(_chain, "explosion"))
    {
        if (!CheckObjectKeys_(name, "explosion", *explosion, kExplosionKeys)
            || !ReadStr_(name, "explosion", *explosion, "fx", projectile.explosion.clip, true)
            || !ReadVec2_(name, "explosion", *explosion, "offset", projectile.explosion.offset))
        {
            return false;
        }
        if (explosion->HasField("sfx") && !ReadStr_(name, "explosion", *explosion, "sfx", projectile.explosionSfx, true))
        {
            return false;
        }
    }

    if (const std::optional<JsonReader> trail = Find_(_chain, "trail"))
    {
        float trailSec = 0.f;
        if (!CheckObjectKeys_(name, "trail", *trail, kExplosionKeys)
            || !ReadStr_(name, "trail", *trail, "fx", projectile.trail.clip, true)
            || !ReadVec2_(name, "trail", *trail, "offset", projectile.trail.offset))
        {
            return false;
        }
        for (const JsonReader& reader: _chain)
        {
            if (!ReadNum_(name, "", reader, "trailSec", trailSec, false))
            {
                return false;
            }
        }
        projectile.trailTicks = Max(1, SecToTicks(trailSec));
    }

    if (const std::optional<JsonReader> blast = Find_(_chain, "blast"))
    {
        BoxDef blastBox;
        if (!CheckObjectKeys_(name, "blast", *blast, kBlastKeys)
            || !ReadNum_(name, "blast", *blast, "w", blastBox.w, true)
            || !ReadNum_(name, "blast", *blast, "h", blastBox.h, true)
            || !ReadNum_(name, "blast", *blast, "up", projectile.blastUp, false)
            || !ReadNum_(name, "blast", *blast, "sec", projectile.blastSec, true)
            || !ReadInt_(name, "blast", *blast, "damage", projectile.blastDamage, false))
        {
            return false;
        }
        projectile.blastSize = { blastBox.w, blastBox.h };
    }

    for (const JsonReader& reader: _chain)
    {
        if (!ReadNum_(name, "", reader, "rollSpeed", projectile.rollSpeed, false)
            || !ReadNum_(name, "", reader, "rollSec", projectile.rollSec, false)
            || !ReadBool_(name, "", reader, "expireFx", projectile.bExpireFx)
            || !ReadBool_(name, "", reader, "rotate", projectile.bRotate)
            || !ReadNum_(name, "", reader, "scale", projectile.scale, false)
            || !ReadNum_(name, "", reader, "groundProbe", projectile.groundProbeY, false))
        {
            return false;
        }
    }

    const bool bMoves = projectile.speed > 0.f
                     || projectile.bRamp
                     || projectile.v0.x != 0.f
                     || projectile.v0.y != 0.f
                     || projectile.gravity != 0.f;
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
    const String&     _def,
    const String&     _name,
    const JsonReader& _reader,
    WeaponDef&        _out)
{
    const String field = std::format("weapons.{}", _name);
    if (!CheckObjectKeys_(_def, field, _reader, kWeaponKeys))
    {
        return false;
    }
    _out.name = _name;

    String projectileName;
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

    String aim;
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
        const Result<JsonReader> tilt = _reader.FindField("tilt");
        if (!tilt
            || !CheckObjectKeys_(_def, field + ".tilt", tilt.GetValue(), kTiltKeys)
            || !ReadNum_(_def, field + ".tilt", tilt.GetValue(), "threshold", _out.tiltThreshold, true)
            || !ReadNum_(_def, field + ".tilt", tilt.GetValue(), "dy", _out.tiltDy, true))
        {
            return false;
        }
    }
    if (_reader.HasField("arc"))
    {
        const Result<JsonReader> arc = _reader.FindField("arc");
        if (!arc
            || !CheckObjectKeys_(_def, field + ".arc", arc.GetValue(), kArcKeys)
            || !ReadNum_(_def, field + ".arc", arc.GetValue(), "minVx", _out.arcMinVx, true)
            || !ReadNum_(_def, field + ".arc", arc.GetValue(), "maxVx", _out.arcMaxVx, true))
        {
            return false;
        }
        _out.bArc = true;
    }
    if (_reader.HasField("muzzleFx"))
    {
        const Result<JsonReader> fx = _reader.FindField("muzzleFx");
        if (!fx || !ReadFx_(_def, field + ".muzzleFx", fx.GetValue(), _out.muzzleFx))
        {
            return false;
        }
    }
    if (!ReadStr_(_def, field, _reader, "motionClip", _out.motionClip, false))
    {
        return false;
    }
    if (_reader.HasField("effects"))
    {
        const Result<JsonReader> effects = _reader.FindField("effects");
        if (!effects || !effects->IsArray())
        {
            Fail_(_def, field + ".effects", "배열이 아닙니다");
            return false;
        }
        int index = 0;
        for (const JsonReader item: effects->GetArray())
        {
            WeaponEffect effect;
            const String path = std::format("{}.effects[{}]", field, index++);
            if (!CheckObjectKeys_(_def, path, item, kWeaponEffectKeys)
                || !ReadStr_(_def, path, item, "clip", effect.clip, true)
                || !ReadVec2_(_def, path, item, "offset", effect.offset)
                || !ReadInt_(_def, path, item, "frame", effect.frame, false)
                || !ReadInt_(_def, path, item, "depth", effect.depth, false))
            {
                return false;
            }
            _out.effects.push_back(std::move(effect));
        }
        if (!_out.effects.empty() && _out.motionClip.empty())
        {
            Fail_(_def, field + ".effects", "motionClip 이 필요합니다");
            return false;
        }
    }
    if (_reader.HasField("hitbox"))
    {
        const Result<JsonReader> hitbox = _reader.FindField("hitbox");
        if (!hitbox || !CheckObjectKeys_(_def, field + ".hitbox", hitbox.GetValue(), kHitboxKeys))
        {
            return false;
        }
        BoxDef box;
        if (!ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "w", box.w, true)
            || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "h", box.h, true)
            || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "cx", box.cx, false)
            || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "cy", box.cy, false)
            || !ReadNum_(_def, field + ".hitbox", hitbox.GetValue(), "lifeSec", _out.hitboxLifeSec, true)
            || !ReadInt_(_def, field + ".hitbox", hitbox.GetValue(), "damage", _out.hitboxDamage, false)
            || !ReadBool_(_def, field + ".hitbox", hitbox.GetValue(), "follow", _out.bHitboxFollows))
        {
            return false;
        }
        _out.hitbox  = box;
        _out.bHitbox = true;
    }
    if (_out.projectile < 0 && !_out.bHitbox)
    {
        Fail_(_def, field, "projectile 또는 hitbox 가 필요합니다");
        return false;
    }
    return true;
}

bool EnemyDefParser::ReadDeathClip_(
    const EnemyDef&   _def,
    const JsonReader& _death,
    const StringView  _key,
    int&              _out)
{
    if (!_death.HasField(_key))
    {
        return true;
    }
    String logical;
    if (!ReadStr_(_def.name, "death", _death, _key, logical, true))
    {
        return false;
    }
    _out = _def.FindClip(logical);
    if (_out < 0)
    {
        Fail_(_def.name, std::format("death.{}", _key), std::format("'{}' 클립이 clips 에 없습니다", logical));
        return false;
    }
    return true;
}

bool EnemyDefParser::ParseDeath_(
    EnemyDef&         _def,
    const JsonReader& _death)
{
    const String& name = _def.name;
    if (!CheckObjectKeys_(name, "death", _death, kDeathKeys))
    {
        return false;
    }

    DeathDef& death = _def.death;

    if (!ReadDeathClip_(_def, _death, "bomb", death.clipBomb)
        || !ReadDeathClip_(_def, _death, "slash", death.clipSlash)
        || !ReadDeathClip_(_def, _death, "stand1", death.clipStand1)
        || !ReadDeathClip_(_def, _death, "stand2", death.clipStand2)
        || !ReadDeathClip_(_def, _death, "fall", death.clipFall)
        || !ReadDeathClip_(_def, _death, "land", death.clipLand)
        || !ReadDeathClip_(_def, _death, "jump", death.clipJump))
    {
        return false;
    }
    if (!ReadInt_(name, "death", _death, "fallAt1", death.fallAt1, false)
        || !ReadInt_(name, "death", _death, "fallAt2", death.fallAt2, false)
        || !ReadNum_(name, "death", _death, "blinkAtSec", death.blinkAtSec, false)
        || !ReadNum_(name, "death", _death, "blinkForSec", death.blinkForSec, false)
        || !ReadNum_(name, "death", _death, "blinkPeriodSec", death.blinkPeriodSec, false)
        || !ReadNum_(name, "death", _death, "removeAtSec", death.removeAtSec, false)
        || !ReadNum_(name, "death", _death, "bloodOffsetY", death.bloodOffsetY, false)
        || !ReadVec2_(name, "death", _death, "bombVelocity", death.bombVelocity))
    {
        return false;
    }

    if (_death.HasField("fx"))
    {
        const Result<JsonReader> fx = _death.FindField("fx");
        if (!fx || !fx->IsArray())
        {
            Fail_(name, "death.fx", "배열이 아닙니다");
            return false;
        }

        int index = 0;
        for (const JsonReader item: fx->GetArray())
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
    const String& name = _raw.name;
    if (!CheckKeys_(_chain, name, kEnemyKeys))
    {
        return false;
    }

    EnemyDef def;
    def.name = name;

    String kind;
    String behaviorName;
    String teamName;
    for (const JsonReader& reader: _chain)
    {
        if (!ReadStr_(name, "", reader, "kind", kind, false)
            || !ReadStr_(name, "", reader, "behavior", behaviorName, false)
            || !ReadStr_(name, "", reader, "team", teamName, false))
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
    for (const JsonReader& reader: _chain)
    {
        if (!ReadInt_(name, "", reader, "hp", def.hp, false)
            || !ReadInt_(name, "", reader, "score", def.score, false)
            || !ReadNum_(name, "", reader, "ttk", ttk, false))
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

    const std::optional<JsonReader> body = Find_(_chain, "body");
    if (!body || !ReadBox_(name, "body", *body, def.body))
    {
        if (!body)
        {
            Fail_(name, "body", "필수 값이 없습니다");
        }
        return false;
    }
    def.hurtbox = def.body;
    if (const std::optional<JsonReader> hurtbox = Find_(_chain, "hurtbox"))
    {
        def.hurtbox = BoxDef {};
        if (!ReadBox_(name, "hurtbox", *hurtbox, def.hurtbox))
        {
            return false;
        }
    }
    if (const std::optional<JsonReader> boxes = Find_(_chain, "hitboxes"))
    {
        if (!boxes->IsArray())
        {
            Fail_(name, "hitboxes", "배열이 아닙니다");
            return false;
        }
        int boxIndex = 0;
        for (const JsonReader item: boxes->GetArray())
        {
            ExtraBoxDef  box;
            String       boxKind;
            Vec2         offset = { 0.f, 0.f };
            Vec2         frames = { 0.f, -1.f };
            const String field  = std::format("hitboxes[{}]", boxIndex++);
            if (!CheckObjectKeys_(name, field, item, kExtraBoxKeys)
                || !ReadStr_(name, field, item, "id", box.id, false)
                || !ReadStr_(name, field, item, "kind", boxKind, true)
                || !ReadNum_(name, field, item, "w", box.box.w, true)
                || !ReadNum_(name, field, item, "h", box.box.h, true)
                || !ReadVec2_(name, field, item, "offset", offset)
                || !ReadVec2_(name, field, item, "frames", frames)
                || !ReadInt_(name, field, item, "damage", box.damage, false))
            {
                return false;
            }
            if (boxKind != "hurt" && boxKind != "attack")
            {
                Fail_(name, field + ".kind", "\"hurt\" 또는 \"attack\" 이어야 합니다");
                return false;
            }
            if (!(box.box.w > 0.f) || !(box.box.h > 0.f))
            {
                Fail_(name, field, "w, h 는 0 보다 커야 합니다");
                return false;
            }
            box.kind      = boxKind == "attack" ? eExtraBoxKind::Attack : eExtraBoxKind::Hurt;
            box.box.cx    = offset.x;
            box.box.cy    = offset.y;
            box.frameFrom = static_cast<int>(frames.x);
            box.frameTo   = static_cast<int>(frames.y);
            def.hitboxes.push_back(std::move(box));
        }
    }
    if (const std::optional<JsonReader> solid = Find_(_chain, "solid"))
    {
        if (!ReadBox_(name, "solid", *solid, def.solid))
        {
            return false;
        }
    }

    if (const std::optional<JsonReader> activation = Find_(_chain, "activation"))
    {
        String mode;
        if (!CheckObjectKeys_(name, "activation", *activation, kActivationKeys)
            || !ReadStr_(name, "activation", *activation, "mode", mode, false)
            || !ReadNum_(name, "activation", *activation, "marginPx", def.activation.marginPx, false)
            || !ReadBool_(name, "activation", *activation, "latch", def.activation.bLatch))
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

    if (const std::optional<JsonReader> physics = Find_(_chain, "physics"))
    {
        PhysicsDef& p = def.physics;
        if (!CheckObjectKeys_(name, "physics", *physics, kPhysicsKeys)
            || !ReadNum_(name, "physics", *physics, "gravity", p.gravity, false)
            || !ReadNum_(name, "physics", *physics, "maxFall", p.maxFall, false)
            || !ReadNum_(name, "physics", *physics, "speed", p.speed, false)
            || !ReadNum_(name, "physics", *physics, "jumpVy", p.jumpVy, false)
            || !ReadNum_(name, "physics", *physics, "dropSpeed", p.dropSpeed, false)
            || !ReadNum_(name, "physics", *physics, "footHalfWidth", p.footHalfWidth, false)
            || !ReadBool_(name, "physics", *physics, "useGravity", p.bGravity))
        {
            return false;
        }
    }

    for (const Entry& entry: Merge_(_chain, "clips"))
    {
        NamedClip clip;
        clip.name = entry.first;
        if (!ReadClip_(name, std::format("clips.{}", entry.first), entry.second, clip.clip))
        {
            return false;
        }
        def.clips.push_back(std::move(clip));
    }
    for (const Entry& entry: Merge_(_chain, "weapons"))
    {
        WeaponDef weapon;
        if (!ParseWeapon_(name, entry.first, entry.second, weapon))
        {
            return false;
        }
        def.weapons.push_back(std::move(weapon));
    }

    if (const std::optional<JsonReader> death = Find_(_chain, "death"))
    {
        if (!ParseDeath_(def, *death))
        {
            return false;
        }
    }

    if (const std::optional<JsonReader> drops = Find_(_chain, "drops"))
    {
        if (!drops->IsArray())
        {
            Fail_(name, "drops", "배열이 아닙니다");
            return false;
        }

        int index = 0;
        for (const JsonReader item: drops->GetArray())
        {
            DropDef      drop;
            const String field = std::format("drops[{}]", index++);
            if (!CheckObjectKeys_(name, field, item, kDropKeys)
                || !ReadStr_(name, field, item, "item", drop.item, true)
                || !ReadInt_(name, field, item, "count", drop.count, false))
            {
                return false;
            }
            def.drops.push_back(std::move(drop));
        }
    }

    if (const std::optional<JsonReader> overlay = Find_(_chain, "overlay"))
    {
        String clip;
        if (!CheckObjectKeys_(name, "overlay", *overlay, kOverlayKeys)
            || !ReadStr_(name, "overlay", *overlay, "clip", clip, true)
            || !ReadVec2_(name, "overlay", *overlay, "offset", def.overlayOffset))
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

    def.bHitFlash     = def.hp >= kHitFlashMinHp;
    bool   bNoOverlay = false;
    String artFacing;
    for (const JsonReader& reader: _chain)
    {
        if (!ReadBool_(name, "", reader, "noOverlay", bNoOverlay)
            || !ReadBool_(name, "", reader, "noDespawn", def.bNoDespawn)
            || !ReadBool_(name, "", reader, "human", def.bHuman)
            || !ReadBool_(name, "", reader, "hitFlash", def.bHitFlash)
            || !ReadBool_(name, "", reader, "centerAnchor", def.bCenterAnchor)
            || !ReadStr_(name, "", reader, "artFacing", artFacing, false))
        {
            return false;
        }
    }
    if (artFacing == "left")
    {
        def.artFacing = eArtFacing::Left;
    }
    else if (artFacing == "right")
    {
        def.artFacing = eArtFacing::Right;
    }
    else if (!artFacing.empty())
    {
        Fail_(name, "artFacing", "\"left\" 또는 \"right\" 이어야 합니다");
        return false;
    }

    if (bNoOverlay)
    {
        def.overlayClip = -1;
    }

    Vector<ParamEntry> params;
    for (const Entry& entry: Merge_(_chain, "params"))
    {
        params.emplace_back(entry.first, entry.second);
    }
    {
        String    message;
        DefReader reader { name, def, params, message };
        switch (def.behavior)
        {
            case eEnemyBehavior::Soldier: ParseSoldierCfg(reader, def); break;
            case eEnemyBehavior::Bzk: ParseBzkCfg(reader, def); break;
            case eEnemyBehavior::Hang: ParseHangCfg(reader, def); break;
            case eEnemyBehavior::FlyingBzk: ParseFlyingBzkCfg(reader, def); break;
            case eEnemyBehavior::Shielder: ParseShielderCfg(reader, def); break;
            case eEnemyBehavior::Pow: ParsePowCfg(reader, def); break;
            case eEnemyBehavior::Jeep: ParseJeepCfg(reader, def); break;
            case eEnemyBehavior::BTank: ParseBTankCfg(reader, def); break;
            case eEnemyBehavior::MissleTank: ParseMissleTankCfg(reader, def); break;
            case eEnemyBehavior::Sarubia: ParseSarubiaCfg(reader, def); break;
            case eEnemyBehavior::Bus: ParseBusCfg(reader, def); break;
            case eEnemyBehavior::Heli: ParseHeliCfg(reader, def); break;
            case eEnemyBehavior::HeliBomb: ParseHeliBombCfg(reader, def); break;
            case eEnemyBehavior::BigShiee: ParseBigShieeCfg(reader, def); break;
            case eEnemyBehavior::Count: break;
        }
        ParseEnemyParts(reader, def);
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
    const JsonReader& _root)
{
    return RunMany(Span<const JsonReader> { &_root, 1 });
}

bool EnemyDefParser::RunMany(
    const Span<const JsonReader> _roots)
{
    for (const JsonReader& root: _roots)
    {
        if (!AddRoot_(root))
        {
            return false;
        }
    }
    return Finish_();
}

bool EnemyDefParser::AddRoot_(
    const JsonReader& _root)
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
        const Result<JsonReader> globals = _root.FindField("globals");
        EnemyGlobals&            g       = m_lib.m_globals;
        if (!globals
            || !CheckObjectKeys_(kRoot, "globals", globals.GetValue(), kGlobalKeys)
            || !ReadNum_(kRoot, "globals", globals.GetValue(), "dpsRef", g.dpsRef, false)
            || !ReadNum_(kRoot, "globals", globals.GetValue(), "activationMarginPx", g.activationMarginPx, false)
            || !ReadNum_(kRoot, "globals", globals.GetValue(), "despawnBehindCameraPx", g.despawnBehindCameraPx, false)
            || !ReadNum_(kRoot, "globals", globals.GetValue(), "despawnBelowCameraPx", g.despawnBelowCameraPx, false))
        {
            return false;
        }
    }

    const Result<JsonReader> defs = _root.FindField("defs");
    if (!defs || !defs->IsObject())
    {
        Fail_(kRoot, "defs", "객체가 필요합니다");
        return false;
    }
    for (const Member& member: defs->GetObject())
    {
        if (m_rawIndex.contains(String { member.first }))
        {
            Fail_(member.first, "", "정의 이름이 중복되었습니다");
            return false;
        }
        m_rawIndex[String { member.first }] = static_cast<int>(m_raw.size());
        m_raw.push_back({ String { member.first }, member.second });
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

            String kind;
            for (const JsonReader& reader: chain)
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
        if (def.behavior == eEnemyBehavior::Bus)
        {
            for (const String* pName: { &def.bus.soldierBombDef, &def.bus.soldierSwordDef })
            {
                if (m_lib.FindIndex(*pName) < 0)
                {
                    Fail_(def.name, "params.soldierBombDef/soldierSwordDef", std::format("'{}' 정의가 없습니다", *pName));
                    return false;
                }
            }
        }
    }
    return true;
}

Result<EnemyDefLibrary> EnemyDefLibrary::Parse(
    const StringView _json,
    String*          _pMessage)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json, eJsonLoadOption::AllowComments);
    if (!doc)
    {
        if (_pMessage)
        {
            *_pMessage = "Enemies.json: JSON 구문 오류";
        }
        SLUG_LOG_ERROR("Enemies.json: JSON 구문 오류");
        return Error { eError::Parse };
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
        return Error { eError::EnemyDef };
    }
    return library;
}

Result<EnemyDefLibrary> EnemyDefLibrary::Load(
    const FilePath& _path,
    String*         _pMessage)
{
    Result<JsonDeserializer> doc = JsonDeserializer::LoadFromFile(_path, eJsonLoadOption::AllowComments);
    if (!doc)
    {
        if (_pMessage)
        {
            *_pMessage = std::format("{}: 파일을 열 수 없거나 JSON 구문 오류", _path.string());
        }
        SLUG_LOG_ERROR("적 정의 파일 로드 실패: {}", _path.string());
        return Error { eError::FileOpen };
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
        return Error { eError::EnemyDef };
    }
    return library;
}

Result<EnemyDefLibrary> EnemyDefLibrary::LoadDirectory(
    const FilePath& _dir,
    String*         _pMessage)
{
    Vector<FilePath> files;
    std::error_code  ec;
    for (const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator(_dir, ec))
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
        return Error { eError::FileOpen };
    }
    std::sort(files.begin(), files.end());

    Vector<JsonDeserializer> docs;
    docs.reserve(files.size());
    Vector<JsonReader> roots;
    roots.reserve(files.size());
    for (const FilePath& file: files)
    {
        Result<JsonDeserializer> doc = JsonDeserializer::LoadFromFile(file, eJsonLoadOption::AllowComments);
        if (!doc)
        {
            if (_pMessage)
            {
                *_pMessage = std::format("{}: 파일을 열 수 없거나 JSON 구문 오류", file.string());
            }
            SLUG_LOG_ERROR("적 정의 파일 로드 실패: {}", file.string());
            return Error { eError::FileOpen };
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
        return Error { eError::EnemyDef };
    }
    return library;
}

FilePath EnemyDefLibrary::GetDefaultPath(
    const FilePath& _assetRoot)
{
    return _assetRoot / "Defs" / "Enemies.json";
}

FilePath EnemyDefLibrary::GetDefaultDirectory(
    const FilePath& _assetRoot)
{
    return _assetRoot / "Defs";
}

const EnemyDef* EnemyDefLibrary::Find(
    const StringView _name) const
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_defs[static_cast<size_t>(index)];
}

EnemyDef* EnemyDefLibrary::FindMutable(
    const StringView _name)
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_defs[static_cast<size_t>(index)];
}

int EnemyDefLibrary::FindIndex(
    const StringView _name) const
{
    const HashMap<String, int>::const_iterator it = m_defIndex.find(String { _name });
    return it == m_defIndex.end() ? -1 : it->second;
}

const ProjectileDef* EnemyDefLibrary::FindProjectile(
    const StringView _name) const
{
    const int index = FindProjectileIndex(_name);
    return index < 0 ? nullptr : &m_projectiles[static_cast<size_t>(index)];
}

int EnemyDefLibrary::FindProjectileIndex(
    const StringView _name) const
{
    const HashMap<String, int>::const_iterator it = m_projIndex.find(String { _name });
    return it == m_projIndex.end() ? -1 : it->second;
}

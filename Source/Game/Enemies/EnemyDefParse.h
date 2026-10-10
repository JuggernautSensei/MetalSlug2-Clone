#pragma once
#include "Base/Json.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Enemies/EnemyDef.h"

namespace slug
{
struct ParamEntry
{
    slug::String      key   = {};
    slug::JsonReader  value;
    bool             bUsed = false;

    ParamEntry(slug::String _key, const slug::JsonReader _value)
        : key(std::move(_key))
        , value(_value)
    {
    }
};

class DefReader
{
public:
    DefReader(slug::StringView _defName, EnemyDef& _def, slug::Vector<ParamEntry>& _params, slug::String& _error);

    [[nodiscard]] float        Num(slug::StringView _key);
    [[nodiscard]] float        Num(slug::StringView _key, float _default);
    [[nodiscard]] int          Int(slug::StringView _key);
    [[nodiscard]] int          Int(slug::StringView _key, int _default);
    [[nodiscard]] int          Ticks(slug::StringView _key);
    [[nodiscard]] int          Ticks(slug::StringView _key, float _defaultSec);
    [[nodiscard]] bool         Bool(slug::StringView _key, bool _default);
    [[nodiscard]] slug::String  Str(slug::StringView _key);
    [[nodiscard]] Vec2 ReadVec2(slug::StringView _key, Vec2 _default);
    [[nodiscard]] BoxDef       Box(slug::StringView _key);
    [[nodiscard]] bool         NumArray(slug::StringView _key, float* _pOut, int _count);
    [[nodiscard]] bool         Has(slug::StringView _key) const;

    [[nodiscard]] int Clip(slug::StringView _logical);
    [[nodiscard]] int ClipOpt(slug::StringView _logical) const;
    [[nodiscard]] int Weapon(slug::StringView _name);
    [[nodiscard]] int WeaponOpt(slug::StringView _name) const;

    void Fail(slug::StringView _field, slug::StringView _message);
    [[nodiscard]] bool HasError() const { return !m_error.empty(); }

    [[nodiscard]] bool Finish();

private:
    [[nodiscard]] ParamEntry* Find_(slug::StringView _key);

    slug::String                m_defName;
    EnemyDef&                  m_def;
    slug::Vector<ParamEntry>&   m_params;
    slug::String&               m_error;
};

void ParseSoldierCfg(DefReader& _reader, EnemyDef& _def);
void ParseBzkCfg(DefReader& _reader, EnemyDef& _def);
void ParseHangCfg(DefReader& _reader, EnemyDef& _def);
void ParseFlyingBzkCfg(DefReader& _reader, EnemyDef& _def);
void ParseShielderCfg(DefReader& _reader, EnemyDef& _def);
void ParsePowCfg(DefReader& _reader, EnemyDef& _def);
void ParseBusCfg(DefReader& _reader, EnemyDef& _def);
void ParseBossShieeCfg(DefReader& _reader, EnemyDef& _def);
void ParseJeepCfg(DefReader& _reader, EnemyDef& _def);
void ParseBTankCfg(DefReader& _reader, EnemyDef& _def);
void ParseMissleTankCfg(DefReader& _reader, EnemyDef& _def);
void ParseSarubiaCfg(DefReader& _reader, EnemyDef& _def);
void ParseAparchCfg(DefReader& _reader, EnemyDef& _def);
}

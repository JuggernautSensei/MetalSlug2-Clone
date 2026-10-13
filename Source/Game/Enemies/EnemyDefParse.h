#pragma once
#include "Base/JsonFields.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyDef.h"

struct ParamEntry
{
    String     key = {};
    JsonReader value;
    bool       bUsed = false;

    ParamEntry(String _key, JsonReader _value);
};

class DefReader
{
public:
    DefReader(StringView _defName, EnemyDef& _def, Vector<ParamEntry>& _params, String& _error);
    [[nodiscard]] float             Num(StringView _key);
    [[nodiscard]] float             Num(StringView _key, float _default);
    [[nodiscard]] int               Int(StringView _key);
    [[nodiscard]] int               Int(StringView _key, int _default);
    [[nodiscard]] int               Ticks(StringView _key);
    [[nodiscard]] int               Ticks(StringView _key, float _defaultSec);
    [[nodiscard]] bool              Bool(StringView _key, bool _bDefault);
    [[nodiscard]] String            Str(StringView _key);
    [[nodiscard]] VECTOR2           ReadVec2(StringView _key, VECTOR2 _default);
    [[nodiscard]] BoxDef            Box(StringView _key);
    [[nodiscard]] bool              NumArray(StringView _key, Span<float> _out);
    [[nodiscard]] int               NumList(StringView _key, Span<float> _out);
    [[nodiscard]] bool              Has(StringView _key) const;
    [[nodiscard]] const JsonReader* FindRaw(StringView _key);
    [[nodiscard]] int               Clip(StringView _logical);
    [[nodiscard]] int               ClipOpt(StringView _logical) const;
    [[nodiscard]] int               Weapon(StringView _name);
    void                            Fail(StringView _field, StringView _message);
    [[nodiscard]] bool              Finish();

private:
    [[nodiscard]] ParamEntry* Find_(StringView _key);
    void                      ReadBoxField_(const ParamEntry& _entry, StringView _key, StringView _name, float& _out, bool _bRequired);

    String              m_defName;
    EnemyDef&           m_def;
    Vector<ParamEntry>& m_params;
    String&             m_error;
};

void ParseSoldierConfig(DefReader& _reader, EnemyDef& _def);
void ParseBzkConfig(DefReader& _reader, EnemyDef& _def);
void ParseHangConfig(DefReader& _reader, EnemyDef& _def);
void ParseFlyingBzkConfig(DefReader& _reader, EnemyDef& _def);
void ParseShielderConfig(DefReader& _reader, EnemyDef& _def);
void ParsePowConfig(DefReader& _reader, EnemyDef& _def);
void ParseJeepConfig(DefReader& _reader, EnemyDef& _def);
void ParseBTankConfig(DefReader& _reader, EnemyDef& _def);
void ParseMissileTankConfig(DefReader& _reader, EnemyDef& _def);
void ParseSarubiaConfig(DefReader& _reader, EnemyDef& _def);
void ParseBusConfig(DefReader& _reader, EnemyDef& _def);
void ParseHeliConfig(DefReader& _reader, EnemyDef& _def);
void ParseHeliBombConfig(DefReader& _reader, EnemyDef& _def);
void ParseBigShieeConfig(DefReader& _reader, EnemyDef& _def);

void ParseEnemyParts(DefReader& _reader, EnemyDef& _def);

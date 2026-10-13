#pragma once
#include "Base/Json.h"
#include "Base/Math.h"
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
    [[nodiscard]] const JsonReader* FindRaw(StringView _key);   // 객체/객체 배열 같은 구조 값을 직접 읽을 때 (사용 표시, 없으면 nullptr)

    [[nodiscard]] int Clip(StringView _logical);
    [[nodiscard]] int ClipOpt(StringView _logical) const;
    [[nodiscard]] int Weapon(StringView _name);

    void               Fail(StringView _field, StringView _message);
    [[nodiscard]] bool HasError() const;

    [[nodiscard]] bool Finish();

private:
    [[nodiscard]] ParamEntry* Find_(StringView _key);

    void ReadBoxField_(const ParamEntry& _entry, StringView _key, StringView _name, float& _out, bool _bRequired);

    String              m_defName;
    EnemyDef&           m_def;
    Vector<ParamEntry>& m_params;
    String&             m_error;
};

void ParseSoldierCfg(DefReader& _reader, EnemyDef& _def);
void ParseBzkCfg(DefReader& _reader, EnemyDef& _def);
void ParseHangCfg(DefReader& _reader, EnemyDef& _def);
void ParseFlyingBzkCfg(DefReader& _reader, EnemyDef& _def);
void ParseShielderCfg(DefReader& _reader, EnemyDef& _def);
void ParsePowCfg(DefReader& _reader, EnemyDef& _def);
void ParseJeepCfg(DefReader& _reader, EnemyDef& _def);
void ParseBTankCfg(DefReader& _reader, EnemyDef& _def);
void ParseMissleTankCfg(DefReader& _reader, EnemyDef& _def);
void ParseSarubiaCfg(DefReader& _reader, EnemyDef& _def);
void ParseBusCfg(DefReader& _reader, EnemyDef& _def);
void ParseHeliCfg(DefReader& _reader, EnemyDef& _def);
void ParseHeliBombCfg(DefReader& _reader, EnemyDef& _def);
void ParseBigShieeCfg(DefReader& _reader, EnemyDef& _def);

// params.parts: GetEnemyPartSlots 의 이름만 허용, 각 부품은 {depth, sort} 정수 (둘 다 선택)
void ParseEnemyParts(DefReader& _reader, EnemyDef& _def);

#pragma once
#include <JugX/JsonDeserializer.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Enemies/EnemyDef.h"

namespace slug
{

// ===========================================
//  DefReader
//   행동별 설정(SoldierCfg 등)을 JSON "params" / "clips" / "weapons" 에서 채울 때 쓰는 읽기 도우미.
//   필수 키가 없거나 형식이 틀리면 첫 오류만 "<def>: <필드>: 사유" 형태로 기록한다. Finish() 가 쓰이지 않은 키도 오류로 본다.
// ===========================================

struct ParamEntry
{
    jug::String      key   = {};
    jug::JsonReader  value;
    bool             bUsed = false;

    ParamEntry(jug::String _key, const jug::JsonReader _value)
        : key(std::move(_key))
        , value(_value)
    {
    }
};

class DefReader
{
public:
    DefReader(jug::StringView _defName, EnemyDef& _def, jug::Vector<ParamEntry>& _params, jug::String& _error);

    // --- 수치 (키는 params) ---
    [[nodiscard]] float        Num(jug::StringView _key);                         // 필수
    [[nodiscard]] float        Num(jug::StringView _key, float _default);
    [[nodiscard]] int          Int(jug::StringView _key);
    [[nodiscard]] int          Int(jug::StringView _key, int _default);
    [[nodiscard]] int          Ticks(jug::StringView _key);                       // 초 → 틱 (필수)
    [[nodiscard]] int          Ticks(jug::StringView _key, float _defaultSec);
    [[nodiscard]] bool         Bool(jug::StringView _key, bool _default);
    [[nodiscard]] jug::String  Str(jug::StringView _key);                         // 필수
    [[nodiscard]] jug::VECTOR2 Vec2(jug::StringView _key, jug::VECTOR2 _default);
    [[nodiscard]] BoxDef       Box(jug::StringView _key);                         // {w,h,cx,cy} 필수
    [[nodiscard]] bool         NumArray(jug::StringView _key, float* _pOut, int _count);   // 길이가 정확히 _count 인 배열. 없으면 false(기본값 유지)
    [[nodiscard]] bool         Has(jug::StringView _key) const;

    // --- 클립 / 무기 (def 안의 논리 이름 → 인덱스) ---
    [[nodiscard]] int Clip(jug::StringView _logical);       // 필수
    [[nodiscard]] int ClipOpt(jug::StringView _logical) const;   // 없으면 -1
    [[nodiscard]] int Weapon(jug::StringView _name);        // 필수
    [[nodiscard]] int WeaponOpt(jug::StringView _name) const;

    void Fail(jug::StringView _field, jug::StringView _message);
    [[nodiscard]] bool HasError() const { return !m_error.empty(); }

    // 사용되지 않은 params 키를 오류로 기록한다. 오류가 없으면 true.
    [[nodiscard]] bool Finish();

private:
    [[nodiscard]] ParamEntry* Find_(jug::StringView _key);

    jug::String                m_defName;
    EnemyDef&                  m_def;
    jug::Vector<ParamEntry>&   m_params;
    jug::String&               m_error;
};

// 행동별 설정 파서. 각 Enemies/<Kind>.cpp 가 구현한다. 실패하면 _reader 에 오류가 기록된다.
void ParseSoldierCfg(DefReader& _reader, EnemyDef& _def);
void ParseBzkCfg(DefReader& _reader, EnemyDef& _def);
void ParseHangCfg(DefReader& _reader, EnemyDef& _def);
void ParseFlyingBzkCfg(DefReader& _reader, EnemyDef& _def);
void ParseShielderCfg(DefReader& _reader, EnemyDef& _def);
void ParsePowCfg(DefReader& _reader, EnemyDef& _def);
void ParseBusCfg(DefReader& _reader, EnemyDef& _def);          // Bus.cpp
void ParseBattleShipCfg(DefReader& _reader, EnemyDef& _def);   // BattleShip.cpp
void ParseJeepCfg(DefReader& _reader, EnemyDef& _def);          // VehicleJeep.cpp
void ParseBTankCfg(DefReader& _reader, EnemyDef& _def);         // VehicleBTank.cpp
void ParseMissleTankCfg(DefReader& _reader, EnemyDef& _def);    // VehicleMissleTank.cpp
void ParseSarubiaCfg(DefReader& _reader, EnemyDef& _def);       // VehicleSarubia.cpp
void ParseAparchCfg(DefReader& _reader, EnemyDef& _def);        // VehicleAparch.cpp

}   // namespace slug

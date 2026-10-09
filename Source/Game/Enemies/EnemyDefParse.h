#pragma once
#include "Base/Json.h"
#include "Base/Types.h"
#include "Base/Math.h"

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

    // --- 수치 (키는 params) ---
    [[nodiscard]] float        Num(slug::StringView _key);                         // 필수
    [[nodiscard]] float        Num(slug::StringView _key, float _default);
    [[nodiscard]] int          Int(slug::StringView _key);
    [[nodiscard]] int          Int(slug::StringView _key, int _default);
    [[nodiscard]] int          Ticks(slug::StringView _key);                       // 초 → 틱 (필수)
    [[nodiscard]] int          Ticks(slug::StringView _key, float _defaultSec);
    [[nodiscard]] bool         Bool(slug::StringView _key, bool _default);
    [[nodiscard]] slug::String  Str(slug::StringView _key);                         // 필수
    [[nodiscard]] Vec2 ReadVec2(slug::StringView _key, Vec2 _default);
    [[nodiscard]] BoxDef       Box(slug::StringView _key);                         // {w,h,cx,cy} 필수
    [[nodiscard]] bool         NumArray(slug::StringView _key, float* _pOut, int _count);   // 길이가 정확히 _count 인 배열. 없으면 false(기본값 유지)
    [[nodiscard]] bool         Has(slug::StringView _key) const;

    // --- 클립 / 무기 (def 안의 논리 이름 → 인덱스) ---
    [[nodiscard]] int Clip(slug::StringView _logical);       // 필수
    [[nodiscard]] int ClipOpt(slug::StringView _logical) const;   // 없으면 -1
    [[nodiscard]] int Weapon(slug::StringView _name);        // 필수
    [[nodiscard]] int WeaponOpt(slug::StringView _name) const;

    void Fail(slug::StringView _field, slug::StringView _message);
    [[nodiscard]] bool HasError() const { return !m_error.empty(); }

    // 사용되지 않은 params 키를 오류로 기록한다. 오류가 없으면 true.
    [[nodiscard]] bool Finish();

private:
    [[nodiscard]] ParamEntry* Find_(slug::StringView _key);

    slug::String                m_defName;
    EnemyDef&                  m_def;
    slug::Vector<ParamEntry>&   m_params;
    slug::String&               m_error;
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

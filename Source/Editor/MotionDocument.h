#pragma once
#include "Base/SlugError.h"
#include "Game/Enemies/EnemyDef.h"

// 무기 하나의 편집 값 묶음
struct MotionWeaponValues
{
    VECTOR2              muzzle     = { 0.f, 0.f };
    bool                 bHasFx     = false;
    VECTOR2              fxOffset   = { 0.f, 0.f };
    String               motionClip = {};
    Vector<WeaponEffect> effects    = {};
};

struct MotionParam
{
    String           label = {};
    Vector<String>   path  = {};
    char             type  = 'n';
    bool             bInt  = false;
    ARRAY<double, 2> v     = {};
};

// 정의 JSON 텍스트를 그대로 들고 바뀐 값의 토큰만 제자리 교체한다
class MotionDocument
{
public:
    [[nodiscard]] Result<void>        Load(const FilePath& _path);
    [[nodiscard]] Result<void>        LoadFromText(StringView _json);
    [[nodiscard]] Result<void>        Save(const FilePath& _path);
    bool                              ApplyWeapon(StringView _def, StringView _weapon, const MotionWeaponValues& _values);
    bool                              ApplyClipSec(StringView _def, StringView _clip, double _sec);
    bool                              ApplyPartDepth(StringView _def, StringView _part, int _depth, int _sort);
    bool                              SetPartDepth(StringView _def, StringView _part, int _depth, int _sort, int _defaultDepth, int _defaultSort);
    [[nodiscard]] Vector<MotionParam> ListParams(StringView _def) const;
    bool                              SetParam(StringView _def, const MotionParam& _param);
    [[nodiscard]] const String&       GetText() const;
    [[nodiscard]] bool                IsDirty() const;

private:
    String m_text      = {};
    String m_savedText = {};
};

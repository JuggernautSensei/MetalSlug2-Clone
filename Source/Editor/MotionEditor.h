#pragma once
#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyDef.h"

class AnimationLibrary;
class EnemyLab;
class Renderer;

// 무기 하나의 편집 값 묶음 (JSON 키: muzzle, muzzleFx.offset, motionClip, effects)
struct MotionWeaponValues
{
    Vec2                 muzzle     = { 0.f, 0.f };
    bool                 bHasFx     = false;
    Vec2                 fxOffset   = { 0.f, 0.f };
    String               motionClip = {};
    Vector<WeaponEffect> effects    = {};
};

// params 의 값 하나: 숫자('n'), 불('b'), [x, y] 두 숫자('v')
struct MotionParam
{
    String         label = {};   // 점 경로 (예: parts.body.depth)
    Vector<String> path  = {};   // params 아래 키 경로
    char           type  = 'n';
    bool           bInt  = false;
    ARRAY<double, 2> v   = {};
};

// 정의 JSON 텍스트를 그대로 들고 바뀐 값의 토큰만 제자리 교체한다 (무편집 저장은 바이트 동일).
class MotionDocument
{
public:
    [[nodiscard]] Result<void> Load(const FilePath& _path);
    [[nodiscard]] Result<void> LoadFromText(StringView _json);
    [[nodiscard]] Result<void> Save(const FilePath& _path);

    // 파일 안에 해당 정의/무기/클립/부위가 직접 있으면 true (상속된 것은 false)
    bool ApplyWeapon(StringView _def, StringView _weapon, const MotionWeaponValues& _values);
    bool ApplyClipSec(StringView _def, StringView _clip, double _sec);
    bool ApplyPartDepth(StringView _def, StringView _part, int _depth, int _sort);

    // params.parts.<part> 을 쓴다: 없는 parts/부품/키는 값이 기본값과 다를 때만 끼워 넣는다 (무편집은 바이트 동일)
    bool SetPartDepth(StringView _def, StringView _part, int _depth, int _sort, int _defaultDepth, int _defaultSort);

    // 정의 자신의 JSON 에 있는 params 값 열거 / 한 값 제자리 교체
    [[nodiscard]] Vector<MotionParam> ListParams(StringView _def) const;
    bool                              SetParam(StringView _def, const MotionParam& _param);

    [[nodiscard]] const String& GetText() const;
    [[nodiscard]] bool          IsDirty() const;

private:
    String m_text      = {};
    String m_savedText = {};
};

// AI 탭 "Motions" 창: 공격 모션(무기/클립/이펙트) 편집 + 월드 십자 표시/드래그 + 부위 깊이(z)
class MotionEditor final
{
public:
    void SetAnimLibrary(const AnimationLibrary* _pAnims);
    void Draw(EnemyLab& _lab);
    void DrawOverlay(EnemyLab& _lab, Renderer& _renderer);

private:
    enum class ePick : int
    {
        None,
        Muzzle,
        MuzzleFx,
        Effect
    };

    void Load_(const FilePath& _assetRoot);
    void Save_(EnemyLab& _lab);
    void MarkChanged_(const String& _key);
    void DrawWeapon_(EnemyLab& _lab, EnemyDef& _def, int _weapon);
    void DrawParts_(EnemyDef& _def);
    void DrawParams_(EnemyLab& _lab, const String& _def);

    const AnimationLibrary* m_pAnims = nullptr;

    Vector<FilePath>       m_paths    = {};
    Vector<MotionDocument> m_docs     = {};
    Vector<String>         m_changed  = {};
    String                 m_status   = {};
    ARRAY<char, 64>        m_filter   = {};
    bool                   m_bLoaded  = false;

    Vector<MotionParam> m_params    = {};
    String              m_paramDef  = {};
    size_t              m_paramDoc  = 0;
    size_t              m_paramSize = 0;

    ePick m_pick       = ePick::None;
    int   m_pickWeapon = 0;
    int   m_pickEffect = 0;
    int   m_fireFrame  = 1;
    bool  m_bPrevDown  = false;
    bool  m_bDragging  = false;
};

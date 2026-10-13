#pragma once
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyDef.h"

class AnimationLibrary;
class EnemyLab;
class Renderer;

#include "Editor/MotionDocument.h"

class MotionEditor
{
public:
    void SetAnimLibrary(const AnimationLibrary* _pAnims);
    void Draw(EnemyLab& _lab);
    void DrawOverlay(EnemyLab& _lab, Renderer& _renderer);

private:
    enum class ePick
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

    Vector<FilePath>       m_paths   = {};
    Vector<MotionDocument> m_docs    = {};
    Vector<String>         m_changed = {};
    String                 m_status  = {};
    ARRAY<char, 64>        m_filter  = {};
    bool                   m_bLoaded = false;

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

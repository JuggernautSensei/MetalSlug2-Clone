#pragma once
#include "Base/Types.h"

class EnemyLab;

struct DepthRow
{
    String name  = {};
    int    value = 0;
    int    file  = -1;
    int    begin = 0;
    int    end   = 0;
};

class DepthEditor
{
public:
    void Draw(EnemyLab& _lab);

private:
    void Load_(const FilePath& _assetRoot);
    void Rebuild_();
    void Patch_(int _row, int _value);
    void DrawParts_(EnemyLab& _lab);
    void PatchPart_(StringView _def, StringView _part, const EnemyPartDepth& _value, const EnemyPartDepth& _default);

    Vector<DepthRow> m_rows    = {};
    Vector<String>   m_names   = {};
    Vector<FilePath> m_paths   = {};
    Vector<String>   m_texts   = {};
    Vector<bool>     m_dirty   = {};
    String           m_status  = {};
    bool             m_bLoaded = false;
};

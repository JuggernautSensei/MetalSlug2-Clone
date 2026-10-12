#pragma once
#include "Base/Types.h"

class EnemyLab;

struct DepthRow
{
    String name  = {};
    int    value = 0;
    int    file  = -1;   // -1 = 코드 상수 (읽기 전용), 그 외 = 정의 JSON 번호
    int    begin = 0;    // JSON 숫자 토큰 위치
    int    end   = 0;
};

// AI 탭의 "Depth" 창: 모든 그리기 depth 를 오름차순 (작을수록 뒤) 으로 보여 준다. JSON 의 depth 값은 편집/저장 가능.
class DepthEditor final
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

#pragma once
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Editor/JsonScan.h"
#include "Game/Enemies/EnemyDef.h"

class EnemyLab;
class Renderer;
class SandboxState;

#include "Editor/HitboxDocument.h"

class HitboxEditor
{
public:
    void Draw(EnemyLab& _lab);
    void DrawOverlay(EnemyLab& _lab, Renderer& _renderer);

private:
    void Load_(const FilePath& _assetRoot);
    void ApplyLive_(SandboxState& _state, const HitboxEntry& _entry);
    void DrawEntries_(EnemyLab& _lab, SandboxState* _pState);
    void DrawExtraBoxes_(SandboxState& _state, HitboxDocument& _doc);
    void DrawExtraOverlay_(EnemyLab& _lab, Renderer& _renderer);

    Vector<String>         m_fileNames   = {};
    Vector<FilePath>       m_filePaths   = {};
    Vector<HitboxDocument> m_docs        = {};
    String                 m_status      = {};
    int                    m_fileIndex   = 0;
    int                    m_ownerIndex  = 0;
    int                    m_activeFile  = -1;
    int                    m_activeEntry = -1;
    bool                   m_bLoaded     = false;

    String   m_ownerName = {};
    int      m_newKind   = 0;
    int      m_boxSel    = -1;
    bool     m_bBoxDrag  = false;
    bool     m_bPrevDown = false;
    uint32_t m_dragEnemy = 0;
    VECTOR2  m_dragGrab  = { 0.f, 0.f };
};

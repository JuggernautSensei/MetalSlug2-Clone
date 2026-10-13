#pragma once
#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyTypes.h"

class Renderer;
class SandboxState;
class SimRunner;

struct EnemyLabSpawn
{
    String  name   = {};
    VECTOR2 feet   = { 0.f, 0.f };
    int     facing = -1;
    int     z      = kDefaultZ;
};

class EnemyLab final
{
public:
    void Init(SimRunner* _pRunner, const String& _stateName, const FilePath& _assetRoot);
    void SetAutoSpawn(const String& _defName);

    EnemyHandle       Spawn(SandboxState& _state, StringView _defName, VECTOR2 _feet, int _facing, int _z = kDefaultZ);
    EnemyHandle       SpawnNearPlayer(SandboxState& _state, StringView _defName, float _offsetX, int _facing, int _z = kDefaultZ);
    void              KillAll(SandboxState& _state);
    void              Clear(SandboxState& _state);
    int               RespawnLast(SandboxState& _state);
    bool              ReloadDefs(SandboxState& _state);
    bool              ReloadDefsKeep(SandboxState& _state);
    void              SetAutoReloadBlocked(bool _bBlocked);
    void              SetMouseCaptured(bool _bCaptured);
    [[nodiscard]] int GetDefIndex() const;
    void              ForgetSet();
    void              SelectAt(const SandboxState& _state, VECTOR2 _world);

    [[nodiscard]] EnemyHandle     GetSelected() const;
    [[nodiscard]] size_t          GetSetCount() const;
    [[nodiscard]] const FilePath& GetAssetRoot() const;

    void                SetPaused(bool _bPaused);
    void                RequestSteps(int _count);
    void                SetTimeScale(float _scale);
    [[nodiscard]] float GetTimeScale() const;
    [[nodiscard]] int   TakeSteps(int _naturalSteps);

    [[nodiscard]] SandboxState* GetSandbox() const;
    void                        EnterLab();
    void                        LeaveLab();

    void Draw();
    void DrawOverlay(Renderer& _renderer);
    void DrawMark(Renderer& _renderer, const Aabb& _world, VECTOR2 _anchor, RGBA _color, StringView _label) const;

private:
    void DrawSpawnUi_(SandboxState& _state);
    void DrawInspector_(SandboxState& _state);
    void HandleMouse_(Renderer& _renderer, SandboxState& _state);
    void PollDefFiles_(SandboxState& _state);

    SimRunner*            m_pRunner     = nullptr;
    FilePath              m_assetRoot   = {};
    String                m_returnState = "title";
    String                m_autoSpawn   = {};
    String                m_status      = {};
    Vector<EnemyLabSpawn> m_set         = {};

    bool  m_bPaused      = false;
    int   m_pendingSteps = 0;
    float m_timeScale    = 1.f;

    int         m_defIndex   = 0;
    float       m_offsetX    = 140.f;
    int         m_facing     = -1;
    int         m_spawnZ     = kDefaultZ;
    bool        m_bPlaceMode = false;
    EnemyHandle m_selected   = {};

    bool m_bShowBoxes  = true;
    bool m_bShowState  = true;
    bool m_bShowTarget = true;
    bool m_bShowProbe  = true;
    bool m_bShowCamera = true;
    bool m_bShowSolids = true;

    bool    m_bAutoBlocked   = false;
    bool    m_bMouseCaptured = false;
    int64_t m_defsStamp      = 0;

    bool m_bPrevMouseDown = false;
    int  m_frame          = 0;
};

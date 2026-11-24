#pragma once
#include <cstdint>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "World/Geometry.h"

inline constexpr int kDefaultTerrainDepth = -90;

#include "Game/Stage/StageParams.h"

enum class eEntityKind
{
    Enemy,
    Npc,
    Prop,
    Decor,
    Item,
    Collider
};

enum class eOrigin
{
    Feet,
    LegacyCenter
};

enum class eStageLayer
{
    FarBackground,
    Background,
    Interact,
    Monster,
    Npc,
    Item,
    FrontBackground,
    Collider,
    Count
};

[[nodiscard]] uint32_t LayerBit(eStageLayer _layer);

enum class eCollisionLayer
{
    Wall,
    GroundExceptPlayer
};

enum class eAnchorX
{
    Left,
    Right,
    Center
};

enum class eAnchorY
{
    Top,
    Bottom,
    Center
};

enum class eActionType
{
    Spawn,
    SpawnWave,
    SetBounds,
    LockCamera,
    UnlockCamera,
    CameraMove,
    SetCameraOffset,
    SetBackdrop,
    SetGround,
    Gate,
    Command,
    Clear,
    Banner,
    Fade,
    PlayBgm,
    PlayerInvincible,
    EndStage
};

enum class eWhen
{
    Normal,
    Silent,
    Always
};

enum class eGateUntil
{
    Defeated,
    DestroyFinished
};

enum class eFadeTarget
{
    Black,
    Clear
};

struct CameraAnchor
{
    eAnchorX ax    = eAnchorX::Left;
    int      dxMin = 0;
    int      dxMax = 0;
    eAnchorY ay    = eAnchorY::Top;
    float    dy    = 0.f;

    [[nodiscard]] bool IsRandomX() const;
};

struct RepeatSpec
{
    int   count = 1;
    float stepX = 0.f;
    float stepY = 0.f;
};

struct AmbushSpec
{
    float xMin = 0.f;
    float xMax = 0.f;
};

inline constexpr float kDefaultCullMargin = 200.f;

struct SpawnSpec
{
    eEntityKind kind = eEntityKind::Enemy;
    String      def  = {};
    String      id   = {};

    bool         bHasX      = false;
    bool         bHasY      = false;
    float        x          = 0.f;
    float        y          = 0.f;
    bool         bHasCamera = false;
    CameraAnchor camera     = {};

    eOrigin origin         = eOrigin::Feet;
    bool    bHasHalfHeight = false;
    float   halfHeight     = 0.f;
    bool    bGroundSnap    = false;
    float   fromY          = 0.f;
    bool    bSettle        = false;

    bool       bHasRepeat = false;
    RepeatSpec repeat     = {};

    int         facing     = 0;
    bool        bHasZ      = false;
    int         z          = 0;
    bool        bHasHp     = false;
    int         hp         = 0;
    String      drop       = {};
    bool        bHasAmbush = false;
    AmbushSpec  ambush     = {};
    bool        bHasLayer  = false;
    eStageLayer layer      = eStageLayer::Monster;
    float       cullMargin = kDefaultCullMargin;

    VECTOR2         size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    String anim   = {};
    String sprite = {};

    ParamValue        params = ParamValue::MakeObject();
    [[nodiscard]] int GetCount() const;
};

struct WaveEntry
{
    float     delay = 0.f;
    SpawnSpec spawn = {};
    float     every = 0.f;   // 시간 반복 주기
    int       times = 1;     // 시간 반복 횟수
};

struct RectRef
{
    bool   bCurrent = false;
    Aabb   rect     = {};
    String name     = {};
};

struct StageAction
{
    eActionType type  = eActionType::Spawn;
    float       delay = 0.f;
    eWhen       when  = eWhen::Normal;

    SpawnSpec         spawn       = {};
    Vector<WaveEntry> waveEntries = {};

    RectRef rect       = {};
    float   glide      = 0.f;
    bool    bPermanent = false;

    VECTOR2 to      = { 0.f, 0.f };
    float   seconds = 0.f;
    VECTOR2 offset  = { 0.f, 0.f };

    String text       = {};
    String mode       = {};
    String name       = {};
    String target     = {};
    String command    = {};
    String next       = {};
    bool   bLoop      = true;
    float  fadeOutSec = 0.f;

    bool     bClearWaves = false;
    uint32_t clearLayers = 0;

    eFadeTarget fadeTo = eFadeTarget::Black;

    String              gateId           = {};
    Vector<String>      waitFor          = {};
    eGateUntil          until            = eGateUntil::Defeated;
    bool                bAbortOnGameOver = false;
    Vector<StageAction> actions          = {};
};

struct StageSection
{
    String              id       = {};
    float               triggerX = 0.f;
    Vector<StageAction> actions  = {};
};

struct BackdropMode
{
    String  id        = {};
    String  sprite    = {};
    VECTOR2 offset    = { 0.f, 0.f };
    float   parallaxX = 0.f;
    float   period    = 0.f;   // 가로 반복 간격
};

struct NamedRect
{
    String name = {};
    Aabb   rect = {};
};

struct BoundsCurve
{
    String          id         = {};
    String          from       = {};
    String          until      = {};
    int             fromIndex  = -1;
    int             untilIndex = -1;
    float           left       = 0.f;
    float           right      = 0.f;
    float           height     = 0.f;
    Vector<VECTOR2> top        = {};
    bool            bTrunc     = false;

    [[nodiscard]] float EvaluateTop(float _playerX) const;
    [[nodiscard]] Aabb  EvaluateBounds(float _playerX) const;
};

struct CameraConfig
{
    bool                bForward      = true;
    Aabb                initialBounds = {};
    VECTOR2             initialOffset = { 0.f, 0.f };
    float               playerMargin  = 14.f;
    Vector<BoundsCurve> boundsCurves  = {};
};

struct PlayerStart
{
    float   x           = 0.f;
    bool    bHasY       = false;
    float   y           = 0.f;
    eOrigin origin      = eOrigin::Feet;
    float   halfHeight  = 0.f;
    bool    bSettle     = false;
    bool    bGroundSnap = false;
    float   fromY       = 0.f;
};

struct EndlessTail
{
    bool  bEnabled = false;
    float fromX    = 0.f;
    float tileX    = 0.f;
    float tileW    = 0.f;
};

struct StageData
{
    int    version = 1;
    String name    = {};

    VECTOR2     worldSize    = { 0.f, 0.f };
    VECTOR2     viewSize     = { 640.f, 448.f };
    String      background   = {};
    EndlessTail endless      = {};
    int         terrainDepth = kDefaultTerrainDepth;

    String bgmName  = {};
    bool   bBgmLoop = true;

    String         groundInitial = {};
    Vector<String> groundMaps    = {};

    String               backdropInitial = {};
    Vector<BackdropMode> backdrops       = {};

    PlayerStart       player = {};
    Vector<NamedRect> rects  = {};
    CameraConfig      camera = {};

    Vector<StageSection> sections = {};

    [[nodiscard]] int                 FindSection(StringView _id) const;
    [[nodiscard]] const NamedRect*    FindRect(StringView _name) const;
    [[nodiscard]] const BackdropMode* FindBackdrop(StringView _id) const;
    [[nodiscard]] bool                HasGroundMap(StringView _name) const;
    [[nodiscard]] int                 CountSpawns() const;
    [[nodiscard]] int                 CountWaveEntries() const;
    [[nodiscard]] int                 CountGates() const;
};

[[nodiscard]] Result<StageData> LoadStageFromJson(StringView _json, String* _pMessage = nullptr);

[[nodiscard]] Result<StageData> LoadStageFromFile(const FilePath& _path, String* _pMessage = nullptr);

[[nodiscard]] bool ShouldRunAction(eWhen _when, bool _bSilent);

[[nodiscard]] const char* GetActionTypeName(eActionType _type);

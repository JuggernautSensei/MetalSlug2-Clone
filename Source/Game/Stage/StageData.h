#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "World/Geometry.h"

inline constexpr int kDefaultTerrainDepth = -90;   // world.terrainDepth 기본: 이 depth 보다 뒤인 스프라이트는 지형 텍스처 앞에 먼저 그린다

class ParamValue
{
public:
    enum class eType : uint8_t
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    [[nodiscard]] static ParamValue MakeBool(bool _bValue);
    [[nodiscard]] static ParamValue MakeNumber(double _value);
    [[nodiscard]] static ParamValue MakeString(String _value);
    [[nodiscard]] static ParamValue MakeArray();
    [[nodiscard]] static ParamValue MakeObject();

    void PushBack(ParamValue _value);
    void Set(String _key, ParamValue _value);

    [[nodiscard]] eType GetType() const;
    [[nodiscard]] bool  IsNull() const;
    [[nodiscard]] bool  IsBool() const;
    [[nodiscard]] bool  IsNumber() const;
    [[nodiscard]] bool  IsString() const;
    [[nodiscard]] bool  IsArray() const;
    [[nodiscard]] bool  IsObject() const;

    [[nodiscard]] bool AsBool(
        bool _bDefault = false) const;

    [[nodiscard]] float AsFloat(
        float _default = 0.f) const;

    [[nodiscard]] int AsInt(
        int _default = 0) const;

    [[nodiscard]] const String& AsString() const;

    [[nodiscard]] size_t             GetSize() const;
    [[nodiscard]] const ParamValue&  At(size_t _index) const;
    [[nodiscard]] Span<const String> GetKeys() const;

    [[nodiscard]] const ParamValue* Find(StringView _key) const;
    [[nodiscard]] bool              Has(StringView _key) const;

    [[nodiscard]] bool GetBool(
        StringView _key,
        bool       _bDefault = false) const;

    [[nodiscard]] int GetInt(
        StringView _key,
        int        _default = 0) const;

    [[nodiscard]] String GetString(
        StringView _key,
        StringView _default = {}) const;

    [[nodiscard]] Vec2 GetVec2(
        StringView _key,
        Vec2       _default = { 0.f, 0.f }) const;

    [[nodiscard]] bool TryGetVec2(Vec2& _out) const;

    [[nodiscard]] static const ParamValue& EmptyObject();
    [[nodiscard]] static const ParamValue& NullValue();

private:
    eType              m_type   = eType::Null;
    bool               m_bValue = false;
    double             m_number = 0.0;
    String             m_string = {};
    Vector<ParamValue> m_items  = {};
    Vector<String>     m_keys   = {};
};

enum class eEntityKind : uint8_t
{
    Enemy,
    Npc,
    Prop,
    Decor,
    Item,
    Collider
};

enum class eOrigin : uint8_t
{
    Feet,
    LegacyCenter
};

enum class eStageLayer : uint8_t
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

[[nodiscard]] uint32_t LayerBit(
    const eStageLayer _layer);

enum class eCollisionLayer : uint8_t
{
    Wall,
    GroundExceptPlayer
};

enum class eAnchorX : uint8_t
{
    Left,
    Right,
    Center
};

enum class eAnchorY : uint8_t
{
    Top,
    Bottom,
    Center
};

enum class eActionType : uint8_t
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

enum class eWhen : uint8_t
{
    Normal,
    Silent,
    Always
};

enum class eGateUntil : uint8_t
{
    Defeated,
    DestroyFinished
};

enum class eFadeTarget : uint8_t
{
    Black,
    Clear
};

struct CameraAnchor
{
    eAnchorX ax    = eAnchorX::Left;
    int      dxMin = 0;   // dx 가 고정값이면 dxMin == dxMax
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

    int         facing     = 0;   // -1 왼쪽, +1 오른쪽, 0 = def 기본
    bool        bHasZ      = false;
    int         z          = 0;   // bHasZ 일 때 인스턴스 depth
    bool        bHasHp     = false;
    int         hp         = 0;
    String      drop       = {};
    bool        bHasAmbush = false;
    AmbushSpec  ambush     = {};
    bool        bHasLayer  = false;
    eStageLayer layer      = eStageLayer::Monster;
    float       cullMargin = kDefaultCullMargin;

    Vec2            size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    String anim   = {};
    String sprite = {};

    ParamValue params = ParamValue::MakeObject();

    [[nodiscard]] int GetCount() const;
};

struct WaveEntry
{
    float     delay = 0.f;   // 초 (액션 실행 시각 기준)
    SpawnSpec spawn = {};
    float     every = 0.f;   // 시간 반복 주기(초). times > 1 이면 필수 (이전 발동 시각 + every)
    int       times = 1;     // 시간 반복 횟수 (1 = 한 번). 취소는 clear waves 가 전체 반복을 지운다
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
    float       delay = 0.f;   // 초
    eWhen       when  = eWhen::Normal;

    SpawnSpec         spawn       = {};
    Vector<WaveEntry> waveEntries = {};

    RectRef rect       = {};
    float   glide      = 0.f;   // LockCamera: 초
    bool    bPermanent = false;

    Vec2  to      = { 0.f, 0.f };   // CameraMove: 목표 뷰 중심
    float seconds = 0.f;
    Vec2  offset  = { 0.f, 0.f };   // SetCameraOffset

    String text    = {};
    String mode    = {};
    String name    = {};
    String target  = {};
    String command = {};
    String next    = {};
    bool   bLoop   = true;
    float  fadeOutSec = 0.f;   // PlayBgm: > 0 이면 현재 BGM 을 줄인 뒤 교체

    bool     bClearWaves = false;
    uint32_t clearLayers = 0;   // LayerBit 마스크

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
    String id        = {};
    String sprite    = {};
    Vec2   offset    = { 0.f, 0.f };
    float  parallaxX = 0.f;
    float  period    = 0.f;   // 가로 반복 간격. 0 이면 스프라이트 너비
};

struct NamedRect
{
    String name = {};
    Aabb   rect = {};
};

struct BoundsCurve
{
    String       id         = {};
    String       from       = {};
    String       until      = {};
    int          fromIndex  = -1;
    int          untilIndex = -1;
    float        left       = 0.f;
    float        right      = 0.f;
    float        height     = 0.f;
    Vector<Vec2> top        = {};
    bool         bTrunc     = false;

    [[nodiscard]] float EvaluateTop(float _playerX) const;

    [[nodiscard]] Aabb EvaluateBounds(float _playerX) const;
};

struct CameraConfig
{
    bool                bForward      = true;
    Aabb                initialBounds = {};
    Vec2                initialOffset = { 0.f, 0.f };
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

// 엔드리스 지형 (보스전): x >= fromX 의 지면 비트맵과 배경 텍스처를 [tileX, tileX + tileW) 구간의 주기 반복으로 본다.
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

    Vec2        worldSize  = { 0.f, 0.f };
    Vec2        viewSize   = { 640.f, 448.f };
    String      background = {};
    EndlessTail endless    = {};
    int         terrainDepth = kDefaultTerrainDepth;   // 지형의 depth: 이보다 작은 depth 의 스프라이트 (보스 선체) 는 지형 텍스처보다 먼저 그려 모래가 덮는다

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

    [[nodiscard]] int CountSpawns() const;
    [[nodiscard]] int CountWaveEntries() const;
    [[nodiscard]] int CountGates() const;
};

struct StageLoadDiagnostics
{
    String message = {};
};

[[nodiscard]] Result<StageData> LoadStageFromJson(
    StringView            _json,
    StageLoadDiagnostics* _pDiag = nullptr);

[[nodiscard]] Result<StageData> LoadStageFromFile(
    const FilePath&       _path,
    StageLoadDiagnostics* _pDiag = nullptr);

[[nodiscard]] bool ShouldRunAction(
    const eWhen _when,
    const bool  _bSilent);

[[nodiscard]] const char* GetEntityKindName(eEntityKind _kind);
[[nodiscard]] const char* GetActionTypeName(eActionType _type);

#pragma once
#include <cstdint>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "World/Geometry.h"

namespace slug
{
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
    [[nodiscard]] static ParamValue MakeString(slug::String _value);
    [[nodiscard]] static ParamValue MakeArray();
    [[nodiscard]] static ParamValue MakeObject();

    void PushBack(ParamValue _value);
    void Set(slug::String _key, ParamValue _value);

    [[nodiscard]] eType GetType() const { return m_type; }
    [[nodiscard]] bool  IsNull() const { return m_type == eType::Null; }
    [[nodiscard]] bool  IsBool() const { return m_type == eType::Bool; }
    [[nodiscard]] bool  IsNumber() const { return m_type == eType::Number; }
    [[nodiscard]] bool  IsString() const { return m_type == eType::String; }
    [[nodiscard]] bool  IsArray() const { return m_type == eType::Array; }
    [[nodiscard]] bool  IsObject() const { return m_type == eType::Object; }

    [[nodiscard]] bool              AsBool(bool _bDefault = false) const;
    [[nodiscard]] double            AsNumber(double _default = 0.0) const;
    [[nodiscard]] float             AsFloat(float _default = 0.f) const;
    [[nodiscard]] int               AsInt(int _default = 0) const;
    [[nodiscard]] const slug::String& AsString() const;

    [[nodiscard]] size_t GetSize() const;
    [[nodiscard]] const ParamValue& At(size_t _index) const;
    [[nodiscard]] slug::Span<const slug::String> GetKeys() const { return m_keys; }

    [[nodiscard]] const ParamValue* Find(slug::StringView _key) const;
    [[nodiscard]] bool              Has(const slug::StringView _key) const { return Find(_key) != nullptr; }

    [[nodiscard]] bool             GetBool(slug::StringView _key, bool _bDefault = false) const;
    [[nodiscard]] float            GetFloat(slug::StringView _key, float _default = 0.f) const;
    [[nodiscard]] int              GetInt(slug::StringView _key, int _default = 0) const;
    [[nodiscard]] slug::String      GetString(slug::StringView _key, slug::StringView _default = {}) const;
    [[nodiscard]] Vec2     GetVec2(slug::StringView _key, Vec2 _default = { 0.f, 0.f }) const;

    [[nodiscard]] bool TryGetVec2(Vec2& _out) const;

    [[nodiscard]] static const ParamValue& EmptyObject();
    [[nodiscard]] static const ParamValue& NullValue();

private:
    eType                    m_type   = eType::Null;
    bool                     m_bValue = false;
    double                   m_number = 0.0;
    slug::String              m_string = {};
    slug::Vector<ParamValue>  m_items  = {};
    slug::Vector<slug::String> m_keys   = {};
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

[[nodiscard]] constexpr uint32_t LayerBit(
    const eStageLayer _layer)
{
    return 1u << static_cast<unsigned>(_layer);
}

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

    [[nodiscard]] bool IsRandomX() const { return dxMax != dxMin; }
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
    slug::String def  = {};
    slug::String id   = {};

    bool         bHasX      = false;
    bool         bHasY      = false;
    float        x          = 0.f;
    float        y          = 0.f;
    bool         bHasCamera = false;
    CameraAnchor camera     = {};

    eOrigin origin     = eOrigin::Feet;
    bool    bHasHalfHeight = false;
    float   halfHeight = 0.f;
    bool    bGroundSnap = false;
    float   fromY       = 0.f;
    bool    bSettle     = false;

    bool       bHasRepeat = false;
    RepeatSpec repeat     = {};

    int         facing      = 0;   // -1 왼쪽, +1 오른쪽, 0 = def 기본
    bool        bHasHp      = false;
    int         hp          = 0;
    slug::String drop        = {};
    bool        bHasAmbush  = false;
    AmbushSpec  ambush      = {};
    bool        bHasLayer   = false;
    eStageLayer layer       = eStageLayer::Monster;
    float       cullMargin  = kDefaultCullMargin;

    Vec2    size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    slug::String anim   = {};
    slug::String sprite = {};

    ParamValue params = ParamValue::MakeObject();

    [[nodiscard]] int GetCount() const { return bHasRepeat ? repeat.count : 1; }
};

struct WaveEntry
{
    float     delay = 0.f;   // 초 (액션 실행 시각 기준)
    SpawnSpec spawn = {};
};

struct RectRef
{
    bool        bCurrent = false;
    Aabb        rect     = {};
    slug::String name     = {};
};

struct StageAction
{
    eActionType type  = eActionType::Spawn;
    float       delay = 0.f;   // 초
    eWhen       when  = eWhen::Normal;

    SpawnSpec              spawn = {};
    slug::Vector<WaveEntry> waveEntries = {};

    RectRef rect       = {};
    float   glide      = 0.f;     // LockCamera: 초
    bool    bPermanent = false;

    Vec2 to      = { 0.f, 0.f };   // CameraMove: 목표 뷰 중심
    float        seconds = 0.f;
    Vec2 offset  = { 0.f, 0.f };   // SetCameraOffset

    slug::String text    = {};
    slug::String mode    = {};
    slug::String name    = {};
    slug::String target  = {};
    slug::String command = {};
    slug::String next    = {};
    bool        bLoop   = true;

    bool     bClearWaves = false;
    uint32_t clearLayers = 0;   // LayerBit 마스크

    eFadeTarget fadeTo = eFadeTarget::Black;

    slug::String              gateId = {};
    slug::Vector<slug::String> waitFor = {};
    eGateUntil               until = eGateUntil::Defeated;
    bool                     bAbortOnGameOver = false;
    slug::Vector<StageAction> actions = {};
};

struct StageSection
{
    slug::String              id      = {};
    slug::String              legacy  = {};
    float                    triggerX = 0.f;
    slug::Vector<StageAction> actions = {};
};

struct BackdropMode
{
    slug::String  id        = {};
    slug::String  sprite    = {};
    Vec2 offset    = { 0.f, 0.f };
    float        parallaxX = 0.f;
    float        period    = 0.f;   // 가로 반복 간격. 0 이면 스프라이트 너비
};

struct NamedRect
{
    slug::String name = {};
    Aabb        rect = {};
};

struct BoundsCurve
{
    slug::String                id     = {};
    slug::String                from   = {};
    slug::String                until  = {};
    int                        fromIndex  = -1;
    int                        untilIndex = -1;
    float                      left   = 0.f;
    float                      right  = 0.f;
    float                      height = 0.f;
    slug::Vector<Vec2>  top    = {};
    bool                       bTrunc = false;

    [[nodiscard]] float EvaluateTop(float _playerX) const;

    [[nodiscard]] Aabb EvaluateBounds(float _playerX) const;
};

struct CameraConfig
{
    bool                     bForward     = true;
    Aabb                     initialBounds = {};
    Vec2             initialOffset = { 0.f, 0.f };
    float                    chaseSpeed    = 10.f;
    float                    playerMargin  = 14.f;
    slug::Vector<BoundsCurve> boundsCurves  = {};
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

struct StageData
{
    int         version = 1;
    slug::String name    = {};
    slug::String title   = {};

    Vec2 worldSize  = { 0.f, 0.f };
    Vec2 viewSize   = { 640.f, 448.f };
    slug::String  background = {};

    slug::String bgmName = {};
    bool        bBgmLoop = true;

    slug::String              groundInitial = {};
    slug::Vector<slug::String> groundMaps    = {};

    slug::String                backdropInitial = {};
    slug::Vector<BackdropMode>  backdrops       = {};

    PlayerStart             player = {};
    slug::Vector<NamedRect>  rects  = {};
    CameraConfig            camera = {};

    slug::Vector<StageSection> sections = {};

    [[nodiscard]] int                 FindSection(slug::StringView _id) const;
    [[nodiscard]] const NamedRect*    FindRect(slug::StringView _name) const;
    [[nodiscard]] const BackdropMode* FindBackdrop(slug::StringView _id) const;
    [[nodiscard]] bool                HasGroundMap(slug::StringView _name) const;

    [[nodiscard]] int CountSpawns() const;
    [[nodiscard]] int CountWaveEntries() const;
    [[nodiscard]] int CountGates() const;
};

struct StageLoadDiagnostics
{
    slug::String message = {};
};

[[nodiscard]] slug::Result<StageData> LoadStageFromJson(slug::StringView _json, StageLoadDiagnostics* _pDiag = nullptr);
[[nodiscard]] slug::Result<StageData> LoadStageFromFile(const slug::FilePath& _path, StageLoadDiagnostics* _pDiag = nullptr);

[[nodiscard]] constexpr bool ShouldRunAction(
    const eWhen _when,
    const bool  _bSilent)
{
    return _when == eWhen::Always || (_bSilent ? _when == eWhen::Silent : _when == eWhen::Normal);
}

[[nodiscard]] const char* GetEntityKindName(eEntityKind _kind);
[[nodiscard]] const char* GetActionTypeName(eActionType _type);
}

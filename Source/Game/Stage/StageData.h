#pragma once
#include <cstdint>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "World/Geometry.h"

namespace slug
{

// ===========================================
//  스테이지 스크립트 데이터 (Docs/StageFormat.md 의 타입 표현)
//   Assets/Stage/*.json 을 엄격하게 읽어 값 타입으로 만든다. 로드 이후에는 불변 데이터로 취급한다.
// ===========================================

// ===========================================
//  ParamValue
//   spawn.params 처럼 def 별로 해석이 달라지는 임의 JSON 값을 담는 작은 DOM.
//   로더는 구조만 검증하고, 키의 의미(알 수 없는 키 거부)는 def 쪽 책임이다.
// ===========================================

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

    // --- 생성 ---
    [[nodiscard]] static ParamValue MakeBool(bool _bValue);
    [[nodiscard]] static ParamValue MakeNumber(double _value);
    [[nodiscard]] static ParamValue MakeString(slug::String _value);
    [[nodiscard]] static ParamValue MakeArray();
    [[nodiscard]] static ParamValue MakeObject();

    void PushBack(ParamValue _value);                       // 배열
    void Set(slug::String _key, ParamValue _value);          // 객체 (같은 키는 덮어쓴다)

    // --- 조회 ---
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
    [[nodiscard]] const slug::String& AsString() const;   // 문자열이 아니면 빈 문자열

    // 배열/객체의 원소 수 (그 외 0)
    [[nodiscard]] size_t GetSize() const;
    [[nodiscard]] const ParamValue& At(size_t _index) const;                  // 배열. 범위 밖이면 Null 값
    [[nodiscard]] slug::Span<const slug::String> GetKeys() const { return m_keys; }

    // 객체에서 키로 찾는다. 없거나 객체가 아니면 nullptr.
    [[nodiscard]] const ParamValue* Find(slug::StringView _key) const;
    [[nodiscard]] bool              Has(const slug::StringView _key) const { return Find(_key) != nullptr; }

    // 객체 필드 편의 조회 (없거나 타입이 다르면 기본값)
    [[nodiscard]] bool             GetBool(slug::StringView _key, bool _bDefault = false) const;
    [[nodiscard]] float            GetFloat(slug::StringView _key, float _default = 0.f) const;
    [[nodiscard]] int              GetInt(slug::StringView _key, int _default = 0) const;
    [[nodiscard]] slug::String      GetString(slug::StringView _key, slug::StringView _default = {}) const;
    [[nodiscard]] Vec2     GetVec2(slug::StringView _key, Vec2 _default = { 0.f, 0.f }) const;

    // 이 값이 [x, y] 숫자 배열이면 true 와 함께 읽는다
    [[nodiscard]] bool TryGetVec2(Vec2& _out) const;

    // 모든 spawn 이 공유하는 "비어 있는 객체" (params 가 없을 때 가리킨다)
    [[nodiscard]] static const ParamValue& EmptyObject();
    [[nodiscard]] static const ParamValue& NullValue();

private:
    eType                    m_type   = eType::Null;
    bool                     m_bValue = false;
    double                   m_number = 0.0;
    slug::String              m_string = {};
    slug::Vector<ParamValue>  m_items  = {};   // 배열 원소 또는 객체 값
    slug::Vector<slug::String> m_keys   = {};   // 객체 키 (m_items 와 같은 순서)
};

// ===========================================
//  열거형
// ===========================================

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
    Feet,           // (x, y) = 발 중앙
    LegacyCenter    // (x, y) = 레거시 SetPos (정적 스프라이트 중앙 / 애니 피벗 / 충돌체 중심)
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

// 레이어 비트마스크 (clear 필터)
[[nodiscard]] constexpr uint32_t LayerBit(
    const eStageLayer _layer)
{
    return 1u << static_cast<unsigned>(_layer);
}

enum class eCollisionLayer : uint8_t
{
    Wall,                // 플레이어와 적 모두 막는다
    GroundExceptPlayer   // 적만 서는 바닥 (원웨이 지붕 등)
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

// normal: 일반 플레이에서만 / silent: 디버그 워프(건너뜀)에서만 / always: 둘 다
enum class eWhen : uint8_t
{
    Normal,
    Silent,
    Always
};

enum class eGateUntil : uint8_t
{
    Defeated,         // HP <= 0 또는 제거
    DestroyFinished   // 사망 연출 종료
};

enum class eFadeTarget : uint8_t
{
    Black,
    Clear
};

// ===========================================
//  spawn
// ===========================================

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
    slug::String id   = {};   // 비어 있으면 참조되지 않는 스폰

    // 위치: 일반 좌표 또는 카메라 기준 (둘은 배타적)
    bool         bHasX      = false;
    bool         bHasY      = false;
    float        x          = 0.f;
    float        y          = 0.f;
    bool         bHasCamera = false;
    CameraAnchor camera     = {};

    eOrigin origin     = eOrigin::Feet;   // 기본값은 kind 로 정해진다 (enemy/npc = Feet, 그 외 LegacyCenter)
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

    // kind == Collider
    Vec2    size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    // kind == Decor
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

// ===========================================
//  rect 참조
// ===========================================

struct RectRef
{
    bool        bCurrent = false;   // "current": 실행 시점의 카메라 뷰 영역
    Aabb        rect     = {};      // bCurrent 가 아닐 때의 사각형 (이름 참조는 로드 시 풀어 둔다)
    slug::String name     = {};      // 이름 참조였다면 그 이름 (진단용)
};

// ===========================================
//  action
// ===========================================

struct StageAction
{
    eActionType type  = eActionType::Spawn;
    float       delay = 0.f;   // 초
    eWhen       when  = eWhen::Normal;

    SpawnSpec              spawn = {};          // Spawn
    slug::Vector<WaveEntry> waveEntries = {};    // SpawnWave

    RectRef rect       = {};      // SetBounds / LockCamera
    float   glide      = 0.f;     // LockCamera: 초
    bool    bPermanent = false;   // LockCamera

    Vec2 to      = { 0.f, 0.f };   // CameraMove: 목표 뷰 중심
    float        seconds = 0.f;            // CameraMove / Banner / Fade / PlayerInvincible
    Vec2 offset  = { 0.f, 0.f };   // SetCameraOffset

    slug::String text    = {};   // Banner
    slug::String mode    = {};   // SetBackdrop
    slug::String name    = {};   // SetGround / PlayBgm
    slug::String target  = {};   // Command: spawn id
    slug::String command = {};   // Command
    slug::String next    = {};   // EndStage
    bool        bLoop   = true; // PlayBgm

    // Clear
    bool     bClearWaves = false;
    uint32_t clearLayers = 0;   // LayerBit 마스크

    eFadeTarget fadeTo = eFadeTarget::Black;

    // Gate
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

// ===========================================
//  최상위 설정
// ===========================================

struct BackdropMode
{
    slug::String  id        = {};
    slug::String  sprite    = {};
    Vec2 offset    = { 0.f, 0.f };
    float        parallaxX = 0.f;
};

struct NamedRect
{
    slug::String name = {};
    Aabb        rect = {};
};

struct BoundsCurve
{
    slug::String                id     = {};
    slug::String                from   = {};   // 섹션 id (발동 시점부터 활성)
    slug::String                until  = {};   // 섹션 id (발동 직전까지 활성)
    int                        fromIndex  = -1;
    int                        untilIndex = -1;
    float                      left   = 0.f;
    float                      right  = 0.f;
    float                      height = 0.f;
    slug::Vector<Vec2>  top    = {};   // (playerX, 경계 윗변 y) 꺾은선, x 오름차순
    bool                       bTrunc = false;

    // 꺾은선을 _playerX 에서 평가한다 (양 끝 밖은 끝 값으로 clamp, bTrunc 면 정수 절사).
    [[nodiscard]] float EvaluateTop(float _playerX) const;

    // 평가한 윗변으로 만든 경계 사각형 [left, T, right, T + height]
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
    slug::String  background = {};   // sprites.json 키

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

    // --- 조회 ---
    [[nodiscard]] int                 FindSection(slug::StringView _id) const;   // 없으면 -1
    [[nodiscard]] const NamedRect*    FindRect(slug::StringView _name) const;
    [[nodiscard]] const BackdropMode* FindBackdrop(slug::StringView _id) const;
    [[nodiscard]] bool                HasGroundMap(slug::StringView _name) const;

    // --- 통계 (Scripts/ValidateStage.ps1 과 같은 규칙: repeat 는 count 로 센다, 게이트 안도 포함) ---
    [[nodiscard]] int CountSpawns() const;
    [[nodiscard]] int CountWaveEntries() const;
    [[nodiscard]] int CountGates() const;
};

// ===========================================
//  로더
// ===========================================

// 로드 실패 시 사람이 읽을 수 있는 첫 오류 (섹션 id / 액션 번호 / 필드 경로 포함)
struct StageLoadDiagnostics
{
    slug::String message = {};
};

// 실패하면 eError::StageInvalid (JSON 문법 오류는 eError::Parse). _pDiag 가 있으면 메시지를 채운다.
[[nodiscard]] slug::Result<StageData> LoadStageFromJson(slug::StringView _json, StageLoadDiagnostics* _pDiag = nullptr);
[[nodiscard]] slug::Result<StageData> LoadStageFromFile(const slug::FilePath& _path, StageLoadDiagnostics* _pDiag = nullptr);

// 액션이 `when` 규칙상 이 실행 모드에서 실행되는가
[[nodiscard]] constexpr bool ShouldRunAction(
    const eWhen _when,
    const bool  _bSilent)
{
    return _when == eWhen::Always || (_bSilent ? _when == eWhen::Silent : _when == eWhen::Normal);
}

[[nodiscard]] const char* GetEntityKindName(eEntityKind _kind);
[[nodiscard]] const char* GetActionTypeName(eActionType _type);

}   // namespace slug

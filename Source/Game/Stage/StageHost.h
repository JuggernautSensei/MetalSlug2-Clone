#pragma once
#include <cstdint>
#include <optional>

#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Game/Stage/StageData.h"

namespace slug
{

// ===========================================
//  IStageHost
//   StageRunner 가 월드에 대해 요구하는 모든 것. 적/소품/UI/오디오 구현을 몰라도 스크립트를 실행할 수 있게 하는 좁은 경계이다.
//   - 실제 게임(Stage0State)은 EnemyWorld/Hud/Audio 로 구현하고, 테스트는 Test/StageTestUtil.h 의 FakeStageHost 로 구현한다.
//   - 카메라는 이 인터페이스를 거치지 않는다 (StageRunner 가 Render/Camera 를 직접 조작한다).
//   - 모든 호출은 시뮬레이션 틱 안에서(결정적 순서로) 일어난다.
// ===========================================

using SpawnId = uint32_t;
inline constexpr SpawnId kInvalidSpawnId = 0;

// SpawnRequest::position 의 의미
enum class ePositionMode : uint8_t
{
    Feet,         // 발 중앙(바닥 접지점)
    LegacyCenter  // 레거시 SetPos: 정적 스프라이트 중앙 / 애니 피벗 / 충돌체 중심 (변환 없이 그 점에 그린다)
};

// 엔티티 생성 요청. 스크립트의 spawn 한 개(또는 repeat 의 i 번째)가 해소된 결과이다.
struct SpawnRequest
{
    eEntityKind kind = eEntityKind::Enemy;
    jug::String def  = {};   // 엔티티 def 이름 (soldier_bomb, jeep, truck, shop_5, pow, anim_decor, box_collider, random_fruit ...)
    jug::String stageId = {};   // 스크립트의 spawn id (없으면 빈 문자열). 게이트/명령은 러너가 SpawnId 로 대응시킨다.

    // 해소된 위치
    //  - positionMode == Feet         : 발 중앙 (enemy/npc 의 기본. legacyCenter + halfHeight 로 쓴 크리처도 여기서 발로 변환되어 온다)
    //  - positionMode == LegacyCenter : 레거시 중심 (prop/decor/item/collider 의 기본)
    //  groundSnap 은 러너가 host.GetGroundFindY 로 풀어서 y 를 확정해 보낸다.
    jug::VECTOR2  position     = { 0.f, 0.f };
    ePositionMode positionMode = ePositionMode::Feet;
    float         halfHeight   = 0.f;   // 스크립트가 legacyCenter + halfHeight 로 쓴 크리처의 반높이 (비행체: 중심 y = position.y - halfHeight)

    int         facing = 0;   // -1 왼쪽, +1 오른쪽, 0 = def 기본
    bool        bHasHp = false;
    int         hp     = 0;   // bHasHp 일 때 def 기본 HP 를 덮어쓴다
    jug::String drop   = {};  // 드롭 아이템 ID (gunitem_mg, random_score ...)

    bool bSettle        = false;   // 이 위치에서 시작해 중력으로 착지 (POW 등)
    bool bFromCamera    = false;   // 카메라 기준 좌표(웨이브)로 만들어진 스폰 — 화면 밖에서 진입하는 적
    bool bWave          = false;   // spawnWave 로 지연 생성된 것
    bool bSilent        = false;   // 디버그 워프의 silent 실행 (연출/전투 없이 상태만 맞춘다)
    int  repeatIndex    = 0;       // repeat 의 몇 번째 (0 부터)
    int  sectionIndex   = -1;

    bool        bHasLayer  = false;
    eStageLayer layer      = eStageLayer::Monster;   // bHasLayer 일 때 def 기본 레이어를 덮어쓴다
    float       cullMargin = kDefaultCullMargin;     // x < camLeft - margin 이면 제거
    bool        bHasAmbush = false;
    AmbushSpec  ambush     = {};

    // collider
    jug::VECTOR2    size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    // decor
    jug::String anim   = {};
    jug::String sprite = {};

    // def 별 인스턴스 파라미터 (알 수 없는 키 거부는 def 의 책임). 항상 유효하며 호출 동안 읽을 수 있다. StageData 가 살아 있는 동안 유지된다.
    const ParamValue* pParams = &ParamValue::EmptyObject();
};

// ClearEntities 필터: layerMask 는 LayerBit(eStageLayer) 의 합
struct EntityFilter
{
    uint32_t layerMask = 0;
};

class IStageHost
{
public:
    virtual ~IStageHost() = default;

    // --- 엔티티 ---
    // 엔티티를 만들고 ID 를 돌려준다. 만들지 못하면 kInvalidSpawnId (러너는 "건너뜀" 으로 취급한다).
    virtual SpawnId SpawnEntity(const SpawnRequest& _request) = 0;

    // 게이트 조건. 제거되었거나 HP <= 0 이면 격파됨. (알 수 없는 ID 는 격파로 본다)
    [[nodiscard]] virtual bool IsDefeated(SpawnId _id) const = 0;

    // 사망 연출까지 끝나 제거(또는 연출 종료)되었는가
    [[nodiscard]] virtual bool IsDestroyFinished(SpawnId _id) const = 0;

    virtual void ClearEntities(const EntityFilter& _filter) = 0;

    // 엔티티에 명령 (예: mb_truck "startBattle")
    virtual void SendCommand(SpawnId _id, jug::StringView _command) = 0;

    // --- 지면 ---
    virtual void SetGround(jug::StringView _name) = 0;

    // 현재 활성 지면 맵에서 열 _x 의 _fromY 부터 아래로 처음 만나는 고체 픽셀 행. 없으면 nullopt (러너는 스폰을 건너뛴다)
    [[nodiscard]] virtual std::optional<float> GetGroundFindY(float _x, float _fromY) const = 0;

    // --- 플레이어 ---
    [[nodiscard]] virtual jug::VECTOR2 PlayerFeet() const = 0;
    [[nodiscard]] virtual bool         PlayerAlive() const = 0;
    [[nodiscard]] virtual bool         IsGameOver() const = 0;
    virtual void                       SetPlayerInvincible(float _seconds) = 0;

    // 디버그 워프: 플레이어를 발 위치 _feet 로 옮긴다 (속도 초기화, 카메라는 러너가 맞춘다)
    virtual void WarpPlayer(jug::VECTOR2 _feet) = 0;

    // --- 연출 / UI / 오디오 ---
    virtual void SetBackdrop(jug::StringView _modeId) = 0;
    virtual void ShowBanner(const jug::String& _text, float _seconds) = 0;   // _text 의 '\n' 은 줄바꿈
    virtual void FadeTo(eFadeTarget _target, float _seconds) = 0;
    virtual void PlayBgm(jug::StringView _name, bool _bLoop) = 0;

    // --- 흐름 ---
    // 스테이지 종료 (러너는 IsFinished() == true 가 된다). _nextState 는 보통 "title".
    virtual void EndStage(jug::StringView _nextState) = 0;

    // 섹션이 발동했음을 알린다 (디버그 UI/체크포인트용). 기본은 무시.
    virtual void OnSectionEntered(int /*_index*/, jug::StringView /*_id*/) {}
};

}   // namespace slug

#pragma once
#include <cstdint>
#include <optional>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Stage/StageData.h"

namespace slug
{
using SpawnId = uint32_t;
inline constexpr SpawnId kInvalidSpawnId = 0;

enum class ePositionMode : uint8_t
{
    Feet,
    LegacyCenter
};

struct SpawnRequest
{
    eEntityKind kind = eEntityKind::Enemy;
    slug::String def  = {};
    slug::String stageId = {};

    Vec2  position     = { 0.f, 0.f };
    ePositionMode positionMode = ePositionMode::Feet;
    float         halfHeight   = 0.f;

    int         facing = 0;   // -1 왼쪽, +1 오른쪽, 0 = def 기본
    bool        bHasHp = false;
    int         hp     = 0;   // bHasHp 일 때 def 기본 HP 를 덮어쓴다
    slug::String drop   = {};

    bool bSettle        = false;
    bool bFromCamera    = false;
    bool bWave          = false;
    bool bSilent        = false;
    int  repeatIndex    = 0;       // repeat 의 몇 번째 (0 부터)
    int  sectionIndex   = -1;

    bool        bHasLayer  = false;
    eStageLayer layer      = eStageLayer::Monster;
    float       cullMargin = kDefaultCullMargin;
    bool        bHasAmbush = false;
    AmbushSpec  ambush     = {};

    Vec2    size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    slug::String anim   = {};
    slug::String sprite = {};

    const ParamValue* pParams = &ParamValue::EmptyObject();
};

struct EntityFilter
{
    uint32_t layerMask = 0;
};

class IStageHost
{
public:
    virtual ~IStageHost() = default;

    virtual SpawnId SpawnEntity(const SpawnRequest& _request) = 0;

    [[nodiscard]] virtual bool IsDefeated(SpawnId _id) const = 0;

    [[nodiscard]] virtual bool IsDestroyFinished(SpawnId _id) const = 0;

    virtual void ClearEntities(const EntityFilter& _filter) = 0;

    virtual void SendCommand(SpawnId _id, slug::StringView _command) = 0;

    virtual void SetGround(slug::StringView _name) = 0;

    [[nodiscard]] virtual std::optional<float> GetGroundFindY(float _x, float _fromY) const = 0;

    [[nodiscard]] virtual Vec2 PlayerFeet() const = 0;
    [[nodiscard]] virtual bool         PlayerAlive() const = 0;
    [[nodiscard]] virtual bool         IsGameOver() const = 0;
    virtual void                       SetPlayerInvincible(float _seconds) = 0;

    virtual void WarpPlayer(Vec2 _feet) = 0;

    virtual void SetBackdrop(slug::StringView _modeId) = 0;
    virtual void ShowBanner(const slug::String& _text, float _seconds) = 0;   // _text 의 '\n' 은 줄바꿈
    virtual void FadeTo(eFadeTarget _target, float _seconds) = 0;
    virtual void PlayBgm(slug::StringView _name, bool _bLoop) = 0;

    virtual void EndStage(slug::StringView _nextState) = 0;

    virtual void OnSectionEntered(int, slug::StringView) {}
};
}

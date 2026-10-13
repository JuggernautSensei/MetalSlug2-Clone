#pragma once
#include <cstdint>
#include <optional>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Stage/StageData.h"

using SpawnId                            = uint32_t;
inline constexpr SpawnId kInvalidSpawnId = 0;

enum class ePositionMode : uint8_t
{
    Feet,
    LegacyCenter
};

struct SpawnRequest
{
    eEntityKind kind    = eEntityKind::Enemy;
    String      def     = {};
    String      stageId = {};

    VECTOR2       position     = { 0.f, 0.f };
    ePositionMode positionMode = ePositionMode::Feet;
    float         halfHeight   = 0.f;

    int    facing = 0;   // -1 왼쪽, +1 오른쪽, 0 = def 기본
    bool   bHasZ  = false;
    int    z      = 0;
    bool   bHasHp = false;
    int    hp     = 0;
    String drop   = {};

    bool bSettle      = false;
    bool bFromCamera  = false;
    bool bWave        = false;
    bool bSilent      = false;
    int  repeatIndex  = 0;
    int  sectionIndex = -1;

    bool        bHasLayer  = false;
    eStageLayer layer      = eStageLayer::Monster;
    float       cullMargin = kDefaultCullMargin;
    bool        bHasAmbush = false;
    AmbushSpec  ambush     = {};

    VECTOR2         size           = { 0.f, 0.f };
    eCollisionLayer collisionLayer = eCollisionLayer::Wall;

    String anim   = {};
    String sprite = {};

    const ParamValue* pParams = &ParamValue::EmptyObject();
};

struct EntityFilter
{
    uint32_t layerMask = 0;
};

class IStageHost
{
public:
    IStageHost()          = default;
    virtual ~IStageHost() = default;

    virtual SpawnId SpawnEntity(const SpawnRequest& _request) = 0;

    [[nodiscard]] virtual bool IsDefeated(SpawnId _id) const        = 0;
    [[nodiscard]] virtual bool IsDestroyFinished(SpawnId _id) const = 0;

    virtual void ClearEntities(const EntityFilter& _filter)    = 0;
    virtual void SendCommand(SpawnId _id, StringView _command) = 0;
    virtual void SetGround(StringView _name)                   = 0;

    [[nodiscard]] virtual std::optional<float> GetGroundFindY(float _x, float _fromY) const = 0;

    [[nodiscard]] virtual VECTOR2 PlayerFeet() const                  = 0;
    [[nodiscard]] virtual bool    IsGameOver() const                  = 0;
    virtual void                  SetPlayerInvincible(float _seconds) = 0;

    virtual void WarpPlayer(VECTOR2 _feet) = 0;

    virtual void SetBackdrop(StringView _modeId)                             = 0;
    virtual void ShowBanner(const String& _text, float _seconds)             = 0;
    virtual void FadeTo(eFadeTarget _target, float _seconds)                 = 0;
    virtual void PlayBgm(StringView _name, bool _bLoop)                      = 0;
    virtual void ChangeBgm(StringView _name, bool _bLoop, float _fadeOutSec) = 0;
    virtual void PlaySfx(StringView _name)                                   = 0;

    virtual void EndStage(StringView _nextState) = 0;

    virtual void OnSectionEntered(int _index, StringView _id);
};

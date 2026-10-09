#pragma once
#include <cstdint>

#include "Base/Types.h"

#include "Audio/AudioSink.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/UiSystem.h"

namespace slug
{

// ===========================================
//  ScreenFlow
//   "배너 → 암전 → 다음 장면 요청" 이라는 끝 장면 흐름을 틱 타이머로 구현한 순수 로직.
//   GAME OVER 와 MISSION COMPLETE 가 같은 틀을 쓰고, 스테이지 러너는 Start() 한 번 + 매 틱 Tick() + IsDone() 만 본다.
//
//   타임라인 (Start() 를 부른 틱을 0 으로, Tick() 마다 +1):
//     [0, delay)                         Delay    : 아무것도 안 함 (승리 포즈 등을 보여 줄 시간)
//     [delay, delay+banner)              Banner   : 효과음 + 배너 (페이드 인/유지/아웃은 배너 시스템이 진행)
//     [delay+banner, +fade)              FadeOut  : 화면이 검정으로
//     [.., +hold)                        Hold     : 검정 유지
//     그 뒤                              Done     : IsDone() == true  → 호출자가 "title" 상태를 요청한다
//
//   Start() 시점에 BGM 을 페이드 아웃한다 (bgmFadeSeconds <= 0 이면 즉시 정지).
//   UiSystem::Tick() 은 호출자가 매 틱 따로 부른다 (배너/페이드 알파 진행). 플로 자체의 단계는 자기 틱만 센다.
// ===========================================

enum class eFlowPhase : uint8_t
{
    Idle,
    Delay,
    Banner,
    FadeOut,
    Hold,
    Done
};

struct ScreenFlowConfig
{
    slug::String bannerText      = {};
    eFontColor  bannerColor     = eFontColor::Yellow;
    slug::String sfx             = {};     // 배너 시작 시 효과음 (비면 없음)
    float       bgmFadeSeconds  = 1.f;    // Start() 시 BGM 페이드 아웃
    int         delayTicks      = 0;
    int         bannerTicks     = 120;
    int         fadeTicks       = 40;
    int         holdTicks       = 20;

    [[nodiscard]] int GetTotalTicks() const { return delayTicks + bannerTicks + fadeTicks + holdTicks; }
};

// GAME OVER: 즉시 배너(2초) + 암전(0.67초) + 유지 → 총 3초 (PlayerSpec: "GAME OVER 3초 후 타이틀")
[[nodiscard]] ScreenFlowConfig MakeGameOverFlowConfig();
// MISSION COMPLETE: 승리 포즈 1.5초 → 배너 4초 → 암전 1초 → 유지
[[nodiscard]] ScreenFlowConfig MakeMissionCompleteFlowConfig();

class ScreenFlow
{
public:
    void Start(const ScreenFlowConfig& _config, UiSystem& _ui, IAudioSink& _audio);
    void Tick(UiSystem& _ui, IAudioSink& _audio);
    void Reset();

    [[nodiscard]] eFlowPhase GetPhase() const { return m_phase; }
    [[nodiscard]] bool       IsRunning() const { return m_phase != eFlowPhase::Idle && m_phase != eFlowPhase::Done; }
    [[nodiscard]] bool       IsDone() const { return m_phase == eFlowPhase::Done; }
    [[nodiscard]] int        GetElapsedTicks() const { return m_elapsed; }
    [[nodiscard]] const ScreenFlowConfig& GetConfig() const { return m_config; }

private:
    void BeginBanner_(UiSystem& _ui, IAudioSink& _audio);
    void BeginFade_(UiSystem& _ui);
    void Advance_(UiSystem& _ui, IAudioSink& _audio);

    ScreenFlowConfig m_config  = {};
    eFlowPhase       m_phase   = eFlowPhase::Idle;
    int              m_elapsed = 0;
};

}   // namespace slug

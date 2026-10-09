#pragma once
#include <JugX/Application.h>
#include <JugX/Vector2.h>

#include "Core/FixedStep.h"
#include "Core/Input.h"
#include "Render/Renderer.h"

namespace slug
{

// ===========================================
//  App
//   JugX Application 파생. 가변 프레임 → 고정 60Hz 시뮬레이션 + 보간 렌더.
// ===========================================

class App final : public jug::Application
{
public:
    App();
    ~App() override;

protected:
    void Init() override;
    void Shutdown() override;
    void OnEvent(jug::Event& _event) override;
    void Update(float _deltaTimeSec) override;

private:
    void OnKeyDown_(const jug::KeyDownEvent& _event);
    void OnKeyUp_(const jug::KeyUpEvent& _event);
    void OnFocusLost_(const jug::WindowFocusLostEvent& _event);

    void Tick_();
    void Render_(float _alpha);

    Renderer       m_renderer = {};
    InputSystem    m_input    = {};
    InputHistory   m_history  = {};
    FixedStepClock m_clock    = {};

    // M0 스모크: 입력으로 움직이는 사각형 (고정 스텝 + 보간 확인용)
    jug::VECTOR2 m_prevPos = { 320.f, 224.f };
    jug::VECTOR2 m_pos     = { 320.f, 224.f };
    uint64_t     m_tickCount = 0;

    // QA 캡처: --capture <path> [--capture-frame <n>]  n 번째 렌더 프레임을 BMP 로 저장하고 종료
    jug::FilePath m_capturePath     = {};
    int           m_captureFrame    = 30;
    int           m_renderFrameCount = 0;
};

}   // namespace slug

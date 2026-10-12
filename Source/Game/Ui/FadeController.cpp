#include "Game/Ui/FadeController.h"

#include <algorithm>
#include <cmath>

void FadeController::SetClear()
{
    m_alpha     = 0.f;
    m_target    = 0.f;
    m_step      = 0.f;
    m_ticksLeft = 0;
    m_phase     = ePhase::Clear;
    m_donePhase = ePhase::Clear;
}

void FadeController::SetBlack()
{
    m_alpha     = 1.f;
    m_target    = 1.f;
    m_step      = 0.f;
    m_ticksLeft = 0;
    m_phase     = ePhase::Black;
    m_donePhase = ePhase::Black;
}

void FadeController::Begin_(
    const float  _target,
    const int    _ticks,
    const ePhase _movingPhase,
    const ePhase _donePhase)
{
    m_target    = _target;
    m_donePhase = _donePhase;

    if (_ticks <= 0 || m_alpha == _target)
    {
        m_alpha     = _target;
        m_step      = 0.f;
        m_ticksLeft = 0;
        m_phase     = _donePhase;
        return;
    }

    m_ticksLeft = _ticks;
    m_step      = (_target - m_alpha) / static_cast<float>(_ticks);
    m_phase     = _movingPhase;
}

void FadeController::FadeToBlack(
    const int _ticks)
{
    Begin_(1.f, _ticks, ePhase::FadingToBlack, ePhase::Black);
}

void FadeController::FadeToClear(
    const int _ticks)
{
    Begin_(0.f, _ticks, ePhase::FadingToClear, ePhase::Clear);
}

void FadeController::Tick()
{
    if (!IsBusy())
    {
        return;
    }

    --m_ticksLeft;
    if (m_ticksLeft <= 0)
    {
        m_alpha = m_target;
        m_phase = m_donePhase;
        return;
    }
    m_alpha = std::clamp(m_alpha + m_step, 0.f, 1.f);
}

uint8_t FadeController::GetAlpha255() const
{
    return static_cast<uint8_t>(std::lround(std::clamp(m_alpha, 0.f, 1.f) * 255.f));
}

float FadeController::GetAlpha() const
{
    return m_alpha;
}

FadeController::ePhase FadeController::GetPhase() const
{
    return m_phase;
}

bool FadeController::IsBusy() const
{
    return m_phase == ePhase::FadingToBlack || m_phase == ePhase::FadingToClear;
}

bool FadeController::IsBlack() const
{
    return m_phase == ePhase::Black;
}

bool FadeController::IsClear() const
{
    return m_phase == ePhase::Clear;
}

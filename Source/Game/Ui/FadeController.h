#pragma once
#include <cstdint>

class FadeController
{
public:
    enum class ePhase
    {
        Clear,
        FadingToBlack,
        Black,
        FadingToClear
    };

    void                  SetClear();
    void                  SetBlack();
    void                  FadeToBlack(int _ticks);
    void                  FadeToClear(int _ticks);
    void                  Tick();
    [[nodiscard]] float   GetAlpha() const;
    [[nodiscard]] bool    IsBusy() const;
    [[nodiscard]] bool    IsBlack() const;
    [[nodiscard]] bool    IsClear() const;
    [[nodiscard]] uint8_t GetAlpha255() const;

private:
    void Begin_(float _target, int _ticks, ePhase _movingPhase, ePhase _donePhase);

    float  m_alpha     = 0.f;
    float  m_target    = 0.f;
    float  m_step      = 0.f;
    int    m_ticksLeft = 0;
    ePhase m_phase     = ePhase::Clear;
    ePhase m_donePhase = ePhase::Clear;
};

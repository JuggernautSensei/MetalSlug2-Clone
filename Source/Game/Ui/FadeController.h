#pragma once
#include <cstdint>

namespace slug
{

// ===========================================
//  FadeController
//   장면 전환용 화면 암전. 검정 오버레이의 불투명도(0 = 투명, 1 = 완전 검정)를 틱 단위로 진행한다.
//   FadeToBlack / FadeToClear 는 현재 알파에서 목표까지 _ticks 틱에 걸쳐 선형으로 움직인다 (이미 목표면 즉시 완료).
// ===========================================

class FadeController
{
public:
    enum class ePhase : uint8_t
    {
        Clear,          // 알파 0, 대기
        FadingToBlack,
        Black,          // 알파 1, 대기 (검정 유지)
        FadingToClear
    };

    // 초기 상태: 투명 / 검정
    void SetClear();
    void SetBlack();

    void FadeToBlack(int _ticks);
    void FadeToClear(int _ticks);

    void Tick();

    [[nodiscard]] float  GetAlpha() const { return m_alpha; }
    [[nodiscard]] ePhase GetPhase() const { return m_phase; }
    [[nodiscard]] bool   IsBusy() const { return m_phase == ePhase::FadingToBlack || m_phase == ePhase::FadingToClear; }
    [[nodiscard]] bool   IsBlack() const { return m_phase == ePhase::Black; }
    [[nodiscard]] bool   IsClear() const { return m_phase == ePhase::Clear; }
    [[nodiscard]] uint8_t GetAlpha255() const;

private:
    void Begin_(float _target, int _ticks, ePhase _movingPhase, ePhase _donePhase);

    float  m_alpha       = 0.f;
    float  m_target      = 0.f;
    float  m_step        = 0.f;   // 틱당 변화량 (부호 포함)
    int    m_ticksLeft   = 0;
    ePhase m_phase       = ePhase::Clear;
    ePhase m_donePhase   = ePhase::Clear;
};

}   // namespace slug

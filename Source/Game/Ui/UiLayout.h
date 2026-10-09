#pragma once
#include <cstdint>

namespace slug
{

// ===========================================
//  UiLayout
//   HUD/배너/타이틀의 모든 배치 수치. 논리 해상도 640x448 기준 픽셀.
//   코드에 숫자를 흩뿌리지 않고 여기서만 바꾼다 (레거시 GameUI 의 좌표를 출발점으로 위쪽으로 정리).
//   "right"/"center" 로 끝나는 값은 정렬 기준점, "Y" 는 그 요소의 위쪽(또는 중심) y 이다.
// ===========================================

struct UiLayout
{
    int screenW = 640;
    int screenH = 448;

    // --- 글리프 ---
    int glyphSize   = 16;   // 비트맵 폰트 한 칸 (Font_*.bmp 는 16x16 격자 35열 x 2행)
    int timeGlyphW  = 32;   // TimeNumberUI 한 칸 (32x32, 숫자 10칸)
    int timeGlyphH  = 32;

    // --- 점수 / 목숨 (왼쪽 위) ---
    int scoreRightX = 148;   // 점수 오른쪽 끝
    int scoreY      = 8;
    int livesX      = 16;    // "1UP X3" 왼쪽
    int livesY      = 28;

    // --- ARMS / BOMB 패널 (PlayerStateUI_*, 124x34) ---
    int armsPanelX     = 154;   // 패널 왼쪽 위
    int armsPanelY     = 8;
    int ammoRightDX    = 60;    // 패널 기준 탄 숫자 오른쪽 끝
    int weaponLetterDX = 6;     // 패널 기준 무기 글자 왼쪽
    int bombRightDX    = 110;   // 패널 기준 폭탄 숫자 오른쪽 끝
    int digitsDY       = 17;    // 패널 기준 숫자 위쪽
    int infiniteCenterDX = 38;  // 패널 기준 무한대 기호 중심
    int infiniteCenterDY = 25;

    // --- 타이머 (위쪽 중앙, TimeNumberUI) ---
    int timerCenterX = 320;
    int timerTopY    = 8;

    // --- 2P PRESS START (오른쪽 위) ---
    int pressStartCenterX = 494;
    int pressStartCenterY = 24;
    int pressStartBlinkTicks = 30;   // 켜짐/꺼짐 각각의 길이

    // --- 레벨 표시 (아래 중앙) ---
    int levelCenterX = 320;
    int levelCenterY = 432;

    // --- 보스 HP 바 (PlayerHPUI 106x16 을 barScale 배로) ---
    int bossBarCenterX = 320;
    int bossBarTopY    = 388;
    int bossBarScale   = 2;
    int bossBarInnerX  = 4;    // 스프라이트 기준 안쪽 채움 영역
    int bossBarInnerY  = 4;
    int bossBarInnerW  = 94;
    int bossBarInnerH  = 8;

    // --- 배너 ---
    int bannerCenterY   = 176;   // 배너 텍스트 블록 중심 y
    int bannerMaxScale  = 3;
    int bannerMargin    = 32;    // 좌우 여백 (자동 배율 계산)
    int bannerLineGap   = 4;     // 줄 간격 (배율 1 기준 px)

    // --- 타이틀 ---
    int titleNameY            = 150;   // "MY METAL SLUG 2" 윗줄
    int titlePressStartY      = 396;   // "PRESS START" 윗줄
    int titleHintY            = 430;   // 힌트 문구 윗줄
    int titleBlinkTicks       = 30;
    int titleFadeTicks        = 24;    // 시작 시 암전
};

}   // namespace slug

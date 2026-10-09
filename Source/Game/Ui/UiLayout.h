#pragma once
#include <cstdint>

namespace slug
{
struct UiLayout
{
    int screenW = 640;
    int screenH = 448;

    int glyphSize   = 16;
    int timeGlyphW  = 32;   // TimeNumberUI 한 칸 (32x32, 숫자 10칸)
    int timeGlyphH  = 32;

    int scoreRightX = 148;   // 점수 오른쪽 끝
    int scoreY      = 8;
    int livesX      = 16;    // "1UP X3" 왼쪽
    int livesY      = 28;

    int armsPanelX     = 154;   // 패널 왼쪽 위
    int armsPanelY     = 8;
    int ammoRightDX    = 60;    // 패널 기준 탄 숫자 오른쪽 끝
    int weaponLetterDX = 6;     // 패널 기준 무기 글자 왼쪽
    int bombRightDX    = 110;   // 패널 기준 폭탄 숫자 오른쪽 끝
    int digitsDY       = 17;    // 패널 기준 숫자 위쪽
    int infiniteCenterDX = 38;  // 패널 기준 무한대 기호 중심
    int infiniteCenterDY = 25;

    int timerCenterX = 320;
    int timerTopY    = 8;

    int insertCoinRightX = 612;      // 원본 "INSERT COIN" 오른쪽 끝 (깜빡이지 않음)
    int insertCoinY      = 8;

    int missionLine1Y     = 144;     // 미션 배너 첫 줄 중심 y (원본 영상 기준, 두 줄이면 두 번째는 +stride)
    int missionLineStride = 92;
    int missionGlyphNum   = 5;       // 미션 폰트 배율 = num/den (트림된 글자 약 25px 기준 약 1.67배)
    int missionGlyphDen   = 3;
    int missionSpaceW     = 22;      // 공백 폭 (배율 적용 전 px)
    int missionGlyphGap   = 2;       // 글자 사이 간격 (배율 적용 전 px)

    // 구출 결과 상자 (원본 "RECAPTURED PRISONER", 논리 좌표)
    int resultsBoxX       = 43;
    int resultsBoxY       = 100;
    int resultsBoxW       = 270;
    int resultsBoxH       = 200;
    int resultsTitleY     = 81;      // 제목 중심 y
    int resultsTitleX     = 298;     // 제목 중심 x (상자 중심과 다르다, 원본 영상 그대로)
    int resultsPlayerY    = 124;     // "1P" 중심 y
    int resultsCountTopY  = 154;     // 구출 수 숫자 윗변 (TimeNumberUI 32px)
    int resultsScoreTopY  = 204;     // 보너스 점수 숫자 윗변
    int resultsRankY      = 251;     // 등급 문구 중심 y
    int resultsFadeTicks  = 12;

    int pressStartCenterX = 494;
    int pressStartCenterY = 24;
    int pressStartBlinkTicks = 30;   // 켜짐/꺼짐 각각의 길이

    int levelCenterX = 320;
    int levelCenterY = 432;

    int bossBarCenterX = 320;
    int bossBarTopY    = 388;
    int bossBarScale   = 2;
    int bossBarInnerX  = 4;    // 스프라이트 기준 안쪽 채움 영역
    int bossBarInnerY  = 4;
    int bossBarInnerW  = 94;
    int bossBarInnerH  = 8;

    int bannerCenterY   = 176;   // 배너 텍스트 블록 중심 y
    int bannerMaxScale  = 3;
    int bannerMargin    = 32;    // 좌우 여백 (자동 배율 계산)
    int bannerLineGap   = 4;     // 줄 간격 (배율 1 기준 px)

    int titleNameY            = 150;   // "MY METAL SLUG 2" 윗줄
    int titlePressStartY      = 396;   // "PRESS START" 윗줄
    int titleHintY            = 430;   // 힌트 문구 윗줄
    int titleBlinkTicks       = 30;
    int titleFadeTicks        = 24;    // 시작 시 암전
};
}

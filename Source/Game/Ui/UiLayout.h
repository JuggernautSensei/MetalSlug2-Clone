#pragma once
#include <cstdint>

struct UiLayout
{
    int screenW = 640;
    int screenH = 448;

    int timeGlyphW = 32;   // TimeNumberUI 한 칸 (32x32, 숫자 10칸)
    int timeGlyphH = 32;

    int scoreRightX = 148;
    int scoreY      = 8;
    int livesX      = 16;
    int livesY      = 28;

    int armsPanelX       = 154;
    int armsPanelY       = 8;
    int ammoRightDX      = 60;
    int weaponLetterDX   = 6;
    int bombRightDX      = 110;
    int digitsDY         = 17;
    int infiniteCenterDX = 38;    // 패널 기준 무한대 기호 중심
    int infiniteCenterDY = 25;

    int gaugeX      = 24;   // 슬러그 게이지 프레임 왼쪽 위 (PlayerHPUI 106x16)
    int gaugeY      = 20;
    int gaugeInnerX = 4;
    int gaugeInnerY = 4;
    int gaugeInnerW = 94;
    int gaugeInnerH = 8;

    int timerCenterX = 320;
    int timerTopY    = 8;

    int insertCoinRightX = 612;
    int insertCoinY      = 8;

    int missionLine1Y     = 144;
    int missionLineStride = 92;
    int missionGlyphNum   = 5;
    int missionGlyphDen   = 3;
    int missionSpaceW     = 22;
    int missionGlyphGap   = 2;

    int resultsBoxX      = 43;
    int resultsBoxY      = 100;
    int resultsBoxW      = 270;
    int resultsBoxH      = 200;
    int resultsTitleY    = 81;
    int resultsTitleX    = 298;
    int resultsPlayerY   = 124;
    int resultsCountTopY = 154;
    int resultsScoreTopY = 204;
    int resultsRankY     = 251;

    int pressStartBlinkTicks = 30;

    int levelCenterX = 320;
    int levelCenterY = 432;

    int bossBarCenterX = 320;
    int bossBarTopY    = 56;   // 상단 HUD 줄 (점수/타이머 y 8..40) 아래 중앙
    int bossBarScale   = 2;
    int bossBarInnerX  = 4;
    int bossBarInnerY  = 4;
    int bossBarInnerW  = 94;
    int bossBarInnerH  = 8;

    int titleNameY       = 150;
    int titlePressStartY = 386;
    int titleHintY       = 428;
    int titleBlinkTicks  = 30;
};

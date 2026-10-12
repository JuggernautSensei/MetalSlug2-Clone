// 에디터 전용 무적 모드 (GameContext/PlayerContext 의 godMode). 기본값(일반/헤드리스)은 Off 이다.

#include <gtest/gtest.h>

#include "Core/Tuning.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"

static constexpr int kLethalHits = 6;

struct GodHarness
{
    Tuning           tuning = {};
    PlayerController player = {};
    InputHistory     history = {};
    eGodMode         mode   = eGodMode::Off;

    explicit GodHarness(const eGodMode _mode)
        : mode(_mode)
    {
        player.Reset(tuning, { 100.f, 378.f });
    }

    void Tick()
    {
        history.Push(InputFrame {});
        PlayerContext ctx;
        ctx.pTuning = &tuning;
        ctx.godMode = mode;
        player.Tick(ctx, history);
    }
};

TEST(PlayerGodMode, DefaultContextsAreOff)
{
    EXPECT_EQ(GameContext {}.godMode, eGodMode::Off);
    EXPECT_EQ(PlayerContext {}.godMode, eGodMode::Off);
}

TEST(PlayerGodMode, OffDiesAndLosesLives)
{
    GodHarness h { eGodMode::Off };
    h.Tick();
    h.player.Kill();
    EXPECT_FALSE(h.player.IsAlive());
    EXPECT_EQ(h.player.GetLives(), h.tuning.player.lives - 1);
}

TEST(PlayerGodMode, NoDeathNeverDiesOrLosesLives)
{
    GodHarness h { eGodMode::NoDeath };
    for (int i = 0; i < kLethalHits; ++i)
    {
        h.Tick();
        h.player.Kill();
        EXPECT_TRUE(h.player.IsAlive());
    }
    EXPECT_EQ(h.player.GetLives(), h.tuning.player.lives);
    EXPECT_FALSE(h.player.IsGameOver());
    EXPECT_FALSE(h.player.MakeHurtbox(h.tuning).bInvulnerable);
}

TEST(PlayerGodMode, InvincibleIgnoresDamageAndHurtboxIsInvulnerable)
{
    GodHarness h { eGodMode::Invincible };
    for (int i = 0; i < kLethalHits; ++i)
    {
        h.Tick();
        h.player.Kill();
    }
    EXPECT_TRUE(h.player.IsAlive());
    EXPECT_EQ(h.player.GetLives(), h.tuning.player.lives);
    EXPECT_TRUE(h.player.MakeHurtbox(h.tuning).bInvulnerable);
}

TEST(PlayerGodMode, ModeFollowsContextEachTick)
{
    GodHarness h { eGodMode::Invincible };
    h.Tick();
    h.mode = eGodMode::Off;
    h.Tick();
    h.player.Kill();
    EXPECT_FALSE(h.player.IsAlive());
}

TEST(PlayerInfiniteAmmo, ContextDefaultIsOff)
{
    EXPECT_FALSE(GameContext {}.bInfiniteAmmo);
    EXPECT_FALSE(PlayerContext {}.bInfiniteAmmo);
}

TEST(PlayerInfiniteAmmo, RefillsBombsAndFiniteAmmoEveryTick)
{
    GodHarness h { eGodMode::Off };
    h.player.PickUpWeapon(eWeapon::Hmg, h.tuning);
    h.player.AddBombs(-h.player.GetWeapon().bombs);
    EXPECT_EQ(h.player.GetWeapon().bombs, 0);
    PlayerContext ctx;
    ctx.pTuning       = &h.tuning;
    ctx.bInfiniteAmmo = true;
    h.history.Push(InputFrame {});
    h.player.Tick(ctx, h.history);
    EXPECT_GE(h.player.GetWeapon().bombs, 10);
    EXPECT_GE(h.player.GetWeapon().ammo, 99);
}

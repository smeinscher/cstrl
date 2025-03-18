//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#include "moon_game.h"
#include <stdio.h>

static void *game_dl = NULL;

static time_t g_last_edit_ts = 0;

#ifdef CSTRL_PLATFORM_WINDOWS
#include "windows.h"
void hot_reload()
{
    if (game_dl)
    {
        FreeLibrary(game_dl);
        game_dl = NULL;
    }
    if (cstrl_copy_file("build-debug/cstrl-moon-game.lib", "build-debug/cstrl-moon-game-load.lib"))
    {
        return;
    }
    game_dl = LoadLibrary("build-debug/cstrl-moon-game-load.lib");
}

void reload_game()
{
    time_t current_ts = cstrl_get_file_timestamp("cstrl-moon-game.lib");
    if (current_ts < g_last_edit_ts)
    {
        return;
    }
    hot_reload();
}
#endif
int main()
{
    render_state_t render_state;
    game_state_t game_state;
    cstrl_platform_state platform_state;
    int result = moon_game_init(&render_state, &game_state, &platform_state);
    double previous_time = cstrl_platform_get_absolute_time();
    double lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        moon_game_update(&game_state, &platform_state, &previous_time, &lag);
        moon_game_render(&render_state, &game_state, &platform_state);
    }
    moon_game_shutdown(&render_state, &game_state, &platform_state);
#ifdef CSTRL_PLATFORM_WINDOWS
    FreeLibrary(game_dl);
#endif
    return result;
}

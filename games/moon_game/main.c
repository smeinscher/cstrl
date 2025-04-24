//
// Created by sterling on 12/5/24.
//

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#include "moon_game.h"
#include <stdio.h>

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
    return result;
}

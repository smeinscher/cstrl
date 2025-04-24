#include "game.h"
#include "cstrl/cstrl_platform.h"

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;
static const char *WINDOW_TITLE = "RPG";

static cstrl_platform_state platform_state;

static double previous_frame_time = 0.0;
static double frame_lag = 0.0;

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
}

GAME_API bool rpg_game_init()
{
    if (!cstrl_platform_init(&platform_state, WINDOW_TITLE, (1920 - WINDOW_WIDTH) / 2, (1080 - WINDOW_HEIGHT) / 2,
                             WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        cstrl_platform_shutdown(&platform_state);
        return false;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);
    return true;
}

GAME_API void rpg_game_update()
{
    cstrl_platform_pump_messages(&platform_state);
    double current_frame_time = cstrl_platform_get_absolute_time();
    double elapsed_frame_time = current_frame_time - previous_frame_time;
    previous_frame_time = current_frame_time;
    frame_lag += elapsed_frame_time;
    while (frame_lag >= 1.0 / 60.0)
    {
        frame_lag -= 1.0 / 60.0;
    }
}

GAME_API void rpg_game_render()
{
}

GAME_API void rpg_game_shutdown()
{
    cstrl_platform_shutdown(&platform_state);
}

GAME_API bool rpg_game_is_running()
{
    return !cstrl_platform_should_exit();
}

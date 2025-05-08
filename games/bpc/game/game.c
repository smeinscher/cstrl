#include "game.h"
#include "cstrl/cstrl_audio.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "scenes/scene_manager.h"
#include "scenes/scenes.h"
#include "util/game_random.h"
#include <stdio.h>
#include <stdlib.h>

static const int WINDOW_WIDTH = 1920;  // 1280;
static const int WINDOW_HEIGHT = 1080; // 720;
static const char *WINDOW_TITLE = "Beer Pong Champion";

int bpc_game_run()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, WINDOW_TITLE, (1920 - WINDOW_WIDTH) / 2, (1080 - WINDOW_HEIGHT) / 2,
                             WINDOW_WIDTH, WINDOW_HEIGHT, true))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    if (!cstrl_audio_init())
    {
        cstrl_platform_shutdown(&platform_state);
        return 2;
    }

    game_random_init((int)cstrl_platform_get_absolute_time());

    cstrl_renderer_init(&platform_state);

    scenes_init(&platform_state);
    scene_set(LOGO_SCENE, NULL);

    double previous_frame_time = cstrl_platform_get_absolute_time();
    double frame_lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_platform_pump_messages(&platform_state);
        double current_frame_time = cstrl_platform_get_absolute_time();
        double elapsed_frame_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;
        frame_lag += elapsed_frame_time;
        while (frame_lag >= 1.0 / 60.0)
        {
            scene_update();
            frame_lag -= 1.0 / 60.0;
        } // end update loop

        scene_render();

        cstrl_renderer_swap_buffers(&platform_state);
    }

    scene_shutdown();
    scenes_clean();

    cstrl_renderer_shutdown(&platform_state);
    cstrl_audio_shutdown();
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

#include "game.h"
#include "cstrl/cstrl_audio.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "random/cozy_random.h"
#include "scenes/gameplay_scene.h"
#include "scenes/main_menu_scene.h"
#include "scenes/scene_manager.h"
#include "ui/cozy_ui.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Cozy Spring Game"

int cozy_game_run()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, WINDOW_TITLE, (1920 - WINDOW_WIDTH) / 2, (1080 - WINDOW_HEIGHT) / 2,
                             WINDOW_WIDTH, WINDOW_HEIGHT, false))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    if (!cstrl_audio_init())
    {
        cstrl_platform_shutdown(&platform_state);
        return 2;
    }

    cozy_random_init((int)cstrl_platform_get_absolute_time());

    cstrl_renderer_init(&platform_state);

    scene_set(MAIN_MENU_SCENE, &platform_state);
    cozy_ui_init(&platform_state);
    main_menu_scene_init(&platform_state);

    double previous_frame_time = cstrl_platform_get_absolute_time();
    double frame_lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        cstrl_renderer_clear(0.24f, 0.28f, 0.66f, 1.0f);
        cstrl_platform_pump_messages(&platform_state);
        double current_frame_time = cstrl_platform_get_absolute_time();
        double elapsed_frame_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;
        frame_lag += elapsed_frame_time;
        while (frame_lag >= 1.0 / 60.0)
        {
            scene_update(&platform_state);
            frame_lag -= 1.0 / 60.0;
        } // end update loop

        scene_render(&platform_state);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    scene_shutdown(&platform_state);

    cozy_ui_shutdown();
    cstrl_renderer_shutdown(&platform_state);
    cstrl_audio_shutdown();
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

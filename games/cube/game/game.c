#include "game.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "helpers.h"
#include "scenes/scene_manager.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "cube"

int cube()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, WINDOW_TITLE, (SCREEN_WIDTH - WINDOW_WIDTH) / 2,
                             (SCREEN_HEIGHT - WINDOW_HEIGHT) / 2, 1280, 720, false))
    {
        return 1;
    }

    cstrl_renderer_init(&platform_state);

    scene_set(MAIN_MENU_SCENE, &platform_state);
    double previous_frame_time = cstrl_platform_get_absolute_time();
    double frame_lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        cstrl_renderer_clear(0.15f, 0.1f, 0.1f, 1.0f);
        cstrl_platform_pump_messages(&platform_state);
        double current_frame_time = cstrl_platform_get_absolute_time();
        double elapsed_frame_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;
        frame_lag += elapsed_frame_time;
        while (frame_lag >= 1.0 / 60.0)
        {
            scene_update(&platform_state);
            frame_lag -= 1.0 / 60.0;
        }
        scene_render(&platform_state);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

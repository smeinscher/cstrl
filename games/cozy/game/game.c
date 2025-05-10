#include "game.h"
#include "cstrl/cstrl_audio.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "random/cozy_random.h"
#include "scenes/gameplay_scene.h"
#include "scenes/main_menu_scene.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Cozy Spring Game"

CSTRL_PACKED_ENUM{MAIN_MENU_SCENE, TRANSITION_TO_GAMEPLAY_SCENE, GAMEPLAY_SCENE,
                  TRANSITION_TO_MAIN_MENU_SCENE} g_scene_state;

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

    g_scene_state = MAIN_MENU_SCENE;
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
            switch (g_scene_state)
            {
            case MAIN_MENU_SCENE:
                if (main_menu_scene_update(&platform_state))
                {
                    g_scene_state = TRANSITION_TO_GAMEPLAY_SCENE;
                }
                break;
            case GAMEPLAY_SCENE:
                if (gameplay_scene_update(&platform_state))
                {
                    g_scene_state = TRANSITION_TO_MAIN_MENU_SCENE;
                }
                break;
            case TRANSITION_TO_GAMEPLAY_SCENE:
                main_menu_scene_shutdown(&platform_state);
                gameplay_scene_init(&platform_state);
                g_scene_state = GAMEPLAY_SCENE;
                break;
            case TRANSITION_TO_MAIN_MENU_SCENE:
                gameplay_scene_shutdown(&platform_state);
                main_menu_scene_init(&platform_state);
                g_scene_state = MAIN_MENU_SCENE;
                break;
            }
            frame_lag -= 1.0 / 60.0;
        } // end update loop

        switch (g_scene_state)
        {
        case TRANSITION_TO_GAMEPLAY_SCENE:
        case MAIN_MENU_SCENE:
            main_menu_scene_render(&platform_state);
            break;
        case TRANSITION_TO_MAIN_MENU_SCENE:
        case GAMEPLAY_SCENE:
            gameplay_scene_render(&platform_state);
            break;
        }
        cstrl_renderer_swap_buffers(&platform_state);
    }

    switch (g_scene_state)
    {
    case TRANSITION_TO_GAMEPLAY_SCENE:
    case MAIN_MENU_SCENE:
        main_menu_scene_shutdown(&platform_state);
        break;
    case TRANSITION_TO_MAIN_MENU_SCENE:
    case GAMEPLAY_SCENE:
        gameplay_scene_shutdown(&platform_state);
        break;
    }

    cstrl_renderer_shutdown(&platform_state);
    cstrl_audio_shutdown();
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

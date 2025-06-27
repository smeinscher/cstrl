#include "scene_manager.h"
#include "main_menu_scene.h"

void (*g_current_scene_update)(cstrl_platform_state *) = NULL;
void (*g_current_scene_render)(cstrl_platform_state *) = NULL;
void (*g_current_scene_shutdown)(cstrl_platform_state *) = NULL;

void scene_set(game_scene_t game_scene, cstrl_platform_state *platform_state)
{
    scene_shutdown(platform_state);
    switch (game_scene)
    {
    case MAIN_MENU_SCENE:
        main_menu_scene_init(platform_state);
        g_current_scene_update = main_menu_scene_update;
        g_current_scene_render = main_menu_scene_render;
        g_current_scene_shutdown = main_menu_scene_shutdown;
        break;
    case GAMEPLAY_SCENE:
        // gameplay_scene_init(platform_state);
        // g_current_scene_update = gameplay_scene_update;
        // g_current_scene_render = gameplay_scene_render;
        // g_current_scene_shutdown = gameplay_scene_shutdown;
        // break;
    default:
        g_current_scene_update = NULL;
        g_current_scene_render = NULL;
        break;
    }
}

void scene_update(cstrl_platform_state *platform_state)
{
    if (g_current_scene_update != NULL)
    {
        g_current_scene_update(platform_state);
    }
}

void scene_render(cstrl_platform_state *platform_state)
{
    if (g_current_scene_render != NULL)
    {
        g_current_scene_render(platform_state);
    }
}

void scene_shutdown(cstrl_platform_state *platform_state)
{
    if (g_current_scene_shutdown != NULL)
    {
        g_current_scene_shutdown(platform_state);
    }
}

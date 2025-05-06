#include "scene_manager.h"
#include "scenes.h"

void (*g_current_scene_update)(void) = NULL;
void (*g_current_scene_render)(void) = NULL;
void (*g_current_scene_shutdown)(void) = NULL;

void scene_set(game_scene_t game_scene, void *user_data)
{
    scene_shutdown();
    switch (game_scene)
    {
    case LOGO_SCENE:
        logo_scene_init(user_data);
        g_current_scene_update = logo_scene_update;
        g_current_scene_render = logo_scene_render;
        g_current_scene_shutdown = logo_scene_shutdown;
        break;
    case MAIN_MENU_SCENE:
        main_menu_scene_init(user_data);
        g_current_scene_update = main_menu_scene_update;
        g_current_scene_render = main_menu_scene_render;
        g_current_scene_shutdown = main_menu_scene_shutdown;
        break;
    case MAIN_GAME_SCENE:
        main_game_scene_init(user_data);
        g_current_scene_update = main_game_scene_update;
        g_current_scene_render = main_game_scene_render;
        g_current_scene_shutdown = main_game_scene_shutdown;
        break;
    default:
        g_current_scene_update = NULL;
        g_current_scene_render = NULL;
        break;
    }
}

void scene_update()
{
    if (g_current_scene_update != NULL)
    {
        g_current_scene_update();
    }
}

void scene_render()
{
    if (g_current_scene_render != NULL)
    {
        g_current_scene_render();
    }
}

void scene_shutdown()
{
    if (g_current_scene_shutdown != NULL)
    {
        g_current_scene_shutdown();
    }
}

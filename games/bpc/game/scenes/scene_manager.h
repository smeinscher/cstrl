#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "cstrl/cstrl_defines.h"
typedef CSTRL_PACKED_ENUM
{
    INIT_SCENE,
    MAIN_MENU_SCENE,
    MAIN_GAME_SCENE
} game_scene_t;

void scene_set(game_scene_t game_scene, void *user_data);

void scene_update();

void scene_render();

void scene_shutdown();

#endif // SCENE_MANAGER_H

#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_platform.h"

typedef CSTRL_PACKED_ENUM
{
    MAIN_MENU_SCENE,
    GAMEPLAY_SCENE
} game_scene_t;

void scene_set(game_scene_t game_scene, cstrl_platform_state *platform_state);

void scene_update(cstrl_platform_state *platform_state);

void scene_render(cstrl_platform_state *platform_state);

void scene_shutdown(cstrl_platform_state *platform_state);

#endif // SCENE_MANAGER_H

#ifndef SCENES_H
#define SCENES_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_platform.h"

typedef CSTRL_PACKED_ENUM
{
    QUICK_PLAY_GAME_MODE
} game_mode_t;

void scenes_init(cstrl_platform_state *platform_state);

void logo_scene_init(void *user_data);

void logo_scene_update();

void logo_scene_render();

void logo_scene_shutdown();

void main_menu_scene_init(void *user_data);

void main_menu_scene_update();

void main_menu_scene_render();

void main_menu_scene_shutdown();

void main_game_scene_init(void *user_data);

void main_game_scene_update();

void main_game_scene_render();

void main_game_scene_shutdown();

void scenes_clean();

#endif // SCENES_H

#ifndef SCENES_H
#define SCENES_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_platform.h"

typedef CSTRL_PACKED_ENUM
{
    PRACTICE_GAME_MODE,
    CASUAL_GAME_MODE,
    LEAGUE_GAME_MODE
} game_mode_t;

void scenes_init(cstrl_platform_state *platform_state);

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

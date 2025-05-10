#ifndef MAIN_MENU_SCENE_H
#define MAIN_MENU_SCENE_H

#include "cstrl/cstrl_platform.h"

void main_menu_scene_init(cstrl_platform_state *platform_state);

bool main_menu_scene_update(cstrl_platform_state *platform_state);

void main_menu_scene_render(cstrl_platform_state *platform_state);

void main_menu_scene_shutdown(cstrl_platform_state *platform_state);

#endif

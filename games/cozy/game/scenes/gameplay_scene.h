#ifndef GAMEPLAY_SCENE_H
#define GAMEPLAY_SCENE_H

#include "cstrl/cstrl_platform.h"

void gameplay_scene_init(cstrl_platform_state *platform_state);

void gameplay_scene_update(cstrl_platform_state *platform_state);

void gameplay_scene_render(cstrl_platform_state *platform_state);

void gameplay_scene_shutdown(cstrl_platform_state *platform_state);

#endif

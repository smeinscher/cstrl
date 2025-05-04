#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include <stdbool.h>

void gameplay_key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods);

void gameplay_mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos);

void gameplay_mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods);

bool gameplay_show_cursor();

void gameplay_get_mouse_position(int *mouse_x, int *mouse_y);

void gameplay_init(int base_screen_x, int base_screen_y, cstrl_shader *default_shader, bool simulation);

void gameplay_update();

void gameplay_render();

void gameplay_shutdown();

#endif // GAMEPLAY_H

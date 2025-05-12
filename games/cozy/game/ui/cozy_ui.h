#ifndef COZY_UI_H
#define COZY_UI_H

#include "cstrl/cstrl_platform.h"

void cozy_ui_init(cstrl_platform_state *platform_state);

void cozy_ui_render_main_menu(cstrl_platform_state *platform_state);

void cozy_ui_render_gameplay(cstrl_platform_state *platform_state, bool *pause);

void cozy_ui_shutdown();

#endif // COZY_UI_H

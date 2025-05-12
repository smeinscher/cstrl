#include "main_menu_scene.h"
#include "../ui/cozy_ui.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_ui.h"

static cstrl_ui_context g_ui_context;
static cstrl_ui_layout g_ui_base_layout;
static cstrl_ui_layout g_ui_button_layout;

static bool g_should_transition = false;

void main_menu_scene_init(cstrl_platform_state *platform_state)
{
    g_should_transition = false;
}

void main_menu_scene_update(cstrl_platform_state *platform_state)
{
}

void main_menu_scene_render(cstrl_platform_state *platform_state)
{
    cozy_ui_render_main_menu(platform_state);
}

void main_menu_scene_shutdown(cstrl_platform_state *platform_state)
{
}

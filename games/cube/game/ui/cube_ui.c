#include "cube_ui.h"
#include "cstrl/cstrl_ui.h"

static cstrl_ui_context g_ui_context;
static cstrl_ui_layout g_ui_base_layout;
static cstrl_ui_layout g_ui_button_layout;

void cube_ui_init(cstrl_platform_state *platform_state)
{
    cstrl_ui_init(&g_ui_context, platform_state);
    g_ui_base_layout = (cstrl_ui_layout){0};
    g_ui_base_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
    g_ui_button_layout = (cstrl_ui_layout){0};
    g_ui_button_layout.font_color = (cstrl_ui_color){0.97f, 0.97f, 0.96f, 1.0f};
    g_ui_button_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
}

void cube_ui_render_main_menu(cstrl_platform_state *platform_state)
{
    int width, height;
    cstrl_platform_get_window_size(platform_state, &width, &height);
    cstrl_ui_begin(&g_ui_context);
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, 0, height - 200, width, 200, GEN_ID(0), true, false, 1,
                                 &g_ui_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Play", 4, 240, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            // scene_set(GAMEPLAY_SCENE, platform_state);
        }
        if (cstrl_ui_button(&g_ui_context, "Options", 7, width / 2 - 120, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
        }
        if (cstrl_ui_button(&g_ui_context, "Quit", 4, width - 480, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            cstrl_platform_set_should_exit(true);
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    cstrl_ui_end(&g_ui_context);
}

void cube_ui_render_gameplay(cstrl_platform_state *platform_state, bool *pause)
{
}

void cube_ui_shutdown()
{
}

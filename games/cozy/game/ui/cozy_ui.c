#include "cozy_ui.h"
#include "../scenes/scene_manager.h"
#include "cstrl/cstrl_ui.h"

static cstrl_ui_context g_ui_context;
static cstrl_ui_layout g_ui_base_layout;
static cstrl_ui_layout g_ui_button_layout;
static cstrl_ui_layout g_ui_text_layout;
static cstrl_ui_layout g_ui_text_container_layout;

void cozy_ui_init(cstrl_platform_state *platform_state)
{
    cstrl_ui_init(&g_ui_context, platform_state);
    g_ui_base_layout = (cstrl_ui_layout){0};
    g_ui_base_layout.color = (cstrl_ui_color){0.36f, 0.45f, 0.17f, 1.0f};
    g_ui_base_layout.border.color = (cstrl_ui_color){0.18f, 0.28f, 0.12f, 1.0f};
    g_ui_base_layout.border.size = (cstrl_ui_border_size){2, 2, 2, 2, 0};
    g_ui_base_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
    g_ui_button_layout = (cstrl_ui_layout){0};
    g_ui_button_layout.color = (cstrl_ui_color){0.89f, 0.49f, 0.71f, 1.0f};
    g_ui_button_layout.border.color = (cstrl_ui_color){0.68f, 0.08f, 0.29f, 1.0f};
    g_ui_button_layout.border.size = (cstrl_ui_border_size){4, 4, 4, 4, 0};
    g_ui_button_layout.font_color = (cstrl_ui_color){0.68f, 0.08f, 0.29f, 1.0f};
    g_ui_button_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
    g_ui_text_layout = (cstrl_ui_layout){0};
    g_ui_text_layout.color = (cstrl_ui_color){0.89f, 0.49f, 0.71f, 1.0f};
    g_ui_text_layout.border.color = (cstrl_ui_color){0.68f, 0.08f, 0.29f, 1.0f};
    g_ui_text_layout.border.size = (cstrl_ui_border_size){4, 4, 4, 4, 0};
    g_ui_text_layout.font_color = (cstrl_ui_color){0.68f, 0.08f, 0.29f, 1.0f};
    g_ui_text_layout.child_alignment = CSTRL_UI_ALIGN_LEFT;
    g_ui_text_container_layout = (cstrl_ui_layout){0};
    g_ui_text_container_layout.child_alignment = CSTRL_UI_ALIGN_LEFT;
}

void cozy_ui_render_main_menu(cstrl_platform_state *platform_state)
{
    int width, height;
    cstrl_platform_get_window_size(platform_state, &width, &height);
    cstrl_ui_begin(&g_ui_context);
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, width / 2 - 200, height / 2 - 300, 400, 600, GEN_ID(0), true,
                                 false, 1, &g_ui_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Play", 4, 100, 100, 200, 100, GEN_ID(0), &g_ui_button_layout))
        {
            scene_set(GAMEPLAY_SCENE, platform_state);
        }
        if (cstrl_ui_button(&g_ui_context, "Options", 7, 100, 250, 200, 100, GEN_ID(0), &g_ui_button_layout))
        {
        }
        if (cstrl_ui_button(&g_ui_context, "Quit", 4, 100, 400, 200, 100, GEN_ID(0), &g_ui_button_layout))
        {
            cstrl_platform_set_should_exit(true);
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    cstrl_ui_end(&g_ui_context);
}

void cozy_ui_render_gameplay(cstrl_platform_state *platform_state, bool *pause)
{
    int width, height;
    cstrl_platform_get_window_size(platform_state, &width, &height);
    cstrl_ui_begin(&g_ui_context);
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, width / 2 - 200, height / 2 - 300, 400, 600, GEN_ID(0), true,
                                 false, 1, &g_ui_text_container_layout))
    {
        int text_height = cstrl_ui_text_height(&g_ui_context, "Your identity has been", 22, 1.0f);
        cstrl_ui_text(&g_ui_context, "Your identity has been", 22, 25, 25, 200, 100, GEN_ID(0), CSTRL_UI_ALIGN_CENTER,
                      &g_ui_text_layout);
        cstrl_ui_text(&g_ui_context, "compromised.", 12, 25, 25 + text_height, 200, 100, GEN_ID(0),
                      CSTRL_UI_ALIGN_CENTER, &g_ui_text_layout);
        cstrl_ui_text(&g_ui_context, "Kill. Everyone.", 15, 25, 45 + text_height * 2, 200, 100, GEN_ID(0),
                      CSTRL_UI_ALIGN_CENTER, &g_ui_text_layout);
        cstrl_ui_container_end(&g_ui_context);
    }
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, width / 2 - 200, height / 2 - 300, 400, 600, GEN_ID(0), true,
                                 false, 1, &g_ui_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Play", 4, 275, 525, 100, 50, GEN_ID(0), &g_ui_button_layout))
        {
            *pause = false;
        }
    }
    cstrl_ui_end(&g_ui_context);
}

void cozy_ui_shutdown()
{
    cstrl_ui_shutdown(&g_ui_context);
}

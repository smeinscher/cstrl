//
// Created by 12105 on 12/13/2024.
//

#include "vulkan_triangle.h"

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
}

int vulkan_triangle()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "cstrl vulkan triangle", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&platform_state, key_callback);

    if (!cstrl_renderer_init(&platform_state))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }
    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&platform_state);
    }

    cstrl_renderer_destroy(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

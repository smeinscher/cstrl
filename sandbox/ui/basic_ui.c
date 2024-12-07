//
// Created by sterling on 12/6/24.
//

#include "basic_ui.h"

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"

#include <stdio.h>

int basic_ui()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "cstrl ui test", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_renderer_init(&platform_state);
    cstrl_ui_context *context = cstrl_ui_init(&platform_state);
    while (!cstrl_platform_should_exit(&platform_state))
    {
        cstrl_platform_pump_messages(&platform_state);
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_ui_begin(context);
        if (cstrl_ui_button(context, 10, 10, 100, 50))
        {
            cstrl_ui_button(context, 200, 10, 100, 50);
        }
        cstrl_ui_end(context);
        cstrl_renderer_swap_buffers(&platform_state);
    }
    cstrl_ui_shutdown(context);
    cstrl_platform_shutdown(&platform_state);
    return 0;
}

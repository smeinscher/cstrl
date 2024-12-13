//
// Created by 12105 on 12/12/2024.
//

#include "basic_android_app.h"

#include "../extern/android/native_app_glue/android_native_app_glue.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"

void android_main(struct android_app *p_app)
{
    cstrl_platform_state state;
    if (!cstrl_platform_init(&state, "android app", 0, 0, 800, 600))
    {
        cstrl_platform_shutdown(&state);
        return;
    }

    cstrl_renderer_init(&state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f,  -0.5f, 0.0f, // right
        0.0f,  0.5f,  0.0f  // top
    };

    cstrl_renderer_add_positions(render_data, vertices, 2, 3);

    cstrl_shader shader = cstrl_load_shaders_from_files("../resources/shaders/android/default.vert",
                                                        "../resources/shaders/android/default.frag");

    // double previous_time = cstrl_platform_get_absolute_time();
    // double lag = 0.0;
    while (!cstrl_platform_should_exit(&state))
    {
        /*
        cstrl_platform_pump_messages(&state);
        double current_time = cstrl_platform_get_absolute_time();
        double elapsed_time = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed_time;
        while (lag >= 1.0 / 60.0)
        {
            lag -= 1.0 / 60.0;
        }
        */
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_renderer_draw(render_data);
        cstrl_renderer_swap_buffers(&state);
    }

    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_destroy(&state);
    cstrl_platform_shutdown(&state);
}
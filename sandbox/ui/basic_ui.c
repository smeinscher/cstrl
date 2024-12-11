//
// Created by sterling on 12/6/24.
//

#include "basic_ui.h"

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"

#include <stdio.h>
#include <string.h>

int basic_ui()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "cstrl ui test", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_renderer_init(&platform_state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float positions[] = {
        0.0f, 600.0f, 800.0f, 0.0f, 0.0f, 0.0f, 0.0f, 600.0f, 800.0f, 0.0f, 800.0f, 600.0f,
    };
    float uvs[] = {
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };
    float colors[24];
    for (int i = 0; i < 6; i++)
    {
        colors[i * 4] = 0.6f;
        colors[i * 4 + 1] = 0.6f;
        colors[i * 4 + 2] = 0.6f;
        colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(render_data, positions, 2, 6);
    cstrl_renderer_add_uvs(render_data, uvs);
    cstrl_renderer_add_colors(render_data, colors);

    cstrl_shader shader =
        cstrl_load_shaders_from_files("../resources/shaders/default.vert", "../resources/shaders/default.frag");
    cstrl_texture texture = cstrl_texture_generate_from_path("../resources/textures/background.jpg");
    cstrl_camera *camera = cstrl_camera_create(800, 600, true);
    cstrl_camera_update(camera, false, false, false, false, false, false, false, false);
    cstrl_set_uniform_mat4(shader.program, "view", camera->view);
    cstrl_set_uniform_mat4(shader.program, "projection", camera->projection);
    cstrl_ui_context *context = cstrl_ui_init(&platform_state);
    while (!cstrl_platform_should_exit(&platform_state))
    {
        cstrl_platform_pump_messages(&platform_state);
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_texture_bind(texture);
        cstrl_renderer_draw(render_data);
        cstrl_ui_begin(context);
        /*
        cstrl_ui_menu_bar(context);
        if (cstrl_ui_container(context, "debug", 5, 10, 10, 100, 300))
        {
            if (cstrl_ui_button(context, "start", 5, 10, 40, 60, 30))
            {
                cstrl_ui_button(context, "finish", 6, 200, 40, 60, 30);
            }
        }
        */
        if (cstrl_ui_container_begin(context, "Test", 4, 10, 10, 100, 300, GEN_ID(0)))
        {
            cstrl_ui_container_end(context);
        }
        cstrl_ui_end(context);
        cstrl_renderer_swap_buffers(&platform_state);
    }
    cstrl_ui_shutdown(context);
    cstrl_platform_shutdown(&platform_state);
    return 0;
}

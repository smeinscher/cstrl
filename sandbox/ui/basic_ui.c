//
// Created by sterling on 12/6/24.
//

#include "basic_ui.h"

#include "cstrl/cstrl_platform.h"
#define CSTRL_RENDER_API_OPENGL
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
}

int basic_ui()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "cstrl ui test", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&platform_state, key_callback);

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
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
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
        if (cstrl_ui_container_begin(context, "Menu", 4, 0, 0, 800, 30, GEN_ID(0), true, 1))
        {
            if (cstrl_ui_button(context, "Quit", 4, 760, 5, 50, 20, GEN_ID(0)))
            {
                cstrl_platform_set_should_exit(true);
            }
            cstrl_ui_container_end(context);
        }
        if (cstrl_ui_container_begin(context, "Test", 4, 10, 10, 200, 300, GEN_ID(0), false, 2))
        {
            char num_buffer[20];
            if (cstrl_ui_text_field(context, "PLACEHOLDER", 11, 10, 75, 180, 30, GEN_ID(0), num_buffer, 20))
            {
            }
            if (cstrl_ui_button(context, "Save", 4, 145, 270, 50, 30, GEN_ID(0)))
            {
                double value = atof(num_buffer);
                printf("%lf\n", value);
            }
            cstrl_ui_container_end(context);
        }
        if (cstrl_ui_container_begin(context, "Console", 7, 10, 390, 780, 200, GEN_ID(0), false, 1))
        {
            cstrl_ui_container_end(context);
        }
        cstrl_ui_end(context);
        cstrl_renderer_swap_buffers(&platform_state);
    }
    cstrl_camera_free(camera);
    cstrl_ui_shutdown(context);
    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_destroy(&platform_state);
    cstrl_platform_shutdown(&platform_state);
    return 0;
}

//
// Created by sterling on 12/6/24.
//

#include "basic_ui.h"

#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
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
    if (!cstrl_platform_init(&platform_state, "cstrl ui test", 560, 240, 1280, 720))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float positions[] = {
        0.0f, 720.0f, 1280.0f, 0.0f, 0.0f, 0.0f, 0.0f, 720.0f, 1280.0f, 0.0f, 1280.0f, 720.0f,
    };
    float uvs[] = {
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };
    float colors[24];
    for (int i = 0; i < 6; i++)
    {
        colors[i * 4] = 1.0f;
        colors[i * 4 + 1] = 1.0f;
        colors[i * 4 + 2] = 1.0f;
        colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(render_data, positions, 2, 6);
    cstrl_renderer_add_uvs(render_data, uvs);
    cstrl_renderer_add_colors(render_data, colors);

    cstrl_shader shader =
        cstrl_load_shaders_from_files("resources/shaders/default.vert", "resources/shaders/default.frag");
    cstrl_texture texture = cstrl_texture_generate_from_path("resources/textures/background.png");
    cstrl_camera *camera = cstrl_camera_create(1280, 720, true);
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(shader.program, "view", camera->view);
    cstrl_set_uniform_mat4(shader.program, "projection", camera->projection);
    cstrl_ui_context context;
    cstrl_ui_init(&context, &platform_state);
    cstrl_ui_layout menu_layout = {0};
    menu_layout.color = (cstrl_ui_color){0.6f, 0.6f, 0.6f, 1.0f};
    menu_layout.font_color = (cstrl_ui_color){0.0f, 0.0f, 0.0f, 0.0f};
    cstrl_ui_layout base_layout = {0};
    base_layout.border.size = (cstrl_ui_border_size){4, 4, 4, 4, 0};
    base_layout.border.color = (cstrl_ui_color){0.96f, 0.98f, 0.85f, 1.0f};
    base_layout.border.gap_color = (cstrl_ui_color){0.29f, 0.89f, 0.75f, 1.0f};
    base_layout.border.gap_size = 1;
    base_layout.color = (cstrl_ui_color){0.36f, 0.32f, 0.84f, 1.0f};
    base_layout.font_color = (cstrl_ui_color){0.12f, 0.1f, 0.13f, 1.0f};
    cstrl_ui_layout sub_layout = {0};
    sub_layout.border.size = (cstrl_ui_border_size){1, 1, 1, 1, 0};
    sub_layout.border.color = (cstrl_ui_color){0.12f, 0.1f, 0.13f, 1.0f};
    sub_layout.color = (cstrl_ui_color){0.29f, 0.89f, 0.75f, 1.0f};
    sub_layout.font_color = (cstrl_ui_color){0.12f, 0.1f, 0.13f, 1.0f};
    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&platform_state);
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_use_shader(shader);
        cstrl_texture_bind(texture);
        cstrl_renderer_draw(render_data);
        cstrl_ui_begin(&context);
        if (cstrl_ui_container_begin(&context, "", 0, 0, 0, 1280, 25, GEN_ID(0), true, false, 2, &menu_layout))
        {
            cstrl_ui_container_end(&context);
        }
        if (cstrl_ui_container_begin(&context, "Menu", 4, 10, 60, 200, 300, GEN_ID(0), false, false, 2, &base_layout))
        {
            if (cstrl_ui_subcontainer_begin(&context, "Sub-Menu", 8, 10, 50, 150, 50, GEN_ID(0), &sub_layout))
            {
                cstrl_ui_subcontainer_end(&context);
            }
            cstrl_ui_container_end(&context);
        }
        if (cstrl_ui_container_begin(&context, "Other", 5, 1070, 10, 200, 300, GEN_ID(0), false, false, 2,
                                     &base_layout))
        {
            if (cstrl_ui_button(&context, "Button", 6, 10, 50, 150, 50, GEN_ID(0), &sub_layout))
            {
                printf("Howdy!\n");
            }
            cstrl_ui_container_end(&context);
        }
        // if (cstrl_ui_container_begin(context, "Menu", 4, 0, 0, 800, 30, GEN_ID(0), true, true, 1))
        // {
        //     if (cstrl_ui_button(context, "Quit", 4, 760, 5, 50, 20, GEN_ID(0)))
        //     {
        //         cstrl_platform_set_should_exit(true);
        //     }
        //     cstrl_ui_container_end(context);
        // }
        // if (cstrl_ui_container_begin(context, "Test", 4, 10, 10, 200, 300, GEN_ID(0), false, false, 2))
        // {
        //     char num_buffer[20];
        //     if (cstrl_ui_text_field(context, "PLACEHOLDER", 11, 10, 75, 180, 30, GEN_ID(0), num_buffer, 20))
        //     {
        //     }
        //     if (cstrl_ui_button(context, "Save", 4, 145, 270, 50, 30, GEN_ID(0)))
        //     {
        //         double value = atof(num_buffer);
        //         printf("%lf\n", value);
        //     }
        //     cstrl_ui_container_end(context);
        // }
        // if (cstrl_ui_container_begin(context, "Console", 7, 10, 390, 780, 200, GEN_ID(0), false, false, 1))
        // {
        //     cstrl_ui_container_end(context);
        // }
        cstrl_ui_end(&context);
        cstrl_renderer_swap_buffers(&platform_state);
    }
    cstrl_camera_free(camera);
    cstrl_ui_shutdown(&context);
    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);
    return 0;
}

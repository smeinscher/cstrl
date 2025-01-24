#include "pong.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_ui.h"
#include <stdio.h>

static bool p1_moving_up = false;
static bool p1_moving_down = false;
static bool p2_moving_up = false;
static bool p2_moving_down = false;

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case (CSTRL_KEY_ESCAPE):
        if (action == CSTRL_ACTION_RELEASE)
        {
            cstrl_platform_set_should_exit(true);
        }
        break;
    case (CSTRL_KEY_W):
        if (action == CSTRL_ACTION_PRESS)
        {
            p1_moving_up = true;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            p1_moving_up = false;
        }
        break;
    case (CSTRL_KEY_S):
        if (action == CSTRL_ACTION_PRESS)
        {
            p1_moving_down = true;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            p1_moving_down = false;
        }
        break;
    }
}

int pong()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Pong", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    const float x00 = 5.0f;
    const float x01 = 15.0f;
    const float x10 = 785.0f;
    const float x11 = 795.0f;
    const float x20 = 400.0f - 5.0f;
    const float x21 = 400.0f + 5.0f;
    const float y0 = 300.0f - 20.0f;
    const float y1 = 300.0f + 20.0f;
    const float y20 = 300.0f - 5.0f;
    const float y21 = 300.0f + 5.0f;
    float positions[] = {x00, y1, x01, y0, x00, y0, x00, y1,  x01, y0,  x01, y1,  x10, y1,  x11, y0,  x10, y0,
                         x10, y1, x11, y0, x11, y1, x20, y21, x21, y20, x20, y20, x20, y21, x21, y20, x21, y21};
    float colors[72];
    for (int i = 0; i < 18; i++)
    {
        colors[i * 4] = 0.6f;
        colors[i * 4 + 1] = 0.6f;
        colors[i * 4 + 2] = 0.6f;
        colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(render_data, positions, 2, 18);
    cstrl_renderer_add_colors(render_data, colors);

    cstrl_shader shader = cstrl_load_shaders_from_files("resources/shaders/default_no_texture.vert",
                                                        "resources/shaders/default_no_texture.frag");

    cstrl_camera *camera = cstrl_camera_create(800, 600, true);
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(shader.program, "view", camera->view);
    cstrl_set_uniform_mat4(shader.program, "projection", camera->projection);
    vec2 ball_velocity = cstrl_vec2_normalize((vec2){-1.0f, 1.0f});
    float speed = 3.0f;
    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&platform_state);
        cstrl_renderer_clear(0.2f, 0.2f, 0.2f, 1.0f);
        if (positions[27] < 0.0f || positions[25] > 600.0f)
        {
            ball_velocity.y *= -1.0f;
        }
        if (positions[26] >= positions[12] && positions[25] > positions[15] && positions[27] < positions[13] ||
            positions[24] <= positions[2] && positions[25] > positions[3] && positions[27] < positions[1])
        {
            ball_velocity.x *= -1.0f;
            speed += 0.5f;
        }
        if (positions[24] <= 0.0f || positions[26] >= 800.0f)
        {
            speed = 3.0f;
            positions[24] = x20;
            positions[25] = y21;
            positions[26] = x21;
            positions[27] = y20;
            positions[28] = x20;
            positions[29] = y20;
            positions[30] = x20;
            positions[31] = y21;
            positions[32] = x21;
            positions[33] = y20;
            positions[34] = x21;
            positions[35] = y21;
        }
        for (int i = 0; i < 6; i++)
        {
            positions[12 * 2 + i * 2] += ball_velocity.x * speed;
            positions[12 * 2 + i * 2 + 1] += ball_velocity.y * speed;
        }
        cstrl_renderer_modify_positions(render_data, positions, 24, 12);
        if (positions[15] < positions[27] && positions[13] > 4.0f)
        {
            for (int i = 0; i < 6; i++)
            {
                positions[12 + i * 2 + 1] += 4.0f;
            }
            cstrl_renderer_modify_positions(render_data, positions, 12, 12);
        }
        if (positions[13] > positions[25] && positions[15] < 596.0f)
        {
            for (int i = 0; i < 6; i++)
            {
                positions[12 + i * 2 + 1] -= 4.0f;
            }
            cstrl_renderer_modify_positions(render_data, positions, 12, 12);
        }
        if (p1_moving_up && positions[3] > 0.0f)
        {
            for (int i = 0; i < 6; i++)
            {
                positions[i * 2 + 1] -= 4.0f;
            }
            cstrl_renderer_modify_positions(render_data, positions, 0, 12);
        }
        if (p1_moving_down && positions[1] < 600.0f)
        {
            for (int i = 0; i < 6; i++)
            {
                positions[i * 2 + 1] += 4.0f;
            }
            cstrl_renderer_modify_positions(render_data, positions, 0, 12);
        }
        cstrl_renderer_draw(render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_camera_free(camera);
    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

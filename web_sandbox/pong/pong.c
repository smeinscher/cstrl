#include "pong.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include <emscripten.h>
#include <emscripten/html5.h>

struct game_data
{
    float positions[36];
    cstrl_platform_state platform_state;
    cstrl_render_data *render_data;
    vec2 ball_velocity;
    float speed;
    bool p1_moving_up;
    bool p1_moving_down;
} game_data;

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
            game_data.p1_moving_up = true;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            game_data.p1_moving_up = false;
        }
        break;
    case (CSTRL_KEY_S):
        if (action == CSTRL_ACTION_PRESS)
        {
            game_data.p1_moving_down = true;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            game_data.p1_moving_down = false;
        }
        break;
    }
}

int pong_init()
{
    const float x00 = 5.0f;
    const float x01 = 15.0f;
    const float x10 = 785.0f;
    const float x11 = 795.0f;
    const float x20 = 400.0f - 5.0f;
    const float x21 = 400.0f + 5.0f;
    const float y_0 = 300.0f - 20.0f;
    const float y_1 = 300.0f + 20.0f;
    const float y20 = 300.0f - 5.0f;
    const float y21 = 300.0f + 5.0f;

    if (!cstrl_platform_init(&game_data.platform_state, "Pong", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&game_data.platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&game_data.platform_state, key_callback);

    cstrl_renderer_init(&game_data.platform_state);
    game_data.render_data = cstrl_renderer_create_render_data();
    float colors[72];
    for (int i = 0; i < 18; i++)
    {
        colors[i * 4] = 0.6f;
        colors[i * 4 + 1] = 0.6f;
        colors[i * 4 + 2] = 0.6f;
        colors[i * 4 + 3] = 1.0f;
    }
    float positions[] = {x00, y_1, x01, y_0, x00, y_0, x00, y_1, x01, y_0, x01, y_1, x10, y_1, x11, y_0, x10, y_0,
                         x10, y_1, x11, y_0, x11, y_1, x20, y21, x21, y20, x20, y20, x20, y21, x21, y20, x21, y21};
    for (int i = 0; i < 36; i++)
    {
        game_data.positions[i] = positions[i];
    }
    cstrl_renderer_add_positions(game_data.render_data, positions, 2, 18);
    cstrl_renderer_add_colors(game_data.render_data, colors);

    const char *vertex_shader = "#version 300 es\n\
        precision mediump float;\n\
        layout(location = 0) in vec2 a_pos;\n\
        layout(location = 2) in vec4 a_color;\n\
        out vec4 color;\n\
        uniform mat4 view;\n\
        uniform mat4 projection;\n\
        void main()\n\
        {\n\
            gl_Position = projection * view * vec4(a_pos, 0.0, 1.0);\n\
            color = a_color;\n\
        }";
    const char *fragment_shader = "#version 300 es\n\
        precision mediump float;\n\
        out vec4 frag_color;\n\
        in vec4 color;\n\
        void main()\n\
        {\n\
            frag_color = color;\n\
        }";
    cstrl_shader shader = cstrl_load_shaders_from_source(vertex_shader, fragment_shader);

    cstrl_camera *camera = cstrl_camera_create(800, 600, true);
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(shader.program, "view", camera->view);
    cstrl_set_uniform_mat4(shader.program, "projection", camera->projection);
    cstrl_camera_free(camera);
    cstrl_renderer_set_viewport(0, 0, camera->viewport.x, camera->viewport.y);
    return 0;
}

bool pong_run(double time, void *user_data)
{
    const float x00 = 5.0f;
    const float x01 = 15.0f;
    const float x10 = 785.0f;
    const float x11 = 795.0f;
    const float x20 = 400.0f - 5.0f;
    const float x21 = 400.0f + 5.0f;
    const float y_0 = 300.0f - 20.0f;
    const float y_1 = 300.0f + 20.0f;
    const float y20 = 300.0f - 5.0f;
    const float y21 = 300.0f + 5.0f;

    struct game_data *data = user_data;
    // cstrl_platform_pump_messages(&g_platform_state);
    cstrl_renderer_clear(0.2f, 0.2f, 0.2f, 1.0f);
    if (data->positions[27] < 0.0f || data->positions[25] > 600.0f)
    {
        data->ball_velocity.y *= -1.0f;
    }
    if (data->positions[26] >= data->positions[12] && data->positions[25] > data->positions[15] &&
            data->positions[27] < data->positions[13] ||
        data->positions[24] <= data->positions[2] && data->positions[25] > data->positions[3] &&
            data->positions[27] < data->positions[1])
    {
        data->ball_velocity.x *= -1.0f;
        data->speed += 0.5f;
    }
    if (data->positions[24] <= 0.0f || data->positions[26] >= 800.0f)
    {
        data->speed = 3.0f;
        data->positions[24] = x20;
        data->positions[25] = y21;
        data->positions[26] = x21;
        data->positions[27] = y20;
        data->positions[28] = x20;
        data->positions[29] = y20;
        data->positions[30] = x20;
        data->positions[31] = y21;
        data->positions[32] = x21;
        data->positions[33] = y20;
        data->positions[34] = x21;
        data->positions[35] = y21;
    }
    for (int i = 0; i < 6; i++)
    {
        data->positions[12 * 2 + i * 2] += data->ball_velocity.x * data->speed;
        data->positions[12 * 2 + i * 2 + 1] += data->ball_velocity.y * data->speed;
    }
    cstrl_renderer_modify_positions(data->render_data, data->positions, 24, 12);
    if (data->positions[15] < data->positions[27] && data->positions[13] > 4.0f)
    {
        for (int i = 0; i < 6; i++)
        {
            data->positions[12 + i * 2 + 1] += 4.0f;
        }
        cstrl_renderer_modify_positions(data->render_data, data->positions, 12, 12);
    }
    if (data->positions[13] > data->positions[25] && data->positions[15] < 596.0f)
    {
        for (int i = 0; i < 6; i++)
        {
            data->positions[12 + i * 2 + 1] -= 4.0f;
        }
        cstrl_renderer_modify_positions(data->render_data, data->positions, 12, 12);
    }
    if (data->p1_moving_up && data->positions[3] > 0.0f)
    {
        for (int i = 0; i < 6; i++)
        {
            data->positions[i * 2 + 1] -= 4.0f;
        }
        cstrl_renderer_modify_positions(data->render_data, data->positions, 0, 12);
    }
    if (data->p1_moving_down && data->positions[1] < 600.0f)
    {
        for (int i = 0; i < 6; i++)
        {
            data->positions[i * 2 + 1] += 4.0f;
        }
        cstrl_renderer_modify_positions(data->render_data, data->positions, 0, 12);
    }
    cstrl_renderer_draw(data->render_data);
    // cstrl_renderer_swap_buffers(&data->platform_state);
    return true;
}

int pong()
{
    int result = pong_init();
    if (result)
    {
        return result;
    }

    game_data.ball_velocity = cstrl_vec2_normalize((vec2){-1.0f, 1.0f});
    game_data.speed = 3.0f;

    emscripten_request_animation_frame_loop(pong_run, &game_data);

    // cstrl_renderer_free_render_data(game_data.render_data);
    // cstrl_renderer_shutdown(&game_data.platform_state);
    // cstrl_platform_shutdown(&game_data.platform_state);

    return 0;
}

//
// Created by 12105 on 12/1/2024.
//

#include "test_4_cubes_scene.h"

#include "../../../src/renderer/glsl_shader_programs.h"
#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_ui.h"

static cstrl_camera *g_main_camera;
static cstrl_camera_direction_mask g_movement;
static cstrl_camera_direction_mask g_rotation;

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case CSTRL_KEY_ESCAPE:
        if (action == CSTRL_ACTION_RELEASE)
        {
            cstrl_platform_set_should_exit(true);
        }
        break;
    case CSTRL_KEY_W:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_movement |= CSTRL_CAMERA_DIRECTION_UP;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_movement &= ~CSTRL_CAMERA_DIRECTION_UP;
        }
        break;
    case CSTRL_KEY_S:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_movement |= CSTRL_CAMERA_DIRECTION_DOWN;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_movement &= ~CSTRL_CAMERA_DIRECTION_DOWN;
        }
        break;
    case CSTRL_KEY_A:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_movement |= CSTRL_CAMERA_DIRECTION_LEFT;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_movement &= ~CSTRL_CAMERA_DIRECTION_LEFT;
        }
        break;
    case CSTRL_KEY_D:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_movement |= CSTRL_CAMERA_DIRECTION_RIGHT;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_movement &= ~CSTRL_CAMERA_DIRECTION_RIGHT;
        }
        break;
    case CSTRL_KEY_Z:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_rotation |= CSTRL_CAMERA_DIRECTION_UP;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_rotation &= ~CSTRL_CAMERA_DIRECTION_UP;
        }
        break;
    case CSTRL_KEY_X:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_rotation |= CSTRL_CAMERA_DIRECTION_DOWN;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_rotation &= ~CSTRL_CAMERA_DIRECTION_DOWN;
        }
        break;
    case CSTRL_KEY_Q:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_rotation |= CSTRL_CAMERA_DIRECTION_LEFT;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_rotation &= ~CSTRL_CAMERA_DIRECTION_LEFT;
        }
        break;
    case CSTRL_KEY_E:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_rotation |= CSTRL_CAMERA_DIRECTION_RIGHT;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_rotation &= ~CSTRL_CAMERA_DIRECTION_RIGHT;
        }
        break;
    case CSTRL_KEY_R:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->forward = (vec3){0.0f, 0.0f, -1.0f};
            g_main_camera->transform.position = (vec3){0.0f, 0.0f, 5.0f};
            g_main_camera->transform.rotation = (quat){1.0f, 0.0f, 0.0f, 0.0f};
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
        }
        break;
    case CSTRL_KEY_M:
        if (action == CSTRL_ACTION_PRESS)
        {
            cstrl_mouse_mode mode = cstrl_platform_get_mouse_mode(state);
            if (mode == CSTRL_MOUSE_DISABLED)
            {
                cstrl_platform_set_mouse_mode(state, CSTRL_MOUSE_NORMAL);
                cstrl_platform_set_show_cursor(state, true);
            }
            else
            {
                cstrl_platform_set_mouse_mode(state, CSTRL_MOUSE_DISABLED);
                cstrl_platform_set_show_cursor(state, false);
            }
        }
    default:
        break;
    }
}

static int last_x = 400;
static int last_y = 300;

static void mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    if (last_x == -1 || last_y == -1)
    {
        last_x = xpos;
        last_y = ypos;
        return;
    }
    float offset_x = ((float)xpos - (float)last_x) * 0.001f;
    float offset_y = ((float)ypos - (float)last_y) * 0.001f;

    if (cstrl_platform_get_mouse_mode(state) == CSTRL_MOUSE_NORMAL)
    {
        last_x = xpos;
        last_y = ypos;
    }
    else
    {
        cstrl_camera_rotate(g_main_camera, offset_x, offset_y);
        last_x = 400;
        last_y = 300;
    }
}

int test_4_cubes_scene()
{
    cstrl_platform_state state;
    if (!cstrl_platform_init(&state, "cstrl window test", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&state);
        return cstrl_test_failure;
    }

    cstrl_platform_set_key_callback(&state, key_callback);
    cstrl_platform_set_mouse_position_callback(&state, mouse_position_callback);

    cstrl_renderer_init(&state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float vertices_old[] = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f,  -0.5f, 0.0f, // right
        0.0f,  0.5f,  0.0f  // top
    };
    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
                        0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

                        -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

                        -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
                        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

                        0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
                        0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

                        -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
                        0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

                        -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};
    float uvs[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

                   0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

                   1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                   1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float vertices2[108];
    for (int i = 0; i < 36; i++)
    {
        vertices2[i * 3] = vertices[i * 3] + 5.0f;
        vertices2[i * 3 + 1] = vertices[i * 3 + 1];
        vertices2[i * 3 + 2] = vertices[i * 3 + 2] + 5.0f;
    }
    float vertices3[108];
    for (int i = 0; i < 36; i++)
    {
        vertices3[i * 3] = vertices[i * 3] - 5.0f;
        vertices3[i * 3 + 1] = vertices[i * 3 + 1];
        vertices3[i * 3 + 2] = vertices[i * 3 + 2] + 5.0f;
    }
    float vertices4[108];
    for (int i = 0; i < 36; i++)
    {
        vertices4[i * 3] = vertices[i * 3];
        vertices4[i * 3 + 1] = vertices[i * 3 + 1];
        vertices4[i * 3 + 2] = vertices[i * 3 + 2] + 10.0f;
    }
    float vertices_final[108 * 4];
    for (int i = 0; i < 108; i++)
    {
        vertices_final[i] = vertices[i];
    }
    for (int i = 108; i < 216; i++)
    {
        vertices_final[i] = vertices2[i - 108];
    }
    for (int i = 216; i < 324; i++)
    {
        vertices_final[i] = vertices3[i - 216];
    }
    for (int i = 324; i < 432; i++)
    {
        vertices_final[i] = vertices4[i - 324];
    }
    float uvs_final[288];
    for (int i = 0; i < 288; i++)
    {
        uvs_final[i] = uvs[i % 72];
    }
    cstrl_renderer_add_positions(render_data, vertices_final, 3, 144);
    cstrl_renderer_add_uvs(render_data, uvs_final);

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->transform.position = (vec3){0.0f, 0.0f, 5.0f};
    cstrl_shader shader = cstrl_load_shaders_from_source(basic_3d_vertex_shader, basic_3d_fragment_shader);
    cstrl_texture texture = cstrl_texture_generate_from_path("../resources/textures/wall.jpg");

    double previous_time = cstrl_platform_get_absolute_time();
    double lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&state);
        double current_time = cstrl_platform_get_absolute_time();
        double elapsed_time = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed_time;
        while (lag >= 1.0 / 60.0)
        {
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
            lag -= 1.0 / 60.0;
        }
        cstrl_renderer_clear(0.1f, 0.2f, 0.4f, 1.0f);
        cstrl_set_uniform_mat4(shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(shader.program, "projection", g_main_camera->projection);
        cstrl_texture_bind(texture);
        cstrl_renderer_draw(render_data);
        cstrl_renderer_swap_buffers(&state);
    }

    cstrl_camera_free(g_main_camera);
    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_destroy(&state);
    cstrl_platform_shutdown(&state);

    return cstrl_test_success;
}

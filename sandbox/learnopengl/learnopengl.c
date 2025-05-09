//
// Created by 12105 on 12/4/2024.
//

#include "learnopengl.h"

#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"

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
            g_main_camera->position = (vec3){0.0f, 0.0f, 5.0f};
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
        cstrl_camera_first_person_rotate(g_main_camera, offset_x, offset_y);
        last_x = 400;
        last_y = 300;
    }
}

int learnopengl()
{
    cstrl_platform_state state;
    if (!cstrl_platform_init(&state, "Learn OpenGL", 560, 240, 800, 600, false))
    {
        cstrl_platform_shutdown(&state);
        return 1;
    }
    cstrl_platform_set_key_callback(&state, key_callback);
    cstrl_platform_set_mouse_position_callback(&state, mouse_position_callback);
    cstrl_platform_set_show_cursor(&state, false);
    cstrl_platform_set_mouse_mode(&state, CSTRL_MOUSE_DISABLED);

    cstrl_renderer_init(&state);
    cstrl_render_data *color_render_data = cstrl_renderer_create_render_data();
    cstrl_render_data *light_render_data = cstrl_renderer_create_render_data();
    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f, -0.5f, -0.5f, 0.5f,
                        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f, 0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  -0.5f,
                        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,  0.5f,
                        0.5f,  0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,
                        -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};
    float uvs[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                   0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                   0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float normals[] = {0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f,
                       -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f,
                       0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f,
                       -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
                       0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f,
                       0.0f,  0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f,
                       0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f,
                       0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  0.0f};

    cstrl_renderer_add_positions(color_render_data, vertices, 3, 36);
    cstrl_renderer_add_uvs(color_render_data, uvs);
    cstrl_renderer_add_normals(color_render_data, normals);
    cstrl_renderer_add_positions(light_render_data, vertices, 3, 36);

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->position = (vec3){0.0f, 0.0f, 5.0f};
    cstrl_shader color_shader = cstrl_load_shaders_from_files("resources/shaders/learnopengl/colors.vert",
                                                              "resources/shaders/learnopengl/colors.frag");
    cstrl_shader light_shader = cstrl_load_shaders_from_files("resources/shaders/learnopengl/light_cube.vert",
                                                              "resources/shaders/learnopengl/light_cube.frag");
    cstrl_set_active_texture(0);
    cstrl_texture diffuse_texture =
        cstrl_texture_generate_from_path("resources/textures/container2.png", CSTRL_TEXTURE_FILTER_LINEAR);
    cstrl_texture_bind(diffuse_texture);
    cstrl_set_uniform_int(color_shader.program, "material.diffuse", 0);

    cstrl_set_active_texture(1);
    cstrl_texture specular_texture =
        cstrl_texture_generate_from_path("resources/textures/container2_specular.png", CSTRL_TEXTURE_FILTER_LINEAR);
    cstrl_texture_bind(specular_texture);
    cstrl_set_uniform_int(color_shader.program, "material.specular", 1);

    cstrl_set_uniform_3f(color_shader.program, "material.specular", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_float(color_shader.program, "material.shininess", 64.0);
    cstrl_set_uniform_3f(color_shader.program, "light.specular", 1.0f, 1.0f, 1.0f);
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

        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);

        // vec3 light_position = {sin(cstrl_platform_get_absolute_time()), cos(cstrl_platform_get_absolute_time()),
        // 0.0f};
        vec3 light_position = {1.2f, 1.0f, 1.0f};
        cstrl_set_uniform_mat4(color_shader.program, "model", cstrl_mat4_identity());
        cstrl_set_uniform_mat4(color_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(color_shader.program, "projection", g_main_camera->projection);
        cstrl_set_uniform_3f(color_shader.program, "view_position", g_main_camera->position.x,
                             g_main_camera->position.y, g_main_camera->position.z);
        cstrl_set_uniform_3f(color_shader.program, "light.position", light_position.x, light_position.y,
                             light_position.z);
        cstrl_set_uniform_3f(color_shader.program, "light.ambient", 0.2f, 0.2f, 0.2f);
        cstrl_set_uniform_3f(color_shader.program, "light.diffuse", 0.5f, 0.5f, 0.5f);
        cstrl_renderer_draw(color_render_data);

        mat4 model = cstrl_mat4_identity();
        model = cstrl_mat4_translate(model, light_position);
        model = cstrl_mat4_scale(model, (vec3){0.2f, 0.2f, 0.2f});
        cstrl_set_uniform_mat4(light_shader.program, "model", model);
        cstrl_set_uniform_mat4(light_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(light_shader.program, "projection", g_main_camera->projection);
        cstrl_renderer_draw(light_render_data);
        cstrl_renderer_swap_buffers(&state);
    }
    return 0;
}

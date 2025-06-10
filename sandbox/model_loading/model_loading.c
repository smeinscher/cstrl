#include "model_loading.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_model.h"
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

static int last_x = 640;
static int last_y = 360;

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
int model_loading()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Model Loading Test", 560, 240, 1280, 720, false))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&platform_state, key_callback);
    cstrl_platform_set_mouse_position_callback(&platform_state, mouse_position_callback);

    cstrl_renderer_init(&platform_state);

    // scene_t scene = cstrl_model_generate_scene_from_gltf_file("resources/models/Duck.gltf");
    mesh_t mesh = cstrl_model_generate_mesh_from_obj_file("resources/models/building.obj");

    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    cstrl_renderer_add_positions(render_data, mesh.positions, 3, mesh.vertex_count);
    cstrl_renderer_add_uvs(render_data, mesh.uvs);
    cstrl_renderer_add_colors(render_data, mesh.colors);
    cstrl_renderer_add_normals(render_data, mesh.normals);
    cstrl_renderer_add_indices(render_data, mesh.indices, mesh.indices_count);
    cstrl_shader shader;
    if (mesh.textures.id != 0)
    {
        shader = cstrl_load_shaders_from_files("resources/shaders/model_loading/default3D.vert",
                                               "resources/shaders/model_loading/default3D.frag");
        cstrl_texture_bind(mesh.textures);
    }
    else
    {
        shader = cstrl_load_shaders_from_files("resources/shaders/model_loading/default3D_no_texture.vert",
                                               "resources/shaders/model_loading/default3D_no_texture.frag");
    }
    g_main_camera = cstrl_camera_create(1280, 720, false);
    g_main_camera->position.z = 15.0f;
    g_main_camera->position.y = 15.0f;
    g_main_camera->forward.y = -1.0f;
    g_main_camera->forward.z = -1.0f;
    g_main_camera->forward = cstrl_vec3_normalize(g_main_camera->forward);
    g_main_camera->far = 1000.0f;
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);

    mat4 model = cstrl_mat4_identity();
    model = cstrl_mat4_scale(model, (vec3){0.1f, 0.1f, 0.1f});
    cstrl_set_uniform_mat4(shader.program, "model", model);

    cstrl_set_uniform_3f(shader.program, "light_pos", 5.0f, 5.0f, 5.0f);

    double previous_time = cstrl_platform_get_absolute_time();
    double lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&platform_state);
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
        cstrl_set_uniform_mat4(shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(shader.program, "projection", g_main_camera->projection);
        cstrl_set_uniform_3f(shader.program, "view_pos", g_main_camera->position.x, g_main_camera->position.y,
                             g_main_camera->position.z);
        cstrl_renderer_draw(render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

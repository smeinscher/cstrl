#include "main_menu_scene.h"
#include "../ui/cube_ui.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"

static int g_window_width;
static int g_window_height;

static cstrl_render_data *g_cube_render_data;
static cstrl_shader g_cube_shader;
static cstrl_camera *g_main_camera;

static mat4 g_cube_model;

void main_menu_scene_init(cstrl_platform_state *platform_state)
{
    cstrl_platform_get_window_size(platform_state, &g_window_width, &g_window_height);
    g_cube_render_data = cstrl_renderer_create_render_data();
    float cube_positions[] = {
        // back face
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, // 0-1
        0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,   // 1-2
        0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,   // 2-3
        -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, // 3-0

        // front face
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, // 4-5
        0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,   // 5-6
        0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,   // 6-7
        -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, // 7-4

        // sides
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, // 0-4
        0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,   // 1-5
        0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,     // 2-6
        -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f    // 3-7
    };
    cstrl_renderer_add_positions(g_cube_render_data, cube_positions, 3, 24);
    g_cube_shader =
        cstrl_load_shaders_from_files("resources/shaders/cube/main_cube.vert", "resources/shaders/cube/main_cube.frag");
    cstrl_use_shader(g_cube_shader);

    g_main_camera = cstrl_camera_create(g_window_width, g_window_height, false);
    g_main_camera->position = (vec3){0.0f, 0.0f, 4.0f};
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(g_cube_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_cube_shader.program, "projection", g_main_camera->projection);

    g_cube_model = cstrl_mat4_identity();
    g_cube_model = cstrl_mat4_rotate(g_cube_model, cstrl_pi_2, cstrl_vec3_normalize((vec3){1.0f, 1.0f, 1.0f}));
    cstrl_set_uniform_mat4(g_cube_shader.program, "model", g_cube_model);
    cube_ui_init(platform_state);
}

void main_menu_scene_update(cstrl_platform_state *platform_state)
{
}

void main_menu_scene_render(cstrl_platform_state *platform_state)
{
    g_cube_model = cstrl_mat4_rotate(g_cube_model, cstrl_pi_2 * cstrl_platform_get_absolute_time() * 0.00000001,
                                     cstrl_vec3_normalize((vec3){1.0f, 1.0f, 1.0f}));
    cstrl_set_uniform_mat4(g_cube_shader.program, "model", g_cube_model);
    cstrl_renderer_draw_lines(g_cube_render_data);
    cube_ui_render_main_menu(platform_state);
}

void main_menu_scene_shutdown(cstrl_platform_state *platform_state)
{
    cstrl_renderer_free_render_data(g_cube_render_data);
    cstrl_camera_free(g_main_camera);
}

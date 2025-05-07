#include "cookbook.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include <stdio.h>

static const int g_torus_side_count = 30;
static const int g_torus_ring_count = 30;
static const int g_torus_face_count = g_torus_side_count * g_torus_ring_count;
static const int g_torus_vertex_count = g_torus_side_count * (g_torus_ring_count + 1);

static const int g_plane_x_divs = 1;
static const int g_plane_z_divs = 1;
static const int g_plane_n_points = (g_plane_x_divs + 1) * (g_plane_z_divs + 1);

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

static void create_torus(float *positions, float *normals, float *uvs, int *indices, float outer_radius,
                         float inner_radius)
{
    float ring_factor = (cstrl_pi * 2.0f) / g_torus_ring_count;
    float side_factor = (cstrl_pi * 2.0f) / g_torus_side_count;
    int index = 0;
    int tindex = 0;
    for (int ring = 0; ring <= g_torus_ring_count; ring++)
    {
        float u = ring * ring_factor;
        float cu = cosf(u);
        float su = sinf(u);
        for (int side = 0; side < g_torus_side_count; side++)
        {
            float v = side * side_factor;
            float cv = cosf(v);
            float sv = sinf(v);
            float r = outer_radius + inner_radius * cv;
            positions[index] = r * cu;
            positions[index + 1] = r * su;
            positions[index + 2] = inner_radius * sv;
            normals[index] = cv * cu * r;
            normals[index + 1] = cv * su * r;
            normals[index + 2] = sv * r;
            float length = sqrtf(normals[index] * normals[index] + normals[index + 1] * normals[index + 1] +
                                 normals[index + 2] * normals[index + 2]);
            normals[index] /= length;
            normals[index + 1] /= length;
            normals[index + 2] /= length;
            index += 3;

            uvs[tindex] = u / (cstrl_pi * 2.0f);
            uvs[tindex + 1] = v / (cstrl_pi * 2.0f);
            tindex += 2;
        }
    }

    index = 0;
    for (int ring = 0; ring < g_torus_ring_count; ring++)
    {
        int ring_start = ring * g_torus_side_count;
        int next_ring_start = (ring + 1) * g_torus_side_count;
        for (int side = 0; side < g_torus_side_count; side++)
        {
            int next_side = (side + 1) % g_torus_side_count;
            indices[index] = ring_start + side;
            indices[index + 1] = next_ring_start + side;
            indices[index + 2] = next_ring_start + next_side;
            indices[index + 3] = ring_start + side;
            indices[index + 4] = next_ring_start + next_side;
            indices[index + 5] = ring_start + next_side;
            index += 6;
        }
    }
}

static void create_plane(float *positions, float *normals, float *uvs, int *indices, float x_size, float z_size,
                         float s_max, float t_max)
{
    float x2 = x_size / 2.0f;
    float z2 = z_size / 2.0f;
    float i_factor = z_size / (float)g_plane_z_divs;
    float j_factor = x_size / (float)g_plane_x_divs;
    float texi = s_max / (float)g_plane_x_divs;
    float texj = t_max / (float)g_plane_z_divs;
    float x, z;
    int vidx = 0;
    int tidx = 0;
    for (int i = 0; i <= g_plane_z_divs; i++)
    {
        z = i_factor * i - z2;
        for (int j = 0; j <= g_plane_x_divs; j++)
        {
            x = j_factor * j - x2;
            positions[vidx] = x;
            positions[vidx + 1] = 0.0f;
            positions[vidx + 2] = z;
            normals[vidx] = 0.0f;
            normals[vidx + 1] = 1.0f;
            normals[vidx + 2] = 0.0f;
            uvs[tidx] = j * texi;
            uvs[tidx + 1] = (g_plane_z_divs - i) * texj;

            vidx += 3;
            tidx += 2;
        }
    }

    int row_start;
    int next_row_start;
    int index = 0;
    for (int i = 0; i < g_plane_z_divs; i++)
    {
        row_start = i * (g_plane_x_divs + 1);
        next_row_start = (i + 1) * (g_plane_x_divs + 1);
        for (int j = 0; j < g_plane_x_divs; j++)
        {
            indices[index] = row_start + j;
            indices[index + 1] = next_row_start + j;
            indices[index + 2] = next_row_start + j + 1;
            indices[index + 3] = row_start + j;
            indices[index + 4] = next_row_start + j + 1;
            indices[index + 5] = row_start + j + 1;
            index += 6;
        }
    }
}

int cookbook()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Shading Language Cookbook", 560, 240, 800, 600, false))
    {
        printf("Failed to initialize platform\n");
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->position = (vec3){0.0f, 0.0f, 3.0f};

    cstrl_render_data *plane_render_data = cstrl_renderer_create_render_data();
    float plane_positions[3 * g_plane_n_points];
    float plane_normals[3 * g_plane_n_points];
    float plane_uvs[2 * g_plane_n_points];
    int plane_indices[6 * g_plane_x_divs * g_plane_z_divs];
    create_plane(plane_positions, plane_normals, plane_uvs, plane_indices, 50.0f, 50.0f, 1.0f, 1.0f);
    cstrl_renderer_add_positions(plane_render_data, plane_positions, 3, g_plane_n_points);
    cstrl_renderer_add_normals(plane_render_data, plane_normals);
    cstrl_renderer_add_uvs(plane_render_data, plane_uvs);
    cstrl_renderer_add_indices(plane_render_data, plane_indices, 6 * g_plane_x_divs * g_plane_z_divs);

    cstrl_shader plane_shader =
        cstrl_load_shaders_from_files("resources/shaders/shading_language_cookbook/spotlight.vert",
                                      "resources/shaders/shading_language_cookbook/spotlight.frag");

    cstrl_set_uniform_3f(plane_shader.program, "spotlight.position", 0.0f, 10.0f, 0.0f);
    cstrl_set_uniform_3f(plane_shader.program, "spotlight.direction", 0.0f, -1.0f, 0.0f);
    cstrl_set_uniform_3f(plane_shader.program, "spotlight.ambient", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(plane_shader.program, "spotlight.l", 0.9f, 0.9f, 0.9f);
    cstrl_set_uniform_float(plane_shader.program, "spotlight.exponent", 50.0f);
    cstrl_set_uniform_float(plane_shader.program, "spotlight.cutoff", 15.0f * cstrl_pi_180);

    cstrl_set_uniform_3f(plane_shader.program, "material.ambient", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(plane_shader.program, "material.diffuse", 0.9f, 0.9f, 0.9f);
    cstrl_set_uniform_3f(plane_shader.program, "material.specular", 0.95f, 0.95f, 0.95f);
    cstrl_set_uniform_float(plane_shader.program, "material.shininess", 100.0f);

    mat4 plane_model = cstrl_mat4_identity();
    plane_model = cstrl_mat4_translate(plane_model, (vec3){0.0f, -5.0f, 0.0f});

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
        cstrl_renderer_clear(0.4f, 0.2f, 0.6f, 1.0f);
        cstrl_set_uniform_mat4(plane_shader.program, "model", plane_model);
        cstrl_set_uniform_mat4(plane_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(plane_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(plane_shader);
        cstrl_renderer_draw_indices(plane_render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_free_render_data(plane_render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

int cookbook_chapter4_b()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Shading Language Cookbook", 560, 240, 800, 600, false))
    {
        printf("Failed to initialize platform\n");
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);

    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float positions[3 * g_torus_vertex_count];
    float normals[3 * g_torus_vertex_count];
    float uvs[2 * g_torus_vertex_count];
    int indices[6 * g_torus_face_count];
    create_torus(positions, normals, uvs, indices, 0.7f, 0.3f);
    cstrl_renderer_add_positions(render_data, positions, 3, g_torus_vertex_count);
    cstrl_renderer_add_normals(render_data, normals);
    cstrl_renderer_add_indices(render_data, indices, 6 * g_torus_face_count);

    cstrl_shader shader = cstrl_load_shaders_from_files("resources/shaders/shading_language_cookbook/blinnphong.vert",
                                                        "resources/shaders/shading_language_cookbook/blinnphong.frag");

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->position = (vec3){0.0f, 0.0f, 3.0f};

    cstrl_set_uniform_4f(shader.program, "light.position", 2.0f, 1.2f, 2.0f, 1.0f);
    cstrl_set_uniform_3f(shader.program, "light.ambient", 0.2f, 0.2f, 0.2f);
    cstrl_set_uniform_3f(shader.program, "light.l", 0.8f, 0.8f, 0.8f);

    cstrl_set_uniform_3f(shader.program, "material.ambient", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(shader.program, "material.diffuse", 0.4f, 0.4f, 0.4f);
    cstrl_set_uniform_3f(shader.program, "material.specular", 0.9f, 0.9f, 0.9f);
    cstrl_set_uniform_float(shader.program, "material.shininess", 180.0f);

    cstrl_render_data *plane_render_data = cstrl_renderer_create_render_data();
    float plane_positions[3 * g_plane_n_points];
    float plane_normals[3 * g_plane_n_points];
    float plane_uvs[2 * g_plane_n_points];
    int plane_indices[6 * g_plane_x_divs * g_plane_z_divs];
    create_plane(plane_positions, plane_normals, plane_uvs, plane_indices, 25.0f, 25.0f, 1.0f, 1.0f);
    cstrl_renderer_add_positions(plane_render_data, plane_positions, 3, g_plane_n_points);
    cstrl_renderer_add_normals(plane_render_data, plane_normals);
    cstrl_renderer_add_uvs(plane_render_data, plane_uvs);
    cstrl_renderer_add_indices(plane_render_data, plane_indices, 6 * g_plane_x_divs * g_plane_z_divs);

    cstrl_shader plane_shader =
        cstrl_load_shaders_from_files("resources/shaders/shading_language_cookbook/blinnphong.vert",
                                      "resources/shaders/shading_language_cookbook/blinnphong.frag");

    cstrl_set_uniform_4f(plane_shader.program, "light.position", 2.0f, 1.2f, 2.0f, 1.0f);
    cstrl_set_uniform_3f(plane_shader.program, "light.ambient", 0.2f, 0.2f, 0.2f);
    cstrl_set_uniform_3f(plane_shader.program, "light.l", 0.8f, 0.8f, 0.8f);

    cstrl_set_uniform_3f(plane_shader.program, "material.ambient", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(plane_shader.program, "material.diffuse", 0.1f, 0.1f, 0.1f);
    cstrl_set_uniform_3f(plane_shader.program, "material.specular", 0.9f, 0.9f, 0.9f);
    cstrl_set_uniform_float(plane_shader.program, "material.shininess", 180.0f);

    mat4 plane_model = cstrl_mat4_identity();
    plane_model = cstrl_mat4_translate(plane_model, (vec3){0.0f, -5.0f, 0.0f});

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
        cstrl_renderer_clear(0.4f, 0.2f, 0.6f, 1.0f);
        cstrl_set_uniform_mat4(shader.program, "model", cstrl_mat4_identity());
        cstrl_set_uniform_mat4(shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(shader);
        cstrl_renderer_draw_indices(render_data);
        cstrl_set_uniform_mat4(plane_shader.program, "model", plane_model);
        cstrl_set_uniform_mat4(plane_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(plane_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(plane_shader);
        cstrl_renderer_draw_indices(plane_render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}
int cookbook_chapter4_a()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Shading Language Cookbook", 560, 240, 800, 600, false))
    {
        printf("Failed to initialize platform\n");
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float positions[3 * g_torus_vertex_count];
    float normals[3 * g_torus_vertex_count];
    float uvs[2 * g_torus_vertex_count];
    int indices[6 * g_torus_face_count];
    create_torus(positions, normals, uvs, indices, 0.7f, 0.3f);
    cstrl_renderer_add_positions(render_data, positions, 3, g_torus_vertex_count);
    cstrl_renderer_add_normals(render_data, normals);
    cstrl_renderer_add_indices(render_data, indices, 6 * g_torus_face_count);

    cstrl_shader shader = cstrl_load_shaders_from_files("resources/shaders/shading_language_cookbook/multilight.vert",
                                                        "resources/shaders/shading_language_cookbook/multilight.frag");

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->position = (vec3){0.0f, 0.0f, 3.0f};

    float x, z;
    for (int i = 0; i < 5; i++)
    {
        char buffer[20];
        sprintf(buffer, "lights[%d].position", i);
        x = 2.0f * cosf((cstrl_pi * 2.0f / 5.0f) * i);
        z = 2.0f * sinf((cstrl_pi * 2.0f / 5.0f) * i);
        cstrl_set_uniform_4f(shader.program, buffer, x, 1.2f, z + 1.0f, 1.0f);
    }
    cstrl_set_uniform_3f(shader.program, "lights[0].ambient", 0.0f, 0.2f, 0.2f);
    cstrl_set_uniform_3f(shader.program, "lights[1].ambient", 0.0f, 0.0f, 0.2f);
    cstrl_set_uniform_3f(shader.program, "lights[2].ambient", 0.2f, 0.0f, 0.0f);
    cstrl_set_uniform_3f(shader.program, "lights[3].ambient", 0.0f, 0.2f, 0.0f);
    cstrl_set_uniform_3f(shader.program, "lights[4].ambient", 0.2f, 0.2f, 0.2f);

    cstrl_set_uniform_3f(shader.program, "lights[0].l", 0.0f, 0.8f, 0.8f);
    cstrl_set_uniform_3f(shader.program, "lights[1].l", 0.0f, 0.0f, 0.8f);
    cstrl_set_uniform_3f(shader.program, "lights[2].l", 0.8f, 0.0f, 0.0f);
    cstrl_set_uniform_3f(shader.program, "lights[3].l", 0.0f, 0.8f, 0.0f);
    cstrl_set_uniform_3f(shader.program, "lights[4].l", 0.8f, 0.8f, 0.8f);

    cstrl_set_uniform_3f(shader.program, "material.ambient", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(shader.program, "material.diffuse", 0.4f, 0.4f, 0.4f);
    cstrl_set_uniform_3f(shader.program, "material.specular", 0.9f, 0.9f, 0.9f);
    cstrl_set_uniform_float(shader.program, "material.shininess", 180.0f);

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
        cstrl_renderer_draw_indices(render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

int cookbook_chapter3()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Shading Language Cookbook", 560, 240, 800, 600, false))
    {
        printf("Failed to initialize platform\n");
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float positions[3 * g_torus_vertex_count];
    float normals[3 * g_torus_vertex_count];
    float uvs[2 * g_torus_vertex_count];
    int indices[6 * g_torus_face_count];
    create_torus(positions, normals, uvs, indices, 0.7f, 0.3f);
    cstrl_renderer_add_positions(render_data, positions, 3, g_torus_vertex_count);
    cstrl_renderer_add_normals(render_data, normals);
    cstrl_renderer_add_indices(render_data, indices, 6 * g_torus_face_count);

    cstrl_shader shader =
        cstrl_load_shaders_from_files("resources/shaders/shading_language_cookbook/phong_gouraud.vert",
                                      "resources/shaders/shading_language_cookbook/phong_gouraud.frag");

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->position = (vec3){0.0f, 0.0f, 3.0f};

    cstrl_set_uniform_4f(shader.program, "light.position", 5.0f, 5.0f, 2.0f, 1.0f);
    cstrl_set_uniform_3f(shader.program, "light.ambient", 0.4f, 0.4f, 0.4f);
    cstrl_set_uniform_3f(shader.program, "light.diffuse", 1.0f, 1.0f, 1.0f);
    cstrl_set_uniform_3f(shader.program, "light.specular", 1.0f, 1.0f, 1.0f);

    cstrl_set_uniform_3f(shader.program, "material.ambient", 0.9f, 0.5f, 0.3f);
    cstrl_set_uniform_3f(shader.program, "material.diffuse", 0.9f, 0.5f, 0.3f);
    cstrl_set_uniform_3f(shader.program, "material.specular", 0.8f, 0.8f, 0.8f);
    cstrl_set_uniform_float(shader.program, "material.shininess", 75.0f);

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
        cstrl_renderer_draw_indices(render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

int cookbook_chapter2()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, "Shading Language Cookbook", 560, 240, 800, 600, false))
    {
        printf("Failed to initialize platform\n");
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);

    cstrl_renderer_init(&platform_state);
    cstrl_render_data *render_data = cstrl_renderer_create_render_data();
    float positions[] = {
        -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
    };
    float uvs[] = {
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };
    cstrl_renderer_add_positions(render_data, positions, 2, 6);
    cstrl_renderer_add_uvs(render_data, uvs);

    cstrl_shader shader = cstrl_load_shaders_from_files("resources/shaders/shading_language_cookbook/ubo.vert",
                                                        "resources/shaders/shading_language_cookbook/ubo.frag");

    unsigned int ubo = cstrl_renderer_add_ubo(2 * sizeof(vec4) + 2 * sizeof(float));

    vec4 inner_color = {1.0f, 1.0f, 0.75f, 1.0f};
    vec4 outer_color = {0.0f, 0.0f, 0.0f, 0.0f};
    float inner_radius = 0.25f;
    float outer_radius = 0.45f;

    const char *names[] = {"inner_color", "outer_color", "radius_inner", "radius_outer"};
    int *offsets = cstrl_get_uniform_offsets(shader, names, 4);
    cstrl_renderer_update_ubo(ubo, inner_color.v, sizeof(vec4), offsets[0]);
    cstrl_renderer_update_ubo(ubo, outer_color.v, sizeof(vec4), offsets[1]);
    cstrl_renderer_update_ubo(ubo, &inner_radius, sizeof(float), offsets[2]);
    cstrl_renderer_update_ubo(ubo, &outer_radius, sizeof(float), offsets[3]);
    cstrl_free_uniform_offsets(&offsets);

    cstrl_set_uniform_block_binding(shader, "BlobSettings", 0);

    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&platform_state);
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_renderer_draw(render_data);
        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_renderer_free_render_data(render_data);
    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}

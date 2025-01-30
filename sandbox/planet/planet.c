#include "planet.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include "game/formation.h"
#include "game/paths.h"
#include "game/units.h"
#include <stdio.h>
#include <stdlib.h>

static cstrl_camera *g_main_camera;
static cstrl_camera_direction_mask g_movement;
static cstrl_camera_direction_mask g_rotation;
static cstrl_platform_state g_platform_state;

static int g_mouse_position_x = -1;
static int g_mouse_position_y = -1;
static int g_last_x = -1;
static int g_last_y = -1;
static float g_rotation_speed = 0.0004f;
static bool g_moving_planet = false;

typedef struct players_t
{
    size_t count;
    size_t capacity;
    float *score;
} players_t;

static units_t g_units;
static formations_t g_formations;
static int g_human_selected_formation;
static paths_t g_paths;
static players_t g_players;

static const vec3 UNIT_SIZE = {0.075f, 0.075f, 0.0f};
static vec3 g_planet_position = (vec3){0.0f, 0.0f, 0.0f};
static const vec3 PLANET_SIZE = (vec3){2.0f, 2.0f, 2.0f};

static vec3 mouse_click_ray_cast()
{
    int width, height;
    cstrl_platform_get_window_size(&g_platform_state, &width, &height);
    float x = (2.0f * g_mouse_position_x) / (float)width - 1.0f;
    float y = 1.0f - (2.0f * g_mouse_position_y) / (float)height;

    vec4 ray_clip = {x, y, -1.0f, 1.0f};
    vec4 ray_eye = cstrl_vec4_mult_mat4(ray_clip, cstrl_mat4_inverse(g_main_camera->projection));
    ray_eye = (vec4){ray_eye.x, ray_eye.y, -1.0f, 0.0f};

    vec4 ray_world = cstrl_vec4_mult_mat4(ray_eye, cstrl_mat4_transpose(cstrl_mat4_inverse(g_main_camera->view)));

    return cstrl_vec3_normalize((vec3){ray_world.x, ray_world.y, ray_world.z});
}

static vec3 get_point_on_path(vec3 start_position, vec3 end_position, float t)
{
    vec3 current = cstrl_vec3_normalize(cstrl_vec3_sub(start_position, g_planet_position));
    vec3 desired = cstrl_vec3_normalize(cstrl_vec3_sub(end_position, g_planet_position));
    float rotation_angle = acosf(cstrl_vec3_dot(current, desired));
    vec3 rotation_axis = cstrl_vec3_normalize(cstrl_vec3_cross(current, desired));
    quat rotation = cstrl_quat_angle_axis(rotation_angle * t, rotation_axis);
    float length = cstrl_vec3_length(cstrl_vec3_sub(end_position, g_planet_position));
    vec3 position = cstrl_vec3_mult_scalar(cstrl_vec3_rotate_by_quat(current, rotation), length);
    position = cstrl_vec3_add(position, g_planet_position);

    return position;
}

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
    case CSTRL_KEY_1:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){-3.0f, 0.0f, 0.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
        }
        break;
    case CSTRL_KEY_2:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){0.0f, 0.0f, -3.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
        }
        break;
    case CSTRL_KEY_3:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){3.0f, 0.0f, 0.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
        }
        break;
    case CSTRL_KEY_4:
    case CSTRL_KEY_R:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){0.0f, 0.0f, 3.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
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

static void mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    g_mouse_position_x = xpos;
    g_mouse_position_y = ypos;

    if (!g_moving_planet)
    {
        return;
    }
    if (g_last_x == -1 || g_last_y == -1)
    {
        g_last_x = xpos;
        g_last_y = ypos;
        return;
    }

    float y_angle_change = ((float)xpos - (float)g_last_x) * 0.004f;
    float z_angle_change = ((float)ypos - (float)g_last_y) * 0.004f;
    float dot_forward_up = cstrl_vec3_dot(g_main_camera->forward, g_main_camera->up);
    if (dot_forward_up < -0.8f)
    {
        z_angle_change = cstrl_min(z_angle_change, 0.0f);
    }
    if (dot_forward_up > 0.8f)
    {
        z_angle_change = cstrl_max(z_angle_change, 0.0f);
    }

    g_last_x = xpos;
    g_last_y = ypos;

    cstrl_camera_rotate_around_point(g_main_camera, g_planet_position, y_angle_change, z_angle_change);
}

static void mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
{
    if (button == CSTRL_MOUSE_BUTTON_LEFT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            g_moving_planet = true;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_moving_planet = false;
            g_last_x = -1;
            g_last_y = -1;
        }
    }
    if (button == CSTRL_MOUSE_BUTTON_RIGHT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_click_ray_cast();
            vec3 l = cstrl_vec3_sub(g_main_camera->position, g_planet_position);
            float b = cstrl_vec3_dot(d, l);
            float c = cstrl_vec3_dot(l, l) - powf((PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f, 2.0f);

            if (powf(b, 2.0) - c >= 0.0f)
            {
                float t = -b - sqrtf(powf(b, 2.0f) - c);
                vec3 end_position = cstrl_vec3_add(g_main_camera->position, cstrl_vec3_mult_scalar(d, t));
                end_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(end_position), 1.0f + UNIT_SIZE.x * 0.5f);
                vec3 start_position =
                    g_units.transforms[g_formations.unit_ids[g_human_selected_formation].array[0]].position;
                int new_id = paths_add(&g_paths, start_position, end_position);
                if (new_id != -1)
                {
                    formations_add_path(&g_formations, g_human_selected_formation, new_id);
                }
            }
        }
    }
}

// TODO: replace all instances of positions, size, and rotation with transform
static void get_points(vec3 *p0, vec3 *p1, vec3 *p2, vec3 *p3, vec3 position, vec3 size, quat rotation)
{
    float x0 = -0.5f;
    float x1 = 0.5f;
    float y0 = -0.5f;
    float y1 = 0.5f;
    float z = 1.0f + UNIT_SIZE.x / 2.0f;

    vec3 point0 = cstrl_vec3_mult((vec3){x0, y0, z}, size);
    vec3 point1 = cstrl_vec3_mult((vec3){x1, y0, z}, size);
    vec3 point2 = cstrl_vec3_mult((vec3){x1, y1, z}, size);
    vec3 point3 = cstrl_vec3_mult((vec3){x0, y1, z}, size);

    point0 = cstrl_vec3_rotate_by_quat(point0, rotation);
    point1 = cstrl_vec3_rotate_by_quat(point1, rotation);
    point2 = cstrl_vec3_rotate_by_quat(point2, rotation);
    point3 = cstrl_vec3_rotate_by_quat(point3, rotation);

    point0 = cstrl_vec3_add(point0, position);
    point1 = cstrl_vec3_add(point1, position);
    point2 = cstrl_vec3_add(point2, position);
    point3 = cstrl_vec3_add(point3, position);

    *p0 = point0;
    *p1 = point1;
    *p2 = point2;
    *p3 = point3;
}

static void update_billboard_position(da_float *positions, size_t index, vec3 new_position, vec3 size, quat rotation)
{
    vec3 point0, point1, point2, point3;
    get_points(&point0, &point1, &point2, &point3, new_position, size, rotation);
    positions->array[index * 12] = point0.x;
    positions->array[index * 12 + 1] = point0.y;
    positions->array[index * 12 + 2] = point0.z;
    positions->array[index * 12 + 3] = point1.x;
    positions->array[index * 12 + 4] = point1.y;
    positions->array[index * 12 + 5] = point1.z;
    positions->array[index * 12 + 6] = point2.x;
    positions->array[index * 12 + 7] = point2.y;
    positions->array[index * 12 + 8] = point2.z;
    positions->array[index * 12 + 9] = point3.x;
    positions->array[index * 12 + 10] = point3.y;
    positions->array[index * 12 + 11] = point3.z;
}

static void add_billboard_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors, vec3 position,
                                 vec3 size, quat rotation, vec4 color)
{
    vec3 point0, point1, point2, point3;
    get_points(&point0, &point1, &point2, &point3, position, size, rotation);

    cstrl_da_float_push_back(positions, point0.x);
    cstrl_da_float_push_back(positions, point0.y);
    cstrl_da_float_push_back(positions, point0.z);
    cstrl_da_float_push_back(positions, point1.x);
    cstrl_da_float_push_back(positions, point1.y);
    cstrl_da_float_push_back(positions, point1.z);
    cstrl_da_float_push_back(positions, point2.x);
    cstrl_da_float_push_back(positions, point2.y);
    cstrl_da_float_push_back(positions, point2.z);
    cstrl_da_float_push_back(positions, point3.x);
    cstrl_da_float_push_back(positions, point3.y);
    cstrl_da_float_push_back(positions, point3.z);

    cstrl_da_int_push_back(indices, 0);
    cstrl_da_int_push_back(indices, 1);
    cstrl_da_int_push_back(indices, 2);
    cstrl_da_int_push_back(indices, 0);
    cstrl_da_int_push_back(indices, 2);
    cstrl_da_int_push_back(indices, 3);

    cstrl_da_float_push_back(uvs, 0.0f);
    cstrl_da_float_push_back(uvs, 0.0f);
    cstrl_da_float_push_back(uvs, 1.0f);
    cstrl_da_float_push_back(uvs, 0.0f);
    cstrl_da_float_push_back(uvs, 1.0f);
    cstrl_da_float_push_back(uvs, 1.0f);
    cstrl_da_float_push_back(uvs, 0.0f);
    cstrl_da_float_push_back(uvs, 1.0f);

    for (int i = 0; i < 4; i++)
    {
        cstrl_da_float_push_back(colors, color.x);
        cstrl_da_float_push_back(colors, color.y);
        cstrl_da_float_push_back(colors, color.z);
        cstrl_da_float_push_back(colors, color.w);
    }
}

int planet()
{
    if (!cstrl_platform_init(&g_platform_state, "Planet", 560, 240, 800, 600))
    {
        cstrl_platform_shutdown(&g_platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&g_platform_state, key_callback);
    cstrl_platform_set_mouse_position_callback(&g_platform_state, mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(&g_platform_state, mouse_button_callback);
    cstrl_renderer_init(&g_platform_state);

    units_init(&g_units);
    formations_init(&g_formations);
    paths_init(&g_paths);

    cstrl_render_data *planet_render_data = cstrl_renderer_create_render_data();
    float planet_positions[19 * 37 * 3];
    float planet_normals[19 * 37 * 3];
    float planet_uvs[19 * 37 * 2];
    int planet_vertices_count = 0;
    int planet_uvs_count = 0;
    float latitude_step = cstrl_pi / 18.0f;
    float longitude_step = 2.0f * cstrl_pi / 36.0f;
    for (int i = 0; i <= 18; i++)
    {
        float latitude_angle = cstrl_pi / 2.0f - (float)i * latitude_step;
        float xy = cosf(latitude_angle);
        float z = sinf(latitude_angle);
        for (int j = 0; j <= 36; j++)
        {
            float longitude_angle = j * longitude_step;
            float x = xy * cosf(longitude_angle);
            float y = xy * sinf(longitude_angle);
            planet_normals[planet_vertices_count] = x;
            planet_positions[planet_vertices_count++] = x;
            planet_normals[planet_vertices_count] = y;
            planet_positions[planet_vertices_count++] = y;
            planet_normals[planet_vertices_count] = z;
            planet_positions[planet_vertices_count++] = z;

            float u = (float)j / 36.0f;
            float v = (float)i / 18.0f;
            planet_uvs[planet_uvs_count++] = u;
            planet_uvs[planet_uvs_count++] = v;
        }
    }
    int planet_indices[17 * 36 * 6];
    int count = 0;
    for (int i = 0; i < 18; i++)
    {
        int k1 = i * 37;
        int k2 = k1 + 37;
        for (int j = 0; j < 36; j++)
        {
            if (i != 0)
            {
                planet_indices[count++] = k1;
                planet_indices[count++] = k2;
                planet_indices[count++] = k1 + 1;
            }
            if (i != 17)
            {
                planet_indices[count++] = k1 + 1;
                planet_indices[count++] = k2;
                planet_indices[count++] = k2 + 1;
            }

            k1++;
            k2++;
        }
    }
    float planet_colors[18 * 36 * 4];
    for (int i = 0; i < 18 * 36; i++)
    {
        planet_colors[i * 4] = 0.4f;
        planet_colors[i * 4 + 1] = 0.2f;
        planet_colors[i * 4 + 2] = 0.8f;
        planet_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(planet_render_data, planet_positions, 3, planet_vertices_count / 3);
    cstrl_renderer_add_uvs(planet_render_data, planet_uvs);
    cstrl_renderer_add_colors(planet_render_data, planet_colors);
    cstrl_renderer_add_normals(planet_render_data, planet_normals);
    cstrl_renderer_add_indices(planet_render_data, planet_indices, count);

    cstrl_shader planet_shader = cstrl_load_shaders_from_files("resources/shaders/default3D_no_texture.vert",
                                                               "resources/shaders/default3D_no_texture.frag");

    cstrl_texture planet_texture = cstrl_texture_generate_from_path("resources/textures/moon1024.bmp");

    cstrl_render_data *unit_render_data = cstrl_renderer_create_render_data();
    da_float unit_positions;
    cstrl_da_float_init(&unit_positions, 12);
    da_int unit_indices;
    cstrl_da_int_init(&unit_indices, 6);
    da_float unit_uvs;
    cstrl_da_float_init(&unit_uvs, 8);
    da_float unit_colors;
    cstrl_da_float_init(&unit_colors, 16);
    add_billboard_object(&unit_positions, &unit_indices, &unit_uvs, &unit_colors, (vec3){0.0f, 0.0f, 1.0375f},
                         UNIT_SIZE, (quat){1.0f, 0.0f, 0.0f, 0.0f}, (vec4){0.0f, 1.0f, 0.0f, 1.0f});
    cstrl_renderer_add_positions(unit_render_data, unit_positions.array, 3, 4);
    cstrl_renderer_add_indices(unit_render_data, unit_indices.array, 6);
    cstrl_renderer_add_uvs(unit_render_data, unit_uvs.array);
    cstrl_renderer_add_colors(unit_render_data, unit_colors.array);

    cstrl_shader unit_shader =
        cstrl_load_shaders_from_files("resources/shaders/default3D.vert", "resources/shaders/default3D.frag");
    cstrl_texture unit_texture = cstrl_texture_generate_from_path("resources/textures/tank.png");

    g_main_camera = cstrl_camera_create(800, 600, false);
    g_main_camera->position.z = 3.0f;

    cstrl_set_uniform_3f(planet_shader.program, "material.ambient", 0.4f, 0.2f, 0.8f);
    cstrl_set_uniform_3f(planet_shader.program, "material.diffuse", 1.0f, 0.5f, 0.31f);
    cstrl_set_uniform_3f(planet_shader.program, "material.specular", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_float(planet_shader.program, "material.shininess", 8.0);
    cstrl_set_uniform_3f(planet_shader.program, "light.specular", 1.0f, 1.0f, 1.0f);

    double previous_time = cstrl_platform_get_absolute_time();
    double lag = 0.0;
    float light_start_x = 0.0f;
    float light_start_z = 0.0f;
    cstrl_ui_context *context = cstrl_ui_init(&g_platform_state);
    while (!cstrl_platform_should_exit())
    {
        cstrl_platform_pump_messages(&g_platform_state);
        double current_time = cstrl_platform_get_absolute_time();
        double elapsed_time = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed_time;
        while (lag >= 1.0 / 60.0)
        {
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
            paths_update(&g_paths);
            quat unit_rotation =
                cstrl_quat_inverse(cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view)));
            for (int i = 0; i < g_formations.unit_ids[g_human_selected_formation].size; i++)
            {
                int id = g_formations.unit_ids[g_human_selected_formation].array[i];
                int path_id = g_formations.path_ids[g_human_selected_formation].array[0];
                vec3 start_position = g_paths.start_positions[path_id];
                vec3 end_position = g_paths.end_positions[path_id];
                float t = g_paths.progress[path_id];
                g_units.transforms[id].position = get_point_on_path(start_position, end_position, t);
                update_billboard_position(&unit_positions, 0, g_units.transforms[id].position, UNIT_SIZE,
                                          unit_rotation);
            }

            cstrl_renderer_modify_positions(unit_render_data, unit_positions.array, 0, 12);
            lag -= 1.0 / 60.0;
            light_start_x += 0.001f;
            light_start_z += 0.001f;
        }
        cstrl_renderer_clear(0.1f, 0.0f, 0.1f, 1.0f);
        vec3 light_position = {5.0f * cosf(light_start_x), 1.0f, 5.0f * sinf(light_start_z)};
        cstrl_set_uniform_mat4(planet_shader.program, "model", cstrl_mat4_identity());
        cstrl_set_uniform_mat4(planet_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(planet_shader.program, "projection", g_main_camera->projection);
        cstrl_set_uniform_3f(planet_shader.program, "view_position", g_main_camera->position.x,
                             g_main_camera->position.y, g_main_camera->position.z);
        cstrl_set_uniform_3f(planet_shader.program, "light.position", light_position.x, light_position.y,
                             light_position.z);
        cstrl_set_uniform_3f(planet_shader.program, "light.ambient", 0.2f, 0.2f, 0.2f);
        cstrl_set_uniform_3f(planet_shader.program, "light.diffuse", 0.8f, 0.8f, 0.8f);
        cstrl_use_shader(planet_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(planet_texture);
        cstrl_renderer_draw_indices(planet_render_data);

        cstrl_set_uniform_mat4(unit_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(unit_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(unit_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(unit_texture);
        cstrl_renderer_draw_indices(unit_render_data);

        cstrl_ui_begin(context);
        if (cstrl_ui_container_begin(context, "Debug", 5, 10, 10, 200, 300, GEN_ID(0), false, 2))
        {
            char buffer[64];
            char title[64];
            vec3 euler = cstrl_quat_to_euler_angles(g_unit_rotation);
            sprintf(title, "%2.5f", g_unit_progress);
            if (cstrl_ui_text(context, title, 10, 0, 50, 300, 50, GEN_ID(0), CSTRL_UI_TEXT_ALIGN_LEFT))
            {
            }
            cstrl_ui_container_end(context);
        }
        cstrl_ui_end(context);
        cstrl_renderer_swap_buffers(&g_platform_state);
    }

    cstrl_camera_free(g_main_camera);
    cstrl_renderer_free_render_data(planet_render_data);
    cstrl_ui_shutdown(context);
    cstrl_renderer_shutdown(&g_platform_state);
    cstrl_platform_shutdown(&g_platform_state);

    return 0;
}

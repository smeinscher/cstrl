#include "planet.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include "game/ai.h"
#include "game/formation.h"
#include "game/paths.h"
#include "game/players.h"
#include "game/sphere.h"
#include "game/units.h"
#include "helpers/helpers.h"
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_WINDOW_WIDTH 1280
#define INITIAL_WINDOW_HEIGHT 720

#define PLANET_LATITUDE_POINTS 18
#define PLANET_LONGITUDE_POINTS 36

#define PLANET_LATITUDE_MAX_COS 0.98f

#define ACTIVE_PLAYERS 6

static int g_current_window_width = INITIAL_WINDOW_WIDTH;
static int g_current_window_height = INITIAL_WINDOW_HEIGHT;

static cstrl_camera *g_main_camera;
static cstrl_camera *g_ui_camera;
static cstrl_camera_direction_mask g_movement;
static cstrl_camera_direction_mask g_rotation;
static cstrl_platform_state g_platform_state;

static int g_mouse_position_x = -1;
static int g_mouse_position_y = -1;
static int g_last_x = -1;
static int g_last_y = -1;
static float g_rotation_speed = 0.0004f;
static bool g_moving_planet = false;

static players_t g_players;
static int g_human_player = 0;

static vec3 g_planet_position = (vec3){0.0f, 0.0f, 0.0f};

static const vec3 PLANET_SIZE = {2.0f, 2.0f, 2.0f};
static const vec3 PATH_MARKER_SIZE = {0.01f, 0.01f, 0.0f};

static const vec4 PATH_MARKER_COLOR = {0.8f, 0.8f, 0.8f, 0.9f};

static const vec4 UNIT_TEAM_COLORS[] = {
    (vec4){0.6f, 0.0f, 0.0f, 0.8f}, (vec4){0.0f, 0.2f, 0.6f, 0.8f}, (vec4){0.0f, 0.6f, 0.0f, 0.8f},
    (vec4){0.6f, 0.6f, 0.0f, 0.8f}, (vec4){0.8f, 0.8f, 0.8f, 0.8f}, (vec4){0.0f, 0.6f, 0.6f, 0.8f},
    (vec4){0.6f, 0.0f, 0.6f, 0.8f}, (vec4){0.3f, 0.3f, 0.3f, 0.8f},
};

static bool g_render_path_markers = true;
static bool g_render_path_lines = true;

static bool g_making_selection = false;
static vec2 g_selection_start;
static vec2 g_selection_end;

static bool g_border_update = false;

static vec3 position_from_ray_cast(vec3 d, float t)
{
    vec3 position = cstrl_vec3_add(g_main_camera->position, cstrl_vec3_mult_scalar(d, t));
    position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(position), 1.0f + UNIT_SIZE.x * 0.5f);
    return position;
}

static vec3 mouse_cursor_ray_cast()
{
    int width, height;
    cstrl_platform_get_window_size(&g_platform_state, &width, &height);
    return screen_ray_cast((vec2){g_mouse_position_x, g_mouse_position_y}, (vec2){width, height},
                           g_main_camera->projection, g_main_camera->view);
}

static void move_units_to_cursor_position(bool path_mode)
{
    if (g_players.selected_units[g_human_player].size == 0)
    {
        return;
    }
    vec3 d = mouse_cursor_ray_cast();
    float t;
    if (hit_check(d, &t, g_main_camera->position, g_planet_position, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
    {
        vec3 end_position = position_from_ray_cast(d, t);
        if (path_mode)
        {
            players_move_units_path_mode(&g_players, g_human_player, end_position);
        }
        else
        {
            players_move_units_normal_mode(&g_players, g_human_player, end_position);
        }
    }
}

static vec3 get_point_on_path(vec3 start_position, vec3 end_position, float t)
{
    vec3 current = cstrl_vec3_normalize(cstrl_vec3_sub(start_position, g_planet_position));
    vec3 desired = cstrl_vec3_normalize(cstrl_vec3_sub(end_position, g_planet_position));
    float rotation_angle = acosf(cstrl_vec3_dot(current, desired));
    vec3 rotation_axis = cstrl_vec3_normalize(cstrl_vec3_cross(current, desired));
    if (cstrl_vec3_is_equal(rotation_axis, (vec3){0.0f, 0.0f, 0.0f}))
    {
        rotation_axis = (vec3){1.0f, 0.0f, 0.0f};
    }
    quat rotation = cstrl_quat_angle_axis(rotation_angle * t, rotation_axis);
    float length = cstrl_vec3_length(cstrl_vec3_sub(end_position, g_planet_position));
    vec3 position = cstrl_vec3_mult_scalar(cstrl_vec3_rotate_by_quat(current, rotation), length);
    position = cstrl_vec3_add(position, g_planet_position);

    return position;
}

static void framebuffer_callback(cstrl_platform_state *state, int width, int height)
{
    g_main_camera->viewport = (vec2i){width, height};
    g_ui_camera->viewport = (vec2i){width, height};
    cstrl_renderer_set_viewport(0, 0, width, height);
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
    case CSTRL_KEY_1:
    case CSTRL_KEY_R:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){0.0f, 0.0f, 3.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
        }
        break;
    case CSTRL_KEY_2:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){-3.0f, 0.0f, 0.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
        }
        break;
    case CSTRL_KEY_3:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){0.0f, 0.0f, -3.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
        }
        break;
    case CSTRL_KEY_4:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){3.0f, 0.0f, 0.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
        }
        break;
    case CSTRL_KEY_5:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_render_path_markers = !g_render_path_markers;
            g_render_path_lines = g_render_path_markers;
        }
        break;
    case CSTRL_KEY_F:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (hit_check(d, &t, g_main_camera->position, g_planet_position, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                vec3 position = position_from_ray_cast(d, t);
                units_add(&g_players.units[g_human_player], position, CITY);
                g_border_update = true;
            }
        }
        break;
    case CSTRL_KEY_C:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (hit_check(d, &t, g_main_camera->position, g_planet_position, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                vec3 position = position_from_ray_cast(d, t);
                int unit_id = -1;
                int player_id = -1;
                for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                {
                    unit_id = units_hit(&g_players.units[i], position);
                    if (unit_id != -1)
                    {
                        player_id = i;
                        break;
                    }
                }
                if (unit_id != -1)
                {
                    vec3 old_position = g_players.units[player_id].position[unit_id];
                    int old_type = g_players.units[player_id].type[unit_id];
                    units_remove(&g_players.units[player_id], unit_id);
                    player_id++;
                    if (player_id >= ACTIVE_PLAYERS)
                    {
                        player_id = 0;
                    }
                    units_add(&g_players.units[player_id], old_position, old_type);
                }
            }
        }
        break;
    case CSTRL_KEY_T:
        if (action == CSTRL_ACTION_PRESS)
        {
            players_set_units_in_formation_selected(&g_players, g_human_player);
        }
        break;
    case CSTRL_KEY_B:
        if (action == CSTRL_ACTION_PRESS)
        {
            players_add_selected_units_to_formation(&g_players, g_human_player);
        }
        break;
    default:
        break;
    }
}

static void mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    g_mouse_position_x = xpos;
    g_mouse_position_y = ypos;

    if (g_making_selection)
    {
        g_selection_end = (vec2){g_mouse_position_x, g_mouse_position_y};
    }
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
    if (dot_forward_up < -PLANET_LATITUDE_MAX_COS)
    {
        z_angle_change = cstrl_min(z_angle_change, 0.0f);
    }
    if (dot_forward_up > PLANET_LATITUDE_MAX_COS)
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
            if (mods & CSTRL_KEY_MOD_CONTROL)
            {
                g_making_selection = true;
                g_selection_start = (vec2){g_mouse_position_x, g_mouse_position_y};
                g_selection_end = g_selection_start;
                g_players.selected_formation[g_human_player] = -1;
                cstrl_da_int_clear(&g_players.selected_units[g_human_player]);
            }
            else
            {
                vec2 mouse_position = {g_mouse_position_x, g_mouse_position_y};
                players_select_units(&g_players, g_human_player, g_main_camera->viewport.x, g_main_camera->viewport.y,
                                     mouse_position, mouse_position, g_main_camera);
            }
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            if (g_making_selection)
            {
                players_select_units(&g_players, g_human_player, g_main_camera->viewport.x, g_main_camera->viewport.y,
                                     g_selection_start, g_selection_end, g_main_camera);
                g_making_selection = false;
            }
        }
    }
    if (button == CSTRL_MOUSE_BUTTON_RIGHT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            move_units_to_cursor_position(mods & CSTRL_KEY_MOD_CONTROL);
        }
    }
    if (button == CSTRL_MOUSE_BUTTON_MIDDLE)
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
}

static void mouse_wheel_callback(cstrl_platform_state *state, int delta_x, int delta_y, int keys_down)
{
    g_main_camera->fov -= (float)delta_y / 6000.0f;
    if (g_main_camera->fov <= 10.0f * cstrl_pi_180)
    {
        g_main_camera->fov = 10.0f * cstrl_pi_180;
    }
    else if (g_main_camera->fov >= 60.0f * cstrl_pi_180)
    {
        g_main_camera->fov = 60.0f * cstrl_pi_180;
    }
}

static void add_billboard_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors,
                                 transform transform, vec4 uv_positions, vec4 color)
{
    vec3 point0, point1, point2, point3;
    get_points(&point0, &point1, &point2, &point3, transform);

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

    unsigned int index = indices->size / 6;
    cstrl_da_int_push_back(indices, index * 4);
    cstrl_da_int_push_back(indices, index * 4 + 1);
    cstrl_da_int_push_back(indices, index * 4 + 2);
    cstrl_da_int_push_back(indices, index * 4);
    cstrl_da_int_push_back(indices, index * 4 + 2);
    cstrl_da_int_push_back(indices, index * 4 + 3);

    if (uvs != NULL)
    {
        cstrl_da_float_push_back(uvs, uv_positions.u0);
        cstrl_da_float_push_back(uvs, uv_positions.v0);
        cstrl_da_float_push_back(uvs, uv_positions.u1);
        cstrl_da_float_push_back(uvs, uv_positions.v0);
        cstrl_da_float_push_back(uvs, uv_positions.u1);
        cstrl_da_float_push_back(uvs, uv_positions.v1);
        cstrl_da_float_push_back(uvs, uv_positions.u0);
        cstrl_da_float_push_back(uvs, uv_positions.v1);
    }

    for (int i = 0; i < 4; i++)
    {
        cstrl_da_float_push_back(colors, color.r);
        cstrl_da_float_push_back(colors, color.g);
        cstrl_da_float_push_back(colors, color.b);
        cstrl_da_float_push_back(colors, color.a);
    }
}

static void update_billboard_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors, size_t index,
                                    transform transform, vec4 new_uv_positions, vec4 new_color)
{
    if ((index + 1) * 12 <= positions->size)
    {
        vec3 point0, point1, point2, point3;
        get_points(&point0, &point1, &point2, &point3, transform);
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

        if (uvs != NULL)
        {
            uvs->array[index * 8] = new_uv_positions.u0;
            uvs->array[index * 8 + 1] = new_uv_positions.v0;
            uvs->array[index * 8 + 2] = new_uv_positions.u1;
            uvs->array[index * 8 + 3] = new_uv_positions.v0;
            uvs->array[index * 8 + 4] = new_uv_positions.u1;
            uvs->array[index * 8 + 5] = new_uv_positions.v1;
            uvs->array[index * 8 + 6] = new_uv_positions.u0;
            uvs->array[index * 8 + 7] = new_uv_positions.v1;
        }

        colors->array[index * 16] = new_color.r;
        colors->array[index * 16 + 1] = new_color.g;
        colors->array[index * 16 + 2] = new_color.b;
        colors->array[index * 16 + 3] = new_color.a;
        colors->array[index * 16 + 4] = new_color.r;
        colors->array[index * 16 + 5] = new_color.g;
        colors->array[index * 16 + 6] = new_color.b;
        colors->array[index * 16 + 7] = new_color.a;
        colors->array[index * 16 + 8] = new_color.r;
        colors->array[index * 16 + 9] = new_color.g;
        colors->array[index * 16 + 10] = new_color.b;
        colors->array[index * 16 + 11] = new_color.a;
        colors->array[index * 16 + 12] = new_color.r;
        colors->array[index * 16 + 13] = new_color.g;
        colors->array[index * 16 + 14] = new_color.b;
        colors->array[index * 16 + 15] = new_color.a;
    }
    else
    {
        add_billboard_object(positions, indices, uvs, colors, transform, new_uv_positions, new_color);
    }
}

static vec3 adjust_billboard_scale(vec3 original_scale)
{
    vec3 modified_scale = cstrl_vec3_mult_scalar(original_scale, g_main_camera->fov / (45.0f * cstrl_pi_180));
    if (modified_scale.x >= original_scale.x)
    {
        modified_scale = original_scale;
    }
    return modified_scale;
}

static void add_ui_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors, vec2 position_start,
                          vec2 position_end, vec4 color)
{
    cstrl_da_float_push_back(positions, position_start.x);
    cstrl_da_float_push_back(positions, position_start.y);
    cstrl_da_float_push_back(positions, position_end.x);
    cstrl_da_float_push_back(positions, position_start.y);
    cstrl_da_float_push_back(positions, position_end.x);
    cstrl_da_float_push_back(positions, position_end.y);
    cstrl_da_float_push_back(positions, position_start.x);
    cstrl_da_float_push_back(positions, position_end.y);

    unsigned int index = indices->size / 6;
    cstrl_da_int_push_back(indices, index * 4);
    cstrl_da_int_push_back(indices, index * 4 + 1);
    cstrl_da_int_push_back(indices, index * 4 + 2);
    cstrl_da_int_push_back(indices, index * 4);
    cstrl_da_int_push_back(indices, index * 4 + 2);
    cstrl_da_int_push_back(indices, index * 4 + 3);

    if (uvs != NULL)
    {
        cstrl_da_float_push_back(uvs, 0.0f);
        cstrl_da_float_push_back(uvs, 0.0f);
        cstrl_da_float_push_back(uvs, 1.0f);
        cstrl_da_float_push_back(uvs, 0.0f);
        cstrl_da_float_push_back(uvs, 1.0f);
        cstrl_da_float_push_back(uvs, 1.0f);
        cstrl_da_float_push_back(uvs, 0.0f);
        cstrl_da_float_push_back(uvs, 1.0f);
    }

    for (int i = 0; i < 4; i++)
    {
        cstrl_da_float_push_back(colors, color.r);
        cstrl_da_float_push_back(colors, color.g);
        cstrl_da_float_push_back(colors, color.b);
        cstrl_da_float_push_back(colors, color.a);
    }
}

static void update_ui_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors, size_t index,
                             vec2 new_position_start, vec2 new_position_end, vec4 new_color)
{
    if ((index + 1) * 8 <= positions->size)
    {
        positions->array[index * 8] = new_position_start.x;
        positions->array[index * 8 + 1] = new_position_start.y;
        positions->array[index * 8 + 2] = new_position_end.x;
        positions->array[index * 8 + 3] = new_position_start.y;
        positions->array[index * 8 + 4] = new_position_end.x;
        positions->array[index * 8 + 5] = new_position_end.y;
        positions->array[index * 8 + 6] = new_position_start.x;
        positions->array[index * 8 + 7] = new_position_end.y;

        colors->array[index * 16] = new_color.r;
        colors->array[index * 16 + 1] = new_color.g;
        colors->array[index * 16 + 2] = new_color.b;
        colors->array[index * 16 + 3] = new_color.a;
        colors->array[index * 16 + 4] = new_color.r;
        colors->array[index * 16 + 5] = new_color.g;
        colors->array[index * 16 + 6] = new_color.b;
        colors->array[index * 16 + 7] = new_color.a;
        colors->array[index * 16 + 8] = new_color.r;
        colors->array[index * 16 + 9] = new_color.g;
        colors->array[index * 16 + 10] = new_color.b;
        colors->array[index * 16 + 11] = new_color.a;
        colors->array[index * 16 + 12] = new_color.r;
        colors->array[index * 16 + 13] = new_color.g;
        colors->array[index * 16 + 14] = new_color.b;
        colors->array[index * 16 + 15] = new_color.a;
    }
    else
    {
        add_ui_object(positions, indices, uvs, colors, new_position_start, new_position_end, new_color);
    }
}

bool inside_polygon(da_float *polygon, vec3 point)
{
    int count = 0;
    for (int i = 0; i < polygon->size / 3 - 1; i++)
    {
        vec3 polygon_point0 = (vec3){polygon->array[i * 3], polygon->array[i * 3 + 1], polygon->array[i * 3 + 2]};
        vec3 polygon_point1 = (vec3){polygon->array[i * 3 + 3], polygon->array[i * 3 + 4], polygon->array[i * 3 + 5]};
        if (cstrl_vec3_is_equal(polygon_point1, (vec3){0.0f, 0.0f, 0.0f}))
        {
            i++;
            continue;
        }
        if (point.y < polygon_point0.y != point.y < polygon_point1.y &&
            point.x < polygon_point0.x + ((point.y - polygon_point0.y) / (polygon_point1.y - polygon_point0.y)) *
                                             (polygon_point1.x - polygon_point0.x))
        {
            count++;
        }
    }
    return count % 2 == 1;
}

void rectangle_intersection_points(da_float *result, da_float *rect0, da_float *rect1)
{
    vec3 p0 = {rect0->array[0], rect0->array[1], rect0->array[2]};
    vec3 p1 = {rect0->array[3], rect0->array[4], rect0->array[5]};
    vec3 p2 = {rect1->array[0], rect1->array[1], rect1->array[2]};
    vec3 p3 = {rect1->array[3], rect1->array[4], rect1->array[5]};

    vec3 result0 = {cstrl_max(p0.x, p2.x), cstrl_max(p0.y, p2.y), cstrl_max(p0.z, p2.z)};
    vec3 result1 = {cstrl_min(p1.x, p3.x), cstrl_min(p1.y, p3.y), cstrl_min(p1.z, p3.z)};

    cstrl_da_float_push_back(result, result0.x);
    cstrl_da_float_push_back(result, result0.y);
    cstrl_da_float_push_back(result, result0.z);
    cstrl_da_float_push_back(result, result1.x);
    cstrl_da_float_push_back(result, result1.y);
    cstrl_da_float_push_back(result, result1.z);
}

void recalculate_border(da_float *positions, da_int *indices, da_float *colors, vec4 color, units_t *units)
{
    size_t initial_size = positions->size;
    da_float border_points;
    cstrl_da_float_init(&border_points, 12);
    for (int i = 0; i < units->count; i++)
    {
        if (units->type[i] != CITY)
        {
            continue;
        }
        vec3 origin_point = units->position[i];
        vec3 p0 = {0.0f, UNIT_SIZE.x * 4.0f, 0.0f};
        vec3 p1 = {-UNIT_SIZE.x * 4.0f, 0.0f, 0.0f};
        vec3 p2 = {0.0f, -UNIT_SIZE.x * 4.0f, 0.0f};
        vec3 p3 = {UNIT_SIZE.x * 4.0f, 0.0f, 0.0f};
        float angle = acosf(cstrl_vec3_dot((vec3){0.0f, 0.0f, 1.0f}, cstrl_vec3_normalize(origin_point)));
        vec3 axis = cstrl_vec3_cross((vec3){0.0f, 0.0f, 1.0f}, origin_point);
        if (!cstrl_vec3_is_equal(axis, (vec3){0.0f, 0.0f, 0.0f}))
        {
            p0 = cstrl_vec3_rotate_along_axis(p0, angle, axis);
            p1 = cstrl_vec3_rotate_along_axis(p1, angle, axis);
            p2 = cstrl_vec3_rotate_along_axis(p2, angle, axis);
            p3 = cstrl_vec3_rotate_along_axis(p3, angle, axis);
        }
        p0 = cstrl_vec3_add(origin_point, p0);
        p1 = cstrl_vec3_add(origin_point, p1);
        p2 = cstrl_vec3_add(origin_point, p2);
        p3 = cstrl_vec3_add(origin_point, p3);

        p0 = cstrl_vec3_normalize(p0);
        p1 = cstrl_vec3_normalize(p1);
        p2 = cstrl_vec3_normalize(p2);
        p3 = cstrl_vec3_normalize(p3);

        da_float tmp_points;
        cstrl_da_float_init(&tmp_points, 12);
        cstrl_da_float_push_back(&tmp_points, p0.x);
        cstrl_da_float_push_back(&tmp_points, p0.y);
        cstrl_da_float_push_back(&tmp_points, p0.z);
        cstrl_da_float_push_back(&tmp_points, p1.x);
        cstrl_da_float_push_back(&tmp_points, p1.y);
        cstrl_da_float_push_back(&tmp_points, p1.z);
        cstrl_da_float_push_back(&tmp_points, p2.x);
        cstrl_da_float_push_back(&tmp_points, p2.y);
        cstrl_da_float_push_back(&tmp_points, p2.z);
        cstrl_da_float_push_back(&tmp_points, p3.x);
        cstrl_da_float_push_back(&tmp_points, p3.y);
        cstrl_da_float_push_back(&tmp_points, p3.z);
        for (int i = 0; i < tmp_points.size; i++)
        {
            cstrl_da_float_push_back(&border_points, tmp_points.array[i]);
        }
        if (tmp_points.size == 12)
        {
            cstrl_da_float_push_back(&border_points, 0.0f);
            cstrl_da_float_push_back(&border_points, 0.0f);
            cstrl_da_float_push_back(&border_points, 0.0f);
        }
        cstrl_da_float_free(&tmp_points);
    }
    vec3 first_point = {border_points.array[0], border_points.array[1], border_points.array[2]};
    for (int i = 0; i < border_points.size / 3 - 1; i++)
    {
        vec3 p0 = {border_points.array[i * 3], border_points.array[i * 3 + 1], border_points.array[i * 3 + 2]};
        vec3 p1 = {border_points.array[i * 3 + 3], border_points.array[i * 3 + 4], border_points.array[i * 3 + 5]};
        if (cstrl_vec3_is_equal(p1, (vec3){0.0f, 0.0f, 0.0f}))
        {
            generate_line_segments(positions, p0, first_point, 0.05f);
            i++;
            first_point =
                (vec3){border_points.array[i * 3 + 3], border_points.array[i * 3 + 4], border_points.array[i * 3 + 5]};
            continue;
        }
        generate_line_segments(positions, p0, p1, 0.05f);
    }
    vec3 last_point = {border_points.array[border_points.size - 3], border_points.array[border_points.size - 2],
                       border_points.array[border_points.size - 1]};
    generate_line_segments(positions, last_point, first_point, 0.05f);
    cstrl_da_float_free(&border_points);
    for (int i = initial_size / 3; i < positions->size / 3; i++)
    {
        cstrl_da_float_push_back(colors, color.r);
        cstrl_da_float_push_back(colors, color.g);
        cstrl_da_float_push_back(colors, color.b);
        cstrl_da_float_push_back(colors, color.a);
    }
}

void old_update_border(da_float *positions, da_int *indices, da_float *colors, da_float *origin_points, vec4 color)
{
    for (int i = 0; i < origin_points->size / 3; i++)
    {
        vec3 origin_point = {origin_points->array[i * 3], origin_points->array[i * 3 + 1],
                             origin_points->array[i * 3 + 2]};
        vec3 p0 = cstrl_vec3_add(origin_point, (vec3){0.0f, UNIT_SIZE.x, 0.0f});
        vec3 p1 = cstrl_vec3_add(origin_point, (vec3){-UNIT_SIZE.x, 0.0f, 0.0f});
        vec3 p2 = cstrl_vec3_add(origin_point, (vec3){0.0f, -UNIT_SIZE.x, 0.0f});
        vec3 p3 = cstrl_vec3_add(origin_point, (vec3){UNIT_SIZE.x, 0.0f, 0.0f});
        float angle = acosf(cstrl_vec3_dot((vec3){0.0f, 0.0f, 1.0f}, origin_point));
        vec3 axis = cstrl_vec3_cross(origin_point, (vec3){0.0f, 0.0f, 1.0f});
        if (!cstrl_vec3_is_equal(axis, (vec3){0.0f, 0.0f, 0.0f}))
        {
            p0 = cstrl_vec3_rotate_along_axis(p0, angle, axis);
            p1 = cstrl_vec3_rotate_along_axis(p1, angle, axis);
            p2 = cstrl_vec3_rotate_along_axis(p2, angle, axis);
            p3 = cstrl_vec3_rotate_along_axis(p3, angle, axis);
        }

        p0 = cstrl_vec3_normalize(p0);
        p1 = cstrl_vec3_normalize(p1);
        p2 = cstrl_vec3_normalize(p2);
        p3 = cstrl_vec3_normalize(p3);

        size_t initial_size = positions->size;

        cstrl_da_float_push_back(positions, origin_point.x);
        cstrl_da_float_push_back(positions, origin_point.y);
        cstrl_da_float_push_back(positions, origin_point.z);

        generate_line_segments(positions, p0, p1, 0.5f);
        generate_line_segments(positions, p1, p2, 0.5f);
        generate_line_segments(positions, p2, p3, 0.5f);
        generate_line_segments(positions, p3, p0, 0.5f);
        for (int j = initial_size / 3 + 1; j < (positions->size - initial_size) / 3 - 1; j++)
        {
            cstrl_da_int_push_back(indices, 0);
            cstrl_da_int_push_back(indices, j);
            cstrl_da_int_push_back(indices, j + 1);
        }

        for (int j = initial_size / 3; j < (positions->size - initial_size) / 3; j++)
        {
            cstrl_da_float_push_back(colors, color.r);
            cstrl_da_float_push_back(colors, color.g);
            cstrl_da_float_push_back(colors, color.b);
            cstrl_da_float_push_back(colors, color.a);
        }
    }
}

void update_formation_state(int player_id)
{
    for (int i = 0; i < g_players.formations[player_id].count; i++)
    {
        g_players.formations[player_id].moving[i] = false;
        for (int j = 0; j < g_players.formations[player_id].unit_ids[i].size; j++)
        {
            int path_id = g_players.formations[player_id].path_heads[i].array[j];
            if (path_id == -1)
            {
                continue;
            }
            g_players.formations[player_id].moving[i] = true;
            int unit_id = g_players.formations[player_id].unit_ids[i].array[j];
            path_update(&g_players.paths[player_id], path_id);
            if (fabsf(cstrl_vec3_length(cstrl_vec3_sub(g_players.paths[player_id].end_positions[path_id],
                                                       g_players.units[player_id].position[unit_id]))) <
                UNIT_SIZE.x * 0.5f)
            {
                g_players.paths[player_id].render[path_id] = false;
            }
            if (g_players.paths[player_id].completed[path_id] || !g_players.paths[player_id].active[path_id])
            {
                g_players.formations[player_id].path_heads[i].array[j] = g_players.paths[player_id].next[path_id];
                if (g_players.formations[player_id].path_heads[i].array[j] != -1)
                {
                    g_players.paths[player_id].in_queue[g_players.formations[player_id].path_heads[i].array[j]] = false;
                }
                paths_remove(&g_players.paths[player_id], path_id);
                continue;
            }
            vec3 start_position = g_players.paths[player_id].start_positions[path_id];
            vec3 end_position = g_players.paths[player_id].end_positions[path_id];
            g_players.units[player_id].position[unit_id] =
                get_point_on_path(start_position, end_position, g_players.paths[player_id].progress[path_id]);
        }
    }
}

int planet_game()
{
    if (!cstrl_platform_init(&g_platform_state, "Planet", (1920 - INITIAL_WINDOW_WIDTH) / 2,
                             (1080 - INITIAL_WINDOW_HEIGHT) / 2, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT))
    {
        cstrl_platform_shutdown(&g_platform_state);
        return 1;
    }
    cstrl_platform_set_framebuffer_size_callback(&g_platform_state, framebuffer_callback);
    cstrl_platform_set_key_callback(&g_platform_state, key_callback);
    cstrl_platform_set_mouse_position_callback(&g_platform_state, mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(&g_platform_state, mouse_button_callback);
    cstrl_platform_set_mouse_wheel_callback(&g_platform_state, mouse_wheel_callback);
    cstrl_renderer_init(&g_platform_state);

    srand(cstrl_platform_get_absolute_time());

    /*
    cstrl_render_data *planet_render_data = cstrl_renderer_create_render_data();
    float planet_positions[(PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1) * 3];
    float planet_normals[(PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1) * 3];
    float planet_uvs[(PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1) * 2];
    int planet_indices[(PLANET_LATITUDE_POINTS + 1) * PLANET_LONGITUDE_POINTS * 6];

    generate_sphere_lat_long(planet_positions, planet_indices, planet_uvs, planet_normals, PLANET_LATITUDE_POINTS,
                             PLANET_LONGITUDE_POINTS);
    float planet_colors[PLANET_LATITUDE_POINTS * PLANET_LONGITUDE_POINTS * 4];
    for (int i = 0; i < PLANET_LATITUDE_POINTS * PLANET_LONGITUDE_POINTS; i++)
    {
        planet_colors[i * 4] = 0.9f;
        planet_colors[i * 4 + 1] = 0.2f;
        planet_colors[i * 4 + 2] = 0.8f;
        planet_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(planet_render_data, planet_positions, 3,
                                 (PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1));
    cstrl_renderer_add_uvs(planet_render_data, planet_uvs);
    cstrl_renderer_add_colors(planet_render_data, planet_colors);
    cstrl_renderer_add_normals(planet_render_data, planet_normals);
    cstrl_renderer_add_indices(planet_render_data, planet_indices,
                               (PLANET_LATITUDE_POINTS - 1) * PLANET_LONGITUDE_POINTS * 6);
    */
    cstrl_render_data *planet_render_data = cstrl_renderer_create_render_data();
    int resolution = 10;
    float planet_positions[resolution * resolution * 18];
    int planet_indices[(resolution - 1) * (resolution - 1) * 36];
    float planet_normals[resolution * resolution * 18];
    float planet_uvs[resolution * resolution * 12];
    float planet_colors[resolution * resolution * 24];
    generate_sphere_cube(planet_positions, planet_indices, planet_uvs, planet_normals, resolution);
    for (int i = 0; i < resolution * resolution * 6; i++)
    {
        planet_colors[i * 4] = 0.9f;
        planet_colors[i * 4 + 1] = 0.2f;
        planet_colors[i * 4 + 2] = 0.8f;
        planet_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(planet_render_data, planet_positions, 3, resolution * resolution * 6);
    cstrl_renderer_add_uvs(planet_render_data, planet_uvs);
    cstrl_renderer_add_colors(planet_render_data, planet_colors);
    cstrl_renderer_add_normals(planet_render_data, planet_normals);
    cstrl_renderer_add_indices(planet_render_data, planet_indices, (resolution - 1) * (resolution - 1) * 36);
    cstrl_shader planet_shader =
        cstrl_load_shaders_from_files("resources/shaders/planet.vert", "resources/shaders/planet.frag");

    cstrl_texture planet_texture = cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/moon/");

    players_init(&g_players, 6);
    units_add(&g_players.units[0], (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 0.5f}, CITY);
    vec3 start_position = (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 0.5f};
    start_position = cstrl_vec3_normalize(cstrl_vec3_add(start_position, cstrl_vec3_mult_scalar(UNIT_SIZE, 7.0f)));
    start_position = cstrl_vec3_mult_scalar(start_position, 1.0f + UNIT_SIZE.x * 0.5f);
    units_add(&g_players.units[0], start_position, CITY);
    units_add(&g_players.units[1], (vec3){1.0f + UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[2], (vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x * 0.5f}, CITY);
    units_add(&g_players.units[3], (vec3){-1.0f - UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[4], (vec3){0.0f, 1.0f + UNIT_SIZE.x * 0.5f, 0.0f}, CITY);
    units_add(&g_players.units[5], (vec3){0.0f, -1.0f - UNIT_SIZE.x * 0.5f, 0.0f}, CITY);
    /*
    players_init(&g_players, 6);
    units_add(&g_players.units[0], (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 0.5f}, CITY);
    units_add(&g_players.units[1], (vec3){1.0f + UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[2], (vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x * 0.5f}, CITY);
    units_add(&g_players.units[3], (vec3){-1.0f - UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[4], (vec3){0.0f, 1.0f + UNIT_SIZE.x * 0.5f, 0.0f}, CITY);
    units_add(&g_players.units[5], (vec3){0.0f, -1.0f - UNIT_SIZE.x * 0.5f, 0.0f}, CITY);

    for (int i = 0; i < 5; i++)
    {
        units_add(&g_players.units[0], (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 0.5f}, HUMVEE);
        units_add(&g_players.units[1], (vec3){1.0f + UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, HUMVEE);
        units_add(&g_players.units[2], (vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x * 0.5f}, HUMVEE);
        units_add(&g_players.units[3], (vec3){-1.0f - UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, HUMVEE);
        units_add(&g_players.units[4], (vec3){0.0f, 1.0f + UNIT_SIZE.x * 0.5f, 0.0f}, HUMVEE);
        units_add(&g_players.units[5], (vec3){0.0f, -1.0f - UNIT_SIZE.x * 0.5f, 0.0f}, HUMVEE);

        units_add(&g_players.units[0], (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 0.5f}, TANK);
        units_add(&g_players.units[1], (vec3){1.0f + UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, TANK);
        units_add(&g_players.units[2], (vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x * 0.5f}, TANK);
        units_add(&g_players.units[3], (vec3){-1.0f - UNIT_SIZE.x * 0.5f, 0.0f, 0.0f}, TANK);
        units_add(&g_players.units[4], (vec3){0.0f, 1.0f + UNIT_SIZE.x * 0.5f, 0.0f}, TANK);
        units_add(&g_players.units[5], (vec3){0.0f, -1.0f - UNIT_SIZE.x * 0.5f, 0.0f}, TANK);

        units_add(&g_players.units[0], (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 5.0f}, JET);
        units_add(&g_players.units[1], (vec3){1.0f + UNIT_SIZE.x * 5.0f, 0.0f, 0.0f}, JET);
        units_add(&g_players.units[2], (vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x * 5.0f * 1.5f}, JET);
        units_add(&g_players.units[3], (vec3){-1.0f - UNIT_SIZE.x * 5.0f, 0.0f, 0.0f}, JET);
        units_add(&g_players.units[4], (vec3){0.0f, 1.0f + UNIT_SIZE.x * 5.0f * 1.5f, 0.0f}, JET);
        units_add(&g_players.units[5], (vec3){0.0f, -1.0f - UNIT_SIZE.x * 5.0f, 0.0f}, JET);

        units_add(&g_players.units[0], (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 5.0f}, PLANE);
        units_add(&g_players.units[1], (vec3){1.0f + UNIT_SIZE.x * 5.0f, 0.0f, 0.0f}, PLANE);
        units_add(&g_players.units[2], (vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x * 5.0f * 1.5f}, PLANE);
        units_add(&g_players.units[3], (vec3){-1.0f - UNIT_SIZE.x * 5.0f, 0.0f, 0.0f}, PLANE);
        units_add(&g_players.units[4], (vec3){0.0f, 1.0f + UNIT_SIZE.x * 5.0f * 1.5f, 0.0f}, PLANE);
        units_add(&g_players.units[5], (vec3){0.0f, -1.0f - UNIT_SIZE.x * 5.0f, 0.0f}, PLANE);
    }
    */

    cstrl_render_data *border_render_data = cstrl_renderer_create_render_data();
    da_float border_positions;
    cstrl_da_float_init(&border_positions, 12);
    da_int border_indices;
    cstrl_da_int_init(&border_indices, 6);
    da_float border_colors;
    cstrl_da_float_init(&border_colors, 16);
    recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[0], &g_players.units[0]);
    recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[1], &g_players.units[1]);
    recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[2], &g_players.units[2]);
    recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[3], &g_players.units[3]);
    recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[4], &g_players.units[4]);
    recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[5], &g_players.units[5]);
    cstrl_renderer_add_positions(border_render_data, border_positions.array, 3, border_positions.size / 3);
    // cstrl_renderer_add_indices(border_render_data, border_indices.array, border_indices.size);
    cstrl_renderer_add_colors(border_render_data, border_colors.array);

    cstrl_shader border_shader = cstrl_load_shaders_from_files("resources/shaders/default3D_no_texture.vert",
                                                               "resources/shaders/default3D_no_texture.frag");

    ai_t ai;
    ai_init(&ai, 6, g_human_player);

    cstrl_render_data *unit_render_data = cstrl_renderer_create_render_data();
    da_float unit_positions;
    cstrl_da_float_init(&unit_positions, 12);
    da_int unit_indices;
    cstrl_da_int_init(&unit_indices, 6);
    da_float unit_uvs;
    cstrl_da_float_init(&unit_uvs, 8);
    da_float unit_colors;
    cstrl_da_float_init(&unit_colors, 16);
    add_billboard_object(
        &unit_positions, &unit_indices, &unit_uvs, &unit_colors,
        (transform){g_players.units[g_human_player].position[0], (quat){1.0f, 0.0f, 0.0f, 0.0f}, UNIT_SIZE},
        (vec4){0.0f, 0.0f, 0.25f, 1.0f}, (vec4){0.0f, 1.0f, 0.0f, 1.0f});
    cstrl_renderer_add_positions(unit_render_data, unit_positions.array, 3, 4);
    cstrl_renderer_add_indices(unit_render_data, unit_indices.array, 6);
    cstrl_renderer_add_uvs(unit_render_data, unit_uvs.array);
    cstrl_renderer_add_colors(unit_render_data, unit_colors.array);

    cstrl_shader unit_shader =
        cstrl_load_shaders_from_files("resources/shaders/default3D.vert", "resources/shaders/default3D.frag");
    cstrl_texture unit_texture = cstrl_texture_generate_from_path("resources/textures/planet_game/icons.png");

    cstrl_render_data *path_marker_render_data = cstrl_renderer_create_render_data();
    da_float path_marker_positions;
    cstrl_da_float_init(&path_marker_positions, 12);
    da_int path_marker_indices;
    cstrl_da_int_init(&path_marker_indices, 6);
    da_float path_marker_colors;
    cstrl_da_float_init(&path_marker_colors, 16);
    add_billboard_object(&path_marker_positions, &path_marker_indices, NULL, &path_marker_colors,
                         (transform){(vec3){0.0f, 0.0f, 0.0f}, (quat){1.0f, 0.0f, 0.0f, 0.0f}, PATH_MARKER_SIZE},
                         (vec4){}, PATH_MARKER_COLOR);
    cstrl_renderer_add_positions(path_marker_render_data, path_marker_positions.array, 3, 4);
    cstrl_renderer_add_indices(path_marker_render_data, path_marker_indices.array, 6);
    cstrl_renderer_add_colors(path_marker_render_data, path_marker_colors.array);

    cstrl_shader path_marker_shader = cstrl_load_shaders_from_files("resources/shaders/default3D_no_texture.vert",
                                                                    "resources/shaders/default3D_no_texture.frag");

    cstrl_render_data *path_line_render_data = cstrl_renderer_create_render_data();
    da_float path_line_positions;
    cstrl_da_float_init(&path_line_positions, 6);
    for (int i = 0; i < 6; i++)
    {
        cstrl_da_float_push_back(&path_line_positions, 0.0f);
    }
    cstrl_renderer_add_positions(path_line_render_data, path_line_positions.array, 3, 2);

    cstrl_shader path_line_shader =
        cstrl_load_shaders_from_files("resources/shaders/line3D.vert", "resources/shaders/line3D.frag");

    cstrl_render_data *selection_box_render_data = cstrl_renderer_create_render_data();
    da_float selection_box_positions;
    cstrl_da_float_init(&selection_box_positions, 8);
    da_int selection_box_indices;
    cstrl_da_int_init(&selection_box_indices, 6);
    da_float selection_box_colors;
    cstrl_da_float_init(&selection_box_colors, 16);
    add_ui_object(&selection_box_positions, &selection_box_indices, NULL, &selection_box_colors, (vec2){0.0f, 0.0f},
                  (vec2){0.0f, 0.0f}, (vec4){0.0f, 0.0f, 1.0f, 0.5f});
    cstrl_renderer_add_positions(selection_box_render_data, selection_box_positions.array, 2, 4);
    cstrl_renderer_add_indices(selection_box_render_data, selection_box_indices.array, 6);
    cstrl_renderer_add_colors(selection_box_render_data, selection_box_colors.array);

    cstrl_shader selection_box_shader = cstrl_load_shaders_from_files("resources/shaders/default_no_texture.vert",
                                                                      "resources/shaders/default_no_texture.frag");

    cstrl_render_data *skybox_render_data = cstrl_renderer_create_render_data();
    float positions[108];
    generate_cube_positions(positions);
    cstrl_renderer_add_positions(skybox_render_data, positions, 3, 36);

    cstrl_shader skybox_shader =
        cstrl_load_shaders_from_files("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
    cstrl_texture skybox_texture = cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/stars/");

    g_main_camera = cstrl_camera_create(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, false);
    g_main_camera->position.z = 3.0f;

    g_ui_camera = cstrl_camera_create(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, true);

    cstrl_set_uniform_3f(planet_shader.program, "material.ambient", 0.3f, 0.3f, 0.3f);
    cstrl_set_uniform_3f(planet_shader.program, "material.diffuse", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(planet_shader.program, "material.specular", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_float(planet_shader.program, "material.shininess", 4.0f);
    cstrl_set_uniform_3f(planet_shader.program, "light.specular", 1.0f, 1.0f, 1.0f);
    cstrl_set_uniform_3f(planet_shader.program, "light.ambient", 0.2f, 0.2f, 0.2f);
    cstrl_set_uniform_3f(planet_shader.program, "light.diffuse", 0.8f, 0.8f, 0.8f);

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
            cstrl_camera_update(g_ui_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
            cstrl_renderer_clear_render_attributes(selection_box_render_data);
            if (g_making_selection)
            {
                update_ui_object(&selection_box_positions, &selection_box_indices, NULL, &selection_box_colors, 0,
                                 g_selection_start, g_selection_end, (vec4){1.0f, 0.0f, 0.0f, 0.1f});
                cstrl_renderer_modify_render_attributes(selection_box_render_data, selection_box_positions.array, NULL,
                                                        selection_box_colors.array, selection_box_positions.size / 2);
                cstrl_renderer_modify_indices(selection_box_render_data, selection_box_indices.array, 0,
                                              selection_box_indices.size);
            }
            if (g_border_update)
            {
                recalculate_border(&border_positions, &border_indices, &border_colors, UNIT_TEAM_COLORS[0],
                                   &g_players.units[0]);
                cstrl_renderer_modify_render_attributes(border_render_data, border_positions.array, NULL,
                                                        border_colors.array, border_positions.size / 3);
                g_border_update = false;
            }
            // TODO: consider not clearing data on each run
            cstrl_da_float_clear(&unit_positions);
            cstrl_da_int_clear(&unit_indices);
            cstrl_da_float_clear(&unit_uvs);
            cstrl_da_float_clear(&unit_colors);
            cstrl_renderer_clear_render_attributes(unit_render_data);

            cstrl_da_float_clear(&path_line_positions);
            cstrl_da_float_clear(&path_marker_positions);
            cstrl_renderer_clear_render_attributes(path_line_render_data);
            cstrl_renderer_clear_render_attributes(path_marker_render_data);
            quat billboard_rotation =
                cstrl_quat_inverse(cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view)));
            ai_update(&ai, &g_players);
            int unit_render_index = 0;
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                update_formation_state(i);
                for (int j = 0; j < g_players.units[i].count; j++)
                {
                    if (!g_players.units[i].active[j])
                    {
                        continue;
                    }
                    vec4 color = UNIT_TEAM_COLORS[i];
                    if (i == g_human_player && cstrl_da_int_find_first(&g_players.selected_units[i], j) != -1)
                    {
                        color.r += 0.2f;
                        color.g += 0.2f;
                        color.b += 0.2f;
                        color.a = 1.0f;
                    }
                    int type = g_players.units[i].type[j];
                    vec4 uv_positions = {(float)type / MAX_UNIT_TYPES, 0.0f, ((float)type + 1.0f) / MAX_UNIT_TYPES,
                                         1.0f};
                    update_billboard_object(&unit_positions, &unit_indices, &unit_uvs, &unit_colors, unit_render_index,
                                            (transform){g_players.units[i].position[j], billboard_rotation,
                                                        adjust_billboard_scale(UNIT_SIZE)},
                                            uv_positions, color);
                    unit_render_index++;
                }
            }
            if (unit_positions.size > 0)
            {
                cstrl_renderer_modify_render_attributes(unit_render_data, unit_positions.array, unit_uvs.array,
                                                        unit_colors.array, unit_positions.size / 3);
                cstrl_renderer_modify_indices(unit_render_data, unit_indices.array, 0, unit_indices.size);
            }

            if ((g_render_path_lines || g_render_path_markers) && g_players.selected_formation[g_human_player] != -1)
            {
                int render_index = 0;
                int g_human_selected_formation = g_players.selected_formation[g_human_player];
                for (int i = 0; i < g_players.formations[g_human_player].path_heads[g_human_selected_formation].size;
                     i++)
                {
                    int path_id = g_players.formations[g_human_player].path_heads[g_human_selected_formation].array[i];
                    while (path_id != -1)
                    {
                        if (g_render_path_markers)
                        {
                            if (!g_players.paths[g_human_player].render[path_id] ||
                                !g_players.paths[g_human_player].active[path_id] ||
                                g_players.paths[g_human_player].completed[path_id])
                            {
                                path_id = g_players.paths[g_human_player].next[path_id];
                                continue;
                            }
                            update_billboard_object(
                                &path_marker_positions, &path_marker_indices, NULL, &path_marker_colors, render_index,
                                (transform){g_players.paths[g_human_player].end_positions[path_id], billboard_rotation,
                                            adjust_billboard_scale(PATH_MARKER_SIZE)},
                                (vec4){}, PATH_MARKER_COLOR);
                            render_index++;
                        }
                        if (g_render_path_lines)
                        {
                            vec3 start = g_players.paths[g_human_player].start_positions[path_id];
                            if (!g_players.paths[g_human_player].in_queue[path_id])
                            {
                                start =
                                    g_players.units[g_human_player].position[g_players.formations[g_human_player]
                                                                                 .unit_ids[g_human_selected_formation]
                                                                                 .array[i]];
                            }
                            generate_line_segments(&path_line_positions, start,
                                                   g_players.paths[g_human_player].end_positions[path_id], 0.1f);
                        }
                        path_id = g_players.paths[g_human_player].next[path_id];
                    }
                }
                if (path_marker_positions.size > 0)
                {
                    cstrl_renderer_modify_render_attributes(path_marker_render_data, path_marker_positions.array, NULL,
                                                            path_marker_colors.array, path_marker_positions.size / 3);
                    cstrl_renderer_modify_indices(path_marker_render_data, path_marker_indices.array, 0,
                                                  path_marker_indices.size);
                }
                if (path_line_positions.size > 0)
                {
                    cstrl_renderer_modify_positions(path_line_render_data, path_line_positions.array, 0,
                                                    path_line_positions.size);
                }
            }
            lag -= 1.0 / 60.0;
            light_start_x += 0.001f;
            light_start_z += 0.001f;
        }
        cstrl_renderer_clear(0.1f, 0.0f, 0.1f, 1.0f);
        vec3 light_position = {10.0f * cosf(light_start_x), 1.0f, 10.0f * sinf(light_start_z)};
        cstrl_set_uniform_mat4(planet_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(planet_shader.program, "projection", g_main_camera->projection);
        cstrl_set_uniform_3f(planet_shader.program, "view_position", g_main_camera->position.x,
                             g_main_camera->position.y, g_main_camera->position.z);
        cstrl_set_uniform_3f(planet_shader.program, "light.position", light_position.x, light_position.y,
                             light_position.z);
        cstrl_use_shader(planet_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_cube_map_bind(planet_texture);
        cstrl_renderer_draw_indices(planet_render_data);

        cstrl_set_uniform_mat4(border_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(border_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(border_shader);
        cstrl_renderer_draw_lines(border_render_data);

        cstrl_set_uniform_mat4(path_marker_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(path_marker_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(path_marker_shader);
        cstrl_renderer_draw_indices(path_marker_render_data);
        cstrl_set_uniform_mat4(path_line_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(path_line_shader.program, "projection", g_main_camera->projection);
        cstrl_set_uniform_4f(path_line_shader.program, "color", PATH_MARKER_COLOR.r, PATH_MARKER_COLOR.g,
                             PATH_MARKER_COLOR.b, PATH_MARKER_COLOR.a);
        cstrl_use_shader(path_line_shader);
        cstrl_renderer_draw_lines(path_line_render_data);

        cstrl_set_uniform_mat4(unit_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(unit_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(unit_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(unit_texture);
        cstrl_renderer_draw_indices(unit_render_data);

        cstrl_set_uniform_mat4(selection_box_shader.program, "view", g_ui_camera->view);
        cstrl_set_uniform_mat4(selection_box_shader.program, "projection", g_ui_camera->projection);
        cstrl_use_shader(selection_box_shader);
        cstrl_renderer_draw_indices(selection_box_render_data);

        cstrl_set_uniform_mat4(skybox_shader.program, "view", cstrl_mat4_view_remove_translation(g_main_camera->view));
        cstrl_set_uniform_mat4(skybox_shader.program, "projection", g_main_camera->projection);
        cstrl_use_shader(skybox_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_cube_map_bind(skybox_texture);
        cstrl_renderer_draw(skybox_render_data);
        /*
        cstrl_ui_begin(context);
        if (cstrl_ui_container_begin(context, "Debug", 5, 10, 10, 200, 300, GEN_ID(0), false, 2))
        {
            char buffer[64];
            char title[64];
            if (g_units.count > 0)
            {
                if (cstrl_ui_text(context, "Placeholder", 11, 0, 50, 300, 50, GEN_ID(0), CSTRL_UI_TEXT_ALIGN_LEFT))
                {
                }
            }
            cstrl_ui_container_end(context);
        }
        cstrl_ui_end(context);
        */
        cstrl_renderer_swap_buffers(&g_platform_state);
    }

    cstrl_da_float_free(&unit_positions);
    cstrl_da_float_free(&unit_uvs);
    cstrl_da_float_free(&unit_colors);
    cstrl_da_int_free(&unit_indices);

    cstrl_da_float_free(&path_marker_positions);
    cstrl_da_int_free(&path_marker_indices);
    cstrl_da_float_free(&path_marker_colors);

    cstrl_da_float_free(&path_line_positions);

    cstrl_da_float_free(&selection_box_positions);
    cstrl_da_int_free(&selection_box_indices);
    cstrl_da_float_free(&selection_box_colors);

    cstrl_camera_free(g_main_camera);
    cstrl_camera_free(g_ui_camera);
    cstrl_renderer_free_render_data(planet_render_data);
    cstrl_renderer_free_render_data(unit_render_data);
    cstrl_renderer_free_render_data(path_marker_render_data);
    cstrl_renderer_free_render_data(path_line_render_data);
    cstrl_renderer_free_render_data(selection_box_render_data);
    cstrl_ui_shutdown(context);
    cstrl_renderer_shutdown(&g_platform_state);
    cstrl_platform_shutdown(&g_platform_state);

    return 0;
}

#include "planet.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include "game/ai.h"
#include "game/formation.h"
#include "game/paths.h"
#include "game/physics_wrapper.h"
#include "game/players.h"
#include "game/projectile.h"
#include "game/sphere.h"
#include "game/units.h"
#include "helpers/helpers.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_WINDOW_WIDTH 1280
#define INITIAL_WINDOW_HEIGHT 720

#define PLANET_LATITUDE_POINTS 18
#define PLANET_LONGITUDE_POINTS 36

#define PLANET_LATITUDE_MAX_COS 0.98f

#define ACTIVE_PLAYERS 6

#define SPHERE_RESOLUTION 10

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
static const vec4 PATH_MARKER_COLOR_ATTACK = {0.8f, 0.2f, 0.2f, 0.9f};

static const vec4 UNIT_TEAM_COLORS[] = {
    (vec4){0.6f, 0.0f, 0.0f, 0.8f}, (vec4){0.0f, 0.2f, 0.8f, 0.8f}, (vec4){0.0f, 0.6f, 0.0f, 0.8f},
    (vec4){0.6f, 0.6f, 0.0f, 0.8f}, (vec4){0.8f, 0.8f, 0.8f, 0.8f}, (vec4){0.0f, 0.6f, 0.6f, 0.8f},
    (vec4){0.6f, 0.0f, 0.6f, 0.8f}, (vec4){0.3f, 0.3f, 0.3f, 0.8f},
};

static const vec4 BORDER_COLORS[] = {
    (vec4){0.6f, 0.0f, 0.0f, 0.2f}, (vec4){0.0f, 0.2f, 0.6f, 0.2f}, (vec4){0.0f, 0.6f, 0.0f, 0.2f},
    (vec4){0.6f, 0.6f, 0.0f, 0.2f}, (vec4){0.8f, 0.8f, 0.8f, 0.2f}, (vec4){0.0f, 0.6f, 0.6f, 0.2f},
    (vec4){0.6f, 0.0f, 0.6f, 0.2f}, (vec4){0.3f, 0.3f, 0.3f, 0.2f},
};

static bool g_render_path_markers = false;
static bool g_render_path_lines = false;

static bool g_making_ground_selection = false;
static bool g_making_air_selection = false;
static vec2 g_selection_start;
static vec2 g_selection_end;

static bool g_border_update = true;

static cstrl_ui_context *g_ui_context;

static da_float g_physics_ray_positions;
static bool g_physics_debug_draw_enabled = false;

static bool g_view_projection_update = true;
static bool g_ui_view_projection_update = true;

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
    int type = g_players.units[g_human_player].type[g_players.selected_units[g_human_player].array[0]];
    if (type == CITY)
    {
        return;
    }
    vec3 d = mouse_cursor_ray_cast();
    float t;
    bool ground_units = type == TANK || type == HUMVEE || type == ASTRONAUT || type == ASTRONAUT_ARMED;
    if (planet_hit_check(d, &t, g_main_camera->position, g_planet_position,
                         (PLANET_SIZE.x + UNIT_SIZE.x * (ground_units ? 1.0 : 10.0f)) * 0.5f))
    {
        vec3 end_position = position_from_ray_cast(d, t);
        if (!ground_units)
        {
            end_position = cstrl_vec3_mult_scalar(end_position, 1.0f + UNIT_SIZE.x * 5.0f);
        }
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

static void framebuffer_callback(cstrl_platform_state *state, int width, int height)
{
    g_main_camera->viewport = (vec2i){width, height};
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    g_ui_camera->viewport = (vec2i){width, height};
    cstrl_camera_update(g_ui_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_renderer_set_viewport(0, 0, width, height);
    g_view_projection_update = true;
    g_ui_view_projection_update = true;
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
    case CSTRL_KEY_R:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->forward =
                cstrl_vec3_normalize(cstrl_vec3_negate(g_players.units[g_human_player].position[0]));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            g_main_camera->position = cstrl_vec3_mult_scalar(g_main_camera->forward, -3.0f);
            g_view_projection_update = true;
        }
        break;
    case CSTRL_KEY_1:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (planet_hit_check(d, &t, g_main_camera->position, g_planet_position,
                                 (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(g_main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t);
                    units_add(&g_players.units[g_human_player], g_human_player, position, ASTRONAUT);
                    g_border_update = true;
                }
            }
        }
        break;
    case CSTRL_KEY_2:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (planet_hit_check(d, &t, g_main_camera->position, g_planet_position,
                                 (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(g_main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t);
                    units_add(&g_players.units[g_human_player], g_human_player, position, ASTRONAUT_ARMED);
                    g_border_update = true;
                }
            }
        }
        break;
    case CSTRL_KEY_3:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (planet_hit_check(d, &t, g_main_camera->position, g_planet_position,
                                 (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(g_main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t);
                    units_add(&g_players.units[g_human_player], g_human_player, position, HUMVEE);
                    g_border_update = true;
                }
            }
        }
        break;
    case CSTRL_KEY_4:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (planet_hit_check(d, &t, g_main_camera->position, g_planet_position,
                                 (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(g_main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t);
                    units_add(&g_players.units[g_human_player], g_human_player, position, TANK);
                    g_border_update = true;
                }
            }
        }
        break;
    case CSTRL_KEY_5:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_render_path_markers = !g_render_path_markers;
            g_render_path_lines = g_render_path_markers;
        }
        break;
    case CSTRL_KEY_C:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (planet_hit_check(d, &t, g_main_camera->position, g_planet_position,
                                 (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
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
                    units_add(&g_players.units[player_id], g_human_player, old_position, old_type);
                }
            }
            g_border_update = true;
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
    case CSTRL_KEY_D:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_physics_debug_draw_enabled = !g_physics_debug_draw_enabled;
        }
        break;
    case CSTRL_KEY_S:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_players.selected_formation[g_human_player] = -1;
            g_human_player = (g_human_player + 1) % 6;
            g_players.selected_formation[g_human_player] = -1;
            g_main_camera->forward =
                cstrl_vec3_normalize(cstrl_vec3_negate(g_players.units[g_human_player].position[0]));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            g_main_camera->position = cstrl_vec3_mult_scalar(g_main_camera->forward, -3.0f);
            g_view_projection_update = true;
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

    if (g_ui_context->mouse_locked)
    {
        return;
    }

    if (g_making_ground_selection || g_making_air_selection)
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
    g_view_projection_update = true;
}

static void mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
{
    if (g_ui_context->mouse_locked)
    {
        return;
    }
    if (button == CSTRL_MOUSE_BUTTON_LEFT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            if (mods & CSTRL_KEY_MOD_ALT)
            {
                vec3 mouse_ray_direction = mouse_cursor_ray_cast();
                vec3 position = g_main_camera->position;
                ray_cast_result_t result = regular_ray_cast(position, mouse_ray_direction, 5.0f, NULL);
                printf("position: %f, %f, %f\n", position.x, position.y, position.z);
                printf("direction: %f, %f, %f\n", mouse_cursor_ray_cast().x, mouse_cursor_ray_cast().y,
                       mouse_cursor_ray_cast().z);
                cstrl_da_float_push_back(&g_physics_ray_positions, position.x);
                cstrl_da_float_push_back(&g_physics_ray_positions, position.y);
                cstrl_da_float_push_back(&g_physics_ray_positions, position.z);
                vec3 end_position = cstrl_vec3_add(position, cstrl_vec3_mult_scalar(mouse_ray_direction, 5.0f));
                cstrl_da_float_push_back(&g_physics_ray_positions, end_position.x);
                cstrl_da_float_push_back(&g_physics_ray_positions, end_position.y);
                cstrl_da_float_push_back(&g_physics_ray_positions, end_position.z);
                if (result.hit)
                {
                    printf("cool\n");
                }
            }
            if (mods & CSTRL_KEY_MOD_CONTROL)
            {
                g_making_ground_selection = true;
                g_selection_start = (vec2){g_mouse_position_x, g_mouse_position_y};
                g_selection_end = g_selection_start;
                g_players.selected_formation[g_human_player] = -1;
                cstrl_da_int_clear(&g_players.selected_units[g_human_player]);
            }
            else if (mods & CSTRL_KEY_MOD_SHIFT)
            {
                g_making_air_selection = true;
                g_selection_start = (vec2){g_mouse_position_x, g_mouse_position_y};
                g_selection_end = g_selection_start;
                g_players.selected_formation[g_human_player] = -1;
                cstrl_da_int_clear(&g_players.selected_units[g_human_player]);
            }
            else
            {
                vec2 mouse_position = {g_mouse_position_x, g_mouse_position_y};
                players_select_units(&g_players, g_human_player, g_main_camera->viewport.x, g_main_camera->viewport.y,
                                     mouse_position, mouse_position, g_main_camera, 0);
            }
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            if (g_making_ground_selection || g_making_air_selection)
            {
                players_select_units(&g_players, g_human_player, g_main_camera->viewport.x, g_main_camera->viewport.y,
                                     g_selection_start, g_selection_end, g_main_camera,
                                     g_making_ground_selection ? 1
                                     : g_making_air_selection  ? 2
                                                               : 0);
                g_making_ground_selection = false;
                g_making_air_selection = false;
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
            g_view_projection_update = true;
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
    if (g_ui_context->mouse_locked)
    {
        return;
    }
    g_main_camera->fov -= (float)delta_y / 6000.0f;
    if (g_main_camera->fov <= 20.0f * cstrl_pi_180)
    {
        g_main_camera->fov = 20.0f * cstrl_pi_180;
    }
    else if (g_main_camera->fov >= 60.0f * cstrl_pi_180)
    {
        g_main_camera->fov = 60.0f * cstrl_pi_180;
    }
    g_view_projection_update = true;
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

    if (colors != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            cstrl_da_float_push_back(colors, color.r);
            cstrl_da_float_push_back(colors, color.g);
            cstrl_da_float_push_back(colors, color.b);
            cstrl_da_float_push_back(colors, color.a);
        }
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

        if (colors != NULL)
        {
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

void update_formation_state(int player_id)
{
    for (int i = 0; i < g_players.formations[player_id].count; i++)
    {
        g_players.formations[player_id].moving[i] = false;
        float min_speed = cstrl_infinity;
        for (int j = 0; j < g_players.formations[player_id].path_heads[i].size; j++)
        {
            int path_id = g_players.formations[player_id].path_heads[i].array[j];
            if (path_id == -1)
            {
                continue;
            }
            min_speed = cstrl_min(g_players.paths[player_id].speed[path_id], min_speed);
        }
        for (int j = 0; j < g_players.formations[player_id].unit_ids[i].size; j++)
        {
            int unit_id = g_players.formations[player_id].unit_ids[i].array[j];
            int path_id = g_players.formations[player_id].path_heads[i].array[j];
            if (path_id == -1)
            {
                continue;
            }
            g_players.paths[player_id].speed[path_id] = min_speed;
            g_players.formations[player_id].moving[i] = true;
            // path_update(&g_players.paths[player_id], path_id);
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
            // vec3 start_position = g_players.paths[player_id].start_positions[path_id];
            vec3 end_position = g_players.paths[player_id].end_positions[path_id];
            // vec3 target_position = get_point_on_path(g_planet_position, start_position, end_position,
            //                                          g_players.paths[player_id].progress[path_id]);
            if (units_move(&g_players.units[player_id], unit_id, end_position))
            {
                g_players.paths[player_id].completed[path_id] = true;
            }
        }
    }
}

static vec3 generate_random_planet_position()
{
    vec3 random_position;
    bool position_valid = false;
    while (!position_valid)
    {
        random_position = (vec3){(float)(rand() % 1000 - 500) / 500.0f, (float)(rand() % 500 - 250) / 500.0f,
                                 (float)(rand() % 1000 - 500) / 500.0f};
        random_position = cstrl_vec3_normalize(random_position);
        random_position = cstrl_vec3_mult_scalar(random_position, 1.0f + UNIT_SIZE_X * 0.5f);
        position_valid = true;
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            if (!g_players.active[i] || cstrl_vec3_is_equal(g_players.units[i].position[0], (vec3){0.0f, 0.0f, 0.0f}))
            {
                continue;
            }
            if (get_spherical_path_length(random_position, g_players.units[i].position[0]) < 1.0f)
            {
                position_valid = false;
            }
        }
    }
    return random_position;
}

static void create_and_place_unit(vec3 city_origin, int player_id, unit_type type)
{
    while (true)
    {
        int randx = rand() % 2 - 1;
        int randy = rand() % 2 - 1;
        int randz = randx != 0 && randy != 0 ? rand() % 2 - 1 : 1;
        vec3 unit_position =
            cstrl_vec3_add(city_origin, (vec3){(float)(randx)*UNIT_SIZE_X * 2.0f, (float)(randy)*UNIT_SIZE_Y * 2.0f,
                                               (float)(randz)*UNIT_SIZE_X * 2.0f});
        unit_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(unit_position), 1.0f + UNIT_SIZE_X * 0.5f);
        ray_cast_result_t result = regular_ray_cast(unit_position, cstrl_vec3_normalize(unit_position), 1.0f, NULL);
        if (!result.hit)
        {
            units_add(&g_players.units[player_id], player_id, unit_position, type);
            break;
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

    cstrl_render_data *planet_render_data = cstrl_renderer_create_render_data();
    float planet_positions[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    int planet_indices[(SPHERE_RESOLUTION - 1) * (SPHERE_RESOLUTION - 1) * 36];
    float planet_normals[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    float planet_tangents[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    float planet_bitangents[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    generate_sphere_cube(planet_positions, planet_indices, planet_normals, planet_tangents, planet_bitangents,
                         SPHERE_RESOLUTION);
    cstrl_renderer_add_positions(planet_render_data, planet_positions, 3, SPHERE_RESOLUTION * SPHERE_RESOLUTION * 6);
    cstrl_renderer_add_normals(planet_render_data, planet_normals);
    cstrl_renderer_add_indices(planet_render_data, planet_indices,
                               (SPHERE_RESOLUTION - 1) * (SPHERE_RESOLUTION - 1) * 36);
    cstrl_renderer_add_tangents(planet_render_data, planet_tangents);
    cstrl_renderer_add_bitangents(planet_render_data, planet_bitangents);

    cstrl_shader planet_shader =
        cstrl_load_shaders_from_files("resources/shaders/planet.vert", "resources/shaders/planet.frag");

    cstrl_texture planet_texture =
        cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/moon/", true);
    cstrl_texture planet_normal =
        cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/moon_normals/", true);

    if (!players_init(&g_players, 6))
    {
        printf("Error initializing players\n");
        return 2;
    }
    units_add(&g_players.units[0], 0, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[1], 1, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[2], 2, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[3], 3, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[4], 4, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&g_players.units[5], 5, (vec3){0.0f, 0.0f, 0.0f}, CITY);

    vec3 player1_origin = generate_random_planet_position();
    g_players.units[0].position[0] = player1_origin;
    units_update_aabb(&g_players.units[0], 0);
    vec3 player2_origin = generate_random_planet_position();
    g_players.units[1].position[0] = player2_origin;
    units_update_aabb(&g_players.units[1], 0);
    vec3 player3_origin = generate_random_planet_position();
    g_players.units[2].position[0] = player3_origin;
    units_update_aabb(&g_players.units[2], 0);
    vec3 player4_origin = generate_random_planet_position();
    g_players.units[3].position[0] = player4_origin;
    units_update_aabb(&g_players.units[3], 0);
    vec3 player5_origin = generate_random_planet_position();
    g_players.units[4].position[0] = player5_origin;
    units_update_aabb(&g_players.units[4], 0);
    vec3 player6_origin = generate_random_planet_position();
    g_players.units[5].position[0] = player6_origin;
    units_update_aabb(&g_players.units[5], 0);

    create_and_place_unit(player1_origin, 0, HUMVEE);
    create_and_place_unit(player1_origin, 0, ASTRONAUT);
    create_and_place_unit(player1_origin, 0, TANK);
    create_and_place_unit(player2_origin, 1, HUMVEE);
    create_and_place_unit(player2_origin, 1, ASTRONAUT);
    create_and_place_unit(player2_origin, 1, TANK);
    create_and_place_unit(player3_origin, 2, HUMVEE);
    create_and_place_unit(player3_origin, 2, ASTRONAUT);
    create_and_place_unit(player3_origin, 2, TANK);
    create_and_place_unit(player4_origin, 3, HUMVEE);
    create_and_place_unit(player4_origin, 3, ASTRONAUT);
    create_and_place_unit(player4_origin, 3, TANK);
    create_and_place_unit(player5_origin, 4, HUMVEE);
    create_and_place_unit(player5_origin, 4, ASTRONAUT);
    create_and_place_unit(player5_origin, 4, TANK);
    create_and_place_unit(player6_origin, 5, HUMVEE);
    create_and_place_unit(player6_origin, 5, ASTRONAUT);
    create_and_place_unit(player6_origin, 5, TANK);

    cstrl_render_data *city_render_data = cstrl_renderer_create_render_data();

    cstrl_renderer_add_positions(city_render_data, planet_positions, 3, SPHERE_RESOLUTION * SPHERE_RESOLUTION * 6);
    cstrl_renderer_add_indices(city_render_data, planet_indices,
                               (SPHERE_RESOLUTION - 1) * (SPHERE_RESOLUTION - 1) * 36);

    cstrl_shader city_shader =
        cstrl_load_shaders_from_files("resources/shaders/city_borders.vert", "resources/shaders/city_borders.frag");
    cstrl_texture city_noise_texture =
        cstrl_texture_generate_from_path("resources/textures/planet_game/noise_texture.png");

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
    float colors[144];
    for (int i = 0; i < 36; i++)
    {
        colors[i * 4] = 0.8f;
        colors[i * 4 + 1] = 0.6f;
        colors[i * 4 + 2] = 0.8f;
        colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(skybox_render_data, positions, 3, 36);
    cstrl_renderer_add_colors(skybox_render_data, colors);

    cstrl_shader skybox_shader =
        cstrl_load_shaders_from_files("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
    cstrl_texture skybox_texture =
        cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/stars/", true);

    cstrl_da_float_init(&g_physics_ray_positions, 3);
    cstrl_render_data *physics_debug_render_data = cstrl_renderer_create_render_data();
    da_float physics_debug_positions;
    cstrl_da_float_init(&physics_debug_positions, 36);
    fill_physics_positions(&physics_debug_positions, &g_physics_ray_positions);
    cstrl_renderer_add_positions(physics_debug_render_data, physics_debug_positions.array, 3,
                                 physics_debug_positions.size / 3);

    cstrl_shader physics_debug_shader =
        cstrl_load_shaders_from_files("resources/shaders/line3D.vert", "resources/shaders/line3D.frag");

    cstrl_render_data *unit_lazer_render_data = cstrl_renderer_create_render_data();
    da_float unit_lazer_positions;
    cstrl_da_float_init(&unit_lazer_positions, 6);
    da_float unit_lazer_start_positions;
    cstrl_da_float_init(&unit_lazer_start_positions, 3);
    cstrl_renderer_add_positions(unit_lazer_render_data, unit_lazer_positions.array, 3, unit_lazer_positions.size / 3);

    cstrl_shader unit_lazer_shader =
        cstrl_load_shaders_from_files("resources/shaders/line3D.vert", "resources/shaders/line3D.frag");

    cstrl_render_data *bullet_render_data = cstrl_renderer_create_render_data();
    da_float bullet_positions;
    cstrl_da_float_init(&bullet_positions, 12);
    da_int bullet_indices;
    cstrl_da_int_init(&bullet_indices, 6);
    da_float bullet_uvs;
    cstrl_da_float_init(&bullet_uvs, 8);
    add_billboard_object(
        &bullet_positions, &bullet_indices, &bullet_uvs, NULL,
        (transform){(vec3){0.0f, 0.0f, 0.0f}, (quat){1.0f, 0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}},
        (vec4){0.0f, 0.0f, 1.0f, 1.0f}, (vec4){});
    cstrl_renderer_add_positions(bullet_render_data, bullet_positions.array, 3, bullet_positions.size / 3);
    cstrl_renderer_add_indices(bullet_render_data, bullet_indices.array, 6);
    cstrl_renderer_add_uvs(bullet_render_data, bullet_uvs.array);

    cstrl_shader bullet_shader = cstrl_load_shaders_from_files("resources/shaders/default3D_no_color.vert",
                                                               "resources/shaders/default3D_no_color.frag");
    cstrl_texture bullet_texture = cstrl_texture_generate_from_path("resources/textures/planet_game/bullet.png");

    cstrl_render_data *explosion_render_data = cstrl_renderer_create_render_data();
    da_float explosion_positions;
    cstrl_da_float_init(&explosion_positions, 12);
    da_int explosion_indices;
    cstrl_da_int_init(&explosion_indices, 6);
    da_float explosion_uvs;
    cstrl_da_float_init(&explosion_uvs, 8);
    add_billboard_object(
        &explosion_positions, &explosion_indices, &explosion_uvs, NULL,
        (transform){(vec3){0.0f, 0.0f, 0.0f}, (quat){1.0f, 0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}},
        (vec4){0.0f, 0.0f, 1.0f / 20.0f, 1.0f / 20.0f}, (vec4){});
    cstrl_renderer_add_positions(explosion_render_data, explosion_positions.array, 3, explosion_positions.size / 3);
    cstrl_renderer_add_indices(explosion_render_data, explosion_indices.array, 6);
    cstrl_renderer_add_uvs(explosion_render_data, explosion_uvs.array);

    cstrl_shader explosion_shader = cstrl_load_shaders_from_files("resources/shaders/default3D_no_color.vert",
                                                                  "resources/shaders/default3D_no_color.frag");
    cstrl_texture explosion_texture = cstrl_texture_generate_from_path("resources/textures/planet_game/explosion2.png");

    g_main_camera = cstrl_camera_create(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, false);
    g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(player1_origin));
    g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
    g_main_camera->position = cstrl_vec3_mult_scalar(g_main_camera->forward, -3.0f);
    cstrl_camera_update(g_main_camera, g_movement, g_rotation);

    g_ui_camera = cstrl_camera_create(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, true);
    cstrl_camera_update(g_ui_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);

    cstrl_set_uniform_3f(planet_shader.program, "material.ambient", 0.3f, 0.3f, 0.3f);
    cstrl_set_uniform_3f(planet_shader.program, "material.diffuse", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(planet_shader.program, "material.specular", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_float(planet_shader.program, "material.shininess", 2.0f);
    cstrl_set_uniform_3f(planet_shader.program, "light.ambient", 0.3f, 0.3f, 0.3f);
    cstrl_set_uniform_3f(planet_shader.program, "light.diffuse", 0.8f, 0.8f, 0.8f);
    cstrl_set_uniform_3f(planet_shader.program, "light.specular", 0.8f, 0.8f, 0.8f);

    cstrl_set_uniform_block_binding(planet_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(city_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(path_marker_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(path_line_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(path_line_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(unit_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(physics_debug_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(explosion_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(skybox_shader, "Matrices", 0);

    int view_projection_ubo = cstrl_renderer_add_ubo(2 * sizeof(mat4));
    cstrl_renderer_update_ubo(view_projection_ubo, g_main_camera->view.m, sizeof(mat4), 0);
    cstrl_renderer_update_ubo(view_projection_ubo, g_main_camera->projection.m, sizeof(mat4), sizeof(mat4));

    cstrl_set_uniform_mat4(selection_box_shader.program, "view", g_ui_camera->view);
    cstrl_set_uniform_mat4(selection_box_shader.program, "projection", g_ui_camera->projection);

    double previous_time = cstrl_platform_get_absolute_time();
    double lag = 0.0;
    float light_start_x = 0.0f;
    float light_start_z = 0.0f;
    g_ui_context = cstrl_ui_init(&g_platform_state);
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
            cstrl_renderer_clear_render_attributes(selection_box_render_data);
            if (g_making_ground_selection || g_making_air_selection)
            {
                update_ui_object(&selection_box_positions, &selection_box_indices, NULL, &selection_box_colors, 0,
                                 g_selection_start, g_selection_end, (vec4){1.0f, 0.0f, 0.0f, 0.1f});
                cstrl_renderer_modify_render_attributes(selection_box_render_data, selection_box_positions.array, NULL,
                                                        selection_box_colors.array, selection_box_positions.size / 2);
                cstrl_renderer_modify_indices(selection_box_render_data, selection_box_indices.array, 0,
                                              selection_box_indices.size);
            }
            // TODO: consider not clearing data on each run
            cstrl_da_float_clear(&unit_positions);
            cstrl_da_int_clear(&unit_indices);
            cstrl_da_float_clear(&unit_uvs);
            cstrl_da_float_clear(&unit_colors);
            cstrl_renderer_clear_render_attributes(unit_render_data);

            cstrl_da_float_clear(&explosion_positions);
            cstrl_da_int_clear(&explosion_indices);
            cstrl_da_float_clear(&explosion_uvs);
            cstrl_renderer_clear_render_attributes(explosion_render_data);

            cstrl_da_float_clear(&path_line_positions);
            cstrl_da_float_clear(&path_marker_positions);
            cstrl_da_float_clear(&path_marker_colors);
            cstrl_renderer_clear_render_attributes(path_line_render_data);
            cstrl_renderer_clear_render_attributes(path_marker_render_data);

            quat camera_rotation = cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view));
            quat billboard_rotation = cstrl_quat_inverse(camera_rotation);
            ai_update(&ai, &g_players, g_human_player);
            int unit_render_index = 0;
            int projectile_render_index = 0;
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                players_update(&g_players, i);
                // update_formation_state(i);
                for (int j = 0; j < g_players.units[i].count; j++)
                {
                    if (!g_players.units[i].active[j])
                    {
                        continue;
                    }
                    if (g_players.units[i].attacking[j])
                    {
                        if (cstrl_platform_get_absolute_time() > g_players.units[i].last_attack_time[j] + 0.1)
                        {
                            g_players.units[i].last_attack_time[j] = cstrl_platform_get_absolute_time();
                            int index = cstrl_da_int_find_first(
                                &g_players.formations[i].unit_ids[g_players.units[i].formation_id[j]], j);
                            int path_id =
                                g_players.formations[i].path_heads[g_players.units[i].formation_id[j]].array[index];
                            vec3 direction = cstrl_vec3_normalize(cstrl_vec3_sub(
                                get_point_on_path((vec3){0.0f, 0.0f, 0.0f}, g_players.units[i].position[j],
                                                  g_players.paths[i].end_positions[path_id], 0.5f),
                                g_players.units[i].position[j]));
                            unit_data_t tracked_unit_data = g_players.paths[i].tracked_unit[path_id];
                            vec3 position =
                                g_players.units[tracked_unit_data.player_id].position[tracked_unit_data.unit_id];
                            position.x += (rand() % 100 - 50) / 5000.0f;
                            position.y += (rand() % 100 - 50) / 5000.0f;
                            position.z += (rand() % 100 - 50) / 5000.0f;
                            position =
                                cstrl_vec3_mult_scalar(cstrl_vec3_normalize(position), 1.0f + UNIT_SIZE_X * 0.6f);
                            projectiles_add(&g_players.projectiles[i], i, position, direction);
                            g_players.units[tracked_unit_data.player_id]
                                .stats[tracked_unit_data.unit_id]
                                .current_health -= 1.0f;
                            if (g_players.units[tracked_unit_data.player_id]
                                    .stats[tracked_unit_data.unit_id]
                                    .current_health <= 0.0f)
                            {
                                units_remove(&g_players.units[tracked_unit_data.player_id], tracked_unit_data.unit_id);
                                g_players.paths[i].tracked_unit[path_id].player_id = -1;
                                g_players.paths[i].tracked_unit[path_id].unit_id = -1;
                                g_players.units[i].attacking[j] = false;
                            }
                        }
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
                    vec3 size =
                        (type != ASTRONAUT && type != ASTRONAUT_ARMED ? UNIT_SIZE
                                                                      : cstrl_vec3_mult_scalar(UNIT_SIZE, 0.65f));
                    update_billboard_object(
                        &unit_positions, &unit_indices, &unit_uvs, &unit_colors, unit_render_index,
                        (transform){g_players.units[i].position[j], billboard_rotation, adjust_billboard_scale(size)},
                        uv_positions, color);
                    unit_render_index++;
                }
                for (int j = 0; j < g_players.projectiles[i].count; j++)
                {
                    if (!g_players.projectiles[i].active[j])
                    {
                        continue;
                    }
                    float increment = 1.0f / 20.0f;
                    // vec3 end_position = cstrl_vec3_normalize(
                    //     cstrl_vec3_add(g_players.projectiles[i].position[j],
                    //                    cstrl_vec3_mult_scalar(g_players.projectiles[i].direction[j], 0.01f)));
                    // quat shot_rotation = cstrl_quat_from_to(
                    //     cstrl_vec3_cross(g_main_camera->up,
                    //     cstrl_vec3_normalize(g_players.projectiles[i].position[j])),
                    //     g_players.projectiles[i].direction[j]);
                    // g_players.projectiles[i].position[j] = end_position;
                    if (cstrl_platform_get_absolute_time() > g_players.projectiles[i].transition_time[j] + 0.0)
                    {
                        g_players.projectiles[i].transition_time[j] = cstrl_platform_get_absolute_time();
                        g_players.projectiles[i].uvs[j].u0 += increment;
                        g_players.projectiles[i].uvs[j].u1 += increment;
                        if (g_players.projectiles[i].uvs[j].u1 >= 1.0f)
                        {
                            projectiles_remove(&g_players.projectiles[i], j);
                            continue;
                        }
                    }
                    update_billboard_object(&explosion_positions, &explosion_indices, &explosion_uvs, NULL,
                                            projectile_render_index++,
                                            (transform){g_players.projectiles[i].position[j], billboard_rotation,
                                                        (vec3){UNIT_SIZE_X * 0.4f, UNIT_SIZE_Y * 0.4f, 0.0f}},
                                            g_players.projectiles[i].uvs[j], (vec4){1.0f, 1.0f, 1.0f, 1.0f});
                }
            }
            if (explosion_positions.size > 0)
            {
                cstrl_renderer_modify_render_attributes(explosion_render_data, explosion_positions.array,
                                                        explosion_uvs.array, NULL, explosion_positions.size / 3);
                cstrl_renderer_modify_indices(explosion_render_data, explosion_indices.array, 0,
                                              explosion_indices.size);
            }
            else
            {
                cstrl_renderer_modify_positions(unit_lazer_render_data, (float[6]){0.0f}, 0, 6);
            }
            if (unit_positions.size > 0)
            {
                cstrl_renderer_modify_render_attributes(unit_render_data, unit_positions.array, unit_uvs.array,
                                                        unit_colors.array, unit_positions.size / 3);
                cstrl_renderer_modify_indices(unit_render_data, unit_indices.array, 0, unit_indices.size);
            }

            int human_selected_formation = g_players.selected_formation[g_human_player];
            if ((g_render_path_lines || g_render_path_markers) && human_selected_formation != -1)
            {
                int render_index = 0;
                for (int i = 0; i < g_players.formations[g_human_player].path_heads[human_selected_formation].size; i++)
                {
                    int path_id = g_players.formations[g_human_player].path_heads[human_selected_formation].array[i];
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
                                (vec4){},
                                !g_players.formations[g_human_player]
                                        .following_enemy[g_players.selected_formation[g_human_player]]
                                    ? PATH_MARKER_COLOR
                                    : PATH_MARKER_COLOR_ATTACK);
                            render_index++;
                        }
                        if (g_render_path_lines)
                        {
                            vec3 start = g_players.paths[g_human_player].start_positions[path_id];
                            if (!g_players.paths[g_human_player].in_queue[path_id])
                            {
                                start = g_players.units[g_human_player].position[g_players.formations[g_human_player]
                                                                                     .unit_ids[human_selected_formation]
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
            light_start_x += 0.001f;
            light_start_z += 0.001f;
            lag -= 1.0 / 60.0;
        }

        if (g_view_projection_update)
        {
            cstrl_renderer_update_ubo(view_projection_ubo, g_main_camera->view.m, sizeof(mat4), 0);
            cstrl_renderer_update_ubo(view_projection_ubo, g_main_camera->projection.m, sizeof(mat4), sizeof(mat4));
            cstrl_set_uniform_3f(planet_shader.program, "view_position", g_main_camera->position.x,
                                 g_main_camera->position.y, g_main_camera->position.z);
            g_view_projection_update = false;
        }
        if (g_ui_view_projection_update)
        {
            cstrl_set_uniform_mat4(selection_box_shader.program, "view", g_ui_camera->view);
            cstrl_set_uniform_mat4(selection_box_shader.program, "projection", g_ui_camera->projection);
        }
        cstrl_renderer_clear(0.1f, 0.0f, 0.1f, 1.0f);
        vec3 light_position = {10.0f * cosf(light_start_x), 1.0f, 10.0f * sinf(light_start_z)};
        cstrl_set_uniform_3f(planet_shader.program, "light.position", light_position.x, light_position.y,
                             light_position.z);
        cstrl_use_shader(planet_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_cube_map_bind(planet_texture);
        cstrl_set_uniform_int(planet_shader.program, "texture0", 0);
        cstrl_set_active_texture(1);
        cstrl_texture_cube_map_bind(planet_normal);
        cstrl_set_uniform_int(planet_shader.program, "normal0", 1);
        cstrl_renderer_draw_indices(planet_render_data);

        if (g_border_update)
        {
            int city_index = 0;
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                if (!g_players.active[i])
                {
                    continue;
                }
                for (int j = 0; j < g_players.units[i].count; j++)
                {
                    if (g_players.units[i].type[j] != CITY)
                    {
                        continue;
                    }
                    char buffer[30];
                    sprintf(buffer, "city_centers[%d]", city_index);
                    cstrl_set_uniform_3f(city_shader.program, buffer, g_players.units[i].position[j].x,
                                         g_players.units[i].position[j].y, g_players.units[i].position[j].z);
                    sprintf(buffer, "team[%d]", city_index);
                    cstrl_set_uniform_int_array(city_shader.program, buffer, 1, &i);
                    sprintf(buffer, "weights[%d]", city_index);
                    cstrl_set_uniform_float(city_shader.program, buffer, 1.0f);
                    sprintf(buffer, "influence_strength[%d]", city_index);
                    cstrl_set_uniform_float(city_shader.program, buffer, 1.0f);
                    sprintf(buffer, "influence_radius[%d]", city_index++);
                    cstrl_set_uniform_float(city_shader.program, buffer, 0.4f);
                }
            }
            cstrl_set_uniform_int(city_shader.program, "cities_count", city_index);
            g_border_update = false;
        }
        cstrl_use_shader(city_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(city_noise_texture);
        cstrl_renderer_draw_indices(city_render_data);

        if (g_render_path_markers)
        {
            cstrl_use_shader(path_marker_shader);
            cstrl_renderer_draw_indices(path_marker_render_data);
        }
        if (g_render_path_lines)
        {
            if (!g_players.formations[g_human_player].following_enemy[g_players.selected_formation[g_human_player]])
            {
                cstrl_set_uniform_4f(path_line_shader.program, "color", PATH_MARKER_COLOR.r, PATH_MARKER_COLOR.g,
                                     PATH_MARKER_COLOR.b, PATH_MARKER_COLOR.a);
            }
            else
            {
                cstrl_set_uniform_4f(path_line_shader.program, "color", PATH_MARKER_COLOR_ATTACK.r,
                                     PATH_MARKER_COLOR_ATTACK.g, PATH_MARKER_COLOR_ATTACK.b,
                                     PATH_MARKER_COLOR_ATTACK.a);
            }
            cstrl_use_shader(path_line_shader);
            cstrl_renderer_draw_lines(path_line_render_data);
        }

        cstrl_use_shader(unit_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(unit_texture);
        cstrl_renderer_draw_indices(unit_render_data);

        cstrl_use_shader(selection_box_shader);
        cstrl_renderer_draw_indices(selection_box_render_data);

        if (g_physics_debug_draw_enabled)
        {
            cstrl_da_float_clear(&physics_debug_positions);
            fill_physics_positions(&physics_debug_positions, &g_physics_ray_positions);
            cstrl_renderer_modify_render_attributes(physics_debug_render_data, physics_debug_positions.array, NULL,
                                                    NULL, physics_debug_positions.size / 3);
            cstrl_set_uniform_4f(physics_debug_shader.program, "color", PATH_MARKER_COLOR.r, PATH_MARKER_COLOR.g,
                                 PATH_MARKER_COLOR.b, PATH_MARKER_COLOR.a);
            cstrl_use_shader(physics_debug_shader);
            cstrl_renderer_draw_lines(physics_debug_render_data);
        }

        cstrl_use_shader(explosion_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(explosion_texture);
        cstrl_renderer_draw_indices(explosion_render_data);

        cstrl_use_shader(skybox_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_cube_map_bind(skybox_texture);
        cstrl_renderer_draw(skybox_render_data);

        /*
        cstrl_ui_begin(g_ui_context);
        if (cstrl_ui_container_begin(g_ui_context, "Economy", 7, 10, 10, 200, 300, GEN_ID(0), false, false, 2))
        {
            static double monies = 10666;
            char buffer[30];
            sprintf(buffer, "w$ %.2lf B", monies);
            monies += (rand() % 90 + 10) * 0.01;
            if (cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 50, 300, 50, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT))
            {
            }
            static double oil = 10.0;
            sprintf(buffer, "Oil: %.2lf ", oil);
            oil += (rand() % 100) / 1000.0;
            if (cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 100, 300, 50, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT))
            {
            }
            static double oxygen = 10000.0;
            sprintf(buffer, "O2: %.2lf ", oxygen);
            oxygen += (rand() % 1000) / 1000.0;
            if (cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 150, 300, 50, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT))
            {
            }
            static double carbon_dioxide = 10000.0;
            sprintf(buffer, "CO2: %.2lf ", carbon_dioxide);
            carbon_dioxide += (rand() % 1000) / 1000.0;
            if (cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 200, 300, 50, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT))
            {
            }
            static double power = 30.0;
            sprintf(buffer, "Power: %.2lf GW", power);
            power += (rand() % 5 - 2) / 10.0;
            if (cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 250, 300, 50, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT))
            {
            }
            cstrl_ui_container_end(g_ui_context);
        }
        if (g_players.selected_units[g_human_player].size > 0 &&
            g_players.units[g_human_player].type[g_players.selected_units[g_human_player].array[0]] == CITY)
        {
            if (cstrl_ui_container_begin(g_ui_context, "City", 4, g_mouse_position_x, g_mouse_position_y, 100, 100,
                                         GEN_ID(0), false, false, 1))
            {
                static double monies = -100;
                char buffer[30];
                sprintf(buffer, "w$ %.2lf M", monies);
                monies += (rand() % 9 + 1) * 0.00005;
                cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 40, 300, 25, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT);
                static int population = 1000;
                sprintf(buffer, "Population: %d", population);
                cstrl_ui_text(g_ui_context, buffer, strlen(buffer), 0, 50, 300, 25, GEN_ID(0),
                              CSTRL_UI_TEXT_ALIGN_LEFT);
                cstrl_ui_container_end(g_ui_context);
            }
        }
        cstrl_ui_end(g_ui_context);
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

    cstrl_da_float_free(&g_physics_ray_positions);
    cstrl_da_float_free(&physics_debug_positions);

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
    cstrl_ui_shutdown(g_ui_context);
    cstrl_renderer_shutdown(&g_platform_state);
    cstrl_platform_shutdown(&g_platform_state);

    return 0;
}

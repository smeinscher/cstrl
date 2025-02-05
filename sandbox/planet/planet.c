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
#include "game/sphere.h"
#include "game/units.h"
#include "helpers/helpers.h"
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PLANET_LATITUDE_POINTS 18
#define PLANET_LONGITUDE_POINTS 36

#define PLANET_LATITUDE_MAX_COS 0.9f

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
    bool active;
    float *score;
    da_int free_ids;
} players_t;

static units_t g_units;
static formations_t g_formations;
static int g_human_selected_formation = -1;
static paths_t g_paths;
static players_t g_players;

static vec3 g_planet_position = (vec3){0.0f, 0.0f, 0.0f};

static const vec3 PLANET_SIZE = {2.0f, 2.0f, 2.0f};
static const vec3 PATH_MARKER_SIZE = {0.02f, 0.02f, 0.0f};

static const vec4 PATH_MARKER_COLOR = {0.8f, 0.8f, 0.8f, 0.9f};

static const vec4 UNIT_TEAM_COLORS[] = {
    (vec4){1.0f, 0.0f, 0.0f, 0.4f}, (vec4){0.0f, 1.0f, 0.0f, 0.4f}, (vec4){0.0f, 0.0f, 1.0f, 0.4f},
    (vec4){1.0f, 1.0f, 0.0f, 0.4f}, (vec4){1.0f, 0.0f, 1.0f, 0.4f}, (vec4){0.0f, 1.0f, 1.0f, 0.4f},
    (vec4){1.0f, 1.0f, 1.0f, 0.4f}, (vec4){0.3f, 0.3f, 0.3f, 0.4f},
};

static const float FORMATION_OFFSETS[] = {0.0f, 1.0f, -1.0f, 2.0f, -2.0f};

static bool g_render_path_markers = true;
static bool g_render_path_lines = true;

static team_t g_human_team = RED;

static bool g_making_selection = false;
static vec2 g_selection_start;
static vec2 g_selection_end;
static da_int g_selected_units;

static void set_units_in_formation_selected()
{
    if (g_selected_units.size == 0)
    {
        return;
    }
    int formation_id = g_units.formation_id[g_selected_units.array[0]];
    if (formation_id == -1)
    {
        return;
    }
    cstrl_da_int_clear(&g_selected_units);
    for (int i = 0; i < g_formations.unit_ids[formation_id].size; i++)
    {
        cstrl_da_int_push_back(&g_selected_units, g_formations.unit_ids[formation_id].array[i]);
    }
}

static void add_selected_units_to_formation()
{
    g_human_selected_formation = formations_add(&g_formations);
    if (g_human_selected_formation == -1)
    {
        return;
    }
    for (int i = 0; i < g_selected_units.size; i++)
    {
        int id = g_selected_units.array[i];
        int old_formation_id = g_units.formation_id[id];
        if (old_formation_id != -1)
        {
            int unit_index = cstrl_da_int_find_first(&g_formations.unit_ids[old_formation_id], id);
            int path_id = g_formations.path_heads[old_formation_id].array[unit_index];
            if (path_id != -1)
            {
                paths_recursive_remove(&g_paths, path_id);
            }
            formations_remove_unit(&g_formations, old_formation_id, id);
        }
        formations_add_unit(&g_formations, g_human_selected_formation, id);
        g_units.formation_id[id] = g_human_selected_formation;
    }
}

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

static void new_path(vec3 start_position, vec3 end_position, bool in_queue, int unit_id, int prev)
{
    int new_path_id = paths_add(&g_paths, start_position, end_position, prev);
    if (new_path_id != -1)
    {
        int unit_index = cstrl_da_int_find_first(&g_formations.unit_ids[g_human_selected_formation], unit_id);
        if (unit_index > 0)
        {
            vec3 path_vector = cstrl_vec3_sub(
                end_position, g_units.position[g_formations.unit_ids[g_human_selected_formation].array[0]]);
            path_vector = cstrl_vec3_normalize(path_vector);
            vec3 formation_line =
                cstrl_vec3_normalize(cstrl_vec3_cross(path_vector, cstrl_vec3_normalize(end_position)));
            vec3 new_end_position = cstrl_vec3_add(
                end_position, cstrl_vec3_mult_scalar(formation_line, FORMATION_OFFSETS[unit_index % 5] * UNIT_SIZE.x));
            new_end_position = cstrl_vec3_sub(
                new_end_position, cstrl_vec3_mult_scalar(path_vector, floorf(unit_index / 5.0f) * UNIT_SIZE.y * 1.5f));
            new_end_position =
                cstrl_vec3_mult_scalar(cstrl_vec3_normalize(new_end_position), 1.0f + UNIT_SIZE.x * 0.5f);
            g_paths.end_positions[new_path_id] = new_end_position;
        }
        if (g_formations.path_heads[g_human_selected_formation].array[unit_index] != -1)
        {
            g_paths.in_queue[new_path_id] = true;
        }
        else
        {
            g_formations.path_heads[g_human_selected_formation].array[unit_index] = new_path_id;
            g_paths.in_queue[new_path_id] = false;
        }
    }
}

static void move_units_normal_mode(vec3 end_position)
{
    if (g_human_selected_formation == -1)
    {
        add_selected_units_to_formation();
    }
    for (int i = 0; i < g_formations.unit_ids[g_human_selected_formation].size; i++)
    {
        int unit_id = g_formations.unit_ids[g_human_selected_formation].array[i];
        vec3 start_position = g_units.position[unit_id];
        int head_path_id = g_formations.path_heads[g_human_selected_formation].array[i];
        if (head_path_id != -1)
        {
            paths_recursive_remove(&g_paths, head_path_id);
            g_formations.path_heads[g_human_selected_formation].array[i] = -1;
        }
        new_path(start_position, end_position, false, unit_id, -1);
    }
}

static void move_units_path_mode(vec3 end_position)
{
    if (g_human_selected_formation == -1)
    {
        add_selected_units_to_formation();
    }
    for (int i = 0; i < g_formations.unit_ids[g_human_selected_formation].size; i++)
    {
        int unit_id = g_formations.unit_ids[g_human_selected_formation].array[i];
        vec3 start_position;
        bool path_in_queue = false;
        int prev_path = -1;
        int path_id = g_formations.path_heads[g_human_selected_formation].array[i];
        if (path_id == -1)
        {
            start_position = g_units.position[unit_id];
        }
        else
        {
            while (g_paths.next[path_id] != -1)
            {
                path_id = g_paths.next[path_id];
            }
            prev_path = path_id;
            start_position = g_paths.end_positions[path_id];
            path_in_queue = true;
        }
        new_path(start_position, end_position, path_in_queue, unit_id, prev_path);
    }
}

static void move_units_to_cursor_position(bool path_mode)
{
    if (g_selected_units.size == 0)
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
            move_units_path_mode(end_position);
        }
        else
        {
            move_units_normal_mode(end_position);
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

static void select_units()
{
    int width, height;
    cstrl_platform_get_window_size(&g_platform_state, &width, &height);
    vec2 min =
        (vec2){cstrl_min(g_selection_start.x, g_selection_end.x), cstrl_min(g_selection_start.y, g_selection_end.y)};
    min.x /= width / 2.0f;
    min.y /= height / 2.0f;
    vec2 max =
        (vec2){cstrl_max(g_selection_start.x, g_selection_end.x), cstrl_max(g_selection_start.y, g_selection_end.y)};
    max.x /= width / 2.0f;
    max.y /= height / 2.0f;
    g_human_selected_formation = -1;
    int formation_state = -1;
    quat unit_rotation = cstrl_quat_inverse(cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view)));
    cstrl_da_int_clear(&g_selected_units);
    for (int i = 0; i < g_units.count; i++)
    {
        float dot =
            cstrl_vec3_dot(cstrl_vec3_normalize(g_units.position[i]), cstrl_vec3_normalize(g_main_camera->position));
        if (dot < 0.05f)
        {
            continue;
        }
        vec3 p0, p1, p2, p3;
        get_points(&p0, &p1, &p2, &p3, (transform){g_units.position[i], unit_rotation, UNIT_SIZE});
        vec2 p0_screen = world_to_screen(p0, (vec2){2.0f, 2.0f}, g_main_camera->projection, g_main_camera->view);
        vec2 p2_screen = world_to_screen(p2, (vec2){2.0f, 2.0f}, g_main_camera->projection, g_main_camera->view);
        vec2 p_min = (vec2){cstrl_min(p0_screen.x, p2_screen.x), cstrl_min(p0_screen.y, p2_screen.y)};
        vec2 p_max = (vec2){cstrl_max(p0_screen.x, p2_screen.x), cstrl_max(p0_screen.y, p2_screen.y)};
        if (max.x >= p_min.x && min.x <= p_max.x && max.y >= p_min.y && min.y <= p_max.y)
        {
            cstrl_da_int_push_back(&g_selected_units, i);
            if (g_units.formation_id[i] != formation_state)
            {
                if (formation_state == -1)
                {
                    formation_state = g_units.formation_id[i];
                }
                else
                {
                    formation_state = -2;
                }
            }
        }
    }
    if (formation_state > -1)
    {
        g_human_selected_formation = formation_state;
        // set_units_in_formation_selected();
    }
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
    case CSTRL_KEY_2:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_main_camera->position = (vec3){-3.0f, 0.0f, 0.0f};
            g_main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(g_main_camera->position));
            g_main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(g_main_camera->forward, g_main_camera->up));
            cstrl_camera_update(g_main_camera, g_movement, g_rotation);
            for (int i = 0; i < 4; i++)
            {
                printf("%f, %f, %f, %f\n", g_main_camera->view.m[i * 4], g_main_camera->view.m[i * 4 + 1],
                       g_main_camera->view.m[i * 4 + 2], g_main_camera->view.m[i * 4 + 3]);
            }
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
        }
        break;
    case CSTRL_KEY_3:
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
    case CSTRL_KEY_4:
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
    case CSTRL_KEY_1:
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
    case CSTRL_KEY_5:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_render_path_markers = !g_render_path_markers;
            g_render_path_lines = g_render_path_markers;
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
        break;
    case CSTRL_KEY_F:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d = mouse_cursor_ray_cast();
            float t;
            if (hit_check(d, &t, g_main_camera->position, g_planet_position, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                vec3 position = position_from_ray_cast(d, t);
                units_add(&g_units, position, g_human_team);
            }
        }
        break;
    case CSTRL_KEY_C:
        if (action == CSTRL_ACTION_PRESS)
        {
            printf("mouse position: %d, %d\n", g_mouse_position_x, g_mouse_position_y);
        }
        break;
    case CSTRL_KEY_6:
        if (action == CSTRL_ACTION_PRESS)
        {
            move_units_normal_mode((vec3){0.0f, 0.0f, -1.0f - UNIT_SIZE.x / 2.0f});
        }
        break;
    case CSTRL_KEY_T:
        if (action == CSTRL_ACTION_PRESS)
        {
            set_units_in_formation_selected();
        }
        break;
    case CSTRL_KEY_B:
        if (action == CSTRL_ACTION_PRESS)
        {
            add_selected_units_to_formation();
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
                g_human_selected_formation = -1;
                cstrl_da_int_clear(&g_selected_units);
            }
            else
            {
                g_moving_planet = true;
            }
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            if (g_making_selection)
            {
                select_units();
                g_making_selection = false;
            }
            g_moving_planet = false;
            g_last_x = -1;
            g_last_y = -1;
        }
    }
    if (button == CSTRL_MOUSE_BUTTON_RIGHT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            move_units_to_cursor_position(mods & CSTRL_KEY_MOD_CONTROL);
        }
    }
}

static void add_billboard_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors,
                                 transform transform, vec4 color)
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

static void update_billboard_object(da_float *positions, da_int *indices, da_float *uvs, da_float *colors, size_t index,
                                    transform transform, vec4 new_color)
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
        add_billboard_object(positions, indices, uvs, colors, transform, new_color);
    }
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

void update_formation_state()
{
    for (int i = 0; i < g_formations.count; i++)
    {
        for (int j = 0; j < g_formations.unit_ids[i].size; j++)
        {
            int path_id = g_formations.path_heads[i].array[j];
            if (path_id == -1)
            {
                continue;
            }
            int unit_id = g_formations.unit_ids[i].array[j];
            path_update(&g_paths, path_id);
            if (fabsf(cstrl_vec3_length(cstrl_vec3_sub(g_paths.end_positions[path_id], g_units.position[unit_id]))) <
                UNIT_SIZE.x * 0.5f)
            {
                g_paths.render[path_id] = false;
            }
            if (g_paths.completed[path_id] || !g_paths.active[path_id])
            {
                g_formations.path_heads[i].array[j] = g_paths.next[path_id];
                if (g_formations.path_heads[i].array[j] != -1)
                {
                    g_paths.in_queue[g_formations.path_heads[i].array[j]] = false;
                }
                paths_remove(&g_paths, path_id);
                continue;
            }
            vec3 start_position = g_paths.start_positions[path_id];
            vec3 end_position = g_paths.end_positions[path_id];
            g_units.position[unit_id] = get_point_on_path(start_position, end_position, g_paths.progress[path_id]);
        }
    }
}

int planet_game()
{
    if (!cstrl_platform_init(&g_platform_state, "Planet", 560, 240, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        cstrl_platform_shutdown(&g_platform_state);
        return 1;
    }
    cstrl_platform_set_key_callback(&g_platform_state, key_callback);
    cstrl_platform_set_mouse_position_callback(&g_platform_state, mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(&g_platform_state, mouse_button_callback);
    cstrl_renderer_init(&g_platform_state);

    cstrl_render_data *planet_render_data = cstrl_renderer_create_render_data();
    float planet_positions[(PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1) * 3];
    float planet_normals[(PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1) * 3];
    float planet_uvs[(PLANET_LATITUDE_POINTS + 1) * (PLANET_LONGITUDE_POINTS + 1) * 2];
    int planet_indices[(PLANET_LATITUDE_POINTS + 1) * PLANET_LONGITUDE_POINTS * 6];

    generate_sphere(planet_positions, planet_indices, planet_uvs, planet_normals, PLANET_LATITUDE_POINTS,
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
    cstrl_shader planet_shader =
        cstrl_load_shaders_from_files("resources/shaders/planet.vert", "resources/shaders/planet.frag");

    cstrl_texture planet_texture = cstrl_texture_generate_from_path("resources/textures/moon1024.bmp");

    units_init(&g_units);
    formations_init(&g_formations);
    paths_init(&g_paths);

    units_add(&g_units, (vec3){0.0f, 0.0f, 1.0f + UNIT_SIZE.x * 0.5f}, g_human_team);

    cstrl_render_data *unit_render_data = cstrl_renderer_create_render_data();
    da_float unit_positions;
    cstrl_da_float_init(&unit_positions, 12);
    da_int unit_indices;
    cstrl_da_int_init(&unit_indices, 6);
    da_float unit_uvs;
    cstrl_da_float_init(&unit_uvs, 8);
    da_float unit_colors;
    cstrl_da_float_init(&unit_colors, 16);
    add_billboard_object(&unit_positions, &unit_indices, &unit_uvs, &unit_colors,
                         (transform){g_units.position[0], (quat){1.0f, 0.0f, 0.0f, 0.0f}, UNIT_SIZE},
                         (vec4){0.0f, 1.0f, 0.0f, 1.0f});
    cstrl_renderer_add_positions(unit_render_data, unit_positions.array, 3, 4);
    cstrl_renderer_add_indices(unit_render_data, unit_indices.array, 6);
    cstrl_renderer_add_uvs(unit_render_data, unit_uvs.array);
    cstrl_renderer_add_colors(unit_render_data, unit_colors.array);

    cstrl_shader unit_shader =
        cstrl_load_shaders_from_files("resources/shaders/default3D.vert", "resources/shaders/default3D.frag");
    cstrl_texture unit_texture = cstrl_texture_generate_from_path("resources/textures/tank.png");

    cstrl_render_data *path_marker_render_data = cstrl_renderer_create_render_data();
    da_float path_marker_positions;
    cstrl_da_float_init(&path_marker_positions, 12);
    da_int path_marker_indices;
    cstrl_da_int_init(&path_marker_indices, 6);
    da_float path_marker_colors;
    cstrl_da_float_init(&path_marker_colors, 16);
    add_billboard_object(&path_marker_positions, &path_marker_indices, NULL, &path_marker_colors,
                         (transform){(vec3){0.0f, 0.0f, 0.0f}, (quat){1.0f, 0.0f, 0.0f, 0.0f}, PATH_MARKER_SIZE},
                         PATH_MARKER_COLOR);
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

    cstrl_da_int_init(&g_selected_units, 1);

    g_main_camera = cstrl_camera_create(WINDOW_WIDTH, WINDOW_HEIGHT, false);
    g_main_camera->position.z = 3.0f;

    cstrl_camera *ui_camera = cstrl_camera_create(WINDOW_WIDTH, WINDOW_HEIGHT, true);
    cstrl_camera_update(ui_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);

    cstrl_set_uniform_3f(planet_shader.program, "material.ambient", 0.4f, 0.2f, 0.8f);
    cstrl_set_uniform_3f(planet_shader.program, "material.diffuse", 1.0f, 0.5f, 0.31f);
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
            // TODO: consider not clearing data on each run
            cstrl_da_float_clear(&unit_positions);
            cstrl_da_int_clear(&unit_indices);
            cstrl_da_float_clear(&unit_uvs);
            cstrl_da_float_clear(&unit_colors);
            cstrl_renderer_clear_render_attributes(unit_render_data);
            quat billboard_rotation =
                cstrl_quat_inverse(cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view)));
            update_formation_state();
            int unit_render_index = 0;
            for (int i = 0; i < g_units.count; i++)
            {
                if (!g_units.active[i])
                {
                    continue;
                }
                vec4 color = UNIT_TEAM_COLORS[g_units.team[i]];
                if (cstrl_da_int_find_first(&g_selected_units, i) != -1)
                {
                    color.a = 1.0f;
                }
                update_billboard_object(&unit_positions, &unit_indices, &unit_uvs, &unit_colors, unit_render_index,
                                        (transform){g_units.position[i], billboard_rotation, UNIT_SIZE}, color);
                unit_render_index++;
            }
            if (g_units.count > 0)
            {
                cstrl_renderer_modify_render_attributes(unit_render_data, unit_positions.array, unit_uvs.array,
                                                        unit_colors.array, unit_positions.size / 3);
                cstrl_renderer_modify_indices(unit_render_data, unit_indices.array, 0, unit_indices.size);
            }

            cstrl_da_float_clear(&path_line_positions);
            cstrl_da_float_clear(&path_marker_positions);
            cstrl_renderer_clear_render_attributes(path_line_render_data);
            cstrl_renderer_clear_render_attributes(path_marker_render_data);
            if ((g_render_path_lines || g_render_path_markers) && g_formations.count > 0 &&
                g_human_selected_formation != -1)
            {
                int render_index = 0;
                for (int i = 0; i < g_formations.path_heads[g_human_selected_formation].size; i++)
                {
                    int path_id = g_formations.path_heads[g_human_selected_formation].array[i];
                    while (path_id != -1)
                    {
                        if (g_render_path_markers)
                        {
                            if (!g_paths.render[path_id] || !g_paths.active[path_id] || g_paths.completed[path_id])
                            {
                                path_id = g_paths.next[path_id];
                                continue;
                            }
                            update_billboard_object(
                                &path_marker_positions, &path_marker_indices, NULL, &path_marker_colors, render_index,
                                (transform){g_paths.end_positions[path_id], billboard_rotation, PATH_MARKER_SIZE},
                                PATH_MARKER_COLOR);
                            render_index++;
                        }
                        if (g_render_path_lines)
                        {
                            vec3 start = g_paths.start_positions[path_id];
                            if (!g_paths.in_queue[path_id])
                            {
                                start = g_units.position[g_formations.unit_ids[g_human_selected_formation].array[i]];
                            }
                            generate_line_segments(&path_line_positions, start, g_paths.end_positions[path_id]);
                        }
                        path_id = g_paths.next[path_id];
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
        vec3 light_position = {5.0f * cosf(light_start_x), 1.0f, 5.0f * sinf(light_start_z)};
        cstrl_set_uniform_mat4(planet_shader.program, "view", g_main_camera->view);
        cstrl_set_uniform_mat4(planet_shader.program, "projection", g_main_camera->projection);
        cstrl_set_uniform_3f(planet_shader.program, "view_position", g_main_camera->position.x,
                             g_main_camera->position.y, g_main_camera->position.z);
        cstrl_set_uniform_3f(planet_shader.program, "light.position", light_position.x, light_position.y,
                             light_position.z);
        cstrl_use_shader(planet_shader);
        cstrl_set_active_texture(0);
        cstrl_texture_bind(planet_texture);
        cstrl_renderer_draw_indices(planet_render_data);

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

        cstrl_set_uniform_mat4(selection_box_shader.program, "view", ui_camera->view);
        cstrl_set_uniform_mat4(selection_box_shader.program, "projection", ui_camera->projection);
        cstrl_use_shader(selection_box_shader);
        cstrl_renderer_draw_indices(selection_box_render_data);

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

    units_free(&g_units);
    formations_free(&g_formations);
    paths_free(&g_paths);

    cstrl_da_float_free(&unit_positions);
    cstrl_da_float_free(&unit_uvs);
    cstrl_da_float_free(&unit_colors);
    cstrl_da_int_free(&unit_indices);

    cstrl_da_float_free(&path_marker_positions);
    cstrl_da_int_free(&path_marker_indices);
    cstrl_da_float_free(&path_marker_colors);

    cstrl_da_float_free(&selection_box_positions);
    cstrl_da_int_free(&selection_box_indices);
    cstrl_da_float_free(&selection_box_colors);

    cstrl_da_int_free(&g_selected_units);

    cstrl_camera_free(g_main_camera);
    cstrl_camera_free(ui_camera);
    cstrl_renderer_free_render_data(planet_render_data);
    cstrl_renderer_free_render_data(unit_render_data);
    cstrl_ui_shutdown(context);
    cstrl_renderer_shutdown(&g_platform_state);
    cstrl_platform_shutdown(&g_platform_state);

    return 0;
}

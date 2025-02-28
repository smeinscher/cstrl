#include "moon_game.h"
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

static const vec3 PLANET_POSITION = (vec3){0.0f, 0.0f, 0.0f};

static const vec3 PLANET_SIZE = {2.0f, 2.0f, 2.0f};
static const vec3 PATH_MARKER_SIZE = {0.01f, 0.01f, 0.0f};

static const vec4 PATH_MARKER_COLOR = {0.8f, 0.8f, 0.8f, 0.9f};
static const vec4 PATH_MARKER_COLOR_ATTACK = {0.8f, 0.2f, 0.2f, 0.9f};

typedef enum render_data_types
{
    RENDER_DATA_PLANET,
    RENDER_DATA_CITY,
    RENDER_DATA_UNIT,
    RENDER_DATA_PATH_MARKER,
    RENDER_DATA_PATH_LINE,
    RENDER_DATA_SELECTION_BOX,
    RENDER_DATA_SKYBOX,
    RENDER_DATA_EXPLOSION,
    MAX_RENDER_DATA_TYPES
} render_data_types;

typedef enum shader_types
{
    SHADER_PLANET,
    SHADER_CITY,
    SHADER_UNIT,
    SHADER_PATH_MARKER,
    SHADER_PATH_LINE,
    SHADER_SELECTION_BOX,
    SHADER_SKYBOX,
    SHADER_EXPLOSION,
    SHADER_FRAMEBUFFER,
    MAX_SHADER_TYPES
} shader_types;

typedef enum texture_types
{
    TEXTURE_PLANET,
    TEXTURE_PLANET_NORMALS,
    TEXTURE_CITY_NOISE,
    TEXTURE_UNIT,
    TEXTURE_SKYBOX,
    TEXTURE_EXPLOSION,
    TEXTURE_FRAMEBUFFER,
    MAX_TEXTURE_TYPES
} texture_types;

typedef enum selection_type
{
    SELECTION_CITY,
    SELECTION_GROUND,
    SELECTION_AIR
} selection_type;

typedef struct combo_state_t
{
    game_state_t *game_state;
    render_state_t *render_state;
} combo_state_t;

static vec3 position_from_ray_cast(vec3 d, float t, vec3 camera_position)
{
    vec3 position = cstrl_vec3_add(camera_position, cstrl_vec3_mult_scalar(d, t));
    position = cstrl_vec3_normalize(position);
    return position;
}

static vec3 mouse_cursor_ray_cast(int window_width, int window_height, int mouse_x, int mouse_y, cstrl_camera *camera)
{
    return screen_ray_cast((vec2){mouse_x, mouse_y}, (vec2){window_width, window_height}, camera->projection,
                           camera->view);
}

static void move_units_to_cursor_position(players_t players, int player_id, bool path_mode, cstrl_camera *camera,
                                          int mouse_x, int mouse_y)
{
    if (players.selected_units[player_id].size == 0)
    {
        return;
    }
    int type = players.units[player_id].type[players.selected_units[player_id].array[0]];
    if (type == CITY)
    {
        return;
    }
    vec3 d = mouse_cursor_ray_cast(camera->viewport.x, camera->viewport.y, mouse_x, mouse_y, camera);
    float t;
    bool ground_units = type == TANK || type == HUMVEE || type == ASTRONAUT || type == ASTRONAUT_ARMED;
    if (planet_hit_check(d, &t, camera->position, PLANET_POSITION,
                         (PLANET_SIZE.x * 0.5f + UNIT_SIZE_X * (ground_units ? 0.5f : 5.0f))))
    {
        vec3 end_position = position_from_ray_cast(d, t, camera->position);
        if (ground_units)
        {
            end_position = cstrl_vec3_mult_scalar(end_position, 1.0f + UNIT_SIZE_X * 0.5f);
        }
        else
        {
            end_position = cstrl_vec3_mult_scalar(end_position, 1.0f + UNIT_SIZE_X * 5.0f);
        }
        if (path_mode)
        {
            players_move_units_path_mode(&players, player_id, end_position);
        }
        else
        {
            players_move_units_normal_mode(&players, player_id, end_position);
        }
    }
}

static void framebuffer_callback(cstrl_platform_state *state, int width, int height)
{
    combo_state_t *combo_state = cstrl_platform_get_user_data(state);
    cstrl_camera *main_camera = combo_state->render_state->camera_objects.main_camera;
    cstrl_camera *ui_camera = combo_state->render_state->camera_objects.ui_camera;
    main_camera->viewport = (vec2i){width, height};
    cstrl_camera_update(main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    ui_camera->viewport = (vec2i){width, height};
    cstrl_camera_update(ui_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_renderer_set_viewport(0, 0, width, height);
    combo_state->render_state->toggles.main_view_projection_update = true;
    combo_state->render_state->toggles.ui_view_projection_update = true;
}

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    combo_state_t *combo_state = cstrl_platform_get_user_data(state);
    cstrl_camera *main_camera = combo_state->render_state->camera_objects.main_camera;
    players_t players = combo_state->game_state->player_data.players;
    int human_player = combo_state->game_state->player_data.human_player;
    int mouse_x = combo_state->game_state->mouse_data.position_x;
    int mouse_y = combo_state->game_state->mouse_data.position_y;
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
            main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(players.units[human_player].position[0]));
            main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(main_camera->forward, main_camera->up));
            main_camera->position = cstrl_vec3_mult_scalar(main_camera->forward, -3.0f);
            combo_state->render_state->toggles.main_view_projection_update = true;
        }
        break;
    case CSTRL_KEY_1:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d =
                mouse_cursor_ray_cast(main_camera->viewport.x, main_camera->viewport.y, mouse_x, mouse_y, main_camera);
            float t;
            if (planet_hit_check(d, &t, main_camera->position, PLANET_POSITION, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t, main_camera->position);
                    position = cstrl_vec3_mult_scalar(position, 1.0f + UNIT_SIZE_X * 0.5f);
                    units_add(&players.units[human_player], human_player, position, ASTRONAUT);
                }
            }
        }
        break;
    case CSTRL_KEY_2:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d =
                mouse_cursor_ray_cast(main_camera->viewport.x, main_camera->viewport.y, mouse_x, mouse_y, main_camera);
            float t;
            if (planet_hit_check(d, &t, main_camera->position, PLANET_POSITION, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t, main_camera->position);
                    position = cstrl_vec3_mult_scalar(position, 1.0f + UNIT_SIZE_X * 0.5f);
                    units_add(&players.units[human_player], human_player, position, ASTRONAUT_ARMED);
                }
            }
        }
        break;
    case CSTRL_KEY_3:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d =
                mouse_cursor_ray_cast(main_camera->viewport.x, main_camera->viewport.y, mouse_x, mouse_y, main_camera);
            float t;
            if (planet_hit_check(d, &t, main_camera->position, PLANET_POSITION, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t, main_camera->position);
                    position = cstrl_vec3_mult_scalar(position, 1.0f + UNIT_SIZE_X * 0.5f);
                    units_add(&players.units[human_player], human_player, position, HUMVEE);
                }
            }
        }
        break;
    case CSTRL_KEY_4:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d =
                mouse_cursor_ray_cast(main_camera->viewport.x, main_camera->viewport.y, mouse_x, mouse_y, main_camera);
            float t;
            if (planet_hit_check(d, &t, main_camera->position, PLANET_POSITION, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                ray_cast_result_t result = regular_ray_cast(main_camera->position, d, 5.0f, NULL);
                if (!result.hit)
                {
                    vec3 position = position_from_ray_cast(d, t, main_camera->position);
                    position = cstrl_vec3_mult_scalar(position, 1.0f + UNIT_SIZE_X * 0.5f);
                    units_add(&players.units[human_player], human_player, position, TANK);
                }
            }
        }
        break;
    case CSTRL_KEY_5:
        if (action == CSTRL_ACTION_PRESS)
        {
            combo_state->render_state->toggles.render_path_markers =
                !combo_state->render_state->toggles.render_path_markers;
            combo_state->render_state->toggles.render_path_lines =
                combo_state->render_state->toggles.render_path_markers;
        }
        break;
    case CSTRL_KEY_6:
        if (action == CSTRL_ACTION_PRESS)
        {
            combo_state->render_state->toggles.render_planet = !combo_state->render_state->toggles.render_planet;
        }
        break;
    case CSTRL_KEY_C:
        if (action == CSTRL_ACTION_PRESS)
        {
            vec3 d =
                mouse_cursor_ray_cast(main_camera->viewport.x, main_camera->viewport.y, mouse_x, mouse_y, main_camera);
            float t;
            if (planet_hit_check(d, &t, main_camera->position, PLANET_POSITION, (PLANET_SIZE.x + UNIT_SIZE.x) * 0.5f))
            {
                vec3 position = position_from_ray_cast(d, t, main_camera->position);
                int unit_id = -1;
                int player_id = -1;
                for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                {
                    unit_id = units_hit(&players.units[i], position);
                    if (unit_id != -1)
                    {
                        player_id = i;
                        break;
                    }
                }
                if (unit_id != -1)
                {
                    vec3 old_position = players.units[player_id].position[unit_id];
                    int old_type = players.units[player_id].type[unit_id];
                    units_remove(&players.units[player_id], unit_id);
                    player_id++;
                    if (player_id >= ACTIVE_PLAYERS)
                    {
                        player_id = 0;
                    }
                    units_add(&players.units[player_id], player_id, old_position, old_type);
                }
            }
            combo_state->render_state->toggles.border_update = true;
        }
        break;
    case CSTRL_KEY_T:
        if (action == CSTRL_ACTION_PRESS)
        {
            players_set_units_in_formation_selected(&players, human_player);
        }
        break;
    case CSTRL_KEY_B:
        if (action == CSTRL_ACTION_PRESS)
        {
            players_add_selected_units_to_formation(&players, human_player);
        }
        break;
    case CSTRL_KEY_D:
        if (action == CSTRL_ACTION_PRESS)
        {
            // g_physics_debug_draw_enabled = !g_physics_debug_draw_enabled;
        }
        break;
    case CSTRL_KEY_S:
        if (action == CSTRL_ACTION_PRESS)
        {
            players.selected_formation[human_player] = -1;
            human_player = (human_player + 1) % 6;
            players.selected_formation[human_player] = -1;
            main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(players.units[human_player].position[0]));
            main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(main_camera->forward, main_camera->up));
            main_camera->position = cstrl_vec3_mult_scalar(main_camera->forward, -3.0f);
            combo_state->render_state->toggles.main_view_projection_update = true;
        }
        break;
    default:
        break;
    }
}

static void mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    combo_state_t *combo_state = cstrl_platform_get_user_data(state);
    cstrl_camera *main_camera = combo_state->render_state->camera_objects.main_camera;

    combo_state->game_state->mouse_data.last_position_x = combo_state->game_state->mouse_data.position_x;
    combo_state->game_state->mouse_data.last_position_y = combo_state->game_state->mouse_data.position_y;

    combo_state->game_state->mouse_data.position_x = xpos;
    combo_state->game_state->mouse_data.position_y = ypos;

    if (cstrl_ui_mouse_locked(&combo_state->render_state->ui_context))
    {
        return;
    }

    if (combo_state->game_state->unit_selection_data.current_selection_type != SELECTION_CITY)
    {
        combo_state->game_state->unit_selection_data.selection_end =
            (vec2){combo_state->game_state->mouse_data.position_x, combo_state->game_state->mouse_data.position_y};
    }
    if (!combo_state->game_state->moving_planet)
    {
        return;
    }
    int last_x = combo_state->game_state->mouse_data.last_position_x;
    int last_y = combo_state->game_state->mouse_data.last_position_y;
    if (last_x == -1 || last_y == -1)
    {
        return;
    }

    float y_angle_change = ((float)xpos - (float)last_x) * 0.004f;
    float z_angle_change = ((float)ypos - (float)last_y) * 0.004f;
    float dot_forward_up = cstrl_vec3_dot(main_camera->forward, main_camera->up);
    if (dot_forward_up < -PLANET_LATITUDE_MAX_COS)
    {
        z_angle_change = cstrl_min(z_angle_change, 0.0f);
    }
    if (dot_forward_up > PLANET_LATITUDE_MAX_COS)
    {
        z_angle_change = cstrl_max(z_angle_change, 0.0f);
    }

    cstrl_camera_rotate_around_point(main_camera, PLANET_POSITION, y_angle_change, z_angle_change);
    combo_state->render_state->toggles.main_view_projection_update = true;
}

static void mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
{
    combo_state_t *combo_state = cstrl_platform_get_user_data(state);
    if (cstrl_ui_mouse_locked(&combo_state->render_state->ui_context))
    {
        return;
    }
    cstrl_camera *main_camera = combo_state->render_state->camera_objects.main_camera;
    int mouse_x = combo_state->game_state->mouse_data.position_x;
    int mouse_y = combo_state->game_state->mouse_data.position_y;
    players_t players = combo_state->game_state->player_data.players;
    int human_player = combo_state->game_state->player_data.human_player;
    if (button == CSTRL_MOUSE_BUTTON_LEFT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            if (mods & CSTRL_KEY_MOD_ALT)
            {
                // vec3 mouse_ray_direction = mouse_cursor_ray_cast();
                // vec3 position = main_camera->position;
                // ray_cast_result_t result = regular_ray_cast(position, mouse_ray_direction, 5.0f, NULL);
                // cstrl_da_float_push_back(&g_physics_ray_positions, position.x);
                // cstrl_da_float_push_back(&g_physics_ray_positions, position.y);
                // cstrl_da_float_push_back(&g_physics_ray_positions, position.z);
                // vec3 end_position = cstrl_vec3_add(position, cstrl_vec3_mult_scalar(mouse_ray_direction, 5.0f));
                // cstrl_da_float_push_back(&g_physics_ray_positions, end_position.x);
                // cstrl_da_float_push_back(&g_physics_ray_positions, end_position.y);
                // cstrl_da_float_push_back(&g_physics_ray_positions, end_position.z);
            }
            if (mods & CSTRL_KEY_MOD_CONTROL)
            {
                combo_state->game_state->unit_selection_data.current_selection_type = SELECTION_GROUND;
                combo_state->game_state->unit_selection_data.selection_start = (vec2){mouse_x, mouse_y};
                combo_state->game_state->unit_selection_data.selection_end = (vec2){mouse_x, mouse_y};
                players.selected_formation[human_player] = -1;
                cstrl_da_int_clear(&players.selected_units[human_player]);
            }
            else if (mods & CSTRL_KEY_MOD_SHIFT)
            {
                combo_state->game_state->unit_selection_data.current_selection_type = SELECTION_AIR;
                combo_state->game_state->unit_selection_data.selection_start = (vec2){mouse_x, mouse_y};
                combo_state->game_state->unit_selection_data.selection_end = (vec2){mouse_x, mouse_y};
                players.selected_formation[human_player] = -1;
                cstrl_da_int_clear(&players.selected_units[human_player]);
            }
            else
            {
                vec2 mouse_position = {mouse_x, mouse_y};
                players_select_units(&players, human_player, main_camera->viewport.x, main_camera->viewport.y,
                                     mouse_position, mouse_position, main_camera, 0);
            }
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            if (combo_state->game_state->unit_selection_data.current_selection_type != SELECTION_CITY)
            {
                players_select_units(&players, human_player, main_camera->viewport.x, main_camera->viewport.y,
                                     combo_state->game_state->unit_selection_data.selection_start,
                                     combo_state->game_state->unit_selection_data.selection_end, main_camera,
                                     combo_state->game_state->unit_selection_data.current_selection_type);
                combo_state->game_state->unit_selection_data.current_selection_type = SELECTION_CITY;
            }
        }
    }
    if (button == CSTRL_MOUSE_BUTTON_RIGHT)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            move_units_to_cursor_position(players, human_player, mods & CSTRL_KEY_MOD_CONTROL, main_camera,
                                          combo_state->game_state->mouse_data.position_x,
                                          combo_state->game_state->mouse_data.position_y);
        }
    }
    if (button == CSTRL_MOUSE_BUTTON_MIDDLE)
    {
        if (action == CSTRL_ACTION_PRESS)
        {
            combo_state->game_state->moving_planet = true;
            combo_state->render_state->toggles.main_view_projection_update = true;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            combo_state->game_state->moving_planet = false;
        }
    }
}

static void mouse_wheel_callback(cstrl_platform_state *state, int delta_x, int delta_y, int keys_down)
{
    combo_state_t *combo_state = cstrl_platform_get_user_data(state);
    if (cstrl_ui_mouse_locked(&combo_state->render_state->ui_context))
    {
        return;
    }
    cstrl_camera *main_camera = combo_state->render_state->camera_objects.main_camera;
    main_camera->fov -= (float)delta_y / 6000.0f;
    if (main_camera->fov <= 20.0f * cstrl_pi_180)
    {
        main_camera->fov = 20.0f * cstrl_pi_180;
    }
    else if (main_camera->fov >= 60.0f * cstrl_pi_180)
    {
        main_camera->fov = 60.0f * cstrl_pi_180;
    }
    combo_state->render_state->toggles.main_view_projection_update = true;
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

static vec3 adjust_billboard_scale(vec3 original_scale, float fov)
{
    vec3 modified_scale = cstrl_vec3_mult_scalar(original_scale, fov);
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

static vec3 generate_random_planet_position(players_t *players)
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
            if (!players->active[i] || cstrl_vec3_is_equal(players->units[i].position[0], (vec3){0.0f, 0.0f, 0.0f}))
            {
                continue;
            }
            if (get_spherical_path_length(random_position, players->units[i].position[0]) < 1.0f)
            {
                position_valid = false;
            }
        }
    }
    return random_position;
}

static void create_and_place_unit(vec3 city_origin, players_t *players, int player_id, unit_type type)
{
    while (true)
    {
        int randx = rand() % 2 - 1;
        int randy = rand() % 2 - 1;
        int randz = randx != 0 && randy != 0 ? rand() % 2 - 1 : 1;
        vec3 unit_position =
            cstrl_vec3_add(city_origin, (vec3){(float)(randx)*UNIT_SIZE_X * 2.0f, (float)(randy)*UNIT_SIZE_Y * 2.0f,
                                               (float)(randz)*UNIT_SIZE_X * 2.0f});
        if (type != JET && type != PLANE)
        {
            unit_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(unit_position), 1.0f + UNIT_SIZE_X * 0.5f);
        }
        else
        {
            unit_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(unit_position), 1.0f + UNIT_SIZE_X * 5.0f);
        }
        ray_cast_result_t result = regular_ray_cast(unit_position, cstrl_vec3_normalize(unit_position), 1.0f, NULL);
        if (!result.hit)
        {
            units_add(&players->units[player_id], player_id, unit_position, type);
            break;
        }
    }
}

GAME_API int moon_game_init(render_state_t *render_state, game_state_t *game_state,
                            cstrl_platform_state *platform_state)
{
    if (!cstrl_platform_init(platform_state, "Planet", (1920 - INITIAL_WINDOW_WIDTH) / 2,
                             (1080 - INITIAL_WINDOW_HEIGHT) / 2, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT))
    {
        cstrl_platform_shutdown(platform_state);
        return 1;
    }
    cstrl_platform_set_framebuffer_size_callback(platform_state, framebuffer_callback);
    cstrl_platform_set_key_callback(platform_state, key_callback);
    cstrl_platform_set_mouse_position_callback(platform_state, mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(platform_state, mouse_button_callback);
    cstrl_platform_set_mouse_wheel_callback(platform_state, mouse_wheel_callback);
    cstrl_renderer_init(platform_state);

    srand(cstrl_platform_get_absolute_time());

    render_state->toggles.render_path_markers = false;
    render_state->toggles.render_path_lines = false;
    render_state->toggles.border_update = true;
    render_state->toggles.physics_debug_draw_enabled = false;
    render_state->toggles.main_view_projection_update = true;
    render_state->toggles.ui_view_projection_update = true;
    render_state->toggles.render_planet = true;
    render_state->render_data = malloc(MAX_RENDER_DATA_TYPES * sizeof(cstrl_render_data *));
    render_state->shaders = malloc(MAX_SHADER_TYPES * sizeof(cstrl_shader));
    render_state->textures = malloc(MAX_TEXTURE_TYPES * sizeof(cstrl_texture));

    render_state->render_data[RENDER_DATA_PLANET] = cstrl_renderer_create_render_data();
    float planet_positions[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    int planet_indices[(SPHERE_RESOLUTION - 1) * (SPHERE_RESOLUTION - 1) * 36];
    float planet_normals[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    float planet_tangents[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    float planet_bitangents[SPHERE_RESOLUTION * SPHERE_RESOLUTION * 18];
    generate_sphere_cube(planet_positions, planet_indices, planet_normals, planet_tangents, planet_bitangents,
                         SPHERE_RESOLUTION);
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_PLANET], planet_positions, 3,
                                 SPHERE_RESOLUTION * SPHERE_RESOLUTION * 6);
    cstrl_renderer_add_normals(render_state->render_data[RENDER_DATA_PLANET], planet_normals);
    cstrl_renderer_add_indices(render_state->render_data[RENDER_DATA_PLANET], planet_indices,
                               (SPHERE_RESOLUTION - 1) * (SPHERE_RESOLUTION - 1) * 36);
    cstrl_renderer_add_tangents(render_state->render_data[RENDER_DATA_PLANET], planet_tangents);
    cstrl_renderer_add_bitangents(render_state->render_data[RENDER_DATA_PLANET], planet_bitangents);

    render_state->shaders[SHADER_PLANET] =
        cstrl_load_shaders_from_files("resources/shaders/planet.vert", "resources/shaders/planet.frag");

    render_state->textures[TEXTURE_PLANET] =
        cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/moon/", true);
    render_state->textures[TEXTURE_PLANET_NORMALS] =
        cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/moon_normals/", true);

    // TODO: do this somewhere else
    game_state->player_data.count = 6;
    players_t *players = &game_state->player_data.players;
    if (!players_init(players, game_state->player_data.count))
    {
        printf("Error initializing players\n");
        return 2;
    }
    units_add(&players->units[0], 0, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&players->units[1], 1, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&players->units[2], 2, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&players->units[3], 3, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&players->units[4], 4, (vec3){0.0f, 0.0f, 0.0f}, CITY);
    units_add(&players->units[5], 5, (vec3){0.0f, 0.0f, 0.0f}, CITY);

    vec3 player1_origin = generate_random_planet_position(players);
    players->units[0].position[0] = player1_origin;
    units_update_aabb(&players->units[0], 0);
    vec3 player2_origin = generate_random_planet_position(players);
    players->units[1].position[0] = player2_origin;
    units_update_aabb(&players->units[1], 0);
    vec3 player3_origin = generate_random_planet_position(players);
    players->units[2].position[0] = player3_origin;
    units_update_aabb(&players->units[2], 0);
    vec3 player4_origin = generate_random_planet_position(players);
    players->units[3].position[0] = player4_origin;
    units_update_aabb(&players->units[3], 0);
    vec3 player5_origin = generate_random_planet_position(players);
    players->units[4].position[0] = player5_origin;
    units_update_aabb(&players->units[4], 0);
    vec3 player6_origin = generate_random_planet_position(players);
    players->units[5].position[0] = player6_origin;
    units_update_aabb(&players->units[5], 0);

    create_and_place_unit(player1_origin, players, 0, HUMVEE);
    create_and_place_unit(player1_origin, players, 0, ASTRONAUT);
    create_and_place_unit(player1_origin, players, 0, TANK);
    create_and_place_unit(player1_origin, players, 0, JET);
    create_and_place_unit(player1_origin, players, 0, PLANE);
    create_and_place_unit(player2_origin, players, 1, HUMVEE);
    create_and_place_unit(player2_origin, players, 1, ASTRONAUT);
    create_and_place_unit(player2_origin, players, 1, TANK);
    create_and_place_unit(player2_origin, players, 1, JET);
    create_and_place_unit(player2_origin, players, 1, PLANE);
    create_and_place_unit(player3_origin, players, 2, HUMVEE);
    create_and_place_unit(player3_origin, players, 2, ASTRONAUT);
    create_and_place_unit(player3_origin, players, 2, TANK);
    create_and_place_unit(player3_origin, players, 2, JET);
    create_and_place_unit(player3_origin, players, 2, PLANE);
    create_and_place_unit(player4_origin, players, 3, HUMVEE);
    create_and_place_unit(player4_origin, players, 3, ASTRONAUT);
    create_and_place_unit(player4_origin, players, 3, TANK);
    create_and_place_unit(player4_origin, players, 3, JET);
    create_and_place_unit(player4_origin, players, 3, PLANE);
    create_and_place_unit(player5_origin, players, 4, HUMVEE);
    create_and_place_unit(player5_origin, players, 4, ASTRONAUT);
    create_and_place_unit(player5_origin, players, 4, TANK);
    create_and_place_unit(player5_origin, players, 4, JET);
    create_and_place_unit(player5_origin, players, 4, PLANE);
    create_and_place_unit(player6_origin, players, 5, HUMVEE);
    create_and_place_unit(player6_origin, players, 5, ASTRONAUT);
    create_and_place_unit(player6_origin, players, 5, TANK);
    create_and_place_unit(player6_origin, players, 5, JET);
    create_and_place_unit(player6_origin, players, 5, PLANE);

    render_state->render_data[RENDER_DATA_CITY] = cstrl_renderer_create_render_data();

    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_CITY], planet_positions, 3,
                                 SPHERE_RESOLUTION * SPHERE_RESOLUTION * 6);
    cstrl_renderer_add_indices(render_state->render_data[RENDER_DATA_CITY], planet_indices,
                               (SPHERE_RESOLUTION - 1) * (SPHERE_RESOLUTION - 1) * 36);

    render_state->shaders[SHADER_CITY] =
        cstrl_load_shaders_from_files("resources/shaders/city_borders.vert", "resources/shaders/city_borders.frag");
    render_state->textures[TEXTURE_CITY_NOISE] =
        cstrl_texture_generate_from_path("resources/textures/planet_game/noise_texture.png");
    // TODO: get this from somewhere else
    game_state->player_data.human_player = 0;
    ai_init(&game_state->player_data.ai, 6, game_state->player_data.human_player);

    render_state->render_data[RENDER_DATA_UNIT] = cstrl_renderer_create_render_data();
    float unit_positions[18] = {0.0f};
    int unit_indices[6] = {0};
    float unit_uvs[12] = {0.0f};
    float unit_colors[24] = {0.0f};
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_UNIT], unit_positions, 3, 4);
    cstrl_renderer_add_indices(render_state->render_data[RENDER_DATA_UNIT], unit_indices, 6);
    cstrl_renderer_add_uvs(render_state->render_data[RENDER_DATA_UNIT], unit_uvs);
    cstrl_renderer_add_colors(render_state->render_data[RENDER_DATA_UNIT], unit_colors);
    render_state->shaders[SHADER_UNIT] =
        cstrl_load_shaders_from_files("resources/shaders/default3D.vert", "resources/shaders/default3D.frag");
    render_state->textures[TEXTURE_UNIT] = cstrl_texture_generate_from_path("resources/textures/planet_game/icons.png");

    render_state->render_data[RENDER_DATA_PATH_MARKER] = cstrl_renderer_create_render_data();
    float path_marker_positions[18] = {0.0f};
    int path_marker_indices[6] = {0};
    float path_marker_colors[12] = {0.0f};
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_PATH_MARKER], path_marker_positions, 3, 4);
    cstrl_renderer_add_indices(render_state->render_data[RENDER_DATA_PATH_MARKER], path_marker_indices, 6);
    cstrl_renderer_add_colors(render_state->render_data[RENDER_DATA_PATH_MARKER], path_marker_colors);
    render_state->shaders[SHADER_PATH_MARKER] = cstrl_load_shaders_from_files(
        "resources/shaders/default3D_no_texture.vert", "resources/shaders/default3D_no_texture.frag");

    render_state->render_data[RENDER_DATA_PATH_LINE] = cstrl_renderer_create_render_data();
    float path_line_positions[6] = {0.0f};
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_PATH_LINE], path_line_positions, 3, 2);
    render_state->shaders[SHADER_PATH_LINE] =
        cstrl_load_shaders_from_files("resources/shaders/line3D.vert", "resources/shaders/line3D.frag");

    render_state->render_data[RENDER_DATA_SELECTION_BOX] = cstrl_renderer_create_render_data();
    float selection_box_positions[8] = {0.0f};
    int selection_box_indices[6] = {0.0f};
    float selection_box_colors[16] = {0.0f};
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_SELECTION_BOX], selection_box_positions, 2, 4);
    cstrl_renderer_add_indices(render_state->render_data[RENDER_DATA_SELECTION_BOX], selection_box_indices, 6);
    cstrl_renderer_add_colors(render_state->render_data[RENDER_DATA_SELECTION_BOX], selection_box_colors);

    render_state->shaders[SHADER_SELECTION_BOX] = cstrl_load_shaders_from_files(
        "resources/shaders/default_no_texture.vert", "resources/shaders/default_no_texture.frag");

    render_state->render_data[RENDER_DATA_SKYBOX] = cstrl_renderer_create_render_data();
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
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_SKYBOX], positions, 3, 36);
    cstrl_renderer_add_colors(render_state->render_data[RENDER_DATA_SKYBOX], colors);
    render_state->shaders[SHADER_SKYBOX] =
        cstrl_load_shaders_from_files("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
    render_state->textures[TEXTURE_SKYBOX] =
        cstrl_texture_cube_map_generate_from_folder("resources/textures/planet_game/stars/", true);

    // cstrl_da_float_init(&g_physics_ray_positions, 3);
    // cstrl_render_data *physics_debug_render_data = cstrl_renderer_create_render_data();
    // da_float physics_debug_positions;
    // cstrl_da_float_init(&physics_debug_positions, 36);
    // fill_physics_positions(&physics_debug_positions, &g_physics_ray_positions);
    // cstrl_renderer_add_positions(physics_debug_render_data, physics_debug_positions.array, 3,
    //                              physics_debug_positions.size / 3);
    //
    // cstrl_shader physics_debug_shader =
    //     cstrl_load_shaders_from_files("resources/shaders/line3D.vert", "resources/shaders/line3D.frag");

    render_state->render_data[RENDER_DATA_EXPLOSION] = cstrl_renderer_create_render_data();
    float explosion_positions[12] = {0.0f};
    int explosion_indices[6] = {0};
    float explosion_uvs[12] = {0.0f};
    cstrl_renderer_add_positions(render_state->render_data[RENDER_DATA_EXPLOSION], explosion_positions, 3, 4);
    cstrl_renderer_add_indices(render_state->render_data[RENDER_DATA_EXPLOSION], explosion_indices, 6);
    cstrl_renderer_add_uvs(render_state->render_data[RENDER_DATA_EXPLOSION], explosion_uvs);
    render_state->shaders[SHADER_EXPLOSION] = cstrl_load_shaders_from_files(
        "resources/shaders/default3D_no_color.vert", "resources/shaders/default3D_no_color.frag");
    render_state->textures[TEXTURE_EXPLOSION] =
        cstrl_texture_generate_from_path("resources/textures/planet_game/explosion2.png");

    render_state->camera_objects.main_camera = cstrl_camera_create(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, false);
    render_state->camera_objects.main_camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(player1_origin));
    render_state->camera_objects.main_camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(
        render_state->camera_objects.main_camera->forward, render_state->camera_objects.main_camera->up));
    render_state->camera_objects.main_camera->position =
        cstrl_vec3_mult_scalar(render_state->camera_objects.main_camera->forward, -3.0f);
    cstrl_camera_update(render_state->camera_objects.main_camera, CSTRL_CAMERA_DIRECTION_NONE,
                        CSTRL_CAMERA_DIRECTION_NONE);

    render_state->camera_objects.ui_camera = cstrl_camera_create(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, true);
    cstrl_camera_update(render_state->camera_objects.ui_camera, CSTRL_CAMERA_DIRECTION_NONE,
                        CSTRL_CAMERA_DIRECTION_NONE);

    cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "material.ambient", 0.3f, 0.3f, 0.3f);
    cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "material.diffuse", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "material.specular", 0.5f, 0.5f, 0.5f);
    cstrl_set_uniform_float(render_state->shaders[SHADER_PLANET].program, "material.shininess", 2.0f);
    cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "light.ambient", 0.3f, 0.3f, 0.3f);
    cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "light.diffuse", 0.8f, 0.8f, 0.8f);
    cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "light.specular", 0.8f, 0.8f, 0.8f);

    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_PLANET], "Matrices", 0);
    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_CITY], "Matrices", 0);
    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_PATH_MARKER], "Matrices", 0);
    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_PATH_LINE], "Matrices", 0);
    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_UNIT], "Matrices", 0);
    // cstrl_set_uniform_block_binding(physics_debug_shader, "Matrices", 0);
    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_EXPLOSION], "Matrices", 0);
    cstrl_set_uniform_block_binding(render_state->shaders[SHADER_SKYBOX], "Matrices", 0);

    render_state->view_projection_ubo = cstrl_renderer_add_ubo(2 * sizeof(mat4));
    cstrl_renderer_update_ubo(render_state->view_projection_ubo, render_state->camera_objects.main_camera->view.m,
                              sizeof(mat4), 0);
    cstrl_renderer_update_ubo(render_state->view_projection_ubo, render_state->camera_objects.main_camera->projection.m,
                              sizeof(mat4), sizeof(mat4));

    cstrl_set_uniform_mat4(render_state->shaders[SHADER_SELECTION_BOX].program, "view",
                           render_state->camera_objects.ui_camera->view);
    cstrl_set_uniform_mat4(render_state->shaders[SHADER_SELECTION_BOX].program, "projection",
                           render_state->camera_objects.ui_camera->projection);

    cstrl_create_framebuffer(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, &render_state->fbo, &render_state->rbo,
                             &render_state->framebuffer_vao);
    render_state->shaders[SHADER_FRAMEBUFFER] =
        cstrl_load_shaders_from_files("resources/shaders/framebuffer.vert", "resources/shaders/framebuffer.frag");
    render_state->textures[TEXTURE_FRAMEBUFFER] =
        cstrl_texture_framebuffer_generate(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);

    cstrl_ui_init(&render_state->ui_context, platform_state);

    game_state->moving_planet = false;
    game_state->mouse_data.position_x = -1;
    game_state->mouse_data.position_y = -1;
    game_state->mouse_data.last_position_x = -1;
    game_state->mouse_data.last_position_y = -1;
    game_state->unit_selection_data.current_selection_type = SELECTION_CITY;
    game_state->unit_selection_data.selection_start = (vec2){-1.0f, -1.0f};
    game_state->unit_selection_data.selection_end = (vec2){-1.0f, -1.0f};

    combo_state_t *combo_state = malloc(sizeof(combo_state_t));
    combo_state->game_state = game_state;
    combo_state->render_state = render_state;
    cstrl_platform_set_user_data(platform_state, (void *)combo_state);
    return 0;
}

GAME_API int moon_game_update(game_state_t *game_state, cstrl_platform_state *platform_state, double *previous_time,
                              double *lag)
{
    cstrl_platform_pump_messages(platform_state);

    players_t players = game_state->player_data.players;
    double current_time = cstrl_platform_get_absolute_time();
    double elapsed_time = current_time - *previous_time;
    *previous_time = current_time;
    *lag += elapsed_time;
    while (*lag >= 1.0 / 60.0)
    {
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            if (!players.active[i])
            {
                continue;
            }
            players_update(&game_state->player_data.players, i);
            for (int j = 0; j < players.units[i].count; j++)
            {
                if (!players.units[i].active[j])
                {
                    continue;
                }
                if (players.units[i].attacking[j])
                {
                    if (cstrl_platform_get_absolute_time() > players.units[i].last_attack_time[j] + 0.1)
                    {
                        players.units[i].last_attack_time[j] = cstrl_platform_get_absolute_time();
                        int index = cstrl_da_int_find_first(
                            &players.formations[i].unit_ids[players.units[i].formation_id[j]], j);
                        int path_id = players.formations[i].path_heads[players.units[i].formation_id[j]].array[index];
                        vec3 direction = cstrl_vec3_normalize(
                            cstrl_vec3_sub(get_point_on_path((vec3){0.0f, 0.0f, 0.0f}, players.units[i].position[j],
                                                             players.paths[i].end_positions[path_id], 0.5f),
                                           players.units[i].position[j]));
                        unit_data_t tracked_unit_data = players.paths[i].tracked_unit[path_id];
                        vec3 position = players.units[tracked_unit_data.player_id].position[tracked_unit_data.unit_id];
                        int type = players.units[tracked_unit_data.player_id].type[tracked_unit_data.unit_id];
                        position.x += (rand() % 100 - 50) / 5000.0f;
                        position.y += (rand() % 100 - 50) / 5000.0f;
                        position.z += (rand() % 100 - 50) / 5000.0f;
                        if (type != JET && type != PLANE)
                        {
                            position =
                                cstrl_vec3_mult_scalar(cstrl_vec3_normalize(position), 1.0f + UNIT_SIZE_X * 0.6f);
                        }
                        else
                        {
                            position =
                                cstrl_vec3_mult_scalar(cstrl_vec3_normalize(position), 1.0f + UNIT_SIZE_X * 5.1f);
                        }
                        projectiles_add(&players.projectiles[i], i, position, direction);
                        players.units[tracked_unit_data.player_id].stats[tracked_unit_data.unit_id].current_health -=
                            1.0f;
                        if (players.units[tracked_unit_data.player_id]
                                .stats[tracked_unit_data.unit_id]
                                .current_health <= 0.0f)
                        {
                            units_remove(&players.units[tracked_unit_data.player_id], tracked_unit_data.unit_id);
                            players.paths[i].tracked_unit[path_id].player_id = -1;
                            players.paths[i].tracked_unit[path_id].unit_id = -1;
                            players.units[i].attacking[j] = false;
                        }
                    }
                }
            }
            for (int j = 0; j < players.projectiles[i].count; j++)
            {
                if (!players.projectiles[i].active[j])
                {
                    continue;
                }
                float increment = 1.0f / 20.0f;
                if (cstrl_platform_get_absolute_time() > players.projectiles[i].transition_time[j] + 0.0)
                {
                    players.projectiles[i].transition_time[j] = cstrl_platform_get_absolute_time();
                    players.projectiles[i].uvs[j].u0 += increment;
                    players.projectiles[i].uvs[j].u1 += increment;
                    if (players.projectiles[i].uvs[j].u1 >= 1.0f)
                    {
                        projectiles_remove(&players.projectiles[i], j);
                        continue;
                    }
                }
            }
        }
        // int human_selected_formation = g_players.selected_formation[g_human_player];
        // if ((g_render_path_lines || g_render_path_markers) && human_selected_formation != -1)
        // {
        //     int render_index = 0;
        //     for (int i = 0; i < g_players.formations[g_human_player].path_heads[human_selected_formation].size; i++)
        //     {
        //         int path_id = g_players.formations[g_human_player].path_heads[human_selected_formation].array[i];
        //         while (path_id != -1)
        //         {
        //             if (g_render_path_markers)
        //             {
        //                 if (!g_players.paths[g_human_player].render[path_id] ||
        //                     !g_players.paths[g_human_player].active[path_id] ||
        //                     g_players.paths[g_human_player].completed[path_id])
        //                 {
        //                     path_id = g_players.paths[g_human_player].next[path_id];
        //                     continue;
        //                 }
        //                 update_billboard_object(&path_marker_positions, &path_marker_indices, NULL,
        //                 &path_marker_colors,
        //                                         render_index,
        //                                         (transform){g_players.paths[g_human_player].end_positions[path_id],
        //                                                     billboard_rotation,
        //                                                     adjust_billboard_scale(PATH_MARKER_SIZE)},
        //                                         (vec4){},
        //                                         !g_players.formations[g_human_player]
        //                                                 .following_enemy[g_players.selected_formation[g_human_player]]
        //                                             ? PATH_MARKER_COLOR
        //                                             : PATH_MARKER_COLOR_ATTACK);
        //                 render_index++;
        //             }
        //             if (g_render_path_lines)
        //             {
        //                 vec3 start = g_players.paths[g_human_player].start_positions[path_id];
        //                 if (!g_players.paths[g_human_player].in_queue[path_id])
        //                 {
        //                     start = g_players.units[g_human_player].position
        //                                 [g_players.formations[g_human_player].unit_ids[human_selected_formation].array[i]];
        //                 }
        //                 generate_line_segments(&path_line_positions, start,
        //                                        g_players.paths[g_human_player].end_positions[path_id], 0.1f);
        //             }
        //             path_id = g_players.paths[g_human_player].next[path_id];
        //         }
        //     }
        //     if (path_marker_positions.size > 0)
        //     {
        //         cstrl_renderer_modify_render_attributes(path_marker_render_data, path_marker_positions.array, NULL,
        //                                                 path_marker_colors.array, path_marker_positions.size / 3);
        //         cstrl_renderer_modify_indices(path_marker_render_data, path_marker_indices.array, 0,
        //                                       path_marker_indices.size);
        //     }
        //     if (path_line_positions.size > 0)
        //     {
        //         cstrl_renderer_modify_positions(path_line_render_data, path_line_positions.array, 0,
        //                                         path_line_positions.size);
        //     }
        // }
        // light_start_x += 0.001f;
        // light_start_z += 0.001f;
        *lag -= 1.0 / 60.0;
    }
    return 0;
}

GAME_API int moon_game_render(render_state_t *render_state, game_state_t *game_state,
                              cstrl_platform_state *platform_state)
{
    cstrl_camera_update(render_state->camera_objects.main_camera, CSTRL_CAMERA_DIRECTION_NONE,
                        CSTRL_CAMERA_DIRECTION_NONE);
    players_t players = game_state->player_data.players;
    int human_player = game_state->player_data.human_player;
    da_float selection_box_positions;
    cstrl_da_float_init(&selection_box_positions, 8);
    da_int selection_box_indices;
    cstrl_da_int_init(&selection_box_indices, 6);
    da_float selection_box_colors;
    cstrl_da_float_init(&selection_box_colors, 16);
    cstrl_renderer_clear_render_attributes(render_state->render_data[RENDER_DATA_SELECTION_BOX]);
    if (game_state->unit_selection_data.current_selection_type != SELECTION_CITY)
    {
        // TODO: get color from somewhere
        update_ui_object(&selection_box_positions, &selection_box_indices, NULL, &selection_box_colors, 0,
                         game_state->unit_selection_data.selection_start, game_state->unit_selection_data.selection_end,
                         (vec4){1.0f, 0.0f, 0.0f, 0.1f});
        cstrl_renderer_modify_render_attributes(render_state->render_data[RENDER_DATA_SELECTION_BOX],
                                                selection_box_positions.array, NULL, selection_box_colors.array,
                                                selection_box_positions.size / 2);
        cstrl_renderer_modify_indices(render_state->render_data[RENDER_DATA_SELECTION_BOX], selection_box_indices.array,
                                      0, selection_box_indices.size);
    }

    cstrl_renderer_clear_render_attributes(render_state->render_data[RENDER_DATA_UNIT]);
    da_float unit_positions;
    cstrl_da_float_init(&unit_positions, 12);
    da_int unit_indices;
    cstrl_da_int_init(&unit_indices, 6);
    da_float unit_uvs;
    cstrl_da_float_init(&unit_uvs, 12);
    da_float unit_colors;
    cstrl_da_float_init(&unit_colors, 16);
    int unit_render_index = 0;

    cstrl_renderer_clear_render_attributes(render_state->render_data[RENDER_DATA_EXPLOSION]);
    da_float explosion_positions;
    cstrl_da_float_init(&explosion_positions, 12);
    da_int explosion_indices;
    cstrl_da_int_init(&explosion_indices, 6);
    da_float explosion_uvs;
    cstrl_da_float_init(&explosion_uvs, 12);
    int projectile_render_index = 0;

    quat camera_rotation =
        cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(render_state->camera_objects.main_camera->view));
    quat billboard_rotation = cstrl_quat_inverse(camera_rotation);
    ai_update(&game_state->player_data.ai, &game_state->player_data.players, game_state->player_data.human_player);
    for (int i = 0; i < game_state->player_data.count; i++)
    {
        for (int j = 0; j < game_state->player_data.players.units[i].count; j++)
        {
            if (!game_state->player_data.players.units[i].active[j])
            {
                continue;
            }
            vec4 color = UNIT_TEAM_COLORS[i];
            if (i == game_state->player_data.human_player &&
                cstrl_da_int_find_first(&players.selected_units[i], j) != -1)
            {
                color.r += 0.2f;
                color.g += 0.2f;
                color.b += 0.2f;
                color.a = 1.0f;
            }
            int type = players.units[i].type[j];
            vec4 uv_positions = {(float)type / MAX_UNIT_TYPES, 0.0f, ((float)type + 1.0f) / MAX_UNIT_TYPES, 1.0f};
            vec3 size =
                (type != ASTRONAUT && type != ASTRONAUT_ARMED ? UNIT_SIZE : cstrl_vec3_mult_scalar(UNIT_SIZE, 0.65f));
            update_billboard_object(
                &unit_positions, &unit_indices, &unit_uvs, &unit_colors, unit_render_index,
                (transform){players.units[i].position[j], billboard_rotation,
                            adjust_billboard_scale(size, render_state->camera_objects.main_camera->fov /
                                                             (45.0f * cstrl_pi_180))},
                uv_positions, color);
            unit_render_index++;
        }
        for (int j = 0; j < players.projectiles[i].count; j++)
        {
            if (!game_state->player_data.players.projectiles[i].active[j])
            {
                continue;
            }
            update_billboard_object(
                &explosion_positions, &explosion_indices, &explosion_uvs, NULL, projectile_render_index++,
                (transform){
                    players.projectiles[i].position[j], billboard_rotation,
                    adjust_billboard_scale((vec3){UNIT_SIZE_X * 0.4f, UNIT_SIZE_Y * 0.4f, 0.0f},
                                           render_state->camera_objects.main_camera->fov / (45.0f * cstrl_pi_180))},
                players.projectiles[i].uvs[j], (vec4){1.0f, 1.0f, 1.0f, 1.0f});
        }
    }

    if (unit_positions.size > 0)
    {
        cstrl_renderer_modify_render_attributes(render_state->render_data[RENDER_DATA_UNIT], unit_positions.array,
                                                unit_uvs.array, unit_colors.array, unit_positions.size / 3);
        cstrl_renderer_modify_indices(render_state->render_data[RENDER_DATA_UNIT], unit_indices.array, 0,
                                      unit_indices.size);
    }

    if (explosion_positions.size > 0)
    {
        cstrl_renderer_modify_render_attributes(render_state->render_data[RENDER_DATA_EXPLOSION],
                                                explosion_positions.array, explosion_uvs.array, NULL,
                                                explosion_positions.size / 3);
        cstrl_renderer_modify_indices(render_state->render_data[RENDER_DATA_EXPLOSION], explosion_indices.array, 0,
                                      explosion_indices.size);
    }

    if (render_state->toggles.main_view_projection_update)
    {
        cstrl_renderer_update_ubo(render_state->view_projection_ubo, render_state->camera_objects.main_camera->view.m,
                                  sizeof(mat4), 0);
        cstrl_renderer_update_ubo(render_state->view_projection_ubo,
                                  render_state->camera_objects.main_camera->projection.m, sizeof(mat4), sizeof(mat4));
        cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "view_position",
                             render_state->camera_objects.main_camera->position.x,
                             render_state->camera_objects.main_camera->position.y,
                             render_state->camera_objects.main_camera->position.z);
        render_state->toggles.main_view_projection_update = false;
    }
    if (render_state->toggles.ui_view_projection_update)
    {
        cstrl_set_uniform_mat4(render_state->shaders[SHADER_SELECTION_BOX].program, "view",
                               render_state->camera_objects.ui_camera->view);
        cstrl_set_uniform_mat4(render_state->shaders[SHADER_SELECTION_BOX].program, "projection",
                               render_state->camera_objects.ui_camera->projection);
        render_state->toggles.ui_view_projection_update = false;
    }
    cstrl_renderer_clear(0.1f, 0.0f, 0.1f, 1.0f);
    cstrl_renderer_bind_framebuffer(render_state->fbo);
    cstrl_renderer_clear(0.1f, 0.0f, 0.1f, 1.0f);
    if (render_state->toggles.render_planet)
    {
        vec3 light_position = {10.0f * cosf(cstrl_platform_get_absolute_time() / 1000.0), 0.0f,
                               10.0f * sinf(cstrl_platform_get_absolute_time() / 1000.0)};
        cstrl_set_uniform_3f(render_state->shaders[SHADER_PLANET].program, "light.position", light_position.x,
                             light_position.y, light_position.z);
        cstrl_use_shader(render_state->shaders[SHADER_PLANET]);
        cstrl_set_active_texture(0);
        cstrl_texture_cube_map_bind(render_state->textures[TEXTURE_PLANET]);
        cstrl_set_uniform_int(render_state->shaders[SHADER_PLANET].program, "texture0", 0);
        cstrl_set_active_texture(1);
        cstrl_texture_cube_map_bind(render_state->textures[TEXTURE_PLANET_NORMALS]);
        cstrl_set_uniform_int(render_state->shaders[SHADER_PLANET].program, "normal0", 1);
        cstrl_renderer_draw_indices(render_state->render_data[RENDER_DATA_PLANET]);
    }
    cstrl_use_shader(render_state->shaders[SHADER_SKYBOX]);
    cstrl_set_active_texture(0);
    cstrl_texture_cube_map_bind(render_state->textures[TEXTURE_SKYBOX]);
    cstrl_renderer_draw(render_state->render_data[RENDER_DATA_SKYBOX]);
    cstrl_renderer_bind_framebuffer(0);

    cstrl_renderer_set_depth_test_enabled(false);
    cstrl_renderer_set_cull_face_enabled(false);
    cstrl_use_shader(render_state->shaders[SHADER_FRAMEBUFFER]);
    cstrl_set_active_texture(0);
    cstrl_texture_bind(render_state->textures[TEXTURE_FRAMEBUFFER]);
    cstrl_set_uniform_int(render_state->shaders[SHADER_FRAMEBUFFER].program, "screen_texture", 0);
    cstrl_set_uniform_float(render_state->shaders[SHADER_FRAMEBUFFER].program, "time",
                            (float)cstrl_platform_get_absolute_time());
    cstrl_renderer_framebuffer_draw(render_state->framebuffer_vao);
    cstrl_renderer_set_depth_test_enabled(true);
    cstrl_renderer_set_cull_face_enabled(true);

    if (render_state->toggles.border_update)
    {
        int city_index = 0;
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            if (!players.active[i])
            {
                continue;
            }
            for (int j = 0; j < players.units[i].count; j++)
            {
                if (players.units[i].type[j] != CITY)
                {
                    continue;
                }
                char buffer[30];
                sprintf(buffer, "city_centers[%d]", city_index);
                cstrl_set_uniform_3f(render_state->shaders[SHADER_CITY].program, buffer, players.units[i].position[j].x,
                                     players.units[i].position[j].y, players.units[i].position[j].z);
                sprintf(buffer, "team[%d]", city_index);
                cstrl_set_uniform_int_array(render_state->shaders[SHADER_CITY].program, buffer, 1, &i);
                sprintf(buffer, "weights[%d]", city_index);
                cstrl_set_uniform_float(render_state->shaders[SHADER_CITY].program, buffer, 1.0f);
                sprintf(buffer, "influence_strength[%d]", city_index);
                cstrl_set_uniform_float(render_state->shaders[SHADER_CITY].program, buffer, 1.0f);
                sprintf(buffer, "influence_radius[%d]", city_index++);
                cstrl_set_uniform_float(render_state->shaders[SHADER_CITY].program, buffer, 0.4f);
            }
        }
        cstrl_set_uniform_int(render_state->shaders[SHADER_CITY].program, "cities_count", city_index);
        render_state->toggles.border_update = false;
    }
    cstrl_use_shader(render_state->shaders[SHADER_CITY]);
    cstrl_set_active_texture(0);
    cstrl_texture_bind(render_state->textures[TEXTURE_CITY_NOISE]);
    cstrl_renderer_draw_indices(render_state->render_data[RENDER_DATA_CITY]);

    if (render_state->toggles.render_path_markers)
    {
        cstrl_use_shader(render_state->shaders[SHADER_PATH_MARKER]);
        cstrl_renderer_draw_indices(render_state->render_data[RENDER_DATA_PATH_MARKER]);
    }
    if (render_state->toggles.render_path_lines)
    {
        if (players.selected_formation[human_player] != -1 &&
            !players.formations[human_player].following_enemy[players.selected_formation[human_player]])
        {
            cstrl_set_uniform_4f(render_state->shaders[SHADER_PATH_LINE].program, "color", PATH_MARKER_COLOR.r,
                                 PATH_MARKER_COLOR.g, PATH_MARKER_COLOR.b, PATH_MARKER_COLOR.a);
        }
        else
        {
            cstrl_set_uniform_4f(render_state->shaders[SHADER_PATH_LINE].program, "color", PATH_MARKER_COLOR_ATTACK.r,
                                 PATH_MARKER_COLOR_ATTACK.g, PATH_MARKER_COLOR_ATTACK.b, PATH_MARKER_COLOR_ATTACK.a);
        }
        cstrl_use_shader(render_state->shaders[SHADER_PATH_LINE]);
        cstrl_renderer_draw_lines(render_state->render_data[RENDER_DATA_PATH_LINE]);
    }

    cstrl_use_shader(render_state->shaders[SHADER_UNIT]);
    cstrl_set_active_texture(0);
    cstrl_texture_bind(render_state->textures[TEXTURE_UNIT]);
    cstrl_renderer_draw_indices(render_state->render_data[RENDER_DATA_UNIT]);

    // if (g_physics_debug_draw_enabled)
    // {
    //     cstrl_da_float_clear(&physics_debug_positions);
    //     fill_physics_positions(&physics_debug_positions, &g_physics_ray_positions);
    //     cstrl_renderer_modify_render_attributes(physics_debug_render_data, physics_debug_positions.array, NULL, NULL,
    //                                             physics_debug_positions.size / 3);
    //     cstrl_set_uniform_4f(physics_debug_shader.program, "color", PATH_MARKER_COLOR.r, PATH_MARKER_COLOR.g,
    //                          PATH_MARKER_COLOR.b, PATH_MARKER_COLOR.a);
    //     cstrl_use_shader(physics_debug_shader);
    //     cstrl_renderer_draw_lines(physics_debug_render_data);
    // }

    cstrl_use_shader(render_state->shaders[SHADER_EXPLOSION]);
    cstrl_set_active_texture(0);
    cstrl_texture_bind(render_state->textures[TEXTURE_EXPLOSION]);
    cstrl_renderer_draw_indices(render_state->render_data[RENDER_DATA_EXPLOSION]);

    cstrl_renderer_set_depth_test_enabled(false);
    cstrl_renderer_set_cull_face_enabled(false);
    cstrl_use_shader(render_state->shaders[SHADER_SELECTION_BOX]);
    cstrl_renderer_draw_indices(render_state->render_data[RENDER_DATA_SELECTION_BOX]);
    cstrl_ui_begin(&render_state->ui_context);
    if (cstrl_ui_container_begin(&render_state->ui_context, "Economy", 7, 10, 10, 200, 300, GEN_ID(0), false, false, 2))
    {
        cstrl_ui_container_end(&render_state->ui_context);
    }
    if (cstrl_ui_container_begin(&render_state->ui_context, "Something Else", 14, 400, 300, 200, 300, GEN_ID(0), false,
                                 false, 2))
    {
        cstrl_ui_container_end(&render_state->ui_context);
    }
    /*
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
    */
    cstrl_ui_end(&render_state->ui_context);
    cstrl_renderer_set_depth_test_enabled(true);
    cstrl_renderer_set_cull_face_enabled(true);
    cstrl_renderer_swap_buffers(platform_state);
    return 0;
}

GAME_API void moon_game_shutdown(render_state_t *render_state, game_state_t *game_state,
                                 cstrl_platform_state *platform_state)
{
    cstrl_camera_free(render_state->camera_objects.main_camera);
    cstrl_camera_free(render_state->camera_objects.ui_camera);

    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_PLANET]);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_CITY]);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_UNIT]);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_PATH_MARKER]);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_PATH_LINE]);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_SELECTION_BOX]);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_SKYBOX]);
    // cstrl_renderer_free_render_data(physics_debug_render_data);
    cstrl_renderer_free_render_data(render_state->render_data[RENDER_DATA_EXPLOSION]);

    cstrl_ui_shutdown(&render_state->ui_context);
    cstrl_renderer_shutdown(platform_state);
    cstrl_platform_free_user_data(platform_state);
    cstrl_platform_shutdown(platform_state);
}

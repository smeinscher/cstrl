#include "players.h"
#include "../helpers/helpers.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include "units.h"
#include <stdio.h>

#define SEPARATION_RADIUS (UNIT_SIZE_X * 0.5f)
#define SEPARATION_WEIGHT 0.0f
#define COHESION_WEIGHT 0.0f
#define AVOIDANCE_WEIGHT 0.0f
#define SEEK_WEIGHT 1.0f

static const float FORMATION_OFFSETS[] = {0.0f, 1.0f, -1.0f, 2.0f, -2.0f};

static void optimize_formation_positions(players_t *players, int player_id, int formation_id)
{
    int leader_unit_id = players->formations[player_id].unit_ids[formation_id].array[0];
    bool ground_units = players->units[player_id].type[leader_unit_id] == TANK ||
                        players->units[player_id].type[leader_unit_id] == HUMVEE ||
                        players->units[player_id].type[leader_unit_id] == ASTRONAUT ||
                        players->units[player_id].type[leader_unit_id] == ASTRONAUT_ARMED;

    da_float distance;
    cstrl_da_float_init(&distance, powf(players->formations[player_id].unit_ids[formation_id].size, 2.0f));
    for (int i = 0; i < players->formations[player_id].unit_ids[formation_id].size; i++)
    {
        int unit_id = players->formations[player_id].unit_ids[formation_id].array[i];
        for (int j = 0; j < players->formations[player_id].path_heads[formation_id].size; j++)
        {
            int path_id = players->formations[player_id].path_heads[formation_id].array[j];
            cstrl_da_float_push_back(&distance,
                                     cstrl_vec3_length(cstrl_vec3_sub(players->paths[player_id].end_positions[path_id],
                                                                      players->units[player_id].position[unit_id])));
        }
    }
    da_int taken_paths;
    cstrl_da_int_init(&taken_paths, players->formations[player_id].path_heads[formation_id].size);
    for (int i = 0; i < players->formations[player_id].path_heads[formation_id].size; i++)
    {
        int unit_id = players->formations[player_id].unit_ids[formation_id].array[i];
        float min_distance = cstrl_infinity;
        int min_index = 0;
        for (int j = 0; j < players->formations[player_id].unit_ids[formation_id].size; j++)
        {
            int path_id = players->formations[player_id].path_heads[formation_id].array[j];
            if (cstrl_da_int_find_first(&taken_paths, path_id) != -1)
            {
                continue;
            }
            if (distance.array[i * players->formations[player_id].path_heads[formation_id].size + j] < min_distance)
            {
                min_distance = distance.array[i * players->formations[player_id].unit_ids[formation_id].size + j];
                min_index = path_id;
            }
        }
        cstrl_da_int_push_back(&taken_paths, min_index);
    }
    for (int i = 0; i < taken_paths.size; i++)
    {
        players->formations[player_id].path_heads[formation_id].array[i] = taken_paths.array[i];
    }
    cstrl_da_float_free(&distance);
    cstrl_da_int_free(&taken_paths);
}

static void new_path(players_t *players, int player_id, vec3 start_position, vec3 end_position, bool in_queue,
                     int unit_id, int prev)
{
    bool ground_units = players->units[player_id].type[unit_id] == TANK ||
                        players->units[player_id].type[unit_id] == HUMVEE ||
                        players->units[player_id].type[unit_id] == ASTRONAUT ||
                        players->units[player_id].type[unit_id] == ASTRONAUT_ARMED;
    float speed = BASE_UNIT_SPEEDS[players->units[player_id].type[unit_id]] * 0.0005f;
    int new_path_id = paths_add(&players->paths[player_id], start_position, end_position, prev, speed);
    if (new_path_id == -1)
    {
        printf("Error creating path\n");
        return;
    }
    int unit_index = cstrl_da_int_find_first(
        &players->formations[player_id].unit_ids[players->selected_formation[player_id]], unit_id);
    if (unit_index > 0)
    {
        float mod = ground_units ? 0.5f : 5.0f;
        vec3 path_vector = cstrl_vec3_sub(
            end_position,
            players->units[player_id]
                .position[players->formations[player_id].unit_ids[players->selected_formation[player_id]].array[0]]);
        path_vector = cstrl_vec3_normalize(path_vector);
        vec3 formation_line = cstrl_vec3_normalize(cstrl_vec3_cross(path_vector, cstrl_vec3_normalize(end_position)));
        vec3 new_end_position = cstrl_vec3_add(
            end_position, cstrl_vec3_mult_scalar(formation_line, FORMATION_OFFSETS[unit_index % 5] * UNIT_SIZE.x));
        new_end_position = cstrl_vec3_sub(
            new_end_position, cstrl_vec3_mult_scalar(path_vector, floorf(unit_index / 5.0f) * UNIT_SIZE.y * 1.5f));
        new_end_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(new_end_position), 1.0f + UNIT_SIZE.x * mod);
        players->paths[player_id].end_positions[new_path_id] = new_end_position;
    }
    if (players->formations[player_id].path_heads[players->selected_formation[player_id]].array[unit_index] != -1)
    {
        players->paths[player_id].in_queue[new_path_id] = true;
    }
    else
    {
        players->formations[player_id].path_heads[players->selected_formation[player_id]].array[unit_index] =
            new_path_id;
        players->paths[player_id].in_queue[new_path_id] = false;
        players->units[player_id].velocity[unit_id] = (vec3){0.0f, 0.0f, 0.0f};
    }
}

void players_set_units_in_formation_selected(players_t *players, int player_id)
{
    if (players->selected_units[player_id].size == 0)
    {
        return;
    }
    int formation_id = players->units[player_id].formation_id[players->selected_units[player_id].array[0]];
    if (formation_id == -1)
    {
        return;
    }
    cstrl_da_int_clear(&players->selected_units[player_id]);
    for (int i = 0; i < players->formations[player_id].unit_ids[formation_id].size; i++)
    {
        cstrl_da_int_push_back(&players->selected_units[player_id],
                               players->formations[player_id].unit_ids[formation_id].array[i]);
    }
}

void players_add_selected_units_to_formation(players_t *players, int player_id)
{
    players->selected_formation[player_id] = formations_add(&players->formations[player_id]);
    if (players->selected_formation[player_id] == -1)
    {
        return;
    }
    for (int i = 0; i < players->selected_units[player_id].size; i++)
    {
        int id = players->selected_units[player_id].array[i];
        int old_formation_id = players->units[player_id].formation_id[id];
        if (old_formation_id != -1)
        {
            int unit_index = cstrl_da_int_find_first(&players->formations[player_id].unit_ids[old_formation_id], id);
            int path_id = players->formations[player_id].path_heads[old_formation_id].array[unit_index];
            if (path_id != -1)
            {
                paths_recursive_remove(&players->paths[player_id], path_id);
            }
            formations_remove_unit(players->formations, old_formation_id, id);
        }
        formations_add_unit(&players->formations[player_id], players->selected_formation[player_id], id);
        players->units[player_id].formation_id[id] = players->selected_formation[player_id];
    }
}

void players_init(players_t *players, int count)
{
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        if (i >= count)
        {
            players->active[i] = false;
            continue;
        }
        players->active[i] = true;
        players->selected_formation[i] = -1;
        units_init(&players->units[i]);
        formations_init(&players->formations[i]);
        paths_init(&players->paths[i]);
        cstrl_da_int_init(&players->selected_units[i], 1);
    }
}
static vec3 compute_alignment(players_t *players, int player_id, int unit_id)
{
    int formation_id = players->units[player_id].formation_id[unit_id];
    if (players->formations[player_id].unit_ids[formation_id].size < 2)
    {
        return (vec3){0.0f, 0.0f, 0.0f};
    }

    vec3 average_velocity = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < players->formations[player_id].unit_ids[formation_id].size; i++)
    {
        int id = players->formations[player_id].unit_ids[formation_id].array[i];
        if (id == unit_id)
        {
            continue;
        }
        average_velocity = cstrl_vec3_add(average_velocity, players->units[player_id].velocity[id]);
    }
    average_velocity =
        cstrl_vec3_div_scalar(average_velocity, players->formations[player_id].unit_ids[formation_id].size - 1);

    return cstrl_vec3_normalize(cstrl_vec3_sub(average_velocity, players->units[player_id].velocity[unit_id]));
}

static vec3 compute_separation(players_t *players, int player_id, int unit_id)
{
    int formation_id = players->units[player_id].formation_id[unit_id];
    if (players->formations[player_id].unit_ids[formation_id].size < 2)
    {
        return (vec3){0.0f, 0.0f, 0.0f};
    }

    vec3 separation_force = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < players->formations[player_id].unit_ids[formation_id].size; i++)
    {
        int id = players->formations[player_id].unit_ids[formation_id].array[i];
        if (id == unit_id)
        {
            continue;
        }
        vec3 difference =
            cstrl_vec3_sub(players->units[player_id].position[unit_id], players->units[player_id].position[id]);
        float distance = cstrl_vec3_length(difference);
        if (distance > cstrl_epsilon && distance < SEPARATION_RADIUS)
        {
            separation_force =
                cstrl_vec3_add(separation_force, cstrl_vec3_div_scalar(cstrl_vec3_normalize(difference), distance));
        }
    }

    return cstrl_vec3_mult_scalar(cstrl_vec3_normalize(separation_force), SEPARATION_WEIGHT);
}

static vec3 compute_cohesion(players_t *players, int player_id, int unit_id)
{
    int formation_id = players->units[player_id].formation_id[unit_id];
    if (players->formations[player_id].unit_ids[formation_id].size < 2)
    {
        return (vec3){0.0f, 0.0f, 0.0f};
    }

    vec3 center = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < players->formations[player_id].unit_ids[formation_id].size; i++)
    {
        int id = players->formations[player_id].unit_ids[formation_id].array[i];
        if (id == unit_id)
        {
            continue;
        }
        cstrl_vec3_add(center, players->units[player_id].position[id]);
    }

    return cstrl_vec3_mult_scalar(
        cstrl_vec3_normalize(cstrl_vec3_sub(center, players->units[player_id].position[unit_id])), COHESION_WEIGHT);
}

static vec3 compute_flocking_force(players_t *players, int player_id, int unit_id)
{
    vec3 alignment = compute_alignment(players, player_id, unit_id);
    vec3 separation = compute_separation(players, player_id, unit_id);
    vec3 cohesion = compute_cohesion(players, player_id, unit_id);

    return cstrl_vec3_add(alignment, cstrl_vec3_add(separation, cohesion));
}

static vec3 compute_avoidance(players_t *players, int player_id, int unit_id)
{
    float ray_side_offset = UNIT_SIZE_X * 0.5f;

    vec3 forward = cstrl_vec3_normalize(players->units[player_id].velocity[unit_id]);
    vec3 left = cstrl_vec3_normalize(
        cstrl_vec3_cross(forward, cstrl_vec3_normalize(players->units[player_id].position[unit_id])));
    left = cstrl_vec3_normalize(cstrl_vec3_mult_scalar(left, ray_side_offset));
    vec3 right = cstrl_vec3_negate(left);

    vec3 avoidance_force = {0.0f, 0.0f, 0.0f};
    vec3 ahead = cstrl_vec3_add(players->units[player_id].position[unit_id],
                                cstrl_vec3_mult_scalar(forward, BASE_UNIT_VIEW_DISTANCES[0]));
    da_int excluded_nodes;
    cstrl_da_int_init(&excluded_nodes, 1);
    cstrl_da_int_push_back(&excluded_nodes, players->units[player_id].collision_id[unit_id]);
    ray_cast_result_t result =
        curved_ray_cast(get_aabb_tree(), (vec3){0.0f, 0.0f, 0.0f}, players->units[player_id].position[unit_id],
                        cstrl_vec3_normalize(ahead), &excluded_nodes);
    if (result.hit)
    {
        avoidance_force = cstrl_vec3_cross(cstrl_vec3_normalize(cstrl_vec3_sub(ahead, result.aabb_center)),
                                           cstrl_vec3_normalize(result.aabb_center));
    }
    ahead = cstrl_vec3_add(
        players->units[player_id].position[unit_id],
        cstrl_vec3_mult_scalar(cstrl_vec3_normalize(cstrl_vec3_add(forward, left)), BASE_UNIT_VIEW_DISTANCES[0]));
    result = curved_ray_cast(get_aabb_tree(), (vec3){0.0f, 0.0f, 0.0f}, players->units[player_id].position[unit_id],
                             cstrl_vec3_normalize(ahead), &excluded_nodes);
    if (result.hit)
    {
        avoidance_force = cstrl_vec3_sub(avoidance_force, left);
    }
    ahead = cstrl_vec3_add(
        players->units[player_id].position[unit_id],
        cstrl_vec3_mult_scalar(cstrl_vec3_normalize(cstrl_vec3_add(forward, right)), BASE_UNIT_VIEW_DISTANCES[0]));
    result = curved_ray_cast(get_aabb_tree(), (vec3){0.0f, 0.0f, 0.0f}, players->units[player_id].position[unit_id],
                             cstrl_vec3_normalize(ahead), &excluded_nodes);
    if (result.hit)
    {
        avoidance_force = cstrl_vec3_sub(avoidance_force, right);
    }
    return cstrl_vec3_mult_scalar(cstrl_vec3_normalize(avoidance_force), AVOIDANCE_WEIGHT);
}

static vec3 compute_seek(players_t *players, int player_id, int unit_id, vec3 target_position)
{
    vec3 end_position = get_point_on_path(
        (vec3){0.0f, 0.0f, 0.0f}, players->units[player_id].position[unit_id], target_position,
        0.1f / get_spherical_path_length(players->units[player_id].position[unit_id], target_position));
    vec3 seek_force = cstrl_vec3_sub(end_position, players->units[player_id].position[unit_id]);

    return cstrl_vec3_mult_scalar(cstrl_vec3_normalize(seek_force), SEEK_WEIGHT);
}

void players_update(players_t *players, int player_id)
{
    for (int i = 0; i < players->formations[player_id].count; i++)
    {
        for (int j = 0; j < players->formations[player_id].unit_ids[i].size; j++)
        {
            int unit_id = players->formations[player_id].unit_ids[i].array[j];
            int index = cstrl_da_int_find_first(&players->formations[player_id].unit_ids[i], unit_id);
            int path_id = players->formations[player_id].path_heads[i].array[index];
            if (path_id == -1)
            {
                continue;
            }
            vec3 end_position = players->paths[player_id].end_positions[path_id];
            vec3 steering_force = cstrl_vec3_add(compute_flocking_force(players, player_id, unit_id),
                                                 compute_avoidance(players, player_id, unit_id));
            steering_force = cstrl_vec3_add(steering_force, compute_seek(players, player_id, unit_id, end_position));
            steering_force = cstrl_vec3_normalize(steering_force);

            players->units[player_id].velocity[unit_id] =
                cstrl_vec3_normalize(cstrl_vec3_add(players->units[player_id].velocity[unit_id], steering_force));
            if (units_move(&players->units[player_id], unit_id, end_position))
            {
                players->paths[player_id].completed[path_id] = true;
                players->formations[player_id].path_heads[i].array[index] = -1;
            }
        }
    }
}

void players_move_units_normal_mode(players_t *players, int player_id, vec3 end_position)
{
    if (players->selected_formation[player_id] == -1)
    {
        players_add_selected_units_to_formation(players, player_id);
    }
    for (int i = 0; i < players->formations[player_id].unit_ids[players->selected_formation[player_id]].size; i++)
    {
        int unit_id = players->formations[player_id].unit_ids[players->selected_formation[player_id]].array[i];
        vec3 start_position = players->units[player_id].position[unit_id];
        int head_path_id = players->formations[player_id].path_heads[players->selected_formation[player_id]].array[i];
        if (head_path_id != -1)
        {
            paths_recursive_remove(&players->paths[player_id], head_path_id);
            players->formations[player_id].path_heads[players->selected_formation[player_id]].array[i] = -1;
        }
        new_path(players, player_id, start_position, end_position, false, unit_id, -1);
    }
    optimize_formation_positions(players, player_id, players->selected_formation[player_id]);
}

void players_move_units_path_mode(players_t *players, int player_id, vec3 end_position)
{
    printf("Disabling for now\n");
    return;
    if (players->selected_formation[player_id] == -1)
    {
        players_add_selected_units_to_formation(players, player_id);
    }
    for (int i = 0; i < players->formations[player_id].unit_ids[players->selected_formation[player_id]].size; i++)
    {
        int unit_id = players->formations[player_id].unit_ids[players->selected_formation[player_id]].array[i];
        vec3 start_position;
        bool path_in_queue = false;
        int prev_path = -1;
        int path_id = players->formations[player_id].path_heads[players->selected_formation[player_id]].array[i];
        if (path_id == -1)
        {
            start_position = players->units[player_id].position[unit_id];
        }
        else
        {
            while (players->paths[player_id].next[path_id] != -1)
            {
                path_id = players->paths[player_id].next[path_id];
            }
            prev_path = path_id;
            start_position = players->paths[player_id].end_positions[path_id];
            path_in_queue = true;
        }
        new_path(players, player_id, start_position, end_position, path_in_queue, unit_id, prev_path);
    }
}

bool players_select_units(players_t *players, int player_id, int viewport_width, int viewport_height,
                          vec2 selection_start, vec2 selection_end, cstrl_camera *camera, int selection_type)
{
    vec2 min = (vec2){cstrl_min(selection_start.x, selection_end.x), cstrl_min(selection_start.y, selection_end.y)};
    min.x /= viewport_width / 2.0f;
    min.y /= viewport_height / 2.0f;
    vec2 max = (vec2){cstrl_max(selection_start.x, selection_end.x), cstrl_max(selection_start.y, selection_end.y)};
    max.x /= viewport_width / 2.0f;
    max.y /= viewport_height / 2.0f;
    players->selected_formation[player_id] = -1;
    int formation_state = -1;
    quat unit_rotation = cstrl_quat_inverse(cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(camera->view)));
    cstrl_da_int_clear(&players->selected_units[player_id]);
    for (int i = 0; i < players->units[player_id].count; i++)
    {
        // TODO: make enum (or not I don't give af)
        switch (selection_type)
        {
        case 0:
            if (players->units[player_id].type[i] != CITY)
            {
                continue;
            }
            break;
        case 1:
            if (players->units[player_id].type[i] != TANK && players->units[player_id].type[i] != HUMVEE &&
                players->units[player_id].type[i] != ASTRONAUT && players->units[player_id].type[i] != ASTRONAUT_ARMED)
            {
                continue;
            }
            break;
        case 2:
            if (players->units[player_id].type[i] != JET && players->units[player_id].type[i] != PLANE)
            {
                continue;
            }
            break;
        }
        float dot = cstrl_vec3_dot(cstrl_vec3_normalize(players->units[player_id].position[i]),
                                   cstrl_vec3_normalize(camera->position));
        if (dot < 0.05f)
        {
            continue;
        }
        vec3 p0, p1, p2, p3;
        get_points(&p0, &p1, &p2, &p3, (transform){players->units[player_id].position[i], unit_rotation, UNIT_SIZE});
        vec2 p0_screen = world_to_screen(p0, (vec2){2.0f, 2.0f}, camera->projection, camera->view);
        vec2 p2_screen = world_to_screen(p2, (vec2){2.0f, 2.0f}, camera->projection, camera->view);
        vec2 p_min = (vec2){cstrl_min(p0_screen.x, p2_screen.x), cstrl_min(p0_screen.y, p2_screen.y)};
        vec2 p_max = (vec2){cstrl_max(p0_screen.x, p2_screen.x), cstrl_max(p0_screen.y, p2_screen.y)};
        if (max.x >= p_min.x && min.x <= p_max.x && max.y >= p_min.y && min.y <= p_max.y)
        {
            cstrl_da_int_push_back(&players->selected_units[player_id], i);
            if (players->units[player_id].formation_id[i] != formation_state)
            {
                if (formation_state == -1)
                {
                    formation_state = players->units[player_id].formation_id[i];
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
        players->selected_formation[player_id] = formation_state;
    }
    return players->selected_units[player_id].size > 0;
}

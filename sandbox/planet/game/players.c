#include "players.h"
#include "../helpers/helpers.h"
#include "cstrl/cstrl_util.h"

static const float FORMATION_OFFSETS[] = {0.0f, 1.0f, -1.0f, 2.0f, -2.0f};

static void new_path(players_t *players, int player_id, vec3 start_position, vec3 end_position, bool in_queue,
                     int unit_id, int prev)
{
    int new_path_id = paths_add(&players->paths[player_id], start_position, end_position, prev,
                                players->units[player_id].type[unit_id] == JET ? 0.005f : 0.0005f);
    if (new_path_id != -1)
    {
        int unit_index = cstrl_da_int_find_first(
            &players->formations[player_id].unit_ids[players->selected_formation[player_id]], unit_id);
        if (unit_index > 0)
        {
            vec3 path_vector = cstrl_vec3_sub(
                end_position,
                players->units[player_id].position
                    [players->formations[player_id].unit_ids[players->selected_formation[player_id]].array[0]]);
            path_vector = cstrl_vec3_normalize(path_vector);
            vec3 formation_line =
                cstrl_vec3_normalize(cstrl_vec3_cross(path_vector, cstrl_vec3_normalize(end_position)));
            vec3 new_end_position = cstrl_vec3_add(
                end_position, cstrl_vec3_mult_scalar(formation_line, FORMATION_OFFSETS[unit_index % 5] * UNIT_SIZE.x));
            new_end_position = cstrl_vec3_sub(
                new_end_position, cstrl_vec3_mult_scalar(path_vector, floorf(unit_index / 5.0f) * UNIT_SIZE.y * 1.5f));
            new_end_position =
                cstrl_vec3_mult_scalar(cstrl_vec3_normalize(new_end_position), 1.0f + UNIT_SIZE.x * 0.5f);
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
        }
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
}

void players_move_units_path_mode(players_t *players, int player_id, vec3 end_position)
{
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

void players_select_units(players_t *players, int player_id, int viewport_width, int viewport_height,
                          vec2 selection_start, vec2 selection_end, cstrl_camera *camera)
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
        if (players->units[player_id].type[i] == CITY || players->units[player_id].type[i] == JET)
        {
            continue;
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
}

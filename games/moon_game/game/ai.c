#include "ai.h"
#include "formation.h"
#include "players.h"
#include "units.h"
#include <math.h>
#include <stdlib.h>

static void move_randomly(ai_t *ai, int ai_id, players_t *players)
{
    if (players->formations[ai_id].count == 0)
    {
        int humvee_formation_id = -1;
        int tank_formation_id = -1;
        for (int i = 0; i < players->units[ai_id].count; i++)
        {
            if (players->units[ai_id].type[i] == HUMVEE)
            {
                if (humvee_formation_id == -1)
                {
                    humvee_formation_id = formations_add(&players->formations[ai_id]);
                }
                formations_add_unit(&players->formations[ai_id], humvee_formation_id, i);
                players->units[ai_id].formation_id[i] = humvee_formation_id;
                if (players->formations[ai_id].unit_ids[humvee_formation_id].size >= 5)
                {
                    humvee_formation_id = -1;
                }
            }
            else if (players->units[ai_id].type[i] == TANK)
            {
                if (tank_formation_id == -1)
                {
                    tank_formation_id = formations_add(&players->formations[ai_id]);
                }
                formations_add_unit(&players->formations[ai_id], tank_formation_id, i);
                players->units[ai_id].formation_id[i] = tank_formation_id;
                if (players->formations[ai_id].unit_ids[tank_formation_id].size >= 5)
                {
                    tank_formation_id = -1;
                }
            }
            else if (players->units[ai_id].type[i] == JET || players->units[ai_id].type[i] == PLANE ||
                     players->units[ai_id].type[i] == ASTRONAUT || players->units[ai_id].type[i] == ASTRONAUT_ARMED)
            {
                players->units[ai_id].formation_id[i] = formations_add(&players->formations[ai_id]);
                formations_add_unit(&players->formations[ai_id], players->units[ai_id].formation_id[i], i);
            }
        }
    }
    for (int i = 0; i < players->formations[ai_id].count; i++)
    {
        players->selected_formation[ai_id] = i;
        if (!players->formations[ai_id].moving[i])
        {
            players->formations[ai_id].moving[i] = true;
            int leader_unit_id = players->formations[ai_id].unit_ids[i].array[0];
            vec3 new_position = cstrl_vec3_add(players->units[ai_id].position[leader_unit_id],
                                               (vec3){cosf(rand()), sinf(rand()), cosf(rand())});
            if (players->units[ai_id].type[leader_unit_id] != JET &&
                players->units[ai_id].type[leader_unit_id] != PLANE)
            {
                new_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(new_position), 1.0f + UNIT_SIZE.x * 0.5f);
            }
            else
            {
                new_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(new_position), 1.0f + UNIT_SIZE.x * 5.0f);
            }
            players_move_units_normal_mode(players, ai_id, new_position);
        }
        // TODO: other logic, this feels stupid atm
        else
        {
            players->formations[ai_id].moving[i] = false;
            for (int j = 0; j < players->formations[ai_id].path_heads[i].size; j++)
            {
                if (players->formations[ai_id].path_heads[i].array[j] == -1)
                {
                    continue;
                }
                players->formations[ai_id].moving[i] = true;
            }
        }
    }
}

void ai_init(ai_t *ai, int player_count, int human_player_id)
{
    for (int i = 0; i < MAX_AI_COUNT; i++)
    {
        if (i >= player_count)
        {
            ai->active[i] = false;
            continue;
        }
        ai->active[i] = true;
        // TODO: make random
        ai->objective[i] = MOVE_RANDOMLY;
        ai->cached_unit_count[i] = 0;
    }
}

void ai_update(ai_t *ai, players_t *players, int human_player_id)
{
    for (int i = 0; i < MAX_AI_COUNT; i++)
    {
        if (!ai->active[i] || i == human_player_id)
        {
            continue;
        }

        switch (ai->objective[i])
        {
        case MOVE_RANDOMLY:
            move_randomly(ai, i, players);
            break;
        default:
            break;
        }
    }
}

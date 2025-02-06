#include "ai.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#include "players.h"
#include <math.h>

static void move_in_circle(ai_t *ai, int ai_id, players_t *players)
{
    if (players->selected_units[ai_id].size == 0)
    {
        cstrl_da_int_push_back(&players->selected_units[ai_id], 0);
    }
    if (players->paths[ai_id].count == 0 || players->paths[ai_id].active[0] == false)
    {
        vec3 new_position =
            cstrl_vec3_add(players->units[ai_id].position[0],
                           (vec3){cosf(cstrl_platform_get_absolute_time()), sinf(cstrl_platform_get_absolute_time()),
                                  cosf(cstrl_platform_get_absolute_time())});
        new_position = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(new_position), 1.0f + UNIT_SIZE.x * 0.5f);
        players_move_units_normal_mode(players, ai_id, new_position);
    }
}

void ai_init(ai_t *ai, int player_count, int human_player_id)
{
    for (int i = 0; i < MAX_AI_COUNT; i++)
    {
        if (i >= player_count || i == human_player_id)
        {
            ai->active[i] = false;
            continue;
        }
        ai->active[i] = true;
        // TODO: make random
        ai->objective[i] = GO_IN_CIRCLES;
    }
}

void ai_update(ai_t *ai, players_t *players)
{
    for (int i = 0; i < MAX_AI_COUNT; i++)
    {
        if (!ai->active[i])
        {
            continue;
        }

        switch (ai->objective[i])
        {
        case GO_IN_CIRCLES:
            move_in_circle(ai, i, players);
            break;
        default:
            break;
        }
    }
}

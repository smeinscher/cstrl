#include "ball.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <stdlib.h>

static int g_current_ball_index = 0;

void balls_init(balls_t *balls)
{
    for (int i = 0; i < MAX_BALLS; i++)
    {
        cstrl_da_int_init(&balls->cups_hit[i], 4);
        balls->position[i] = (vec2){-100.0f, -100.0f};
        balls->target[i] = (vec2){-100.0f, -100.0f};
        balls->origin[i] = (vec2){-100.0f, -100.0f};
        balls->error[i] = (vec2){0.0f, 0.0f};
        balls->velocity[i] = (vec2){0.0f, 0.0f};
        balls->speed[i] = 0.0f;
        balls->bounce_length[i] = 0.0f;
        balls->angle[i] = 0.0f;
        balls->cup_hit_distance[i] = 0.0f;
        balls->shot_complete[i] = false;
        balls->team[i] = -1;
        balls->active[i] = false;
    }
}

void balls_shoot(balls_t *balls, vec2 target, vec2 origin, vec2 error, float speed, int team)
{
    cstrl_da_int_clear(&balls->cups_hit[g_current_ball_index]);
    balls->position[g_current_ball_index] = origin;
    balls->target[g_current_ball_index] = cstrl_vec2_add(target, error);
    balls->origin[g_current_ball_index] = origin;
    balls->error[g_current_ball_index] = error;
    balls->velocity[g_current_ball_index] =
        cstrl_vec2_normalize(cstrl_vec2_sub(balls->target[g_current_ball_index], origin));
    balls->speed[g_current_ball_index] = speed;
    balls->angle[g_current_ball_index] =
        team == 0
            ? atan2(origin.y - balls->target[g_current_ball_index].y, origin.x - balls->target[g_current_ball_index].x)
            : atan2(balls->target[g_current_ball_index].y - origin.y, balls->target[g_current_ball_index].x - origin.x);
    balls->cup_made[g_current_ball_index] = -1;
    balls->team[g_current_ball_index] = team;
    balls->shot_complete[g_current_ball_index] = false;
    balls->active[g_current_ball_index++] = true;
}

void balls_update(balls_t *balls, cups_t *cups, players_t *players)
{
    for (int i = 0; i < MAX_BALLS; i++)
    {
        if (!balls->active[i] || balls->shot_complete[i])
        {
            continue;
        }
        balls->position[i] =
            cstrl_vec2_add(balls->position[i], cstrl_vec2_mult_scalar(balls->velocity[i], balls->speed[i]));
        if (cstrl_vec2_length(cstrl_vec2_sub(balls->position[i], balls->origin[i])) >=
            cstrl_vec2_length(cstrl_vec2_sub(balls->target[i], balls->origin[i])))
        {
            balls->position[i] = balls->target[i];
            int cup_hit = cups_shot_test(cups, BALL_SIZE, balls->position[i], &balls->cup_hit_distance[i]);
            if (cup_hit >= 0)
            {
                if (cstrl_da_int_find_first(&balls->cups_hit[i], cup_hit) == CSTRL_DA_INT_ITEM_NOT_FOUND)
                {
                    cstrl_da_int_push_back(&balls->cups_hit[i], cup_hit);
                }
                float t = (float)players->stats[players->current_player_turn].accuracy / 100.0f;
                float d = (1.0f - t) * 5.0f + t * 3.5f;
                if (balls->cup_hit_distance[i] < CUP_SIZE / d)
                {
                    balls->cup_made[i] = cup_hit;
                    balls->shot_complete[i] = true;
                }
                else
                {
                    balls->origin[i] = balls->position[i];
                    balls->bounce_length[i] = (float)(rand() % INITIAL_BOUNCE_DISTANCE * 100 + 1) / 100.0f;
                    balls->velocity[i].y = (float)(rand() % 1000) / 1000.0f;
                    balls->velocity[i] = cstrl_vec2_normalize(balls->velocity[i]);
                    balls->target[i] = cstrl_vec2_add(
                        balls->origin[i], cstrl_vec2_mult_scalar(balls->velocity[i], balls->bounce_length[i]));
                    balls->error[i] = (vec2){0.0f, 0.0f};
                }
            }
            else
            {
                balls->shot_complete[i] = true;
            }
        }
    }
}

void balls_clear(balls_t *balls)
{
    for (int i = 0; i < MAX_BALLS; i++)
    {
        balls->active[i] = false;
        balls->shot_complete[i] = false;
        cstrl_da_int_clear(&balls->cups_hit[i]);
        balls->cup_made[i] = -1;
    }
    g_current_ball_index = 0;
}
